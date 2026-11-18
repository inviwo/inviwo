# clang-tidy rollout: lessons learned

## 1. CI / portability constraints (apply to any `modernize-use-ranges`-style rewrite)

- **`std::ranges::iota` is not implemented in AppleClang (libc++)**. Do not use it; keep
  `std::iota` (or a loop / `std::views::iota`).
- **`std::ranges::find` (and friends) return iterators that are not necessarily raw pointers on
  MSVC.** Code that assumes the result of a ranges algorithm on a contiguous container is a
  pointer (assigning to `T*`, comparing with a pointer, `auto* it = ...`) breaks there. Use
  `auto`, compare against `end()`, and use `&*it` if a pointer is needed.
- General rule: a local macOS build is not sufficient evidence for standard-library-heavy
  rewrites (`modernize-use-ranges`, `modernize-use-starts-ends-with`, `modernize-type-traits`,
  `modernize-use-std-numbers`...). Expect MSVC/libc++/libstdc++ differences and wait for CI.

## 2. Checks that were rejected or only partially applied

| Check | Outcome | Why |
|---|---|---|
| `google-readability-todo` | rejected | Stamps `$USER` on every old TODO. |
| `modernize-use-transparent-functors` | rejected | Changes a non-deduced fixed parameter type (`std::equal_to<Processor*>` -> `std::equal_to<>`), breaking callers. |
| `bugprone-unused-raii` | rejected | Intentional unnamed temporaries in a test. Fix names them all identically. |
| `cppcoreguidelines-pro-type-static-cast-downcast` | rejected | Project uses own type-tag dispatch (`Event::hash()`); `dynamic_cast` adds RTTI cost on every input event. Many casts are correct by construction. |
| `bugprone-macro-parentheses` | rejected | `IVW_ASSERT(cond, msg)` is designed so `msg` can be a chain of `<<`; parenthesizing breaks it. |
| `llvm-use-ranges` | rejected | Rewrites to `llvm::` algorithms. Use `modernize-use-ranges` instead. |
| `readability-trailing-comma` | rejected | Inserts commas in init lists, default args and call lists (compile errors), and clang-format explodes lists (+9k/-6k lines). |
| `bugprone-crtp-constructor-accessibility` | rejected | Not safe to apply here. |
| `performance-unnecessary-value-param` | `.cpp`-only subset | Header edits garbled ("reference to reference", mismatched declarations). |
| `readability-redundant-casting` | partial | Strips `static_cast<R (C::*)(A)>(&C::f)` overload selectors (Qt `connect`, pybind11 `.def`). |
| `modernize-use-constraints` | partial | `serializer.h`/`deserializer.h`: declaration and definition no longer match. |
| `readability-static-accessed-through-instance` | partial | Reverted hunks emitting `inviwo::Type::fn()`. |
| `readability-convert-member-functions-to-static` | partial | Breaks `&Class::method` pointer-to-member dispatch tables. |
| `modernize-use-string-view` | partial | Broke `std::visit` uniform return type and explicit-ctor call sites. |
| `modernize-use-std-numbers` | partial | Matches by numeric proximity; hit generated ColorBrewer table and an empirical YCbCr coefficient. |

## 3. Recurring false-positive / breakage patterns

Use these as a review checklist on any diff.

### Implicit conversions are often by design
- Wrapper types meant to convert implicitly (`Property` family, `ValueWrapper`, `StaticString`,
  `StrBuffer`, `Version`, `Document`, ports, `BitSet`, ...) get hit by
  `misc-explicit-constructor` and `misc-const-correctness`.
  A doc comment saying "implicit conversion" is a strong signal to revert.
- Brace-init/aggregate lists (`ProcessorInfo{...}`, `std::vector<Port>{{"a"},{"b"}}`) rely on
  copy-list-init, which `explicit` breaks even when direct-init still compiles.
- Iterator mutable -> const converting constructors must stay implicit.

### Signature/qualifier changes ripple to address-of-member sites
- Adding/removing `const`, `noexcept`, `static` to a method breaks every
  `&Class::method` / `static_cast<R (Class::*)(A) const>(...)` use: explicit casts for pybind11
  `.def`, Qt `connect`, and dispatch tables like `traverseNodes(node, &Converter::updateX)`.
  Grep the repo for `&ClassName::method` and `(ClassName::\*)` before trusting the fix.
- Same for declaration/definition pairs: header hunks that don't match out-of-line definitions.
  Revert header hunks, `git checkout --` the mismatching `.cpp`, rebuild with `ninja -k 0`, repeat.

### Type deduction side effects
- Non-deduced template parameters (`std::equal_to<T*>` fixed in a signature): "improving" the
  type breaks callers.
- `readability-const-return-type`: stripping top-level `const` makes `auto& x = f();` fail to
  bind to a temporary. Fix forward with `const auto&`. Grep other `auto&` call sites of changed getters.
- `misc-const-correctness` changes `decltype(v)`, which breaks `is_same` static/EXPECT checks
  (caught only by ctest) and non-const `operator()` of functors/RNGs/distributions/`utildoc::TableBuilder`.
  It also misses mutation inside `if constexpr` and non-const-iterable views (`views::zip`,
  `views::transform`, `views::concat`).
- `modernize-type-traits` can produce a self-referential `X_v = X_v<...>`. Grep for variable
  templates naming themselves.

### Brace-init and sign handling
- `modernize-return-braced-init-list` turns narrowing into hard errors
  (`std::string(ptr, GLsizei)`, `QColor(uvec)`).
- `modernize-use-integer-sign-comparison`: `(size_t)-1` sentinel checks (`mbsrtowcs` error)
  become `std::cmp_equal(len, -1)`, which is always false. Keep the cast form. Also
  `std::cmp_*` needs real integers, so property wrappers need `.get()`.

### Moving code across init-list/body boundaries
- `cppcoreguidelines-prefer-member-initializer`: verify that any *other member* referenced by
  the moved expression already has its final value at init-list time, and that nothing leaves a
  mutex-guarded region (`dispatcher.h`, `firstivwprocessor.cpp`, `inviwomainwindow.cpp` were all
  load-bearing).
- `modernize-pass-by-value`: watch for mismatched out-of-line constructor signatures.

### Semantics the check cannot see
- Documented defensive copies (`welcomewidget.cpp`
  `// defensive copy: collapseAll() below mutates expandedPaths_`) vs
  `performance-unnecessary-copy-initialization`.
- Deliberate flushes (`FileLogger` + `std::endl`) vs `performance-avoid-endl`.
- Qt `slots:` sections vs `readability-redundant-access-specifiers`. Grep for `Q_OBJECT`.
- Move-from-then-use and `std::move(std::move(x))` from `performance-*` move checks.
  Run `bugprone-use-after-move` over the changed files afterwards.
- `bugprone-reserved-identifier` renames only `#define`, not the matching `#ifndef`. Re-read include
  guard pairs.
- Clang-tidy fix bugs: `cppcoreguidelines-virtual-class-destructor` dropped a `Class::`
  qualifier from an out-of-line `= default`.

### Real behavior changes (not style)
- `bugprone-parent-virtual-call` is the one check that changes runtime behavior (it restores
  skipped intermediate overrides). Every hit was a genuine bug, but tests may not exercise these
  paths, so flag it for manual testing.
- `modernize-use-ranges`: partial hand-fixes were needed. Beware non-pointer iterators (see CI
  notes above).

### Cosmetic artifacts after merging/restructuring control flow
- `readability-redundant-nested-if` left whitespace-only lines and stranded comments. Run
  `git diff | grep -n '^\+[[:space:]]*$'` on any check that restructures control flow.

## 4. Techniques that worked well

- Grep the whole diff for a suspect pattern in one shot (`git diff | grep -B3 "^\+.*explicit operator"`)
  instead of discovering instances one build cycle at a time.
- When one class breaks, grep for its siblings (the `ValueWrapper` specializations, the
  `AxisRenderer`/`AxisRenderer3D` NOLINT pair, six port classes).
- Prefer fixing forward (hand-correct) over reverting a whole check; revert per hunk/file
  only when the fix is semantically wrong.
- Replace `std::bind` in generic templates with a lambda using an init-capture pack and
  `std::invoke`, so one body covers `void` and non-`void` returns.
- Keep a record: each commit message lists the reverted hunks and the reason. This is the most
  useful artifact for later auditing.

## 5. Lessons from the "all checks, folder by folder" pass over `core`

- **Special member functions** (`cppcoreguidelines-special-member-functions`): pick per class kind.
  Interfaces/observers: `= default`. Factories, managers, resource owners, RAII guards: `= delete`.
  Classes with custom copy semantics (properties, `clone()`-based): delegating move constructor.
  Never "delete" a move constructor on something that is returned by value elsewhere (modules
  outside this repo, e.g. dataframe/postprocessing, broke on deleted property move ctors).
  Do not add `void operator=(const T&) = delete` next to `T& operator=(const T&) = delete`
  (the "differ only in return type" error), and keep `= default` copy ctor where `clone()` needs it.
- **`performance-unnecessary-value-param`**: changing a by-value parameter to `const&` flips overload
  resolution when a sibling overload is a forwarding-reference template
  (`operator()(const PathComponent&, Args&&...)` vs `operator()(Args&&...)` -> infinite template
  recursion). Keep by-value + `NOLINT` there. Also check `static_cast<R (C::*)(Args)>(&C::f)` in the
  python bindings after any signature change.
- **`misc-const-correctness`** is wrong for: views that are not const-iterable
  (`set_union`, `set_intersection`, `concat_view`), mutable lambdas, and variables whose
  `decltype` is tested. Revert and add `NOLINT` with a reason.
- **`performance-enum-size`**: needs `<cstdint>`; generated headers (colorbrewer) must be fixed in
  the generator too. `Colormap` needs `uint16_t`. Tests that exercise the underlying type keep
  `NOLINT`.
- **`misc-use-anonymous-namespace` / `misc-use-internal-linkage`** have no fix-it; in test files wrap
  helpers, fixtures and mock classes in `namespace { }` and drop `static`. Use `NOLINTBEGIN/END`
  rather than `NOLINTNEXTLINE` when the diagnostic location is on a later line of a multi-line
  declaration (e.g. `Args&&... args` for `missing-std-forward`).
- **`misc-override-with-different-visibility`**: make the override private to match the base, if
  nobody calls it through the derived type.
- **Re-apply CI constraints after auto-fix**: `modernize-use-ranges` re-introduced
  `std::ranges::iota` and `ranges::sort` on zip iterators; both fail on AppleClang/zip proxies.
- Auto-fix runs of `-fix` sometimes skip diagnostics (e.g. repeated `ms` in one file); rerun the
  check with `CHECKS=` restricted to it before fixing by hand.
- Redirect build/tidy output to a file and grep it; a single recursive-template error can
  produce hundreds of KB of output.
