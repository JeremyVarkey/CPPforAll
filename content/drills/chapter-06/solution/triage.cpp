// Chapter 6 — Operators · Project: Day-One Triage Console   (REFERENCE SOLUTION)
// ─────────────────────────────────────────────────────────────────────────────
// One complete, correct, warning-clean implementation. Peek only after you've
// taken a real swing at starter/triage.cpp — the learning is in getting your own
// operators right, then comparing.

#include "../triage.h"
#include <cmath>   // std::abs, std::pow

// TASK 1 — parity.
// An int is even iff its remainder when divided by 2 is zero.
bool isEven(int value)
{
    return (value % 2) == 0;
}

// classifyParity reuses the same idea via the conditional operator `?:`.
// The condition is parenthesized for readability (notes 6.6 / 6.1).
std::string_view classifyParity(int value)
{
    return ((value % 2) == 0) ? "even" : "odd";
}

// TASK 2 — FizzBuzz category for a single value (no loop, no I/O).
// Check the most specific case (BOTH divisors) first; otherwise the plain
// "fizz" branch would shadow it. `&&` combines the two divisibility tests.
std::string_view fizzbuzzCategory(int value)
{
    if (((value % 3) == 0) && ((value % 5) == 0))
        return "fizzbuzz";
    if ((value % 3) == 0)
        return "fizz";
    if ((value % 5) == 0)
        return "buzz";
    return "number";
}

// TASK 3 — Gregorian leap year as one logical expression.
// Grouping: divisible by 4 AND (not a century OR a 400-multiple).
// The inner parentheses are required for correctness, not just style: without
// them `&&` would bind tighter than `||` and change the meaning.
bool isLeapYear(int year)
{
    return ((year % 4) == 0) && (((year % 100) != 0) || ((year % 400) == 0));
}

// TASK 4 — safe floating-point "equality".
// Calculated doubles carry tiny representation error, so we ask "are they within
// epsilon?" instead of "are they bit-identical?". std::abs gives |a - b|.
bool approxEqual(double a, double b, double epsilon)
{
    return std::abs(a - b) <= epsilon;
}

// TASK 5 — non-negative wrap.
// In C++ the sign of `%` follows the LEFT operand, so index % size can be
// negative. Fold a negative remainder back into [0, size) by adding size once.
int wrapIndex(int index, int size)
{
    int wrapped { index % size };

    if (wrapped < 0)
        wrapped += size;   // arithmetic-assignment shorthand (notes 6.2)

    return wrapped;
}

// TASK 6 — sampling gate (CS6340 instrumentation predicate).
// `sampleEvery > 0` MUST be the first operand: short-circuit `&&` then guarantees
// the `% sampleEvery` on the right is never reached when sampleEvery == 0, so we
// never trigger `x % 0` (undefined behavior). Parentheses make the gate auditable.
bool shouldSampleInstruction(int instructionIndex, int sampleEvery, int budgetRemaining)
{
    return (sampleEvery > 0)
        && (budgetRemaining > 0)
        && ((instructionIndex % sampleEvery) == 0);
}

// TASK 7 — priority ladder from a severity score.
// Nested `?:` reads as a top-down ladder; the first satisfied band wins. Because
// each `>=` test is strictly higher than the next, order alone resolves the bands.
std::string_view ticketPriority(int score)
{
    return (score >= 90) ? "P0"
         : (score >= 70) ? "P1"
         : (score >= 40) ? "P2"
         :                 "P3";
}

// TASK 8 — exactly-one-of via logical XOR.
// C++ has no `^^`; for two bool operands, `!=` means "exactly one is true".
// Valid ONLY because both operands are already bool (notes 6.8).
bool exactlyOneSource(bool fromFile, bool fromStdin)
{
    return fromFile != fromStdin;
}

// TASK 9 — integer power through std::pow.
// `^` is bitwise XOR, not exponent, and there is no `**` in C++. std::pow works
// in double and can land just below the true integer (e.g. 124.999…), so we add
// 0.5 before truncating with static_cast<int> to round to nearest.
int powInt(int base, int exponent)
{
    return static_cast<int>(std::pow(base, exponent) + 0.5);
}
