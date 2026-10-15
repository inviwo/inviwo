#!/bin/zsh
# One-time (well, per-refresh) full-codebase clang-tidy dry run that collects
# diagnostics for *every* check enabled in .clang-tidy in a single pass.
#
# Rationale: clang-tidy's cost is dominated by parsing/building the AST for
# each translation unit, not by how many checks are enabled while doing so.
# Running once with the full check set and recording which TUs raised which
# checks (via -export-fixes) is far cheaper than re-scanning the whole
# codebase once per check, which is what the original per-check workflow did.
#
# Output:
#   - $EXPORT_DIR: one YAML file per translation unit (clang-tidy
#     -export-fixes format), containing every diagnostic (fixable or not)
#     with its check name and location.
#   - $LOG_FILE: raw textual run-clang-tidy output, useful for a quick
#     human-readable skim / grep, but the YAML dir is the source of truth
#     for clang-tidy-group-warnings.py.
#
# After this completes, run clang-tidy-group-warnings.py to build
# clang-tidy-warnings-by-check.json, then use clang-tidy-fix-check.sh
# <check-name> to apply fixes only to the files that actually need them.
#
# Usage: clang-tidy-dry-run-all.sh [build-dir] [export-dir] [log-file]
set -e

BUILD="${1:-/Users/peter/Work/Inviwo/builds/copilot}"
EXPORT_DIR="${2:-/tmp/clang-tidy-all-fixes}"
LOG_FILE="${3:-/tmp/clang-tidy-all-dry.log}"

RUN_CLANG_TIDY=/opt/homebrew/Cellar/llvm/23.1.2/bin/run-clang-tidy
CLANG_TIDY_BIN=/opt/homebrew/opt/llvm/bin/clang-tidy
APPLY_REPL_BIN=/opt/homebrew/opt/llvm/bin/clang-apply-replacements

# Scope: inviwo-stage only (core + built-in modules/apps), excluding ext/.
# The sibling modules/ workspace folder (external modules) is out of scope.
FILES_RE='^/Users/peter/Work/Inviwo/inviwo-stage/(?!(.*/)?ext/).*\.(cpp|cc|mm)$'
HEADER_FILTER='.*inviwo-stage.*\.(h|hpp)$'
EXCLUDE_HEADER='.*/ext/.*|.*vcpkg_installed.*|.*/builds/.*'

mkdir -p "$EXPORT_DIR"
rm -f "$EXPORT_DIR"/*.yaml(N)

# No -checks override here on purpose: we want whatever is enabled in the
# repo's .clang-tidy (the full default set), not a single check.
"$RUN_CLANG_TIDY" -p "$BUILD" \
  -clang-tidy-binary "$CLANG_TIDY_BIN" \
  -clang-apply-replacements-binary "$APPLY_REPL_BIN" \
  -header-filter="$HEADER_FILTER" \
  -exclude-header-filter="$EXCLUDE_HEADER" \
  -export-fixes="$EXPORT_DIR" \
  -j 8 \
  "$FILES_RE" > "$LOG_FILE" 2>&1

echo "Dry run complete. YAML files: $EXPORT_DIR  Log: $LOG_FILE"
