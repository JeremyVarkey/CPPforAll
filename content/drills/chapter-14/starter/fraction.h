// Chapter 14 — Introduction to Classes · Project: The Fraction Class  (STARTER)
// ─────────────────────────────────────────────────────────────────────────────
// This is the ONLY file you edit. It contains the complete Fraction class with
// all member function bodies written INLINE — that is, inside the class braces.
// This is the Chapter 14 idiom; the .h/.cpp member split comes in Chapter 15.
//
// The file compiles immediately (the stubs return wrong-but-harmless values),
// so `make build` is GREEN right now. Your goal: fill in the six TASK blocks
// below until `make test` is also GREEN.
//
//     make build         compile-check this header (should pass immediately)
//     make test          grade your class  ->  RED until you fill these in
//     make test-solution run the grader against the reference if you get stuck
//
// ── CLASS DESIGN OVERVIEW ─────────────────────────────────────────────────────
//
//   A FRACTION represents the ratio numerator / denominator.
//
//   INVARIANT: denominator is never 0; we always keep it > 0 (denominator sign
//   is "absorbed" into the numerator so, e.g., 1/-2 is stored as -1/2).
//   Maintaining this INVARIANT is the class's main job — protecting it from
//   the outside world via PRIVATE data and a controlled PUBLIC interface.
//
//   CS6340 tie-in: LLVM uses classes like this everywhere. An `llvm::Fraction`
//   doesn't exist, but the idea — private data, public interface, invariant
//   maintenance via constructors — is exactly how `llvm::APInt` (arbitrary-
//   precision integer), `llvm::DebugLoc`, and many analysis result types work.
//
// ── SCOPE ─────────────────────────────────────────────────────────────────────
//   This header uses ONLY Chapter ≤ 14 features:
//     • class, public:/private:, member functions (Ch 14.2–14.5)
//     • const member functions (Ch 14.4)
//     • constructors + member-init lists (Ch 14.9–14.10)
//     • explicit one-arg constructor (Ch 14.16)
//     • loops, if/else (Ch 8)       — used in gcd() and reduce()
//   FORBIDDEN in this file: ClassName::method out-of-class definitions (Ch 15),
//   static members (Ch 15), operator overloading (Ch 21), destructors (Ch 15).
//   Use NAMED methods like equals() / multipliedBy(), not operator== / operator*.

#ifndef FRACTION_H
#define FRACTION_H

#include <iostream>    // std::ostream (used by print())
#include <string>      // std::string  (used by toString())

// ════════════════════════════════════════════════════════════════════════════
//  class Fraction
// ════════════════════════════════════════════════════════════════════════════
//
//  A RATIONAL NUMBER stored as (numerator, denominator) in reduced form with
//  denominator always POSITIVE (the numerator carries any negative sign).
//
//  Public interface (what callers can do):
//    Fraction{}             — constructs 0/1 (the additive identity)
//    Fraction{n, d}         — constructs n/d, normalized and reduced
//    explicit Fraction{w}   — constructs the whole number w/1; explicit
//                             prevents silent int→Fraction coercions
//    numerator()   const    — return the numerator
//    denominator() const    — return the denominator (always > 0)
//    isValid()     const    — true (denominator is always !=0, but still useful)
//    equals(other) const    — true when this == other (both fully reduced)
//    multipliedBy(other) const  — returns a NEW reduced Fraction (this * other)
//    print(os)     const    — prints "n/d" to the given stream
//    toString()    const    — returns "n/d" as a std::string
//
//  Private helper:
//    gcd(a, b)              — greatest common divisor (Euclidean algorithm)
//    reduce()               — reduces m_numerator / m_denominator in-place
//                             and ensures denominator is positive
class Fraction
{
public:
    // ── Member data (PRIVATE — callers cannot read or write these directly) ──
    // (Note: data is declared private at the bottom of the class. It can still
    // be listed here as a roadmap for readers.)

    // ─── TASK 1: Default constructor ─────────────────────────────────────────
    // Construct 0/1 — the "zero fraction". Use a MEMBER-INITIALIZER LIST
    // (the preferred C++ idiom — see notes 14.10). No body needed (empty {}).
    //
    // Pattern:
    //   Fraction()
    //       : m_numerator { 0 }    <-- list the members in DECLARATION order
    //       , m_denominator { 1 }  <-- a zero denominator would be invalid
    //   {
    //   }
    //
    //   >>> YOUR CODE HERE <<<
    //
    // ─────────────────────────────────────────────────────────────────────────
    Fraction()
        : m_numerator   { 0 }
        , m_denominator { 2 }   // placeholder: 0/2 is WRONG — fix the list to make 0/1
    {
        // body stays empty; do all the work in the member-initializer list above
    }

    // ─── TASK 2: Two-argument constructor ────────────────────────────────────
    // Construct n/d, normalized so denominator > 0, then reduced.
    //
    // Steps (all in the CONSTRUCTOR BODY — member-init list handles the raw
    // storage, then the body cleans it up):
    //   1. Member-init list: m_numerator{n}, m_denominator{d}.
    //   2. Body: guard against denominator 0 by setting m_denominator = 1 if
    //      d == 0.  (An invalid fraction becomes 0/1.)
    //   3. Call reduce() to normalize sign and divide out common factors.
    //
    // SIGN RULE (invariant): keep m_denominator POSITIVE; move any negative
    // sign into m_numerator. Example: Fraction{1,-2} stores -1/2.
    // reduce() handles this, so just call it.
    //
    //   >>> YOUR CODE HERE <<<
    //
    // ─────────────────────────────────────────────────────────────────────────
    Fraction(int /*n*/, int /*d*/)
        : m_numerator { 0 }
        , m_denominator { 1 }
    {
        // placeholder: always produces 0/1
    }

    // ─── TASK 3: Explicit one-argument constructor (whole-number conversion) ──
    // Construct the whole number w as w/1.
    //
    // WHY explicit? Without it, C++ allows IMPLICIT conversion: anywhere a
    // Fraction is expected, you could accidentally write just an int and the
    // compiler would silently construct a Fraction from it.  That is convenient
    // for well-known types like std::string (which accepts const char* implicitly)
    // but SURPRISING for Fraction, so we use the `explicit` keyword to require
    // the caller to write Fraction{5} explicitly. (notes 14.16)
    //
    //   >>> YOUR CODE HERE <<<
    //
    // ─────────────────────────────────────────────────────────────────────────
    explicit Fraction(int /*w*/)
        : m_numerator { 0 }
        , m_denominator { 1 }
    {
        // placeholder: always produces 0/1
    }

    // ─── TASK 4: const accessor (getter) functions ───────────────────────────
    // Return m_numerator and m_denominator respectively.
    // Marked `const` (after the parameter list) so they can be called on CONST
    // Fraction objects and through const references (notes 14.4).
    //
    //   >>> YOUR CODE HERE <<<
    //
    // ─────────────────────────────────────────────────────────────────────────
    int numerator() const
    {
        return 0;   // placeholder
    }

    int denominator() const
    {
        return 1;   // placeholder — 0 would be an illegal denominator
    }

    // isValid() — returns true when the denominator is non-zero.
    // Because our constructors guarantee denominator > 0, this always returns
    // true. Exposed as part of the public contract so callers can check.
    // Must be const (only reads, never modifies). (notes 14.4)
    bool isValid() const
    {
        return m_denominator != 0;
    }

    // ─── TASK 5: equals() — compare two Fractions ────────────────────────────
    // Return true when this fraction equals `other`.
    //
    // Because BOTH fractions are fully reduced (constructors call reduce()),
    // two equal fractions have IDENTICAL numerator AND denominator — so a
    // simple field comparison suffices.  Do NOT cross-multiply (unnecessary
    // and introduces overflow risk).
    //
    // Parameter type: `const Fraction& other` — const reference to avoid a
    // copy AND allow passing const Fractions (notes 14.4–14.5).
    //
    //   >>> YOUR CODE HERE <<<
    //
    // ─────────────────────────────────────────────────────────────────────────
    bool equals(const Fraction& /*other*/) const
    {
        return false;   // placeholder — never says equal (will fail most checks)
    }

    // ─── TASK 6: multipliedBy() — multiply two Fractions ─────────────────────
    // Return a NEW Fraction whose value is (this * other).
    //
    // Fraction multiplication: (a/b) * (c/d) = (a*c) / (b*d).
    // Construct and return a new Fraction with those products.
    // The two-argument constructor will call reduce(), so the result is
    // already in lowest terms. Do NOT mutate *this.
    //
    // Return type: Fraction (by VALUE — C++17 copy elision makes this free;
    // notes 14.15). Parameter: `const Fraction& other` (read-only, no copy).
    //
    //   >>> YOUR CODE HERE <<<
    //
    // ─────────────────────────────────────────────────────────────────────────
    Fraction multipliedBy(const Fraction& /*other*/) const
    {
        return Fraction{};   // placeholder — always returns 0/1
    }

    // ── Print helpers (provided, no task) ────────────────────────────────────
    // These are given to you so tests can display Fraction values. They use the
    // public accessors numerator() / denominator() only — no private access.

    // print() — write "n/d" to os (default: std::cout). const because it only
    // reads from the object. (notes 14.3, 14.4)
    void print(std::ostream& os = std::cout) const
    {
        os << numerator() << '/' << denominator();
    }

    // toString() — return "n/d" as a std::string for easy CHECK comparisons.
    std::string toString() const
    {
        return std::to_string(numerator()) + '/' + std::to_string(denominator());
    }

private:
    // ── PRIVATE DATA MEMBERS ─────────────────────────────────────────────────
    // The `m_` prefix is the LearnCpp / LLVM codebase convention for private
    // data members (notes 14.5). Callers cannot read or write these directly.
    int m_numerator {};    // numerator; carries the sign (may be negative)
    int m_denominator {};  // denominator; ALWAYS positive (enforced by reduce())

    // ── PRIVATE HELPER FUNCTIONS ─────────────────────────────────────────────

    // gcd — Greatest Common Divisor via the Euclidean algorithm.
    // This is a PRIVATE member function; callers outside the class cannot use it.
    // Returns the GCD of |a| and |b|; gcd(0, n) == n by convention.
    // We roll our own loop (not std::gcd from <numeric>) because <numeric>
    // advanced features are taught later. (loop skill: Ch 8)
    int gcd(int a, int b) const
    {
        if (a < 0) a = -a;   // work on magnitudes
        if (b < 0) b = -b;
        while (b != 0)
        {
            int tmp { b };
            b = a % b;
            a = tmp;
        }
        return a;    // when b reaches 0, a holds the GCD
    }

    // reduce — normalize in-place: ensure denominator > 0, divide out the GCD.
    // Called at the end of any constructor that accepts arbitrary arguments.
    // Keeping this in reduce() centralizes the invariant-maintenance code.
    void reduce()
    {
        // Step 1: keep denominator positive (move sign to numerator).
        if (m_denominator < 0)
        {
            m_numerator   = -m_numerator;
            m_denominator = -m_denominator;
        }
        // Step 2: divide out the greatest common divisor so the fraction is
        // already in lowest terms when callers receive it.
        int g { gcd(m_numerator, m_denominator) };
        if (g > 1)
        {
            m_numerator   /= g;
            m_denominator /= g;
        }
    }
};

#endif // FRACTION_H
