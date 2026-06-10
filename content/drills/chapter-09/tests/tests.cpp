// Chapter 9 — Error Handling · Project: The Robust Input Desk   (GRADER — pure functions)
// ─────────────────────────────────────────────────────────────────────────────
// A tiny no-framework unit-test harness (same style as the drills/CLAUDE.md spec).
// It includes ../desk.h and calls the PURE validators/parsers (Tasks 1–5) across
// MANY inputs — good ones AND deliberately malicious ones — checking each returns
// the right signal. Every CHECK that fails prints its expression and line number;
// any failure -> non-zero exit -> `make test` is RED.
//
// Task 6 (readIntInRange) reads std::cin, so it can't be unit-tested by calling it
// here — it's graded separately by FEEDING hostile input on stdin. See
// tests/cin_driver.cpp and the `cin-test` target in the Makefile.
//
// NOTE on assertions: this file is compiled with assertions LIVE (no -DNDEBUG), so
// clampToRange() and isInStock() are only ever called here with VALID preconditions
// (lo <= hi; itemNumber in 1..kMenuItemCount). Tripping a precondition on purpose
// would correctly abort the program — that's the assert doing its job, not a test
// failure. (Try it yourself from the README to watch one fire.)

#include <iostream>
#include "../desk.h"

static int fails = 0;

// CHECK: assert a boolean condition; on failure, report what and where.
#define CHECK(cond) \
    do { if(!(cond)){ std::cerr << "FAIL: " #cond "  @line " << __LINE__ << "\n"; ++fails; } } while(0)

int main()
{
    // ── Task 1: isValidQuantity — the inclusive [1, 99] boundary ─────────────
    CHECK(isValidQuantity(1)  == true);    // lower boundary is VALID
    CHECK(isValidQuantity(50) == true);    // ordinary middle value
    CHECK(isValidQuantity(99) == true);    // upper boundary is VALID
    CHECK(isValidQuantity(0)  == false);   // malicious: just below the floor
    CHECK(isValidQuantity(100) == false);  // malicious: just above the cap
    CHECK(isValidQuantity(-5) == false);   // malicious: negative quantity
    CHECK(isValidQuantity(kMinQuantity) == true);   // stays in sync with the constants
    CHECK(isValidQuantity(kMaxQuantity) == true);

    // ── Task 2: clampToRange — pin values into [lo, hi] (valid preconditions) ─
    CHECK(clampToRange(5, 0, 10) == 5);    // already in range -> unchanged
    CHECK(clampToRange(-3, 0, 10) == 0);   // below low  -> low
    CHECK(clampToRange(42, 0, 10) == 10);  // above high -> high
    CHECK(clampToRange(0, 0, 10) == 0);    // boundary: equals low
    CHECK(clampToRange(10, 0, 10) == 10);  // boundary: equals high
    CHECK(clampToRange(7, 7, 7) == 7);     // edge: degenerate range lo == hi (still legal)
    CHECK(clampToRange(-100, -10, -1) == -10);  // works with negative ranges too

    // ── Task 3: parseMenuChoice — digits map; EVERYTHING else is the sentinel ─
    CHECK(parseMenuChoice('1') == 1);
    CHECK(parseMenuChoice('2') == 2);
    CHECK(parseMenuChoice('3') == 3);
    CHECK(parseMenuChoice('4') == 4);                  // top valid menu digit
    CHECK(parseMenuChoice('0') == kInvalidChoice);     // malicious: digit, but not on the menu
    CHECK(parseMenuChoice('5') == kInvalidChoice);     // malicious: digit just past the menu
    CHECK(parseMenuChoice('9') == kInvalidChoice);     // malicious: high digit
    CHECK(parseMenuChoice('a') == kInvalidChoice);     // malicious: a letter
    CHECK(parseMenuChoice('!') == kInvalidChoice);     // malicious: punctuation
    CHECK(parseMenuChoice(' ') == kInvalidChoice);     // malicious: whitespace
    CHECK(parseMenuChoice('\0') == kInvalidChoice);    // edge: null character

    // ── Task 4: isInStock — the fixed table (item 3 sold out), valid numbers ─
    CHECK(isInStock(1) == true);
    CHECK(isInStock(2) == true);
    CHECK(isInStock(3) == false);          // the deliberately out-of-stock item
    CHECK(isInStock(4) == true);           // boundary: last valid menu number

    // ── Task 5: digitsOnly — read-then-validate; reject anything non-digit ───
    CHECK(digitsOnly("12345") == true);    // normal: all digits
    CHECK(digitsOnly("0") == true);        // single digit
    CHECK(digitsOnly("007") == true);      // leading zeros are still digits
    CHECK(digitsOnly("") == false);        // EDGE: empty string is NOT digits-only
    CHECK(digitsOnly("12a45") == false);   // malicious: an embedded letter
    CHECK(digitsOnly(" 123") == false);    // malicious: a leading space
    CHECK(digitsOnly("123 ") == false);    // malicious: a trailing space
    CHECK(digitsOnly("-5") == false);      // malicious: a sign is not a digit
    CHECK(digitsOnly("3.14") == false);    // malicious: a decimal point
    CHECK(digitsOnly("abc") == false);     // malicious: all letters

    if (!fails)
        std::cout << "PASS \xE2\x9C\x85  all input-desk checks passed.\n";
    else
        std::cerr << "\nFAIL \xE2\x9D\x8C  " << fails << " check(s) failed — fix the TASK blocks in desk.cpp.\n";

    return fails ? 1 : 0;
}
