// Chapter 16 — Dynamic Arrays: std::vector · Sensor-Readings Toolkit  (SOLUTION)
// ─────────────────────────────────────────────────────────────────────────────
// Reference implementation of ../sensor.h. Each function is complete, correct,
// and -Wall -Wextra clean. Peek here only AFTER taking a real swing at the
// TASK blocks — the learning is in implementing the vector idioms yourself.
//
// CS6340 lens: these patterns appear constantly in static-analysis tooling:
//   range-for on coverage-point vectors, push_back to grow seed pools,
//   maxReading-style scans over function-size statistics, normalizeInPlace-
//   style passes that rewrite LLVM Value attributes in place.

#include "../sensor.h"
#include <cassert>   // assert — Chapter 9 precondition guard

namespace sensor
{

// ─── TASK 1: totalReading — range-for (read-only) ────────────────────────────
// `const auto&` avoids copying each int (good habit for heavier element types;
// for int the cost is negligible, but the idiom is the lesson). The loop body
// runs zero times on an empty vector, so the return-0 base case is automatic.
int totalReading(const std::vector<int>& v)
{
    int sum { 0 };
    for (const auto& x : v)   // KEY: range-for, const reference (notes 16.8)
        sum += x;
    return sum;
}

// ─── TASK 2: averageReading — static_cast-before-divide ──────────────────────
// The classic Chapter 4 / 10 trap: integer division silently truncates.
// `static_cast<double>(sum)` forces double arithmetic before the division
// happens. Dividing by v.size() (a std::size_t) is safe here because the
// implicit unsigned->double conversion loses nothing for realistic sizes.
double averageReading(const std::vector<int>& v)
{
    if (v.empty())             // documented: empty vector -> 0.0
        return 0.0;

    int sum { totalReading(v) };                        // reuse Task 1 (function composition)
    return static_cast<double>(sum)                     // KEY: cast before divide (Ch 4 / Ch 10 trap)
         / static_cast<double>(v.size());               // v.size() is std::size_t (notes 16.3)
}

// ─── TASK 3: maxReading — index-based loop, std::size_t ──────────────────────
// We use an index loop deliberately (notes 16.3, 16.6, 16.7): std::size_t for
// the counter matches vector's size_type, so there is no signed/unsigned warning.
// Seeding with v[0] and comparing from index 1 onward is the clean pattern.
// The precondition (non-empty) is guarded with assert (notes 16.4 pattern).
int maxReading(const std::vector<int>& v)
{
    assert(!v.empty());        // precondition: notes 16.4 says assert for programmer assumptions

    int maxVal { v[0] };       // seed with the first element
    for (std::size_t i { 1 }; i < v.size(); ++i)   // KEY: std::size_t, no cast needed (notes 16.3)
    {
        if (v[i] > maxVal)
            maxVal = v[i];
    }
    return maxVal;
}

// ─── TASK 4: countAbove — range-for + conditional ────────────────────────────
// Straightforward accumulator. The empty-vector case is automatic: the body
// never runs and count stays 0. Notes 16.8: "if the container is empty, the
// body runs zero times."
int countAbove(const std::vector<int>& v, int threshold)
{
    int count { 0 };
    for (const auto& x : v)   // range-for: no index needed (notes 16.7, 16.8)
    {
        if (x > threshold)
            ++count;
    }
    return count;
}

// ─── TASK 5: normalizeInPlace — `auto&` range-for for MUTATION ────────────────
// The central lesson of this task (notes 16.8 decision table):
//
//   `const auto& x` -> read only, no copy
//   `auto& x`       -> read + MUTATE in place
//
// Without the `&`, `x` would be a copy of the element — the division would
// happen on that throwaway copy and the vector's actual storage would be
// untouched. The `&` makes x an alias for the real element.
//
// `v` is passed by non-const ref so mutations persist for the caller (16.4).
void normalizeInPlace(std::vector<double>& v, double scale)
{
    for (auto& x : v)          // KEY: auto& (mutable alias) — not `auto` (copy) (notes 16.8)
        x /= scale;
    // Empty vector: loop body runs zero times; no-op. Correct by construction.
}

// ─── TASK 6: buildRamp — push_back + reserve + return-by-value ───────────────
// Three distinct vector idioms in one function:
//
//   reserve(n)    — allocates capacity upfront so the n push_backs below
//                   don't trigger repeated reallocations (notes 16.10, 16.11).
//                   It does NOT change the length — v.size() is still 0 here.
//
//   push_back(i)  — appends one element, increments length (notes 16.11).
//
//   return result — returning a local vector by value is efficient: modern C++
//                   uses Named Return Value Optimization (NRVO) or move semantics
//                   to transfer storage rather than copying every element (16.5).
std::vector<int> buildRamp(int n)
{
    std::vector<int> result {};

    if (n <= 0)                // edge case: non-positive n -> empty vector
        return result;

    result.reserve(static_cast<std::size_t>(n));  // KEY: reserve BEFORE push_back (notes 16.10)
                                                   // cast: n is int, reserve takes size_t (16.3)
    for (int i { 0 }; i < n; ++i)
        result.push_back(i);   // KEY: push_back grows length one at a time (notes 16.11)

    return result;             // KEY: return-by-value is safe & efficient (notes 16.5)
}

// ─── TASK 7: lastReading — .at() safe subscript ──────────────────────────────
// Notes 16.2 / 16.3 contrast:
//   v[i]    — unchecked operator[]: undefined behavior if i >= v.size()
//   v.at(i) — bounds-checked: throws std::out_of_range if i >= v.size()
//
// Here the assert already guarantees non-empty (so index size-1 is valid), but
// using .at() is belt-and-suspenders and shows the idiom explicitly. The
// subtraction is safe because size() >= 1 is ensured by the assert.
int lastReading(const std::vector<int>& v)
{
    assert(!v.empty());        // precondition guard (notes 16.4 pattern)
    return v.at(v.size() - 1);  // KEY: .at() with bounds check (notes 16.2, 16.3)
}

} // namespace sensor
