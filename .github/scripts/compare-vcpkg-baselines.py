#!/usr/bin/env python3
"""Compare two vcpkg baseline.json files and print changed dependencies.

Usage:
    compare-vcpkg-baselines.py <old_baseline.json> <new_baseline.json> <deps.txt>

deps.txt should contain one dependency name per line.
Output is one line per changed dependency: "name: old_version -> new_version"
"""
import json
import sys


def main() -> None:
    if len(sys.argv) != 4:
        print(__doc__, file=sys.stderr)
        sys.exit(1)

    old_file, new_file, deps_file = sys.argv[1], sys.argv[2], sys.argv[3]

    with open(old_file) as f:
        old = json.load(f)["default"]
    with open(new_file) as f:
        new = json.load(f)["default"]
    with open(deps_file) as f:
        deps = [line.strip() for line in f if line.strip()]

    changed = []
    for dep in deps:
        old_entry = old.get(dep)
        new_entry = new.get(dep)
        if old_entry is None and new_entry is None:
            continue
        if old_entry != new_entry:
            old_ver = old_entry.get("baseline", "N/A") if old_entry else "N/A"
            new_ver = new_entry.get("baseline", "N/A") if new_entry else "N/A"
            changed.append(f"{dep}: {old_ver} -> {new_ver}")

    print("\n".join(changed))


if __name__ == "__main__":
    main()
