# Chapter 18 — Iterators and Algorithms

> Source: <https://www.learncpp.com/> (Chapter 18)
>
> Goal for CS6340: become comfortable reading and writing the standard C++
> patterns that show up when Lab 1 code keeps collections of seeds, mutants,
> coverage records, crashes, and input files.

## Contents

- [18.1 — Sorting an array using selection sort](#181--sorting-an-array-using-selection-sort)
- [18.2 — Introduction to iterators](#182--introduction-to-iterators)
- [18.3 — Introduction to standard library algorithms](#183--introduction-to-standard-library-algorithms)
- [18.4 — Timing your code](#184--timing-your-code)
- [Chapter 18 summary](#chapter-18-summary)
- [CS6340 patterns](#cs6340-patterns)
- [Mini drill](#mini-drill)

## 18.1 — Sorting an array using selection sort

Sorting means rearranging values into a chosen order.

Common orders:

- increasing numeric order: `1, 2, 3, 4`
- decreasing numeric order: `4, 3, 2, 1`
- alphabetical order: `"apple", "banana", "pear"`
- custom order: shortest string first, highest score first, lowest cost first

Sorting matters because ordered data is often easier to inspect, compare, and
search.

In CS6340 terms:

- sort generated seeds by length before trying shorter reproducers
- sort mutants by operator kind for debugging
- sort coverage points by file/line so output is stable
- sort crash reports by frequency or first-seen time
- sort timings so the slowest cases rise to the top

### A case for sorting

Suppose we have coverage counts:

```cpp
std::vector<int> hits { 9, 1, 4, 7, 2 };
```

Unsorted:

```text
index: 0  1  2  3  4
value: 9  1  4  7  2
```

Sorted ascending:

```text
index: 0  1  2  3  4
value: 1  2  4  7  9
```

Same values, different arrangement.

Important distinction:

- sorting changes the order of elements
- sorting does not create new element values
- sorting can destroy the original order unless you saved it elsewhere

If original order matters, copy before sorting:

```cpp
std::vector<int> original { 9, 1, 4, 7, 2 };
std::vector<int> sorted { original };

std::sort(sorted.begin(), sorted.end());
```

### How sorting works

Most sorting algorithms repeatedly compare elements and move or swap them until
the collection satisfies the ordering rule.

For ascending order, the ordering rule is:

```text
for every adjacent pair:

left <= right
```

Example:

```text
1  2  4  7  9   sorted
1  4  2  7  9   not sorted: 4 > 2
```

At a high level:

```text
unsorted data
    |
    v
compare elements
    |
    v
swap or move elements
    |
    v
repeat until all elements are in order
```

Different sorting algorithms choose different comparison and movement
strategies.

### Selection sort

Selection sort is simple and useful for learning, even though it is usually not
the algorithm you should use in production C++.

Idea:

1. Treat the array as two regions: sorted prefix and unsorted suffix.
2. Find the smallest value in the unsorted suffix.
3. Swap that value into the next sorted position.
4. Move the boundary one element to the right.
5. Repeat until the suffix is empty.

ASCII picture:

```text
start:

sorted prefix     unsorted suffix
[]                [9 1 4 7 2]
 ^
 next position

after first pass:

sorted prefix     unsorted suffix
[1]               [9 4 7 2]
    ^
    next position

after second pass:

sorted prefix     unsorted suffix
[1 2]             [4 7 9]
      ^
      next position
```

Walkthrough:

```text
values: 9 1 4 7 2

pass 0:
  unsorted: 9 1 4 7 2
  smallest: 1
  swap into index 0
  result:   1 9 4 7 2

pass 1:
  sorted:   1
  unsorted: 9 4 7 2
  smallest: 2
  swap into index 1
  result:   1 2 4 7 9

pass 2:
  sorted:   1 2
  unsorted: 4 7 9
  smallest: 4
  already in place
  result:   1 2 4 7 9

pass 3:
  sorted:   1 2 4
  unsorted: 7 9
  smallest: 7
  already in place
  result:   1 2 4 7 9
```

Once only one unsorted element remains, it must already be in the right place.

### Selection sort in C++

With `std::vector`:

```cpp
#include <iostream>
#include <utility> // std::swap
#include <vector>

void selectionSort(std::vector<int>& values)
{
    for (std::size_t start { 0 }; start + 1 < values.size(); ++start)
    {
        std::size_t smallest { start };

        for (std::size_t current { start + 1 }; current < values.size(); ++current)
        {
            if (values[current] < values[smallest])
                smallest = current;
        }

        std::swap(values[start], values[smallest]);
    }
}

int main()
{
    std::vector<int> hits { 9, 1, 4, 7, 2 };

    selectionSort(hits);

    for (int hit : hits)
        std::cout << hit << ' ';

    return 0;
}
```

Important pieces:

```cpp
void selectionSort(std::vector<int>& values)
```

Pass by non-const reference because sorting mutates the vector.

```cpp
for (std::size_t start { 0 }; start + 1 < values.size(); ++start)
```

`start` is the boundary between the sorted prefix and unsorted suffix.

```cpp
std::size_t smallest { start };
```

Assume the first unsorted element is smallest until proven otherwise.

```cpp
if (values[current] < values[smallest])
    smallest = current;
```

The comparison decides which element should come earlier.

```cpp
std::swap(values[start], values[smallest]);
```

Swap the chosen element into place.

### Why selection sort is educational but not the tool of choice

Selection sort is easy to reason about:

```text
position 0: choose the smallest remaining item
position 1: choose the smallest remaining item
position 2: choose the smallest remaining item
...
```

But it is slow for large collections.

For `n` elements, it performs roughly:

```text
(n - 1) + (n - 2) + ... + 1 comparisons
```

That grows on the order of `n^2`.

Examples:

```text
n = 10       about 45 comparisons
n = 1,000    about 499,500 comparisons
n = 100,000  about 4,999,950,000 comparisons
```

That is much too much work for large inputs.

### `std::sort`

In real C++, prefer the standard library:

```cpp
#include <algorithm> // std::sort
#include <iostream>
#include <vector>

int main()
{
    std::vector<int> hits { 9, 1, 4, 7, 2 };

    std::sort(hits.begin(), hits.end());

    for (int hit : hits)
        std::cout << hit << ' ';

    return 0;
}
```

Output:

```text
1 2 4 7 9
```

This line is the key:

```cpp
std::sort(hits.begin(), hits.end());
```

`std::sort` needs two iterators:

- `hits.begin()` points to the first element
- `hits.end()` points one past the last element

That pair describes the half-open range to sort:

```text
sort this:

begin()                         end()
  |                              |
  v                              v
[ 9 ][ 1 ][ 4 ][ 7 ][ 2 ]      one-past-last
```

Half-open means:

```text
[begin, end)
```

Include `begin`; exclude `end`.

This convention is everywhere in C++ standard library algorithms.

### Quiz-level takeaway

You should understand selection sort well enough to trace it, but you should
normally use `std::sort`.

Selection sort teaches:

- loops over arrays/vectors
- indices
- comparisons
- swaps
- algorithmic cost

`std::sort` teaches:

- use the standard library when possible
- pass ranges using iterators
- separate "what I want" from "how to implement it"

## 18.2 — Introduction to iterators

An iterator is an object that represents a position in a sequence.

If a container is a row of boxes, an iterator is like a cursor pointing at one
box.

```text
container:

[ "a" ][ "bb" ][ "ccc" ]
          ^
          iterator points here
```

Iterators let algorithms work with many containers through one common interface.

Instead of `std::sort` needing separate logic for every container type, it can
operate on a range described by iterators.

### The core iterator operations

A typical iterator supports some or all of these operations:

```cpp
*it       // access the element at the iterator
++it      // move to the next element
it != end // test whether we have reached the end
```

Example:

```cpp
#include <iostream>
#include <vector>

int main()
{
    std::vector<int> values { 10, 20, 30 };

    auto it { values.begin() };

    std::cout << *it << '\n'; // 10

    ++it;
    std::cout << *it << '\n'; // 20

    return 0;
}
```

Important:

```cpp
*it
```

does not mean "multiply." With an iterator, unary `*` means dereference: get the
element at the current position.

### `begin()` and `end()`

Containers usually provide:

```cpp
container.begin()
container.end()
```

`begin()` returns an iterator to the first element.

`end()` returns an iterator one past the last element.

For:

```cpp
std::vector<int> values { 10, 20, 30 };
```

The range looks like:

```text
values:

begin()                  end()
  |                       |
  v                       v
[10] [20] [30]          one-past-last
```

The `end()` iterator is not an element. Do not dereference it.

Bad:

```cpp
std::cout << *values.end(); // wrong: end() is not a valid element
```

Loop pattern:

```cpp
for (auto it { values.begin() }; it != values.end(); ++it)
{
    std::cout << *it << '\n';
}
```

This reads as:

```text
start at first element
while not one-past-last
    use current element
    advance to next element
```

### Pointers as iterators

Pointers can act like iterators for contiguous arrays.

```cpp
#include <iostream>

int main()
{
    int values[] { 10, 20, 30 };

    int* begin { values };
    int* end { values + 3 };

    for (int* ptr { begin }; ptr != end; ++ptr)
        std::cout << *ptr << '\n';

    return 0;
}
```

ASCII view:

```text
values decays to pointer to first element:

values
  |
  v
[10] [20] [30]
 ^
 begin

values + 3
             |
             v
[10] [20] [30] one-past-last
```

This is one reason iterators feel pointer-like:

- `*it` accesses the current element
- `++it` moves to the next element
- `it != end` checks whether the scan is still inside the range

But modern code should prefer container iterators over raw pointer arithmetic
unless the code specifically works at a low level.

### Standard library iterators

For `std::vector<int>`, the iterator type is something like:

```cpp
std::vector<int>::iterator
```

You usually do not write this explicitly. Use `auto`:

```cpp
std::vector<int> values { 10, 20, 30 };

for (auto it { values.begin() }; it != values.end(); ++it)
{
    std::cout << *it << '\n';
}
```

For a const container, `begin()` gives an iterator that does not allow mutation:

```cpp
const std::vector<int> values { 10, 20, 30 };

for (auto it { values.begin() }; it != values.end(); ++it)
{
    // *it = 99; // not allowed
    std::cout << *it << '\n';
}
```

You can also ask explicitly for const iterators:

```cpp
for (auto it { values.cbegin() }; it != values.cend(); ++it)
{
    std::cout << *it << '\n';
}
```

Use `cbegin()` / `cend()` when you want to communicate "I am reading, not
modifying."

### `operator<` vs `operator!=` for iterators

With indices, you often write:

```cpp
for (std::size_t i { 0 }; i < values.size(); ++i)
```

With iterators, prefer:

```cpp
for (auto it { values.begin() }; it != values.end(); ++it)
```

Why not use `<`?

```cpp
for (auto it { values.begin() }; it < values.end(); ++it) // less general
```

Some iterators support `<`, but not all do.

Iterator categories differ:

- vector iterators can jump and compare positions
- list iterators can move forward/back but cannot do random access
- stream iterators may only move forward

`!=` is more widely supported and communicates the usual sentinel pattern:

```text
keep moving until the cursor reaches the end sentinel
```

For generic iterator loops, `it != end` is the safer mental model.

### Back to range-based for loops

This:

```cpp
for (int value : values)
{
    std::cout << value << '\n';
}
```

is roughly based on this idea:

```cpp
for (auto it { values.begin() }; it != values.end(); ++it)
{
    int value { *it };
    std::cout << value << '\n';
}
```

That is why range-based for works on containers that provide `begin()` and
`end()`.

If you want to mutate elements, use a reference:

```cpp
for (int& value : values)
{
    value *= 2;
}
```

If you want to read complex elements without copying them:

```cpp
for (const std::string& seed : seeds)
{
    std::cout << seed << '\n';
}
```

### Iterator invalidation

Iterator invalidation means an iterator that used to point into a container no
longer safely points there.

This is a big deal in C++ because invalid iterators are dangling positions.

Example:

```cpp
#include <iostream>
#include <vector>

int main()
{
    std::vector<int> values { 1, 2, 3 };

    auto it { values.begin() };

    values.push_back(4); // may reallocate vector storage

    std::cout << *it << '\n'; // unsafe if push_back invalidated it

    return 0;
}
```

Why can `push_back` invalidate?

`std::vector` stores elements contiguously. If it runs out of capacity, it
allocates a bigger block and moves/copies elements there.

Before:

```text
old vector storage:

it
 |
 v
[1] [2] [3]
```

After reallocation:

```text
old storage: no longer owned

it
 |
 v
[?] [?] [?]

new vector storage:

[1] [2] [3] [4]
```

The iterator still points at the old memory, not the new vector.

Safe habit:

- do not keep iterators across operations that may resize or reorder a container
- reacquire iterators after mutation
- be especially careful with `push_back`, `insert`, `erase`, `resize`, and
  sorting

Safer:

```cpp
std::vector<int> values { 1, 2, 3 };

values.push_back(4);

auto it { values.begin() }; // reacquire after mutation
std::cout << *it << '\n';
```

### Iterator mental model for Lab 1

You will often see:

```cpp
auto it = std::find(seeds.begin(), seeds.end(), target);
```

Read it as:

```text
search the half-open range:

seeds.begin() up to but not including seeds.end()

return an iterator pointing at the found item,
or seeds.end() if not found
```

Then:

```cpp
if (it != seeds.end())
{
    std::cout << "found: " << *it << '\n';
}
```

Never dereference before checking against `end()`:

```cpp
auto it = std::find(seeds.begin(), seeds.end(), target);

std::cout << *it << '\n'; // unsafe if target was not found
```

Correct:

```cpp
auto it = std::find(seeds.begin(), seeds.end(), target);

if (it != seeds.end())
    std::cout << *it << '\n';
```

## 18.3 — Introduction to standard library algorithms

The standard library contains reusable algorithms for common operations:

- find an element
- count elements
- sort elements
- apply a function to each element
- copy, transform, partition, accumulate, and more

These algorithms usually operate on iterator ranges:

```cpp
algorithm(container.begin(), container.end(), extra_arguments...)
```

The major idea:

```text
container owns data
iterator pair describes range
algorithm performs operation
```

Example:

```text
std::vector<std::string> seeds
        |
        v
seeds.begin(), seeds.end()
        |
        v
std::find_if(...)
        |
        v
iterator to matching seed or seeds.end()
```

### Why algorithms are better than hand-written loops

A hand-written loop can be correct, but it hides intent.

Manual search:

```cpp
bool found { false };

for (const std::string& seed : seeds)
{
    if (seed == "crash")
    {
        found = true;
        break;
    }
}
```

Algorithm:

```cpp
auto it { std::find(seeds.begin(), seeds.end(), "crash") };
bool found { it != seeds.end() };
```

The algorithm version says directly: find this value.

Benefits:

- less boilerplate
- less room for off-by-one bugs
- clearer intent
- standard behavior other C++ programmers recognize
- often optimized well by library implementers

### `std::find`

`std::find` searches for a value.

```cpp
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

int main()
{
    std::vector<std::string> seeds { "tiny", "medium", "crash", "large" };

    auto it { std::find(seeds.begin(), seeds.end(), "crash") };

    if (it != seeds.end())
        std::cout << "found seed: " << *it << '\n';
    else
        std::cout << "not found\n";

    return 0;
}
```

Return value:

- iterator to the first matching element, if found
- `end()` iterator if no match

ASCII:

```text
seeds:

begin()                                      end()
  |                                           |
  v                                           v
["tiny"] ["medium"] ["crash"] ["large"]     one-past
                     ^
                     returned iterator
```

If searching for `"missing"`:

```text
returned iterator == seeds.end()
```

Always check before dereferencing.

### `std::find_if`

`std::find_if` searches for the first element matching a condition.

The condition is a predicate: a callable that returns `true` or `false`.

Example: find the first empty seed.

```cpp
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

bool isEmpty(const std::string& seed)
{
    return seed.empty();
}

int main()
{
    std::vector<std::string> seeds { "abc", "", "xyz" };

    auto it { std::find_if(seeds.begin(), seeds.end(), isEmpty) };

    if (it != seeds.end())
        std::cout << "first empty seed at index-like distance\n";

    return 0;
}
```

With a lambda:

```cpp
auto it { std::find_if(seeds.begin(), seeds.end(),
                       [](const std::string& seed)
                       {
                           return seed.empty();
                       }) };
```

Read the lambda as:

```text
for each seed:
    if seed.empty() is true:
        return iterator to this seed
```

Find first seed that is long enough to stress parsing:

```cpp
auto it { std::find_if(seeds.begin(), seeds.end(),
                       [](const std::string& seed)
                       {
                           return seed.size() > 1024;
                       }) };
```

Find first crash record with a specific signal:

```cpp
struct Crash
{
    std::string input;
    int signal {};
};

auto it { std::find_if(crashes.begin(), crashes.end(),
                       [](const Crash& crash)
                       {
                           return crash.signal == 11;
                       }) };
```

### `std::count` and `std::count_if`

`std::count` counts values equal to a target:

```cpp
std::vector<int> outcomes { 0, 1, 0, 2, 0 };

auto zeros { std::count(outcomes.begin(), outcomes.end(), 0) };
```

`zeros` is `3`.

`std::count_if` counts elements satisfying a predicate:

```cpp
std::vector<std::string> seeds { "a", "", "abc", "", "abcdef" };

auto emptyCount { std::count_if(seeds.begin(), seeds.end(),
                                [](const std::string& seed)
                                {
                                    return seed.empty();
                                }) };
```

`emptyCount` is `2`.

CS6340-style example:

```cpp
struct RunResult
{
    std::string input;
    bool crashed {};
    int coveragePoints {};
};

auto crashCount { std::count_if(results.begin(), results.end(),
                                [](const RunResult& result)
                                {
                                    return result.crashed;
                                }) };
```

That reads better than a manual counter loop when the task is simply "count all
crashing results."

### `std::sort` with custom ordering

Default `std::sort` uses `<`.

```cpp
std::vector<int> values { 9, 1, 4 };
std::sort(values.begin(), values.end()); // 1, 4, 9
```

For custom order, pass a comparison function.

Sort strings by length:

```cpp
std::vector<std::string> seeds { "abcdef", "x", "abc" };

std::sort(seeds.begin(), seeds.end(),
          [](const std::string& a, const std::string& b)
          {
              return a.size() < b.size();
          });
```

Result:

```text
x abc abcdef
```

The comparator answers:

```text
Should a come before b?
```

If yes, return `true`.

For descending order:

```cpp
std::sort(values.begin(), values.end(),
          [](int a, int b)
          {
              return a > b;
          });
```

Sort run results by highest coverage first:

```cpp
std::sort(results.begin(), results.end(),
          [](const RunResult& a, const RunResult& b)
          {
              return a.coveragePoints > b.coveragePoints;
          });
```

Important comparator rule:

Do not write a comparator that gives contradictory answers.

Bad:

```cpp
[](int a, int b)
{
    return a <= b; // wrong for std::sort comparator
}
```

Why? If `a == b`, then both `a <= b` and `b <= a` are true. The sort algorithm
expects a strict ordering, where equal values do not come before each other.

Good:

```cpp
[](int a, int b)
{
    return a < b;
}
```

### `std::for_each`

`std::for_each` applies a callable to every element in a range.

```cpp
#include <algorithm>
#include <iostream>
#include <vector>

int main()
{
    std::vector<int> values { 1, 2, 3 };

    std::for_each(values.begin(), values.end(),
                  [](int value)
                  {
                      std::cout << value << '\n';
                  });

    return 0;
}
```

For simple loops, range-based `for` is often clearer:

```cpp
for (int value : values)
{
    std::cout << value << '\n';
}
```

Use `std::for_each` when it fits an algorithm pipeline or you specifically want
the algorithm style. Do not force it when a normal loop is more readable.

### Performance and order of execution

Some algorithms may have different performance characteristics than a manual
loop. In most beginner and intermediate code, the standard algorithm is at least
as good and often clearer.

But order matters.

For many standard algorithms:

- `std::find` stops at the first match
- `std::find_if` stops at the first predicate match
- `std::count` must inspect the whole range
- `std::count_if` must inspect the whole range
- `std::sort` reorders the range

`find` vs `count`:

```cpp
bool hasCrash { std::find(outcomes.begin(), outcomes.end(), "crash") != outcomes.end() };
```

stops as soon as it finds `"crash"`.

```cpp
auto crashCount { std::count(outcomes.begin(), outcomes.end(), "crash") };
```

must scan all outcomes because it needs the total.

Choose the algorithm that matches the question.

### Ranges in C++20

C++20 ranges let many algorithms operate on a container directly:

```cpp
#include <algorithm>
#include <ranges>
#include <vector>

int main()
{
    std::vector<int> values { 9, 1, 4 };

    std::ranges::sort(values);

    return 0;
}
```

Instead of:

```cpp
std::sort(values.begin(), values.end());
```

you can write:

```cpp
std::ranges::sort(values);
```

Range algorithms can be more readable and can avoid accidentally passing
iterators from different containers.

However, many course environments and older codebases still use the classic
iterator-pair algorithms. For CS6340, be fluent in the classic style:

```cpp
std::find(v.begin(), v.end(), value)
std::sort(v.begin(), v.end())
```

Then treat ranges as a nice modern layer when available.

### Algorithm selection cheat sheet

| Question | Algorithm |
| --- | --- |
| Is value present? | `std::find` |
| Where is first value? | `std::find` |
| Where is first element matching a condition? | `std::find_if` |
| How many exact values? | `std::count` |
| How many elements match a condition? | `std::count_if` |
| Put elements in order? | `std::sort` |
| Apply an operation to every element? | range-based `for` or `std::for_each` |

Lab 1 flavor:

| Task | Candidate |
| --- | --- |
| Find whether a seed already exists | `std::find` |
| Find first empty/oversized seed | `std::find_if` |
| Count crashing runs | `std::count_if` |
| Rank inputs by coverage | `std::sort` with comparator |
| Print all interesting inputs | range-based `for` |

## 18.4 — Timing your code

Timing means measuring how long code takes to run.

This matters when:

- a fuzzer is too slow
- a mutation pass adds too much overhead
- an algorithm becomes expensive on large inputs
- a debug build is much slower than expected
- you want to compare two implementations

But timing is noisy. A single measurement is rarely the whole truth.

### Things that can impact performance

Performance can vary because of:

- CPU speed and current load
- debug vs release build
- compiler optimizations
- input size and input shape
- memory allocation patterns
- cache behavior
- disk I/O
- logging volume
- OS scheduling
- background processes

Example: this can dominate runtime:

```cpp
std::cout << "testing seed " << seed << '\n';
```

If it runs inside a hot fuzzing loop, printing may cost more than the operation
you are trying to measure.

For meaningful timings:

- avoid timing excessive debug printing
- measure release builds when you care about production-like speed
- run enough iterations for the duration to be significant
- repeat measurements
- keep input data comparable
- be careful about I/O, which is often much slower than memory operations

### Measuring performance with `<chrono>`

Use `<chrono>` for timing.

Basic pattern:

```cpp
#include <chrono>
#include <iostream>

int main()
{
    auto start { std::chrono::steady_clock::now() };

    // code to measure

    auto stop { std::chrono::steady_clock::now() };

    auto elapsed { stop - start };

    std::cout << std::chrono::duration<double>(elapsed).count() << " seconds\n";

    return 0;
}
```

Prefer `steady_clock` for elapsed-time measurements because it is monotonic:
it should not jump backward or forward because the system clock changed.

### A small timer helper

For repeated experiments:

```cpp
#include <chrono>

class Timer
{
private:
    using Clock = std::chrono::steady_clock;
    Clock::time_point m_start { Clock::now() };

public:
    void reset()
    {
        m_start = Clock::now();
    }

    double elapsed() const
    {
        auto now { Clock::now() };
        return std::chrono::duration<double>(now - m_start).count();
    }
};
```

Usage:

```cpp
#include <algorithm>
#include <iostream>
#include <random>
#include <vector>

int main()
{
    std::vector<int> values(1'000'000);

    std::mt19937 rng { std::random_device{}() };
    std::uniform_int_distribution<int> dist { 1, 1'000'000 };

    for (int& value : values)
        value = dist(rng);

    Timer timer {};

    std::sort(values.begin(), values.end());

    std::cout << "sort took " << timer.elapsed() << " seconds\n";

    return 0;
}
```

### Measure enough work

Tiny operations can be too fast to measure reliably.

Weak timing:

```cpp
Timer timer {};

int x { 1 + 2 };

std::cout << timer.elapsed() << '\n';
```

The measurement overhead may be larger than the operation.

Better:

```cpp
Timer timer {};

int total {};

for (int i { 0 }; i < 10'000'000; ++i)
{
    total += i % 7;
}

std::cout << "elapsed: " << timer.elapsed() << '\n';
std::cout << "total: " << total << '\n';
```

The `total` output helps prevent the compiler from deleting the whole loop as
useless work.

### Debug vs release

Debug builds are often much slower because they preserve information for
debugging and may disable many optimizations.

When measuring performance:

```text
Debug build:
  easier to inspect
  slower
  more checks/symbols

Release build:
  optimized
  faster
  harder to step through
```

For CS6340:

- use debug builds when learning and debugging correctness
- use release-ish builds when measuring throughput or overhead
- do not compare one implementation in debug to another in release

### Timing fuzzer-like loops

Example:

```cpp
#include <chrono>
#include <iostream>
#include <string>
#include <vector>

std::string mutate(std::string seed)
{
    if (!seed.empty())
        seed[0] = static_cast<char>(seed[0] + 1);

    return seed;
}

int main()
{
    std::vector<std::string> seeds { "abc", "xyz", "hello" };
    std::vector<std::string> mutants {};

    Timer timer {};

    for (int round { 0 }; round < 100'000; ++round)
    {
        for (const std::string& seed : seeds)
            mutants.push_back(mutate(seed));
    }

    std::cout << "generated " << mutants.size() << " mutants\n";
    std::cout << "elapsed " << timer.elapsed() << " seconds\n";

    return 0;
}
```

This measures mutation-loop throughput more meaningfully than timing one call to
`mutate`.

But there are still caveats:

- `push_back` allocation may dominate
- string copies may dominate
- `mutants` may consume a lot of memory
- the example does not execute target programs
- real fuzzing often includes process launch, file I/O, and timeout handling

If you want to isolate mutation cost, reserve capacity:

```cpp
mutants.reserve(seeds.size() * 100'000);
```

That avoids repeatedly reallocating the vector as it grows.

### Timing and algorithm choice

Chapter 18 connects sorting and timing:

```text
selection sort:
  easy to understand
  O(n^2)
  bad for large n

std::sort:
  standard library
  much better asymptotic behavior
  heavily optimized
```

If your vector grows from 10 elements to 100,000 elements, algorithm choice can
become the difference between "instant" and "painfully slow."

## Chapter 18 summary

- Sorting rearranges elements into an order.
- Selection sort repeatedly selects the smallest remaining value and swaps it
  into the next sorted position.
- Selection sort is useful for learning but generally too slow for large inputs.
- Prefer `std::sort` for real C++ sorting.
- Standard algorithms usually operate on half-open iterator ranges:
  `[begin, end)`.
- `begin()` points at the first element.
- `end()` points one past the last element and must not be dereferenced.
- Iterators are position objects. Use `*it` to access, `++it` to advance, and
  `it != end` to test whether the scan should continue.
- Pointers can act like iterators for arrays, but container iterators are the
  usual modern pattern.
- Iterator invalidation happens when a container operation makes existing
  iterators unsafe to use.
- `std::find` finds a value.
- `std::find_if` finds a value matching a predicate.
- `std::count` counts exact values.
- `std::count_if` counts predicate matches.
- `std::sort` can use a comparator for custom ordering.
- `std::for_each` applies a callable to each element, though range-based `for`
  is often clearer for simple loops.
- `<chrono>` is the standard timing library.
- Use `std::chrono::steady_clock` for elapsed time.
- Measure enough work, repeat measurements, and distinguish debug from release
  timing.

## CS6340 patterns

### Find whether a seed is already present

```cpp
bool containsSeed(const std::vector<std::string>& seeds,
                  const std::string& candidate)
{
    return std::find(seeds.begin(), seeds.end(), candidate) != seeds.end();
}
```

### Find an input that looks suspicious

```cpp
auto it { std::find_if(seeds.begin(), seeds.end(),
                       [](const std::string& seed)
                       {
                           return seed.empty() || seed.size() > 4096;
                       }) };

if (it != seeds.end())
{
    std::cout << "suspicious seed: " << *it << '\n';
}
```

### Sort coverage records for stable output

```cpp
struct CoveragePoint
{
    std::string file;
    int line {};
};

std::sort(points.begin(), points.end(),
          [](const CoveragePoint& a, const CoveragePoint& b)
          {
              if (a.file != b.file)
                  return a.file < b.file;

              return a.line < b.line;
          });
```

This comparator sorts by file first, then line.

ASCII:

```text
before:
  b.cpp:10
  a.cpp:30
  a.cpp:12

after:
  a.cpp:12
  a.cpp:30
  b.cpp:10
```

### Count crashes

```cpp
struct Result
{
    std::string seed;
    bool crashed {};
};

auto crashes { std::count_if(results.begin(), results.end(),
                             [](const Result& result)
                             {
                                 return result.crashed;
                             }) };
```

### Measure a batch

```cpp
Timer timer {};

for (const std::string& seed : seeds)
{
    std::string mutant { mutate(seed) };
    runTarget(mutant);
}

std::cout << "batch seconds: " << timer.elapsed() << '\n';
```

Do not over-interpret one batch. Repeat and compare under similar conditions.

## Mini drill

Given:

```cpp
struct Run
{
    std::string input;
    bool crashed {};
    int coverage {};
};

std::vector<Run> runs {
    { "abc", false, 10 },
    { "", true, 1 },
    { "long-seed", false, 25 },
    { "xyz", true, 11 },
};
```

Write code to:

1. Find the first crashing run.
2. Count all crashing runs.
3. Sort runs by descending coverage.
4. Print each input and coverage.

One possible solution:

```cpp
auto firstCrash { std::find_if(runs.begin(), runs.end(),
                               [](const Run& run)
                               {
                                   return run.crashed;
                               }) };

if (firstCrash != runs.end())
    std::cout << "first crash input: " << firstCrash->input << '\n';

auto crashCount { std::count_if(runs.begin(), runs.end(),
                                [](const Run& run)
                                {
                                    return run.crashed;
                                }) };

std::sort(runs.begin(), runs.end(),
          [](const Run& a, const Run& b)
          {
              return a.coverage > b.coverage;
          });

for (const Run& run : runs)
{
    std::cout << run.input << ": " << run.coverage << '\n';
}
```

New syntax:

```cpp
firstCrash->input
```

For an iterator to a struct/class object, `it->member` is shorthand for:

```cpp
(*it).member
```

That pattern appears constantly when iterators point at objects.

