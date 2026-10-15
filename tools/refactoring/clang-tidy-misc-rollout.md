# clang-tidy `misc-*` rollout — status & resume notes

**Status:** `misc-*` complete (2026-10-15). Tooling (Workflow v2, below) is
reusable as-is for the next check group.
**Branch:** `feature/more-conv-warnings`
**Last commit:** `839d148e52` ("clang-tidy: apply misc-use-internal-linkage fixes")

## Workflow v2 (2026-10-15 evening) — batch dry-run + scoped fixes

The original per-check workflow (`/tmp/tidy-check.sh`, see "v1" section below)
re-scanned the **entire** codebase once for the dry run and again for the fix,
for **every single check**. That's wasteful: clang-tidy's cost is dominated by
parsing/building the AST per translation unit (TU), not by how many checks are
enabled while doing so. The new workflow pays that AST cost **once** for *all*
enabled checks, then only re-touches the handful of files that actually need a
given fix.

Committed scripts (in this directory):

- [clang-tidy-dry-run-all.sh](tools/refactoring/clang-tidy-dry-run-all.sh) —
  runs `run-clang-tidy` **once**, with no `-checks` override (so it uses the
  full enabled set from `.clang-tidy`), across every in-scope TU, writing one
  `-export-fixes` YAML file per TU into `/tmp/clang-tidy-all-fixes/` (YAML
  records every diagnostic, fixable or not, with its check name and file —
  verified this includes diagnostics whose only replacement lives on a nested
  `Notes` entry, e.g. `readability-inconsistent-declaration-parameter-name`).
  Takes a while (≈1490 TUs in scope) but is a **one-time** cost per refresh,
  not per-check.
- [clang-tidy-group-warnings.py](tools/refactoring/clang-tidy-group-warnings.py)
  — parses all those YAML files (no PyYAML dependency; the export format's
  indentation is regular enough for a line-based regex scanner) and writes
  `clang-tidy-warnings-by-check.json`: for every check that fired, the set of
  TU (main source) files that raised it at least once, plus raw diagnostic
  counts. Also prints a summary table to stdout.
- [clang-tidy-fix-check.sh](tools/refactoring/clang-tidy-fix-check.sh)
  `<check-name>` — looks up that check's file list in the JSON and applies
  `-fix` scoped to **only** those TUs. Internally this is still a two-phase
  export-then-apply (per-TU `-export-fixes` in parallel, then a single serial
  `clang-apply-replacements --format --style=file` pass) rather than a direct
  parallel `clang-tidy -fix`, because two TUs in the same check's file list
  can legitimately share an included header; applying fixes to that header
  from two parallel `-fix` processes at once would race. `clang-apply-replacements`
  merges/dedupes across files safely in one pass, matching what `run-clang-tidy
  -fix` does internally.

### Refreshing the dry-run data

```sh
# One-time (or whenever a meaningful number of files have changed since the
# last run): re-scan everything, for every enabled check at once.
./tools/refactoring/clang-tidy-dry-run-all.sh
# -> /tmp/clang-tidy-all-fixes/*.yaml, /tmp/clang-tidy-all-dry.log

./tools/refactoring/clang-tidy-group-warnings.py
# -> tools/refactoring/clang-tidy-warnings-by-check.json (committed; see below)
```

`clang-tidy-warnings-by-check.json` is committed next to this doc so the
current inventory (counts, affected files) is visible in review/diffs without
needing to re-run the scan. Re-run and re-commit it whenever it goes stale
(e.g. after a batch of fixes, or after enough unrelated code churn).

**Known staleness as of 2026-10-15:** checks 8-10 (`misc-unused-parameters`,
`misc-unused-using-decls`, `misc-use-internal-linkage`) were fixed after the
JSON was generated, touching 61 files total — those entries (and any other
check's file list that happens to include those same 61 files) are now
slightly stale. Not worth a full re-scan for 61/1490 files; just keep this in
mind, or re-run the scan before starting a fresh group if you want a clean
baseline.

### Applying a fix for one check

```sh
./tools/refactoring/clang-tidy-fix-check.sh misc-unused-parameters
# then the same build/test/commit steps as before (step 3 onward in the
# "Per-check workflow" section)
```

### Scope note

The user is ultimately interested in **all** enabled `.clang-tidy` checks, not
just `misc-*` — the full enabled set (see `.clang-tidy`) spans `bugprone-*`,
`cppcoreguidelines-*`, `google-*`, `llvm-*`, `misc-*`, `modernize-*`,
`performance-*`, `portability-*`, `readability-*`, and `clang-analyzer-*`.
Since the dry-run data collected above already covers every enabled check (not
just `misc-*`), `clang-tidy-warnings-by-check.json` can be used as-is to start
any other group later — no need to re-scan. We're still doing `misc-*` first,
in the same order as before, before moving to the other groups.

## Goal

Go through every auto-fixable check in the `misc-*` group of `.clang-tidy`
(the checks that have an "offers fixes" entry upstream), one at a time:

1. Dry-run the check across the codebase to see how many warnings it finds.
2. Apply `-fix -format`.
3. Build, and if anything breaks, decide per-instance whether the fix was
   correct (fix forward) or a false positive from the checker (revert that
   specific hunk, not the whole check).
4. Run the test suite (compiling cleanly is not sufficient — see lessons
   learned below, some breakage is silent/behavioral, not a compile error).
5. Commit everything for that one check with a message describing what was
   applied and, importantly, *every* false positive that was reverted and why.
6. Move to the next check.

Order (alphabetical, as originally given to the agent):
`const-correctness`, `definitions-in-headers`, `explicit-constructor`,
`redundant-expression`, `static-assert`, `uniqueptr-reset-release`,
`unused-alias-decls`, `unused-parameters`, `unused-using-decls`,
`use-internal-linkage`.

Scope: **inviwo-stage only** (core + built-in modules/apps). The sibling
`modules/` workspace folder (external modules: misc, medvis, molvis,
tensorvis, topovis, vectorvis, infravis) is explicitly **out of scope** for
this rollout, per the user's choice when this task started.

## Status

| # | Check | Status | Notes |
|---|-------|--------|-------|
| 1 | `misc-const-correctness` | ✅ Done | Commit `f6b3da3e03`. 932 files. Huge number of false positives reverted — see below. User also manually extended this to `ext/ticpp` in commit `59c0f17a6c` (outside agent scope, FYI only). |
| 2 | `misc-definitions-in-headers` | ✅ Done (no-op) | 0 applicable warnings in scope (all diagnostics were in vcpkg/system headers, filtered out by header-filter). No commit. |
| 3 | `misc-explicit-constructor` | ✅ Done | Commit `56faf930c7`. 302 files. Also huge — see below. |
| 4 | `misc-redundant-expression` | ✅ Done (no-op) | 98 warnings, all `py::self == py::self` pybind11 operator-registration boilerplate (intentional, not a bug). No fixes offered by clang-tidy for this sub-pattern. No commit. |
| 5 | `misc-static-assert` | ✅ Done (no-op) | 0 warnings. No commit. |
| 6 | `misc-uniqueptr-reset-release` | ✅ Done (no-op) | 0 warnings. No commit. |
| 7 | `misc-unused-alias-decls` | ✅ Done | Commit `ef0e4b205c`. Only 2 warnings, both genuine, trivial fix, no reverts needed. |
| 8 | `misc-unused-parameters` | ✅ Done | Commit `28cb91d514`. 5 warnings / 3 TUs. 2 auto-fixed by clang-tidy; 1 case (`eventtransformer.cpp`) applied manually because clang-tidy declined with "overlaps with another fix" (confirmed genuinely unused — the function body is entirely commented out behind a `TODO`). |
| 9 | `misc-unused-using-decls` | ✅ Done | Commit `00570d379b`. 1 warning / 1 TU, trivial. |
| 10 | `misc-use-internal-linkage` | ✅ Done | Commit `839d148e52`. 251 warnings / 89 TUs, 57 files actually changed. All changes are `static` on free functions/variables in `.cpp` files; no headers touched. Build is fully link-clean across every target, which is strong evidence no cross-TU usage was missed. |

**The `misc-*` rollout (checks 1-10) is complete as of commit `839d148e52`.** Next: pick any other enabled group (`bugprone-*`, `cppcoreguidelines-*`, `google-*`, `llvm-*`, `modernize-*`, `performance-*`, `portability-*`, `readability-*`, `clang-analyzer-*`) using the same Workflow v2 tooling — see "To resume" below.

## One-time environment setup already done

- The local (gitignored, not committed) `CMakeUserPresets.json` was edited so
  that the `copilot` preset uses the **Ninja** generator instead of Xcode,
  and no longer sets `IVW_EXTERNAL_MODULES` (dropping the external
  `modules/` workspace folder from this build, per the "inviwo-stage only"
  scope decision). This is required to get `compile_commands.json`, which the
  Xcode generator cannot produce. **If you start a fresh machine/clone, you
  need to redo this edit** — see `/memories/repo/build-notes.md` in this
  workspace for the exact preset diff description, or just re-derive it from
  the `ninja-developer` style presets already in the tracked
  `CMakePresets.json`, minus the modules.
- Build dir: `/Users/peter/Work/Inviwo/builds/copilot` (now Ninja,
  `RelWithDebInfo`, single-config — no more `-C Debug`/Xcode quirks).
- `compile_commands.json` lives at the root of that build dir.
- A reusable helper script exists at `/tmp/tidy-check.sh` (**not** committed,
  it's outside the repo and only on this machine — recreate it if missing,
  contents below) that wraps `run-clang-tidy` with the right scope filters.
  **Superseded by Workflow v2 above** (`clang-tidy-fix-check.sh`) — kept here
  only because the "Lessons learned" section below still refers to it and
  because it's a handy one-off for ad-hoc dry runs of a single check that
  isn't in the committed JSON yet.

### `/tmp/tidy-check.sh` (recreate if missing) — v1, historical

```sh
#!/bin/zsh
# Usage: tidy-check.sh <clang-tidy-check-name> <dry|fix>
set -e

CHECK="$1"
MODE="$2"

RUN_CLANG_TIDY=/opt/homebrew/Cellar/llvm/23.1.2/bin/run-clang-tidy
CLANG_TIDY_BIN=/opt/homebrew/opt/llvm/bin/clang-tidy
APPLY_REPL_BIN=/opt/homebrew/opt/llvm/bin/clang-apply-replacements
BUILD=/Users/peter/Work/Inviwo/builds/copilot

FILES_RE='^/Users/peter/Work/Inviwo/inviwo-stage/(?!.*/ext/).*\.(cpp|cc|mm)$'
HEADER_FILTER='.*inviwo-stage.*\.(h|hpp)$'
EXCLUDE_HEADER='.*/ext/.*|.*vcpkg_installed.*|.*/builds/.*'

FIXFLAGS=()
if [[ "$MODE" == "fix" ]]; then
  FIXFLAGS=(-fix -format -style=file)
fi

"$RUN_CLANG_TIDY" -p "$BUILD" \
  -clang-tidy-binary "$CLANG_TIDY_BIN" \
  -clang-apply-replacements-binary "$APPLY_REPL_BIN" \
  -checks="-*,$CHECK" \
  -header-filter="$HEADER_FILTER" \
  -exclude-header-filter="$EXCLUDE_HEADER" \
  -j 8 \
  "${FIXFLAGS[@]}" \
  "$FILES_RE"
```

Note: `-checks="-*,$CHECK"` overrides whatever is in `.clang-tidy` for this
one invocation, so each run only exercises exactly one check, independent of
what's normally enabled project-wide.

## Per-check workflow (exact commands)

**Current (v2):**

```sh
# 1. Gauge scope from the already-collected inventory (no clang-tidy run
#    needed — see Workflow v2 above for how/when to refresh it).
python3 -c "import json; d=json.load(open('tools/refactoring/clang-tidy-warnings-by-check.json')); print(d.get('misc-<check-name>'))"

# 2. Apply fix + format, scoped to only the files that need it.
./tools/refactoring/clang-tidy-fix-check.sh misc-<check-name>

# 3. See what changed
cd /Users/peter/Work/Inviwo/inviwo-stage
git diff --stat | tail -5

# 4. Build (incremental — only recompiles what the fix touched)
ninja -C /Users/peter/Work/Inviwo/builds/copilot -j8 2>&1 | tee /tmp/ninja-<check>-build1.log | tail -3
# if it fails:
grep -B8 "error:" /tmp/ninja-<check>-build1.log | head -80
# find the root cause type/class, decide fix-forward vs revert (see lessons
# learned below for the common false-positive categories), edit, then:
ninja -C /Users/peter/Work/Inviwo/builds/copilot -j8 2>&1 | tee /tmp/ninja-<check>-build2.log | tail -3
# repeat until clean (BUILD_EXIT_0 and 0 "error:" lines in the log)

# 5. Run tests — must be 100% green, not just "it compiles"
ctest --test-dir /Users/peter/Work/Inviwo/builds/copilot -j4 2>&1 | tail -20

# 6. Commit (scope to tracked source dirs only; don't touch build/vcpkg dirs)
git add -A -- apps include modules src tests tools
git commit -F - <<'EOF'
clang-tidy: apply misc-<check-name> fixes

<describe what the check does, how many files/warnings, and in detail every
false-positive category that was reverted and why — this is the most useful
part of the commit message for future readers/auditors>

Verified: full incremental Ninja build is clean and `ctest` is 100% green
(17/17 suites).
EOF

# 7. Move to the next check.
```

IMPORTANT gotcha with the terminal tool used in this session: `cd X && ninja
...` in async mode sometimes had the `cd` silently stripped, running the
command in the wrong directory. Workaround: always use
`ninja -C /abs/path/builds/copilot -j8 ...` instead of `cd`-ing first.

## Lessons learned / common false-positive categories

These are the recurring reasons a clang-tidy-suggested fix had to be
reverted. Keep an eye out for all of these in the remaining checks too.

1. **NOLINT comment aliasing bug.** Several places already had
   `// NOLINT(google-explicit-constructor)` (or similar) comments
   *intentionally* suppressing a fix. But `google-explicit-constructor` is
   only an **alias** for `misc-explicit-constructor` (see
   `clang-tidy --list-checks` / the upstream alias table), and
   when `run-clang-tidy` is invoked with `-checks=-*,misc-explicit-constructor`
   (the canonical name), the alias-named NOLINT comment does **not** suppress
   it. Found ~10 instances of this for `explicit-constructor`; worth
   double-checking for any other check that has an alias (`google-*`,
   `llvm-*`, `cert-*` names map onto `misc-*`/`modernize-*`/etc. — see the
   check-aliases table on the upstream clang-tidy docs page).
2. **"Transparent wrapper" types designed for implicit conversion.** This
   codebase has many value-wrapper classes (`Property` family via
   `TemplateProperty`/`OrdinalProperty`/`OptionProperty`, `ValueWrapper<T>`,
   `StaticString`, `StrBuffer`, `Tags`/`Tag`, `Version`, `Document`,
   `ProcessorStatus`, port classes `DataInport`/`DataOutport`/...,
   `BitSet`, ffmpeg's `CodecID`/`OutputFormat`, etc.) whose entire point is to
   be constructed from / converted to their underlying value *implicitly*.
   `misc-const-correctness` and `misc-explicit-constructor` both have no way
   to know this is intentional and will "fix" them anyway. Symptom: compile
   errors like "no viable conversion" / "chosen constructor is explicit in
   copy-initialization" at call sites far away from the header that was
   edited.
3. **Brace-init-list / aggregate init relies on implicit per-element
   construction.** `SomeVector{ {"a"}, {"b"} }` or
   `ProcessorInfo{ cid, name, "Category", CodeState::Stable, "Tags", "help"_help }`
   are copy-list-initialization per element; marking the element type's
   constructor `explicit` breaks these even though a single
   `T x{args};` direct-init elsewhere in the same file would still compile
   fine. This bit `ProcessorInfo`, `IsoValueCollection`,
   `MeshShaderCache::Requirement`, `PropagationParameters`, `TypedMesh`,
   `InviwoModuleFactoryObjectTemplate` (via `Version`), and the port
   constructors (used inside `std::vector<Port>{ {"a"}, {"b"} }`).
4. **Iterator "mutable → const" converting constructors.** Standard container
   idiom (`iterator` implicitly converts to `const_iterator`). Hit this with
   `IndirectIterator`, `TransformIterator` (both the core one and the
   standalone copy in `tools/meta`), and the internal `proxy`/`zipIterator`/
   `zipper` types in `include/inviwo/core/util/zip.h`.
5. **Mutation inside `if constexpr` branches not detected by
   `const-correctness`'s mutation analyzer.** E.g. `delta.x -= 1.0f;` inside
   `if constexpr (...)` on a variable the tool just marked `const`. Found in
   several spots (`volumevoronoi.cpp`, `pixelvalue.cpp`, Qt widgets, etc.).
6. **Functors / distributions / random engines whose `operator()` is
   non-const.** `std::uniform_real_distribution`, `std::mt19937`, and this
   codebase's own `utildoc::TableBuilder` all have a non-const `operator()`;
   `const-correctness` doesn't realize calling `foo(...)` later requires
   `foo` to stay non-const.
7. **C++20 range views that aren't const-iterable.** `std::views::zip`,
   `std::views::transform`, and this project's own `views::concat`,
   `views::set_intersection`/`set_union` views don't support `begin()`/`end()`
   on a `const`-qualified instance (when passed to another range adaptor like
   `views::concat(matched, rest)`, or iterated twice in a test). Only const
   the result if it's been materialized already (e.g. piped through
   `| std::ranges::to<std::vector>()`).
8. **Silent (non-compile-error) breakage via `decltype()`/type traits.**
   `misc-const-correctness` broke `src/core/tests/unittests/glm-test.cpp` by
   making a local `const`, which changed `decltype(v)` from
   `Vector<ROWS,int>` to `const Vector<ROWS,int>`, failing a
   `std::is_same<...>` static check used in a `EXPECT_TRUE`. The build was
   still green; only `ctest` caught this (9 `GLMTest.MatrixSizeTest*`
   failures). **Always run the full test suite, not just the build, before
   committing a check.**

## How to find the exact revert sites quickly (techniques that worked well)

- `git diff | grep -B3 "^\+.*explicit operator"` / `"^\+.*const auto"` etc. to
  list every instance of a suspicious pattern across the whole diff in one
  shot, rather than discovering them one error-cycle at a time.
- Look for a doc-comment on the line above a conversion operator that
  literally says "implicit conversion" — very strong signal it needs
  reverting (found this exact wording above `StrBuffer::operator
  std::string_view()`).
- When one class/pattern breaks once, grep for the same class name or
  pattern everywhere else in the diff before rebuilding again — several
  checks had the "same bug repeated in 3-4 sibling classes" shape (e.g. the
  `ValueWrapper` specialization for `TransferFunction`, the
  `AxisRenderer`/`AxisRenderer3D` NOLINT pair, the six port classes).

## Build/test verification commands (copy-paste ready)

```sh
# Full incremental build
ninja -C /Users/peter/Work/Inviwo/builds/copilot -j8

# Full test suite (17 suites as of this writing)
ctest --test-dir /Users/peter/Work/Inviwo/builds/copilot -j4 --output-on-failure
```

## To resume

1. Confirm `builds/copilot` still configures/builds (it should, nothing
   should have changed since the last green build at commit `cab52b663e`).
2. Check whether `tools/refactoring/clang-tidy-warnings-by-check.json` is
   still fresh (diff against current `git log` churn since it was last
   generated); if stale, re-run `clang-tidy-dry-run-all.sh` +
   `clang-tidy-group-warnings.py` (see Workflow v2 above) and recommit the
   JSON.
3. Continue with check 8, `misc-unused-parameters`, using
   `clang-tidy-fix-check.sh misc-unused-parameters` then the usual
   build/test/commit steps. Then 9 (`misc-unused-using-decls`) and 10
   (`misc-use-internal-linkage`, flagged above as the riskiest remaining one).
4. Once all `misc-*` checks are done, this same process/workflow applies
   as-is to the other enabled groups (`bugprone-*`, `cppcoreguidelines-*`,
   `google-*`, `llvm-*`, `modernize-*`, `performance-*`, `portability-*`,
   `readability-*`, `clang-analyzer-*`) \u2014 the dry-run data already covers
   them, so just pick a check from `clang-tidy-warnings-by-check.json` and
   run `clang-tidy-fix-check.sh <check-name>`.
