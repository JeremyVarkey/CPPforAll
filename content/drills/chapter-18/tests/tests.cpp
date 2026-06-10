// Chapter 18 — Iterators and Algorithms · Project: VecTools   (GRADER)
// ─────────────────────────────────────────────────────────────────────────────
// A tiny no-framework unit-test harness (same style as drills/CLAUDE.md spec).
// It includes ../vectools.h and calls the API through the vectools:: namespace.
//
// The Makefile links this file against starter/vectools.cpp (`make test`) or
// solution/vectools.cpp (`make test-solution`).
//
// WHAT PASSES: the solution.   WHAT FAILS: the starter (all stubs return 0/-1).
//
// TESTING STRATEGY — each pair of hand/algo functions is checked to AGREE on
// the same inputs. If they disagree, one of them is wrong. Edge cases tested:
//   • empty vector                (all functions must handle it gracefully)
//   • single-element vector       (boundary: begin == one past last element)
//   • duplicate values            (find returns the FIRST; count counts all)
//   • all-even / all-odd vectors  (count edge)
//   • negative numbers            (isEven works for negatives: -4 % 2 == 0)
//   • already-sorted input        (sort must not corrupt it)
//   • all-same elements           (sort of equal values; greaterThan must not
//                                  claim equal elements are > each other)

#include <iostream>
#include <vector>
#include "../vectools.h"

static int fails = 0;

// CHECK: assert a boolean condition; on failure, report the expression and line.
#define CHECK(cond) \
    do { if(!(cond)){ std::cerr << "FAIL: " #cond "  @line " << __LINE__ << "\n"; ++fails; } } while(0)

int main()
{
    // =========================================================================
    //  Task 1 — isEven (free-function predicate)
    // =========================================================================
    CHECK(vectools::isEven(0)  == true);    // 0 is even
    CHECK(vectools::isEven(2)  == true);
    CHECK(vectools::isEven(4)  == true);
    CHECK(vectools::isEven(-4) == true);    // edge: negative even
    CHECK(vectools::isEven(1)  == false);
    CHECK(vectools::isEven(3)  == false);
    CHECK(vectools::isEven(-3) == false);   // edge: negative odd

    // =========================================================================
    //  Task 1 — greaterThan (free-function comparator)
    // =========================================================================
    CHECK(vectools::greaterThan(5, 3) == true);
    CHECK(vectools::greaterThan(3, 5) == false);
    CHECK(vectools::greaterThan(5, 5) == false);   // CRITICAL: equal -> false
                                                   // (strict weak ordering)
    CHECK(vectools::greaterThan(0, -1) == true);   // edge: negative
    CHECK(vectools::greaterThan(-1, 0) == false);

    // =========================================================================
    //  Task 2 — handFind and algoFind
    // =========================================================================
    {
        std::vector<int> v { 3, 1, 4, 1, 5, 9, 2, 6 };

        // basic finds
        CHECK(vectools::handFind(v, 3) == 0);        // first element
        CHECK(vectools::algoFind(v, 3) == 0);
        CHECK(vectools::handFind(v, 9) == 5);        // middle element
        CHECK(vectools::algoFind(v, 9) == 5);
        CHECK(vectools::handFind(v, 6) == 7);        // last element
        CHECK(vectools::algoFind(v, 6) == 7);

        // not found
        CHECK(vectools::handFind(v, 7) == -1);
        CHECK(vectools::algoFind(v, 7) == -1);

        // duplicates: must return the FIRST occurrence
        CHECK(vectools::handFind(v, 1) == 1);        // two 1s; first is index 1
        CHECK(vectools::algoFind(v, 1) == 1);

        // hand and algo must agree on every value
        for (int x : { 3, 1, 4, 5, 9, 2, 6, 7, 0 })
            CHECK(vectools::handFind(v, x) == vectools::algoFind(v, x));
    }

    {
        // empty vector — not found
        std::vector<int> empty {};
        CHECK(vectools::handFind(empty, 1) == -1);
        CHECK(vectools::algoFind(empty, 1) == -1);

        // single-element vector
        std::vector<int> one { 42 };
        CHECK(vectools::handFind(one, 42) == 0);
        CHECK(vectools::algoFind(one, 42) == 0);
        CHECK(vectools::handFind(one, 99) == -1);
        CHECK(vectools::algoFind(one, 99) == -1);
    }

    // =========================================================================
    //  Task 3 — handCountEven and algoCountEven
    // =========================================================================
    {
        std::vector<int> v { 1, 2, 3, 4, 5, 6 };   // three even: 2,4,6
        CHECK(vectools::handCountEven(v) == 3);
        CHECK(vectools::algoCountEven(v) == 3);
        CHECK(vectools::handCountEven(v) == vectools::algoCountEven(v));
    }

    {
        // all even
        std::vector<int> allEven { 2, 4, 6, 8 };
        CHECK(vectools::handCountEven(allEven) == 4);
        CHECK(vectools::algoCountEven(allEven) == 4);
    }

    {
        // all odd
        std::vector<int> allOdd { 1, 3, 5, 7 };
        CHECK(vectools::handCountEven(allOdd) == 0);
        CHECK(vectools::algoCountEven(allOdd) == 0);
    }

    {
        // empty vector
        std::vector<int> empty {};
        CHECK(vectools::handCountEven(empty) == 0);
        CHECK(vectools::algoCountEven(empty) == 0);
    }

    {
        // single element
        std::vector<int> one { 8 };
        CHECK(vectools::handCountEven(one) == 1);
        CHECK(vectools::algoCountEven(one) == 1);

        std::vector<int> oneOdd { 7 };
        CHECK(vectools::handCountEven(oneOdd) == 0);
        CHECK(vectools::algoCountEven(oneOdd) == 0);
    }

    {
        // negative numbers: -4 is even, -3 is odd
        std::vector<int> negs { -4, -3, -2, -1, 0 };
        // even: -4, -2, 0  -> count 3
        CHECK(vectools::handCountEven(negs) == 3);
        CHECK(vectools::algoCountEven(negs) == 3);
        CHECK(vectools::handCountEven(negs) == vectools::algoCountEven(negs));
    }

    // =========================================================================
    //  Task 4 — sortAscending and sortDescending
    // =========================================================================
    {
        std::vector<int> v { 9, 1, 4, 7, 2 };
        vectools::sortAscending(v);
        // must be fully sorted ascending
        CHECK(v[0] == 1);
        CHECK(v[1] == 2);
        CHECK(v[2] == 4);
        CHECK(v[3] == 7);
        CHECK(v[4] == 9);
    }

    {
        std::vector<int> v { 9, 1, 4, 7, 2 };
        vectools::sortDescending(v);
        // must be fully sorted descending
        CHECK(v[0] == 9);
        CHECK(v[1] == 7);
        CHECK(v[2] == 4);
        CHECK(v[3] == 2);
        CHECK(v[4] == 1);
    }

    {
        // already sorted ascending — must remain ascending
        std::vector<int> asc { 1, 2, 3, 4, 5 };
        vectools::sortAscending(asc);
        CHECK(asc[0] == 1 && asc[4] == 5);

        // already sorted descending — must remain descending
        std::vector<int> desc { 5, 4, 3, 2, 1 };
        vectools::sortDescending(desc);
        CHECK(desc[0] == 5 && desc[4] == 1);
    }

    {
        // all same elements — sort should not corrupt the vector
        std::vector<int> same { 7, 7, 7 };
        vectools::sortAscending(same);
        CHECK(same[0] == 7 && same[1] == 7 && same[2] == 7);

        vectools::sortDescending(same);
        CHECK(same[0] == 7 && same[1] == 7 && same[2] == 7);
    }

    {
        // single element
        std::vector<int> one { 42 };
        vectools::sortAscending(one);
        CHECK(one[0] == 42);

        vectools::sortDescending(one);
        CHECK(one[0] == 42);
    }

    {
        // empty vector — both sorts must be no-ops
        std::vector<int> empty {};
        vectools::sortAscending(empty);
        CHECK(empty.empty());

        vectools::sortDescending(empty);
        CHECK(empty.empty());
    }

    // =========================================================================
    //  Task 5 — largest
    // =========================================================================
    {
        std::vector<int> v { 3, 1, 4, 1, 5, 9, 2, 6 };
        CHECK(vectools::largest(v) == 9);
    }

    {
        // single element
        std::vector<int> one { 42 };
        CHECK(vectools::largest(one) == 42);
    }

    {
        // EMPTY VECTOR: must return 0 (documented fallback), not crash
        std::vector<int> empty {};
        CHECK(vectools::largest(empty) == 0);
    }

    {
        // all same
        std::vector<int> same { 7, 7, 7 };
        CHECK(vectools::largest(same) == 7);
    }

    {
        // negative numbers — largest is still the one closest to 0
        std::vector<int> negs { -5, -1, -3 };
        CHECK(vectools::largest(negs) == -1);
    }

    {
        // mixed: the only positive value is the largest
        std::vector<int> mixed { -10, -3, 2, -7 };
        CHECK(vectools::largest(mixed) == 2);
    }

    // =========================================================================
    //  Cross-checks: hand* and algo* must agree on every shared input
    // =========================================================================
    {
        std::vector<int> v { 10, 3, 8, 2, 6, 1, 4 };
        CHECK(vectools::handFind(v, 8)      == vectools::algoFind(v, 8));
        CHECK(vectools::handFind(v, 99)     == vectools::algoFind(v, 99));
        CHECK(vectools::handCountEven(v)    == vectools::algoCountEven(v));
    }

    // =========================================================================
    //  Result summary
    // =========================================================================
    if (!fails)
        std::cout << "PASS \xe2\x9c\x85  all vectools checks passed.\n";
    else
        std::cerr << "\nFAIL \xe2\x9d\x8c  " << fails
                  << " check(s) failed — fix the TASK blocks in vectools.cpp.\n";

    return fails ? 1 : 0;
}
