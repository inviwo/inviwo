#!/usr/bin/env python3
"""Extract bare vcpkg port names from `vcpkg depend-info --format=list` output.

The baseline comparison uses `versions/baseline.json`, which is keyed by bare
port name. depend-info list output may include features and host qualifiers,
for example `sqlite3[json1]:host: vcpkg-cmake:host`.
"""

from __future__ import annotations

import re
import sys

FEATURES = re.compile(r"\[[^\]]*\]")
PORT_NAME = re.compile(r"^[A-Za-z0-9][A-Za-z0-9_.+-]*$")


def normalize_spec(spec: str) -> str | None:
    spec = spec.split(":", 1)[0]
    spec = FEATURES.sub("", spec)
    return spec if PORT_NAME.fullmatch(spec) else None


def parse_dependency_names(text: str) -> list[str]:
    names: set[str] = set()
    for line in text.splitlines():
        if ":" not in line:
            continue
        name = normalize_spec(line.strip())
        if name:
            names.add(name)
    return sorted(names)


def main() -> int:
    print("\n".join(parse_dependency_names(sys.stdin.read())))
    return 0


if __name__ == "__main__":
    sys.exit(main())
