// Chapter 16 — Dynamic Arrays: std::vector · Sensor-Readings Toolkit  (GRADER)
// ─────────────────────────────────────────────────────────────────────────────
// Tiny no-framework unit-test harness (drills/CLAUDE.md Style B spec).
// Includes ../sensor.h and calls the API through the sensor:: namespace.
// The Makefile links this file against starter/sensor.cpp for `make test`,
// and against solution/sensor.cpp for `make test-solution`.
//
// Each CHECK that fails prints its expression and line number; any failure ->
// non-zero exit -> `make test` is RED. When all pass: PASS ✅.
//
// Coverage strategy:
//   • Normal inputs covering the main loop path.
//   • EMPTY-vector edges — every function is tested on {} (the spec requires it).
//   • Boundary values (single-element, negative, large n).
//   • The static_cast-before-divide trap in averageReading (odd-sum case).
//   • normalizeInPlace mutation persistence (alias-vs-copy trap).
//   • buildRamp length and content correctness.
//   • lastReading with .at() — verifies the correct element, not just v[0].
//
// IMPORTANT: maxReading and lastReading have a documented precondition
// (non-empty vector, guarded by assert). They are only called here with non-empty
// vectors to avoid triggering the assert as a false failure; the empty-vector
// behavior is documented as "caller's responsibility" in sensor.h.

#include <iostream>
#include <cmath>          // std::abs for floating-point comparison
#include "../sensor.h"

static int fails = 0;

// Exact-equality check (for int-returning functions).
#define CHECK(cond) \
    do { if(!(cond)){ std::cerr << "FAIL: " #cond "  @line " << __LINE__ << "\n"; ++fails; } } while(0)

// Floating-point near-equality check (avoids rounding-dust false failures).
#define CHECK_NEAR(got, want) \
    do { double g_ = (got), w_ = (want); \
         if(std::abs(g_ - w_) > 1e-9){ \
             std::cerr << "FAIL: " #got " ~= " #want "  (got " << g_ << ", want " << w_ \
                       << ")  @line " << __LINE__ << "\n"; ++fails; } } while(0)

int main()
{
    // ── Task 1: totalReading — range-for sum ─────────────────────────────────
    CHECK(sensor::totalReading({}) == 0);                  // EDGE: empty vector -> 0
    CHECK(sensor::totalReading({5}) == 5);                 // single element
    CHECK(sensor::totalReading({1, 2, 3}) == 6);           // basic sum
    CHECK(sensor::totalReading({10, 20, 30, 40}) == 100);  // four elements
    CHECK(sensor::totalReading({-3, -2, -1}) == -6);       // negative readings
    CHECK(sensor::totalReading({-5, 5}) == 0);             // positive and negative cancel
    CHECK(sensor::totalReading({100, 200, 300}) == 600);

    // ── Task 2: averageReading — static_cast-before-divide ───────────────────
    CHECK_NEAR(sensor::averageReading({}), 0.0);           // EDGE: empty -> 0.0
    CHECK_NEAR(sensor::averageReading({4}), 4.0);          // single element
    CHECK_NEAR(sensor::averageReading({1, 2, 3}), 2.0);    // integer-friendly
    // KEY TRAP: 7 / 2 == 3 (integer division), but 7.0 / 2 == 3.5. This CHECK
    // catches the missing-cast bug that truncates the result.
    CHECK_NEAR(sensor::averageReading({3, 4}), 3.5);       // 7/2 = 3.5, NOT 3
    CHECK_NEAR(sensor::averageReading({1, 3}), 2.0);
    CHECK_NEAR(sensor::averageReading({10, 20, 30}), 20.0);
    CHECK_NEAR(sensor::averageReading({1, 1, 1, 1, 1}), 1.0);  // uniform

    // ── Task 3: maxReading — index loop with std::size_t ─────────────────────
    // Only non-empty vectors are tested here; non-empty is the documented
    // precondition (assert-guarded in the implementation).
    CHECK(sensor::maxReading({7}) == 7);                   // single element
    CHECK(sensor::maxReading({3, 1, 4, 1, 5, 9}) == 9);   // max at end
    CHECK(sensor::maxReading({9, 3, 1, 4, 1, 5}) == 9);   // max at front
    CHECK(sensor::maxReading({3, 9, 1, 4, 5}) == 9);      // max in middle
    CHECK(sensor::maxReading({5, 5, 5}) == 5);             // all equal
    CHECK(sensor::maxReading({-10, -3, -7}) == -3);        // all negative: -3 is max
    CHECK(sensor::maxReading({0, 0, 1, 0}) == 1);          // max among zeros

    // ── Task 4: countAbove — range-for + condition ────────────────────────────
    CHECK(sensor::countAbove({}, 5) == 0);                 // EDGE: empty -> 0
    CHECK(sensor::countAbove({1, 2, 3}, 10) == 0);         // none above threshold
    CHECK(sensor::countAbove({1, 2, 3}, 0) == 3);          // all above threshold
    CHECK(sensor::countAbove({5, 10, 15, 20}, 10) == 2);   // 15 and 20
    CHECK(sensor::countAbove({5, 10, 15, 20}, 5) == 3);    // strictly above 5: 10,15,20
    CHECK(sensor::countAbove({5}, 5) == 0);                // equal is NOT above
    CHECK(sensor::countAbove({5}, 4) == 1);                // just above threshold
    CHECK(sensor::countAbove({-1, 0, 1}, -1) == 2);        // 0 and 1 are above -1

    // ── Task 5: normalizeInPlace — auto& mutation (the alias-vs-copy trap) ────
    {
        std::vector<double> v {};
        sensor::normalizeInPlace(v, 2.0);
        CHECK(v.empty());                                  // EDGE: empty -> no-op
    }
    {
        std::vector<double> v { 4.0, 8.0, 12.0 };
        sensor::normalizeInPlace(v, 4.0);
        CHECK_NEAR(v[0], 1.0);                            // mutation persisted
        CHECK_NEAR(v[1], 2.0);
        CHECK_NEAR(v[2], 3.0);
    }
    {
        // KEY TRAP TEST: if the loop uses `auto x` (copy) instead of `auto& x`
        // (alias), the original vector is untouched. This check catches that.
        std::vector<double> v { 10.0, 20.0, 30.0 };
        sensor::normalizeInPlace(v, 10.0);
        CHECK_NEAR(v[0], 1.0);                            // if this fails: missing `&`
        CHECK_NEAR(v[1], 2.0);
        CHECK_NEAR(v[2], 3.0);
    }
    {
        std::vector<double> v { 1.0 };
        sensor::normalizeInPlace(v, 2.0);
        CHECK_NEAR(v[0], 0.5);                            // single element
    }

    // ── Task 6: buildRamp — push_back + reserve + return by value ────────────
    {
        auto r = sensor::buildRamp(0);
        CHECK(r.empty());                                  // EDGE: n==0 -> empty
    }
    {
        auto r = sensor::buildRamp(-5);
        CHECK(r.empty());                                  // EDGE: negative n -> empty
    }
    {
        auto r = sensor::buildRamp(1);
        CHECK(static_cast<int>(r.size()) == 1);
        // Only access r[0] when size is confirmed correct.
        if (static_cast<int>(r.size()) == 1)
            CHECK(r[0] == 0);                              // single-element ramp
    }
    {
        auto r = sensor::buildRamp(5);
        CHECK(static_cast<int>(r.size()) == 5);
        if (static_cast<int>(r.size()) == 5)
        {
            CHECK(r[0] == 0);
            CHECK(r[1] == 1);
            CHECK(r[2] == 2);
            CHECK(r[3] == 3);
            CHECK(r[4] == 4);                              // {0,1,2,3,4}
        }
    }
    {
        // Large ramp — ensures reserve didn't confuse capacity with length.
        auto r = sensor::buildRamp(100);
        CHECK(static_cast<int>(r.size()) == 100);
        if (static_cast<int>(r.size()) == 100)
        {
            CHECK(r[0] == 0);
            CHECK(r[99] == 99);
        }
    }

    // ── Task 7: lastReading — .at() bounds-checked subscript ─────────────────
    // Only non-empty vectors; non-empty is the documented precondition.
    CHECK(sensor::lastReading({42}) == 42);               // single element
    CHECK(sensor::lastReading({1, 2, 3}) == 3);           // last of three
    CHECK(sensor::lastReading({10, 20, 30, 40, 50}) == 50); // last of five
    // KEY TRAP TEST: if the implementation returns v[0] instead of the last
    // element, these catch it:
    CHECK(sensor::lastReading({1, 99}) == 99);            // not the first
    CHECK(sensor::lastReading({99, 1}) == 1);             // not the larger one

    // ── Integration: chain tasks together ────────────────────────────────────
    // Mimic a real sensor-pipeline: manually construct a known vector,
    // compute its stats using the toolkit functions.
    {
        // {0,1,2,3,4,5}: sum=15, avg=2.5, max=5, countAbove(2)=3, last=5
        std::vector<int> readings { 0, 1, 2, 3, 4, 5 };
        CHECK(sensor::totalReading(readings) == 15);
        CHECK_NEAR(sensor::averageReading(readings), 2.5); // 15/6 = 2.5
        CHECK(sensor::maxReading(readings) == 5);
        CHECK(sensor::countAbove(readings, 2) == 3);       // 3,4,5 are above 2
        CHECK(sensor::lastReading(readings) == 5);
    }
    {
        // Verify buildRamp produces the same vector as {0,1,2,3,4,5}.
        auto r = sensor::buildRamp(6);
        if (static_cast<int>(r.size()) == 6)
        {
            // Reuse totalReading to cross-check the ramp sum (0+1+2+3+4+5==15).
            CHECK(sensor::totalReading(r) == 15);
        }
        else
        {
            // Size wrong: count this as at least one failure.
            CHECK(static_cast<int>(r.size()) == 6);
        }
    }
    {
        // Normalize a small vector and verify results.
        std::vector<double> dv { 0.0, 2.0, 4.0, 6.0, 8.0 };
        sensor::normalizeInPlace(dv, 2.0);
        CHECK_NEAR(dv[0], 0.0);
        CHECK_NEAR(dv[4], 4.0);                            // 8 / 2 == 4
    }

    if (!fails)
        std::cout << "PASS \xE2\x9C\x85  all sensor checks passed.\n";
    else
        std::cerr << "\nFAIL \xE2\x9D\x8C  " << fails
                  << " check(s) failed -- fill in the TASK blocks in starter/sensor.cpp.\n";

    return fails ? 1 : 0;
}
