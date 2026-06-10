// ============================================================================
//  tests/tests.cpp  —  the automated grader for the geo library  (Chapter 2)
// ----------------------------------------------------------------------------
//  This is a CONSUMER of your library: like a tiny main(), it #includes the
//  header (the contract) and calls the API through the geo:: namespace. It does
//  NOT include any .cpp — the Makefile compiles this file together with EITHER
//  starter/geo.cpp (the `make test` target) OR solution/geo.cpp (the
//  `make test-solution` target) and lets the LINKER join the call to its
//  definition (2.8). That separation — declarations here, definition compiled
//  in separately — is the whole point of the chapter.
//
//  Tiny no-framework harness: CHECK fails loudly with the line number; the
//  program's EXIT CODE (0 = all good, 1 = something failed) is what `make`
//  turns into PASS / FAIL.
// ============================================================================

#include <iostream>
#include <cmath>        // std::abs for the floating-point comparison
#include "geo.h"        // the library's public interface (declarations only).
                        // Found via the -I. include path set in the Makefile.

static int fails = 0;

// Exact-equality check — for the int-returning functions, where == is correct.
#define CHECK(cond) \
    do { if(!(cond)){ std::cerr << "FAIL: " #cond "  @line " << __LINE__ << "\n"; ++fails; } } while(0)

// Floating-point check — doubles can carry tiny rounding dust, so compare
// "close enough" rather than bit-for-bit. (You'll formalize float comparison in
// Chapter 6; a small fixed epsilon is plenty for these whole-number answers.)
#define CHECK_NEAR(got, want) \
    do { double g_=(got), w_=(want); if(std::abs(g_ - w_) > 1e-9){ \
        std::cerr << "FAIL: " #got " == " #want "  (got " << g_ << ", want " << w_ \
                  << ")  @line " << __LINE__ << "\n"; ++fails; } } while(0)

int main()
{
    // ── geo::rectangleArea ────────────────────────────────────────────────
    CHECK(geo::rectangleArea(3, 4) == 12);
    CHECK(geo::rectangleArea(7, 1) == 7);
    CHECK(geo::rectangleArea(10, 10) == 100);
    CHECK(geo::rectangleArea(0, 5) == 0);     // edge case: zero-width → zero area

    // ── geo::rectanglePerimeter ───────────────────────────────────────────
    CHECK(geo::rectanglePerimeter(3, 4) == 14);    // 2*3 + 2*4
    CHECK(geo::rectanglePerimeter(5, 5) == 20);    // square
    CHECK(geo::rectanglePerimeter(0, 6) == 12);    // edge case: degenerate side

    // ── geo::rightTriangleArea ────────────────────────────────────────────
    CHECK_NEAR(geo::rightTriangleArea(3, 4), 6.0);    // even product → whole
    CHECK_NEAR(geo::rightTriangleArea(6, 8), 24.0);
    CHECK_NEAR(geo::rightTriangleArea(3, 5), 7.5);    // EDGE: odd product must be
                                                      // 7.5, not 7 — this is the
                                                      // integer-division trap.

    // ── geo::hypotenuse ───────────────────────────────────────────────────
    // Pythagorean triples chosen so the answers are exact whole numbers.
    CHECK_NEAR(geo::hypotenuse(3, 4), 5.0);
    CHECK_NEAR(geo::hypotenuse(5, 12), 13.0);
    CHECK_NEAR(geo::hypotenuse(6, 8), 10.0);

    // ── geo::rectangleDiagonal  (must REUSE geo::hypotenuse) ──────────────
    CHECK_NEAR(geo::rectangleDiagonal(3, 4), 5.0);     // same 3-4-5 triangle
    CHECK_NEAR(geo::rectangleDiagonal(8, 6), 10.0);    // legs swapped: still 10
    // Cross-check the composition: the diagonal of a w x h rectangle MUST equal
    // hypotenuse(w, h). If Task 5 re-derived the math wrong, this still catches
    // it; if it correctly delegates, the two are equal by construction.
    CHECK_NEAR(geo::rectangleDiagonal(5, 12), geo::hypotenuse(5, 12));

    if (!fails) std::cout << "PASS ✅  all geo checks passed.\n";
    else        std::cout << "FAIL ❌  " << fails << " check(s) failed — see lines above.\n";
    return fails ? 1 : 0;
}
