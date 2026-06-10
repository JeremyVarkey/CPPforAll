# Chapter 3 — Debugging C++ Programs · Project: Coin-Tray Auditor

> Reinforces [`../../notes/chapter-03.md`](../../notes/chapter-03.md) · LearnCpp [Chapter 3](https://www.learncpp.com/)

## The project
You've inherited a tiny maintenance tool for a vending machine. The **Coin-Tray
Auditor** is a little library of functions that totals the cash inside a
machine: the value of a pile of coins, the value of a loose coin tray, the
change a customer is owed, the value of full bank rolls of quarters, and the
grand-total **audit number** printed on the maintenance ticket.

There's a catch. **The code compiles cleanly and runs — and still gives wrong
answers.** That is the whole lesson of Chapter 3: a clean compile only rules out
**syntax errors** and **compile-time semantic errors**; it says *nothing* about
**logical errors**, where valid C++ computes the wrong thing (notes §3.1). Your
job is not to *write* this program — it's to **debug** it. You'll work a real
**bug report**, compare each function's *expected* output against what it
*actually* produces, and find the first place they diverge (notes §3.3). There
are **six planted bugs**, one of each classic beginner flavor.

## Concepts practiced
- **Syntax vs. compile-time-semantic vs. logical errors** — and why the last
  kind sails right past the compiler (3.1).
- **The debugging strategy:** compare *expected* state to *actual* state and
  localize the **first divergence** (3.2–3.3).
- **Functions as known-good / known-bad checkpoints** — each function is a
  stage you can confirm in isolation, then trust (3.3, "trace from known-good
  to known-bad").
- **The six classic logical bugs:** wrong operator, copy-paste slip, swapped
  operands, off-by-one constant, swapped call arguments, wrong initial value.
- *Reused from earlier chapters:* user-defined **functions**, parameters &
  **return values**, the **header/source split** and **header guards** (Ch 2);
  **variable initialization** and **arithmetic expressions** (Ch 1).

## The bug report (your "expected" column)
Maintenance filed these tickets. Each is a falsifiable expectation — exactly the
form notes §3.2 asks for ("expected X, actual Y"), and exactly what the grader
checks:

| Call | Expected | Buggy program prints |
|---|---:|---:|
| `coinWorth(3, kQuarter)` | `75` | `28` |
| `trayValue(2, 1, 0, 3)` | `63` | `42` |
| `changeOwed(100, 75)` | `25` | `-25` |
| `rollsValue(1)` | `1000` | `64` |
| `machineCents(1, 2, 1, 0, 3)` | `1063` | `131` |

> These "prints" are the **raw** output of the untouched starter, so several
> bugs **compound**: `trayValue`, `rollsValue`, and `machineCents` all call the
> broken `coinWorth`, so its wrong-operator bug leaks into their numbers too.
> That is itself the lesson — fix the earliest function first (`coinWorth`), and
> the downstream numbers shift to expose each *remaining* bug in isolation (the
> per-task hints below walk those isolated values).

## Your tasks
Every bug lives in [`starter/coin_tray.cpp`](starter/coin_tray.cpp). The
**interface and the contracts are correct** — read each `EXPECTED:` comment in
[`coin_tray.h`](coin_tray.h); that's your source of truth. Suspect lines are
flagged `// BUG?` (a *hypothesis* to confirm, not a guarantee). The five
`TASK` blocks below contain the six bugs:

1. **`coinWorth` — wrong operator.** The value of `count` coins is a *product*,
   not a sum. Make `coinWorth(3, kQuarter)` give `75`.
2. **`trayValue` — copy-paste slip.** One line was duplicated from its neighbour
   and never updated, so it multiplies by the wrong coin constant. Match every
   term's constant to the variable it scales.
3. **`changeOwed` — swapped operands.** Subtraction isn't commutative; the
   change is coming out negative. Order the operands so change is non-negative.
4. **`rollsValue` — off-by-one constant.** A roll is **40** quarters, not the
   hard-coded number. Use `kQuartersPerRoll` from the header so it can't drift.
5. **`machineCents` — two bugs: swapped call arguments + wrong initial value.**
   The call to `trayValue` passes its arguments in the wrong **order**, *and*
   the running total starts from a stray seed instead of `0`. Fix both.

Each numbered task maps 1:1 to a `// ─── TASK n ───` block in the starter.

## Constraints
- **Debug; don't rewrite.** Make the **smallest correct fix** per bug (notes
  §3.2, "avoid debugging by demolition"). Each bug is a one-token / one-line
  change.
- **Do not edit** [`coin_tray.h`](coin_tray.h) or
  [`tests/tests.cpp`](tests/tests.cpp) — the contracts and the bug report are
  fixed. You only edit `starter/coin_tray.cpp`.
- **Stay in scope:** straight-line arithmetic and function calls only. No `if`,
  no loops, no new types — none are taught yet, and none are needed.
- Keep every variable **initialized** and the code **warning-clean**
  (`-Wall -Wextra`).
- You may delete a `// BUG?` breadcrumb once you've confirmed that line is
  correct, but you don't have to.

## Build & run
```sh
make            # compile starter/coin_tray.cpp  ->  proves it BUILDS (clean compile != correct)
make test       # grade your code against the bug report  (RED until all six bugs are fixed)
make solution   # run the grader against the reference solution if you're stuck
make clean      # remove build artifacts
```
`make run` is an alias for `make test` (this is a library — running it *is*
grading it).

## Success criteria
`make test` prints **PASS ✅ all checks — every planted bug is fixed.** Until
then it prints, for each remaining bug, the failing expression and its line, e.g.

```text
FAIL: coinWorth(3, kQuarter) == 75  @line 26
FAIL: ...
FAIL ❌  N check(s) failing. Fix the FIRST one, rebuild, repeat.
```

Work it like the chapter teaches: **read the first failure, fix that one
function, rebuild, repeat.** Each failing line names the function to inspect;
its `EXPECTED:` comment in the header tells you what it should compute. Turning
that red into green — six bugs, one localized fix at a time — is the exercise.

## Hints
<details><summary>How to attack this (read first)</summary>

Don't scan for "wrong-looking" code. Let the **evidence** drive (notes §3.3).
Run `make test`, take the **first** `FAIL` line, open that function, and put its
`// BUG?` line next to the header's `EXPECTED:` promise. Ask: *does this line
compute the promise?* When two values disagree, you've found the divergence.
Fix only that, then rebuild — small change, clear cause and effect.
</details>

<details><summary>Task 1 — coinWorth</summary>

Three quarters is `3 * 25 = 75`. The body adds. What operator turns "three of
something worth 25" into 75? (This is the `area = width + height` bug from
notes §3.1, in miniature.)
</details>

<details><summary>Task 2 — trayValue</summary>

Run the isolating checks in your head: `trayValue(0,1,0,0)` should be `10` (one
dime). Once Task 1 is fixed so `coinWorth` multiplies, this call comes out `5` —
and a dime valued at `5` means the **dime** line is using the **nickel** constant
`kNickel`. It should use `kDime`.
</details>

<details><summary>Task 3 — changeOwed</summary>

`changeOwed(100, 75)` printing `-25` is the tell: the result is the exact
negative of the right answer. `a - b` vs `b - a`. Which order gives `+25`?
</details>

<details><summary>Task 4 — rollsValue</summary>

`rollsValue(1)` is one roll = `40 * 25 = 1000`. Once Task 1 is fixed so
`coinWorth` multiplies, this call prints `975 = 39 * 25` — the literal `39` is
off by one. Replace it with `kQuartersPerRoll` (defined as `40` in the header) —
named constants don't drift.
</details>

<details><summary>Task 5 — machineCents (two bugs)</summary>

Two separate slips:
- **Swapped arguments.** `trayValue`'s parameters are
  `(quarters, dimes, nickels, pennies)`. The call hands it `dimes` and
  `quarters` in the wrong slots — they get valued at each other's denomination.
  This is the notes §3.9 "caller passed the bad value" case: the symptom is in
  `trayValue`, the cause is the call.
- **Wrong initial value.** The accumulator starts at `kQuarter` (25), so the
  total is always 25c too high. Accumulators start at `0` (notes §3.8: a local
  that doesn't begin where you assumed).
</details>

## Stretch goals (optional)
- **Hard mode:** before reading any `// BUG?` flag, delete *all* of them, then
  re-find the six bugs using only the grader's failures and the header
  contracts. That's the real-world experience — bugs don't announce themselves.
- **Print-debugging drill (notes §3.4):** add temporary `std::cerr` traces
  inside `machineCents` — e.g. `std::cerr << "[audit] loose=" << loose << '\n';`
  — to *watch* `loose` and `rolled` and confirm where the number goes wrong.
  Use `std::cerr`, not `std::cout`, and remove the traces before you're done.
- **Regression test:** add one new `CHECK` to a copy of the grader that would
  have caught the off-by-one in `rollsValue` from a *different* input (e.g.
  `rollsValue(10) == 10000`) — practice writing a boundary test (notes §3.10).
- Add an `assert`-guarded contract such as "change is never negative" once you
  reach assertions (Chapter 9).
