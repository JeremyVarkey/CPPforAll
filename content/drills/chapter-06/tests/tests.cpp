// Chapter 6 — Operators · Project: Day-One Triage Console   (GRADER)
// ─────────────────────────────────────────────────────────────────────────────
// A tiny no-framework unit-test harness (same style as the drills/CLAUDE.md
// spec). It includes ../triage.h and calls each function across MANY inputs —
// THIS is the "loop" you don't have to write. Each CHECK that fails prints its
// expression and line number. Any failure -> non-zero exit -> `make test` is RED.
//
// The Makefile links this file against starter/triage.cpp (your code) for
// `make test`, and against solution/triage.cpp for `make test-solution`.

#include <iostream>
#include "../triage.h"

static int fails = 0;

// CHECK: assert a boolean condition; on failure, report what and where.
#define CHECK(cond) \
    do { if(!(cond)){ std::cerr << "FAIL: " #cond "  @line " << __LINE__ << "\n"; ++fails; } } while(0)

int main()
{
    // ── Task 1: parity ───────────────────────────────────────────────────────
    CHECK(isEven(0) == true);     // zero is even
    CHECK(isEven(2) == true);
    CHECK(isEven(3) == false);
    CHECK(isEven(-4) == true);    // edge: negative even
    CHECK(isEven(-7) == false);   // edge: negative ODD — the `% 2 == 1` trap
    CHECK(classifyParity(10) == "even");
    CHECK(classifyParity(11) == "odd");
    CHECK(classifyParity(-7) == "odd");   // edge: must NOT misfire on negatives

    // ── Task 2: FizzBuzz category (specific case first) ──────────────────────
    CHECK(fizzbuzzCategory(1)  == "number");
    CHECK(fizzbuzzCategory(9)  == "fizz");      // 3 only
    CHECK(fizzbuzzCategory(10) == "buzz");      // 5 only
    CHECK(fizzbuzzCategory(15) == "fizzbuzz");  // both — must beat plain "fizz"
    CHECK(fizzbuzzCategory(30) == "fizzbuzz");
    CHECK(fizzbuzzCategory(0)  == "fizzbuzz");  // edge: 0 % 3 == 0 and 0 % 5 == 0

    // ── Task 3: leap year (the famous century edge cases) ────────────────────
    CHECK(isLeapYear(2024) == true);    // divisible by 4, not a century
    CHECK(isLeapYear(2023) == false);   // not divisible by 4
    CHECK(isLeapYear(1900) == false);   // edge: century, not /400
    CHECK(isLeapYear(2000) == true);    // edge: century AND /400
    CHECK(isLeapYear(2100) == false);   // edge: century, not /400

    // ── Task 4: safe float comparison (both sides of epsilon) ────────────────
    CHECK(approxEqual(0.1 + 0.2, 0.3, 1e-9) == true);   // the classic == failure
    CHECK(approxEqual(1.0, 1.0, 1e-9) == true);         // exactly equal
    CHECK(approxEqual(1.0, 1.5, 0.1) == false);         // far apart -> false
    CHECK(approxEqual(5.0, 5.0 + 1e-12, 1e-9) == true); // within tolerance
    CHECK(approxEqual(-2.0, -2.0, 0.0) == true);        // edge: zero epsilon, identical

    // ── Task 5: non-negative wrap (negatives are the whole point) ────────────
    CHECK(wrapIndex(0, 3) == 0);
    CHECK(wrapIndex(2, 3) == 2);
    CHECK(wrapIndex(3, 3) == 0);
    CHECK(wrapIndex(7, 3) == 1);
    CHECK(wrapIndex(-1, 3) == 2);   // edge: raw -1 % 3 is -1; must fold to 2
    CHECK(wrapIndex(-3, 3) == 0);   // edge: lands exactly on 0
    CHECK(wrapIndex(-4, 3) == 2);

    // ── Task 6: sampling gate (short-circuit must guard the %) ───────────────
    CHECK(shouldSampleInstruction(0, 4, 10) == true);    // index 0 is on-stride
    CHECK(shouldSampleInstruction(8, 4, 10) == true);    // 8 % 4 == 0
    CHECK(shouldSampleInstruction(5, 4, 10) == false);   // off-stride
    CHECK(shouldSampleInstruction(8, 4, 0)  == false);   // out of budget
    CHECK(shouldSampleInstruction(8, 0, 10) == false);   // edge: sampleEvery 0 -> must NOT do % 0
    CHECK(shouldSampleInstruction(8, -2, 10) == false);  // edge: negative stride rejected

    // ── Task 7: priority ladder (every band + boundaries) ────────────────────
    CHECK(ticketPriority(100) == "P0");
    CHECK(ticketPriority(90)  == "P0");   // boundary
    CHECK(ticketPriority(89)  == "P1");   // just below
    CHECK(ticketPriority(70)  == "P1");   // boundary
    CHECK(ticketPriority(40)  == "P2");   // boundary
    CHECK(ticketPriority(39)  == "P3");
    CHECK(ticketPriority(0)   == "P3");

    // ── Task 8: exactly-one-of (XOR via !=) ──────────────────────────────────
    CHECK(exactlyOneSource(true,  false) == true);
    CHECK(exactlyOneSource(false, true)  == true);
    CHECK(exactlyOneSource(true,  true)  == false);   // both -> not "exactly one"
    CHECK(exactlyOneSource(false, false) == false);   // neither -> not "exactly one"

    // ── Task 9: integer power via std::pow ───────────────────────────────────
    CHECK(powInt(2, 0) == 1);     // edge: anything^0 == 1
    CHECK(powInt(2, 8) == 256);   // the value `2 ^ 8` would WRONGLY give (that's XOR = 10)
    CHECK(powInt(5, 3) == 125);   // rounding: pow may return 124.999…
    CHECK(powInt(10, 4) == 10000);
    CHECK(powInt(0, 5) == 0);     // edge: 0^positive == 0
    CHECK(powInt(7, 1) == 7);

    if (!fails)
        std::cout << "PASS ✅  all triage-console checks passed.\n";
    else
        std::cerr << "\nFAIL ❌  " << fails << " check(s) failed — fix the TASK blocks in triage.cpp.\n";

    return fails ? 1 : 0;
}
