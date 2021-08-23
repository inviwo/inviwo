import json
from pathlib import Path

# /opt/homebrew/Cellar/llvm/14.0.6_1/include/c++/v1


base = Path("/opt/homebrew/Cellar/llvm/14.0.6_1/include/c++/v1")

maps = []

for f in base.glob('*'):
    if f.is_dir() and f.name.startswith("__"):
        dst = f.name[2:]
        for i in f.glob("*.h"):
            src = i.relative_to(base).as_posix()
            print(f"<{src}> -> <{dst}>")
            maps.append({"include": [f'<{src}>', "private", f'<{dst}>', "public"]})


with open("libcxxgen.imp", 'w') as f:
    json.dump(maps, f)
