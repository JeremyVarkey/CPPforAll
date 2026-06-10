# Chapter 10 — Type Conversion, Aliases & Deduction · Project: statkit

> Reinforces [`../../notes/chapter-10.md`](../../notes/chapter-10.md) · LearnCpp [Chapter 10](https://www.learncpp.com/)

## The project
You're building **`statkit`**, the number-crunching core of a grade book. It's a
small library of pure functions — `mean`, `percentage`, `roundToInt`, `roundTo`,
`clampScore`, `letterGrade`, `safeLength`, `weightedMean` — the kind of math a
teacher runs a hundred times a term. Tiny as it is, almost every function is a
landmine for the one mistake Chapter 10 exists to cure: **a conversion the
compiler performs silently is not always the conversion you meant.**

The headline bug is **integer-division truncation**. You *know* the mean of a
7-point total over 2 students is 3.5 — so when `mean(7, 2)` returns **3**, the
error isn't abstract, it's wrong arithmetic you can see. The fix is never "add a
cast somewhere"; it's putting a **`static_cast` at exactly the right spot** so the
division happens in `double`, not `int`. Other functions drill the rest of the
chapter: documenting an *intentional* float→int narrowing, taming the **unsigned
`size_t`** that `.length()` hands you, leaning on the **usual arithmetic
conversions** when an `int` meets a `double`, and using **`using` aliases** and
**`auto`** to keep the code readable. (In CS6340, these are the exact judgment
calls you make every time an LLVM API hands you an `unsigned`, a `size_t`, or a
line number and you have to decide what type the expression should really become.)

## Concepts practiced
- **Integer vs floating-point division** and the truncation trap (`7 / 2 == 3`) (10.1, 10.3)
- **`static_cast`** to force a real-valued division and to **document a narrowing** (10.4, 10.6)
- **Numeric conversions** & data-loss categories — float→int, **unsigned→signed** (10.3)
- **Narrowing** awareness: when truncation is intentional, say so with a cast (10.4)
- The **usual arithmetic conversions** when `int` and `double` mix under an operator (10.5)
- **Integral promotion** of `char` to `int` during arithmetic — and keeping a `char` a `char` (10.2)
- **Type aliases** with `using` (`Score`, `Percent`, `Average`) for readable, one-place-maintainable types (10.7)
- **`auto`** for a local whose type is obvious from its initializer (10.8)
- Reused from earlier chapters: **functions / headers / header guard** (Ch 2),
  `double` / `bool` / `char` / `if` (Ch 4), `constexpr` & **`std::string_view`** (Ch 5),
  the **conditional `?:`**, relational/logical operators, and **epsilon float comparison** (Ch 6)

## Your tasks
The starter compiles immediately, but every function returns a **placeholder** —
and several deliberately reproduce the *classic bug* (e.g. `mean` does integer
division and `safeLength` returns 0), so `make test` starts **RED** and the
failures point straight at the conversion you must fix. Fill in the eight
`>>> YOUR CODE HERE <<<` blocks in [`starter/statkit.cpp`](starter/statkit.cpp).
They ramp from the core trap to the subtler conversions:

1. **`mean(total, count)`** → `Average`. Return `total / count` as a real average.
   `mean(7, 2)` must be **3.5**. Convert one operand to `double` **before** the
   divide — casting the result is too late, the truncation already happened.
2. **`percentage(part, whole)`** → `Percent`. `percentage(1, 4)` must be **25.0**.
   Same trap, and watch the order: do the division in floating point, then scale
   by `100.0`.
3. **`roundToInt(value)`** → `int`. Round to the nearest int, **half away from
   zero** (`2.5`→3, `-2.5`→-3), then cast. `static_cast<int>` alone *truncates*; the
   cast also documents the intentional float→int narrowing.
4. **`roundTo(value, places)`** → `double`. Round to `places` decimals via
   scale → round (reuse Task 3) → unscale. Use **`auto`** for the scale-factor
   local. `roundTo(3.14159, 2)` is **3.14**.
5. **`clampScore(raw)`** → `Score`. Clamp into `[0, 100]` (below→0.0, above→100.0).
   The `Score` alias is just `double`, so the fractional part survives.
6. **`letterGrade(score)`** → `char`. `>=90`→`'A'`, `>=80`→`'B'`, `>=70`→`'C'`,
   `>=60`→`'D'`, else `'F'`. Boundaries go to the higher grade. Return a `char`
   literal — the return type being `char` (not `int`) is the point.
7. **`safeLength(text)`** → `int`. `.length()` returns an **unsigned** size type;
   return it as a signed `int` with a `static_cast` that **documents** the
   signed/unsigned conversion. `safeLength("")` is 0.
8. **`weightedMean(p0,w0, p1,w1, p2,w2)`** → `Average`. Compute
   `(p0*w0 + p1*w1 + p2*w2) / (w0+w1+w2)`. The `int*double` products convert via
   the **usual arithmetic conversions** — so here you need **no cast at all**.

You may **not** edit [`statkit.h`](statkit.h) or anything in `tests/` — those are
the contract and the grader.

## Constraints
- **Allowed:** `static_cast`, the `using` aliases already declared, `auto` (Task 4),
  the conditional `?:`, `if`/`return`, relational/arithmetic operators, `char`
  literals, and the `std::string_view` / function machinery already in the files.
- **Required idioms (from the notes):**
  - **`static_cast` BEFORE the divide** in Tasks 1 & 2 (not on the result).
  - **`static_cast`** to document the float→int narrowing in Task 3 and the
    unsigned→signed conversion in Task 7.
  - **`auto`** for the scale-factor local in Task 4 (obvious type from initializer).
  - **No cast** in Task 8 — adding one would lie about a narrowing that isn't there.
- **Forbidden (not taught yet or against the lesson):** any **loop** (`for`/`while` —
  Chapter 8; the grader is the loop), **C-style casts** like `(int)x` (notes 10.6 —
  use `static_cast`), `<cmath>`'s `std::round` (do the rounding by hand so you *see*
  the conversion), containers, classes. Scattering casts just to silence warnings is
  also out — a cast is a promise; make it true.
- Keep every function **pure**: read the parameters, return a value — no I/O, no
  globals, no side effects.

## Build & run
```sh
make            # compile-check your starter/statkit.cpp (warning-clean)
make test       # grade your code  ->  RED until the TASK blocks are filled in
make solution   # run the grader against the reference solution
make clean      # remove build artifacts
```
(`make run` is an alias for `make test` here — for this lab, "running" your code
*is* running the grader against it, since the grader supplies `main`.)

## Success criteria
`make test` prints **PASS ✅ all statkit checks passed.** and exits 0. Until then
it prints one `FAIL: …  @line N` per broken check (the expression and the line in
[`tests/tests.cpp`](tests/tests.cpp) it tripped on) and ends with
`FAIL ❌  N check(s) failed`. Calculated doubles are compared with an **epsilon**
helper (`close(a, b)`), never `==` — exactly as the notes insist. The grader hits
every function across many inputs, including the edge cases that separate a correct
conversion from a plausible-looking wrong one:
- `mean(7, 2) == 3.5` and `mean(-7, 2) == -3.5` — the integer-division trap, signed too
- `percentage(1, 4) == 25.0` and `percentage(1, 8) == 12.5` — fractional percents survive
- `roundToInt(2.5) == 3` and `roundToInt(-2.5) == -3` — rounds half **away from zero**, not truncate
- `roundTo(2.675, 1) == 2.7`, `roundTo(-1.2345, 2) == -1.23` — decimal rounding, negatives
- `clampScore(150.0) == 100.0`, `clampScore(99.9) == 99.9` — clamp without losing the fraction
- `letterGrade(90.0) == 'A'`, `letterGrade(60.0) == 'D'` — the band boundaries
- `safeLength("ab") - safeLength("abcd") == -2` — the subtraction that would **wrap** if left unsigned
- `weightedMean(90,0.2, 80,0.3, 100,0.5) == 92.0` — `int*double` via the usual arithmetic conversions

Turning that wall of red into one green line is the whole exercise.

## Hints
<details><summary>Task 1 — convert before you divide</summary>

```cpp
return static_cast<double>(total) / count;
```
Once the left operand is `double`, the usual arithmetic conversions promote
`count` to `double` too, so the whole division is real-valued. Writing
`static_cast<double>(total / count)` is the classic mistake — the `int / int`
truncates *first*, then you cast 3 to 3.0.
</details>
<details><summary>Task 2 — do the division in floating point, then scale</summary>

```cpp
return static_cast<double>(part) / whole * 100.0;
```
`(part / whole)` as ints is 0 for `1/4`, and `0 * 100` is still 0. Cast `part`
so the division is real before you multiply by `100.0`.
</details>
<details><summary>Task 3 — nudge by 0.5, in the value's direction</summary>

```cpp
return (value >= 0.0) ? static_cast<int>(value + 0.5)
                      : static_cast<int>(value - 0.5);
```
`static_cast<int>` truncates toward zero, so add `0.5` for positives and subtract
`0.5` for negatives to round half away from zero (`2.5`→3, `-2.5`→-3).
</details>
<details><summary>Task 4 — auto for the obvious local; build the factor without a loop</summary>

```cpp
const double powers[] { 1.0, 10.0, 100.0, 1000.0, 10000.0, 100000.0, 1000000.0 };
auto factor { powers[places] };          // auto deduces double
return roundToInt(value * factor) / factor;
```
`auto` is the right call here — the initializer makes the type obvious. Dividing
the `int` from `roundToInt` by the `double` `factor` converts it back to `double`.
(The `powers[]` table is a C-style array — formally Chapter 17 — fine to copy as
given; or build the factor by multiplying `10.0` the right number of times.)
</details>
<details><summary>Task 5 — a nested conditional</summary>

```cpp
return (raw < 0.0) ? 0.0 : (raw > 100.0) ? 100.0 : raw;
```
The `Score` return type is just `double`, so returning `raw` keeps its fractional
part intact — aliases rename a type, they don't narrow it.
</details>
<details><summary>Task 6 — a char ladder</summary>

```cpp
return (score >= 90.0) ? 'A'
     : (score >= 80.0) ? 'B'
     : (score >= 70.0) ? 'C'
     : (score >= 60.0) ? 'D'
     :                   'F';
```
Each test is strictly above the next, so order resolves the boundaries (exactly
90 → `'A'`). You return a `char` literal directly — no arithmetic, so nothing
promotes it to `int`.
</details>
<details><summary>Task 7 — one documented cast</summary>

```cpp
return static_cast<int>(text.length());
```
`.length()` is unsigned; the `static_cast` is your promise that the string is
short enough to fit in `int`, and it silences the signed/unsigned warning an
implicit conversion would raise.
</details>
<details><summary>Task 8 — let the conversions work for you (no cast)</summary>

```cpp
return (p0 * w0 + p1 * w1 + p2 * w2) / (w0 + w1 + w2);
```
In every `int * double` product the `int` is converted to `double` automatically,
so numerator and denominator are both `double`. Adding a `static_cast` here would
falsely imply a narrowing that never happens.
</details>

## Stretch goals (optional — some need later chapters)
- Make the aliases **type-safe**: an alias is *not* a distinct type, so `Percent`
  and `Score` are interchangeable today. Wrap one in a tiny `struct`/`enum class`
  so the compiler rejects mixing them (needs classes/enums, Chapters 13–14).
- Replace the `powers[]` table in `roundTo` with a `constexpr` helper, and have it
  reject `places` outside `0..6` with `assert` (assert is Chapter 9).
- Add `meanOf(const std::vector<int>& xs)` that sums with a loop and divides as a
  `double` average — the same truncation trap, now over a real container (loops
  Ch 8, vectors Ch 16).
- Add a `formatPercent(Percent)` returning a 1-decimal `std::string` and watch the
  float-formatting issues the notes warn about (`std::setprecision`, Chapter 28).
- CS6340 tie-in: write `auto *p { … }`-style code where a pretend API returns a
  pointer, and a `using CoverageSet = std::set<std::pair<int,int>>;` alias to see
  how aliases tame noisy LLVM types (notes 10.7 / 10.8).
