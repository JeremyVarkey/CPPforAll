# Chapter 8 — Control Flow · Project: The Number-Guessing Engine

> Reinforces [`../../notes/chapter-08.md`](../../notes/chapter-08.md) · LearnCpp [Chapter 8](https://www.learncpp.com/)

## The project
The number-guessing game is this chapter's own capstone — so let's build it the
way a real codebase would: split the **engine** (the pure decision logic) from
the **randomness and I/O** (the dice roll and the keyboard). You'll write the
engine as **six small pure functions** — judge a guess, label the hint, sum some
digits, build a countdown, validate input, and play a whole bounded round — each
one driven by a different Chapter-8 control-flow tool. A ready-made `main.cpp`
then wraps your engine with a Mersenne-Twister RNG and `std::cin`/`std::cout` so
you can actually **play** (`make run`).

Why the split? Because `<random>` and `std::cin` make code **non-deterministic**
— you can't unit-test "did the engine roll a 7?". So we keep the randomness and
the keyboard in `main` (ungraded) and keep the logic in a deterministic engine
the grader can pin to exact answers. That's the whole lesson, and it's exactly
how you'd make a fuzzer's decision logic testable in CS6340: the mutation/branch
choices are unit-tested; the random target launch is not.

## Concepts practiced
- **`if` / `else if` / `else` chains** for mutually-exclusive cases (8.2)
- **`switch`** with `case` labels, `default`, and `break` (no fallthrough) (8.5, 8.6)
- **`while` loops** — counting, and *building up* a result across iterations (8.8)
- **`do while` loops** — the run-at-least-once shape for "read, then validate" (8.9)
- **`for` loops** for bounded iteration, with **`break`** (stop early) and
  **`continue`** (skip this one) (8.10, 8.11)
- The **modulo / divide digit-peel** idiom `n % 10`, `n /= 10` (8.8, reuses `%` from 6.2)
- Seeing **`<random>`** in action — engine + distribution + **seed once** (8.13, 8.14)
  — in `main` only, deliberately *outside* the graded, deterministic engine
- Reused from earlier chapters: **functions / headers / header guard** (Ch 2),
  `int` & **`static_cast`** & `if` (Ch 4), **`std::string` / `std::string_view`**
  and indexing a string (Ch 5)

## Your tasks
The starter compiles immediately but every function returns a **placeholder**, so
`make test` starts **RED**. Fill in the six `>>> YOUR CODE HERE <<<` blocks in
[`starter/engine.cpp`](starter/engine.cpp). They ramp from warm-up to capstone:

1. **Judge a guess (`if`/`else` chain).** `judgeGuess(secret, guess)` returns a
   three-way *direction code* — `-1` too low, `+1` too high, `0` correct — using
   one `if` / `else if` / `else` chain. The **sign** is the signal the player
   reads to know which way to move. No loops.
2. **Label the hint (`switch`).** `hintText(code)` maps `-1 → "too low"`,
   `0 → "correct"`, `+1 → "too high"`, and anything else `→ "invalid"` via the
   `default` case. Use a **`switch`** (not if/else here — practicing `switch` *is*
   the task), with a `break` after each case so you don't fall through.
3. **Digit sum (loop + `%`/`/`).** `sumOfDigits(n)` adds the decimal digits with a
   `while`/`for` loop: `n % 10` grabs the last digit, `n /= 10` drops it. Use the
   **magnitude**, so `sumOfDigits(-123) == sumOfDigits(123)`. `sumOfDigits(0) == 0`.
4. **Countdown (`while` that builds a string).** `countdownString(start)` returns
   `"3..2..1..liftoff"` for `3`, counting **down** with a `while` loop. The `".."`
   separator goes **only between** counts (none before the first, none after the
   last number); a `start <= 0` runs the body zero times and yields just
   `"liftoff"`. Return the string — don't print it.
5. **Validate input (`do while`).** `promptsUntilValid(typed, low, high)` is a
   *deterministic* stand-in for a re-prompt loop: the "typed" values are handed in
   as a string of digits. Walk them with a `do { … } while (…)` loop and return
   the **1-based count** of values consumed until one lands in `[low, high]` (or
   the total, if none do). A `do while` fits because you must read **at least one**
   value before you can test it.
6. **Play a round (`for` + `break` + `continue`).** `playRound(secret, guesses,
   maxAttempts)` plays one bounded round; each char of `guesses` is one numeric
   guess. Loop with `for`. A guess of **`0` is a "misclick" sentinel** — `continue`
   without counting it. Otherwise it's a counted attempt; if it's correct (judge
   it!), `break` and return the **1-based counted attempt** that won; once the
   `maxAttempts` budget is spent, stop. Return `0` for a loss. This pulls the
   whole chapter together.

You may **not** edit [`engine.h`](engine.h) or anything in `tests/` — those are
the contract and the grader. You don't need to touch `main.cpp` either (it's the
ungraded driver), though you're welcome to read it to see the RNG wiring.

## Constraints
- **Allowed:** `if`/`else`, `switch`/`case`/`default`/`break`, `while`,
  `do while`, `for`, `break`/`continue`, `%` and `/`, `static_cast`,
  `std::string` (`+=`, `std::to_string`), `std::string_view` indexing
  (`sv[i]`, `sv.size()`), and the function/header machinery already in the files.
- **Forbidden (not taught yet):** `<random>` **inside the engine** (it belongs in
  `main`, and would make your logic non-deterministic), `std::cin`/`std::cout`
  inside the engine, arrays / `std::vector` / pointers (Ch 12+), `enum` (Ch 13),
  `goto` (8.7 — you have `break`/`continue`/`return`; use them). The engine must
  stay **pure**: arguments in, value out, no globals, no side effects.
- **Idioms required by the notes:** give every loop a believable **exit story**
  (8.8); prefer **half-open** counting (`< size`, not `<= size-1`) (8.10); use
  `break` to prevent `switch` fallthrough (8.5); a `do while` only where the body
  must run **once before** the test (8.9).

## Build & run
```sh
make            # compile-check starter/engine.cpp AND build the playable driver
make run        # PLAY the game against your engine (uses <random> + your logic)
make test       # grade your engine  ->  RED until the TASK blocks are filled in
make solution   # play the game against the reference engine
make clean      # remove build artifacts
```
`make test` is the grader (it supplies its own `main` and calls your engine
across many fixed inputs). `make run` is the *game* — fun once your engine works,
but the **grade** comes from `make test`.

## Success criteria
`make test` prints **PASS ✅ all engine checks passed.** and exits 0. Until then
it prints one `FAIL: …  @line N` per broken check (the expression and the line in
[`tests/tests.cpp`](tests/tests.cpp) it tripped on) and ends with
`FAIL ❌  N check(s) failed`. The grader exercises every function across many
inputs — including the edge cases that separate correct control flow from a
plausible-looking near-miss:
- `judgeGuess(-5, -9)` / `(-5, -1)` — the chain must order **negatives** correctly
- `hintText(2)` and `hintText(-2)` — the **`default`** case must catch out-of-range codes
- `sumOfDigits(-123) == 6` — must use the **magnitude**, not a negative remainder
- `sumOfDigits(2147483647) == 46` — the digit loop on **`INT_MAX`** (no overflow)
- `countdownString(0) == "liftoff"` — the `while` body must run **zero** times
- `countdownString(3) == "3..2..1..liftoff"` — separators **only between** counts
- `promptsUntilValid("7", 1, 6) == 1` — `do while` still consumes the **one** value
- `promptsUntilValid("999", 1, 6) == 3` — none valid → return the **total** consumed
- `playRound(5, "405", 3) == 2` — the **`0` sentinel** is skipped (`continue`), not counted
- `playRound(9, "123459", 5) == 0` vs `(…, 6) == 6` — the **budget** must cut the win off

Turning that wall of red into a single green line is the whole exercise.

## Hints
<details><summary>Task 1 — a chain, not three separate ifs</summary>

```cpp
if (guess < secret)       return -1;
else if (guess > secret)  return +1;
else                      return 0;
```
A *chain* picks exactly one branch. Three independent `if`s would also work here
because the cases happen to be exclusive, but the chain says "these are
alternatives" out loud — and it's the construct the task is practicing.
</details>
<details><summary>Task 2 — switch shape, and why break matters</summary>

```cpp
switch (code)
{
case -1: return "too low";   // a return ends the case…
case 0:  return "correct";
case 1:  return "too high";
default: return "invalid";
}
```
Each `case` here ends in `return`, so it can't fall through. If you instead
*assign* a label and want to read it after the `switch`, you **must** `break`
after each case — otherwise execution falls into the next one (8.6). Either style
is fine; just don't forget the `default`.
</details>
<details><summary>Task 3 — peel digits with % and /</summary>

```cpp
if (n < 0) n = -n;              // work on |n|
int sum { 0 };
while (n > 0) { sum += n % 10; n /= 10; }
return sum;
```
`n % 10` is the last digit; `n /= 10` drops it. When `n` reaches 0 the condition
is false and the body stops — that's the loop's exit story. (Handle the negative
*first* so you never take `%` of a negative and get a negative digit.)
</details>
<details><summary>Task 4 — separators go between, not around</summary>

```cpp
std::string out {};
int n { start };
while (n >= 1)
{
    if (n != start) out += "..";     // skip the "before-first" separator
    out += std::to_string(n);
    --n;
}
if (!out.empty()) out += "..";       // one more before the finale
out += "liftoff";
return out;
```
`std::to_string(3)` gives `"3"`. The two guards (`n != start`, and `!out.empty()`)
are what keep the `".."` strictly *between* tokens.
</details>
<details><summary>Task 5 — why a do-while, and where it stops</summary>

```cpp
int count { 0 };
int value { 0 };
do {
    value = typed[static_cast<std::size_t>(count)] - '0';
    ++count;
} while ((value < low || value > high) && count < static_cast<int>(typed.size()));
return count;
```
The body must run **once** before there's a `value` to test — that's the do-while
signature. The second half of the condition, `count < size`, is the exit story
that stops you reading past the end when nothing is ever valid.
</details>
<details><summary>Task 6 — for + continue (skip) + break (stop)</summary>

```cpp
int attempts { 0 };
for (std::size_t i { 0 }; i < guesses.size(); ++i)
{
    int guess { guesses[i] - '0' };
    if (guess == 0) continue;            // sentinel: skip, don't count
    ++attempts;
    if (attempts > maxAttempts) break;   // budget spent
    if (judgeGuess(secret, guess) == 0) return attempts;  // correct!
}
return 0;                                // loss
```
`continue` jumps to the next character without spending an attempt; `break` (or
the `return`) leaves the loop. Note the `++attempts` happens *before* the budget
check, so a guess judged on attempt `maxAttempts` still counts, but the one after
it is cut off.
</details>

## Stretch goals (optional — most need later chapters)
- Make `playRound` accept a real *range* of guesses 1–99 instead of single
  digits — which needs a parsed list of numbers (arrays / `std::vector`,
  Chapter 16) instead of a string of digit chars.
- Replace the `-1/0/+1` code in `judgeGuess` with a **scoped `enum class`**
  `Hint { TooLow, Correct, TooHigh }` and switch on it (Chapter 13).
- In `main`, **log the seed** before each game (`std::mt19937`’s seed) so a fun
  or buggy round can be replayed exactly — the reproducibility trick from notes
  8.14, and a real fuzzing habit.
- Add full input validation to the driver loop with `std::cin.fail()` /
  `clear()` / `ignore()` so non-numeric input can't wedge the game (Chapter 9).
- Turn `promptsUntilValid` into an actual `std::cin` `do while` in `main` and
  feed the engine the first valid value — keeping the *count* logic unit-tested.
