// Chapter 12 — Compound Types: References and Pointers · Alias Workshop (STARTER)
// ─────────────────────────────────────────────────────────────────────────────
// Fill in the six TASK blocks below. Each maps 1:1 to a task in the README and
// to a declaration in ../aliases.h. The bodies currently return PLACEHOLDERS so
// the file compiles immediately — that's why `make test` is RED right now. Your
// job is to turn it GREEN by wiring up real references and pointer logic.
//
//     make build         compile your code  (already works — start here)
//     make test          grade it           (RED until you fill these in)
//     make test-solution run grader on the reference if you get stuck
//
// KEY TERMS this file practices:
//   lvalue reference (&)   — another name for an existing object (notes 12.3)
//   pointer (*)            — a variable that stores an address   (notes 12.7)
//   nullptr                — the "points to nothing" constant    (notes 12.8)
//   dereference (*)        — reach the object through a pointer  (notes 12.7)
//   in/out parameter       — a reference the function reads AND writes (notes 12.13)
//   return by reference    — return an alias so the caller can write through it (12.12)
//   std::optional<T>       — a value that may or may not be present (notes 12.15)

#include "../aliases.h"

// ─── TASK 1: swapByRef — swap via NON-CONST LVALUE REFERENCES ─────────────────
// Declare a temporary int to hold one value during the exchange, then copy the
// two values across using the REFERENCES (which aliases the caller's variables).
// Nothing is returned — the modification IS the output (in/out parameters).
//
// Hint: you need a local int to hold the intermediate value.
//   int tmp { a };    // save a's current value
//   a = b;            // overwrite a with b's value
//   b = tmp;          // give b a's original value
//
//   >>> YOUR CODE HERE <<<
//
void swapByRef(int& /*a*/, int& /*b*/)
{
    // placeholder — does nothing (swap is a no-op)
}
// ─────────────────────────────────────────────────────────────────────────────

// ─── TASK 2: swapByPtr — swap via POINTERS, null-safe ─────────────────────────
// Same swap logic as Task 1, but the caller passes ADDRESSES (int*) instead of
// references. FIRST check that neither pointer is nullptr — if either is null,
// return immediately without touching anything. Then dereference with *a and *b
// to reach the ints, and swap just like Task 1 but writing through the pointers.
//
// Recall (notes 12.8):  `if (!a || !b) return;`   — short for nullptr check.
// Recall (notes 12.7):  `*a = 5`   writes 5 to the int that `a` points at.
//
//   >>> YOUR CODE HERE <<<
//
void swapByPtr(int* /*a*/, int* /*b*/)
{
    // placeholder — does nothing (null-safe no-op is actually correct for nullptr,
    // but the non-null case is wrong)
}
// ─────────────────────────────────────────────────────────────────────────────

// ─── TASK 3: maxOf — RETURN BY REFERENCE ──────────────────────────────────────
// Return a REFERENCE to the larger of the two parameters. The returned reference
// aliases the original variable in the caller — modifying the return value
// changes the caller's variable.
//
// TRAP WARNING (notes 12.12): do NOT return a reference to a local variable!
// A local dies when the function returns, leaving a dangling reference (UB).
// This function is safe because `a` and `b` are THEMSELVES references that
// alias the caller's objects — returning one of them returns an alias that
// outlives the call.
//
// If a >= b, return a reference to `a`; otherwise return a reference to `b`.
//
//   >>> YOUR CODE HERE <<<
//
int& maxOf(int& a, int& /*b*/)
{
    return a;   // placeholder — always returns a reference to 'a' (wrong when b > a)
}
// ─────────────────────────────────────────────────────────────────────────────

// ─── TASK 4: describePointer — read a CONST POINTER, label the state ──────────
// Inspect `ptr` (a pointer to const int). Check for nullptr first (notes 12.8).
// Return "null" if `ptr == nullptr`, otherwise return "value".
// Do NOT dereference a null pointer — that is undefined behavior.
//
// Returning a string literal as string_view is safe: string literals have
// static storage duration and never expire.
//
//   >>> YOUR CODE HERE <<<
//
std::string_view describePointer(const int* /*ptr*/)
{
    return "value";   // placeholder — never reports "null" (wrong for nullptr)
}
// ─────────────────────────────────────────────────────────────────────────────

// ─── TASK 5: addBonusInPlace — textbook IN/OUT PARAMETER ──────────────────────
// Add `bonus` to `score` in place. `score` is the non-const reference (in/out —
// the function both reads and writes it). `bonus` is passed by value (in only).
//
// CS6340 parallel: like incrementing a counter inside a pass without returning
// a new value — the reference IS the output channel.
//
//   >>> YOUR CODE HERE <<<
//
void addBonusInPlace(int& /*score*/, int /*bonus*/)
{
    // placeholder — does nothing (score unchanged)
}
// ─────────────────────────────────────────────────────────────────────────────

// ─── TASK 6: findFirst — std::optional SEARCH ─────────────────────────────────
// Walk `text` character by character with a `for` loop. Compare each character
// to `target`. If they match, return the 0-based index (as std::optional<int>).
// If the loop finishes without a match, return std::nullopt.
//
// Important: when indexing with an `int` index, cast to `std::size_t` to keep
// -Wall -Wextra warning-clean:
//     text[static_cast<std::size_t>(i)]
//
// std::optional recap (notes 12.15):
//     return i;             // wraps the value in an optional
//     return std::nullopt;  // the "no value" sentinel
//     if (result)           // true if the optional has a value
//     *result               // access the value (dereference the optional)
//
//   >>> YOUR CODE HERE <<<
//
std::optional<int> findFirst(std::string_view /*text*/, char /*target*/)
{
    return std::nullopt;   // placeholder — always reports "not found" (wrong when target is present)
}
// ─────────────────────────────────────────────────────────────────────────────
