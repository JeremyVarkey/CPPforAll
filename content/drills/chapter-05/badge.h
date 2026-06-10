// ============================================================================
// Chapter 5 — Constants and Strings · Project: Name-Badge / Greeting Formatter
// badge.h  —  the API you implement.  DECLARATIONS ONLY (this file is complete).
// ============================================================================
//
// You fill in the function *bodies* in starter/badge.cpp. This header is the
// CONTRACT: it fixes each function's name, parameters, and return type. The
// grader (tests/tests.cpp) and the reference solution both #include this file,
// so do NOT change it — change starter/badge.cpp.
//
// Read the parameter and return TYPES carefully — they are the whole lesson:
//
//   * Read-only inputs are std::string_view.  A view is a cheap (pointer+length)
//     READ-ONLY window onto characters that already live somewhere else. It does
//     NOT own or copy them, so it is the right type for a parameter a function
//     only reads. It happily accepts a C-style literal, a std::string, or
//     another std::string_view — no copy required.  (notes 5.8)
//
//   * Functions that BUILD text return std::string (by value). A std::string
//     OWNS its characters and can grow as you concatenate, so it is the right
//     type for text you assemble and hand back. Returning it by value is normal
//     and efficient in modern C++.  (notes 5.7)
//
//   * The org name is a constexpr std::string_view — a compile-time string
//     symbolic constant. That is the Chapter 5 sweet spot: constexpr gives a
//     named compile-time constant, string_view avoids owning/copying, and the
//     "..." literal storage lives for the whole program.  (notes 5.6, 5.8)
//
// SCOPE: const / constexpr, std::string, std::string_view, functions, and `if`.
// No loops (those are Chapter 8) — every task here is solved by BUILDING strings
// (concatenate with +, peek a character with .front(), measure with .size()),
// never by scanning character-by-character.

#ifndef BADGE_H            // header guard: include this file safely many times
#define BADGE_H

#include <string>         // std::string — owning, growable text
#include <string_view>    // std::string_view — cheap read-only window

// ─── Compile-time configuration (named constants, not magic values) ─────────
// These are constexpr: their values are fixed and known AT COMPILE TIME, so the
// compiler can fold them straight into the generated code. Naming them is the
// whole point of notes 5.1 / 5.2 — a reader sees MEANING instead of a bare "."
// or "27" floating in the logic, and a future change happens in exactly one
// place. You will USE these in your function bodies instead of re-typing literals.
// (constexpr constants are safe to define in a header: each translation unit gets
//  its own private copy. Chapter 7 later introduces `inline constexpr` for sharing
//  ONE entity program-wide — not needed here.)

// The organization printed across the top of every badge.
constexpr std::string_view kOrgName { "Cpp Foundations Lab" };

// The character that joins a username, e.g. "ada.lovelace".
constexpr char kUsernameSep { '.' };

// The total width of a framed badge line. 27 columns is a layout decision, not
// a magic number — so it gets a name. None of the six required tasks need it; it
// is provided for the optional padBadge() stretch goal in the README.
constexpr int kBadgeWidth { 27 };

// ─── The API you implement (bodies live in starter/badge.cpp) ───────────────

// TASK 1 — Return the organization name (the constant kOrgName above).
// Note the conversion: kOrgName is a std::string_view but this returns a
// std::string. A view does NOT implicitly become an owning string — you make
// the copy explicit (std::string { kOrgName }) because the caller wants its
// own copy of the header text.
std::string badgeHeader();

// TASK 2 — Build a person's full name as "First Last" (one space between).
// Example: fullName("Ada", "Lovelace")  ->  "Ada Lovelace"
std::string fullName(std::string_view first, std::string_view last);

// TASK 3 — Build a friendly greeting that REUSES fullName():
//   greeting("Ada", "Lovelace")  ->  "Hello, my name is Ada Lovelace"
std::string greeting(std::string_view first, std::string_view last);

// TASK 4 — Build a username "first<sep>last" using the kUsernameSep constant:
//   username("Ada", "Lovelace")  ->  "Ada.Lovelace"
// (No case-changing — that needs a loop, which is Chapter 8. Just join them.)
std::string username(std::string_view first, std::string_view last);

// TASK 5 — Build initials from the FIRST character of each name, each followed
// by kUsernameSep:   initials("Ada", "Lovelace")  ->  "A.L."
// EDGE CASE: calling .front() on an EMPTY view is undefined behavior. If a name
// is empty (.size() == 0), contribute no letter for it (but still no crash).
//   initials("", "Lovelace")  ->  "L."
//   initials("Ada", "")       ->  "A."
//   initials("", "")          ->  ""
std::string initials(std::string_view first, std::string_view last);

// TASK 6 — Report the length of the full name as a plain int, using .size().
// .size() returns an UNSIGNED type (std::size_t); convert it to int for the
// return. Example: nameLength("Ada", "Lovelace") -> 12  ("Ada Lovelace" is 12).
int nameLength(std::string_view first, std::string_view last);

#endif // BADGE_H
