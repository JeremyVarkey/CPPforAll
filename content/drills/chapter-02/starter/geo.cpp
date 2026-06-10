// ============================================================================
//  starter/geo.cpp  —  the IMPLEMENTATION of the geo library   (STARTER)
// ----------------------------------------------------------------------------
//  This is the file you edit. The header geo.h already DECLARED these five
//  functions; here you DEFINE them (write the bodies). Each one sits inside
//  `namespace geo { ... }` so its full name matches the declaration in the
//  header — geo::rectangleArea, geo::hypotenuse, and so on.
//
//  Fill in the five TASK blocks. Build / grade with:
//      make build   compile the starter (it already compiles)
//      make test    run the unit tests  ->  RED until you finish the tasks
//      make solution / make test-solution   the reference, for when you're stuck
//
//  WHY each stub currently returns a wrong-but-compiling placeholder: a value-
//  returning function must return a value on every path (2.2) — running off the
//  end is undefined behavior. The placeholders keep the file compiling and
//  warning-clean so your FIRST `make test` is a clean RED, not a build error.
//  Replacing them with the real formulas turns that red into green.
// ============================================================================

#include "geo.h"   // 2.11 best practice: a .cpp #includes its OWN paired header
                    // so the compiler checks your bodies against the contract.
#include <cmath>    // std::sqrt — you will need it for the hypotenuse.

namespace geo
{
    // ─── TASK 1: rectangleArea ─────────────────────────────────────────────
    // Return the area of a width x height rectangle: width * height.
    // Both parameters are int; the product is int. Easiest one — warm up.
    //
    //   >>> YOUR CODE HERE <<<
    int rectangleArea(int width, int height)
    {
        return width + height;   // placeholder: wrong (should be *, not +),
                                 // but compiles warning-clean. Replace it.
    }
    // ───────────────────────────────────────────────────────────────────────

    // ─── TASK 2: rectanglePerimeter ────────────────────────────────────────
    // Return the perimeter (the distance around all four sides):
    //   2 * width + 2 * height.
    //
    //   >>> YOUR CODE HERE <<<
    int rectanglePerimeter(int width, int height)
    {
        return width + height;  // placeholder: wrong (forgets the other two
                                // sides), but compiles warning-clean. Replace it.
    }
    // ───────────────────────────────────────────────────────────────────────

    // ─── TASK 3: rightTriangleArea ─────────────────────────────────────────
    // Return HALF the bounding rectangle's area: (a * b) / 2.
    // Watch the types: a and b are int, but you must return a double, and for
    // an ODD product (e.g. 3 * 5 = 15) the answer is 7.5, NOT 7. Make the
    // division happen in floating point — for example divide by 2.0, so the
    // fractional half is kept instead of being chopped off by integer division.
    //
    //   >>> YOUR CODE HERE <<<
    double rightTriangleArea(int a, int b)
    {
        return a + b;   // placeholder
    }
    // ───────────────────────────────────────────────────────────────────────

    // ─── TASK 4: hypotenuse ────────────────────────────────────────────────
    // Return the length of the long slanted side by the Pythagorean theorem:
    //   c = square_root(a*a + b*b).
    // Use std::sqrt (from <cmath>, already included above). a*a + b*b is an int
    // expression; std::sqrt takes/returns a double, so the result comes back as
    // a double automatically.
    //
    //   >>> YOUR CODE HERE <<<
    double hypotenuse(int a, int b)
    {
        return a + b;   // placeholder
    }
    // ───────────────────────────────────────────────────────────────────────

    // ─── TASK 5: rectangleDiagonal  (the stretch — COMPOSE, don't re-derive) ─
    // The diagonal of a width x height rectangle is the hypotenuse of the right
    // triangle whose two legs ARE the rectangle's sides. So DON'T call std::sqrt
    // again here — CALL the geo::hypotenuse function you just wrote, passing the
    // width and height as its two legs, and return whatever it gives you. This
    // is one of your own functions calling another (2.1 / 2.6). Because both
    // live in `namespace geo`, you can call it as just hypotenuse(width, height)
    // from in here (no geo:: prefix needed inside the namespace).
    //
    //   >>> YOUR CODE HERE <<<
    double rectangleDiagonal(int width, int height)
    {
        return width + height;  // placeholder
    }
    // ───────────────────────────────────────────────────────────────────────
}
