# clang-tidy rollout — status & resume notes

**Status:** `misc-*` complete. Now working through **all** enabled checks,
ordered smallest-diagnostic-count-first (see "All-checks rollout" section
below for current progress and the exact resume point). 55 checks done so
far (34 + a batch of 11 + a batch of 6 + 4 more:
`google-upgrade-googletest-case`, `performance-use-std-move`,
`readability-isolate-declaration`,
`readability-avoid-return-with-void-value`), ~46 remaining (plus 3
permanently-skipped false positives, see below).
**Branch:** `feature/more-conv-warnings`
**Last commit:** see `git log --oneline -1` on that branch.

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

## All-checks rollout (2026-10-15 night) — status

After `misc-*` finished, the workflow switched to **all** enabled checks that
have `has_fix: true` in `clang-tidy-warnings-by-check.json`, taken in order of
**ascending diagnostic count** (smallest/cheapest checks first, to clear as
many checks as possible quickly). Checks with `has_fix: false` (no autofix
ever offered anywhere in the scan — mostly `clang-analyzer-*`,
`clang-diagnostic-*`, and pure-detection checks like
`google-readability-function-size`) are skipped entirely; see
"Workflow v2" above for how `has_fix` is computed.

```sh
# Get the next N smallest not-yet-done, fixable checks:
python3 -c "
import json
d = json.load(open('tools/refactoring/clang-tidy-warnings-by-check.json'))
done = { ... }  # see DONE_CHECKS below, keep in sync
items = [(k, v['diagnostic_count'], v['tu_count']) for k, v in d.items()
         if k not in done and v.get('has_fix')]
items.sort(key=lambda x: x[1])
for k, c, t in items[:15]: print(f'{k:<55} {c:>8} {t:>6}')
"
```

### DONE_CHECKS so far (34 checks, all committed on this branch)

`misc-const-correctness`, `misc-explicit-constructor`,
`misc-unused-alias-decls`, `misc-unused-parameters`, `misc-unused-using-decls`,
`misc-use-internal-linkage`, `misc-redundant-expression` (no-op),
`misc-definitions-in-headers` (no-op), `misc-static-assert` (no-op),
`misc-uniqueptr-reset-release` (no-op) — the original 10 `misc-*` checks —
plus: `readability-delete-null-pointer`, `modernize-make-unique`,
`modernize-use-bool-literals`, `readability-container-data-pointer`,
`readability-redundant-control-flow`,
`readability-ambiguous-smartptr-reset-call`, `modernize-use-scoped-lock`,
`modernize-raw-string-literal`, `performance-string-view-conversions`,
`readability-redundant-string-cstr`, `clang-diagnostic-range-loop-construct`,
`bugprone-copy-constructor-init`, `cppcoreguidelines-macro-to-enum`,
`bugprone-optional-value-conversion`, `modernize-use-structured-binding`,
`readability-static-definition-in-anonymous-namespace`,
`modernize-use-starts-ends-with`, `modernize-deprecated-headers`,
`readability-enum-initial-value`, `performance-for-range-copy`,
`performance-unnecessary-copy-initialization`,
`bugprone-parent-virtual-call`, `readability-use-std-min-max`,
`cppcoreguidelines-virtual-class-destructor`, `readability-string-compare`,
`performance-move-const-arg`, `readability-redundant-smartptr-get`,
`readability-simplify-boolean-expr`, `modernize-avoid-bind`,
`readability-use-concise-preprocessor-directives`,
`modernize-use-string-view`, `modernize-use-emplace`,
`performance-avoid-endl`, `readability-redundant-access-specifiers`,
`readability-redundant-string-init` — plus a further batch of 6:
`readability-redundant-nested-if`, `modernize-loop-convert`,
`modernize-use-noexcept`, `modernize-use-std-numbers`,
`cppcoreguidelines-init-variables`, `readability-container-contains` —
plus 4 more: `google-upgrade-googletest-case`, `performance-use-std-move`,
`readability-isolate-declaration`,
`readability-avoid-return-with-void-value`.

**Reverted as false positives for this codebase (do NOT blanket-apply these
checks again without manual review)**:
- `google-readability-todo` — the fix stamps the local `$USER` (whatever
  account happens to be running clang-tidy) onto every pre-existing `// TODO`
  comment as `// TODO: <user> - ...`, regardless of who actually wrote that
  TODO originally. Applying it would have misattributed ~14 files' worth of
  old TODOs (some clearly years old) to whoever runs this rollout. Reverted
  in full, no commit; the check also has no way to supply a real
  bug-tracker/author mapping, so there's no safe automated way to apply it.
- `modernize-use-transparent-functors` — changed a template function
  parameter's type from `std::equal_to<Processor*>` to the transparent
  `std::equal_to<>` in `networkutils.h`'s `traverseNetwork()`, but the
  parameter type is a *fixed, non-deduced* concrete type (not inferred from
  the caller), so existing call sites that locally declare
  `std::unordered_set<..., std::equal_to<Processor*>, ...>` no longer match
  and fail to link/compile. Same root cause class as the `misc-*`
  "non-deduced context" false positives documented below.
- `bugprone-unused-raii` — in `src/core/tests/unittests/bitset-test.cpp`, a
  `TEST` deliberately constructs several `BitSet` temporaries with no name,
  purely to check they compile. The check's fix names every instance with
  the exact same placeholder (`give_me_a_name`), which doesn't even compile
  (duplicate declaration in the same scope) and defeats the test's actual
  intent. Reverted; left un-silenced (no `NOLINT` added) in case a future,
  smarter instance of this check offers a better per-occurrence fix.
- `readability-redundant-access-specifiers` has a Qt `slots`-specific
  false-positive category, hit on 4 files (not full-check revert, just
  those hunks — see the commit message for the exact list): it treats
  `"public slots:"`/`"protected slots:"` as the same accessibility as a
  plain `"public:"`/`"protected:"` (true for pure C++ access control), but
  Qt's moc specifically looks for the `slots:` keyword to decide which
  methods get registered as invocable slots in the generated meta-object —
  dropping or merging a `slots:` section changes that registration even
  though the check reports zero accessibility change. Watch for this
  pattern in any future check that manipulates access specifiers in
  `Q_OBJECT` classes.
- `modernize-use-std-numbers` has a "coincidental floating-point literal"
  false-positive category: it matches float/double literals against
  `std::numbers::*` constants by numeric *proximity*, with no understanding
  of where the literal came from. Hit this hard in
  `src/core/util/colorbrewer-generated.cpp` (a generated ColorBrewer RGB
  palette table — 8-bit-quantized channel values like 81/255 =
  0.3176470588235294 happen to sit within tolerance of `inv_pi`, `ln2`,
  `log10e`, `egamma`, `inv_sqrtpi`; 59 bogus replacements, reverted the
  whole file) and once in `src/core/util/colorconversion.cpp` (the
  empirically-derived YCbCr chroma-blue coefficient `0.565` got "fixed" to
  `std::numbers::inv_sqrtpi`, reverted that one hunk). Any check that
  matches literals by closeness-of-value rather than by where/how they're
  used can misfire on quantized or empirically-derived constants (color
  data, calibration coefficients, lookup tables) — double check every hit
  against a generated/data file or a known standards-derived coefficient
  before trusting it.

### Two notable `-fix` bugs hit while applying this batch (not false
positives — the underlying warning was correct, but clang-tidy's suggested
edit was broken or dangerous and had to be corrected manually)

1. **`cppcoreguidelines-virtual-class-destructor` dropped a class qualifier.**
   In `volumeraycasterbase.cpp`, clang-tidy turned
   `VolumeRaycasterBase::~VolumeRaycasterBase() = default;` into
   `~VolumeRaycasterBase() = default;` — not valid outside a class body.
   Manually restored the qualifier (net diff for that file is now zero; the
   real fix, removing the redundant `virtual` keyword, landed in the header).
2. **`performance-unnecessary-copy-initialization` broke a documented
   defensive copy.** In `welcomewidget.cpp::applyExpandedPaths`, a local
   `const QSet<QString> target = paths;` has an inline comment: `// defensive
   copy: collapseAll() below mutates expandedPaths_`. The parameter aliases
   the `expandedPaths_` member; the function then calls `collapseAll()`,
   which (via a connected signal/slot) mutates `expandedPaths_` while the
   code below still needs to read the *original* snapshot. Clang-tidy can't
   see that side effect and "fixed" it to `const QSet<QString>& target =
   paths;`, which would silently reintroduce the exact bug the comment
   warns about. Reverted that one hunk; kept the rest of the check's fixes
   (which were genuine copies with no such lifetime subtlety).
3. **`modernize-avoid-bind` couldn't auto-apply at all for template
   functions with multiple instantiations.** `ThreadPool::enqueue` and
   `InviwoApplication::dispatchFront` both wrap a generic `F&&` in
   `std::bind(std::forward<F>(f), std::forward<Args>(args)...)`; clang-tidy
   suggests a *different* lambda body per instantiation depending on
   whether that particular `F` returns `void` (`{ Func(); }`) or not
   (`{ return Func(); }`) — but both suggestions target the exact same
   physical source location, so `clang-apply-replacements` correctly
   detected the conflict and refused to merge, leaving the script's run a
   no-op. Fixed by hand instead: replaced the `std::bind(...)` with a lambda
   using a C++20 init-capture pack (`... args = std::forward<Args>(args)`)
   that does `return std::invoke(f, args...);` — this deduces to the right
   return type (including `void`) for every instantiation, so one body
   covers both cases.
4. **`modernize-use-string-view` broke a `std::visit` uniform-return-type
   requirement and several explicit-constructor call sites.** Changing only
   some overrides of a virtual-ish/CRTP `classIdentifier()`/`api()` method
   family to return `std::string_view` while siblings still return a
   *computed* `std::string` breaks call sites that require every
   alternative to agree on one type (`std::visit`) or that pass the result
   to an `explicit SomeType(const std::string&)` constructor (same
   non-deduced/explicit-constructor root cause documented earlier in this
   rollout). Reverted 7 of the 12 locations the autofix touched — see the
   `modernize-use-string-view` commit message for the exact list
   (`tools/meta`'s `InviwoConf`/`QtEditorConf::api()`, and 5
   `classIdentifier()` overrides in `modules/animationqt`).
5. **`performance-avoid-endl` would have silently removed a deliberate
   flush.** `FileLogger::log()` writes one entry per log message straight
   to an on-disk `std::ofstream` meant to survive a crash (this project's
   CI explicitly inspects this kind of log / attaches a debugger when the
   app fails to launch cleanly — see "CI / GitHub Actions" in
   `copilot-instructions.md`). Converting its `std::endl` to `'\n'` would
   mean the last few log lines before an abnormal exit might never reach
   disk. Reverted that one hunk (kept the check's other, purely
   stream/stdout-based conversions, including two other one-time,
   non-crash-relevant `std::endl`s in the same file's constructor).
6. **`readability-redundant-nested-if`'s `clang-apply-replacements --format`
   pass left cosmetic-but-ugly artifacts in ~3 files** when merging a
   nested if's condition into the outer one: a stray blank
   whitespace-only line left where the (now-removed) inner `{` used to be,
   and in two cases a comment that documented the *inner* if got stranded
   between the newly-merged condition and the opening brace instead of
   staying above the `if` (`fontsizepropertywidgetqt.cpp`,
   `inviwomainwindow.cpp` x3 spots, `camerawidget.cpp` x2 spots — see that
   check's commit message). Not a correctness bug (the code still compiled
   and ran correctly) but left trailing whitespace (violates
   `.editorconfig`) and misleadingly-placed comments; fixed by hand after
   noticing via `grep -n '^\+[[:space:]]*$'` on the raw diff for
   whitespace-only added lines. Worth running that same grep on any future
   check's diff that merges/restructures control flow (loop fusion,
   if/else collapsing, etc.) before committing.

### `bugprone-parent-virtual-call` — the one check so far that is a real
behavior change, not just style

Unlike every other check applied in this rollout (misc or otherwise), this
one is NOT behavior-preserving by construction — it catches code that
explicitly qualifies a virtual call with a base class *further up* the
hierarchy than the immediate parent, silently skipping whatever override the
immediate parent provides. Every instance found in this codebase turned out
to be a genuine skipped override (verified individually against the
intermediate class's header before applying — see the commit message for
the full list: `ConstantInterpolation`/`Interpolation::serialize`,
`PoolProcessor`/`Processor::invalidate`,
`CameraProperty`+`ListProperty`/`CompositeProperty`, and a Qt
`ChangeLog`/`QTextBrowser::loadResource` case). Build and unit tests are
clean, but **unit tests may not fully exercise these particular runtime
dispatch paths** — flagging this for extra scrutiny/manual testing rather
than assuming "tests passed" is sufficient here, unlike the rest of this
rollout.

### To resume the all-checks rollout

1. Confirm the branch builds/tests clean (see commands at the bottom of this
   doc).
2. Re-run the Python snippet above with the full `DONE_CHECKS` set (copy from
   this doc, or just grep `git log --oneline` for `"clang-tidy: apply"` commit
   subjects — one per check, named exactly) to get the next batch of
   smallest remaining fixable checks. As of the last commit there were
   **~46 fixable checks remaining** (out of 98 total fixable checks found by
   the dry run); next up by size:
   `cppcoreguidelines-pro-type-static-cast-downcast`,
   `readability-const-return-type`,
   `modernize-use-integer-sign-comparison`,
   `performance-inefficient-vector-operation`,
   `readability-qualified-auto`, `readability-make-member-function-const`,
   `modernize-use-equals-default`, `modernize-use-nullptr`,
   `performance-faster-string-find`, `bugprone-macro-parentheses`, ...
   (`google-readability-todo` was evaluated and skipped — see "Reverted as
   false positives" above — do not reapply it.)
3. For each check: `./tools/refactoring/clang-tidy-fix-check.sh <check>`,
   review the diff (don't skip this — two of the fixes applied so far needed
   hand-correction, see above), build, test, commit. Batch several small
   checks' fixes before doing one build+test pass if they touch disjoint
   files, to save iteration time, but still commit one check per commit.
4. Watch for the same false-positive categories documented in "Lessons
   learned" below (non-deduced template parameter types, defensive copies
   masked by comments, "transparent wrapper" implicit-conversion types,
   brace-init-list element construction, mutation inside `if constexpr`).

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
