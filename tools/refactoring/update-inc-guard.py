import refactoring  # Note: refactoring.py need to be in the current working directory
import codecs
import re
import os

paths = [
    "C:/Users/petst55.AD/Documents/Inviwo/inviwo-stage/modules"
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

files = refactoring.find_files(paths, extensions=['*.h', '*.hpp'],
                               excludes=excludespatterns)

copyright = r"^(?P<p1>\/[*]{81}\r?\n(?: [*].*?\r?\n){26} [*]{81}\/\r?\n)"
guard1 = r"\s*#ifndef (?P<guard>IVW_[A-Z0-9_]+)\r?\n#define (?P=guard)\r?\n"
code = r"(?P<p2>(?:.|\r?\n)+?)"
guard2 = r"\r?\n#endif\s+// (?P<end>[A-Z0-9_]+)\r?\n?$"

full = re.compile(copyright + guard1 + code + guard2, re.MULTILINE)
begin = re.compile(copyright + guard1, re.MULTILINE)
end = re.compile(guard2, re.MULTILINE)

haspragma = re.compile(r"#pragma once", re.MULTILINE)

for file in files:
    with codecs.open(file, 'r', encoding="UTF-8") as f:
        try:
            text = f.read()
        except UnicodeDecodeError:
            refactoring.print_error("Encoding error: " + file)
            continue

    if haspragma.search(text):
        continue

    print(file)
    if not begin.match(text):
        refactoring.print_error(f"    No Begin")
        continue

    if m := full.match(text):
        if m.group('guard') != m.group('end'):
            refactoring.print_error(f"    Mismatch {m.group('guard')} {m.group('end')}")
            continue

        refactoring.print_warn("    Match")
        with codecs.open(file, 'w', encoding="UTF-8") as f:
            f.write(full.sub(r"\g<p1> " + os.linesep + "#pragma once"
                             + os.linesep + r"\g<p2>", text))
    else:
        refactoring.print_error("    No Match")
