// Chapter 21 — Operator Overloading · Fraction v2  (GRADER)
// ─────────────────────────────────────────────────────────────────────────────
// Tiny no-framework unit-test harness (same style as drills/CLAUDE.md spec).
// Each CHECK that fails prints the expression and line number.
// Any failure -> non-zero exit -> `make test` is RED.
//
// The Makefile links this file against:
//   starter/fraction.cpp  (for `make test`         — the learner's code)
//   solution/fraction.cpp (for `make test-solution` — the reference)
//
// ── MACRO GOTCHA ─────────────────────────────────────────────────────────────
// The C preprocessor splits macro arguments on commas. Fraction{1,2} has a bare
// comma inside {}, which confuses the preprocessor into thinking it sees TWO
// arguments to CHECK(). Fix: assign the Fraction to a named variable first, or
// use the F() helper defined below (wraps Fraction{n,d} so the comma is inside a
// function call, not a raw braced-init — C++ parses it correctly before the
// preprocessor splits, because () is balanced before the macro expander runs).
//
// What makes tests MEANINGFUL here (rather than trivially true):
//   1. Values that catch "forgot to reduce": 1/4 + 1/4 should be 1/2 not 2/8.
//   2. Postfix++ must return the OLD value; prefix++ returns the NEW value.
//   3. operator<< verified by capturing into ostringstream and comparing strings.
//   4. operator*(int,Fraction) and operator*(Fraction,int) must give equal results.
//   5. Small values so cross-multiply in operator< stays well within int range.
//
// std::ostringstream: a string-backed stream from <sstream>.
//   oss << value;   writes into an internal buffer
//   oss.str()       returns the accumulated std::string
// This is a PREVIEW — formally Chapter 28 (I/O). Provided as grader scaffolding.
// ─────────────────────────────────────────────────────────────────────────────

#include <iostream>
#include <sstream>    // std::ostringstream — grader infrastructure (preview Ch 28)
#include <string>
#include "../fraction.h"

static int fails = 0;

// CHECK: if condition is false, print the expression and line number; count it.
#define CHECK(cond) \
    do { if(!(cond)){ \
        std::cerr << "FAIL: " #cond "  @line " << __LINE__ << "\n"; \
        ++fails; \
    } } while(0)

// F(n,d) — convenience alias that lets you write  F(1,2)  inside CHECK()
// without triggering the preprocessor comma-split.  F(n,d) is a function call;
// the C++ parser balances the () before the macro expander splits on commas.
static Fraction F(int n, int d) { return Fraction { n, d }; }

// Helper: stream a Fraction into an ostringstream and return the result string.
// Lets us test operator<< with a simple string comparison.
static std::string str(const Fraction& f)
{
    std::ostringstream oss;  // (grader plumbing — formally Ch 28 std::stringstream)
    oss << f;
    return oss.str();
}

int main()
{
    // ── Task 1: operator*(Fraction, Fraction) ─────────────────────────────────
    // Note: F(a,b) is the same as Fraction{a,b} — just safe inside CHECK().
    {
        Fraction half  { 1, 2 };
        Fraction third { 1, 3 };

        CHECK((half * third) == F(1,6));              // (1/2)*(1/3) = 1/6
        CHECK((F(2,3) * F(3,4)) == F(1,2));          // 6/12 -> 1/2
        CHECK((F(3,4) * F(4,3)) == F(1,1));          // 12/12 -> 1/1
        CHECK((F(0,1) * F(5,7)) == F(0,1));          // 0 * anything = 0

        // Negative operands
        CHECK((F(-1,2) * F(1,3)) == F(-1,6));
        CHECK((F(-1,2) * F(-1,3)) == F(1,6));        // neg*neg = pos

        // Result should be fully reduced
        CHECK((F(2,4) * F(1,1)) == F(1,2));          // 2/4*1/1 = 2/4 -> 1/2
    }

    // ── Task 2: operator*(Fraction, int) and operator*(int, Fraction) ──────────
    {
        Fraction half { 1, 2 };

        CHECK((half * 4) == F(2,1));                  // (1/2)*4 = 4/2 -> 2/1
        CHECK((4 * half) == F(2,1));                  // 4*(1/2) = same

        // Commutativity of mixed multiply
        CHECK((half * 3) == (3 * half));

        CHECK((F(2,3) * 3) == F(2,1));               // 6/3 -> 2/1
        CHECK((F(1,5) * 0) == F(0,1));               // anything * 0 = 0
        CHECK((0 * F(7,3)) == F(0,1));

        // Negative int
        CHECK((F(1,3) * -6) == F(-2,1));
    }

    // ── Task 3: operator+(Fraction, Fraction) ─────────────────────────────────
    {
        // Basic sums — result must be fully reduced
        CHECK((F(1,4) + F(1,4)) == F(1,2));          // 2/8 -> 1/2
        CHECK((F(1,3) + F(1,3)) == F(2,3));
        CHECK((F(1,2) + F(1,3)) == F(5,6));          // 3/6+2/6=5/6
        CHECK((F(1,6) + F(1,3)) == F(1,2));          // 1/6+2/6=3/6->1/2

        // Adding zero
        CHECK((F(3,4) + F(0,1)) == F(3,4));

        // Negative addend
        CHECK((F(3,4) + F(-1,4)) == F(1,2));         // 3/4-1/4=2/4->1/2

        // Sum that reduces to 1
        CHECK((F(1,4) + F(3,4)) == F(1,1));          // 4/4 -> 1/1
    }

    // ── Task 4: operator== and operator!= ─────────────────────────────────────
    {
        // Fractions that compare equal despite different construction
        CHECK(F(1,2) == F(2,4));                     // 2/4 reduces to 1/2
        CHECK(F(2,3) == F(4,6));                     // 4/6 reduces to 2/3
        CHECK(F(0,1) == F(0,5));                     // 0/5 reduces to 0/1

        // Distinct fractions
        CHECK(!(F(1,2) == F(1,3)));
        CHECK(F(1,2) != F(1,3));
        CHECK(!(F(3,4) != F(3,4)));                  // same fraction: != must be false

        // Negative fractions (sign normalization)
        CHECK(F(-1,2) == F(1,-2));                   // 1/(-2) reduces to -1/2
        CHECK(F(-2,4) == F(-1,2));                   // both reduce to -1/2
    }

    // ── Task 5: operator< ──────────────────────────────────────────────────────
    // All values have |num| <= 20, |den| <= 20 to stay safely within int.
    {
        CHECK(F(1,3) < F(1,2));                      // 1/3 < 1/2
        CHECK(F(1,4) < F(1,3));                      // 1/4 < 1/3
        CHECK(!(F(1,2) < F(1,3)));                   // 1/2 > 1/3
        CHECK(!(F(1,2) < F(1,2)));                   // equal: NOT less-than

        // Negative fractions
        CHECK(F(-1,2) < F(1,2));                     // negative < positive
        CHECK(F(-3,4) < F(-1,4));                    // -3/4 < -1/4

        // Zero comparisons
        CHECK(F(-1,2) < F(0,1));                     // negative < zero
        CHECK(F(0,1)  < F(1,2));                     // zero < positive
        CHECK(!(F(0,1) < F(0,1)));                   // zero not less than zero

        // Mixed denominators (the whole point of cross-mult)
        CHECK(F(2,5) < F(3,7));                      // 0.4 < ~0.43 -> true
        CHECK(!(F(3,7) < F(2,5)));
    }

    // ── Task 6: unary operator- ────────────────────────────────────────────────
    {
        Fraction half  { 1, 2 };
        Fraction neg   { -1, 2 };

        CHECK(-half == neg);                         // -(1/2) = -1/2
        CHECK(-neg  == half);                        // -(-1/2) = 1/2
        CHECK(-F(0,1) == F(0,1));                    // -(0/1) = 0/1 (sign of 0)

        // Original is unmodified
        Fraction a { 3, 4 };
        Fraction b { -a };
        CHECK(a == F(3,4));                          // a unchanged
        CHECK(b == F(-3,4));                         // b is the negation
    }

    // ── Task 7: prefix operator++ ─────────────────────────────────────────────
    {
        Fraction f { 1, 3 };

        // Prefix ++ returns a reference to the updated object (notes 21.8).
        Fraction& ref { ++f };
        CHECK(f   == F(4,3));                        // 1/3 + 1 = (1+3)/3 = 4/3
        CHECK(&ref == &f);                           // ref IS f (same address)

        // Further increment
        ++f;
        CHECK(f == F(7,3));                          // 4/3 + 1 = (4+3)/3 = 7/3

        // Increments that stay unreduced
        Fraction g { 1, 2 };
        ++g;                                         // (1+2)/2 = 3/2
        CHECK(g == F(3,2));

        // Increment from negative
        Fraction h { -3, 4 };
        ++h;                                         // (-3+4)/4 = 1/4
        CHECK(h == F(1,4));
    }

    // ── Task 8: postfix operator++ ────────────────────────────────────────────
    // THE KEY INVARIANT: postfix returns the value BEFORE the increment.
    {
        Fraction f { 1, 3 };

        // postfix: old gets the PRE-increment value; f itself is incremented
        Fraction old { f++ };
        CHECK(old == F(1,3));                        // old holds what f was BEFORE
        CHECK(f   == F(4,3));                        // f itself was incremented

        // Contrast prefix vs postfix explicitly
        Fraction a { 2, 5 };
        Fraction b { 2, 5 };

        Fraction prefix_result  { ++a };             // prefix: returns the new value
        Fraction postfix_result { b++ };             // postfix: returns the old value

        CHECK(prefix_result  == F(7,5));             // prefix returned the updated a
        CHECK(postfix_result == F(2,5));             // postfix returned the old b
        CHECK(a == F(7,5));                          // a was incremented
        CHECK(b == F(7,5));                          // b was also incremented

        // After double postfix++: each call returned its own old value
        Fraction c { 1, 5 };
        c++;   c++;
        CHECK(c == F(11,5));                         // 1/5 +1 -> 6/5 +1 -> 11/5
    }

    // ── Task 9: operator<< ────────────────────────────────────────────────────
    {
        CHECK(str(Fraction{ 1, 2 }) == "1/2");
        CHECK(str(Fraction{ 3, 4 }) == "3/4");
        CHECK(str(Fraction{-1, 2 }) == "-1/2");
        CHECK(str(Fraction{ 0, 1 }) == "0/1");
        CHECK(str(Fraction{ 1, 1 }) == "1/1");

        // Auto-reduce: operator<< must print the REDUCED form
        CHECK(str(F(2,4)) == "1/2");                 // 2/4 -> 1/2 -> "1/2"
        CHECK(str(F(6,4)) == "3/2");                 // 6/4 -> 3/2 -> "3/2"

        // Chain: two fractions in one stream expression
        std::ostringstream oss;  // (grader plumbing — formally Ch 28)
        oss << F(1,2) << "+" << F(1,3);
        CHECK(oss.str() == "1/2+1/3");
    }

    // ── Integration: operators compose naturally ───────────────────────────────
    {
        Fraction a { 1, 2 };
        Fraction b { 1, 3 };

        // -(a * b) == (-a) * b  (negation distributes over multiplication)
        CHECK(-(a * b) == (-a) * b);

        // Adding a positive makes the sum larger
        Fraction sum { a + b };                      // 5/6
        CHECK(b < sum);                              // 1/3 < 5/6 (use operator<)

        // operator< round-trip
        CHECK(b < a);                                // 1/3 < 1/2
        CHECK(!(a < b));
        CHECK(!(a < a));

        // Postfix++ old value is strictly less than the new value
        Fraction c { 1, 4 };
        Fraction before { c++ };
        CHECK(before < c);                           // old < new
    }

    // ── Final report ──────────────────────────────────────────────────────────
    if (!fails)
        std::cout << "PASS \xE2\x9C\x85  all Fraction operator checks passed.\n";
    else
        std::cerr << "\nFAIL \xE2\x9D\x8C  " << fails
                  << " check(s) failed — fix the TASK blocks in starter/fraction.cpp.\n";

    return fails ? 1 : 0;
}
