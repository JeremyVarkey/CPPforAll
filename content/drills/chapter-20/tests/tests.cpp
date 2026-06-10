// ============================================================================
//  tests/tests.cpp  —  automated grader for the calc-core library  (Chapter 20)
// ----------------------------------------------------------------------------
//  Tiny no-framework harness (same style as drills/CLAUDE.md spec).
//  The Makefile compiles this against starter/calccore.cpp for `make test`
//  and against solution/calccore.cpp for `make test-solution`.
//
//  Strategy: every test targets a known property, and at least one check in
//  each group is an EDGE CASE designed to catch a common wrong answer:
//    - factorial base case (n=0) and first recursive step (n=1)
//    - sumDigitsRecursive: zero, single digit, negative input
//    - fibonacci: both base cases (0 and 1), modest depth (n=10)
//    - apply: null pointer guard (must NOT crash), correct dispatch
//    - pickOp: all three known characters and an unknown one
//    - countMatching: empty vector, all-match, no-match, and — critically —
//      a CAPTURING lambda that captures a local `threshold` variable (the test
//      would FAIL to compile if countMatching only accepted a raw fn pointer)
// ============================================================================

#include <iostream>
#include <vector>
#include "../calccore.h"

static int fails = 0;

#define CHECK(cond) \
    do { if (!(cond)) { \
        std::cerr << "FAIL: " #cond "  @line " << __LINE__ << "\n"; \
        ++fails; \
    } } while(0)

// ── A plain (non-capturing) predicate function — for testing that countMatching
//    also accepts a raw function-pointer-shaped callable (notes 20.1).
static bool isEven(int x) { return x % 2 == 0; }

int main()
{
    // ─────────────────────────────────────────────────────────────────────────
    // TASK 1 — factorial
    // ─────────────────────────────────────────────────────────────────────────
    // Base case: 0! == 1 (not 0!)
    CHECK(factorial(0) == 1);        // EDGE: base case
    CHECK(factorial(1) == 1);        // first recursive step (1 * 0!)
    CHECK(factorial(2) == 2);
    CHECK(factorial(3) == 6);
    CHECK(factorial(5) == 120);
    CHECK(factorial(10) == 3628800LL);
    // Ensure long long range — 20! fits; 13! would overflow int
    CHECK(factorial(12) == 479001600LL);
    CHECK(factorial(15) == 1307674368000LL);

    // ─────────────────────────────────────────────────────────────────────────
    // TASK 2 — sumDigitsRecursive
    // (Same job as Chapter 8's sumOfDigits loop — same tests, different path.)
    // ─────────────────────────────────────────────────────────────────────────
    CHECK(sumDigitsRecursive(0)    == 0);   // EDGE: base case, n < 10 -> return 0
    CHECK(sumDigitsRecursive(7)    == 7);   // EDGE: single digit, direct return
    CHECK(sumDigitsRecursive(10)   == 1);   // 1 + 0
    CHECK(sumDigitsRecursive(99)   == 18);
    CHECK(sumDigitsRecursive(123)  == 6);   // 1 + 2 + 3
    CHECK(sumDigitsRecursive(1000) == 1);   // zeros contribute nothing
    CHECK(sumDigitsRecursive(-7)   == 7);   // EDGE: negative single digit -> |n|
    CHECK(sumDigitsRecursive(-123) == 6);   // EDGE: negative -> same as +123

    // ─────────────────────────────────────────────────────────────────────────
    // TASK 3 — fibonacci
    // ─────────────────────────────────────────────────────────────────────────
    CHECK(fibonacci(0)  == 0);    // EDGE: base case 1
    CHECK(fibonacci(1)  == 1);    // EDGE: base case 2
    CHECK(fibonacci(2)  == 1);    // first real recursive case
    CHECK(fibonacci(3)  == 2);
    CHECK(fibonacci(4)  == 3);
    CHECK(fibonacci(5)  == 5);
    CHECK(fibonacci(6)  == 8);
    CHECK(fibonacci(7)  == 13);
    CHECK(fibonacci(10) == 55);
    CHECK(fibonacci(15) == 610);
    CHECK(fibonacci(20) == 6765); // largest safe test for naive recursive fib

    // ─────────────────────────────────────────────────────────────────────────
    // TASK 4 — apply
    // ─────────────────────────────────────────────────────────────────────────
    CHECK(apply(10, 3, &addOp) == 13);   // dispatch to addOp
    CHECK(apply(10, 3, &subOp) == 7);    // dispatch to subOp
    CHECK(apply(10, 3, &mulOp) == 30);   // dispatch to mulOp
    CHECK(apply(0,  0, &addOp) == 0);    // EDGE: zero arguments
    CHECK(apply(5,  5, &subOp) == 0);    // EDGE: subtraction -> zero result
    CHECK(apply(7,  0, &mulOp) == 0);    // EDGE: multiply by zero
    // Null-pointer guard: must NOT crash or produce nonsense; returns 0
    CHECK(apply(10, 3, nullptr) == 0);   // EDGE: null guard

    // ─────────────────────────────────────────────────────────────────────────
    // TASK 5 — pickOp
    // ─────────────────────────────────────────────────────────────────────────
    // pickOp gives back a pointer; apply exercises that pointer
    CHECK(apply(6, 2, pickOp('+')) == 8);    // '+' -> addOp
    CHECK(apply(6, 2, pickOp('-')) == 4);    // '-' -> subOp
    CHECK(apply(6, 2, pickOp('*')) == 12);   // '*' -> mulOp
    // pickOp on an unknown character must return nullptr
    CHECK(pickOp('?')  == nullptr);          // EDGE: unknown op
    CHECK(pickOp('\0') == nullptr);          // EDGE: null char
    // apply(null) must be safe (tested above, but let's chain the full path)
    CHECK(apply(6, 2, pickOp('/')) == 0);    // '/' unknown -> nullptr -> safe sentinel

    // ─────────────────────────────────────────────────────────────────────────
    // TASK 6 — countMatching
    // ─────────────────────────────────────────────────────────────────────────
    const std::vector<int> nums { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

    // --- plain function pointer (non-capturing) ---
    // std::function must accept a plain fn pointer, not just lambdas
    CHECK(countMatching(nums, &isEven) == 5);    // evens: 2,4,6,8,10

    // --- non-capturing lambda ---
    CHECK(countMatching(nums, [](int x){ return x > 5; }) == 5);   // 6,7,8,9,10
    CHECK(countMatching(nums, [](int x){ return x == 1; }) == 1);  // exactly one
    CHECK(countMatching(nums, [](int x){ return x < 0; }) == 0);   // EDGE: no matches

    // --- CAPTURING lambda (must capture a local variable) ---
    // This test is the HEART of Task 6: `threshold` is a local int; the lambda
    // captures it by value.  A raw `bool (*)(int)` parameter would FAIL to
    // accept this — only `std::function` (or a template) can hold it.
    {
        int threshold { 7 };
        CHECK(countMatching(nums,
            [threshold](int x){ return x >= threshold; }) == 4); // 7,8,9,10
    }
    {
        int threshold { 3 };
        CHECK(countMatching(nums,
            [threshold](int x){ return x < threshold; }) == 2);  // 1, 2
    }
    {
        int threshold { 1 };
        CHECK(countMatching(nums,
            [threshold](int x){ return x == threshold; }) == 1); // only 1
    }

    // --- edge cases for countMatching ---
    // Empty vector -> always 0
    CHECK(countMatching({}, [](int x){ return x > 0; }) == 0);  // EDGE: empty
    // All elements match
    CHECK(countMatching({ 2, 4, 6 }, &isEven) == 3);            // all match
    // Single element
    CHECK(countMatching({ 5 }, [](int x){ return x == 5; }) == 1);

    // ─────────────────────────────────────────────────────────────────────────
    if (!fails)
        std::cout << "PASS \xE2\x9C\x85  all calc-core checks passed.\n";
    else
        std::cerr << "\nFAIL \xE2\x9D\x8C  " << fails
                  << " check(s) failed — fix the TASK blocks in starter/calccore.cpp.\n";

    return fails ? 1 : 0;
}
