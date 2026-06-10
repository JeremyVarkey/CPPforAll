// Chapter 14 — Introduction to Classes · Project: The Fraction Class  (SOLUTION)
// ─────────────────────────────────────────────────────────────────────────────
// Reference implementation. Peek only after a real attempt at starter/fraction.h.
// The learning is in designing the constructors and const-member-function
// signatures yourself; then compare here to see if your intuitions held.
//
// All member bodies are INLINE (defined inside the class braces) — the Ch 14
// idiom. The .h/.cpp split (ClassName::method out-of-class defs) arrives in
// Chapter 15.  Everything here uses only Chapter ≤ 14 concepts.
//
// ── CLASS DESIGN NOTES ────────────────────────────────────────────────────────
//
//   INVARIANT: m_denominator is always STRICTLY POSITIVE.
//   We absorb any negative sign from the denominator into the numerator so that
//   the reduced form is unique: 2/3 and -2/3 are each their own canonical form.
//   This makes equals() a simple field comparison — no cross-multiplication.
//
//   reduce() is the single place that enforces the invariant; it is called at
//   the end of every constructor that accepts caller-supplied values.  The
//   private visibility of gcd() and reduce() is important: callers outside the
//   class have no way to put the fraction into an un-reduced state.
//
//   CS6340 tie-in: LLVM's APInt uses a similar "normalize-on-construction"
//   pattern.  llvm::Fraction itself doesn't exist, but every class with a
//   "canonical" internal form encapsulates the normalization exactly like this.

#ifndef FRACTION_H
#define FRACTION_H

#include <iostream>    // std::ostream
#include <string>      // std::string, std::to_string

class Fraction
{
public:

    // ─── TASK 1: Default constructor (0/1) ───────────────────────────────────
    // MEMBER-INITIALIZER LIST (notes 14.10): initialize members BEFORE the
    // constructor body runs. For simple value types like Fraction, the list is
    // the whole story — the body is empty. List members in DECLARATION ORDER
    // (m_numerator first, then m_denominator) to match init order.
    Fraction()
        : m_numerator   { 0 }
        , m_denominator { 1 }
    {
    }

    // ─── TASK 2: Two-argument constructor ────────────────────────────────────
    // The member-init list stores the raw arguments; the body guards the zero-
    // denominator case and then calls reduce() to enforce the invariant.
    //
    // WHY NOT put the guard in the init list?
    //   We could write:  m_denominator { d == 0 ? 1 : d }
    //   but that leaves the sign-normalization to reduce() anyway, so keeping
    //   all invariant work in the body + reduce() is cleaner.
    Fraction(int n, int d)
        : m_numerator   { n }
        , m_denominator { d }
    {
        if (m_denominator == 0)
            m_denominator = 1;   // invalid input -> fall back to n/1
        reduce();                // normalize sign, divide out GCD
    }

    // ─── TASK 3: Explicit whole-number constructor ────────────────────────────
    // `explicit` blocks IMPLICIT conversion (notes 14.16).  Without it:
    //
    //   bool isHalf(Fraction f);
    //   isHalf(2);   // silently constructs Fraction{2} — surprising!
    //
    // With `explicit`, the caller MUST write Fraction{2} — the intent is clear.
    // Single-argument constructors should default to `explicit` unless you
    // specifically WANT the implicit conversion (e.g., std::string from const char*).
    //
    // Whole number w is stored as w/1 — already reduced, so no reduce() needed.
    explicit Fraction(int w)
        : m_numerator   { w }
        , m_denominator { 1 }
    {
    }

    // ─── TASK 4: Const accessor (getter) functions ───────────────────────────
    // The `const` AFTER the parameter list is the key syntax (notes 14.4):
    //   return_type fn_name(params) const { ... }
    //
    // It promises the function will NOT modify the implicit object. This lets
    // the function be called on const Fraction objects and on const Fraction& params.
    //
    // Without const, passing a Fraction by const reference and then calling
    // numerator() would fail at compile time — even though the function
    // "obviously" only reads. The compiler enforces const correctness strictly.
    int numerator() const
    {
        return m_numerator;
    }

    int denominator() const
    {
        return m_denominator;
    }

    // isValid() — always true (constructors guarantee m_denominator > 0).
    // Useful as a sanity assertion in callers and as a teaching example.
    bool isValid() const
    {
        return m_denominator != 0;
    }

    // ─── TASK 5: equals() — same-class private access ─────────────────────────
    // Both fractions are fully reduced, so equal fractions have IDENTICAL
    // numerator and denominator.  No cross-multiplication needed (and
    // cross-multiplying could overflow for large values).
    //
    // KEY INSIGHT (notes 14.5): objects of the SAME class may access each
    // other's private members inside member functions.  Here, `other.m_numerator`
    // and `other.m_denominator` are private, but we are inside Fraction's
    // member function, so the access is legal.  This pattern shows up constantly
    // in LLVM: one Instruction examining another's private flags.
    bool equals(const Fraction& other) const
    {
        return (m_numerator   == other.m_numerator)
            && (m_denominator == other.m_denominator);
    }

    // ─── TASK 6: multipliedBy() — return a NEW Fraction ──────────────────────
    // Fraction multiplication: (a/b) * (c/d) = (a*c) / (b*d).
    //
    // We construct a NEW Fraction via the two-arg constructor — which calls
    // reduce() — so the result is already in lowest terms.  `*this` is not
    // modified (const guarantees that).  The return is by value; C++17 copy
    // elision (notes 14.15) means the Fraction is constructed directly in the
    // caller's storage — no copy overhead.
    //
    // Like equals(), we access other.m_numerator / other.m_denominator directly
    // (same-class private access).
    Fraction multipliedBy(const Fraction& other) const
    {
        return Fraction { m_numerator   * other.m_numerator,
                         m_denominator * other.m_denominator };
    }

    // ── Print helpers (provided as scaffolding, no task) ─────────────────────

    void print(std::ostream& os = std::cout) const
    {
        os << m_numerator << '/' << m_denominator;
    }

    std::string toString() const
    {
        return std::to_string(m_numerator) + '/' + std::to_string(m_denominator);
    }

private:
    // ── PRIVATE DATA MEMBERS ──────────────────────────────────────────────────
    // `m_` prefix: the LearnCpp / LLVM naming convention for private data.
    // Declared AFTER the public interface; readers see the API first (notes 14.8).
    int m_numerator {};    // carries the sign; may be negative, zero, or positive
    int m_denominator {};  // ALWAYS > 0 after construction (invariant)

    // ── PRIVATE HELPERS ───────────────────────────────────────────────────────

    // gcd — Euclidean algorithm for Greatest Common Divisor.
    // Private because callers have no reason to use it; it is an implementation
    // detail of reduce().  We hand-roll the loop rather than calling std::gcd
    // (from <numeric>) because that header's advanced features are taught later.
    //
    // gcd(0, n) == n, gcd(n, 0) == n by convention.
    int gcd(int a, int b) const
    {
        if (a < 0) a = -a;   // Euclidean GCD works on magnitudes
        if (b < 0) b = -b;
        while (b != 0)
        {
            int tmp { b };   // rotate (a, b) -> (b, a % b); plain ints, no library needed
            b = a % b;
            a = tmp;
        }
        return a;
    }

    // reduce — enforce the class invariant in-place.
    //   1. If denominator is negative, negate both parts so denominator > 0.
    //   2. Divide both parts by gcd to produce the lowest-terms fraction.
    //
    // Called at the end of the two-argument constructor (the default and
    // explicit one-arg constructors already produce valid, reduced fractions
    // without needing this).
    void reduce()
    {
        // Invariant part 1: denominator must be positive.
        if (m_denominator < 0)
        {
            m_numerator   = -m_numerator;
            m_denominator = -m_denominator;
        }

        // Invariant part 2: no common factors (lowest terms).
        // gcd(0, d) == d and gcd(n, 1) == 1, so edge cases are safe.
        int g { gcd(m_numerator, m_denominator) };
        if (g > 1)
        {
            m_numerator   /= g;
            m_denominator /= g;
        }
    }
};

#endif // FRACTION_H
