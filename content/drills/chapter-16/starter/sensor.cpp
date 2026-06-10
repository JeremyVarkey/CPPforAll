// Chapter 16 — Dynamic Arrays: std::vector · Sensor-Readings Toolkit  (STARTER)
// ─────────────────────────────────────────────────────────────────────────────
// Fill in the seven TASK blocks below. Each maps 1:1 to a task in the README
// and to a declaration in ../sensor.h. The bodies currently return PLACEHOLDERS
// so the file compiles immediately — that's why `make test` is RED right now.
// Your job is to turn it GREEN by implementing each function.
//
//     make build        compile your code  (should already pass)
//     make test         grade it           (RED until you fill these in)
//     make test-solution verify the reference passes (always green)
//
// READING GUIDE — absorb the comments before writing code. Each block:
//   • names the KEY CONCEPT from the notes,
//   • calls out any TRAP (signed/unsigned, empty-vector edge, wrong loop style),
//   • gives just enough of a HINT to get unstuck without spoiling the answer.

#include "../sensor.h"
#include <cassert>   // assert — Chapter 9 precondition guard

namespace sensor
{

// ─── TASK 1: totalReading ─────────────────────────────────────────────────────
// KEY CONCEPT: range-based for loop (notes 16.8).
//   Use: `for (const auto& x : v)` — `const auto&` avoids copying each element
//   while making clear you only READ it. Accumulate into a running `sum`.
// EMPTY EDGE: return 0 — the loop body runs zero times (notes 16.8: "if the
//   container is empty, the body runs zero times. No special case needed.")
//
//   >>> YOUR CODE HERE <<<
//
int totalReading(const std::vector<int>& /*v*/)
{
    return 0;   // placeholder — correct only when v is empty
}
// ─────────────────────────────────────────────────────────────────────────────

// ─── TASK 2: averageReading ───────────────────────────────────────────────────
// KEY CONCEPT: static_cast-before-divide (notes 10 / this chapter's tie-in).
//   Dividing two integers in C++ does INTEGER DIVISION (truncates). Convert at
//   least one side to double BEFORE dividing:
//     static_cast<double>(sum) / static_cast<double>(v.size())
// SIGNED/UNSIGNED TRAP: v.size() returns std::size_t (unsigned). Dividing a
//   double by a std::size_t is fine — the implicit conversion to double is safe
//   here because size is always non-negative (notes 16.3).
// EMPTY EDGE: return 0.0 — documented behavior; check v.empty() first.
//
//   >>> YOUR CODE HERE <<<
//
double averageReading(const std::vector<int>& /*v*/)
{
    return 0.0;   // placeholder — correct only when v is empty
}
// ─────────────────────────────────────────────────────────────────────────────

// ─── TASK 3: maxReading ───────────────────────────────────────────────────────
// KEY CONCEPT: index-based loop with std::size_t (notes 16.3, 16.6, 16.7).
//   Use: `for (std::size_t i{0}; i < v.size(); ++i)` — matching types avoids the
//   signed/unsigned comparison warning. Start `maxVal` at v[0], then update.
// PRECONDITION: v must be non-empty — guard with assert (notes 16.4).
// WHY std::size_t HERE SPECIFICALLY? To practice the unsigned-index lesson.
//   Notes 16.3 says: if you need the index, use std::size_t for the counter
//   and index the vector with the same type. No cast needed.
//
//   >>> YOUR CODE HERE <<<
//
int maxReading(const std::vector<int>& v)
{
    assert(!v.empty());   // precondition: caller guarantees v is non-empty
    return v[0];          // placeholder — always returns the first element
}
// ─────────────────────────────────────────────────────────────────────────────

// ─── TASK 4: countAbove ───────────────────────────────────────────────────────
// KEY CONCEPT: range-based for loop with a conditional (notes 16.8).
//   Use: `for (const auto& x : v)` — accumulate a count inside `if (x > threshold)`.
// EMPTY EDGE: loop body never runs -> count stays 0. No special case.
//
//   >>> YOUR CODE HERE <<<
//
int countAbove(const std::vector<int>& /*v*/, int /*threshold*/)
{
    return 0;   // placeholder — correct only when v is empty or no elements > threshold
}
// ─────────────────────────────────────────────────────────────────────────────

// ─── TASK 5: normalizeInPlace ─────────────────────────────────────────────────
// KEY CONCEPT: `auto&` (non-const reference) in range-for for MUTATION (notes 16.8).
//   Use: `for (auto& x : v) { x /= scale; }`
//   The `&` is CRITICAL: without it `x` is a copy and writes are discarded.
//   The function signature already takes `v` by non-const reference so the
//   mutation is visible to the caller.
// TRAP: if scale == 0.0, the divide is not a crash but a garbage result:
//   floating-point division by zero is well-defined under IEEE-754 and yields
//   inf or nan (it is INTEGER division by zero that is undefined behavior).
//   A real toolkit would still guard scale==0; the tests here never pass it.
// EMPTY EDGE: loop body never runs -> no-op. No special case needed.
//
//   >>> YOUR CODE HERE <<<
//
void normalizeInPlace(std::vector<double>& /*v*/, double /*scale*/)
{
    // placeholder — does nothing (v is unchanged)
}
// ─────────────────────────────────────────────────────────────────────────────

// ─── TASK 6: buildRamp ────────────────────────────────────────────────────────
// KEY CONCEPT: push_back + reserve + return-by-value (notes 16.10, 16.11, 16.5).
//
//   Step 1 — reserve(n): tell the vector how many elements are coming so it
//     allocates once instead of repeatedly (notes 16.10):
//       result.reserve(static_cast<std::size_t>(n));
//   Step 2 — fill with push_back in a loop 0..n-1:
//       for (int i{0}; i < n; ++i) result.push_back(i);
//   Step 3 — return by value (move semantics / NRVO makes this efficient, 16.5).
//
// SIGNED/UNSIGNED: `n` is int; reserve takes std::size_t — cast before passing.
//   The notes say: "cast at the boundary, not casually everywhere" (16.3).
// EMPTY EDGE: if n <= 0, skip the loop and return the (empty) vector.
//
//   >>> YOUR CODE HERE <<<
//
std::vector<int> buildRamp(int /*n*/)
{
    return {};   // placeholder — always returns an empty vector
}
// ─────────────────────────────────────────────────────────────────────────────

// ─── TASK 7: lastReading ──────────────────────────────────────────────────────
// KEY CONCEPT: .at() for safe bounds-checked access (notes 16.2, 16.3).
//   Use: `v.at(v.size() - 1)` — returns the last element OR throws if out of range.
//   Compare: `v[v.size() - 1]` works too but is unchecked.
// SIGNED/UNSIGNED: v.size() returns std::size_t; subtracting 1 is safe here
//   because the assert guarantees v is non-empty (so size >= 1).
// PRECONDITION: v must be non-empty — guard with assert.
//
//   >>> YOUR CODE HERE <<<
//
int lastReading(const std::vector<int>& v)
{
    assert(!v.empty());   // precondition: caller guarantees v is non-empty
    return v[0];          // placeholder — returns first element, not last
}
// ─────────────────────────────────────────────────────────────────────────────

} // namespace sensor
