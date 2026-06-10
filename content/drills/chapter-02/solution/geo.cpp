// ============================================================================
//  solution/geo.cpp  —  reference IMPLEMENTATION of the geo library
// ----------------------------------------------------------------------------
//  One correct, warning-clean way to write the five bodies. Peek only after you
//  have tried your own — you learn the .h/.cpp split by getting YOUR version to
//  pass `make test` first.
//
//  Notice the shape: same #include of the paired header, same `namespace geo`
//  wrapper, and the bodies match the prototypes in geo.h exactly. This Foo.h /
//  Foo.cpp pairing is the template for every LLVM pass you will write later.
// ============================================================================

#include "geo.h"   // include our own paired header (2.11) — compile-time check
#include <cmath>   // std::sqrt

namespace geo
{
    // TASK 1 — both ints, product is int.
    int rectangleArea(int width, int height)
    {
        return width * height;
    }

    // TASK 2 — all four sides: two widths + two heights.
    int rectanglePerimeter(int width, int height)
    {
        return 2 * width + 2 * height;
    }

    // TASK 3 — divide by 2.0 (a double literal) so the division is done in
    // floating point and the ".5" on odd products survives. Writing `/ 2`
    // instead would truncate (15/2 == 7) before the double conversion — a
    // classic integer-division bug.
    double rightTriangleArea(int a, int b)
    {
        return (a * b) / 2.0;
    }

    // TASK 4 — Pythagoras. a*a + b*b is an int; std::sqrt promotes it to double
    // and returns a double.
    double hypotenuse(int a, int b)
    {
        return std::sqrt(a * a + b * b);
    }

    // TASK 5 — COMPOSITION: the diagonal IS the hypotenuse of the triangle made
    // by the rectangle's two sides. Reuse the function above instead of redoing
    // the square-root math (DRY, 2.2 / 2.6). Inside namespace geo we can call it
    // unqualified as hypotenuse(...).
    double rectangleDiagonal(int width, int height)
    {
        return hypotenuse(width, height);
    }
}
