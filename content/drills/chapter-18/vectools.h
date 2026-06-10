// ============================================================================
//  vectools.h  —  the PUBLIC INTERFACE of the vectools library  (Chapter 18)
// ----------------------------------------------------------------------------
//  This header is COMPLETE and PROVIDED. Do not edit it. The grader, the demo,
//  the starter, and the solution all include it. Changing a signature will
//  break the build.
//
//  WHAT THIS LIBRARY DEMONSTRATES — the "two ways" pattern:
//  For each operation you write TWO versions that MUST agree on every input:
//
//    hand*   — written with explicit iterator objects (.begin() / .end() /
//              ++it / *it) so you SEE the abstraction at work.
//    algo*   — delegates to a standard <algorithm>, with free-function
//              predicates/comparators (no lambdas — those arrive in Ch 20).
//
//  Seeing both forms side-by-side is the central lesson of Chapter 18: iterators
//  are the VOCABULARY that algorithms speak, and the standard library is using
//  exactly the same loop you would have written by hand — just packaged and named.
//
//  FREE-FUNCTION PREDICATES (not lambdas):
//    The <algorithm> functions that accept a callable (count_if, sort, …) take
//    any callable. Chapter 18 teaches them via free functions (plain C++ function
//    pointers) — e.g. `bool isEven(int n)`. Lambdas are the natural next step,
//    but they are not introduced until Chapter 20. This file therefore declares
//    two free-function predicates/comparators in namespace vectools — those are
//    ALSO tasks the learner implements.
//
//  CS6340 / LLVM LENS:
//    LLVM's pass infrastructure passes collections of instructions, basic blocks,
//    and functions as ranges. The same `begin()` / `end()` / algorithm idioms you
//    practice here appear verbatim in real LLVM passes. Being fluent now means you
//    can read and write that code without hesitation in Lab 1.
//
//  Header guard (Chapter 2): prevents double inclusion.
#ifndef VECTOOLS_H
#define VECTOOLS_H

#include <vector>   // std::vector (Chapter 16)
#include <cstddef>  // std::ptrdiff_t

namespace vectools
{
    // =========================================================================
    //  FREE-FUNCTION PREDICATES / COMPARATORS (Tasks 1a and 1b)
    //  -------------------------------------------------------------------------
    //  These are passed BY POINTER to std::count_if and std::sort respectively.
    //  They must be FREE FUNCTIONS (not lambdas, not functors) — Ch 20 introduces
    //  lambdas; here we use the plain function-pointer form.
    // =========================================================================

    // ── TASK 1a predicate ────────────────────────────────────────────────────
    // isEven — return true iff n is divisible by 2.
    // Passed as a function pointer to std::count_if in algoCountEven (Task 3b).
    bool isEven(int n);

    // ── TASK 1b comparator ───────────────────────────────────────────────────
    // greaterThan — return true iff a should come BEFORE b in descending order.
    // A valid strict-weak-ordering comparator: returns true iff a > b.
    // Passed as a function pointer to std::sort in sortDescending (Task 4b).
    // IMPORTANT: "less than or equal" (<=) would violate strict weak ordering;
    // use strictly greater than (>).
    bool greaterThan(int a, int b);

    // =========================================================================
    //  FIND (Tasks 2a & 2b)
    //  -------------------------------------------------------------------------
    //  Search vec for the first occurrence of `target`.
    //  Returns the 0-based INDEX if found, or -1 if not found.
    //  (Returning an index is clean and avoids exposing iterator internals in
    //  the public API; internally the implementations use iterators.)
    // =========================================================================

    // ── TASK 2a: hand-written search with explicit iterators ─────────────────
    // Write your own loop with:
    //   auto it { vec.begin() }; … it != vec.end(); ++it … *it
    // Do NOT call std::find or any other <algorithm>.
    int handFind(const std::vector<int>& vec, int target);

    // ── TASK 2b: algorithm search wrapping std::find ─────────────────────────
    // Use: std::find(vec.begin(), vec.end(), target)
    // Check the returned iterator against vec.end() to distinguish found/not-found.
    // Convert the iterator to an index with std::distance(vec.begin(), it).
    int algoFind(const std::vector<int>& vec, int target);

    // =========================================================================
    //  COUNT EVEN (Tasks 3a & 3b)
    //  -------------------------------------------------------------------------
    //  Count how many elements in vec are even (divisible by 2).
    // =========================================================================

    // ── TASK 3a: hand-written count with explicit iterators ──────────────────
    // Iterate with the iterator loop pattern; test each *it with `% 2 == 0`.
    // Do NOT call std::count_if or any other <algorithm>.
    int handCountEven(const std::vector<int>& vec);

    // ── TASK 3b: algorithm count using std::count_if + free function ─────────
    // Use: std::count_if(vec.begin(), vec.end(), vectools::isEven)
    // Pass the FREE FUNCTION `isEven` (declared above) as the predicate.
    // NO lambdas — those are Chapter 20.
    int algoCountEven(const std::vector<int>& vec);

    // =========================================================================
    //  SORT (Tasks 4a & 4b)
    //  -------------------------------------------------------------------------
    //  Both functions sort IN PLACE (modify the vector passed by reference).
    // =========================================================================

    // ── TASK 4a: ascending sort ───────────────────────────────────────────────
    // Use: std::sort(vec.begin(), vec.end())  — the default uses operator<.
    void sortAscending(std::vector<int>& vec);

    // ── TASK 4b: descending sort with free-function comparator ───────────────
    // Use: std::sort(vec.begin(), vec.end(), vectools::greaterThan)
    // Pass the FREE FUNCTION `greaterThan` (declared above) as the comparator.
    // greaterThan(a,b) returns true iff a > b — std::sort puts `a` before `b`
    // when the comparator says a "comes first".
    void sortDescending(std::vector<int>& vec);

    // =========================================================================
    //  LARGEST (Task 5)
    //  -------------------------------------------------------------------------
    //  Return the largest element in vec, or 0 if vec is empty.
    //
    //  Use std::max_element(vec.begin(), vec.end()).
    //  EMPTY-RANGE TRAP: std::max_element returns vec.end() when the range is
    //  empty. You MUST check for that before dereferencing. The documented
    //  fallback for an empty vector is 0.
    // =========================================================================

    // ── TASK 5: largest element via std::max_element ─────────────────────────
    int largest(const std::vector<int>& vec);

} // namespace vectools

#endif // VECTOOLS_H
