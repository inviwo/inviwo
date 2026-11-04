#!/usr/bin/env python3
"""Build a markdown report of all clang-tidy diagnostics collected by tidy-folder.sh.

Usage: tidy-report.py [out.md]   (reads /tmp/tidy/*.diag)
"""
import re
import sys
from collections import defaultdict
from pathlib import Path

ROOT = "/Users/peter/Work/Inviwo/inviwo-stage/"
LINE = re.compile(r"^(?P<file>[^:]+):(?P<line>\d+):(?P<col>\d+): (?P<kind>warning|error): "
                  r"(?P<msg>.*) \[(?P<check>[^\]]+)\]$")

by_check = defaultdict(list)
folders = {}
for diag in sorted(Path("/tmp/tidy").glob("*.diag")):
    for raw in diag.read_text().splitlines():
        m = LINE.match(raw)
        if not m:
            continue
        f = m["file"].removeprefix(ROOT)
        by_check[m["check"]].append((diag.stem, f, int(m["line"]), m["msg"]))
        folders.setdefault(m["check"], defaultdict(int))[diag.stem] += 1

out = ["# clang-tidy diagnostics in src/core and include/inviwo/core\n"]
total = sum(len(v) for v in by_check.values())
out.append(f"Total unique diagnostics: **{total}** across {len(by_check)} checks.\n")
out.append("## Summary\n")
out.append("| Check | Count | Ruling |")
out.append("|---|---:|---|")
for check, items in sorted(by_check.items(), key=lambda kv: -len(kv[1])):
    out.append(f"| `{check}` | {len(items)} | |")

out.append("\n## Details\n")
for check, items in sorted(by_check.items(), key=lambda kv: -len(kv[1])):
    out.append(f"### `{check}` ({len(items)})\n")
    out.append("Per folder: " + ", ".join(f"{k} {v}" for k, v in sorted(folders[check].items())))
    out.append("\nRuling: \n")
    for folder, f, line, msg in sorted(items, key=lambda x: (x[1], x[2])):
        short = msg if len(msg) < 140 else msg[:137] + "..."
        out.append(f"- `{f}:{line}` {short}")
    out.append("")

Path(sys.argv[1] if len(sys.argv) > 1 else "/tmp/tidy/core-warnings.md").write_text("\n".join(out))
