# Script to update regression reference images
# example usage:
# python.exe .\update-regression-images.py --user <github username> --token <github token> --save --min 0.00 --max 0.05 \
#  -j "http://jenkins.inviwo.org:8080/job/inviwo/job/feature%252Fworkspaces2" \
#  -r inviwo=C:/Users/petst55/Work/Inviwo/Inviwo-dev modules=C:/Users/petst55/Work/Inviwo/Inviwo-modules

import argparse
import requests
from pathlib import Path


try:
    import colorama
    colorama.init()

    def print_error(mess, **kwargs):
        print(colorama.Fore.RED + colorama.Style.BRIGHT + mess + colorama.Style.RESET_ALL, **kwargs)

    def print_warn(mess, **kwargs):
        print(colorama.Fore.YELLOW + colorama.Style.BRIGHT + mess + colorama.Style.RESET_ALL,
              **kwargs)

except ImportError:
    def print_error(mess, **kwargs):
        print(mess, **kwargs)

    def print_warn(mess, **kwargs):
        print(mess, **kwargs)


class ImageTest:
    def __init__(self, **entries):
        self.image = None
        self.difference_percent = 0.0
        self.max_differences = 0.0
        self.difference_pixels = None
        self.test_size = [None, None]
        self.ref_size = [None, None]
        self.test_mode = "RGBA"
        self.ref_mode = "RGBA"
        self.__dict__.update(entries)


def updateImg(src, dst, auth):
    img = requests.get(src, auth=auth)
    with open(dst, "wb") as f:
        f.write(img.content)


def main():
    desc = '''Script for updating the regression test images. Example call:\n\n
     python.exe ./update-regression-images.py --user <github username> --token <github token>\n
            --save --min 0.00 --max 0.05 -j "http://jenkins.inviwo.org:8080/job/inviwo/job/feature%252Fworkspaces2"\n
            -r inviwo=<path to repo> modules=<path to repo>\n
    Tokens can be created at https://github.com/settings/tokens and needs user access to authenticate with jenkins
    '''

    parser = argparse.ArgumentParser(description=desc)
    parser.add_argument('-u', '--user', help='Github user name', required=True)
    parser.add_argument(
        '-t', '--token', help='Github password token (see https://github.com/settings/tokens)', required=True)
    parser.add_argument('-j', '--job', help='Jenkins url to job', required=True)
    parser.add_argument('-r', '--repos', required=True, nargs='+',
                        help='List of name=repo "inviwo=C:/.../inviwo"')
    parser.add_argument('-s', '--save', action="store_true", help="Save local reference images")
    parser.add_argument('--min', type=float, default=0.0,
                        help='Only images with errors larger then min')
    parser.add_argument('--max', type=float, default=0.1,
                        help='Only images with errors smaller then max')

    args = parser.parse_args()

    auth = (args.user, args.token)

    localRepos = {x.split('=')[0]: Path(x.split('=')[1]) for x in args.repos}
    jsonReport = Path('Regression', 'report.json')

    request = requests.get(args.job + "/" + jsonReport.as_posix(), auth=auth)
    if not request.ok:
        print_error(f"Request failed: {request.status_code} {request.reason}")
        exit(1)

    report = request.json()
    imgcount = 0

    for testName, testResult in report.items():
        basedir, testdir = [Path(x) for x in testResult['outputdir'].split('/regress/')]
        srcdir = Path(testResult['path'])
        repo = srcdir.relative_to(basedir).parts[0]
        if repo not in localRepos.keys():
            print_error("Can't find local path for repo: '{}'".format(repo))
            parser.print_help()
            exit(1)

        localdir = localRepos[repo] / srcdir.relative_to(basedir / repo)

        for imageTest in testResult['images']['tests']:
            test = ImageTest(**imageTest)
            if test.test_size != test.ref_size:
                imgcount += 1
                print(testName)
                print_error(
                    "   {0.image} has wrong size: {0.test_size} != {0.ref_size}".format(test))
            elif (test.difference_percent < args.max and test.difference_percent > args.min
                  and test.difference_percent > 0.0):
                imgcount += 1
                print_error(testName)
                print_warn(f"   {test.image:30} Diff: {test.difference_percent:<9.4}%,"
                           + f" Max: {test.max_differences}, "
                           + f"#Pixels: {test.difference_pixels}")
                src = (args.job + "/"
                       + (Path('Regression') / testdir / 'imgtest' / test.image).as_posix())
                dst = localdir / test.image
                print(f"   src: {src}")
                print(f"   dst: {dst}")
                if args.save:
                    print_warn("   reference image saved")
                    updateImg(src, dst, auth)

    print_warn("\n{} errors found".format(imgcount))


if __name__ == '__main__':
    main()
