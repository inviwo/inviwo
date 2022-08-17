import refactoring  # Note: refactoring.py need to be in the current working directory
import argparse
import colorama
import time
colorama.init()

excludespatterns = ["*/ext/*", "*moc_*", "*/proteindocking/*", "*/proteindocking2/*",
                    "*/genetree/*", "*.DS_Store", "*DS_mapp", ".md", "*.suo", "*.h5",
                    "*.jpg", "*.JPG", "*.jpeg", "*.lib", "*.dll", "*.inv", "*.dat", "*.ivf",
                    "*.tiff", "*.png", "*.ttf", "*.tif", "*.pyc", "*.raw", "*.bmp", "*.wav",
                    "*.xcf", "*.ico", "*.icns", "*.qch", "*.qhc", "*.exr", "*.pwm", "*.pvm",
                    "*.pdf", "*.otf", "*.exe", "*.fbx", "*.svg", "*.itf", "*.qrc", "*.md",
                    "*/.git*", "*/.clang-format", "*/LICENSE", ".git", "Jenkinsfile",
                    ".gitattributes", "*/AUTHORS", "" "*/tools/meta/templates/*", "*.natvis",
                    "*/depends.cmake", "*moduledefine.h", "*moduledefine.hpp", "*/config.json",
                    "*.js", "*/CMakeLists.txt"]


def update(paths, year):
    copyright_replacements = {
        f"(\\s*[*#]\\s+Copyright \\(c\\) 20\\d\\d-)(?!{year})20\\d\\d( Inviwo Foundation\\s*)":
        f"\\g<1>{year}\\g<2>",
        f"(\\s*[*#]\\s+Copyright \\(c\\) )(?!{year})20\\d\\d( Inviwo Foundation\\s*)":
        f"\\g<1>\\g<2>-{year}\\g<3>"
    }

    files = refactoring.find_files(paths, ['*'], excludes=excludespatterns)
    print("Looking in {} files".format(len(files)))

    summary = ""
    for pattern, replacement in copyright_replacements.items():
        print("Matches: {}".format(pattern))
        matches = refactoring.find_matches(files, pattern)

        print("\nReplacing:")
        refactoring.replace_matches(matches, pattern, replacement)
        summary += "Replaced {} matches of {}\n".format(len(matches), pattern)

    print("\n")
    print(summary)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description="Run regression tests",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter
    )
    parser.add_argument('-p', '--paths', type=str, nargs='+', action="store",
                        help='Paths to inviwo repos')
    args = parser.parse_args()
    year = time.localtime().tm_year

    update(args.paths, year)




