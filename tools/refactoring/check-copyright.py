import sys
import re
import datetime
from enum import Enum
import refactoring  # Note: refactoring.py need to be in the current working directory

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


# match Inviwo Foundation copyright line
copyrightLine_regex = re.compile(r"\s*[*#]\s+Copyright \(c\) (.*?) Inviwo Foundation\s*")
copyrightSingleYear_regex = re.compile(r"^(\d\d\d\d)$")
copyrightRangeYear_regex = re.compile(r"^(\d\d\d\d)-(\d\d\d\d)$")


class CopyrightState(Enum):
    Correct = 0
    Outdated = 1
    MalformedYear = 2
    Missing = 3  # or malformed


def findCopyrightYears(filehandle):
    for (i, line) in enumerate(filehandle):
        if linematch := copyrightLine_regex.search(line):
            return i, line, linematch.group(1)
    return None


def checkYear(yearStr, currentYear):
    if rangeMatch := copyrightRangeYear_regex.match(yearStr):
        startYear = int(rangeMatch.group(1))
        endYear = int(rangeMatch.group(2))

        if endYear != currentYear:
            return CopyrightState.Outdated
        elif startYear > endYear:
            return CopyrightState.Malformed
        else:
            return CopyrightState.Correct

    elif sigleYearMatch := copyrightSingleYear_regex.match(yearStr):
        year = int(sigleYearMatch.group(1))

        if year != currentYear:
            return CopyrightState.Outdated
        else:
            return CopyrightState.Correct

    else:
        return CopyrightState.MalformedYear


def checkfile(filehandle, filename, currentYear):
    if res := findCopyrightYears(filehandle):
        i, line, yearStr = res
        state = checkYear(yearStr, currentYear)
        if state == CopyrightState.Outdated:
            print_error("Copyright outdated in " + str(filename))
            print(str(i) + ": " + line.rstrip())
            return 1  # flag copyright error
        elif state == CopyrightState.MalformedYear:
            print_error("Copyright year malformed in " + str(filename))
            print(str(i) + ": " + line.rstrip())
            print("Expecting either '201x' or '201x-201y'")
            return 1  # flag copyright error
        elif state == CopyrightState.Correct:
            return 0

    else:
        print_warn("Copyright information missing in " + str(filename))
        return 0  # TODO: flag copyright error


def test(file, currentYear):
    with open(file, 'r', encoding="UTF-8") as f:
        try:
            return checkfile(f, file, currentYear)
        except UnicodeDecodeError:
            print_warn(file + ": File not utf-8 encoded, "
                       "fall-back to Western encoding (Windows 1252)")
            with open(file, 'r', encoding="cp1252") as f:
                try:
                    return checkfile(f, file, currentYear)
                except UnicodeDecodeError:
                    print_error("Encoding error: " + file)
    return 0


excludespatterns = ["*.DS_Store", "*DS_mapp", ".md", "*.suo", "*.h5", "*.gz",
                    "*.jpg", "*.JPG", "*.jpeg", "*.lib", "*.dll", "*.inv", "*.dat", "*.ivf",
                    "*.tiff", "*.png", "*.ttf", "*.tif", "*.pyc", "*.raw", "*.bmp", "*.wav",
                    "*.xcf", "*.ico", "*.icns", "*.qch", "*.qhc", "*.exr", "*.pwm", "*.pvm",
                    "*.pdf", "*.otf", "*.exe", "*.fbx", "*.svg", "*.itf", "*.qrc", "*.md",
                    "*.css", "*.obj", "*.mpvm", "*.csv", "*.html", "*.rc", "*.pro", "*.plist",
                    "*.off", "*.js", "*.monopic", "*.natvis", "*.json", "*.manifest",

                    "*/LICENSE", "*license.txt", "*LICENSE.txt", "*License.txt", "*.LICENSE",
                    "*/AUTHORS", "*/CITATION.cff", "*.README",

                    "*/depends.cmake", "*moduledefine.h", "*moduledefine.hpp", "*/meta.cmake",
                    "*/inviwometadefine.hpp",

                    "*/.git*", ".gitattributes", "*/.clang-format", "*/CMakeLists.txt",
                    "*/.cache/*", "*/compile_commands.json", "*/CMakeUserPresets.json",
                    "*/CMakePresets.json", "*/vcpkg.json", "*/Jenkinsfile", "*/.issuetracker",
                    "*/compile_commands.json",

                    "*/__init__.py",
                    "*/requirements.txt",
                    "*/modules/python3qt/templates/templateprocessor.py",
                    "*/regression/*.py",
                    "*/data/scripts/*.py",

                    "*/tools/tracy/include/inviwo/tracy/*",
                    "*/tools/meta/templates/*",
                    "*/tools/converters/*",
                    "*/tools/codegen/*",
                    "*/tools/refactoring/*",
                    "*/tools/jenkins/*",
                    "*/tools/iwyu/*",
                    "*/tools/vcpkg/*",
                    "*/tools/docker/*",
                    "*/tools/tracy/include/inviwo/tracy/*",

                    "*/ext/*",
                    "*/docs/*",
                    "*/cmake/*",
                    "*/resources/*",

                    "*/proteindocking/*", "*/proteindocking2/*", "*/genetree/*",
                    ]


paths = sys.argv[1:]
files = refactoring.find_files(paths, ['*'], excludes=excludespatterns)
currentYear = datetime.date.today().year

print("Checking copyright for year " + str(currentYear))
print("Looking in " + str(len(files)) + " files")

errors = 0
for file in files:
    errors = errors + test(file, currentYear)

if errors > 0:
    print_error("\nFound " + str(errors) + " copyright errors")

sys.exit(errors)
