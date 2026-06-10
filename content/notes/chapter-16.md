# Chapter 16 — Dynamic Arrays: `std::vector`

> Source: <https://www.learncpp.com/> (Chapter 16)
> One file per chapter. Each lesson is a section below.

## Contents
- [16.1 — Introduction to containers and arrays](#161--introduction-to-containers-and-arrays)
- [16.2 — Introduction to std::vector and list constructors](#162--introduction-to-stdvector-and-list-constructors)
- [16.3 — std::vector and the unsigned length and subscript problem](#163--stdvector-and-the-unsigned-length-and-subscript-problem)
- [16.4 — Passing std::vector](#164--passing-stdvector)
- [16.5 — Returning std::vector, and an introduction to move semantics](#165--returning-stdvector-and-an-introduction-to-move-semantics)
- [16.6 — Arrays and loops](#166--arrays-and-loops)
- [16.7 — Arrays, loops, and sign challenge solutions](#167--arrays-loops-and-sign-challenge-solutions)
- [16.8 — Range-based for loops (for-each)](#168--range-based-for-loops-for-each)
- [16.9 — Array indexing and length using enumerators](#169--array-indexing-and-length-using-enumerators)
- [16.10 — std::vector resizing and capacity](#1610--stdvector-resizing-and-capacity)
- [16.11 — std::vector and stack behavior](#1611--stdvector-and-stack-behavior)
- [16.12 — std::vector<bool>](#1612--stdvectorbool)
- [16.x — Chapter 16 summary and quiz](#16x--chapter-16-summary-and-quiz)

---

## 16.1 — Introduction to containers and arrays

### The variable scalability challenge

If you need to store three values, separate variables are tolerable:

```cpp
int a {};
int b {};
int c {};
```

If you need 3,000 values, separate variables collapse immediately. You need a
single object that can hold many elements.

### Containers

A **container** is an object that stores a collection of unnamed elements.

```cpp
std::vector<int> scores { 90, 85, 100 };
```

Diagram:

```
scores
+----+----+-----+
| 90 | 85 | 100 |
+----+----+-----+
  0    1     2       indices
```

The elements do not have their own variable names. You access them by position
or iteration.

### Length

The **length** of a container is how many elements it currently has.

```cpp
scores.size(); // 3
```

### Container operations

Common operations:

- create container,
- ask for length,
- access element,
- insert/add element,
- remove element,
- iterate through elements,
- search/sort/count.

### Element type

Containers have an element type:

```cpp
std::vector<int> numbers;
std::vector<std::string> seeds;
std::vector<CoveragePoint> points;
```

All elements in a `std::vector<T>` have the same type `T`.

### Arrays

An **array** is a container whose elements are arranged sequentially. C++ has
several array-like types:

| Type | Size | Notes |
|---|---|---|
| C-style array | fixed | inherited from C, sharp edges |
| `std::array` | fixed | safer fixed-size standard container |
| `std::vector` | dynamic | resizable array; this chapter's star |

For CS6340 Lab 1, `std::vector<std::string>` is the practical workhorse for seed
lists, mutant pools, coverage points, and test results.

---

## 16.2 — Introduction to std::vector and list constructors

### `std::vector`

`std::vector` is a dynamic array from the standard library. Include `<vector>`.

```cpp
#include <vector>

std::vector<int> values {};
```

It owns a contiguous sequence of elements and can grow/shrink at runtime.

### List initialization

```cpp
std::vector<int> primes { 2, 3, 5, 7, 11 };
```

Each initializer becomes one element.

```
primes
index:  0  1  2  3   4
value:  2  3  5  7  11
```

### Subscript operator

Access by index:

```cpp
std::cout << primes[0] << '\n'; // 2
primes[1] = 13;
```

Indexes are zero-based:

```
first element  -> index 0
second element -> index 1
last element   -> index size - 1
```

### Out-of-bounds access

`operator[]` does not check bounds:

```cpp
std::cout << primes[99]; // undefined behavior
```

Use `.at()` when you want runtime checking:

```cpp
std::cout << primes.at(99); // throws exception
```

For learning/debugging, `.at()` can be a helpful guard. For performance-critical
inner loops, code often uses `[]` after proving indexes are valid.

### Contiguous memory

`std::vector` stores elements contiguously:

```
vector<int> v { 10, 20, 30 }

memory:
+----+----+----+
| 10 | 20 | 30 |
+----+----+----+
  &v[0] &v[1] &v[2]
```

This is one reason vectors are fast for iteration and indexing.

### Constructing a vector of a specific length

```cpp
std::vector<int> zeros(5);      // 5 ints, value-initialized to 0
std::vector<int> sevens(5, 7);  // 5 ints, each 7
```

Be careful:

```cpp
std::vector<int> a { 5 };    // one element: 5
std::vector<int> b(5);       // five elements: 0 0 0 0 0
```

Brace initialization prefers list construction for non-empty lists.

### Const and constexpr vector

`const std::vector` is allowed:

```cpp
const std::vector<int> values { 1, 2, 3 };
```

But `std::vector` is generally not a constexpr-friendly compile-time container in
the beginner sense because it dynamically manages storage. Use it as a runtime
container.

---

## 16.3 — std::vector and the unsigned length and subscript problem

### The sign problem

`std::vector::size()` returns an unsigned size type, usually `std::size_t` or the
vector's `size_type`.

```cpp
std::vector<int> values { 1, 2, 3 };
auto length { values.size() }; // unsigned type
```

But many ordinary loop counters are signed `int`.

```cpp
for (int i { 0 }; i < values.size(); ++i) // signed/unsigned comparison warning
{
}
```

### Why this matters

Signed/unsigned conversions can surprise you:

```cpp
int i { -1 };
std::size_t size { 3 };

// i may convert to a huge unsigned value for comparison
```

### `std::ssize` (C++20)

`std::ssize(container)` returns a signed length.

```cpp
#include <iterator>

for (int i { 0 }; i < std::ssize(values); ++i)
{
    std::cout << values[static_cast<std::size_t>(i)] << '\n';
}
```

You still may need a cast when indexing, because `operator[]` expects an unsigned
index type.

### `operator[]` vs `.at()`

```cpp
values[index];    // no bounds check
values.at(index); // bounds check, throws on invalid index
```

Use `.at()` when you want safety during exploration. Use `[]` when index
validity is already guaranteed or performance matters.

### Best practical options

Avoid indexing when possible:

```cpp
for (const auto& value : values)
{
    std::cout << value << '\n';
}
```

If you need indexes:

```cpp
for (std::size_t i { 0 }; i < values.size(); ++i)
{
    std::cout << values[i] << '\n';
}
```

If you need signed math:

```cpp
for (int i { 0 }; i < std::ssize(values); ++i)
{
    auto index { static_cast<std::size_t>(i) };
    std::cout << values[index] << '\n';
}
```

CS6340 tie-in: fuzzer code often does signed random index generation, then uses
that index on a `std::string` or `std::vector`. Cast at the boundary, not
casually everywhere.

---

## 16.4 — Passing std::vector

### Do not pass vectors by value unless you want a copy

```cpp
void print(std::vector<int> values); // copies whole vector
```

For read-only use:

```cpp
void print(const std::vector<int>& values)
{
    for (const auto& value : values)
        std::cout << value << '\n';
}
```

For mutation:

```cpp
void appendSeed(std::vector<std::string>& seeds, std::string seed)
{
    seeds.push_back(seed);
}
```

### Different element types are different vector types

```cpp
std::vector<int>
std::vector<double>
std::vector<std::string>
```

These are distinct types. A function taking `const std::vector<int>&` does not
accept `std::vector<double>`.

### Generic templates

To accept vectors of different element types:

```cpp
template <typename T>
void printVector(const std::vector<T>& values)
{
    for (const auto& value : values)
        std::cout << value << '\n';
}
```

For this CS6340 note set, understand the pattern; you do not need to become a
template wizard yet.

### Asserting length

If a function expects a particular vector length, check it:

```cpp
#include <cassert>

void processRgb(const std::vector<int>& rgb)
{
    assert(rgb.size() == 3);
}
```

Assertions are for programmer assumptions during development, not user-facing
error handling.

---

## 16.5 — Returning std::vector, and an introduction to move semantics

### Copy semantics

Copying a vector copies its elements:

```cpp
std::vector<std::string> a { "x", "y" };
std::vector<std::string> b { a }; // copies strings
```

This can be expensive.

### Move semantics

Move semantics lets a destination take resources from a temporary/source object
instead of copying every element.

```
copy:
  duplicate all elements into new storage

move:
  transfer ownership of existing storage
```

Vector move intuition:

```
before move:
source -> [a][b][c]
dest   -> empty

after move:
source -> valid but unspecified/empty-ish
dest   -> [a][b][c]
```

### Returning vector by value is okay

```cpp
std::vector<std::string> makeSeeds()
{
    std::vector<std::string> seeds {};
    seeds.push_back("abc");
    seeds.push_back("xyz");
    return seeds;
}
```

Modern C++ can use copy elision or move semantics, so this is a normal pattern.

### Why pass-by-value and return-by-value differ

Passing by value copies an existing object into the function:

```cpp
void f(std::vector<int> values); // caller's vector copied unless moved
```

Returning by value often returns a local temporary-like object whose resources can
be moved/elided efficiently.

Do not use out parameters just because "vectors are expensive." Return-by-value
is often clean and efficient.

---

## 16.6 — Arrays and loops

### Arrays unlock scalable processing

Without arrays:

```cpp
std::cout << a << b << c;
```

With arrays/vectors:

```cpp
for (const auto& value : values)
{
    std::cout << value << '\n';
}
```

The same code handles 3 elements or 3,000 elements.

### Index-based loop

```cpp
for (std::size_t i { 0 }; i < values.size(); ++i)
{
    std::cout << values[i] << '\n';
}
```

### Range-based loop

```cpp
for (const auto& value : values)
{
    std::cout << value << '\n';
}
```

Use range-for when you do not need the index.

### Off-by-one errors

For a vector of length `n`, valid indexes are:

```
0 through n - 1
```

Invalid:

```cpp
for (std::size_t i { 0 }; i <= values.size(); ++i)
{
    std::cout << values[i]; // last iteration out of bounds
}
```

Correct:

```cpp
for (std::size_t i { 0 }; i < values.size(); ++i)
{
}
```

Half-open range:

```
[0, size)
```

---

## 16.7 — Arrays, loops, and sign challenge solutions

### The challenge

You want:

- signed loop counters because signed arithmetic is often safer,
- unsigned indexes because standard containers use unsigned size/index types,
- no warning soup,
- no accidental narrowing.

### Option 1: unsigned loop variable

```cpp
for (std::size_t i { 0 }; i < values.size(); ++i)
{
    std::cout << values[i] << '\n';
}
```

This works naturally with vector APIs. Be careful with reverse loops, because
unsigned values cannot go below zero.

Bad reverse pattern:

```cpp
for (std::size_t i { values.size() - 1 }; i >= 0; --i) // broken
{
}
```

### Option 2: signed loop variable with cast at index

```cpp
for (int i { 0 }; i < std::ssize(values); ++i)
{
    std::cout << values[static_cast<std::size_t>(i)] << '\n';
}
```

The cast is localized to the boundary where the vector requires its index type.

### Option 3: avoid indexing

Best when possible:

```cpp
for (const auto& value : values)
{
    std::cout << value << '\n';
}
```

LearnCpp calls this the only sane choice in many cases because it sidesteps the
sign/index problem entirely.

CS6340 tie-in:

```cpp
for (auto& seed : seeds)
{
    mutate(seed);
}
```

If you do not need the index, do not invent one.

---

## 16.8 — Range-based for loops (for-each)

### Basic range-for

```cpp
for (int value : values)
{
    std::cout << value << '\n';
}
```

Reads as:

```
for each value in values
```

### Empty containers

If the container is empty, the body runs zero times. No special case needed.

### Use `auto`

```cpp
for (auto value : values)
{
}
```

But `auto value` copies each element.

### Avoid copies with references

For expensive elements:

```cpp
for (const auto& seed : seeds)
{
    std::cout << seed << '\n'; // read-only, no copy
}
```

For mutation:

```cpp
for (auto& seed : seeds)
{
    seed += "!";
}
```

Decision table:

| Loop variable | Meaning |
|---|---|
| `auto value` | copy each element |
| `const auto& value` | read each element without copying |
| `auto& value` | mutate each element |
| `auto&& value` | advanced generic/range code |

Best default for read-only class/string elements:

```cpp
for (const auto& element : container)
```

### Getting the index

Range-for does not directly provide the index. If you need it, use an index loop
or maintain a counter:

```cpp
int index { 0 };
for (const auto& seed : seeds)
{
    std::cout << index << ": " << seed << '\n';
    ++index;
}
```

### Reverse range-for

C++20 has range adaptors for reverse iteration in newer ranges facilities, but a
plain beginner-friendly approach is still an index loop or reverse iterators.
Do not overcomplicate until needed.

---

## 16.9 — Array indexing and length using enumerators

### Enum indexes

Enums can name meaningful indexes:

```cpp
enum Stat
{
    testsRun,
    crashesFound,
    statCount,
};

std::vector<int> stats(statCount);
stats[testsRun] = 100;
stats[crashesFound] = 2;
```

This is more meaningful than:

```cpp
stats[0] = 100;
stats[1] = 2;
```

### Count enumerator

The final enumerator can represent the number of enum values:

```cpp
enum Campaign
{
    mutationA,
    mutationB,
    mutationC,
    campaignCount,
};
```

Then:

```cpp
std::vector<int> counts(campaignCount);
```

### Asserting length

If an array/vector must match the enum count, assert it:

```cpp
assert(counts.size() == campaignCount);
```

### Enum class friction

Scoped enums (`enum class`) do not implicitly convert to indexes:

```cpp
enum class Campaign { A, B, C, Count };
```

You need casts/helpers:

```cpp
auto index { static_cast<std::size_t>(Campaign::A) };
```

This is more type-safe but more verbose. For simple indexing, consider whether a
`std::map`, `std::array`, or dedicated struct would be clearer.

---

## 16.10 — std::vector resizing and capacity

### Fixed-size vs dynamic arrays

`std::vector` can resize at runtime:

```cpp
std::vector<int> values { 1, 2, 3 };
values.resize(5); // now length 5
values.resize(2); // now length 2
```

### Length vs capacity

**Length**: number of active elements.

**Capacity**: how many elements the vector can hold before needing to allocate
new storage.

Diagram:

```
capacity 8
+----+----+----+----+----+----+----+----+
| 10 | 20 | 30 |    |    |    |    |    |
+----+----+----+----+----+----+----+----+
  active length = 3
```

```cpp
values.size();     // length
values.capacity(); // storage capacity
```

### Reallocation

When length grows beyond capacity, vector allocates a new larger block and moves
or copies elements.

```
old storage: [a][b][c] capacity 3
push d
new storage: [a][b][c][d][ ][ ] capacity 6-ish
old storage released
```

Reallocation is expensive and invalidates pointers/references/iterators to
elements.

### Why capacity exists

If vector allocated on every `push_back`, repeated growth would be very slow.
Extra capacity lets several future pushes happen without reallocating.

### Indexing uses length, not capacity

Capacity is not permission to access unconstructed elements:

```cpp
std::vector<int> values {};
values.reserve(10);

// values[0] = 1; // wrong: size is still 0
```

Use `resize` to create elements:

```cpp
values.resize(10);
values[0] = 1; // ok
```

### Shrinking

`resize(smaller)` removes elements from the active length. It may not reduce
capacity.

```cpp
values.resize(2);
```

Capacity can remain larger for future growth.

---

## 16.11 — std::vector and stack behavior

### Stack behavior

A stack is last-in, first-out (LIFO):

```
push A
push B
push C

pop -> C
pop -> B
pop -> A
```

### Vector stack operations

```cpp
std::vector<std::string> stack {};

stack.push_back("seed1");
stack.push_back("seed2");

std::string top { stack.back() };
stack.pop_back();
```

| Operation | Meaning |
|---|---|
| `push_back(x)` | add element to end |
| `back()` | access last element |
| `pop_back()` | remove last element |

### Reserve

`reserve` changes capacity but not length:

```cpp
std::vector<std::string> seeds {};
seeds.reserve(1000);
```

This avoids repeated reallocations if you know roughly how many elements you will
push.

```
reserve(1000):
length = 0
capacity >= 1000
```

### `push_back` vs `emplace_back`

`push_back` adds an existing object or value:

```cpp
seeds.push_back("abc");
```

`emplace_back` constructs the element in place:

```cpp
seeds.emplace_back("abc");
```

For many simple cases, either is fine. Prefer `push_back` when you already have
the object; use `emplace_back` when constructing complex elements directly in the
vector.

### Fuzzer seed pool

```cpp
std::vector<std::string> interestingSeeds {};
interestingSeeds.reserve(1000);

interestingSeeds.push_back(initialSeed);

while (!interestingSeeds.empty())
{
    std::string seed { interestingSeeds.back() };
    interestingSeeds.pop_back();

    auto mutant { mutate(seed) };
    if (isInteresting(mutant))
        interestingSeeds.push_back(mutant);
}
```

This uses vector as a work stack.

---

## 16.12 — std::vector<bool>

### The odd specialization

`std::vector<bool>` is not a normal vector of bool objects. It is a specialized
space-saving representation that packs booleans into bits.

```
normal vector<T>:
  each element behaves like a T object

vector<bool>:
  bits packed together, element access returns proxy objects
```

This can cause surprising behavior with references, pointers, and generic code.

### Avoid it

Best practice: avoid `std::vector<bool>`.

Alternatives:

```cpp
std::vector<char> flags;
std::vector<std::uint8_t> flags;
std::bitset<N> fixedFlags;       // fixed compile-time size
```

For coverage sets in CS6340, a `std::set`, `std::unordered_set`, `std::vector` of
points, or byte vector is usually more predictable than `std::vector<bool>`.

---

## 16.x — Chapter 16 summary and quiz

### Core takeaways

- Containers store collections of unnamed elements.
- `std::vector<T>` is a dynamic contiguous array of `T`.
- Vector indexes are zero-based.
- `operator[]` is unchecked; `.at()` checks bounds.
- Vector length and capacity are different.
- Reallocation can invalidate pointers/references/iterators.
- `reserve` changes capacity, not length.
- `resize` changes length.
- Prefer passing vectors by const reference for read-only use.
- Returning vectors by value is normal in modern C++.
- Prefer range-for when you do not need indexes.
- Use `const auto&` for read-only expensive elements.
- Use `auto&` when mutating elements.
- Be deliberate around signed/unsigned length/index types.
- `std::vector<bool>` is special and best avoided.

### CS6340 vector patterns

```cpp
std::vector<std::string> seeds {};
std::vector<CoveragePoint> coverage {};
```

Read-only:

```cpp
void printSeeds(const std::vector<std::string>& seeds)
{
    for (const auto& seed : seeds)
        std::cout << seed << '\n';
}
```

Mutation:

```cpp
for (auto& seed : seeds)
{
    seed = mutate(seed);
}
```

Index needed:

```cpp
for (std::size_t i { 0 }; i < seeds.size(); ++i)
{
    std::cout << i << ": " << seeds[i] << '\n';
}
```

### Mini drill

```cpp
#include <iostream>
#include <string>
#include <vector>

std::vector<std::string> makeMutants(std::string seed)
{
    std::vector<std::string> mutants {};
    mutants.reserve(3);

    mutants.push_back(seed + "!");
    mutants.push_back(seed + "?");
    mutants.push_back(seed + "#");

    return mutants;
}

int main()
{
    auto mutants { makeMutants("abc") };

    for (const auto& mutant : mutants)
    {
        std::cout << mutant << '\n';
    }
}
```

What this reinforces:

- vector owns a dynamic list,
- `reserve` prepares capacity,
- `push_back` grows length,
- return-by-value is acceptable,
- `const auto&` avoids string copies during iteration.
