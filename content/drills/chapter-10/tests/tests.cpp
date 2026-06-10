// Chapter 10 — Type Conversion, Aliases, and Deduction · statkit   (GRADER)
// ─────────────────────────────────────────────────────────────────────────────
// A tiny no-framework unit-test harness (same style as the drills/CLAUDE.md
// spec). It includes ../statkit.h and calls each function across MANY inputs —
// THIS is the "loop" you don't have to write. Each CHECK that fails prints its
// expression and line number. Any failure -> non-zero exit -> `make test` is RED.
//
// Calculated doubles carry tiny representation error, so we never compare them
// with `==` (notes 6.7 / 10). CLOSE(a, b) asks "within a small epsilon?" instead.
// The exact-value checks (roundToInt, letterGrade, safeLength) return int/char,
// which DO compare exactly.
//
// The Makefile links this file against starter/statkit.cpp (your code) for
// `make test`, and against solution/statkit.cpp for `make test-solution`.

#include <iostream>
#include <cmath>            // std::fabs — for the epsilon comparison
#include <string_view>
#include "../statkit.h"

static int fails = 0;

// CHECK: assert a boolean condition; on failure, report what and where.
#define CHECK(cond) \
    do { if(!(cond)){ std::cerr << "FAIL: " #cond "  @line " << __LINE__ << "\n"; ++fails; } } while(0)

// CLOSE: the SAFE way to compare two doubles (never use == on calculated ones).
static bool close(double a, double b)
{
    return std::fabs(a - b) <= 1e-9;
}

int main()
{
    using namespace statkit;

    // ── Task 1: mean — the integer-division truncation trap ──────────────────
    CHECK(close(mean(7, 2), 3.5));     // THE trap: must be 3.5, not 3
    CHECK(close(mean(10, 4), 2.5));    // 10/4 == 2 in int division; want 2.5
    CHECK(close(mean(6, 3), 2.0));     // exact case still works
    CHECK(close(mean(1, 3), 0.3333333333)); // repeating fraction, real division
    CHECK(close(mean(0, 5), 0.0));     // edge: total 0
    CHECK(close(mean(-7, 2), -3.5));   // edge: negative total stays fractional

    // ── Task 2: percentage — same trap + operand order ───────────────────────
    CHECK(close(percentage(1, 4), 25.0));   // int 1/4 would be 0 -> 0%
    CHECK(close(percentage(1, 2), 50.0));
    CHECK(close(percentage(3, 4), 75.0));
    CHECK(close(percentage(0, 10), 0.0));   // edge: 0%
    CHECK(close(percentage(7, 7), 100.0));  // edge: full marks
    CHECK(close(percentage(1, 8), 12.5));   // fractional percent survives

    // ── Task 3: roundToInt — rounds, does not truncate; handles negatives ────
    CHECK(roundToInt(2.4) == 2);
    CHECK(roundToInt(2.5) == 3);     // half rounds UP (truncation would give 2)
    CHECK(roundToInt(2.6) == 3);
    CHECK(roundToInt(3.0) == 3);     // exact integer
    CHECK(roundToInt(-2.5) == -3);   // edge: half away from zero, not -2
    CHECK(roundToInt(-2.4) == -2);   // edge: negative rounds toward zero here
    CHECK(roundToInt(0.0) == 0);     // edge: zero

    // ── Task 4: roundTo — decimal rounding via scale/round/unscale ───────────
    CHECK(close(roundTo(3.14159, 2), 3.14));
    CHECK(close(roundTo(3.14559, 2), 3.15));   // rounds the 3rd place up
    CHECK(close(roundTo(2.5, 0), 3.0));        // edge: 0 places == roundToInt
    CHECK(close(roundTo(1.0 / 3.0, 3), 0.333));
    CHECK(close(roundTo(2.675, 1), 2.7));      // rounds to one decimal
    CHECK(close(roundTo(-1.2345, 2), -1.23));  // edge: negative value

    // ── Task 5: clampScore — clamp into [0,100], keep fractional part ────────
    CHECK(close(clampScore(87.5), 87.5));   // in range -> unchanged (and fractional)
    CHECK(close(clampScore(0.0), 0.0));
    CHECK(close(clampScore(100.0), 100.0));
    CHECK(close(clampScore(-5.0), 0.0));    // edge: below floor -> 0
    CHECK(close(clampScore(150.0), 100.0)); // edge: above ceiling -> 100
    CHECK(close(clampScore(99.9), 99.9));   // just under the ceiling

    // ── Task 6: letterGrade — char ladder, boundaries to the higher grade ────
    CHECK(letterGrade(95.0) == 'A');
    CHECK(letterGrade(90.0) == 'A');   // boundary -> higher grade
    CHECK(letterGrade(89.9) == 'B');   // just below
    CHECK(letterGrade(80.0) == 'B');   // boundary
    CHECK(letterGrade(70.0) == 'C');   // boundary
    CHECK(letterGrade(60.0) == 'D');   // boundary
    CHECK(letterGrade(59.9) == 'F');
    CHECK(letterGrade(0.0)  == 'F');   // edge: zero

    // ── Task 7: safeLength — unsigned size_t length -> signed int ────────────
    CHECK(safeLength("hello") == 5);
    CHECK(safeLength("") == 0);                 // edge: empty view
    CHECK(safeLength("a") == 1);
    CHECK(safeLength("cs6340") == 6);
    // The whole point: the result is a signed int we can safely do signed math on.
    CHECK(safeLength("ab") - safeLength("abcd") == -2);  // would WRAP if left unsigned

    // ── Task 8: weightedMean — usual arithmetic conversions (int*double) ─────
    // Equal weights reduce to the plain mean of the points.
    CHECK(close(weightedMean(80, 1.0, 90, 1.0, 100, 1.0), 90.0));
    // Skewed weights pull the result toward the heavily-weighted point.
    // (90*0.2 + 80*0.3 + 100*0.5) / (0.2+0.3+0.5) = (18+24+50)/1.0 = 92.0
    CHECK(close(weightedMean(90, 0.2, 80, 0.3, 100, 0.5), 92.0));
    // Fractional result: (70*1 + 75*1 + 0*0) / 2 = 72.5
    CHECK(close(weightedMean(70, 1.0, 75, 1.0, 0, 0.0), 72.5));
    // Single non-zero weight returns that point exactly (no int truncation).
    CHECK(close(weightedMean(7, 1.0, 0, 0.0, 0, 0.0), 7.0));

    if (!fails)
        std::cout << "PASS ✅  all statkit checks passed.\n";
    else
        std::cerr << "\nFAIL ❌  " << fails << " check(s) failed — fix the TASK blocks in statkit.cpp.\n";

    return fails ? 1 : 0;
}
