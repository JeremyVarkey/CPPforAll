// Chapter 21 — Operator Overloading · Fraction v2  (STARTER)
// ─────────────────────────────────────────────────────────────────────────────
// You built Fraction in Chapter 14.  Now you upgrade it to speak C++ arithmetic
// natively.  Each TASK below asks you to implement one family of operator
// overloads.  Fill in the marked regions; everything else is scaffolding.
//
// The Makefile compiles THIS file together with tests/tests.cpp to produce the
// grader.  The grader runs the same operator expressions a user would write —
// if your implementation is correct, it goes GREEN.
//
// ── WHAT'S ALREADY HERE ──────────────────────────────────────────────────────
//
//   fraction.h — the complete class declaration, with m_numerator,
//                m_denominator, constructors, accessors, and the private
//                gcd()/reduce() helpers. Read it carefully before editing here.
//
// ── OPERATOR DESIGN RULES (notes 21.1, 21.5, 21.6, 21.8) ────────────────────
//
//   Operator     | Return type        | Why
//  ──────────────┼────────────────────┼───────────────────────────────────────
//   binary arith | new Fraction       | does not modify either operand
//   comparison   | bool               | observes only; never mutates
//   unary -      | new Fraction       | does not modify *this
//   prefix ++    | Fraction&          | mutates *this, returns ref for chaining
//   postfix ++   | Fraction (by val)  | returns the OLD value (copy made first)
//   operator<<   | std::ostream&      | chain: cout << a << b
//
// ── THE DUMMY-INT TRICK (notes 21.8) ─────────────────────────────────────────
//
//   C++ has only one operator++ name. To tell prefix from postfix, the language
//   uses a "dummy int" parameter in the POSTFIX overload:
//
//     Fraction& operator++();      // prefix:  ++x  ->  Fraction::operator++()
//     Fraction  operator++(int);   // postfix:  x++ ->  Fraction::operator++(0)
//
//   When you write x++, the compiler supplies 0 for that argument (see the
//   rewrite above), and the value carries no meaning — it is purely a syntax
//   marker. Do not use the dummy parameter in your body.
//
// ── WARNING-CLEAN PLACEHOLDERS ───────────────────────────────────────────────
//   Each stub uses (void) casts to silence -Wunused-parameter warnings so the
//   starter compiles warning-clean out of the box. Remove the (void) casts when
//   you write the real body — you'll need the parameter names.

#include "../fraction.h"

// ─── TASK 1: operator*(Fraction, Fraction) ───────────────────────────────────
// Fraction multiplication: (a/b) * (c/d) = (a*c) / (b*d).
//
// Construct and RETURN a new Fraction — use the two-argument constructor
// Fraction{num, den} which calls reduce() automatically. Return by value;
// C++17 copy elision means no copy overhead (notes 14.15).
//
// friend non-member: both operands are Fraction, no "left" object is privileged.
// Access rhs.m_numerator and rhs.m_denominator directly (friend private access).
//
// Constraints: const refs in, new Fraction out. No mutation of lhs or rhs.
//
Fraction operator*(const Fraction& lhs, const Fraction& rhs)
{
    // ─── TASK 1: operator*(Fraction, Fraction) ───────────────────────────────
    // (a/b) * (c/d) = (a*c)/(b*d). Use the two-arg constructor so reduce() runs.
    //
    //   >>> YOUR CODE HERE <<<
    //
    // ────────────────────────────────────────────────────────────────────────
    (void)lhs; (void)rhs;   // suppress unused warnings from placeholder
    return Fraction{ 0 };   // placeholder — remove and replace with the real body
}

// ─── TASK 2: operator*(Fraction, int) and operator*(int, Fraction) ────────────
// A Fraction times a whole number n is the same as Fraction * Fraction{n,1}.
// Implement operator*(Fraction, int) directly; make operator*(int, Fraction)
// delegate to it so you write the arithmetic only once (notes 21.2 — reuse).
//
Fraction operator*(const Fraction& f, int n)
{
    // ─── TASK 2a: operator*(Fraction, int) ───────────────────────────────────
    // Treat n as a whole-number fraction n/1. Return a new Fraction.
    //
    //   >>> YOUR CODE HERE <<<
    //
    // ────────────────────────────────────────────────────────────────────────
    (void)f; (void)n;       // suppress unused warnings from placeholder
    return Fraction{ 0 };   // placeholder
}

Fraction operator*(int n, const Fraction& f)
{
    // ─── TASK 2b: operator*(int, Fraction) ───────────────────────────────────
    // Reuse operator*(Fraction, int) — swap operand order, delegate.
    // One-liner: return f * n;
    //
    //   >>> YOUR CODE HERE <<<
    //
    // ────────────────────────────────────────────────────────────────────────
    (void)n; (void)f;       // suppress unused warnings from placeholder
    return Fraction{ 0 };   // placeholder
}

// ─── TASK 3: operator+(Fraction, Fraction) ───────────────────────────────────
// Fraction addition: (a/b) + (c/d) = (a*d + b*c) / (b*d).
//
// Again, return a new Fraction{...} so reduce() runs and the result is in
// lowest terms. Does NOT modify lhs or rhs.
//
Fraction operator+(const Fraction& lhs, const Fraction& rhs)
{
    // ─── TASK 3: operator+(Fraction, Fraction) ───────────────────────────────
    // (a/b) + (c/d) = (a*d + b*c) / (b*d), then reduce.
    //
    //   >>> YOUR CODE HERE <<<
    //
    // ────────────────────────────────────────────────────────────────────────
    (void)lhs; (void)rhs;   // suppress unused warnings from placeholder
    return Fraction{ 0 };   // placeholder
}

// ─── TASK 4: operator== and operator!= ───────────────────────────────────────
// Because both fractions are fully reduced (see invariant in fraction.h), two
// equal fractions have IDENTICAL m_numerator AND m_denominator.
// No cross-multiplication needed — just compare the two fields.
//
// operator!= should express itself in terms of operator==: !(a == b).
// (notes 21.7 — minimize redundancy, keep definitions consistent)
//
bool operator==(const Fraction& lhs, const Fraction& rhs)
{
    // ─── TASK 4a: operator== ─────────────────────────────────────────────────
    // Both fractions are reduced; equal fractions have identical stored fields.
    //
    //   >>> YOUR CODE HERE <<<
    //
    // ────────────────────────────────────────────────────────────────────────
    (void)lhs; (void)rhs;   // suppress unused warnings from placeholder
    return false;           // placeholder — always returns false (tests will fail)
}

bool operator!=(const Fraction& lhs, const Fraction& rhs)
{
    // ─── TASK 4b: operator!= — express this as !(lhs == rhs) ─────────────────
    //
    //   >>> YOUR CODE HERE <<<
    //
    // ────────────────────────────────────────────────────────────────────────
    (void)lhs; (void)rhs;   // suppress unused warnings from placeholder
    return true;            // placeholder
}

// ─── TASK 5: operator< ───────────────────────────────────────────────────────
// a/b < c/d  iff  a*d < b*c   (cross-multiplication, denominators always > 0).
//
// Because m_denominator is ALWAYS positive (the invariant), multiplying both
// sides of  a/b < c/d  by (b*d) — which is positive — does NOT flip the
// inequality direction.
//
// OVERFLOW NOTE (engineering aside — not in the chapter notes): the tests use
// |num| ≤ 20 and |den| ≤ 20, so the worst case is 20 * 20 = 400, well within
// int range. For a production library you would use long long or a checked
// multiply. (The ordering rule itself is notes 21.7; overflow is our own note.)
//
bool operator<(const Fraction& lhs, const Fraction& rhs)
{
    // ─── TASK 5: operator< via cross-multiplication ───────────────────────────
    // a/b < c/d  iff  a*d < b*c  (both denominators positive -> safe to cross-
    // multiply without flipping the comparison direction).
    //
    //   >>> YOUR CODE HERE <<<
    //
    // ────────────────────────────────────────────────────────────────────────
    (void)lhs; (void)rhs;   // suppress unused warnings from placeholder
    return false;           // placeholder
}

// ─── TASK 6: unary operator- ─────────────────────────────────────────────────
// Returns a NEW Fraction with the numerator sign flipped; denominator unchanged.
// *this is NOT modified (const member function).
//
// One operand -> member function (notes 21.6).
// Return by value (the new value, not a reference to a local).
//
Fraction Fraction::operator-() const
{
    // ─── TASK 6: unary operator- (negation) ──────────────────────────────────
    // Return a new Fraction with numerator negated: Fraction{-m_numerator, m_denominator}.
    // The denominator is already positive, so no reduce() needed — but passing
    // through the two-arg constructor is fine and harmless.
    //
    //   >>> YOUR CODE HERE <<<
    //
    // ────────────────────────────────────────────────────────────────────────
    return Fraction{ 0 };   // placeholder — wrong sign, tests will catch it
}

// ─── TASK 7: prefix operator++ ────────────────────────────────────────────────
// Add exactly 1 to the fraction.  1 as a fraction = denominator/denominator,
// so:  (num/den) + 1  =  (num + den) / den.
//
// Mutate *this, then return *this by reference (notes 21.8):
//   m_numerator += m_denominator;
//   reduce();
//   return *this;
//
// Returning by reference lets chaining work: ++(++x). (Unusual in practice,
// but the return convention MUST match built-in prefix++ behavior.)
//
Fraction& Fraction::operator++()
{
    // ─── TASK 7: prefix operator++ ────────────────────────────────────────────
    // (num/den) + 1 = (num + den) / den. Modify m_numerator, call reduce(),
    // return *this by reference.
    //
    //   >>> YOUR CODE HERE <<<
    //
    // ────────────────────────────────────────────────────────────────────────
    return *this;           // placeholder — does nothing (no increment)
}

// ─── TASK 8: postfix operator++ ───────────────────────────────────────────────
// Return the OLD value THEN increment.  The dummy int parameter (always 0) is
// the C++ syntax marker for postfix — do not use its value.
//
// Pattern (notes 21.8):
//   1. Copy *this into a local named `old`.
//   2. Increment *this (call the prefix ++ you just wrote).
//   3. Return `old` by value.
//
// Why by value?  The local `old` is destroyed when this function returns, so
// returning a reference to it would be dangling.
//
Fraction Fraction::operator++(int /*unused*/)
{
    // ─── TASK 8: postfix operator++ ────────────────────────────────────────────
    // Save *this (the old value), then call prefix ++(*this), then return old.
    // The dummy `int` parameter is 0 — ignore it.
    //
    //   >>> YOUR CODE HERE <<<
    //
    // ────────────────────────────────────────────────────────────────────────
    return Fraction{ 0 };   // placeholder — returns wrong value
}

// ─── TASK 9: operator<< (stream insertion) ────────────────────────────────────
// Output format:  "num/den"   e.g. Fraction{3,4}  prints "3/4"
//                             e.g. Fraction{-1,2} prints "-1/2"
//                             e.g. Fraction{0}    prints "0/1"
//
// Return the stream BY REFERENCE so chaining works:
//   std::cout << a << " and " << b << '\n';
// Each link in that chain receives and returns the same stream object.
//
// MUST be non-member: the left operand is std::ostream (not Fraction), so there
// is no *this to put this in as a member.  It is declared `friend` in
// fraction.h so we can access m_numerator / m_denominator directly.
//
// (The tests feed output into std::ostringstream — a preview of Ch 28's
//  in-memory string streams.  The grader infrastructure is provided as
//  scaffolding; you do not need to understand ostringstream to complete the task.)
//
std::ostream& operator<<(std::ostream& out, const Fraction& f)
{
    // ─── TASK 9: operator<< ───────────────────────────────────────────────────
    // Write f.m_numerator, then '/', then f.m_denominator to `out`. Return `out`.
    //
    //   >>> YOUR CODE HERE <<<
    //
    // ────────────────────────────────────────────────────────────────────────
    (void)f;                // suppress unused warnings from placeholder
    out << "?/?";           // placeholder — wrong output, tests will catch it
    return out;
}
