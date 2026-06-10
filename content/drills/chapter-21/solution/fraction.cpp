// Chapter 21 — Operator Overloading · Fraction v2  (REFERENCE SOLUTION)
// ─────────────────────────────────────────────────────────────────────────────
// Complete, correct, warning-clean implementation of ../fraction.h.
// Peek only AFTER a real attempt at starter/fraction.cpp — the learning is in
// reasoning about operator return types and member-vs-friend choices yourself;
// then compare here.
//
// IMPORTANT CONCEPTS TO OBSERVE in this file:
//
//  1. FRIEND BINARY OPS (tasks 1–5): both operands are Fraction, so neither is
//     "the object." We implement them as non-member friend functions that access
//     m_numerator and m_denominator directly — no public getters needed.
//
//  2. MEMBER UNARY OP (task 6): only one operand (*this). Member is the natural
//     choice; returns a new value, so const and returns by value.
//
//  3. PREFIX vs POSTFIX ++ (tasks 7/8): the dummy-int trick. Prefix mutates and
//     returns *this by reference. Postfix saves a copy FIRST, increments second,
//     returns the copy — so the caller sees the OLD value. This is the #1 gotcha.
//
//  4. OPERATOR<< as FRIEND NON-MEMBER (task 9): left operand is std::ostream,
//     not Fraction — so it can never be a member of Fraction. Returns stream
//     by reference for chain-ability.
//
//  5. IMPLEMENT OP IN TERMS OF OP: operator!= delegates to operator==;
//     operator*(int,Fraction) delegates to operator*(Fraction,int);
//     postfix++ delegates to prefix++. This keeps logic in ONE place.
//
//  CS6340 lens: when you read LLVM source, every   ++it, it != end, *it,
//    out << value   you see is exactly these patterns on iterators, streams, and
//    APInt. Translating   it != end  to  operator!=(it, end)  to  !(it == end)
//    makes the code no longer magical.

#include "../fraction.h"

// ─── TASK 1: operator*(Fraction, Fraction) ───────────────────────────────────
// (a/b) * (c/d) = (a*c)/(b*d).
//
// We construct Fraction{a*c, b*d} and rely on the two-arg constructor's
// call to reduce() to put the result in lowest terms automatically.
// Same-class private access (friend): we read m_numerator/m_denominator
// directly, avoiding the need for public getters in the hot path.
//
// Notes 21.2: "arithmetic operators usually do not modify either operand."
// Both lhs and rhs are const-ref; return is by value.
Fraction operator*(const Fraction& lhs, const Fraction& rhs)
{
    return Fraction { lhs.m_numerator * rhs.m_numerator,
                      lhs.m_denominator * rhs.m_denominator };
}

// ─── TASK 2: operator*(Fraction, int) and operator*(int, Fraction) ────────────
// A whole number n is the fraction n/1. Rather than duplicating the arithmetic,
// operator*(int, Fraction) simply delegates to operator*(Fraction, int) — the
// "implement in terms of a smaller set" principle (notes 21.2).
//
// operator*(Fraction, int): multiply numerator by n, denominator unchanged.
Fraction operator*(const Fraction& f, int n)
{
    // n as a fraction is n/1, so (a/b) * (n/1) = (a*n) / (b*1).
    return Fraction { f.m_numerator * n, f.m_denominator };
}

// operator*(int, Fraction): reverse operands and call the Fraction*int overload.
// No arithmetic here — pure delegation. This is the standard pattern for
// symmetric mixed-type operators (notes 21.2).
Fraction operator*(int n, const Fraction& f)
{
    return f * n;   // delegate: calls operator*(const Fraction&, int) above
}

// ─── TASK 3: operator+(Fraction, Fraction) ───────────────────────────────────
// Standard fraction addition formula:
//   (a/b) + (c/d) = (a*d + b*c) / (b*d)
//
// The Fraction{...} constructor calls reduce(), so the result is automatically
// in lowest terms (e.g. 1/4 + 1/4 = 2/8 -> stored as 1/4).
//
// Pitfall: do NOT forget to reduce. Without it, 1/4 + 1/4 would store as 2/8
// and operator== would incorrectly claim 2/8 != 1/4.
Fraction operator+(const Fraction& lhs, const Fraction& rhs)
{
    return Fraction { lhs.m_numerator   * rhs.m_denominator
                    + rhs.m_numerator   * lhs.m_denominator,
                      lhs.m_denominator * rhs.m_denominator };
}

// ─── TASK 4: operator== and operator!= ───────────────────────────────────────
// INSIGHT: because reduce() is called on construction, equal fractions have
// IDENTICAL m_numerator and m_denominator. There is no need for
// cross-multiplication. This is the payoff for maintaining the invariant.
//
// Example: Fraction{2,4} and Fraction{1,2} both store {1,2} after reduce(),
// so == is just (1==1) && (2==2) -> true. ✅
//
// operator!= is expressed as !(a == b) so there is ONE truth — any change
// to the equality definition is automatically inherited by inequality.
// (notes 21.7: "minimize redundancy")
bool operator==(const Fraction& lhs, const Fraction& rhs)
{
    return (lhs.m_numerator   == rhs.m_numerator)
        && (lhs.m_denominator == rhs.m_denominator);
}

bool operator!=(const Fraction& lhs, const Fraction& rhs)
{
    return !(lhs == rhs);   // delegate — do not duplicate the equality logic
}

// ─── TASK 5: operator< ───────────────────────────────────────────────────────
// Cross-multiplication: a/b < c/d  iff  a*d < b*c.
//
// WHY IS THIS SAFE? m_denominator is ALWAYS positive (the class invariant), so
// multiplying both sides of  a/b < c/d  by (b*d) — which is positive — does NOT
// flip the inequality direction.
//
// OVERFLOW NOTE (engineering aside — not in the chapter notes): the tests keep
// |num| ≤ 20 and |den| ≤ 20, so the worst case is 20 * 20 = 400, well within
// int range. For a production library you would use long long or a checked
// multiply. (The ordering rule itself is notes 21.7; overflow is our own note.)
bool operator<(const Fraction& lhs, const Fraction& rhs)
{
    // a/b < c/d  iff  a*d < b*c  (denominators positive -> safe to cross-mult)
    return lhs.m_numerator * rhs.m_denominator
         < rhs.m_numerator * lhs.m_denominator;
}

// ─── TASK 6: unary operator- ─────────────────────────────────────────────────
// Negation: -(a/b) = (-a)/b.
// The denominator is already positive, so we can pass it straight through the
// two-arg constructor (which calls reduce() — harmless because gcd(a,b) ==
// gcd(-a,b)). Returning by value creates a new Fraction.
//
// `const` member: does NOT modify *this. (notes 21.6)
Fraction Fraction::operator-() const
{
    return Fraction { -m_numerator, m_denominator };
}

// ─── TASK 7: prefix operator++ ────────────────────────────────────────────────
// Add 1 to the fraction: (num/den) + 1 = (num + den) / den.
//
// We update m_numerator in-place, then call reduce() (the helper is accessible
// because we're inside a member function). Return *this by reference so that
// chaining (e.g. ++(++x)) would work — matching built-in prefix++ behavior.
//
// Not const: this function MUTATES the object (notes 21.5 const correctness).
Fraction& Fraction::operator++()
{
    m_numerator += m_denominator;   // add one whole unit (denominator/denominator)
    reduce();                        // re-normalize; handles sign and GCD
    return *this;                    // return ref to the now-modified object
}

// ─── TASK 8: postfix operator++ ───────────────────────────────────────────────
// The classic three-step pattern (notes 21.8):
//   1. Save *this (the pre-increment value) into a local copy.
//   2. Increment *this (delegate to prefix++ — ONE source of truth).
//   3. Return the saved copy by VALUE.
//
// Returning by VALUE is mandatory: `old` is a local variable that will be
// destroyed when this function returns. A reference to it would dangle.
//
// The dummy `int` parameter is always 0; we name it `/*unused*/` to suppress
// the -Wunused-parameter warning that -Wextra would otherwise emit.
Fraction Fraction::operator++(int /*unused*/)
{
    Fraction old { *this };   // step 1: copy the current (pre-increment) value
    ++(*this);                 // step 2: apply prefix++ (defined above)
    return old;                // step 3: return the OLD value (not a reference)
}

// ─── TASK 9: operator<< ───────────────────────────────────────────────────────
// Output "num/den" to the stream.  Return stream by reference for chaining.
//
// MUST be a non-member: the expression  cout << f  maps to  operator<<(cout, f).
// If this were a member of Fraction, the call shape would need to be  f << cout
// — backwards and impossible to use naturally. (notes 21.4)
//
// `friend` in the header grants private access so we can read m_numerator and
// m_denominator without going through public getters.
//
// The tests pass an std::ostringstream (a string-backed stream) instead of
// std::cout so output can be checked as a string value. This is a grader
// infrastructure borrow — std::ostringstream is formally Chapter 28 I/O. It is
// provided as scaffolding; you do not need to understand it to complete the task.
std::ostream& operator<<(std::ostream& out, const Fraction& f)
{
    out << f.m_numerator << '/' << f.m_denominator;
    return out;   // return the SAME stream for chaining
}
