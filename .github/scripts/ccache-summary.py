#!/usr/bin/env python3
"""Aggregate per-job ccache summary-artifacts into a single markdown report.

Each matrix build job uploads a `ccache-summary.json` file (via the
petersteneteg/ccache-action `summary-artifact` input) as its own uniquely
named artifact. This script walks a directory containing all of those
artifacts, downloaded one subfolder per artifact (the default behaviour of
`actions/download-artifact` without `merge-multiple`), and prints a combined
markdown table suitable for the GitHub Actions job summary.
"""

from __future__ import annotations

import glob
import json
import os
import re
import sys

ARTIFACT_PREFIX = "inviwo-ccache-summary-"
# Artifact names get a random UUIDv4 suffix appended by ccache-action to keep them unique.
UUID_SUFFIX = re.compile(
    r"-[0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12}$"
)


def job_name(artifact_dir: str) -> str:
    name = os.path.basename(artifact_dir.rstrip("/"))
    name = UUID_SUFFIX.sub("", name)
    if name.startswith(ARTIFACT_PREFIX):
        name = name[len(ARTIFACT_PREFIX) :]
    return name


def load_summaries(root: str) -> list[tuple[str, dict]]:
    summaries = []
    for path in sorted(glob.glob(os.path.join(root, "**", "ccache-summary.json"), recursive=True)):
        with open(path, encoding="utf-8") as f:
            data = json.load(f)
        summaries.append((job_name(os.path.dirname(path)), data))
    return summaries


def format_row(name: str, data: dict) -> tuple[str, int, int]:
    stats = data.get("stats", {})
    hits = stats.get("direct_cache_hit", 0) + stats.get("preprocessed_cache_hit", 0)
    misses = stats.get("cache_miss", 0)
    total = hits + misses
    rate = f"{(hits / total * 100):.1f}%" if total else "n/a"
    row = (
        f"| {name} | {data.get('restored', 'n/a')} | {hits} / {total} | {rate} "
        f"| {data.get('evicted', 0)} | {data.get('saved', 'n/a')} |"
    )
    return row, hits, total


def main() -> int:
    root = sys.argv[1] if len(sys.argv) > 1 else "."
    summaries = load_summaries(root)

    if not summaries:
        print("No ccache summary artifacts were found.")
        return 0

    print("| Job | Restored | Hits / Total | Hit Rate | Evicted | Saved |")
    print("|---|---|---|---|---|---|")

    total_hits = 0
    total_calls = 0
    for name, data in summaries:
        row, hits, total = format_row(name, data)
        print(row)
        total_hits += hits
        total_calls += total

    overall_rate = f"{(total_hits / total_calls * 100):.1f}%" if total_calls else "n/a"
    print(f"| **Total** | | **{total_hits} / {total_calls}** | **{overall_rate}** | | |")
    return 0


if __name__ == "__main__":
    sys.exit(main())
