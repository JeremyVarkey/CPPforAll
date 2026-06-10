// Chapter 12 — Compound Types: References and Pointers · Alias Workshop  (GRADER)
// ─────────────────────────────────────────────────────────────────────────────
// A tiny no-framework unit-test harness (drills/CLAUDE.md Style B).
// Includes ../aliases.h and calls each function across many inputs.
//
// WHAT MAKES THESE TESTS MEANINGFUL:
//   Every ref/pointer test does the physical check the chapter demands — it
//   verifies that the ORIGINAL caller-side variable changed, not just a copy.
//   The aliasing is PHYSICAL: we confirm it by inspecting the originals after
//   each call, and for maxOf we compare addresses to prove the reference aliases
//   the correct variable (not a copy with the same value).
//
// `make test`          -> links against starter/aliases.cpp  (RED on stub code)
// `make test-solution` -> links against solution/aliases.cpp (must be GREEN)

#include <iostream>
#include <string_view>
#include <optional>
#include "../aliases.h"

static int fails = 0;

// CHECK: evaluate cond; on failure, report the expression and line number.
#define CHECK(cond) \
    do { if(!(cond)){ std::cerr << "FAIL: " #cond "  @line " << __LINE__ << "\n"; ++fails; } } while(0)

int main()
{
    // ── Task 1: swapByRef — aliasing proved by checking the ORIGINALS ─────────
    {
        int x { 10 };
        int y { 20 };
        swapByRef(x, y);
        CHECK(x == 20);    // x must have received y's old value
        CHECK(y == 10);    // y must have received x's old value
    }
    {
        // Edge: both values equal — swap should leave both unchanged
        int a { 5 };
        int b { 5 };
        swapByRef(a, b);
        CHECK(a == 5);
        CHECK(b == 5);
    }
    {
        // Edge: negative values
        int a { -3 };
        int b { -7 };
        swapByRef(a, b);
        CHECK(a == -7);
        CHECK(b == -3);
    }
    {
        // Edge: zero involved
        int a { 0 };
        int b { 42 };
        swapByRef(a, b);
        CHECK(a == 42);
        CHECK(b == 0);
    }

    // ── Task 2: swapByPtr — non-null case checks ORIGINALS; null is a no-op ───
    {
        int x { 100 };
        int y { 200 };
        swapByPtr(&x, &y);
        CHECK(x == 200);   // must have swapped through the pointers
        CHECK(y == 100);
    }
    {
        // Edge: first pointer is null -> do nothing
        int y { 99 };
        swapByPtr(nullptr, &y);
        CHECK(y == 99);    // y must be UNCHANGED
    }
    {
        // Edge: second pointer is null -> do nothing
        int x { 42 };
        swapByPtr(&x, nullptr);
        CHECK(x == 42);    // x must be UNCHANGED
    }
    {
        // Edge: BOTH pointers null -> do nothing (no crash)
        swapByPtr(nullptr, nullptr);   // must not crash
        CHECK(true);                   // reaching here proves no UB/crash
    }
    {
        // Edge: negative values through pointers
        int a { -1 };
        int b { -2 };
        swapByPtr(&a, &b);
        CHECK(a == -2);
        CHECK(b == -1);
    }

    // ── Task 3: maxOf — return-by-reference, ALIASING proved by address + write-through
    {
        int a { 3 };
        int b { 7 };
        // Address test: the returned reference must alias `b` (the larger)
        CHECK(&maxOf(a, b) == &b);    // physical aliasing check

        // Write-through test: modifying the return changes THE ORIGINAL
        maxOf(a, b) = 100;
        CHECK(b == 100);              // b changed — not a copy
        CHECK(a == 3);                // a is untouched
    }
    {
        int a { 9 };
        int b { 2 };
        CHECK(&maxOf(a, b) == &a);    // now a is larger
        maxOf(a, b) = 55;
        CHECK(a == 55);               // a changed through the reference
        CHECK(b == 2);                // b untouched
    }
    {
        // Edge: equal values — must return a reference to one of them (we allow `a`)
        int a { 5 };
        int b { 5 };
        int& m { maxOf(a, b) };
        CHECK(&m == &a || &m == &b);  // must alias one of the two — not a third object
        m = 99;
        // At least one of them changed; neither became a mystery third value.
        CHECK(a == 99 || b == 99);
    }
    {
        // Edge: negative values — max of negatives is the one closer to zero
        int a { -3 };
        int b { -1 };
        CHECK(&maxOf(a, b) == &b);    // -1 > -3, so b is the max
    }

    // ── Task 4: describePointer — null and value cases ─────────────────────────
    {
        CHECK(describePointer(nullptr) == std::string_view{"null"});
    }
    {
        int val { 42 };
        CHECK(describePointer(&val) == std::string_view{"value"});
    }
    {
        // Edge: pointer to zero — value, not null (zero is a valid integer)
        int zero { 0 };
        CHECK(describePointer(&zero) == std::string_view{"value"});
    }
    {
        // Edge: pointer to a negative value — still "value"
        int neg { -7 };
        CHECK(describePointer(&neg) == std::string_view{"value"});
    }
    {
        // Edge: const int can be pointed at
        const int cval { 10 };
        CHECK(describePointer(&cval) == std::string_view{"value"});
    }

    // ── Task 5: addBonusInPlace — IN/OUT parameter, ORIGINAL checked ───────────
    {
        int score { 50 };
        addBonusInPlace(score, 10);
        CHECK(score == 60);    // the original variable must reflect the change
    }
    {
        // Edge: zero bonus — score unchanged
        int score { 30 };
        addBonusInPlace(score, 0);
        CHECK(score == 30);
    }
    {
        // Edge: negative bonus (penalty)
        int score { 100 };
        addBonusInPlace(score, -25);
        CHECK(score == 75);
    }
    {
        // Edge: bonus that drives score negative
        int score { 5 };
        addBonusInPlace(score, -20);
        CHECK(score == -15);
    }
    {
        // Chaining: two addBonusInPlace calls compound correctly
        int score { 0 };
        addBonusInPlace(score, 7);
        addBonusInPlace(score, 3);
        CHECK(score == 10);
    }

    // ── Task 6: findFirst — std::optional<int> search ──────────────────────────
    {
        // Basic find: first character
        auto r { findFirst("hello", 'h') };
        CHECK(r.has_value());
        CHECK(*r == 0);
    }
    {
        // Basic find: character in the middle
        auto r { findFirst("hello", 'l') };
        CHECK(r.has_value());
        CHECK(*r == 2);    // first 'l' is at index 2
    }
    {
        // Basic find: last character
        auto r { findFirst("hello", 'o') };
        CHECK(r.has_value());
        CHECK(*r == 4);
    }
    {
        // Not found — must return nullopt
        auto r { findFirst("hello", 'z') };
        CHECK(!r.has_value());
    }
    {
        // Edge: empty string — nothing to find
        auto r { findFirst("", 'a') };
        CHECK(!r.has_value());
    }
    {
        // Edge: single-character string, found
        auto r { findFirst("x", 'x') };
        CHECK(r.has_value());
        CHECK(*r == 0);
    }
    {
        // Edge: single-character string, not found
        auto r { findFirst("x", 'y') };
        CHECK(!r.has_value());
    }
    {
        // Edge: repeated characters — returns FIRST (index 0, not the later ones)
        auto r { findFirst("aaa", 'a') };
        CHECK(r.has_value());
        CHECK(*r == 0);
    }
    {
        // CS6340 flavor: search for a marker in a coverage-log prefix
        auto r { findFirst("BB:42:func", ':') };
        CHECK(r.has_value());
        CHECK(*r == 2);    // first ':' is at index 2
    }

    // ── Final report ────────────────────────────────────────────────────────────
    if (!fails)
        std::cout << "PASS \xe2\x9c\x85  all alias-workshop checks passed.\n";
    else
        std::cerr << "\nFAIL \xe2\x9d\x8c  " << fails
                  << " check(s) failed — fix the TASK blocks in starter/aliases.cpp.\n";

    return fails ? 1 : 0;
}
