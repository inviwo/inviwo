import subprocess
import colorama
import refactoring  # Note: refactoring.py need to be in the current working directory

colorama.init()

clangformat = "C:/Program Files/Microsoft Visual Studio/18/Community/VC/Tools/Llvm/x64/bin/clang-format.exe"

paths = [
    "C:/Users/petst55.AD/Projects/inviwo-stage"
]

excludespatterns = ["*/ext/*", "*/templates/*", "*/tools/codegen/*", "*moc_*", "*cmake*"]

files = refactoring.find_files(paths, ['*.h', '*.hpp', '*.cpp'], excludes=excludespatterns)

for file in files:
    print("check " + file)
    with subprocess.Popen([clangformat, "-i", file],
                          stdout=subprocess.PIPE,
                          stderr=subprocess.STDOUT,
                          universal_newlines=True) as proc:
        for line in proc.stdout:
            print(line, end='', flush=True)
