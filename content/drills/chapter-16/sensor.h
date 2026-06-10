// ============================================================================
//  sensor.h  —  PUBLIC INTERFACE of the sensor-readings toolkit  (Chapter 16)
// ----------------------------------------------------------------------------
//  This header is COMPLETE and PROVIDED. You do not edit it. It declares the
//  seven functions that make up the toolkit; the bodies live in a separate
//  .cpp file (starter/sensor.cpp) that you implement.
//
//  Why a header? The same reason as Chapter 2's geo.h: the GRADER (tests/tests.cpp)
//  must share a common contract with both your implementation and the reference.
//  Change a signature here and nothing compiles. (CS6340 / LLVM lens: every LLVM
//  pass you write follows exactly this pattern — Foo.h declares the API, Foo.cpp
//  defines the bodies.)
//
//  Header guard (Chapter 2): stops the file from being pasted in twice per build.
// ============================================================================

#ifndef SENSOR_H
#define SENSOR_H

#include <vector>   // std::vector — the star of Chapter 16

// ─────────────────────────────────────────────────────────────────────────────
// All functions live in namespace sensor to match project naming conventions
// (Chapter 2 / 7 — namespaces keep names from colliding).
// ─────────────────────────────────────────────────────────────────────────────

namespace sensor
{

// ─── TASK 1: totalReading ─────────────────────────────────────────────────────
// Sum all readings in `v`. Return 0 for an empty vector (defined behavior:
// the sum of nothing is zero, by convention — state this in your mind before
// coding, and note it applies across ALL the statistics here).
//
// Pass-by-const-reference (notes 16.4): the caller's vector is NOT copied.
// Use a range-based for loop (notes 16.8): `for (const auto& x : v)`.
// Return type: int (readings are whole-number sensor counts).
int totalReading(const std::vector<int>& v);

// ─── TASK 2: averageReading ───────────────────────────────────────────────────
// Return the average of all readings as a double.
// IMPORTANT: return 0.0 for an empty vector (documented precondition).
//
// The static_cast-before-divide lesson from Chapter 4 / Chapter 10 applies:
//   average = static_cast<double>(sum) / static_cast<double>(v.size())
// Without the cast, integer division would silently truncate (e.g. 7/3 == 2).
// This is the same trap as the Ch10 statkit grade-book.
double averageReading(const std::vector<int>& v);

// ─── TASK 3: maxReading ───────────────────────────────────────────────────────
// Return the maximum element in `v`.
// PRECONDITION (documented): v must be non-empty. Calling this on an empty
// vector is a programmer error; an assert guards it (Chapter 9 idiom).
// Use an index-based loop (`for (std::size_t i{0}; i < v.size(); ++i)`) so
// you can practice the SIGNED/UNSIGNED indexing lesson (notes 16.3).
int maxReading(const std::vector<int>& v);

// ─── TASK 4: countAbove ───────────────────────────────────────────────────────
// Count how many elements of `v` are strictly greater than `threshold`.
// Empty vector -> 0 (no elements to count; loop body never runs).
// Use a range-based for loop (notes 16.8).
int countAbove(const std::vector<int>& v, int threshold);

// ─── TASK 5: normalizeInPlace ─────────────────────────────────────────────────
// Divide every element of `v` by `scale` IN PLACE (mutating the vector).
// Empty vector: no-op (loop body never runs).
//
// KEY LESSON (notes 16.8): to MUTATE elements through a range-for, declare
// the loop variable as `auto& x` (a non-const reference):
//   for (auto& x : v) { x /= scale; }
// Without the `&`, `x` is a copy and writes are silently thrown away.
// `v` is taken by (non-const) reference so mutations persist for the caller.
void normalizeInPlace(std::vector<double>& v, double scale);

// ─── TASK 6: buildRamp ────────────────────────────────────────────────────────
// Return a vector of `n` integers {0, 1, 2, ..., n-1} built with push_back.
// If n == 0, return an empty vector.
//
// This task demonstrates:
//   1. Constructing an empty vector and growing it with push_back (notes 16.11).
//   2. Using reserve(n) BEFORE the loop to avoid repeated reallocation
//      (notes 16.10 / 16.11 — reserve changes capacity, not length).
//   3. Returning a vector by value — efficient thanks to move semantics
//      or NRVO (notes 16.5).
//   Return type: `std::vector<int>` returned by value (NOT an out-parameter).
std::vector<int> buildRamp(int n);

// ─── TASK 7: lastReading ──────────────────────────────────────────────────────
// Return the last element of `v` using the SAFE accessor `.at()`.
// PRECONDITION (documented): v must be non-empty. An assert guards it.
//
// `.at(index)` vs `operator[]` (notes 16.2, 16.3):
//   v[i]   — unchecked: undefined behavior on out-of-bounds index
//   v.at(i)— checked:   throws std::out_of_range on invalid index
// For learning/debugging, `.at()` is a helpful guard. For performance-critical
// inner loops the index validity is already known and `[]` is used.
// Here the precondition says non-empty; using .at() is belt-and-suspenders.
int lastReading(const std::vector<int>& v);

} // namespace sensor

#endif // SENSOR_H
