// ============================================================================
//  Chapter 4 — Fundamental Data Types · Project: Numeric Types Lab
//  numeric_lab.h  —  the public API you implement (DECLARATIONS ONLY).
// ============================================================================
//
//  This header is COMPLETE — do not edit it. It declares six small functions.
//  You write their bodies in `numeric_lab.cpp`; the grader in `tests/` calls
//  these exact signatures and checks the results.
//
//  The whole point of this lab is to make the behavior of the *fundamental
//  types* PHYSICAL — you'll observe overflow wrap to a small number, a `char`
//  turn into an `int` and back, two `double`s that "look equal" but aren't, and
//  a signed→unsigned cast that you must guard before it bites you.
//
//  A header guard keeps this file from being included twice in one translation
//  unit (Chapter 2). `#pragma once` does the same job in one line.
// ----------------------------------------------------------------------------
#ifndef NUMERIC_LAB_H
#define NUMERIC_LAB_H

#include <cstdint>   // std::uint8_t, std::int32_t — FIXED-WIDTH integers (4.6)
#include <cstddef>   // std::size_t — the unsigned type sizeof/.size() return (4.3, 4.6)

// ─── TASK 1 ─────────────────────────────────────────────────────────────────
// How many BYTES does one `std::int32_t` occupy?
//
// We deliberately ask about a FIXED-WIDTH type. Unlike plain `int` (whose size
// the standard only bounds as "at least 16 bits"), `std::int32_t` is *exactly*
// 32 bits wherever it exists — so its byte count is the same on every platform
// the course targets. That makes this a PLATFORM-INDEPENDENT fact a test can
// assert. Use the `sizeof` operator; it yields a `std::size_t`.
std::size_t int32ByteWidth();

// ─── TASK 2 ─────────────────────────────────────────────────────────────────
// Shift a character by `delta` positions in its encoding and return the result.
//
// A `char` is a number underneath (4.11): in ASCII, 'A' is 65, 'a' is 97. To do
// arithmetic on it cleanly you convert char→int, add, then convert int→char on
// the way back — a CHAR↔INT ROUND-TRIP. Example: shiftChar('A', 1) == 'B';
// shiftChar('a', -1) == '`'. Use `static_cast` for BOTH conversions so the
// intent is explicit and searchable. The caller guarantees the result stays in
// valid char range, so you don't need to validate here.
char shiftChar(char ch, int delta);

// ─── TASK 3 ─────────────────────────────────────────────────────────────────
// Does adding `addend` to the 8-bit unsigned value `start` WRAP AROUND?
//
// Unsigned arithmetic is modular (4.5): an 8-bit unsigned holds 0..255, and
// 255 + 1 becomes 0 — a defined "wraparound", NOT undefined behavior. Return
// true exactly when the true mathematical sum start+addend would exceed 255
// (i.e. the stored `std::uint8_t` result is smaller than `start`). Do the test
// WITHOUT signed overflow and WITHOUT just trusting the wrapped value blindly.
// Edge: start=255, addend=1 → wraps (returns true). start=255, addend=0 → no.
bool wrapsAround(std::uint8_t start, std::uint8_t addend);

// ─── TASK 4 ─────────────────────────────────────────────────────────────────
// Are two doubles equal WITHIN a tolerance `epsilon`?
//
// Floating-point values are approximate (4.8): 0.1 + 0.2 is NOT exactly 0.3, so
// comparing doubles with `==` is a classic bug. Instead, two values are "close
// enough" when the magnitude of their difference is at most `epsilon`. Return
// |a - b| <= epsilon. (You may use std::abs from <cmath>, already included in
// the .cpp; or build the absolute value yourself with an `if`.)
bool nearlyEqual(double a, double b, double epsilon);

// ─── TASK 5 ─────────────────────────────────────────────────────────────────
// Is `index` a valid position into a container of length `length`?
//
// `length` is a `std::size_t` (UNSIGNED — what .size()/sizeof give you). The
// caller's `index` is a signed `int`, which might be negative. The trap (4.12):
// casting a negative `int` to `std::size_t` yields a HUGE positive number, so
// you must check `index >= 0` FIRST, and only then compare it against `length`.
// Return true iff 0 <= index < length. When you finally compare to `length`,
// use a `static_cast<std::size_t>(index)` so signed/unsigned don't collide.
bool isValidIndex(int index, std::size_t length);

// ─── TASK 6 (capstone) ──────────────────────────────────────────────────────
// Classify a double using bool logic + an if / else-if chain. Return a CHAR tag:
//   'Z' if the value is (nearly) zero        — within 1e-9 of 0.0
//   'N' if it is negative                    — strictly less than 0
//   'B' if it is a "big" positive            — >= 1000.0
//   'P' otherwise (an ordinary positive)
// Check the cases IN THIS ORDER (4.10: conditions are tested top-to-bottom until
// one matches). Reuse your nearlyEqual for the zero test. Every `if` here is a
// control-flow BRANCH — exactly what coverage analysis in CS6340 measures.
char classifyNumber(double value);

#endif // NUMERIC_LAB_H
