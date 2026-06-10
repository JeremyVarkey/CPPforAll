# Chapter 6 — Operators · Project: Day-One Triage Console

> Reinforces [`../../notes/chapter-06.md`](../../notes/chapter-06.md) · LearnCpp [Chapter 6](https://www.learncpp.com/)

## The project
A release just shipped and you're on call. Build/test events stream into the
**triage console**, and the console has to make snap decisions about each one:
is this build number even or odd? does this number fall in a special bucket? is
this a leap year for the date math? are two measured timings "the same"? which
ticket priority does this severity score deserve? should the instrumentation
fire on *this* instruction?

You'll write the console's brain as **nine small pure functions** — each one is
a single operator-driven decision. The twist: **you never write a loop.** The
grader hammers each function with many inputs (even numbers, negative numbers,
century years, both sides of an epsilon, off-by-one boundaries), so all you have
to get right is the **operators**. That's the entire point of Chapter 6:
precedence, `%`, `?:`, relational and logical operators, and the floating-point
comparison pitfall — the small mechanics that decide whether a predicate fires
in the right place. (In CS6340 these are exactly the predicates that decide
whether an instrumentation pass touches the right LLVM instructions.)

## Concepts practiced
- **Remainder operator `%`** and the divisibility idiom `(x % n) == 0` (6.2, 6.3)
- The **sign-follows-left-operand** rule for `%` on negatives, and fixing it (6.3)
- **Conditional operator `?:`** as an expression-level if/else, including nesting (6.6)
- **Relational operators** `< <= > >= == !=` producing `bool` (6.7)
- **Logical operators** `&& || !` and **short-circuit evaluation** as a guard (6.8)
- **Logical XOR via `!=`** on two `bool`s (6.8)
- **Floating-point comparison** with an epsilon instead of `==` (6.7)
- **Precedence & associativity** — parenthesizing for an unambiguous reader (6.1)
- No `**` in C++: **exponentiation via `std::pow`** + `static_cast` (6.3)
- Reused from earlier chapters: **functions / headers / header guard** (Ch 2),
  `bool` and **`static_cast`** and `if` (Ch 4), **`std::string_view`** (Ch 5)

## Your tasks
The starter compiles immediately but every function returns a **placeholder**, so
`make test` starts **RED**. Fill in the nine `>>> YOUR CODE HERE <<<` blocks in
[`starter/triage.cpp`](starter/triage.cpp). They ramp from warm-up to tricky:

1. **Parity.** `isEven(int)` returns `true` when the value divides by 2 evenly.
   `classifyParity(int)` returns `"even"`/`"odd"` using **one `?:` expression**.
   Test oddness with `% 2 != 0`, *not* `== 1` (negatives break the `== 1` version).
2. **FizzBuzz category.** `fizzbuzzCategory(int)` returns `"fizzbuzz"`/`"fizz"`/
   `"buzz"`/`"number"` for a *single* value. Check "divisible by both 3 and 5"
   **first** (use `&&`). No loop, no printing — the grader does the repetition.
3. **Leap year.** `isLeapYear(int)` as **one logical expression** (no `if`):
   divisible by 4 **and** (not divisible by 100 **or** divisible by 400).
4. **Safe float compare.** `approxEqual(double,double,double)` returns whether
   `std::abs(a - b) <= epsilon` — never `==` on calculated doubles.
5. **Non-negative wrap.** `wrapIndex(int,int)` folds an index into `[0, size)`,
   staying non-negative even for negative indices (raw `%` can go negative).
6. **Sampling gate.** `shouldSampleInstruction(idx, sampleEvery, budget)` fires
   only when `sampleEvery > 0` **and** `budget > 0` **and** `idx % sampleEvery == 0`.
   Put `sampleEvery > 0` first so short-circuit `&&` never lets you compute `% 0`.
7. **Priority ladder.** `ticketPriority(int)` maps a score to `"P0"`/`"P1"`/
   `"P2"`/`"P3"` with relational tests (nested `?:` or `if`/`return`).
8. **Exactly-one-of.** `exactlyOneSource(bool,bool)` returns the logical XOR —
   for two `bool`s that's just `!=`.
9. **Integer power.** `powInt(int,int)` computes `base^exponent` via `std::pow`
   (C++ has no `**`; `^` is XOR). Convert the `double` result back to `int`.

You may **not** edit [`triage.h`](triage.h) or anything in `tests/` — those are
the contract and the grader.

## Constraints
- **Allowed:** `%`, `?:`, relational (`< <= > >= == !=`), logical (`&& || !`),
  arithmetic-assignment (`+=`), `if`/`return`, `static_cast`, `std::abs`,
  `std::pow`, and the function/`string_view` machinery already in the files.
- **Forbidden (not taught yet):** any **loop** (`for`/`while` — Chapter 8),
  `switch` (Ch 8), `<random>`, containers, classes. If you reach for a loop,
  stop — the grader is the loop.
- **Idioms required by the notes:** divisibility as `(x % n) == 0`; oddness as
  `!= 0`; **parenthesize** mixed logical/relational/`?:` expressions; `sampleEvery > 0`
  **before** the `%` in Task 6; epsilon comparison (never `==`) for Task 4.
- Keep every function **pure**: read the parameters, return a value — no I/O, no
  globals, no side effects.

## Build & run
```sh
make            # compile-check your starter/triage.cpp (warning-clean)
make test       # grade your code  ->  RED until the TASK blocks are filled in
make solution   # run the grader against the reference solution
make clean       # remove build artifacts
```
(`make run` is an alias for `make test` here — for this lab, "running" your code
*is* running the grader against it, since the grader supplies `main`.)

## Success criteria
`make test` prints **PASS ✅ all triage-console checks passed.** and exits 0.
Until then it prints one `FAIL: …  @line N` per broken check (the expression and
the line in [`tests/tests.cpp`](tests/tests.cpp) it tripped on) and ends with
`FAIL ❌  N check(s) failed`. The grader exercises every function across many
inputs — including the edge cases that separate a correct operator from a
plausible-looking wrong one:
- `isEven(-7)` / `classifyParity(-7)` — the negative-odd `% 2 == 1` trap
- `fizzbuzzCategory(15)` and `(0)` — "both" must beat plain "fizz"
- `isLeapYear(1900)` vs `isLeapYear(2000)` — the century/400 rule
- `approxEqual(0.1 + 0.2, 0.3, 1e-9)` — the classic `==` failure
- `wrapIndex(-1, 3) == 2` — folding a negative remainder
- `shouldSampleInstruction(8, 0, 10)` — short-circuit must dodge `% 0`
- `ticketPriority(90)` / `(70)` / `(40)` — the band boundaries
- `powInt(2, 8) == 256` — what `2 ^ 8` would *wrongly* give you (XOR = 10)

Turning that wall of red into a single green line is the whole exercise.

## Hints
<details><summary>Task 1 — parity and the negative trap</summary>

Even: `return (value % 2) == 0;`. For the label, the conditional operator is an
expression, so you can `return` it directly:
`return ((value % 2) == 0) ? "even" : "odd";`. Don't write the odd test as
`% 2 == 1` — in C++ `-7 % 2` is `-1`, so that check is `false` for negative odds.
</details>
<details><summary>Task 2 — order the checks</summary>

The `"fizzbuzz"` case is the most specific, so test it first:
`if (((value % 3) == 0) && ((value % 5) == 0)) return "fizzbuzz";` then `"fizz"`,
then `"buzz"`, then fall through to `"number"`. If you check `"fizz"` before
`"fizzbuzz"`, 15 would wrongly return `"fizz"`.
</details>
<details><summary>Task 3 — where the parentheses go</summary>

`return ((year % 4) == 0) && (((year % 100) != 0) || ((year % 400) == 0));`
The inner `||` group is required: `&&` binds tighter than `||`, so without the
parentheses the meaning changes.
</details>
<details><summary>Task 4 — magnitude of the difference</summary>

`return std::abs(a - b) <= epsilon;` (`std::abs` from `<cmath>`, already included).
Using `<=` lets `epsilon == 0` still report identical values as equal.
</details>
<details><summary>Task 5 — fold the negative remainder</summary>

```cpp
int wrapped { index % size };
if (wrapped < 0) wrapped += size;   // -1 % 3 is -1; +3 makes it 2
return wrapped;
```
</details>
<details><summary>Task 6 — let short-circuit protect you</summary>

```cpp
return (sampleEvery > 0) && (budgetRemaining > 0) && ((instructionIndex % sampleEvery) == 0);
```
Because `&&` stops at the first `false`, when `sampleEvery` is `0` the `%` on the
right is never evaluated — so you never hit `x % 0` (undefined behavior).
</details>
<details><summary>Task 7 — read the ladder top-down</summary>

```cpp
return (score >= 90) ? "P0"
     : (score >= 70) ? "P1"
     : (score >= 40) ? "P2"
     :                 "P3";
```
The first `>=` that holds wins, so order alone resolves the bands.
</details>
<details><summary>Task 8 — XOR is just !=</summary>

`return fromFile != fromStdin;` — valid precisely because both operands are
already `bool`. (Don't use `!=` as XOR on ints/pointers.)
</details>
<details><summary>Task 9 — pow returns a double</summary>

`return static_cast<int>(std::pow(base, exponent) + 0.5);` — the `+ 0.5` rounds
to the nearest int, because `std::pow` can land just under the true value (e.g.
`124.999…` for `5^3`).
</details>

## Stretch goals (optional — most need later chapters)
- Make `powInt` exact with an integer loop instead of `std::pow`, and detect
  overflow against a wider type (needs loops, Chapter 8).
- Replace the fixed-epsilon `approxEqual` with a **relative+absolute** tolerance
  (`std::max(std::abs(a), std::abs(b)) * relEps`) — see notes 6.7.
- Add a `classifyCoverage(covered, total)` that returns `"complete"` /
  `"partial"` / `"none"` using `%`-free integer comparisons, then a
  `double` coverage ratio compared with `approxEqual(ratio, 1.0, 1e-12)`.
- Turn the console into a real CLI that reads events from `std::cin` and prints
  decisions (needs `cin` loops + validation, Chapters 8–9).
