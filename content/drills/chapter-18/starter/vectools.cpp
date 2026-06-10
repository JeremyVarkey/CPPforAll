// Chapter 18 — Iterators and Algorithms · Project: VecTools   (STARTER)
// ─────────────────────────────────────────────────────────────────────────────
// Fill in the TASK blocks below. Each maps 1:1 to a task in the README and to a
// declaration in ../vectools.h. The bodies currently return WRONG-but-compiling
// placeholders — that is why `make test` is RED right now. Your job: turn it GREEN
// by implementing the real logic.
//
//     make build         compile your code (should already work)
//     make test          grade it (RED until you fill in the tasks)
//     make solution      run the reference if you get stuck
//
// THE PATTERN YOU ARE BUILDING TOWARDS:
//   Every "hand*" function walks the vector with an explicit iterator loop —
//   the same loop the standard library runs internally. Every "algo*" function
//   delegates to a standard <algorithm>, using a free function as the
//   predicate/comparator (NO lambdas — those arrive in Chapter 20).
//
// ITERATOR MENTAL MODEL (notes 18.2):
//
//   auto it { vec.begin() };   // cursor pointing at the FIRST element
//
//   vec.begin()        vec.end()
//       |                  |
//       v                  v
//   [ 3 ][ 1 ][ 4 ][ 1 ] one-past-last  <-- DO NOT dereference end()!
//
//   *it      -> value at the cursor     (dereference)
//   ++it     -> advance cursor by one
//   it != end -> test: are we still inside the range?
//
// CS6340 / LLVM LENS:
//   LLVM passes iterate over BasicBlock::iterator, Function::iterator, etc.
//   The identical begin()/end()/++it/!= pattern you drill here shows up verbatim
//   in every real pass. Being fluent now means you can read LLVM source code
//   without stopping.

#include "../vectools.h"
#include <algorithm>   // std::find, std::count_if, std::sort, std::max_element
#include <iterator>    // std::distance

namespace vectools
{
    // ─── TASK 1a: isEven — free-function predicate ────────────────────────────
    // Return true if n is even (divisible by 2).
    // Used as the predicate argument to std::count_if in algoCountEven.
    // NO lambdas — this IS the Chapter 18 way to pass behaviour to an algorithm.
    //
    //   >>> YOUR CODE HERE <<<
    //
    bool isEven(int /*n*/)
    {
        return false;   // placeholder — always says "not even"
    }
    // ─────────────────────────────────────────────────────────────────────────

    // ─── TASK 1b: greaterThan — free-function comparator ──────────────────────
    // Return true iff a > b (a should come BEFORE b in descending order).
    // Passed to std::sort as a strict-weak-ordering comparator.
    //
    // IMPORTANT: do NOT use `>=` or `<=` — that would violate strict weak
    // ordering (equal elements would each claim to come before the other), which
    // is undefined behaviour in std::sort. Use strictly `>`.
    //
    //   >>> YOUR CODE HERE <<<
    //
    bool greaterThan(int /*a*/, int /*b*/)
    {
        return false;   // placeholder — always says "no swap needed" (no ordering)
    }
    // ─────────────────────────────────────────────────────────────────────────

    // ─── TASK 2a: handFind — explicit iterator loop ────────────────────────────
    // Search `vec` for `target` using an iterator loop. Return the 0-based index
    // of the first match, or -1 if not found.
    //
    // Skeleton:
    //   auto it { vec.begin() };
    //   while (it != vec.end()) {
    //       if (*it == target) return <index>;
    //       ++it;
    //   }
    //   return -1;
    //
    // To get the index from an iterator, use std::distance(vec.begin(), it),
    // which counts how many steps from begin to it.
    //
    // DO NOT call std::find or any other <algorithm> here.
    //
    //   >>> YOUR CODE HERE <<<
    //
    int handFind(const std::vector<int>& /*vec*/, int /*target*/)
    {
        return -1;   // placeholder — always says "not found"
    }
    // ─────────────────────────────────────────────────────────────────────────

    // ─── TASK 2b: algoFind — wrapping std::find ────────────────────────────────
    // Use std::find(vec.begin(), vec.end(), target) to search.
    // std::find returns an ITERATOR — you must:
    //   1. Check it against vec.end() (not-found case).
    //   2. If found, convert to an index: std::distance(vec.begin(), it).
    //
    //   >>> YOUR CODE HERE <<<
    //
    int algoFind(const std::vector<int>& /*vec*/, int /*target*/)
    {
        return -1;   // placeholder
    }
    // ─────────────────────────────────────────────────────────────────────────

    // ─── TASK 3a: handCountEven — iterator loop ────────────────────────────────
    // Count elements where *it % 2 == 0 using an explicit iterator loop.
    // DO NOT call std::count_if or any other <algorithm>.
    //
    //   >>> YOUR CODE HERE <<<
    //
    int handCountEven(const std::vector<int>& /*vec*/)
    {
        return 0;   // placeholder — always says "zero even elements"
    }
    // ─────────────────────────────────────────────────────────────────────────

    // ─── TASK 3b: algoCountEven — wrapping std::count_if ─────────────────────
    // Use: std::count_if(vec.begin(), vec.end(), vectools::isEven)
    //
    // Pass the FREE FUNCTION isEven as the predicate (no lambdas!).
    // std::count_if returns a std::ptrdiff_t; cast it to int before returning.
    //
    //   >>> YOUR CODE HERE <<<
    //
    int algoCountEven(const std::vector<int>& /*vec*/)
    {
        return 0;   // placeholder
    }
    // ─────────────────────────────────────────────────────────────────────────

    // ─── TASK 4a: sortAscending — std::sort, default ordering ─────────────────
    // Sort vec in ascending order using std::sort(vec.begin(), vec.end()).
    // The default comparator uses operator< — smallest element goes first.
    //
    //   >>> YOUR CODE HERE <<<
    //
    void sortAscending(std::vector<int>& /*vec*/)
    {
        // placeholder: does nothing — vec stays unsorted
    }
    // ─────────────────────────────────────────────────────────────────────────

    // ─── TASK 4b: sortDescending — std::sort with free-function comparator ────
    // Sort vec in descending order using std::sort with the greaterThan
    // comparator declared in this namespace:
    //   std::sort(vec.begin(), vec.end(), vectools::greaterThan)
    //
    // greaterThan(a, b) returns true iff a > b — std::sort puts `a` before `b`
    // when the comparator says "a should come first". Passing greaterThan
    // therefore puts larger elements first.
    //
    //   >>> YOUR CODE HERE <<<
    //
    void sortDescending(std::vector<int>& /*vec*/)
    {
        // placeholder: does nothing — vec stays unsorted
    }
    // ─────────────────────────────────────────────────────────────────────────

    // ─── TASK 5: largest — std::max_element, with empty-range guard ──────────
    // Return the largest element in vec, or 0 if vec is empty.
    //
    // EMPTY-RANGE TRAP (notes 18.2):
    //   auto it { std::max_element(vec.begin(), vec.end()) };
    //   if (it == vec.end()) return 0;   // MUST check before dereferencing!
    //   return *it;
    //
    // std::max_element returns vec.end() when the range is empty. Dereferencing
    // end() is undefined behaviour — always guard against it.
    //
    //   >>> YOUR CODE HERE <<<
    //
    int largest(const std::vector<int>& /*vec*/)
    {
        return 0;   // placeholder — happens to be correct ONLY for empty vector
    }
    // ─────────────────────────────────────────────────────────────────────────

} // namespace vectools
