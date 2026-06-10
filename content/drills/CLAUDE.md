# Exercises — Authoring Guide

> **Read this before creating or editing any exercise in `drills/`.** It is the
> authoritative spec for how a CS6340 C++/LLVM-foundations exercise is built.
> The canonical, fully-worked example is [`chapter-01/`](chapter-01/) — when in
> doubt, mirror its structure, tone, and Makefile.

## What these are
Course-lab-style coding exercises that turn each [LearnCpp](https://www.learncpp.com/)
chapter into a small, motivating **project** the learner completes by filling in
marked code blocks, then checks with an **automated grader**. Notes live in
[`../notes/`](../notes/); each exercise reinforces the matching chapter and is
verified non-buggy before it ships.

## Naming & location
- **One exercise per chapter:** `drills/chapter-NN/` (zero-padded, mirrors `../notes/chapter-NN.md`).
- `NN` is the LearnCpp chapter number, exactly.

## Folder layout (every exercise)
```
chapter-NN/
  README.md          # the lab handout — see "Handout structure"
  Makefile           # standard targets — see "Makefile contract"
  starter/           # what the LEARNER edits: scaffolding + TODO blocks
  solution/          # reference implementation: complete, correct, commented
  tests/             # the automated grader (output check OR unit tests)
```

## The learner's loop (design every exercise for this)
1. Read `README.md`.
2. `make` → the **starter compiles out of the box** (stubs return placeholders).
3. `make test` → grader runs against the starter → **RED** (fails, shows what's missing).
4. Fill in the `>>> YOUR CODE HERE <<<` blocks.
5. `make test` → **GREEN**.
6. Stuck? `make solution` runs the reference; peek at `solution/`.

**This red→green property is REQUIRED:** the starter must compile, tests must fail
on it, and the solution must make them pass.

## TODO block convention (in `starter/` files)
```cpp
// ─── TASK n: <short title> ─────────────────────────────────────────
// <what to implement>. <constraints>. <hint, optional>.
//
//   >>> YOUR CODE HERE <<<
//
// ───────────────────────────────────────────────────────────────────
```
- Each `TASK n` maps 1:1 to a numbered task in the README.
- **Stub the body so it still compiles** (`return {};`, `return 0;`, a `{}`-init,
  or a placeholder print). No unused-variable warnings if avoidable.

## Makefile contract (same targets in every exercise; use TABS, not spaces)
| Target | Does |
|---|---|
| `make` / `make build` | compile the **starter** |
| `make run` | run the starter program |
| `make test` | grade the **learner's** starter code (their red→green loop) |
| `make solution` | build + run the **reference** solution |
| `make test-solution` | grade the **solution** — **MUST be green** (our proof it's solvable) |
| `make clean` | remove build artifacts |

Compiler: `clang++ -std=c++17 -Wall -Wextra`. Binaries are gitignored (`../.gitignore`).

## Two grader styles (pick what fits the chapter)

### Style A — output comparison (good for "everything in `main`" chapters, e.g. Ch 1)
`tests/input.txt` is fed on stdin; output is diffed against `tests/expected.txt`.
See [`chapter-01/Makefile`](chapter-01/Makefile). Choose inputs that produce
**exact, unambiguous output** (e.g. whole numbers — avoid float-formatting traps).

### Style B2 — header-only unit tests (for class/template chapters)
Some chapters' code must live in a header (class templates, Ch 26) or reads most
naturally with member bodies in-class (intro classes, Ch 14). There the learner
edits `starter/<name>.h` (the full class/template, bodies inline) and the grader
switches include paths instead of linked `.cpp`s:
```
test:          ; $(CXX) $(CXXFLAGS) -Istarter  tests/tests.cpp -o tests/run && ./tests/run
test-solution: ; $(CXX) $(CXXFLAGS) -Isolution tests/tests.cpp -o tests/run && ./tests/run
```
`tests/tests.cpp` does `#include "<name>.h"` and picks up starter/ or solution/
via `-I`. Everything else (TASK blocks, red→green, README) is unchanged.

### Style B — unit tests (preferred once functions exist, Ch 2+)
Put the learner-facing API in a header the learner *implements*:
```
chapter-NN/
  <name>.h            # DECLARATIONS only (provided, complete)
  starter/<name>.cpp  # bodies with TODO blocks (learner fills in)
  solution/<name>.cpp # reference bodies
  tests/tests.cpp     # includes ../<name>.h, calls the API, CHECKs results
```
Tiny no-framework harness:
```cpp
#include <iostream>
#include "../<name>.h"
static int fails = 0;
#define CHECK(cond) do { if(!(cond)){ std::cerr<<"FAIL: "#cond" @line "<<__LINE__<<"\n"; ++fails; } } while(0)
int main() {
    CHECK(area(3, 4) == 12);
    // ...more, INCLUDING edge cases...
    if (!fails) std::cout << "PASS ✅ all checks\n";
    return fails ? 1 : 0;
}
```
Makefile links the tests against starter vs solution:
```
test:          ; $(CXX) $(CXXFLAGS) tests/tests.cpp starter/<name>.cpp  -o tests/run && ./tests/run
test-solution: ; $(CXX) $(CXXFLAGS) tests/tests.cpp solution/<name>.cpp -o tests/run && ./tests/run
```

## Handout structure (`README.md` sections, in this order)
1. `# Chapter NN — <Title>: <Project Name>` + links to `../../notes/chapter-NN.md` and the LearnCpp chapter.
2. **The project** — 1–2 paragraphs: what you build and why it teaches this chapter.
3. **Concepts practiced** — bullets; **include reused concepts from earlier chapters**.
4. **Your tasks** — numbered, mapping to the `TASK` blocks; each: what + constraints.
5. **Constraints** — explicit allowed/forbidden constructs and required idioms.
6. **Build & run** — the `make` commands.
7. **Success criteria** — exactly how the learner knows they passed.
8. **Hints** — progressive, in `<details>` blocks.
9. **Stretch goals** — optional extensions (may name later-chapter concepts).

## Quality bar (NON-NEGOTIABLE — this is why we verify)
- Reference solution **compiles warning-clean** (`-Wall -Wextra`) and **passes all tests**.
- Starter **compiles** out of the box; tests **fail** on it (red→green).
- Tests are **meaningful**: cover the concept and at least one **edge case**; never `assert(true)`.
- **No undefined behavior** anywhere in the solution.
- **Scope discipline:** an exercise for chapter NN uses **only concepts from chapters ≤ NN**.
  Reusing earlier concepts is encouraged; pulling in *not-yet-taught* concepts is a bug.
- **Previews & honest borrows:** a *small* ahead-of-scope borrow is allowed only when it
  serves the chapter's central lesson, is **provided** scaffolding (not learner-authored)
  where possible, and is **labeled in place** with its real chapter — e.g.
  `// (a preview — formally Chapter 17)`. **Never cite a note section that doesn't
  cover the claim** — check `../notes/chapter-NN.md` headings before writing `(notes N.M)`.
- **Teaching voice:** heavy, explanatory comments with **key terms emphasized**, matching
  `../notes/` and the existing drills. The learner should *learn from reading the scaffolding.*

## Pedagogy (expert-tutor principles)
- **One central concept** per exercise; supporting concepts serve it.
- Make the concept **physical** — the learner should *observe* behavior (output, a failing
  assert, an address, an overflow), not just read about it.
- Prefer a **motivating project** (a tool, a game, a mini-library) over an abstract drill.
- **Ramp difficulty within the exercise:** first task easy (confidence), last task stretches.
- Connect to the **CS6340/LLVM "why"** when natural (linkage→passes, classes→`Instruction` hierarchy).

## Verification checklist (run before declaring an exercise done)
- [ ] `make test-solution` → all green
- [ ] `make` (starter) → compiles
- [ ] `make test` (starter) → fails as intended
- [ ] solution is `-Wall -Wextra` clean
- [ ] README has all 9 sections; tasks map 1:1 to `TASK` blocks
- [ ] only chapter-≤NN concepts used
- [ ] update the **Index** table below

## Scope map (chapters 1–28) — what each may use
Everything from earlier rows is always available; a row lists what is NEW.
| Ch | Central concept(s) | New tools available |
|----|--------------------|---------------------|
| 1 | variables, init `{}`, `cin`/`cout`, operators, expressions | `int`, `<iostream>`, `+ - * /` |
| 2 | functions, params/return, multi-file, headers, header guards, namespaces | user functions, `.h`/`.cpp` split |
| 3 | debugging: syntax vs semantic errors, print-debugging, the debugger | (a "fix the planted bugs" exercise) |
| 4 | fundamental types: int widths, `sizeof`, float/double, `bool`, `char`, overflow, `static_cast`, `if` (4.10) | `double`, `bool`, `char`, fixed-width ints, `if` |
| 5 | `const`/`constexpr`, literals, `std::string`, `std::string_view` | strings, const-correctness |
| 6 | operators in depth: `%`, `?:`, relational, logical, float comparison | `%`, `&& || !`, `?:` (still **no loops**) |
| 7 | scope, namespaces, local/global, **linkage**, `static` duration, `inline` | unnamed namespace, `static`, `inline`, `static_assert` |
| 8 | control flow: `if`/`switch`, `while`/`for`, `break`/`continue`, random | loops, branches, `<random>` |
| 9 | error handling: validation, `assert`, bad-`cin` recovery, testing | `assert`, `cin.fail()`/`clear()`/`ignore()` |
| 10 | type conversion, narrowing, `static_cast`, type aliases (`using`), `auto` | `using` aliases, `auto` |
| 11 | function overloading & resolution, deleted functions, default args, **function templates**, non-type template params | overloads, `template<typename T>` functions, `= delete`, default arguments |
| 12 | compound types I: lvalue refs, const refs, pass-by-ref, **pointers**, `nullptr`, pointers+const, pass-by-address, return by ref/address, in/out params, `std::optional` | `&`, `*`, `->` n/a yet, `nullptr`, `std::optional` |
| 13 | compound types II: unscoped/scoped **enums**, **structs** (aggregates), member selection, passing/returning structs, struct templates & CTAD | `enum class`, `struct`, aggregate init, `.` |
| 14 | intro to **classes**: member functions, const objects/members, access specifiers, constructors + member-init lists, default/copy ctors, converting ctors & `explicit` | `class`, `public:`/`private:`, ctors, `explicit` |
| 15 | more classes: `this`, class-in-header (.h/.cpp member split), nested types, intro destructors, **static members/functions**, friend | `this`, `ClassName::member` out-of-class defs, `static` members, `friend` |
| 16 | **`std::vector`**: construction, `[]` vs `.at`, size & signedness, passing/returning, range-`for`, resize/reserve, `push_back` stack ops | `std::vector`, range-`for` (16.8), `.size()`, `push_back` |
| 17 | fixed-size arrays: **`std::array`** + C-style arrays, `sizeof`, **decay**, pointer arithmetic, multidimensional arrays | `std::array`, C-arrays, pointer arithmetic |
| 18 | **iterators & algorithms**: begin/end, `std::find`/`find_if`/`count_if`/`sort`/`max_element`, function-pointer predicates (NO lambdas yet — Ch 20) | iterators, `<algorithm>` |
| 19 | **dynamic allocation**: `new`/`delete`, `new[]`/`delete[]`, dangling pointers, leaks, null-out after delete (NO smart pointers — Ch 22) | `new`, `delete`, `new[]`, `delete[]` |
| 20 | functions II: **recursion**, function pointers, stack vs heap, CLI args, **lambdas** + captures, `std::function` | recursion, `int(*)(int)`, lambdas, `std::function` |
| 21 | **operator overloading**: arithmetic, I/O `<<`/`>>`, comparison, unary, `++`/`--`, subscript; member vs friend | `operator+`, `operator<<`, `operator==`, … |
| 22 | **move semantics & smart pointers**: rvalue refs, move ctor/assign, `std::move`, `std::unique_ptr`, `std::shared_ptr` (beware C++17 mandatory elision in tests) | `&&`, `std::move`, `std::unique_ptr`, `std::shared_ptr` |
| 23 | **object relationships**: composition, aggregation, association, dependency, container classes, `std::initializer_list` | `std::initializer_list` |
| 24 | **inheritance** (non-virtual): base/derived, construction order, `protected`, calling/overriding base fns, hiding — static binding's limits set up Ch 25 | `: public Base`, `protected`, `Base::fn()` calls |
| 25 | **virtual functions**: `override`/`final`, pure virtual & abstract bases, interfaces, **virtual destructors**, slicing, `dynamic_cast` — the LLVM `Instruction`-hierarchy chapter | `virtual`, `override`, `dynamic_cast` |
| 26 | **class templates**: member fn templates, out-of-class member syntax, full specialization, non-type params (header-only — use Style B2) | `template<class T> class` |
| 27 | **exceptions**: `throw`/`try`/`catch`, stack unwinding, `std::exception` hierarchy, custom exception classes, rethrow, `noexcept` | `try`/`catch`, custom exceptions, `noexcept` |
| 28 | **I/O**: istream/ostream state, **`std::stringstream`**, formatting & `<iomanip>`, file I/O (`<fstream>`) | `std::stringstream`, `<iomanip>`, `std::fstream` |

## Parallel generation rule
When exercises are built by multiple agents at once, **agents edit only their own
`chapter-NN/` folder — never this file**. The orchestrator updates the Index after
verification. (A previous run hit a lost-update race on the Index table.)

## Adding the next chapter, or regenerating one
1. Read this file + `../notes/chapter-NN.md` (and the LearnCpp chapter if needed).
2. Copy `chapter-01/`'s shape. Design the **best** project for the chapter's central concept
   (see Pedagogy). Reuse earlier concepts freely; never exceed chapter NN.
3. Implement `README.md`, `Makefile`, `starter/`, `solution/`, `tests/`.
4. Run the **Verification checklist**. Do not ship until `make test-solution` is green and
   `make test` is red on the starter.
5. Update the Index.

## Index of exercises
| Ch | Project | Central concept | Grader | Status |
|----|---------|-----------------|--------|--------|
| 01 | Cash Register | variables, I/O, arithmetic expressions | output | ✅ done (template) |
| 02 | `geo` mini-library | functions / multi-file / headers / namespace | unit | ✅ done |
| 03 | Coin-Tray Auditor | debugging (fix six planted bugs) | unit | ✅ done |
| 04 | Numeric Types Lab | fundamental data types | unit | ✅ done |
| 05 | Name-Badge Formatter | const & strings | unit | ✅ done |
| 06 | Day-One Triage Console | operators | unit | ✅ done |
| 07 | Pass Telemetry | scope, duration, linkage | unit | ✅ done |
| 08 | Number-Guessing Engine | control flow | unit | ✅ done |
| 09 | Robust Input Desk | error handling (validation, assert, cin recovery) | unit | ✅ done |
| 10 | `statkit` grade-book | type conversion / aliases / `auto` | unit | ✅ done |
| 11 | The Generic Toolkit | overloading & function templates | unit (B2) | ✅ done |
| 12 | Alias Workshop | references & pointers | unit | ✅ done |
| 13 | Inventory Ledger | enums & structs | unit | ✅ done |
| 14 | The Fraction Class | intro to classes | unit (B2) | ✅ done |
| 15 | IdCard Badge Printer | more on classes (`this`, statics, dtors) | unit | ✅ done |
| 16 | Sensor-Readings Toolkit | `std::vector` | unit | ✅ done |
| 17 | Tic-Tac-Toe Referee | fixed-size arrays, decay, ptr arithmetic | unit | ✅ done |
| 18 | VecTools ("two ways") | iterators & algorithms | unit | ✅ done |
| 19 | DynBuffer Workbench | dynamic allocation (`new[]`/`delete[]`) | unit | ✅ done |
| 20 | calc-core | recursion, function pointers, lambdas | unit | ✅ done |
| 21 | Fraction v2 | operator overloading (resumes Ch 14) | unit | ✅ done |
| 22 | TrackedBuffer | move semantics & smart pointers | unit | ✅ done |
| 23 | Garage Simulation | object relationships (lifetime traces) | unit | ✅ done |
| 24 | Report-Logger Family | inheritance (static-binding cliffhanger) | unit | ✅ done |
| 25 | MINI-LLVM `Inst` hierarchy | virtual functions, `dynamic_cast` | unit | ✅ done |
| 26 | `Stack<T>` | class templates & specialization | unit (B2) | ✅ done |
| 27 | SafeConfig Parser | exceptions | unit | ✅ done |
| 28 | The Report Engine | streams, `<iomanip>`, file I/O | unit | ✅ done |
