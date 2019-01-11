import argparse
import difflib
import re
import string
import subprocess
import json
import StringIO

def main():
    parser = argparse.ArgumentParser(description="")
    parser.add_argument('-compile_commands', default='compile_commands.json',
                        help='location of binary to use for clang-format')
    parser.add_argument('-binary', default='clang-format-6.0',
                        help='location of binary to use for clang-format')
    args = parser.parse_args()

    with open(args.compile_commands) as f:
        data = json.load(f)
        for item in data:
            filename = item['file']
            command = [args.binary, filename]
            p = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=None, stdin=subprocess.PIPE)
            stdout, stderr = p.communicate()
            if p.returncode != 0:
                sys.exit(p.returncode);

            with open(filename) as f:
                code = f.readlines()
            
            formatted_code = StringIO.StringIO(stdout).readlines()

            diff = difflib.unified_diff(code, formatted_code,
                                        filename, filename,
                                        '(before formatting)', '(after formatting)')
            diff_string = string.join(diff, '')
            if len(diff_string) > 0:
                sys.stdout.write("Warning: Inconsistent format" + filename)
                sys.stdout.write(diff_string)


if __name__ == '__main__':
    main()