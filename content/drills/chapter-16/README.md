# Chapter 16 — Dynamic Arrays: `std::vector` · Project: Sensor-Readings Toolkit

> Reinforces [`../../notes/chapter-16.md`](../../notes/chapter-16.md) · LearnCpp [Chapter 16](https://www.learncpp.com/)

## The project

Sensor telemetry is everywhere in systems software: a coverage-guided fuzzer
accumulates branch-hit counts, a profiler tracks per-function instruction
counts, a load balancer logs request latency readings. All of those are
**vectors of numbers** at the core. In this lab you build the **sensor-readings
toolkit** — a small library of seven functions over `std::vector<int>` and
`std::vector<double>` — while touching every key `std::vector` idiom from
Chapter 16.

Every function in the toolkit is **pure** (arguments in, value out, no I/O)
so the grader can check exact answers. That separation also mirrors how the
chapter notes describe real CS6340 code: "if you do not need the index, do not
invent one" — and a pure function never invents one just because a `for` loop
is nearby.

## Concepts practiced

**New this chapter (Chapter 16):**
- `std::vector<T>` declaration, list-init, and subscript access (16.2)
- **Passing by `const` reference** (`const std::vector<int>&`) for read-only
  functions; passing by **non-const reference** for mutation (16.4)
- **Returning by value** — efficient thanks to NRVO / move semantics (16.5)
- **`std::size_t`** indexing in a count loop and the signed/unsigned warning
  (notes 16.3, 16.6, 16.7)
- **Range-based `for`** in the two forms this lab uses (notes 16.8 decision table):
  - `for (const auto& x : v)` — read-only, no copy
  - `for (auto& x : v)` — mutate in place (the alias-vs-copy distinction)
- `push_back` to grow a vector element by element (notes 16.11)
- `reserve(n)` to pre-allocate capacity and avoid repeated reallocations
  (notes 16.10, 16.11)
- `.at(i)` vs `operator[]` — bounds-checked vs unchecked access (notes 16.2, 16.3)

**Reused from earlier chapters:**
- `static_cast<double>(...)` before integer division — the Ch 4 / Ch 10 trap
- `assert` for programmer preconditions (Ch 9)
- Header guard + namespace (Ch 2, Ch 7)
- `auto` for type deduction in loop variables (Ch 10)
- **Function composition** (Ch 2): `averageReading` calls `totalReading` rather
  than re-summing — a plain function call, not a template. (Both are ordinary
  non-template functions over `std::vector<int>`.)

## Your tasks

The starter compiles immediately but every function returns a **placeholder**, so
`make test` starts **RED**. Fill in the seven `>>> YOUR CODE HERE <<<` blocks in
[`starter/sensor.cpp`](starter/sensor.cpp). They ramp from warm-up to capstone:

1. **`totalReading(v)`** — Sum all readings. Use a range-for with `const auto&`.
   Empty vector returns `0` (the loop body runs zero times automatically).

2. **`averageReading(v)`** — Return the mean as a `double`. Check for empty (return
   `0.0`). **Cast before dividing:** `static_cast<double>(sum) / static_cast<double>(v.size())`.
   Without the cast you get truncating integer division.

3. **`maxReading(v)`** — Return the maximum element. Use an **index-based loop**
   with `std::size_t` as the counter (`for (std::size_t i{0}; i < v.size(); ++i)`)
   so you practice the unsigned-index lesson. Seed `maxVal` with `v[0]`, update
   from index 1 onward. The assert guards the non-empty precondition.

4. **`countAbove(v, threshold)`** — Count how many elements are **strictly greater
   than** `threshold`. Range-for + `if`. Empty vector → `0` (automatic).

5. **`normalizeInPlace(v, scale)`** — Divide every element by `scale` **in place**.
   Use `for (auto& x : v)` — the `&` is the whole lesson. Without it, `x` is a
   copy and every write is silently discarded. `v` is already a non-const ref.

6. **`buildRamp(n)`** — Return `{0, 1, 2, …, n-1}`. Call `reserve(n)` first,
   then push each value with `push_back`. Return the vector by value. If `n <= 0`
   return an empty vector.

7. **`lastReading(v)`** — Return the last element using **`.at(v.size() - 1)`**
   (the safe, bounds-checked accessor). The assert already guarantees non-empty.
   Shows `.at()` vs `operator[]` in concrete use.

You may **not** edit [`sensor.h`](sensor.h) or anything in `tests/` — those are
the contract and the grader. Edit only `starter/sensor.cpp`.

## Constraints

**Allowed:** `std::vector`, range-based `for`, index `for` with `std::size_t`,
`push_back`, `reserve`, `.at()`, `operator[]`, `.size()`, `.empty()`,
`static_cast`, `assert`, `const auto&`, `auto&`, `return <vector>` by value,
functions from earlier chapters.

**Forbidden (not taught yet):**
- `std::array` or C-style arrays (Ch 17)
- Explicit iterators `.begin()` / `.end()` (Ch 18)
- `<algorithm>` (`std::max_element`, `std::accumulate`, etc.) (Ch 18) —
  the hand-rolled loops ARE the lesson
- `new[]` / `delete[]` (Ch 19)
- Lambdas (Ch 20)

**Required idioms:**
- Task 3 must use an index-based loop with `std::size_t` (sign lesson).
- Task 5 must use `for (auto& x : v)` — not a separate variable or index.
- Task 6 must call `reserve` before the `push_back` loop.
- Task 7 must use `.at()`, not `operator[]`.

## Build & run

```sh
make               # compile-check starter/sensor.cpp
make test          # grade your code  ->  RED until TASK blocks are filled in
make test-solution # verify the reference (always green)
make solution      # compile-check the reference implementation
make clean         # remove build artifacts
```

`make test` is the grader. There is no interactive driver for this lab — the
functions are pure, and all the interesting behavior shows up in the test output.

## Success criteria

`make test` prints **PASS ✅  all sensor checks passed.** and exits `0`. Until
then it prints one `FAIL: …  @line N` per broken check. The grader exercises
every function across normal inputs AND the edge cases that separate correct code
from plausible-looking near-misses:

- `totalReading({})` — empty-vector edge (loop body must run zero times)
- `averageReading({3, 4})` — returns `3.5`, **not** `3` (the integer-division trap;
  missing `static_cast` fails this check)
- `maxReading({-10, -3, -7})` — max of **negative** readings is `-3`
- `countAbove({5, 10, 15, 20}, 5)` — `5` is **not** strictly above `5`; only 3 pass
- `normalizeInPlace` after the call — values must have changed in the caller's
  vector (missing `&` in `auto& x` means they won't)
- `buildRamp(0)` / `buildRamp(-5)` — both return empty
- `buildRamp(100)` — size must be exactly 100 (not 101, not capacity confusion)
- `lastReading({1, 99})` — returns `99`, not `1` (wrong-element trap)
- `lastReading({99, 1})` — returns `1`, not `99` (larger ≠ last)

## Hints

<details><summary>Task 1 — range-for pattern for accumulation</summary>

```cpp
int sum { 0 };
for (const auto& x : v)   // const auto& = read each element without copying
    sum += x;
return sum;
```

`const auto&` is the right qualifier for reading elements of any type without
copying them (notes 16.8 decision table). For `int` the copy cost is tiny, but
building the habit here pays off on heavier types.
</details>

<details><summary>Task 2 — the cast-before-divide rule</summary>

```cpp
if (v.empty()) return 0.0;
int sum { totalReading(v) };   // reuse Task 1
return static_cast<double>(sum) / static_cast<double>(v.size());
```

The trap: `7 / 2` in C++ is `3` (truncates). You must cast to `double` before
the `/` operator runs. Casting the result afterward is too late — the truncation
has already happened. (Integer-division truncation is the Ch 4 / Ch 10 lesson —
the same trap as Ch 10's statkit — not a Chapter 16 topic.)
</details>

<details><summary>Task 3 — seeding max and the std::size_t loop</summary>

```cpp
assert(!v.empty());
int maxVal { v[0] };
for (std::size_t i { 1 }; i < v.size(); ++i)
{
    if (v[i] > maxVal) maxVal = v[i];
}
return maxVal;
```

Start the loop at index `1` (you already used `v[0]` as the seed). Using
`std::size_t` for `i` matches `v.size()`'s type so there's no
signed/unsigned comparison warning (notes 16.3, 16.7 Option 1).
</details>

<details><summary>Task 4 — range-for with a conditional</summary>

```cpp
int count { 0 };
for (const auto& x : v)
{
    if (x > threshold) ++count;
}
return count;
```

"Strictly greater than" means `>`, not `>=`. If the grader has `{5, 10}` and
threshold `5`, only `10` passes the test; `5 > 5` is false.
</details>

<details><summary>Task 5 — auto& is the whole lesson</summary>

```cpp
for (auto& x : v)   // & makes x an alias for the real element
    x /= scale;
```

`auto x` (no `&`) would make `x` a copy. The division would happen on that
throwaway copy; the vector's storage would not change. The `&` makes `x` a
reference (an alias) for the actual element in memory. This is the Chapter 16.8
mutation pattern (notes decision table: `auto& value` → "mutate each element").
</details>

<details><summary>Task 6 — reserve then push_back</summary>

```cpp
std::vector<int> result {};
if (n <= 0) return result;
result.reserve(static_cast<std::size_t>(n));  // capacity without length change
for (int i { 0 }; i < n; ++i)
    result.push_back(i);
return result;
```

`reserve` changes **capacity** (internal storage) but NOT **length** (`size()`
stays 0 after reserve). Only `push_back` / `resize` change the length. Confusing
the two is the classic bug: `result.reserve(5); result[0] = 0;` is UB because the
vector still has zero elements (notes 16.10).
</details>

<details><summary>Task 7 — .at() vs operator[]</summary>

```cpp
assert(!v.empty());
return v.at(v.size() - 1);
```

`v.at(i)` checks that `i < v.size()` at runtime and throws `std::out_of_range`
if not. `v[i]` skips that check — undefined behavior if `i` is out of bounds.
Here the assert already makes the index valid, so `.at()` is belt-and-suspenders;
the goal is to show the idiom explicitly (notes 16.2, 16.3).
</details>

## Stretch goals (optional — some need later chapters)

- Add a **`minReading`** that mirrors `maxReading` (same pattern, different comparison).
- Add a **`medianReading`** — sort a copy of `v` and return the middle element.
  Sorting requires `<algorithm>` (Chapter 18 — a preview).
- Replace the `int` toolkit with a **function template** `totalReading<T>` so it
  works on both `std::vector<int>` and `std::vector<double>` (Chapter 11/26 preview).
- Wire `buildRamp` up to a real sensor device mock: `buildRamp(1000)` then run
  `countAbove(readings, threshold)` to compute a simple threshold-crossing alert rate.
  This is essentially what a fuzzer's coverage-hit counter does: count how many
  branch edges exceeded a hit threshold.
- Refactor `normalizeInPlace` into a **returning** `normalize` that takes a
  `const std::vector<double>&` and returns a new normalized vector — comparing
  the in-place vs copy-and-return tradeoffs (notes 16.5).
