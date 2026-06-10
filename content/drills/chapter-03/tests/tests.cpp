// tests.cpp — the automated grader for the Coin-Tray Auditor.
//
// This is the falsifiable "expected" side of the debugging story (notes §3.2):
// every CHECK is one row of the bug report — a concrete input with the answer
// the auditor is supposed to produce. A failing CHECK prints the offending
// expression and its line, which LOCALIZES the bug to one function. Read the
// FIRST failure first: fix the earliest divergence, rebuild, repeat.
//
// Tiny no-framework harness (same pattern as the other drills).

#include <iostream>
#include "../coin_tray.h"

static int fails { 0 };

#define CHECK(cond) \
    do { if (!(cond)) { \
        std::cerr << "FAIL: " #cond "  @line " << __LINE__ << '\n'; \
        ++fails; \
    } } while (0)

int main()
{
    // ── coinWorth: a count of coins is a PRODUCT, not a sum ──────────────
    CHECK(coinWorth(3, kQuarter) == 75);     // bug report: "expect 75, got 28"
    CHECK(coinWorth(4, kDime)    == 40);
    CHECK(coinWorth(1, kPenny)   == 1);
    CHECK(coinWorth(0, kQuarter) == 0);      // edge: zero coins -> zero value
    CHECK(coinWorth(7, 0)        == 0);      // edge: a zero-value coin

    // ── trayValue: each denomination must use ITS OWN constant ───────────
    CHECK(trayValue(2, 1, 0, 3) == 63);      // bug report: "expect 63, got 42"
    CHECK(trayValue(0, 1, 0, 0) == 10);      // isolates the dime term alone
    CHECK(trayValue(0, 0, 1, 0) == 5);       // isolates the nickel term alone
    CHECK(trayValue(0, 0, 0, 0) == 0);       // edge: empty tray
    CHECK(trayValue(4, 0, 0, 0) == 100);     // four quarters == one dollar

    // ── changeOwed: amountPaid - price, and it must be non-negative ──────
    CHECK(changeOwed(100, 75) == 25);        // bug report: "expect 25, got -25"
    CHECK(changeOwed(50, 50)  == 0);         // edge: exact payment -> no change

    // ── rollsValue: a roll is 40 quarters ($10.00) ──────────────────────
    CHECK(rollsValue(1) == 1000);            // bug report: "expect 1000, got 64"
    CHECK(rollsValue(0) == 0);               // edge: no rolls
    CHECK(rollsValue(3) == 3000);

    // ── machineCents: loose tray + rolled quarters, starting from zero ───
    CHECK(machineCents(1, 2, 1, 0, 3) == 1063);  // bug report: "expect 1063, got 131"
    CHECK(machineCents(0, 0, 0, 0, 0) == 0);     // edge: empty machine -> 0
    CHECK(machineCents(0, 1, 1, 1, 1) == 41);    // tray only: 25+10+5+1
    CHECK(machineCents(2, 0, 0, 0, 0) == 2000);  // rolls only: two $10 rolls

    if (!fails)
        std::cout << "PASS ✅  all checks — every planted bug is fixed.\n";
    else
        std::cerr << "FAIL ❌  " << fails
                  << " check(s) failing. Fix the FIRST one, rebuild, repeat.\n";

    return fails ? 1 : 0;
}
