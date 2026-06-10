// ============================================================================
// Chapter 5 — Constants and Strings · Project: Name-Badge / Greeting Formatter
// starter/badge.cpp  —  YOU EDIT THIS FILE.
// ============================================================================
//
// Fill in the six >>> YOUR CODE HERE <<< blocks. Each maps 1:1 to a TASK in the
// README. The function signatures and the named constants come from ../badge.h
// (already complete) — read it first; it explains WHY each type was chosen.
//
//   make build   compile this file (it already compiles — stubs return junk)
//   make test    grade YOUR code  ->  RED until you finish, then GREEN
//   make solution / make test-solution   the reference, if you get stuck
//
// Right now every function returns a placeholder, so `make build` succeeds but
// `make test` FAILS. Turning that red into green is the exercise.
//
// REMINDERS for this chapter:
//   * std::string_view params are read-only windows — cheap to pass, no copy.
//   * Build text by CONCATENATING with + (or +=) into a std::string you return.
//   * A std::string_view can be appended to a std::string directly.
//   * .front() reads the first char; .size() gives the length (UNSIGNED).
//   * NO loops — every task is a fixed bit of string assembly.

#include "../badge.h"     // the contract: signatures + the kOrg/kUsername/kBadge constants

// ─── TASK 1: return the organization name ───────────────────────────────────
// Return the kOrgName constant from badge.h. It is a std::string_view; this
// function returns a std::string, so the view is copied into an owning string
// on the way out (that is exactly what the caller wants — its own text).
//
//   >>> YOUR CODE HERE <<<
//
// ────────────────────────────────────────────────────────────────────────────
std::string badgeHeader()
{
    return {};   // STUB: empty string. Replace with the real header.
}

// ─── TASK 2: build "First Last" ─────────────────────────────────────────────
// Concatenate first, a single space " ", and last into one std::string.
// Hint: std::string result { first }; then result += " "; then result += last;
//       (a std::string_view appends straight onto a std::string with +=).
//
//   >>> YOUR CODE HERE <<<
//
// ────────────────────────────────────────────────────────────────────────────
std::string fullName(std::string_view first, std::string_view last)
{
    (void)first;        // STUB: silence "unused parameter" until you use them.
    (void)last;
    return {};          // replace with "First Last".
}

// ─── TASK 3: build "Hello, my name is First Last" (REUSE fullName) ──────────
// Don't re-assemble the name by hand — call fullName(first, last) and build the
// greeting around it. Composing small functions is the point.
//
//   >>> YOUR CODE HERE <<<
//
// ────────────────────────────────────────────────────────────────────────────
std::string greeting(std::string_view first, std::string_view last)
{
    (void)first;
    (void)last;
    return {};          // replace with "Hello, my name is First Last".
}

// ─── TASK 4: build "first<sep>last" using kUsernameSep ──────────────────────
// Join first and last with the kUsernameSep constant ('.') from badge.h —
// do NOT hard-code a '.' here; use the named constant so the separator lives in
// one place. Result for ("Ada","Lovelace") is "Ada.Lovelace".
//
//   >>> YOUR CODE HERE <<<
//
// ────────────────────────────────────────────────────────────────────────────
std::string username(std::string_view first, std::string_view last)
{
    (void)first;
    (void)last;
    return {};          // replace with "first.last".
}

// ─── TASK 5: build initials "F.L." with an EMPTY-name guard ─────────────────
// For each name: IF it is non-empty (.size() > 0), append its .front() character
// followed by kUsernameSep. If it is empty, append nothing for it (do NOT call
// .front() on an empty view — that is undefined behavior). Use an `if` guard.
//   ("Ada","Lovelace") -> "A.L."   ("","Lovelace") -> "L."   ("","") -> ""
//
//   >>> YOUR CODE HERE <<<
//
// ────────────────────────────────────────────────────────────────────────────
std::string initials(std::string_view first, std::string_view last)
{
    (void)first;
    (void)last;
    return {};          // replace with the initials string.
}

// ─── TASK 6: report the length of the full name with .size() ────────────────
// Build the full name (reuse fullName!), then return its .size(). Because
// .size() is UNSIGNED (std::size_t) and this returns int, convert it:
//   return static_cast<int>(theName.size());
//
//   >>> YOUR CODE HERE <<<
//
// ────────────────────────────────────────────────────────────────────────────
int nameLength(std::string_view first, std::string_view last)
{
    (void)first;
    (void)last;
    return 0;           // replace with the real length.
}
