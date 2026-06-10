// ============================================================================
// Chapter 5 — Constants and Strings · Project: Name-Badge / Greeting Formatter
// tests/tests.cpp  —  the automated grader (tiny no-framework harness).
// ============================================================================
//
// Linked against starter/badge.cpp by `make test`, and against solution/badge.cpp
// by `make test-solution`. Each CHECK is one assertion; any failure prints the
// expression + line and the run ends non-zero (RED). All pass -> "PASS" (GREEN).
//
// Coverage: a normal case for every function, PLUS edge cases — empty names and
// single-character names — because .front() on an empty view is undefined
// behavior and the length/initials logic must hold at the boundaries.

#include <iostream>
#include <string>
#include "../badge.h"

static int fails = 0;

// Print the failing expression and source line; tally the failure.
#define CHECK(cond) do { \
    if(!(cond)) { std::cerr << "FAIL: " #cond "  @line " << __LINE__ << "\n"; ++fails; } \
} while(0)

int main()
{
    // ── TASK 1: badgeHeader() returns the configured org name ───────────────
    CHECK(badgeHeader() == "Cpp Foundations Lab");
    CHECK(badgeHeader() == std::string { kOrgName });  // stays in sync with the constant

    // ── TASK 2: fullName() -> "First Last" ──────────────────────────────────
    CHECK(fullName("Ada", "Lovelace") == "Ada Lovelace");
    CHECK(fullName("Grace", "Hopper") == "Grace Hopper");
    CHECK(fullName("A", "B") == "A B");                 // single-char edge case

    // ── TASK 3: greeting() -> "Hello, my name is First Last" (reuses fullName)
    CHECK(greeting("Ada", "Lovelace") == "Hello, my name is Ada Lovelace");
    CHECK(greeting("Grace", "Hopper") == "Hello, my name is Grace Hopper");

    // ── TASK 4: username() -> "first.last" using kUsernameSep ───────────────
    CHECK(username("Ada", "Lovelace") == "Ada.Lovelace");
    CHECK(username("Grace", "Hopper") == "Grace.Hopper");
    // The separator must come from the named constant, not a hard-coded '.':
    CHECK(username("x", "y") == std::string { "x" } + kUsernameSep + "y");

    // ── TASK 5: initials() -> "F.L." with the empty-name guard ──────────────
    CHECK(initials("Ada", "Lovelace") == "A.L.");
    CHECK(initials("Grace", "Hopper") == "G.H.");
    CHECK(initials("", "Lovelace") == "L.");            // empty first name
    CHECK(initials("Ada", "") == "A.");                 // empty last name
    CHECK(initials("", "") == "");                      // both empty -> no crash, no letters
    CHECK(initials("a", "b") == "a.b.");                // single-char names

    // ── TASK 6: nameLength() -> length of "First Last" via .size() ──────────
    CHECK(nameLength("Ada", "Lovelace") == 12);         // "Ada Lovelace"
    CHECK(nameLength("Grace", "Hopper") == 12);         // "Grace Hopper"
    CHECK(nameLength("A", "B") == 3);                   // "A B"
    CHECK(nameLength("", "") == 1);                     // " " (just the joining space)

    if (!fails) std::cout << "PASS ✅  all checks\n";
    else        std::cerr << "FAIL ❌  " << fails << " check(s) failed — fill in the TODO blocks.\n";
    return fails ? 1 : 0;
}
