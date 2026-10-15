#!/bin/zsh
# Apply -fix for a single clang-tidy check, scoped to only the translation
# units (TUs) recorded in clang-tidy-warnings-by-check.json as having raised
# that check — instead of re-scanning the whole codebase (what the old
# per-check dry+fix workflow did).
#
# Prerequisite: clang-tidy-dry-run-all.sh + clang-tidy-group-warnings.py have
# already been run (see clang-tidy-misc-rollout.md) and
# clang-tidy-warnings-by-check.json exists next to this script.
#
# Usage: clang-tidy-fix-check.sh <check-name> [build-dir] [json-file]
set -e

CHECK="$1"
if [[ -z "$CHECK" ]]; then
  echo "Usage: $0 <check-name> [build-dir] [json-file]" >&2
  exit 1
fi

BUILD="${2:-/Users/peter/Work/Inviwo/builds/copilot}"
JSON_FILE="${3:-$(dirname "$0")/clang-tidy-warnings-by-check.json}"
REPO_ROOT="/Users/peter/Work/Inviwo/inviwo-stage"
CLANG_TIDY_BIN=/opt/homebrew/opt/llvm/bin/clang-tidy
APPLY_REPL_BIN=/opt/homebrew/opt/llvm/bin/clang-apply-replacements

FILES=("${(@f)$(python3 -c "
import json, sys
data = json.load(open('$JSON_FILE'))
entry = data.get('$CHECK')
if not entry:
    sys.exit(0)
for f in entry['files']:
    print(f)
")}")

if [[ ${#FILES[@]} -eq 0 || -z "${FILES[1]}" ]]; then
  echo "No recorded files for check '$CHECK' in $JSON_FILE — nothing to fix."
  exit 0
fi

# Two-phase apply, same pattern run-clang-tidy itself uses: first export
# fixes per-TU in parallel (read-only, so no race even when two TUs share a
# header), then apply+merge all replacements in a single serial pass via
# clang-apply-replacements. Doing `clang-tidy -fix` directly per TU in
# parallel would be unsafe here: multiple TUs in the file list can
# legitimately share a header that raised the same check, and two parallel
# -fix invocations writing to that same header file would race.
EXPORT_DIR="$(mktemp -d /tmp/clang-tidy-fix-"$CHECK".XXXXXX)"
HELPER="$(mktemp /tmp/clang-tidy-fix-helper.XXXXXX)"
trap 'rm -rf "$EXPORT_DIR" "$HELPER"' EXIT

# A tiny on-disk helper script, rather than inlining the export-fixes command
# into the xargs -I{} template directly: macOS/BSD xargs imposes a small
# fixed buffer for -I-substituted command lines ("xargs: command line cannot
# be assembled" if exceeded), which the absolute tool/build/export paths
# blow past even for a handful of files.
cat > "$HELPER" <<EOF
#!/bin/zsh
"$CLANG_TIDY_BIN" -p "$BUILD" -checks="-*,$CHECK" \\
  -export-fixes="$EXPORT_DIR/fix-\$\$-\$RANDOM.yaml" "\$1"
EOF
chmod +x "$HELPER"

echo "Exporting fixes for $CHECK across ${#FILES[@]} TU(s)..."
printf '%s\n' "${FILES[@]/#/$REPO_ROOT/}" | \
  xargs -P8 -I{} "$HELPER" {}

echo "Applying merged replacements..."
"$APPLY_REPL_BIN" --format --style=file "$EXPORT_DIR"

echo "Done. Review with 'git diff --stat', then build + test."
