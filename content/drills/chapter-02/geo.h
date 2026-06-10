// ============================================================================
//  geo.h  —  the PUBLIC INTERFACE of the geo mini-library  (Chapter 2)
// ----------------------------------------------------------------------------
//  This header is COMPLETE and PROVIDED. You do not edit it. Read it closely —
//  a header is a contract. It tells every other file *what functions exist*,
//  *what they take*, and *what they return*, WITHOUT revealing how they work.
//  The bodies (the "how") live in a separate .cpp file that you implement.
//
//  Three Chapter-2 ideas are on display in this one little file:
//
//    1) HEADER GUARD  — the #ifndef / #define / #endif sandwich below stops the
//       file's contents from being pasted in twice if it gets #included more
//       than once in the same translation unit (2.12). The guard macro is the
//       FILENAME IN ALL CAPS with punctuation turned into underscores: GEO_H.
//
//    2) NAMESPACE     — every name lives inside `namespace geo { ... }` (2.9),
//       so you call them as geo::rectangleArea(...). The qualifier keeps these
//       names from colliding with anyone else's `area` or `perimeter`.
//
//    3) DECLARATIONS ONLY — a header holds forward declarations (prototypes):
//       return type, name, parameter types, semicolon, NO body (2.7, 2.11).
//       Putting a *definition* here would break the moment two .cpp files
//       included it (an ODR / "duplicate definition" linker error).
//
//  CS6340 / LLVM lens: every LLVM pass you will write looks exactly like this —
//  a `Foo.h` that declares an API inside a namespace behind a header guard, and
//  a `Foo.cpp` that defines the bodies. You are building that muscle now.
// ============================================================================

#ifndef GEO_H              // if GEO_H has not been defined yet...
#define GEO_H              // ...define it now, so a 2nd include skips the body

namespace geo              // a named scope region: everything here is geo::name
{
    // ── Rectangles (use int — whole-number sides) ──────────────────────────
    //
    // area = width * height.  Returns the enclosed area.
    int rectangleArea(int width, int height);

    // perimeter = twice the width plus twice the height (all four sides).
    int rectanglePerimeter(int width, int height);

    // ── Right triangles (use int legs; the hypotenuse may be fractional) ───
    //
    // A right triangle has two perpendicular "legs" a and b meeting at the
    // square corner. Its area is half of the bounding rectangle: (a * b) / 2.
    // Returns a double because halving an odd product is not a whole number.
    double rightTriangleArea(int a, int b);

    // The hypotenuse is the long slanted side, by the Pythagorean theorem:
    //   c = square_root(a*a + b*b).
    // Returns a double (square roots are rarely whole numbers).
    double hypotenuse(int a, int b);

    // ── Composition: a function built FROM the functions above ─────────────
    //
    // The straight-line diagonal of a width x height rectangle is the
    // hypotenuse of the right triangle whose legs are the rectangle's sides.
    // Implement this by REUSING geo::hypotenuse — do not re-derive the math.
    // (This is the "functions call functions" idea made physical, 2.1 / 2.6.)
    double rectangleDiagonal(int width, int height);
}

#endif // GEO_H
