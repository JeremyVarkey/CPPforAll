# Chapter 9 — Error Handling · Project: The Robust Input Desk

> Reinforces [`../../notes/chapter-09.md`](../../notes/chapter-09.md) · LearnCpp [Chapter 9](https://www.learncpp.com/)

## The project
You're building the **front desk of an order kiosk** — the layer that touches the
outside world. Outside-world input is *hostile*: users mistype, fat-finger extra
characters, paste junk, leave fields blank, and (in CS6340 terms) **fuzzers feed
your program malformed input on purpose**. A program that merely *compiled* is not
a program that *survives* that. This chapter is about making it survive.

You'll write the desk as a small library of **pure, testable** functions — a
quantity validator, a range clamp, a keystroke parser, a stock lookup, a
strict-digits check — and then the capstone: a **`readIntInRange` routine that
recovers from bad `std::cin` input** and keeps re-prompting until it gets a
valid number. Along the way you use the two error tools Chapter 9 introduces for
two different jobs: **`assert`/`static_assert`** to document states that are
*impossible in correct code* (a caller's bug), and **runtime validation/recovery**
for states that *happen in normal operation* (a user typing letters). The grader
hammers every function with good **and malicious** inputs — empty strings, letters
where numbers go, out-of-range values, trailing junk, EOF — and proves your desk
signals correctly and **never crashes**.

## Concepts practiced
- **Detecting & handling errors**: happy path vs **sad path**, and *who* recovers (9.4)
- **Sentinel values** — returning an out-of-band signal (`kInvalidChoice`) for bad input (9.4)
- **`std::cin` failure recovery**: `.fail()` state, `.clear()`, `.ignore(...)`, and **EOF** (9.5)
- **Read-then-validate** vs letting `operator>>` stop early — strict token checking (9.5)
- **`assert`** for **preconditions / impossible states**, with a descriptive `&& "msg"` (9.6)
- **`static_assert`** for a **compile-time** invariant (table size) — no runtime cost (9.6)
- **Input validation** & **boundary/category testing**: empty, negative, out-of-range, junk (9.1, 9.2)
- Reused from earlier chapters: **named `constexpr` constants** & `std::string_view` (Ch 5),
  **`%`/relational/logical operators** (Ch 6), **loops, `if`, `while(true)`** (Ch 8)

## Your tasks
The starter compiles immediately but every function returns a **placeholder**, so
`make test` starts **RED**. Fill in the six `>>> YOUR CODE HERE <<<` blocks in
[`starter/desk.cpp`](starter/desk.cpp). They ramp from a one-line validator to the
full input-recovery loop:

1. **Validate a quantity.** `isValidQuantity(int)` returns `true` iff the value is
   in `[kMinQuantity, kMaxQuantity]` **inclusive**. Use `>=` and `<=` — the
   boundaries `1` and `99` are *valid* (an off-by-one with `>`/`<` would wrongly
   reject them).
2. **Clamp with a precondition.** `clampToRange(value, lo, hi)` pins `value` into
   `[lo, hi]` (below→`lo`, above→`hi`, in-range→unchanged). `lo > hi` is a **caller
   bug**, not user error, so open with
   `assert(lo <= hi && "clampToRange precondition: lo must be <= hi");`.
3. **Parse a keystroke (sentinel on junk).** `parseMenuChoice(char)` maps `'1'..'4'`
   to `1..4` and returns **`kInvalidChoice`** for *anything else* — other digits,
   letters, punctuation, whitespace, `'\0'`. Never crash on junk; hand back the
   sentinel so the caller can react.
4. **Stock lookup + compile-time invariant.** `isInStock(int)` reads a fixed table
   (item 3 is sold out). Add a **`static_assert`** proving the table has exactly
   `kMenuItemCount` entries, and **`assert`** that `itemNumber` is a valid menu
   number (the caller's responsibility). Remember the table is 0-based, the menu
   is 1-based.
5. **Strict digits-only.** `digitsOnly(std::string_view)` returns `true` iff the
   text is **non-empty** and **every** character is `'0'..'9'`. Empty → `false`.
   This is the *read-the-whole-token-then-validate* approach — stricter than
   `operator>>`, which would stop at the first bad char.
6. **The capstone — robust `readIntInRange(min, max)`.** Read one integer in
   `[min, max]` from `std::cin`, **retrying until valid**. Implement the full 9.5
   recovery loop: detect failed extraction, `clear()` the stream, `ignore()` the
   rest of the line, reject out-of-range numbers, and handle **EOF** so a closed
   stream can't loop forever. It must survive letters, blank lines, trailing junk
   (`12x`), and out-of-range input without crashing.

You may **not** edit [`desk.h`](desk.h) or anything in [`tests/`](tests) — those are
the contract and the grader.

## Constraints
- **Allowed:** `assert` / `static_assert` (`<cassert>`), `std::cin` / `std::cout`
  with `.clear()` and `.ignore(std::numeric_limits<std::streamsize>::max(), '\n')`
  (`<limits>`), `while`/`for`/`if`, range-`for`, `%`/relational/logical operators,
  named `constexpr` constants, `std::string_view`, and the function machinery
  already in the files.
- **Forbidden (not taught yet, after Ch 9):** `try`/`catch`/`throw` exceptions
  (mentioned in 9.4 but covered later), `std::optional`, `std::stoi`/`std::from_chars`,
  containers (`std::vector`), classes, templates. The notes name some of these as
  *future* options — using them here is out of scope.
- **Idioms required by the notes:**
  - `assert` is for **programmer bugs / impossible states**; runtime checks +
    re-prompting are for **user input** — don't `assert` on user data (9.6).
  - In `readIntInRange`, **snapshot the success flag before `clear()`**, then
    **always** `ignore()` the rest of the line (so `12x`'s `x` can't poison the
    next read), and **handle EOF** before retrying (9.5).
  - Use a **descriptive assertion** — append `&& "message"` — so a failure
    explains itself (9.6).
  - `static_assert` (not `assert`) for the table-size check, because it's
    **compile-time knowable** (9.6 decision table).
- Keep Tasks 1–5 **pure**: read the parameters, return a value — no I/O, no globals.
  All interactive I/O lives in Task 6.

## Build & run
```sh
make            # compile-check your starter/desk.cpp (warning-clean)
make test       # grade your code  ->  RED until the TASK blocks are filled in
make solution   # run the grader against the reference solution
make clean      # remove build artifacts
```
(`make run` is an alias for `make test` here — for this lab, "running" your code
*is* running the graders against it, since the graders supply `main`.) `make test`
runs **two** graders: [`tests/tests.cpp`](tests/tests.cpp) unit-tests the pure
functions (Tasks 1–5), and [`tests/cin_driver.cpp`](tests/cin_driver.cpp) feeds
hostile input from [`tests/cin_input.txt`](tests/cin_input.txt) to your
`readIntInRange` (Task 6).

## Success criteria
`make test` prints **PASS ✅ all parts passed — the input desk is robust.** and
exits 0. Until then:
- The pure-function grader prints one `FAIL: …  @line N` per broken check (the
  expression and the line in [`tests/tests.cpp`](tests/tests.cpp) it tripped on)
  and ends `FAIL ❌ N check(s) failed`.
- The `std::cin` grader expects your reader to accept exactly **7** then **50** out
  of the hostile script; anything else prints `FAIL ❌ readIntInRange did not
  recover correctly` with the values it actually got.

The graders exercise the inputs that separate *robust* from *plausible-looking*:
- `isValidQuantity(0)` / `(100)` / `(-5)` — just outside the inclusive `[1,99]` band
- `clampToRange(7, 7, 7)` — the degenerate `lo == hi` range (still legal)
- `parseMenuChoice('0')` / `('5')` / `('a')` / `(' ')` / `('\0')` — every junk → sentinel
- `digitsOnly("")` — **empty string is not digits-only** (the easy boundary to miss)
- `digitsOnly("12a45")` / `(" 123")` / `("-5")` / `("3.14")` — one bad char fails the whole token
- the `cin` script: `abc` (not a number), `200x` (out of range **and** trailing junk),
  blank line (skipped, no crash), then a valid value — all recovered without a crash

Turning that wall of red into a single green line is the whole exercise.

## Hints
<details><summary>Task 1 — inclusive bounds</summary>

`return (quantity >= kMinQuantity) && (quantity <= kMaxQuantity);`. Both endpoints
must count, so it's `>=`/`<=`, not `>`/`<`. Referencing the **named constants**
(not literal `1`/`99`) keeps the validator in sync with the rest of the desk.
</details>
<details><summary>Task 2 — assert first, then clamp</summary>

```cpp
assert(lo <= hi && "clampToRange precondition: lo must be <= hi");
if (value < lo) return lo;
if (value > hi) return hi;
return value;            // already in range — return it UNCHANGED
```
The string literal is always true, so it doesn't change the logic; it just shows
up in the abort message if the precondition is ever violated. (Want to *see* it
fire? Call `clampToRange(5, 10, 0)` from a scratch `main` — the program aborts
with `Assertion failed: (lo <= hi && "…")`.)
</details>
<details><summary>Task 3 — char arithmetic + the sentinel</summary>

The menu digits are consecutive characters, so once you know `key` is one of them,
`key - '0'` gives the number:
```cpp
if (key >= '1' && key <= '0' + kMenuItemCount)   // '1'..'4'
    return key - '0';
return kInvalidChoice;
```
`'0' + kMenuItemCount` is the char `'4'`. Everything that isn't `'1'..'4'` —
including other digits like `'0'` and `'5'` — falls through to the sentinel.
</details>
<details><summary>Task 4 — static_assert vs assert</summary>

```cpp
constexpr bool inStock[] { true, true, false, true };
static_assert(sizeof(inStock) / sizeof(inStock[0]) == kMenuItemCount,
              "stock table must have exactly kMenuItemCount entries");
assert(itemNumber >= 1 && itemNumber <= kMenuItemCount
       && "isInStock precondition: itemNumber must be a valid menu number");
return inStock[itemNumber - 1];                 // 1-based menu -> 0-based index
```
`static_assert` checks the *table size* at **compile time** (it's a constant, costs
nothing at runtime, and can't be disabled). `assert` checks the *argument* at
runtime, because `itemNumber` isn't a constant. Note the `- 1`: item 1 lives at
index 0.
</details>
<details><summary>Task 5 — reject empty, then scan</summary>

```cpp
if (text.empty())
    return false;                  // no characters -> not digits-only
for (char c : text)
    if (c < '0' || c > '9')        // any non-digit kills the whole token
        return false;
return true;
```
Comparing the raw `char` against `'0'..'9'` keeps the rule explicit (no
`std::isdigit` locale/sign surprises). The empty-string guard is the boundary the
notes' category list puts first.
</details>
<details><summary>Task 6 — the recovery loop, step by step</summary>

```cpp
assert(min <= max && "readIntInRange precondition: min must be <= max");
while (true)
{
    std::cout << "Enter a number [" << min << ", " << max << "]: ";
    int value {};
    std::cin >> value;

    if (std::cin.eof())            // stream closed: don't loop forever
    {
        std::cout << "Input closed; using " << min << ".\n";
        return min;
    }

    const bool extracted { static_cast<bool>(std::cin) };   // SNAPSHOT before clear()
    std::cin.clear();                                        // reset a failed stream
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');  // drop leftovers

    if (!extracted)                 { std::cout << "That was not a whole number. Try again.\n"; continue; }
    if (value < min || value > max) { std::cout << "That number is out of range. Try again.\n";  continue; }
    return value;                   // happy path
}
```
The three things people get wrong: (1) reading the success flag *after* `clear()`
(too late — `clear()` already wiped it); (2) forgetting `ignore()`, so `12x`
leaves `x` to break the next prompt; (3) not handling EOF, so a closed stream spins
forever re-failing. Order: snapshot → clear → ignore → decide.
</details>

## Stretch goals (optional — most need later chapters)
- Replace the `kInvalidChoice` sentinel with **`std::optional<int>`**, so "no valid
  choice" can't be confused with a real value (notes 9.4 previews this; Ch 13+).
- Rewrite `digitsOnly` + a parser as a real **`parseQuantity(std::string_view) ->
  std::optional<int>`** using `std::from_chars`, validating range during the parse.
- Make `readIntInRange` distinguish **EOF** from a fail state more richly (e.g.
  return a status enum instead of the `min` fallback) and report via `std::cerr`.
- Add **exceptions**: have a low-level parser `throw` on malformed input and let
  the desk `catch` it, contrasting that channel with sentinels (Ch 27).
- Wire the five validators into a full **interactive ordering loop** that reads a
  menu choice, a quantity, and confirms stock — a true integration test on top of
  your unit-tested parts (notes 9.1: unit vs integration).
