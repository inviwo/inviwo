#!/usr/bin/env python3
"""Group clang-tidy diagnostics (from clang-tidy-dry-run-all.sh's -export-fixes
YAML output) by check name, recording which translation units (TUs) raised
each check at least once.

We deliberately avoid a YAML parser dependency (PyYAML isn't installed in
this environment) since clang-tidy's -export-fixes format is machine
generated with a very regular, predictable indentation style. A small
regex-based line scanner is sufficient and avoids needing to `pip install`
anything.

For each YAML file (= one TU):
  - "MainSourceFile:" gives the TU's source path.
  - Each top-level "  - DiagnosticName: <check>" entry (2-space indent list
    item) is one diagnostic. Diagnostics nested under "Notes:" do NOT have a
    "DiagnosticName" key, so they're naturally excluded.

We record, per check, the set of TUs that raised it at least once (that's
the file list you actually need to pass to clang-tidy to re-fix that check,
since fixing a header's warning only requires reanalyzing one TU that
includes it) plus a raw diagnostic count for reporting.

Usage:
  clang-tidy-group-warnings.py [export-dir] [output-json] [repo-root]
"""
import json
import re
import sys
from collections import defaultdict
from pathlib import Path

EXPORT_DIR = Path(sys.argv[1] if len(sys.argv) > 1 else "/tmp/clang-tidy-all-fixes")
OUTPUT_JSON = Path(
    sys.argv[2]
    if len(sys.argv) > 2
    else "/Users/peter/Work/Inviwo/inviwo-stage/tools/refactoring/clang-tidy-warnings-by-check.json"
)
REPO_ROOT = Path(sys.argv[3] if len(sys.argv) > 3 else "/Users/peter/Work/Inviwo/inviwo-stage")

MAIN_SOURCE_RE = re.compile(r"^MainSourceFile:\s*'?([^'\n]*)'?\s*$")
DIAG_NAME_RE = re.compile(r"^ {2}- DiagnosticName:\s*(\S+)\s*$")


def to_relative(path: str) -> str:
    try:
        return str(Path(path).resolve().relative_to(REPO_ROOT))
    except ValueError:
        return path


def is_in_scope(relative_path: str) -> bool:
    # ext/ (vendored third-party code) is intentionally out of scope even
    # though older FILES_RE regexes let it leak in; filter defensively here
    # too so re-grouping doesn't require re-running the expensive scan.
    parts = Path(relative_path).parts
    return "ext" not in parts


def parse_yaml_file(path: Path):
    """Return (main_source_file, list_of_check_names) for one export-fixes YAML."""
    main_source = None
    checks = []
    with open(path, "r", encoding="utf-8", errors="replace") as f:
        for line in f:
            if main_source is None:
                m = MAIN_SOURCE_RE.match(line)
                if m:
                    main_source = m.group(1)
                    continue
            m = DIAG_NAME_RE.match(line)
            if m:
                checks.append(m.group(1))
    return main_source, checks


def main():
    yaml_files = sorted(EXPORT_DIR.glob("*.yaml"))
    if not yaml_files:
        print(f"No YAML files found in {EXPORT_DIR}. Run clang-tidy-dry-run-all.sh first.")
        sys.exit(1)

    # check -> set(relative TU paths)
    check_to_files: dict[str, set[str]] = defaultdict(set)
    # check -> total diagnostic count (includes duplicates across TUs, e.g. a
    # header warning reported once per including TU)
    check_to_count: dict[str, int] = defaultdict(int)
    tus_with_no_diagnostics = 0

    for yf in yaml_files:
        main_source, checks = parse_yaml_file(yf)
        if not checks:
            tus_with_no_diagnostics += 1
            continue
        rel_main = to_relative(main_source) if main_source else str(yf)
        if not is_in_scope(rel_main):
            continue
        for check in checks:
            check_to_count[check] += 1
            check_to_files[check].add(rel_main)

    result = {
        check: {
            "diagnostic_count": check_to_count[check],
            "tu_count": len(files),
            "files": sorted(files),
        }
        for check, files in check_to_files.items()
    }

    OUTPUT_JSON.parent.mkdir(parents=True, exist_ok=True)
    with open(OUTPUT_JSON, "w", encoding="utf-8") as f:
        json.dump(result, f, indent=2, sort_keys=True)
        f.write("\n")

    print(f"Parsed {len(yaml_files)} TU YAML files ({tus_with_no_diagnostics} clean).")
    print(f"Found {len(result)} distinct checks with diagnostics.")
    print(f"Wrote {OUTPUT_JSON}")
    print()
    print(f"{'check':<55} {'diagnostics':>11} {'TUs':>6}")
    for check in sorted(result):
        info = result[check]
        print(f"{check:<55} {info['diagnostic_count']:>11} {info['tu_count']:>6}")


if __name__ == "__main__":
    main()
