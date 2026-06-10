# Chapter 18 — Iterators and Algorithms: VecTools

> Reinforces [`../../notes/chapter-18.md`](../../notes/chapter-18.md) · LearnCpp [Chapter 18](https://www.learncpp.com/cpp-tutorial/introduction-to-iterators/)

## The project

You are completing **vectools**, a small library that performs five common
operations on `std::vector<int>`. For each operation you implement **two
versions** that must agree on every input:

- A **`hand*`** version that walks the vector with an explicit **iterator
  loop** — you write the `begin()`/`end()`/`++it`/`*it` machinery yourself so
  you see the abstraction in full.
- An **`algo*`** version that delegates to a **standard `<algorithm>`**
  (`std::find`, `std::count_if`, `std::sort`, `std::max_element`), passing a
  free-function predicate or comparator.

The grader confirms that both versions agree, so a mismatch in either direction
is caught immediately. The "two ways, same result" structure is the lab's
physical demonstration of Chapter 18's central lesson: the standard library
algorithms are not magic — they run the same iterator loop you would have written
by hand, just packaged with a name.

**Why free functions, not lambdas?** Lambdas are the natural next step as
predicates, but they are introduced in Chapter 20. The Chapter 18 way is a plain
free function whose address decays to a function pointer:
```cpp
bool isEven(int n) { return n % 2 == 0; }
std::count_if(v.begin(), v.end(), vectools::isEven);  // function pointer
```
This is also historically how all C++ algorithm predicates worked before C++11.
Lambdas will make this more concise in Chapter 20; for now, the free-function form
is what you use.

## Concepts practiced

- **Iterator objects** — `begin()`, `end()`, dereference `*it`, advance `++it`,
  sentinel test `it != end()` (notes 18.2)
- **Half-open range** `[begin, end)` — `begin()` is valid, `end()` must not be
  dereferenced (notes 18.2)
- **`std::find`** — searches a range; returns an iterator or `end()` (notes 18.3)
- **`std::count_if`** — counts predicate matches using a free-function predicate
  (notes 18.3)
- **`std::sort`** — default ascending and custom descending via a free-function
  comparator; strict-weak-ordering rule (notes 18.3)
- **`std::max_element`** — returns an iterator; **empty-range guard** required.
  `max_element` itself is not in the note; the iterator-return + `end()`-guard
  pattern it shares with `std::find` is (notes 18.2)
- **Function pointers as predicates/comparators** — free functions passed by name
  to algorithms (Chapter 18 scope; lambdas arrive Chapter 20)
- **`std::distance`** — converts an iterator to an index (Chapter 18 / 17 pointer arithmetic)
- Reused from earlier chapters: `std::vector` construction and `.begin()`/`.end()`
  (Ch 16), `const` references and pass-by-ref (Ch 12), namespaces and header/source
  split (Ch 2), `for` loops (Ch 8)

## Your tasks

The starter compiles and runs immediately (`make build` is green), but every
function returns a wrong placeholder, so `make test` is **RED**. Fill in the
`>>> YOUR CODE HERE <<<` blocks in [`starter/vectools.cpp`](starter/vectools.cpp).
Each task label below maps 1:1 to a `TASK` block in the starter:

**Task 1a — `isEven(n)`** — return `true` iff `n` is divisible by 2. This is the
**predicate** passed by name to `std::count_if` in Task 3b. Negative numbers
count too: `isEven(-4) == true`.

**Task 1b — `greaterThan(a, b)`** — return `true` iff `a > b`. This is the
**comparator** passed by name to `std::sort` in Task 4b. **Use strictly `>`
— not `>=`**: equal elements must not claim to come before each other
(strict-weak-ordering rule; violating it is undefined behaviour in `std::sort`).

**Task 2a — `handFind(vec, target)`** — search `vec` for `target` using an
**explicit iterator loop**; return the 0-based index of the first match, or `-1`
if not found. Write the loop with `auto it { vec.begin() }; … ++it; *it`. Convert
the winning iterator to an index with `std::distance(vec.begin(), it)`. Do NOT
call any `<algorithm>`.

**Task 2b — `algoFind(vec, target)`** — same contract as `handFind`; implement by
calling `std::find(vec.begin(), vec.end(), target)`. The returned iterator must be
checked against `vec.end()` before dereferencing (notes 18.2).

**Task 3a — `handCountEven(vec)`** — count even elements using an **explicit
iterator loop**; test `*it % 2 == 0`; tally manually. Do NOT call any
`<algorithm>`.

**Task 3b — `algoCountEven(vec)`** — same contract; call
`std::count_if(vec.begin(), vec.end(), vectools::isEven)`. Pass the **free
function** `isEven` (no lambdas — Chapter 20). Cast the `std::ptrdiff_t` return
to `int`.

**Task 4a — `sortAscending(vec)`** — sort the vector in ascending order using
`std::sort(vec.begin(), vec.end())`. The default comparator uses `operator<`.

**Task 4b — `sortDescending(vec)`** — sort descending by calling
`std::sort(vec.begin(), vec.end(), vectools::greaterThan)`. Pass the **free
function** `greaterThan` as the comparator.

**Task 5 — `largest(vec)`** — return the largest element, or `0` for an empty
vector. Call `std::max_element(vec.begin(), vec.end())`. **Guard the empty-range
trap**: the returned iterator equals `vec.end()` when the vector is empty —
dereferencing it is undefined behaviour. Check first; return the `0` fallback.

You do **not** edit `vectools.h` or `tests/tests.cpp` — only `starter/vectools.cpp`.

## Constraints

**Allowed:** `std::vector`, iterator objects (`.begin()`, `.end()`, `++it`, `*it`,
`it != end`), `<algorithm>` algorithms, `std::distance`, free-function predicates
and comparators, everything from Chapters ≤ 18.

**Required idioms:**
- `handFind` and `handCountEven` must use explicit iterator loops — no `<algorithm>`
  inside those two functions.
- `algoCountEven` must pass `vectools::isEven` as the predicate (a free function
  pointer).
- `sortDescending` must pass `vectools::greaterThan` as the comparator.
- `largest` must guard the empty-range case before dereferencing `std::max_element`'s
  return value.

**Forbidden (not taught yet):**
- **Lambdas** — Chapter 20. Use free functions (`isEven`, `greaterThan`) instead.
- `std::function` — Chapter 20.
- C++20 ranges (`std::ranges::sort`, etc.) — out of scope here.
- `new`/`delete` — Chapter 19.

## Build & run

```sh
make            # compile-check starter/vectools.cpp  (warning-clean)
make test       # grade your code against the unit tests  <-- the main goal
make solution   # run the grader against the reference if you get stuck
make test-solution   # verify the reference solution (must be green)
make clean      # remove build artefacts
```

## Success criteria

`make test` prints **PASS ✅  all vectools checks passed.**

The grader tests each function family in depth:

- `isEven` and `greaterThan` are tested directly before being used in algorithms.
- `handFind` / `algoFind` are checked against each other (they must agree on
  every input), and edge-tested: empty vector, single element, duplicates (first
  occurrence wins), target absent.
- `handCountEven` / `algoCountEven` are cross-checked: all-even, all-odd, empty,
  single element, negative numbers (`-4` is even).
- `sortAscending` / `sortDescending` are checked element-by-element: already-sorted
  input, all-same elements, single element, empty vector.
- `largest` handles: normal case, single element, empty vector (must return `0`,
  not crash), all-same, all-negative, mixed signs.

Until then `make test` prints `FAIL ❌` and lists each failing check with its
line number.

## Hints

<details><summary>Tasks 1a & 1b — isEven and greaterThan</summary>

Both are one-liners:
```cpp
bool isEven(int n)   { return n % 2 == 0; }
bool greaterThan(int a, int b) { return a > b; }
```
The `greaterThan` placeholder returns `false` for everything, so `sortDescending`
will leave the vector in its original order — a clear test failure.
</details>

<details><summary>Tasks 2a & 3a — the iterator loop (handFind, handCountEven)</summary>

The canonical iterator loop from notes 18.2:
```cpp
for (auto it { vec.begin() }; it != vec.end(); ++it)
{
    // *it is the current element
}
```
For `handFind`, once `*it == target`, convert to an index:
```cpp
return static_cast<int>(std::distance(vec.begin(), it));
```
`std::distance` counts the steps from `begin` to `it` — for a `std::vector`
iterator this is O(1) because vector iterators support random access.
</details>

<details><summary>Task 2b — checking the std::find iterator before using it (algoFind)</summary>

```cpp
auto it { std::find(vec.begin(), vec.end(), target) };
if (it == vec.end())
    return -1;                   // not found — do NOT dereference here
return static_cast<int>(std::distance(vec.begin(), it));
```
Always check `it != vec.end()` before writing `*it`. Dereferencing `end()` is
undefined behaviour — the iterator points one past the last element, not at any
real value (notes 18.2).
</details>

<details><summary>Task 3b — passing a free function to std::count_if</summary>

```cpp
int algoCountEven(const std::vector<int>& vec)
{
    return static_cast<int>(
        std::count_if(vec.begin(), vec.end(), vectools::isEven));
}
```
`vectools::isEven` decays to a `bool(*)(int)` function pointer. That is the
Chapter 18 predicate form — lambdas (`[](int n){ return n%2==0; }`) do the same
thing but aren't introduced until Chapter 20.
</details>

<details><summary>Task 1b — strict-weak-ordering in the comparator (greaterThan)</summary>

`std::sort` requires the comparator to be a strict weak ordering: `comp(x, x)`
must always return `false` (irreflexivity). Using `>=` breaks this: when `a == b`,
`a >= b` is `true`, meaning `a` claims to come before `b` and `b` claims to come
before `a` simultaneously. That contradiction gives `std::sort` undefined behaviour.

Use strictly `>`:
```cpp
bool greaterThan(int a, int b) { return a > b; }
```
When `a == b`, `a > b` is `false` — neither claims priority. Correct.
</details>

<details><summary>Task 5 — the empty-range guard for std::max_element (largest)</summary>

```cpp
int largest(const std::vector<int>& vec)
{
    auto it { std::max_element(vec.begin(), vec.end()) };
    if (it == vec.end())
        return 0;    // empty range — fallback; do NOT dereference
    return *it;
}
```
When `vec` is empty, `vec.begin() == vec.end()` and `std::max_element` returns
`vec.end()` immediately. The placeholder `return 0` is accidentally correct for
the empty case but wrong for everything else — the grader has non-empty cases that
will still fail.
</details>

## Stretch goals (optional — some need later chapters)

- Add `handMin` and `algoMin` (using `std::min_element`) and test them the same
  way. Reinforce the empty-range guard.
- Add `algoContains(vec, target)` returning `bool`, implemented with `std::find`.
  Once you have it, observe that `handFind(v, t) != -1` is equivalent — exactly
  what the standard library's `std::ranges::contains` (C++23) does under the hood.
- Add `handFindIf` and `algoFindIf` using `std::find_if` with a free-function
  predicate of your choice. This previews `std::find_if` from notes 18.3.
- Add a `printAll(vec)` that uses a range-based `for` loop (Ch 16) — then rewrite
  it as `std::for_each` with a free function, and compare readability. Notes 18.3
  addresses when `std::for_each` adds value vs. when a plain loop is clearer.
- Once you reach Chapter 20 (lambdas), rewrite `algoCountEven` and `sortDescending`
  to use inline lambda predicates instead of free functions. Notice that the call
  sites become single self-contained expressions — that is the payoff lambdas deliver.
