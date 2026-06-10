// ============================================================================
//  Chapter 4 — Fundamental Data Types · Project: Numeric Types Lab  (STARTER)
//  numeric_lab.cpp  —  fill in the six TASK blocks, then `make test`.
// ============================================================================
//
//  This file COMPILES AS-IS (warning-clean), but every function returns a
//  deliberately wrong placeholder, so `make test` starts RED. Replace each
//  `>>> YOUR CODE HERE <<<` with a real body to turn it GREEN. The contract for
//  each function lives in ../numeric_lab.h — read those comments first.
//
//  Allowed this chapter: int / unsigned / fixed-width ints, float/double, bool,
//  char, sizeof, static_cast, and if / else (4.10). NO loops, NO switch, NO
//  std::string — those are later chapters.
// ----------------------------------------------------------------------------
#include "../numeric_lab.h"

#include <cmath>   // std::abs(double) — handy for the floating-point tasks

// ─── TASK 1: int32ByteWidth ──────────────────────────────────────────────────
// Return how many BYTES a std::int32_t occupies. Use the `sizeof` operator
// (it already yields a std::size_t, so you can return it directly).
// Hint: `sizeof(std::int32_t)`.
//
//   >>> YOUR CODE HERE <<<
//
std::size_t int32ByteWidth()
{
    return 0;   // placeholder — a real int32_t is never 0 bytes wide
}
// ─────────────────────────────────────────────────────────────────────────────

// ─── TASK 2: shiftChar ───────────────────────────────────────────────────────
// Convert `ch` to int, add `delta`, convert the sum back to char, return it.
// Use static_cast for BOTH conversions (char→int, then int→char).
//
//   >>> YOUR CODE HERE <<<
//
char shiftChar(char ch, int delta)
{
    (void)delta;     // silence "unused parameter" until you use it
    return ch;       // placeholder — returns the char unchanged (no shift)
}
// ─────────────────────────────────────────────────────────────────────────────

// ─── TASK 3: wrapsAround ─────────────────────────────────────────────────────
// Return true exactly when start + addend overflows an 8-bit unsigned value
// (true sum > 255). One clean way: store the wrapped result in a std::uint8_t
// and check whether it came out SMALLER than `start`. Don't use signed overflow.
//
//   >>> YOUR CODE HERE <<<
//
bool wrapsAround(std::uint8_t start, std::uint8_t addend)
{
    (void)start;
    (void)addend;
    return false;    // placeholder — claims nothing ever wraps (wrong for 255+1)
}
// ─────────────────────────────────────────────────────────────────────────────

// ─── TASK 4: nearlyEqual ─────────────────────────────────────────────────────
// Return true when |a - b| <= epsilon. Never compare doubles with ==.
// std::abs(double) (from <cmath>) gives you the magnitude of the difference.
//
//   >>> YOUR CODE HERE <<<
//
bool nearlyEqual(double a, double b, double epsilon)
{
    (void)a;
    (void)b;
    (void)epsilon;
    return false;    // placeholder — claims nothing is ever "close enough"
}
// ─────────────────────────────────────────────────────────────────────────────

// ─── TASK 5: isValidIndex ────────────────────────────────────────────────────
// Return true iff 0 <= index < length. CHECK index >= 0 FIRST; only then cast
// it to std::size_t and compare with length. (Casting a negative int to
// std::size_t makes a huge number — that's the bug we're guarding against.)
//
//   >>> YOUR CODE HERE <<<
//
bool isValidIndex(int index, std::size_t length)
{
    (void)index;
    (void)length;
    return true;     // placeholder — claims EVERY index is valid (even -1)
}
// ─────────────────────────────────────────────────────────────────────────────

// ─── TASK 6: classifyNumber (capstone) ───────────────────────────────────────
// Return a char tag using an if / else-if / else chain, IN THIS ORDER:
//   'Z' if nearlyEqual(value, 0.0, 1e-9)   (use your own Task-4 function)
//   'N' if value < 0.0
//   'B' if value >= 1000.0
//   'P' otherwise
//
//   >>> YOUR CODE HERE <<<
//
char classifyNumber(double value)
{
    (void)value;
    return '?';      // placeholder — not one of the real tags Z/N/B/P
}
// ─────────────────────────────────────────────────────────────────────────────
