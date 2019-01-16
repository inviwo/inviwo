
import argparse
import difflib
import re
import subprocess
import json
import sys
import io

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


    with open(args.compile_commands) as f: 
        data = json.load(f)
    
    with open(args.output, 'w') as out:
        for item in data:
            filename = item['file']
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

