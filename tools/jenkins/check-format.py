
import argparse
import difflib
import re
import subprocess
import json
import sys
import io
import pathlib

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

def getReleatedFiles():
    repo = git.Repo("../inviwo")
    repo.remotes.origin.fetch("+refs/heads/master:refs/remotes/origin/master", no_tags=True)
    if repo.head.object == repo.remotes.origin.refs.master.object: # this is the master brach
        def is_relevant(file):
            return True
        return is_relevant
    else:
        mb = repo.merge_base(repo.head, repo.remotes.origin.refs.master)[0]
        wdir = pathlib.Path(repo.working_dir)
        relevantFiles = set()
        for i in mb.diff(repo.head):
            relevantFiles.add(wdir / i.a_path)
            relevantFiles.add(wdir / i.b_path)
        def is_relevant(file):
            return pathlib.Path(file) in relevantFiles
        return is_relevant

def test_cmd(cmd):
    try:  
        subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE).check_returncode()
        return True
    except:
        return False

def main():
    parser = argparse.ArgumentParser(description="")
    parser.add_argument('-compile_commands', default='compile_commands.json',
                        help='location of binary to use for clang-format')
    parser.add_argument('-binary', help='location of binary to use for clang-format')
    parser.add_argument('-output', default='clang-format-result.diff',
                        help='output file')
    
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
    sys.stdout.write(version)
    
    with open(args.compile_commands) as f: 
        data = json.load(f)
    
    is_relevant = getReleatedFiles()

    with open(args.output, 'w') as out:
        for item in data:
            filename = item['file']
            if not is_relevant(filename): continue
            command = [args.binary, filename]
            p = subprocess.Popen(command,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                universal_newlines=True)
            formatted_code, err = p.communicate()
            if p.returncode != 0:
                sys.write(err)
                sys.exit(p.returncode);

            with open(filename) as f:
                code = f.read().split('\n')

            diff = difflib.unified_diff(code, formatted_code.split('\n'), 
                filename + " (original)", 
                filename + " (formatted)", '', '', 3, "")
            diff_string = '\n'.join(diff)
            if len(diff_string) > 0:
                sys.stdout.write("\n\nWarning: Inconsistent format " + filename + "\n")
                sys.stdout.write(diff_string)
                out.write("\n\nWarning: Inconsistent format " + filename + "\n")
                out.write(diff_string)

if __name__ == '__main__':
    main()

