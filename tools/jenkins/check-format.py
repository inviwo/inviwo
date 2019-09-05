import os
import io
import re
import sys
import json
import codecs
import pathlib
import fnmatch
import difflib
import argparse
import subprocess

missing_modules = {}
try:
    import git
except ImportError:
    missing_modules['gitpython'] = "needed for git access"

if len(missing_modules)>0: 
    print_error("Error: Missing python modules:")
    for k,v in missing_modules.items():
        print_error("    {:20s} {}".format(k,v))    
    print_info("    To install run: 'python -m pip install {}'".format(" ".join(missing_modules.keys())))
    exit()

def test_cmd(cmd):
    try:  
        subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE).check_returncode()
        return True
    except:
        return False

def find_files(paths, extensions, excludes=[""]):
    return (f for ext in extensions for path in paths for f in pathlib.Path(path).rglob(ext) 
                if not any(fnmatch.fnmatch(f, x) for x in excludes))

def getModifiedFiles(repoPaths, extensions, excludes=[""]):
    files = []
    for path in repoPaths:
        repo = git.Repo(path)
        repo.remotes.origin.fetch("+refs/heads/master:refs/remotes/origin/master", no_tags=True)
        mb = repo.merge_base(repo.head, repo.remotes.origin.refs.master)[0]
        wdir = pathlib.Path(repo.working_dir)
        relevantFiles = set()
        for i in mb.diff(repo.head):
            relevantFiles.add(wdir / i.a_path)
            relevantFiles.add(wdir / i.b_path)
        files.extend([pathlib.Path(f) for f in relevantFiles 
                    if any(f.match(ext) for ext in extensions) 
                    and not any(fnmatch.fnmatch(f, x) for x in excludes)
                    and f.exists()])
    return files

def main():
    parser = argparse.ArgumentParser(description="")
    parser.add_argument('-b', '--binary', help='location of binary to use for clang-format')
    parser.add_argument('-o', '--output', default='clang-format-result.diff', help='output file')
    parser.add_argument('-m', '--master', action="store_true", help='run on all found files not just modifed ones')
    parser.add_argument('-f', '--fix', action="store_true", help='apply the format fixes')
    parser.add_argument('repos', type=str, nargs='+', help='path to a code repo')
    
    args = parser.parse_args()

    if not args.binary:
        for ext in ["", "-6.0", "-7.0", "-8.0", "-9.0"]:
            if test_cmd(["clang-format" + ext, "--version"]): 
                args.binary = "clang-format" + ext
                break
        else:
            sys.stdout.write("Could not find clang format please use the '-binary'\n")
            sys.exit(1);

    version = subprocess.getoutput(args.binary + " --version")
    sys.stdout.write(version + "\n")
    
    extensions = ['*.h', '*.hpp', '*.cpp']
    excludes = ["*/ext/*", "*/templates/*", "*/tools/codegen/*" , "*moc_*", "*cmake*"]
    if args.master:
        files = find_files(args.repos, extensions, excludes)
    else:
        files = getModifiedFiles(args.repos, extensions, excludes)
    
    with codecs.open(args.output, 'w', encoding="UTF-8") as out:
        for filename in files:
            command = [args.binary, str(filename)]
            p = subprocess.Popen(command,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE)
            formatted_code_raw, err_raw = p.communicate()
            formatted_code = codecs.decode(formatted_code_raw, encoding="UTF-8")
            
            if p.returncode != 0:
                sys.stdout.write("Problem checking format for: " + str(filename) + "\n")
                err = codecs.decode(err_raw, encoding="UTF-8")
                sys.stdout.write("Errore: \n")
                sys.stdout.write(err)
                sys.stdout.write("\nOutput: \n")
                sys.stdout.write(formatted_code)
                sys.stdout.write("\n")
                sys.exit(p.returncode);

            formatted_code = codecs.decode(formatted_code_raw, encoding="UTF-8")

            with codecs.open(filename, 'r', encoding="UTF-8") as f:
                code = f.read().split('\n')

            diff = difflib.unified_diff(code, formatted_code.split('\n'), 
                str(filename) + " (original)", 
                str(filename) + " (formatted)", '', '', 3, "")
            diff_string = '\n'.join(diff)
            if len(diff_string) > 0:
                sys.stdout.write(str(filename) + " Warning: Inconsistent format\n")
                sys.stdout.write(diff_string + "\n")
                out.write(diff_string + "\n")
                if args.fix:
                    with codecs.open(filename, 'w', encoding="UTF-8") as f:
                        f.write(formatted_code)

if __name__ == '__main__':
    main()

