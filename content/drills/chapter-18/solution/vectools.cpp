// Chapter 18 — Iterators and Algorithms · Project: VecTools  (REFERENCE SOLUTION)
// ─────────────────────────────────────────────────────────────────────────────
// One complete, correct, warning-clean implementation of ../vectools.h.
// Peek only after you've taken a real swing at starter/vectools.cpp — the
// learning is in writing the explicit iterator loops yourself, then comparing
// them against the identical logic that the standard library hides inside
// std::find, std::count_if, etc.
//
// KEY OBSERVATION: every "hand*" loop here is exactly what the "algo*" version
// executes internally. The algorithms are not magic — they are the same cursor
// advancing, the same dereference, the same end() check, just packaged with a
// name. Knowing that makes the standard library readable instead of opaque.

#include "../vectools.h"
#include <algorithm>   // std::find, std::count_if, std::sort, std::max_element
#include <iterator>    // std::distance

namespace vectools
{
    // ─── isEven — predicate passed to std::count_if ───────────────────────────
    // A free function, not a lambda (lambdas are Chapter 20). The function
    // pointer `vectools::isEven` decays to a bool(*)(int) which std::count_if
    // accepts as its Predicate argument.
    bool isEven(int n)
    {
        return n % 2 == 0;
    }

    // ─── greaterThan — comparator passed to std::sort ─────────────────────────
    // Returns true iff `a` should come before `b` in the sorted sequence.
    // For descending order: `a` comes first when `a` is larger.
    //
    // STRICT WEAK ORDERING: the comparator MUST satisfy these rules for std::sort
    // to behave correctly (undefined behaviour otherwise):
    //   • irreflexivity:  greaterThan(x, x) == false  (an element is not > itself)
    //   • asymmetry:      if greaterThan(a,b) then !greaterThan(b,a)
    //   • transitivity:   if gt(a,b) && gt(b,c) then gt(a,c)
    // Using `a > b` (strictly greater, NOT >=) satisfies all three.
    bool greaterThan(int a, int b)
    {
        return a > b;
    }

    // ─── TASK 2a: handFind — the iterator loop in full view ──────────────────
    // This is the loop that std::find runs internally. Writing it out makes the
    // abstraction physical: `it` is a cursor; `*it` is the box it points at;
    // `++it` moves it one step to the right; `it != vec.end()` tells us we
    // haven't walked off the end.
    int handFind(const std::vector<int>& vec, int target)
    {
        for (auto it { vec.begin() }; it != vec.end(); ++it)
        {
            if (*it == target)
            {
                // std::distance counts the steps from begin to `it`, giving us
                // the 0-based index. (For vector iterators this is O(1) because
                // they support random access, but the general form works for any
                // forward iterator.)
                return static_cast<int>(std::distance(vec.begin(), it));
            }
        }
        return -1;   // not found
    }

    // ─── TASK 2b: algoFind — delegating to std::find ─────────────────────────
    // std::find returns an iterator, not an index. The two-step pattern:
    //   1. get the iterator;
    //   2. check for not-found BEFORE dereferencing (notes 18.2).
    // Never dereference a vec.end() iterator — it has no element behind it.
    int algoFind(const std::vector<int>& vec, int target)
    {
        auto it { std::find(vec.begin(), vec.end(), target) };

        if (it == vec.end())
            return -1;   // not found — do NOT dereference here

        return static_cast<int>(std::distance(vec.begin(), it));
    }

    // ─── TASK 3a: handCountEven — counting loop with explicit iterators ───────
    // The predicate test (`*it % 2 == 0`) is inlined here. Compare with
    // algoCountEven: they encode the same logic, one visible, one named.
    int handCountEven(const std::vector<int>& vec)
    {
        int count { 0 };
        for (auto it { vec.begin() }; it != vec.end(); ++it)
        {
            if (*it % 2 == 0)
                ++count;
        }
        return count;
    }

    // ─── TASK 3b: algoCountEven — delegating to std::count_if ────────────────
    // isEven is a plain free function; passing it as `vectools::isEven` decays
    // to a function pointer — the Chapter 18 way. std::count_if returns
    // std::ptrdiff_t (a signed integer type); cast to int for the API.
    int algoCountEven(const std::vector<int>& vec)
    {
        return static_cast<int>(
            std::count_if(vec.begin(), vec.end(), vectools::isEven));
    }

    // ─── TASK 4a: sortAscending — default std::sort (uses operator<) ─────────
    // The default comparator is std::less<int>, which uses `<`. Elements move
    // left until the sequence satisfies "for all adjacent pairs: left <= right."
    void sortAscending(std::vector<int>& vec)
    {
        std::sort(vec.begin(), vec.end());
    }

    // ─── TASK 4b: sortDescending — std::sort with greaterThan ────────────────
    // Passing greaterThan flips the ordering rule: std::sort places `a` before
    // `b` whenever greaterThan(a, b) is true, i.e. whenever a > b — so the
    // largest element ends up at the front.
    void sortDescending(std::vector<int>& vec)
    {
        std::sort(vec.begin(), vec.end(), vectools::greaterThan);
    }

    // ─── TASK 5: largest — std::max_element with empty-range guard ───────────
    // std::max_element scans the half-open range [begin, end) and returns an
    // iterator to the maximum element — or vec.end() if the range is empty.
    //
    // THE TRAP: dereferencing vec.end() is undefined behaviour. Always check
    // the returned iterator before using it. The documented fallback here is 0.
    //
    // CS6340 PARALLEL: when scanning a BasicBlock for its heaviest instruction,
    // the pass must guard against an empty block in exactly the same way.
    int largest(const std::vector<int>& vec)
    {
        auto it { std::max_element(vec.begin(), vec.end()) };

        if (it == vec.end())
            return 0;   // empty vector — no maximum exists; return fallback

        return *it;     // dereference is safe: we confirmed it != end()
    }

} // namespace vectools
