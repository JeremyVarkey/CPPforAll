// ============================================================================
//  Chapter 4 — Numeric Types Lab · unit-test grader (no framework).
// ============================================================================
//  Links against either starter/numeric_lab.cpp (→ RED) or
//  solution/numeric_lab.cpp (→ GREEN). See the Makefile.
//
//  Every assertion below is a PLATFORM-INDEPENDENT fact: we test fixed-width
//  widths and RELATIONSHIPS, never absolute sizeof(int)/sizeof(long) values
//  (those legitimately differ across machines).
// ----------------------------------------------------------------------------
#include <iostream>
#include "../numeric_lab.h"

static int fails = 0;

// Print the offending expression + line on failure; keep counting.
#define CHECK(cond) \
    do { if(!(cond)){ std::cerr << "FAIL: " #cond "  @line " << __LINE__ << "\n"; ++fails; } } while(0)

int main()
{
    // ── Task 1: int32ByteWidth ───────────────────────────────────────────────
    // 32 bits / 8 bits-per-byte = 4 bytes, guaranteed for the fixed-width type.
    CHECK(int32ByteWidth() == 4);
    // Relationship sanity: a 32-bit value is exactly four 8-bit bytes wide.
    CHECK(int32ByteWidth() == sizeof(std::uint8_t) * 4);

    // ── Task 2: shiftChar (char ↔ int round-trip) ────────────────────────────
    CHECK(shiftChar('A', 1) == 'B');     // 65 -> 66
    CHECK(shiftChar('A', 0) == 'A');     // identity shift
    CHECK(shiftChar('z', -25) == 'a');   // walk back down the alphabet
    CHECK(shiftChar('5', 1) == '6');     // digit chars are numeric too ('5'->'6')
    // Edge: a *negative* delta must move DOWN the encoding ('a'=97 -> '`'=96).
    CHECK(shiftChar('a', -1) == '`');

    // ── Task 3: wrapsAround (unsigned 8-bit modular arithmetic) ───────────────
    CHECK(wrapsAround(255, 1) == true);    // 255 + 1 -> 0 : wraps
    CHECK(wrapsAround(200, 100) == true);  // 300 > 255   : wraps
    CHECK(wrapsAround(100, 50) == false);  // 150 fits    : no wrap
    CHECK(wrapsAround(0, 0) == false);     // 0 fits      : no wrap
    // Edge: adding 0 to the maximum must NOT report a wrap.
    CHECK(wrapsAround(255, 0) == false);
    // Edge: the smallest possible overflow, 255 + 1, sits right on the boundary.
    CHECK(wrapsAround(254, 1) == false);   // 255 exactly : still fits

    // ── Task 4: nearlyEqual (floating-point tolerance) ───────────────────────
    CHECK(nearlyEqual(1.0, 1.0, 1e-9) == true);    // identical
    CHECK(nearlyEqual(1.0, 1.5, 1e-9) == false);   // far apart
    CHECK(nearlyEqual(2.0, 2.0 + 1e-12, 1e-9) == true); // within tolerance
    // THE classic trap: 0.1 + 0.2 is NOT exactly 0.3 in binary floating point...
    CHECK((0.1 + 0.2 == 0.3) == false);            // proof the trap is real
    // ...but nearlyEqual sees through it.
    CHECK(nearlyEqual(0.1 + 0.2, 0.3, 1e-9) == true);
    // Edge: a difference EXACTLY equal to epsilon counts as "near" (<=, not <).
    // We use exactly-representable values (1.0, 1.5, 0.5) so the boundary is
    // crisp — at larger magnitudes 5.0+1e-9 would round to *just over* epsilon,
    // which is itself the floating-point lesson, but a brittle thing to assert.
    CHECK(nearlyEqual(1.0, 1.5, 0.5) == true);
    // ...and just past it (epsilon a hair too small) is NOT near.
    CHECK(nearlyEqual(1.0, 1.5, 0.4) == false);

    // ── Task 5: isValidIndex (guard signed→unsigned cast) ────────────────────
    CHECK(isValidIndex(0, 5) == true);     // first slot
    CHECK(isValidIndex(4, 5) == true);     // last valid slot
    CHECK(isValidIndex(5, 5) == false);    // one past the end
    CHECK(isValidIndex(0, 0) == false);    // empty container: nothing is valid
    // Edge: the whole point — a negative index must be rejected, NOT cast into a
    // gigantic std::size_t that would slip past the length check.
    CHECK(isValidIndex(-1, 5) == false);

    // ── Task 6: classifyNumber (bool logic + if/else-if, branch coverage) ─────
    CHECK(classifyNumber(0.0)     == 'Z'); // (nearly) zero
    CHECK(classifyNumber(1e-12)   == 'Z'); // within 1e-9 of zero -> still 'Z'
    CHECK(classifyNumber(-3.5)    == 'N'); // negative
    CHECK(classifyNumber(42.0)    == 'P'); // ordinary positive
    CHECK(classifyNumber(2500.0)  == 'B'); // big positive (>= 1000)
    // Edge: exactly 1000.0 is the boundary into 'B' (>=, not >).
    CHECK(classifyNumber(1000.0)  == 'B');
    // Edge: ordering matters — -1e-12 is negative, but it sits within the zero
    // epsilon (1e-9), and the 'Z' test runs FIRST in the chain — so 'Z' beats 'N'.
    CHECK(classifyNumber(-1e-12)  == 'Z');

    if (!fails)
    {
        std::cout << "PASS ✅  all checks (" << "Numeric Types Lab" << ")\n";
    }
    return fails ? 1 : 0;
}
