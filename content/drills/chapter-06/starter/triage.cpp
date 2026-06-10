// Chapter 6 — Operators · Project: Day-One Triage Console   (STARTER)
// ─────────────────────────────────────────────────────────────────────────────
// Fill in the nine TASK blocks below. Each maps 1:1 to a task in the README and
// to a declaration in ../triage.h. The bodies currently return PLACEHOLDERS so
// the file compiles immediately — that's why `make test` is RED right now. Your
// job is to turn it GREEN by implementing the real operator logic.
//
//     make build         compile your code (should already work)
//     make test          grade it          (RED until you fill these in)
//     make solution      run the reference if you get stuck
//
// No loops, no I/O, no globals — just operators, `if`, and return values.

#include "../triage.h"
#include <cmath>   // std::abs (Task 4), std::pow (Task 9)

// ─── TASK 1: parity with `%` and `?:` ────────────────────────────────────────
// isEven: an int is even when it divides by 2 with no remainder, i.e. its
// remainder when divided by 2 is 0. Use `%` and `==`.
// classifyParity: pick the label with ONE conditional operator:
//     condition ? "even" : "odd"
// Watch the trap from the notes: test oddness with `% 2 != 0`, NOT `% 2 == 1`,
// because in C++ a negative odd number has remainder -1, not 1.
//
//   >>> YOUR CODE HERE <<<
//
bool isEven(int /*value*/)
{
    return false;   // placeholder — replace with the real check
}

std::string_view classifyParity(int /*value*/)
{
    return "?";     // placeholder — replace with a ?: expression
}
// ─────────────────────────────────────────────────────────────────────────────

// ─── TASK 2: FizzBuzz as a category (no loop!) ───────────────────────────────
// Return one of "fizzbuzz" / "fizz" / "buzz" / "number" for a SINGLE value.
// Check the most specific case first: divisible by 3 AND by 5 (use `&&`).
// You may use `if`/`return`, or a nested `?:`. No printing, no loop — the grader
// supplies the loop by calling this across many numbers.
//
//   >>> YOUR CODE HERE <<<
//
std::string_view fizzbuzzCategory(int /*value*/)
{
    return "number";   // placeholder
}
// ─────────────────────────────────────────────────────────────────────────────

// ─── TASK 3: leap year with logical operators ────────────────────────────────
// Return, as ONE boolean expression (no `if`):
//     (year % 4 == 0) && ( (year % 100 != 0) || (year % 400 == 0) )
// Keep the parentheses — they make the precedence (and the intent) unambiguous.
//
//   >>> YOUR CODE HERE <<<
//
bool isLeapYear(int /*year*/)
{
    return false;   // placeholder
}
// ─────────────────────────────────────────────────────────────────────────────

// ─── TASK 4: safe floating-point comparison ──────────────────────────────────
// Calculated doubles rarely match with `==` (e.g. 0.1 + 0.2 != 0.3). Instead,
// return whether the magnitude of their difference is within epsilon:
//     std::abs(a - b) <= epsilon
//
//   >>> YOUR CODE HERE <<<
//
bool approxEqual(double /*a*/, double /*b*/, double /*epsilon*/)
{
    return false;   // placeholder
}
// ─────────────────────────────────────────────────────────────────────────────

// ─── TASK 5: non-negative wrap (ring-buffer index) ───────────────────────────
// Raw `index % size` can be negative in C++ (the result takes the sign of the
// left operand: -1 % 3 is -1). Compute the remainder, and if it came out
// negative, add `size` once to fold it into [0, size). Assume size > 0.
//
//   >>> YOUR CODE HERE <<<
//
int wrapIndex(int /*index*/, int /*size*/)
{
    return 0;   // placeholder
}
// ─────────────────────────────────────────────────────────────────────────────

// ─── TASK 6: instrumentation sampling gate (CS6340 tie-in) ───────────────────
// Fire only when ALL hold:  sampleEvery > 0,  budgetRemaining > 0,
// and instructionIndex lands on the stride (instructionIndex % sampleEvery == 0).
// ORDER MATTERS: put `sampleEvery > 0` FIRST so short-circuit `&&` skips the `%`
// when sampleEvery is 0 (otherwise you'd compute `x % 0` — undefined behavior).
//
//   >>> YOUR CODE HERE <<<
//
bool shouldSampleInstruction(int /*instructionIndex*/, int /*sampleEvery*/, int /*budgetRemaining*/)
{
    return false;   // placeholder
}
// ─────────────────────────────────────────────────────────────────────────────

// ─── TASK 7: ticket priority from a severity score ───────────────────────────
// Map score -> "P0"/"P1"/"P2"/"P3" using relational operators. A nested `?:`
// reads top-down like a ladder:
//     (score >= 90) ? "P0" : (score >= 70) ? "P1" : (score >= 40) ? "P2" : "P3"
// (You may use `if`/`return` instead if you prefer.)
//
//   >>> YOUR CODE HERE <<<
//
std::string_view ticketPriority(int /*score*/)
{
    return "P3";   // placeholder
}
// ─────────────────────────────────────────────────────────────────────────────

// ─── TASK 8: exactly-one-of (logical XOR via !=) ─────────────────────────────
// "Exactly one is true" for two bools is simply `fromFile != fromStdin`.
// (This trick is ONLY valid because both operands are already bool.)
//
//   >>> YOUR CODE HERE <<<
//
bool exactlyOneSource(bool /*fromFile*/, bool /*fromStdin*/)
{
    return false;   // placeholder
}
// ─────────────────────────────────────────────────────────────────────────────

// ─── TASK 9: integer power via std::pow ──────────────────────────────────────
// C++ has no exponent operator. Compute std::pow(base, exponent) in double, then
// convert back with static_cast<int>. std::pow lands on a value like 124.99999
// for some inputs, so add 0.5 before truncating to round to the nearest int:
//     static_cast<int>(std::pow(base, exponent) + 0.5)
// Assume exponent >= 0 and the true result is non-negative and fits in an int.
//
//   >>> YOUR CODE HERE <<<
//
int powInt(int /*base*/, int /*exponent*/)
{
    return 0;   // placeholder
}
// ─────────────────────────────────────────────────────────────────────────────
