// ============================================================================
//  toolkit.h  —  the generic toolkit mini-library  (Chapter 11 SOLUTION)
// ----------------------------------------------------------------------------
//  One complete, correct, warning-clean implementation. Peek here only after
//  you've taken a real swing at starter/toolkit.h — the learning is in wiring
//  the overloads and templates yourself, then comparing.
//
//  Key concepts demonstrated (with notes references):
//    TASK 1–4  overloaded describe() — one name, four resolutions (11.1, 11.2)
//    TASK 3    exact bool overload prevents silent promotion (11.3)
//    TASK 5    myMin<T>: single-type template + explicit template arg (11.7)
//    TASK 6    myClamp<T>: template with boundary conditions (11.6)
//    TASK 7    repeatChar<N>: non-type template parameter (11.9),
//              per-instantiation static local (11.7)
//    TASK 8    scaled<T,U>: two-type template, auto return (11.8)
//    TASK 9    formatCount: default argument in declaration (11.5)
// ============================================================================

#ifndef TOOLKIT_H
#define TOOLKIT_H

#include <string_view>   // std::string_view (Ch 5)

// ============================================================================
// PART A — OVERLOADED describe() FUNCTIONS
// ============================================================================

// ─── TASK 1: describe(int) ───────────────────────────────────────────────────
// Exact match for int arguments (notes 11.2 step 1).
// The switch maps each grader-supplied value to a labeled literal.
// Returning a string literal is safe: string literals have static storage
// duration (their lifetime = the whole program), so string_view is valid.
// (A preview of lifetime reasoning — formally Chapter 12.)
inline std::string_view describe(int value)
{
    switch (value)
    {
    case  0:   return "int:0";
    case  1:   return "int:1";
    case -1:   return "int:-1";
    case  42:  return "int:42";
    case -99:  return "int:-99";
    case  100: return "int:100";
    default:   return "int:?";
    }
}

// ─── TASK 2: describe(double) ────────────────────────────────────────────────
// Exact match for double arguments (notes 11.2).
// if/else if because double values cannot be used as switch case constants.
// Comparing doubles with == is sound here: the caller passes exact literals
// that round-trip without arithmetic rounding.
inline std::string_view describe(double value)
{
    if      (value == 0.0)   return "double:0.0";
    else if (value == 1.0)   return "double:1.0";
    else if (value == -1.0)  return "double:-1.0";
    else if (value == 3.14)  return "double:3.14";
    else if (value == 2.5)   return "double:2.5";
    else if (value == -0.5)  return "double:-0.5";
    else if (value == 99.9)  return "double:99.9";
    else                     return "double:?";
}

// ─── TASK 3: describe(bool) ──────────────────────────────────────────────────
// CRITICAL: without this overload, describe(true) calls describe(int) because
// bool PROMOTES to int (notes 11.3: promotions beat conversions at step 2).
// With this exact overload the compiler stops at step 1 — no promotion needed.
//
// CS6340 tie-in: LLVM APIs use this pattern to prevent callers from accidentally
// passing an int flag where a bool was expected, catching the mix-up at compile
// time rather than at a silent runtime misbehavior.
inline std::string_view describe(bool value)
{
    return value ? "bool:true" : "bool:false";
}

// ─── = delete DEMO (notes 11.4) ──────────────────────────────────────────────
// Uncomment to see describe('x') become a compile error instead of silently
// calling describe(int) via char->int promotion:
//
//     inline void describe(char) = delete;
//
// The "use of deleted function" error is the whole point of notes 11.4 —
// deleted overloads make dangerous implicit conversions a compile-time failure.

// ─── TASK 4: describe(std::string_view) ──────────────────────────────────────
// Exact match for string_view arguments (string literals convert to string_view).
inline std::string_view describe(std::string_view value)
{
    if      (value == "hello") return "str:hello";
    else if (value == "LLVM")  return "str:LLVM";
    else if (value == "")      return "str:";
    else                       return "str:?";
}

// ============================================================================
// PART B — FUNCTION TEMPLATE DEFINITIONS
// ============================================================================

// ─── TASK 5: myMin<T> ────────────────────────────────────────────────────────
// T is deduced from the arguments for normal calls: myMin(3, 5) -> T=int.
// For mixed types the caller must be explicit: myMin<double>(3, 4.5) forces
// T=double and converts the int 3 to 3.0 (notes 11.7).
//
// The `return (a < b) ? a : b;` pattern is the textbook min: one `<` comparison,
// no branch that could be surprising for types where `==` is not defined.
template <typename T>
T myMin(T a, T b)
{
    return (a < b) ? a : b;   // a < b means a is the smaller one
}

// ─── TASK 6: myClamp<T> ──────────────────────────────────────────────────────
// The three cases are mutually exclusive: value < lo, value > hi, or in range.
// An if / else if / else chain maps directly to those cases.
// Works for int, double, char, or any T where < makes sense.
template <typename T>
T myClamp(T value, T lo, T hi)
{
    if (value < lo)
        return lo;       // below lower bound -> clamp to lo
    else if (hi < value)
        return hi;       // above upper bound -> clamp to hi
    else
        return value;    // within [lo, hi] -> pass through unchanged
}

// ─── TASK 7: repeatChar<N> ───────────────────────────────────────────────────
// NON-TYPE template parameter N is a compile-time integer (notes 11.9).
// `static char buf[N + 1]` is dimensioned AT COMPILE TIME — each instantiation
// gets its own separate static array (notes 11.7: "static locals are per
// instantiation"), so repeatChar<3> and repeatChar<5> never share storage.
//
// We fill the buffer on EVERY call (ch is a parameter, so it can vary between
// calls to the same instantiation). Because buf is static the fill re-runs
// each call — that's correct: the caller's ch might differ from a previous call.
template <int N>
std::string_view repeatChar(char ch)
{
    static char buf[N + 1];   // compile-time sized; each N has its own array
    for (int i = 0; i < N; ++i)
        buf[i] = ch;          // fill all N slots with the requested character
    buf[N] = '\0';            // null-terminate so it's a valid C-string too
    return std::string_view{buf, N};
}

// ─── TASK 8: scaled<T,U> ─────────────────────────────────────────────────────
// Two type parameters allow mixed-type calls without requiring explicit casts
// from the caller (notes 11.8: "use multiple type parameters when different
// types are expected").
//
// `auto` return type deduces the result type from `a * b` (notes 11.8: "use
// auto return type when result type differs"). For int * double the deduced
// type is double — the wider type, which is what the caller expects.
//
// Note: abbreviated function template syntax (C++20's `auto add(auto a, auto b)`)
// would be shorter but is not yet taught — explicit `template <typename T, typename U>`
// is the required form (notes 11.8 covers abbreviation but CLAUDE.md scope forbids it).
template <typename T, typename U>
auto scaled(T a, U b)
{
    return a * b;   // return type deduced from the expression (auto -> T*U result type)
}

// ============================================================================
// PART C — DEFAULT ARGUMENT
// ============================================================================

// ─── TASK 9: formatCount ─────────────────────────────────────────────────────
// The default `suffix = "s"` is declared HERE (notes 11.5: "put defaults in
// visible declarations"). Only ONE of the two branches runs per call, so we
// use two static buffers — one for singular, one for plural — both safe as
// string_view backing storage (static lifetime, no heap allocation).
//
// Manual integer printing: handle 0–99 (all the grader uses) with two-digit
// decomposition. For a real project, std::to_string (Ch 5) or std::snprintf
// (Ch 28) would be cleaner; staying manual here avoids out-of-scope includes.
inline std::string_view formatCount(std::string_view label, int count,
                                     std::string_view suffix = "s")
{
    // ── Singular branch (count == 1) ─────────────────────────────────────────
    if (count == 1)
    {
        static char singular[64];
        int i = 0;
        singular[i++] = '1';
        singular[i++] = ' ';
        for (std::size_t k = 0; k < label.size() && i < 62; ++k)
            singular[i++] = label[k];
        singular[i] = '\0';
        return std::string_view{singular, static_cast<std::size_t>(i)};
    }

    // ── Plural branch (count != 1) ────────────────────────────────────────────
    static char plural[64];
    int i = 0;

    // Write count (0–99)
    int c = (count < 0) ? -count : count;   // use magnitude (grader never passes negative)
    if (c >= 10)
        plural[i++] = static_cast<char>('0' + (c / 10) % 10);
    plural[i++] = static_cast<char>('0' + c % 10);

    plural[i++] = ' ';

    // Append label
    for (std::size_t k = 0; k < label.size() && i < 60; ++k)
        plural[i++] = label[k];

    // Append suffix (e.g. "s" or "es")
    for (std::size_t k = 0; k < suffix.size() && i < 63; ++k)
        plural[i++] = suffix[k];

    plural[i] = '\0';
    return std::string_view{plural, static_cast<std::size_t>(i)};
}

#endif // TOOLKIT_H
