# generate glm include what you use mapping

import json

from pathlib import Path


def begins_with(alist, prefix):
    if len(prefix) > len(alist):
        return False
    return alist[:len(prefix)] == prefix


glm_base = "/Users/peter/Work/Inviwo/inviwo/ext/glm/glm"

base = Path(glm_base)

maps = []

for f in base.glob('**/*.inl'):
    src = f.relative_to(base).as_posix()
    dstpath = f.with_name(f.stem + ".hpp")
    if dstpath.exists():
        dst = dstpath.relative_to(base)
    elif f.stem.endswith("_simd") and f.with_name(f.stem[:-5] + ".hpp").exists():
        dst = f.with_name(f.stem[:-5] + ".hpp").relative_to(base)
    elif (begins_with(f.relative_to(base).parts, ('glm', 'detail')) and f.stem.startswith("func_")
          and (base / 'glm' / (f.stem[5:] + '.hpp')).exists()):
        dst = Path('glm') / (f.stem[5:] + '.hpp')
    elif (begins_with(f.relative_to(base).parts, ('glm', 'detail')) and f.stem.startswith("func_")
          and f.stem.endswith('_simd')
          and (base / 'glm' / (f.stem[5:-5] + '.hpp')).exists()):
        dst = Path('glm') / (f.stem[5:-5] + '.hpp')
    else:
        print(f"Missing dst {src}")
        continue

    if begins_with(dst.parts, ('glm', 'detail')):
        if dst.stem.startswith("type_") and (base / 'glm' / (dst.stem[5:] + '.hpp')).exists():
            dst2 = Path('glm') / (dst.stem[5:] + '.hpp')
            # print(src, dst, dst2)
            maps.append({"include": [f'"{dst}"', "private", f'<{dst2}>', "public"]})
            maps.append({"include": [f'<{dst}>', "private", f'<{dst2}>', "public"]})
            dst = dst2
        else:
            print(f"Priv {src}, {dst}")

    maps.append({"include": [f'"{src}"', "private", f'<{dst}>', "public"]})
    maps.append({"include": [f'<{src}>', "private", f'<{dst}>', "public"]})

for i in maps:
    (s, _, d, _) = i["include"]
    print(f"{s:50} {d}")

with open("glm.imp", 'w') as f:
    json.dump(maps, f, indent=4)

