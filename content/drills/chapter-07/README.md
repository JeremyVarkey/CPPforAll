# Chapter 7 — Scope, Duration, and Linkage · Project: Pass Telemetry

> Reinforces [`../../notes/chapter-07.md`](../../notes/chapter-07.md) · LearnCpp [Chapter 7](https://www.learncpp.com/)

## The project
You're building **Pass Telemetry** — a tiny multi-file library that a (pretend)
LLVM optimization pass uses to keep score. Each time the pass runs over a function
it wants two things: to know **how many times it has run so far** this process, and
to **hand out a fresh unique ID** for every counter it inserts. Both need state
that *survives between calls* — which is the whole reason `static` storage duration
exists. This is the exact reason real LLVM passes wrap their helpers and counters in
their own namespace with internal linkage.

The library is split the professional way: a header
([`telemetry.h`](telemetry.h)) holds the **interface** — `inline constexpr` config
constants, a `static_assert`, and the public API declarations inside
`namespace telemetry`. You implement the **bodies** in
[`starter/telemetry.cpp`](starter/telemetry.cpp): an internal-linkage helper hidden
in an **unnamed namespace**, a function whose **static local** call-counter climbs
across calls, a **static-local** unique-ID dispenser, and a function that reaches a
**shadowed** global with `::`. There are **no loops** (that's Chapter 8) — the tests
make persistence visible by *calling your functions several times in a row* and
watching the numbers go up.

## Concepts practiced
- **User namespaces** and the **scope-resolution operator** `::`, incl. nested `telemetry::config` (7.2)
- **Static local variables**: block scope but **static duration** — they *remember* across calls (7.11)
- **Internal linkage** via an **unnamed (anonymous) namespace** for file-local helpers (7.6, 7.14)
- **External linkage**: a non-const global *declared* `extern` in the header, *defined* once in a `.cpp` (7.7)
- **Variable shadowing** and reaching the global with the `::name` form (7.5)
- **`inline constexpr`** namespace constants shared across translation units, the C++17 way (7.9, 7.10)
- **`static_assert`** as a compile-time guard — a *preview* of Chapter 9 (notes 9.6); it's provided in the header, you don't write one
- *Reused from earlier chapters:* multi-file build, headers + header guards, declarations vs definitions (Ch 2);
  `std::string_view` and `constexpr` (Ch 5); the `?:`/relational feel for tiny predicates (Ch 6)

## Your tasks
The starter compiles and runs immediately — but every function returns a placeholder,
so `make test` is **RED**. Fill in the five `>>> YOUR CODE HERE <<<` blocks in
[`starter/telemetry.cpp`](starter/telemetry.cpp). They map 1:1 to the `TASK` markers:

1. **`telemetry::configuredFirstId()`** — return `config::firstCounterId`. Trivial,
   but it makes you reach into the nested config namespace with `::`.
2. **The global + `globalBuildTag()`** *(two parts, both marked `TASK 2`)* — first
   **define** the global `g_buildTag` (the header only *declared* it `extern`),
   initialized to `6340`. Then in the global-namespace function `globalBuildTag()`,
   declare a **local** named `g_buildTag` that **shadows** the global, and return the
   **global** one via `::g_buildTag`.
3. **`recordRun()` + `runCount()`** — `recordRun()` increments the shared
   `s_runCount` and returns the **new** total; because it's static-duration state, the
   value **persists** (call → 1, call → 2, call → 3). `runCount()` returns it
   **without** mutating.
4. **`nextCounterId()`** — a unique-ID dispenser built on a **static local**
   initialized to `config::firstCounterId`. Hand out `1000, 1001, 1002, …`, one per
   call, never repeating.
5. **`isFirstRunNumber()` + `isFirstRun()`** *(two parts, both marked `TASK 5`)* —
   implement the internal helper (`runNumber == 1`) and have `isFirstRun()` call it
   with `s_runCount`. Before any run is recorded it must return `false`.

The header, the `demo.cpp` consumer, and the tests are written for you — leave them
alone.

## Constraints
- **Edit only `starter/telemetry.cpp`.** Do not change `telemetry.h`, `demo.cpp`, or `tests/`.
- The internal helper and `s_runCount` **must stay inside the unnamed namespace**
  (internal linkage) — do not move them into `namespace telemetry` or make them part
  of the public API.
- `recordRun()` and `nextCounterId()` **must use static-duration state**, not a plain
  (automatic) local — otherwise the value resets every call and the persistence checks fail.
- The global `g_buildTag` must be **defined exactly once** (here), matching the
  header's `extern` declaration.
- **No loops, no `if`-as-loop tricks, no `<random>`** (Chapter 8). Observe persistence
  through the tests' repeated calls. No mutable globals beyond the one taught here.
- Stays warning-clean under `clang++ -std=c++17 -Wall -Wextra`.

## Build & run
```sh
make            # compile starter library + demo  ->  starter/app
make run        # run the demo against YOUR library (placeholders print zeros)
make test       # grade your code  (RED until you fill in the tasks)
make solution   # build + run the demo against the REFERENCE (numbers climb)
make clean
```

## Success criteria
`make test` prints **PASS ✅ all checks**. The grader links
[`tests/tests.cpp`](tests/tests.cpp) against your `starter/telemetry.cpp` and, among
other things, **calls `recordRun()` three times** expecting `1, 2, 3`, **calls
`nextCounterId()` three times** expecting `1000, 1001, 1002`, and checks the
**edge case** that *before any run* `runCount() == 0` and `isFirstRun() == false`.
Until you implement the tasks the placeholders return `0`/`false` and `make test`
shows **FAIL ❌** with the exact `CHECK` lines that failed. Turning that red into
green is the whole exercise.

A good gut-check: `make run` (your code) vs `make solution` (reference). Yours prints
all zeros; the reference prints the counter climbing `1 → 2 → 3` and IDs
`1000 → 1001 → 1002`. When your `make run` matches the reference, you're done.

## Hints
<details><summary>Task 1 — reaching a nested namespace</summary>

`config` is nested inside `telemetry`, and you're already *inside* `namespace
telemetry` when you write the body, so the constant is just `config::firstCounterId`.
(From outside it would be `telemetry::config::firstCounterId`.) The `::` operator
reads as "look inside the left-hand scope for the right-hand name."
</details>

<details><summary>Task 2 — define the global, then shadow it</summary>

Part A is one line at file scope: `int g_buildTag { 6340 };` — that's the single
**definition** the header's `extern int g_buildTag;` promised. Part B: inside
`globalBuildTag()`, `int g_buildTag { -1 };` creates a **local** that hides the
global; `return ::g_buildTag;` — the leading `::` means "the global namespace," so it
skips your local and returns `6340`.
</details>

<details><summary>Task 3 — static local vs ordinary local</summary>

`s_runCount` already lives in the unnamed namespace (static duration). `recordRun()`
is just `++s_runCount; return s_runCount;`. The key idea: because it's **static**, it
is *not* recreated each call — its value carries over. `runCount()` is `return
s_runCount;` with no `++`. (If you instead wrote `int s_runCount { 0 };` *inside*
`recordRun`, it would reset to 0 every call — try it to see the difference, then put
it back.)
</details>

<details><summary>Task 4 — a static local that remembers</summary>

```cpp
static int s_nextId { config::firstCounterId }; // runs ONCE, the first call only
return s_nextId++;                              // return current value, THEN advance
```
The `static` is what makes the initialization happen once and the value persist.
**Post**-increment (`s_nextId++`) returns the value *before* adding 1, so the first
call yields `firstCounterId`, the next `+1`, and so on.
</details>

<details><summary>Task 5 — use the internal helper</summary>

The helper is one line: `return runNumber == 1;`. Then `isFirstRun()` is `return
isFirstRunNumber(s_runCount);`. Before any `recordRun()`, `s_runCount` is `0`, so
`isFirstRunNumber(0)` is `false` — which is exactly the behavior the edge-case check
wants. Delete the `(void)…;` placeholder lines once your real code uses the names.
</details>

## Stretch goals (optional — some need later chapters)
- Add `telemetry::resetRuns()` that sets the internal counter back to 0, and a test
  that records, resets, then records again. (Notice how *hidden* the state is — only
  this `.cpp` can touch it.)
- Make `nextCounterId()` configurable with a *stride* (`config::idStride`) so IDs go
  `1000, 1010, 1020, …`. Add an `inline constexpr int idStride` to the config and a
  `static_assert(idStride > 0, …)`.
- Move the file-local helper from an unnamed namespace to a `static` free function and
  confirm `make test` still passes — then read 7.14 on why unnamed namespaces scale
  better for *groups* of helpers and for user-defined types.
- Add a second telemetry `.cpp` (a different pass) with its **own** `s_runCount` and
  prove the two counters are independent — the payoff of internal linkage. *(Wiring a
  second translation unit edges into Chapter 2 multi-file territory; fine to explore.)*
