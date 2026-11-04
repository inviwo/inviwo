#!/bin/zsh
# Run all enabled clang-tidy checks on one folder of src/core and its matching public headers.
# Usage: tidy-folder.sh <folder> [fix]     e.g. tidy-folder.sh rendering
# Writes the full log to /tmp/tidy/<folder>.log and a per-check summary to stdout.
set -e

FOLDER="$1"
MODE="${2:-dry}"

LLVM=/opt/homebrew/opt/llvm/bin
BUILD=/Users/peter/Work/Inviwo/builds/copilot
SRC=/Users/peter/Work/Inviwo/inviwo-stage
LOG=/tmp/tidy/$FOLDER.log
mkdir -p /tmp/tidy

FIXFLAGS=()
if [[ "$MODE" == "fix" ]]; then
  FIXFLAGS=(-fix -format -style=file)
fi

"$LLVM/run-clang-tidy" -p "$BUILD" \
  -clang-tidy-binary "$LLVM/clang-tidy" \
  -clang-apply-replacements-binary "$LLVM/clang-apply-replacements" \
  -quiet -j8 \
  -header-filter="$SRC/include/inviwo/core/$FOLDER/.*" \
  "${FIXFLAGS[@]}" \
  "$SRC/src/core/$FOLDER/.*\.cpp" > "$LOG" 2>&1 || true

# Unique diagnostics located in this folder's sources/headers, grouped by check.
grep -E "^$SRC/(src|include/inviwo)/core/$FOLDER/.*(warning|error):" "$LOG" | sort -u > "/tmp/tidy/$FOLDER.diag"
echo "total unique diagnostics: $(wc -l < /tmp/tidy/$FOLDER.diag)"
sed -E 's/.*\[([^]]+)\]$/\1/' "/tmp/tidy/$FOLDER.diag" | sort | uniq -c | sort -rn
