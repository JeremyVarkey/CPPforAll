// coin_tray.cpp — Vending-Machine Coin-Tray Auditor  (STARTER / BUGGY)
//
// This program COMPILES and RUNS, warning-clean. That is exactly the danger of
// this chapter: a clean compile only rules out syntax errors and compile-time
// semantic errors. It says NOTHING about logical errors — code that builds and
// runs but computes the wrong answer (notes/chapter-03.md, §3.1).
//
// There are SIX planted logical bugs in the bodies below. Each is one of the
// classic beginner mistakes:
//     wrong operator · copy-paste slip · swapped operands ·
//     off-by-one constant · swapped call arguments · wrong initial value
//
// Every suspect line is flagged with a `// BUG?` breadcrumb. A breadcrumb is a
// HYPOTHESIS, not a confession: some are genuinely wrong, and you must confirm
// each one by comparing what the line computes against the EXPECTED contract in
// coin_tray.h. Fix the real bugs; if a flagged line is actually fine, you may
// delete its breadcrumb. `make test` is RED now and turns GREEN when all six
// are fixed.
//
// SCOPE: straight-line arithmetic and function calls only — no if, no loops
// (those are later chapters). Every bug here lives in the math, the operands,
// the constants, or the call arguments.

#include "../coin_tray.h"

// ─── TASK 1: fix coinWorth — value of `count` coins of `coinValue` cents ────
// EXPECTED (per coin_tray.h): count * coinValue.  e.g. coinWorth(3, 25) == 75.
// Compare the operator below against that promise.
//
//   >>> YOUR CODE HERE <<<
//
int coinWorth(int count, int coinValue)
{
    return count + coinValue;   // BUG?
}
// ────────────────────────────────────────────────────────────────────────────

// ─── TASK 2: fix trayValue — total cents of a loose coin tray ───────────────
// EXPECTED: quarters*25 + dimes*10 + nickels*5 + pennies*1.
// One line was copy-pasted from its neighbour and never updated. Read each
// term and check the coin constant matches the variable it multiplies.
//
//   >>> YOUR CODE HERE <<<
//
int trayValue(int quarters, int dimes, int nickels, int pennies)
{
    int q { coinWorth(quarters, kQuarter) };
    int d { coinWorth(dimes,    kNickel)  };   // BUG?
    int n { coinWorth(nickels,  kNickel)  };
    int p { coinWorth(pennies,  kPenny)   };
    return q + d + n + p;
}
// ────────────────────────────────────────────────────────────────────────────

// ─── TASK 3: fix changeOwed — cents of change after the customer overpays ───
// EXPECTED: amountPaid - price.  e.g. changeOwed(100, 75) == 25.
// The operands look swapped. Which order makes change come out POSITIVE?
//
//   >>> YOUR CODE HERE <<<
//
int changeOwed(int amountPaid, int price)
{
    return price - amountPaid;   // BUG?
}
// ────────────────────────────────────────────────────────────────────────────

// ─── TASK 4: fix rollsValue — value of `rolls` full rolls of quarters ───────
// EXPECTED: rolls * kQuartersPerRoll * kQuarter, and kQuartersPerRoll is 40.
// Someone hard-coded the count instead of using the named constant — and got
// it off by one. Prefer the constant from the header so it can never drift.
//
//   >>> YOUR CODE HERE <<<
//
int rollsValue(int rolls)
{
    int quartersInRolls { rolls * 39 };   // BUG?
    return coinWorth(quartersInRolls, kQuarter);
}
// ────────────────────────────────────────────────────────────────────────────

// ─── TASK 5: fix machineCents — grand-total audit number ────────────────────
// EXPECTED: trayValue(quarters, dimes, nickels, pennies) + rollsValue(rolls).
// TWO things are wrong in this body:
//   (a) the call to trayValue passes its arguments in the wrong ORDER — the
//       header's parameter order is (quarters, dimes, nickels, pennies); and
//   (b) the running total starts from a stray seed value instead of 0, so even
//       a correct sum comes out too large (a "wrong initial value" bug — the
//       local does not begin where you assumed; notes §3.8).
// Fix BOTH `// BUG?` lines.
//
//   >>> YOUR CODE HERE <<<
//
int machineCents(int rolls, int quarters, int dimes, int nickels, int pennies)
{
    int total { kQuarter };                                   // BUG?
    int loose { trayValue(dimes, quarters, nickels, pennies) };   // BUG?
    int rolled { rollsValue(rolls) };
    return total + loose + rolled;
}
// ────────────────────────────────────────────────────────────────────────────
