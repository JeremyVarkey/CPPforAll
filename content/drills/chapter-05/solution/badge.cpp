// ============================================================================
// Chapter 5 — Constants and Strings · Project: Name-Badge / Greeting Formatter
// solution/badge.cpp  —  REFERENCE SOLUTION (complete, correct, commented).
// ============================================================================
//
// One clean way to satisfy every test. Peek only after you've tried — the
// learning is in getting YOUR starter/badge.cpp to pass `make test` first.
//
// Throughline of the chapter, visible in the types below:
//   * Every read-only input is a std::string_view (cheap window, no copy).
//   * Every function that produces text returns an OWNING std::string.
//   * The separator, org name, and width are named constexpr constants from
//     ../badge.h — never re-typed as bare literals in the logic.
//   * No loops: each result is a short, fixed piece of string assembly.

#include "../badge.h"

// TASK 1 — Return the org-name constant.
// kOrgName is a std::string_view (a view onto the long-lived "..." literal).
// This function's return type is std::string, so we construct an owning string
// FROM the view: std::string { kOrgName }. That copy is intentional — the
// caller asked for its own header text. (A view -> string conversion must be
// explicit when narrowing ownership, which is exactly what the braces do.)
std::string badgeHeader()
{
    return std::string { kOrgName };
}

// TASK 2 — "First Last".
// Seed an owning std::string from the first view, then append a space and the
// last view with +=. A std::string_view appends directly onto a std::string,
// so no manual character work (and no loop) is needed.
std::string fullName(std::string_view first, std::string_view last)
{
    std::string result { first };   // owning copy of the first name
    result += ' ';                  // single separating space
    result += last;                 // append the last-name view
    return result;
}

// TASK 3 — "Hello, my name is First Last".
// Compose: reuse fullName() instead of re-joining the two names by hand. Build
// the fixed prefix as a std::string, then append the assembled name.
std::string greeting(std::string_view first, std::string_view last)
{
    std::string result { "Hello, my name is " };
    result += fullName(first, last);   // fullName returns a std::string; append it
    return result;
}

// TASK 4 — "first<sep>last".
// Join with the NAMED separator kUsernameSep ('.') from badge.h — not a hard
// coded '.'. If the separator ever changes, this code does not.
std::string username(std::string_view first, std::string_view last)
{
    std::string result { first };
    result += kUsernameSep;   // a char appends to a std::string with +=
    result += last;
    return result;
}

// TASK 5 — initials "F.L." with the empty-name guard.
// .front() on an EMPTY view is undefined behavior, so each contribution is
// guarded by an `if`. A non-empty name has .size() > 0 and adds
// "<first-char><sep>"; an empty one adds nothing.
// ("Ada","Lovelace") -> "A.L."   ("","Lovelace") -> "L."   ("","") -> ""
std::string initials(std::string_view first, std::string_view last)
{
    std::string result {};            // start empty; build up what we can

    if (first.size() > 0)             // guard: never .front() an empty view
    {
        result += first.front();      // first character of the first name
        result += kUsernameSep;
    }
    if (last.size() > 0)
    {
        result += last.front();
        result += kUsernameSep;
    }
    return result;
}

// TASK 6 — length of the full name via .size().
// Reuse fullName(), then measure it. .size() returns std::size_t (UNSIGNED);
// the contract returns int, so convert explicitly with static_cast to make the
// signed/unsigned narrowing deliberate and warning-free.
int nameLength(std::string_view first, std::string_view last)
{
    const std::string name { fullName(first, last) };
    return static_cast<int>(name.size());
}
