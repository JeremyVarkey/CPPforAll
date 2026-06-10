// ============================================================================
//  Chapter 4 — Fundamental Data Types · Project: Numeric Types Lab  (SOLUTION)
//  numeric_lab.cpp  —  one complete, correct reference implementation.
// ============================================================================
//
//  Peek only after you've tried your own version — you learn the types by
//  *fighting* them, not by reading the answer. Every function is warning-clean
//  under `clang++ -std=c++17 -Wall -Wextra`.
// ----------------------------------------------------------------------------
#include "../numeric_lab.h"

#include <cmath>   // std::abs(double)

// ─── TASK 1: int32ByteWidth ──────────────────────────────────────────────────
std::size_t int32ByteWidth()
{
    // `sizeof` reports the size of a TYPE in bytes and already evaluates to a
    // std::size_t, so we can hand it straight back. For std::int32_t this is 4
    // on every conforming platform (32 bits / 8 bits-per-byte) — a fixed-width
    // guarantee plain `int` does NOT give us.
    return sizeof(std::int32_t);
}

// ─── TASK 2: shiftChar ───────────────────────────────────────────────────────
char shiftChar(char ch, int delta)
{
    // A char is an integer underneath. Promote it to int EXPLICITLY, do the
    // arithmetic in int (where +/- behave normally), then cast the sum back to
    // char. The two static_casts document the char↔int round-trip; the caller
    // promised the result stays in valid char range, so no validation needed.
    int code { static_cast<int>(ch) };
    int shifted { code + delta };
    return static_cast<char>(shifted);
}

// ─── TASK 3: wrapsAround ─────────────────────────────────────────────────────
bool wrapsAround(std::uint8_t start, std::uint8_t addend)
{
    // Unsigned arithmetic is modular and DEFINED: an 8-bit unsigned holds
    // 0..255, so the sum naturally "wraps" past 255 back toward 0.
    //
    // Caution: std::uint8_t is narrower than int, so `start + addend` would be
    // promoted to int and would NOT wrap. To observe the real 8-bit wraparound
    // we force the result back into a std::uint8_t. If that wrapped result is
    // smaller than where we started, the true sum must have exceeded 255.
    std::uint8_t wrapped { static_cast<std::uint8_t>(start + addend) };
    return wrapped < start;
}

// ─── TASK 4: nearlyEqual ─────────────────────────────────────────────────────
bool nearlyEqual(double a, double b, double epsilon)
{
    // The right way to compare approximate values: ask whether they're within a
    // tolerance, never `a == b`. std::abs gives the magnitude of the gap.
    return std::abs(a - b) <= epsilon;
}

// ─── TASK 5: isValidIndex ────────────────────────────────────────────────────
bool isValidIndex(int index, std::size_t length)
{
    // Guard the sign FIRST. A negative int cast to std::size_t becomes a huge
    // positive number, which would sail past almost any length check — the
    // exact signed→unsigned trap from the notes. Only once we know index >= 0
    // is it safe to convert and compare against the unsigned length.
    if (index < 0)
    {
        return false;
    }
    return static_cast<std::size_t>(index) < length;
}

// ─── TASK 6: classifyNumber (capstone) ───────────────────────────────────────
char classifyNumber(double value)
{
    // An if / else-if / else chain: conditions are tested top-to-bottom and the
    // FIRST match wins, so order matters. Each branch below is a distinct
    // control-flow path — precisely what coverage analysis exercises.
    if (nearlyEqual(value, 0.0, 1e-9))   // reuse Task 4 instead of value == 0.0
    {
        return 'Z';
    }
    else if (value < 0.0)
    {
        return 'N';
    }
    else if (value >= 1000.0)
    {
        return 'B';
    }
    else
    {
        return 'P';
    }
}
