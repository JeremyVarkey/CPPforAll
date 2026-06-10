// ============================================================================
//  toolkit.h  —  the generic toolkit mini-library  (Chapter 11 STARTER)
// ----------------------------------------------------------------------------
//  This is the file you edit. It combines DECLARATIONS and DEFINITIONS in one
//  header. That is unusual — normally you split declarations into .h and bodies
//  into .cpp (Ch 2). Templates are the exception: the compiler must see the
//  full template body at every call site to generate the needed function, so
//  templates LIVE in headers (notes 11.10).
//
//  The non-template functions (TASK 1–4, TASK 9) are also inline here so
//  that the same grader binary works for both starter and solution without
//  requiring a separate link step (Style B2 — same pattern as Ch 14).
//
//  THREE Chapter-11 ideas are on display:
//
//    1) OVERLOADED FUNCTIONS — four describe() functions share one name but
//       differ by parameter type.  The compiler picks the right one based only
//       on the argument type at the call site (notes 11.1, 11.2). Each overload
//       returns a LABELED string so you can see which one the compiler chose.
//
//    2) FUNCTION TEMPLATES — myMin<T>, myClamp<T>, and scaled<T,U> are written
//       once and work for any type that supports the required operators.  No
//       copy-pasting the algorithm for each type (notes 11.6, 11.7, 11.8).
//
//    3) NON-TYPE TEMPLATE PARAMETER — repeatChar<N>() bakes the repetition
//       count into the type itself, giving a compile-time constant (notes 11.9).
//
//  CS6340 / LLVM lens: LLVM uses all three patterns.  Overloads let one name
//  handle Function, BasicBlock, and Instruction; templates power SmallVector<T>
//  and ArrayRef<T>; non-type params appear in PointerIntPair<T,N>.
//
//  Header guard (Ch 2): stops this file being pasted in twice per build.
// ============================================================================

#ifndef TOOLKIT_H
#define TOOLKIT_H

#include <string_view>   // std::string_view — cheap read-only text view (Ch 5)

// ============================================================================
// PART A — OVERLOADED describe() FUNCTIONS  (TASK 1 – 4)
// ============================================================================
//
// Four overloads of describe() differentiated by parameter type (notes 11.2).
// Each returns a short labeled string_view, e.g. describe(42) -> "int:42".
// The label makes OVERLOAD RESOLUTION visible: if the wrong overload fires,
// the test fails with a mismatch you can read.
//
// All four bodies are inline in this header (the Style B2 pattern) — that lets
// the Makefile swap starter/ vs solution/ with just a change in -I flags.

// ─── TASK 1: describe(int) ───────────────────────────────────────────────────
// Return "int:<value>" for each expected input.
// The grader passes: 0, 1, -1, 42, -99, 100.
// Use a switch on `value`. Return "int:?" for any other value.
//
// Why a switch over pre-canned values rather than std::to_string?
//   Templates and overloads are the focus here, not string formatting (Ch 28).
//   Hard-coded labels keep the tests crisp and eliminate formatting surprises.
//
// ─── TASK 1: replace the placeholder body ───────────────────────────────────
inline std::string_view describe(int value)
{
    (void)value;       // suppress unused-parameter warning on the placeholder
    return "int:?";   // placeholder — always wrong; fill in a switch statement
    //   >>> YOUR CODE HERE <<<
}
// ─────────────────────────────────────────────────────────────────────────────

// ─── TASK 2: describe(double) ────────────────────────────────────────────────
// Return "double:<value>" for each expected input.
// The grader passes: 0.0, 1.0, -1.0, 3.14, 2.5, -0.5, 99.9.
// Use an if / else if chain. Return "double:?" for any other value.
//
// ─── TASK 2: replace the placeholder body ───────────────────────────────────
inline std::string_view describe(double value)
{
    (void)value;
    return "double:?";  // placeholder
    //   >>> YOUR CODE HERE <<<
}
// ─────────────────────────────────────────────────────────────────────────────

// ─── TASK 3: describe(bool) ──────────────────────────────────────────────────
// Return "bool:true" or "bool:false".
//
// KEY CONCEPT — overload resolution and promotions (notes 11.3):
// Without this exact overload, describe(true) would call describe(int) because
// bool PROMOTES to int (step 2 of resolution). By providing an exact bool
// overload the compiler stops at step 1 (exact match). The test checks that
// describe(true) returns "bool:true", not "int:1" — that difference is the
// whole lesson made visible.
//
// ─── TASK 3: replace the placeholder body ───────────────────────────────────
inline std::string_view describe(bool value)
{
    (void)value;
    return "bool:?";   // placeholder — wrong; use a ternary or if/else
    //   >>> YOUR CODE HERE <<<
}
// ─────────────────────────────────────────────────────────────────────────────

// ─── TASK 4: describe(std::string_view) ──────────────────────────────────────
// Return "str:<value>" for each expected input.
// The grader passes "hello", "LLVM", and "" (empty).
// Use if / else if chains comparing value == "hello", etc.
//
// ─── TASK 4: replace the placeholder body ───────────────────────────────────
inline std::string_view describe(std::string_view value)
{
    (void)value;
    return "str:?";    // placeholder
    //   >>> YOUR CODE HERE <<<
}
// ─────────────────────────────────────────────────────────────────────────────

// ─── NOTE — = delete to block unwanted conversions (notes 11.4) ──────────────
// Uncomment the line below to see describe(char) become a compile error instead
// of silently calling describe(int) via char->int promotion. The grader cannot
// test compile errors, so this is a teaching note, not an active check.
//
//     inline void describe(char) = delete;  // try: describe('x'); -> error

// ============================================================================
// PART B — FUNCTION TEMPLATE DEFINITIONS  (TASK 5 – 8)
// ============================================================================
//
// Templates MUST be defined in headers so every translation unit that calls
// them can instantiate the needed function (notes 11.10). Edit the stub bodies
// below — they compile but return WRONG values until you fill them in.

// ─── TASK 5: myMin<T> ────────────────────────────────────────────────────────
// Return the smaller of two same-type values.
// Works for any T that supports operator<.
//
// DEDUCTION: myMin(3, 5) deduces T=int automatically.
// EXPLICIT:  myMin<double>(3, 4.5) forces T=double so int 3 -> 3.0 (notes 11.7).
//
// If you pass an int and a double without an explicit type, the compiler cannot
// deduce a single T from conflicting types — see notes 11.7.
//
// ─── TASK 5: replace the placeholder body ───────────────────────────────────
template <typename T>
T myMin(T a, T b)
{
    (void)b;       // suppress unused-parameter warning on placeholder
    return a;      // placeholder — always returns the first argument
    //   >>> YOUR CODE HERE <<<
}
// ─────────────────────────────────────────────────────────────────────────────

// ─── TASK 6: myClamp<T> ──────────────────────────────────────────────────────
// Clamp `value` into the closed interval [lo, hi]:
//   value < lo  ->  return lo
//   value > hi  ->  return hi
//   otherwise   ->  return value (unchanged)
// Works for any T with operator<. Use two branches (if / else if / else).
//
// ─── TASK 6: replace the placeholder body ───────────────────────────────────
template <typename T>
T myClamp(T value, T lo, T hi)
{
    (void)lo; (void)hi;   // suppress unused-parameter warnings on placeholder
    return value;          // placeholder — unclamped (wrong at boundaries)
    //   >>> YOUR CODE HERE <<<
}
// ─────────────────────────────────────────────────────────────────────────────

// ─── TASK 7: repeatChar<N> ───────────────────────────────────────────────────
// Return a string_view of N identical copies of `ch`.
// N is a NON-TYPE template parameter (int) — a compile-time constant (notes 11.9).
//
// Implementation:
//   Use `static char buf[N + 1]` — a static array sized at COMPILE TIME.
//   Each instantiation (repeatChar<3>, repeatChar<5>, …) gets its OWN static
//   buffer (notes 11.7: "static locals are per instantiation"), so repeated
//   calls for different N don't clobber each other. Fill all N slots with ch,
//   null-terminate with buf[N] = '\0', return string_view{buf, N}.
//
// Constraint: 1 <= N <= 32 (keep the array small).
//
// ─── TASK 7: replace the placeholder body ───────────────────────────────────
template <int N>
std::string_view repeatChar(char /*ch*/)
{
    // Placeholder: a zero-filled static buffer. The returned view has N chars
    // but they are all '\0', so every string comparison will fail.
    static char buf[N + 1]{};
    return std::string_view{buf, N};
    //   >>> YOUR CODE HERE <<<
}
// ─────────────────────────────────────────────────────────────────────────────

// ─── TASK 8: scaled<T,U> ─────────────────────────────────────────────────────
// Multiply two (possibly different) numeric types and return the result.
//
// Use TWO type parameters (T, U) so int and double can be mixed (notes 11.8).
// Use `auto` as the return type so the compiler deduces the result type from
// the expression `a * b` — e.g. int * double -> double (notes 11.8).
//
// NOTE: abbreviated auto parameters (C++20 `auto add(auto a, auto b)`) are
// FORBIDDEN — use explicit `template <typename T, typename U>` syntax instead.
//
// ─── TASK 8: replace the placeholder body ───────────────────────────────────
template <typename T, typename U>
auto scaled(T /*a*/, U /*b*/)
{
    return T{};   // placeholder — always returns a zero-value T
    //   >>> YOUR CODE HERE <<<
}
// ─────────────────────────────────────────────────────────────────────────────

// ============================================================================
// PART C — DEFAULT ARGUMENT  (TASK 9)
// ============================================================================

// ─── TASK 9: formatCount ─────────────────────────────────────────────────────
// Return a labeled count string.  The `suffix` parameter has a DEFAULT
// argument of "s" — declared HERE in the header so every caller can see it
// (notes 11.5: "put defaults in declarations, not repeated definitions").
//
// Logic:
//   count == 1  ->  "1 <label>"           (singular — no suffix)
//   count != 1  ->  "<count> <label><suffix>"  (plural — append suffix)
//
// Examples:
//   formatCount("item",  3)         -> "3 items"    (default suffix "s")
//   formatCount("error", 1)         -> "1 error"    (singular, no suffix)
//   formatCount("pass",  2, "es")   -> "2 passes"   (explicit suffix)
//
// Implementation: use two static char buffers (64 bytes each) — one for the
// singular branch, one for the plural branch — so the returned string_view
// stays valid after the function returns.
//
//   Build strings MANUALLY: copy label chars, append suffix, null-terminate.
//   (std::to_string arrives cleanly in Ch 5; manual char copy is fine here.)
//   Counts 0–99 are all the grader uses.
//
// ─── TASK 9: replace the placeholder body ───────────────────────────────────
inline std::string_view formatCount(std::string_view /*label*/, int /*count*/,
                                     std::string_view /*suffix*/ = "s")
{
    return "?";   // placeholder — wrong for all inputs
    //   >>> YOUR CODE HERE <<<
}
// ─────────────────────────────────────────────────────────────────────────────

#endif // TOOLKIT_H
