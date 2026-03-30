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

def main():
    desc = '''Script for updating the regression test images. Example call:\n\n
     python.exe ./update-regression-images.py --save --min_percent 0.00 --max_percent 0.05 
            -j "https://inviwo.org/regression/macos/refs/pull/1928/merge"\n
            -m "<module dir> [<module dir>...]
    '''

    parser = argparse.ArgumentParser(description=desc)
    parser.add_argument('-j', '--job', help='Jenkins url to job', required=True)
    parser.add_argument('-m', '--modules', nargs='*', type=Path, default=[],
                        help='List of module directories"')
    parser.add_argument('-s', '--save', action="store_true", help="Save local reference images")
    parser.add_argument('-v', '--verbose', action="store_true", help="Print verbose output")
    parser.add_argument('--min_percent', type=float, default=0.0,
                        help='Only images with errors larger then min_percent')
    parser.add_argument('--max_percent', type=float, default=1.0,
                        help='Only images with errors smaller then max_percent')

    parser.add_argument('--min_pixels', type=int, default=0,
                        help='Only images with errors larger then min_pixels')
    parser.add_argument('--max_pixels', type=int, default=1024**3,
                        help='Only images with errors smaller then max_pixels')

    args = parser.parse_args()

    if args.job.endswith("/report.html"):
        baseDir = args.job[:-len("/report.html")]
    elif args.job.endswith("/report.json"):
        baseDir = args.job[:-len("/report.json")]
    elif args.job.endswith("/"):
        baseDir = args.job[:-len("/")]
    else:
        baseDir = args.job

    request = requests.get(baseDir + "/report.json")
    if not request.ok:
        print_error(f"Request failed: {request.status_code} {request.reason}")
        exit(1)

    report = request.json()
    imgcount = 0

    def findLocalPath(moduleName: str):
        for modDir in args.modules:
            for subDir in modDir.iterdir():
                if subDir.is_dir() and subDir.name == moduleName:
                    return subDir / "tests" / "regression"
                    
        print_error(f"Could nof find local directory for module: {module}")
        exit(1)   


    for testName, testResult in report.items():
        module = testResult['module']
        name = testResult['name']

        [_, testDir] = testResult['outputdir'].split(f"/{module}/{name}/")

        for imageTest in testResult['images']['tests']:
            test = ImageTest(**imageTest)
            if test.test_size != test.ref_size:
                imgcount += 1
                print(testName)
                print_error(
                    "   {0.image} has wrong size: {0.test_size} != {0.ref_size}".format(test))
            elif (test.difference_percent > args.min_percent and 
                  test.difference_percent <= args.max_percent and 
                  test.difference_pixels > args.min_pixels and
                  test.difference_pixels <= args.max_pixels):
                imgcount += 1
                print_error(testName)
                print_warn(f"   {test.image:30} Diff: {test.difference_percent:<9.4}%,"
                           + f" Max: {test.max_differences}, "
                           + f"#Pixels: {test.difference_pixels}")

                src = testResult['images']['imgs-map'][test.image]
                #src = f"{baseDir}/{module}/{name}/{testDir}/imgtest/{test.image}"
                dst = findLocalPath(module) / name / test.image

                print(f"   src: {src}")
                print(f"   dst: {dst}")
                if args.save:
                    print_warn("   reference image saved")
                    img = requests.get(src)
                    with open(dst, "wb") as f:
                        f.write(img.content)
            elif args.verbose:
                print(testName)
                print(f"   {test.image:30} Diff: {test.difference_percent:<9.4}%,"
                      + f" Max: {test.max_differences}, "
                      + f"#Pixels: {test.difference_pixels}")
                        
    print_warn("\n{} errors found".format(imgcount))


if __name__ == '__main__':
    main()
