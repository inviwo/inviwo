import refactoring  # Note: refactoring.py need to be in the current working directory

import colorama
colorama.init()

paths = [
    "C:/Users/petst55.AD/Documents/Inviwo/inviwo-stage"  # ,
    #  "C:/Users/petst55/Work/Inviwo/Inviwo-research",
    #  "C:/Users/petst55/Work/Inviwo/Inviwo-modules"
]

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

copyright_replacements = {
    r"(\s*[*#]\s+Copyright \(c\) 201\d-)20(?:1\d|2[0])( Inviwo Foundation\s*)": r"\g<1>2021\g<2>",
    r"(\s*[*#]\s+Copyright \(c\) )(20(?:1\d|2[0]))( Inviwo Foundation\s*)": r"\g<1>\g<2>-2021\g<3>"
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
