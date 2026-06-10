# Chapter 18 — Iterators and Algorithms

> Source: <https://www.learncpp.com/> (Chapter 18)

For seventeen chapters you have been writing loops by hand. You declare an index, compare it against `size()`, increment it, reach inside the container with `[]`, and somewhere in that machinery do the actual work — the search, the count, the sort. The loop scaffolding is so routine you barely see it anymore, which is exactly the problem: it is routine, repetitive, and a quiet source of off-by-one bugs. Every time you write `for (std::size_t i { 0 }; i < v.size(); ++i)` you are re-deriving something the language could have handed you.

This chapter is about handing it to you. It introduces two ideas that, together, change how you write almost all collection code in C++. The first is the **iterator**: a small object that represents a *position* in a sequence — a cursor you can move, dereference, and compare. The second is the **standard library algorithm**: a named, tested, reusable operation — find, count, sort, and dozens more — that works on any range an iterator pair can describe. The payoff is that you stop writing *how* to walk a container and start writing *what* you want done. `std::find(v.begin(), v.end(), target)` says "find this," and means it, where a hand-rolled loop only implies it through five lines of bookkeeping.

We'll start by building a sort from scratch, so you appreciate what the library is doing for you before you delegate to it. Then we'll meet iterators properly — what they are, the half-open range convention that runs through the entire standard library, and the famous trap of *iterator invalidation*. From there we'll tour the workhorse algorithms you'll reach for constantly, and finish with how to *measure* code, because once you have a choice of algorithms, "which is faster?" becomes a question you can actually answer.

## Contents
- [18.1 — Sorting an array using selection sort](#181--sorting-an-array-using-selection-sort)
- [18.2 — Introduction to iterators](#182--introduction-to-iterators)
- [18.3 — Introduction to standard library algorithms](#183--introduction-to-standard-library-algorithms)
- [18.4 — Timing your code](#184--timing-your-code)

---

## 18.1 — Sorting an array using selection sort

### Why ordering matters

To **sort** a collection is to rearrange its elements into a chosen order. You already have an intuition for the common ones:

- increasing numeric order: `1, 2, 3, 4`
- decreasing numeric order: `4, 3, 2, 1`
- alphabetical order: `"apple", "banana", "pear"`
- some custom order: shortest string first, highest score first, lowest cost first

Sorting matters because ordered data is easier to *inspect*, easier to *compare*, and — as you'll see in a moment — dramatically easier to *search*. When the values arrive in no particular order, putting them in order is often the first thing you do.

Hold onto one distinction before we go further, because it trips people up:

- Sorting **changes the arrangement** of elements.
- Sorting **does not create new values** — the same numbers come out, just in different seats.
- Sorting **destroys the original order**, unless you saved a copy first.

That last point is a practical warning. If you need the original ordering for anything — say, to remember which input produced which result — copy before you sort:

```cpp
#include <algorithm>
#include <vector>

int main()
{
    std::vector<int> original { 9, 1, 4, 7, 2 };
    std::vector<int> sorted { original };      // copy first

    std::sort(sorted.begin(), sorted.end());   // 'original' is untouched

    return 0;
}
```

### How sorting works, in the abstract

Almost every sorting algorithm boils down to the same two moves repeated until done: **compare** elements, then **swap or move** them. The rule that tells you when to stop is the ordering rule. For ascending order it is simply this — for every adjacent pair, the left element is no greater than the right:

```text
1  2  4  7  9   sorted   (every left <= its right)
1  4  2  7  9   not sorted: 4 > 2
```

At a high level, every sort follows the same loop:

```text
unsorted data
    -> compare elements
    -> swap or move elements
    -> repeat until everything is in order
```

What separates one sorting algorithm from another is *which* elements they choose to compare and *how* they move them. We'll write one of the simplest by hand to make the idea concrete.

### Selection sort, step by step

**Selection sort** is the algorithm you'd probably invent yourself if asked to sort a hand of cards. Picture the array split into two regions: a **sorted prefix** on the left that's already finished, and an **unsorted suffix** on the right that still needs work. The prefix starts empty.

1. Look through the unsorted suffix and find the smallest value.
2. Swap that value into the first unsorted position.
3. Move the boundary one step right — that position is now part of the sorted prefix.
4. Repeat until the suffix is empty.

Here is the boundary marching rightward across `9 1 4 7 2`:

```text
start:
  sorted []            unsorted [9 1 4 7 2]
   ^ next position

after pass 0 (smallest is 1, swap into index 0):
  sorted [1]           unsorted [9 4 7 2]
      ^ next position

after pass 1 (smallest is 2, swap into index 1):
  sorted [1 2]         unsorted [4 7 9]
        ^ next position
```

Tracing the whole thing out:

```text
values: 9 1 4 7 2

pass 0: smallest of [9 1 4 7 2] is 1 -> swap into index 0 -> 1 9 4 7 2
pass 1: smallest of [9 4 7 2]   is 2 -> swap into index 1 -> 1 2 4 7 9
pass 2: smallest of [4 7 9]     is 4 -> already in place   -> 1 2 4 7 9
pass 3: smallest of [7 9]       is 7 -> already in place   -> 1 2 4 7 9
```

Notice we stop one short of the end. Once a single unsorted element remains, there's nothing left to compare it against — it must already be in its correct spot.

### Writing it in C++

Here is selection sort as a function over a `std::vector<int>`:

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
        std::cout << hit << ' ';      // 1 2 4 7 9

    return 0;
}
```

Each line earns its place. The parameter is a **non-const reference** because sorting mutates the vector in place — we're rearranging the caller's data, not a copy:

```cpp
void selectionSort(std::vector<int>& values)
```

The outer loop variable `start` *is* the boundary between sorted prefix and unsorted suffix. The condition `start + 1 < values.size()` encodes the "stop one short" insight from the trace.

```cpp
std::size_t smallest { start };
```

We optimistically assume the first unsorted element is the smallest, then let the inner loop prove us wrong. Every time we find something smaller, we remember *where* it is — `smallest` holds an index, not a value, because at the end we need to swap by position:

```cpp
std::swap(values[start], values[smallest]);
```

> **Note:** `std::swap` (from `<utility>`) exchanges the contents of two variables. Writing the swap by hand with a temporary works too, but `std::swap` says what you mean and the standard library implements it well for any type.

### Why selection sort is a teaching tool, not a production tool

Selection sort is wonderfully easy to reason about — "position 0 gets the smallest remaining item, position 1 gets the next smallest, and so on." But that clarity comes at a cost: it is **slow** on large inputs.

Count the comparisons. The first pass examines `n - 1` elements, the next `n - 2`, and so on down to 1. That sum grows on the order of `n²`:

```text
n = 10       about 45 comparisons
n = 1,000    about 499,500 comparisons
n = 100,000  about 4,999,950,000 comparisons
```

Five billion comparisons to sort a hundred thousand items is unacceptable. Doubling the input quadruples the work — that's the signature of an `O(n²)` algorithm, and it's why selection sort doesn't scale.

> **Key insight:** Learn selection sort to understand *how* sorting works — loops, indices, comparisons, swaps, and algorithmic cost. But in real code, never write it. Reach for `std::sort`.

### `std::sort`: the one you'll actually use

The standard library gives you a sort that is heavily optimized and far faster than anything you'd reasonably hand-write:

```cpp
#include <algorithm> // std::sort
#include <iostream>
#include <vector>

int main()
{
    std::vector<int> hits { 9, 1, 4, 7, 2 };

    std::sort(hits.begin(), hits.end());

    for (int hit : hits)
        std::cout << hit << ' ';      // 1 2 4 7 9

    return 0;
}
```

The whole job is one line:

```cpp
std::sort(hits.begin(), hits.end());
```

`std::sort` doesn't take a container. It takes **two iterators** that describe the range to sort:

- `hits.begin()` — a position pointing at the first element
- `hits.end()` — a position pointing *one past* the last element

```text
sort this range:

begin()                          end()
  |                               |
  v                               v
[ 9 ][ 1 ][ 4 ][ 7 ][ 2 ]      one-past-last
```

That pair `[begin, end)` is a **half-open range**: include `begin`, exclude `end`. The square bracket means "closed, included"; the round bracket means "open, excluded." This convention — start included, end excluded — is everywhere in the standard library, and getting comfortable with it now will pay off in every algorithm you meet for the rest of the chapter.

> **Best practice:** Prefer `std::sort` over any sort you write yourself. It is correct, fast, and instantly recognizable to other C++ programmers.

What `std::sort` really teaches isn't sorting at all. It's three habits that define modern C++: **lean on the standard library**, **describe work with iterator ranges**, and **separate *what* you want from *how* it's implemented.** That separation is the whole reason iterators exist — so let's meet them properly.

---

## 18.2 — Introduction to iterators

### A cursor into a sequence

An **iterator** is an object that represents a *position* in a sequence. If a container is a row of boxes, an iterator is a finger pointing at one of them:

```text
container:

[ "a" ][ "bb" ][ "ccc" ]
          ^
          iterator points here
```

That's the whole idea, and it's deceptively powerful. Because an iterator stands for a position rather than a particular kind of container, an algorithm written in terms of iterators works on *any* container that can produce them. `std::sort` doesn't need one version for vectors and another for arrays — it needs a range, and the range knows how to walk itself. The iterator is the common language that lets one algorithm serve many containers.

### The three core operations

Almost everything you do with an iterator comes down to three moves:

```cpp
*it          // dereference: access the element at this position
++it         // advance: move to the next element
it != end    // compare: have we reached the end yet?
```

Here they are in action:

```cpp
#include <iostream>
#include <vector>

int main()
{
    std::vector<int> values { 10, 20, 30 };

    auto it { values.begin() };       // position at the first element

    std::cout << *it << '\n';         // 10  (dereference)

    ++it;                             // advance
    std::cout << *it << '\n';         // 20

    return 0;
}
```

One point of confusion worth heading off immediately: `*it` here is **not multiplication**. Applied to a single iterator, unary `*` means *dereference* — "give me the element this position points at." Same symbol, completely different job from the binary `*` of arithmetic.

### `begin()` and `end()`

Containers hand you their two boundary positions through member functions:

```cpp
container.begin()   // iterator to the first element
container.end()     // iterator one past the last element
```

For `std::vector<int> values { 10, 20, 30 };` the picture is:

```text
values:

begin()                  end()
  |                       |
  v                       v
[10] [20] [30]          one-past-last
```

That `end()` position is the crucial subtlety. It does **not** point at an element — it points at the imaginary slot just *after* the last one. It exists to mark "you've gone as far as there is to go." Dereferencing it is undefined behavior:

```cpp
std::cout << *values.end();   // WRONG: end() is not a real element
```

> **Warning:** Never dereference `end()`. It marks the boundary of a range, not a value inside it. Every algorithm that "returns `end()`" is telling you "nothing valid here" — treat `end()` as a sentinel, never as data.

Why one-past-the-end rather than pointing at the last element? Because it makes the standard loop fall out cleanly. "Start at `begin()`, keep going while you're not at `end()`":

```cpp
for (auto it { values.begin() }; it != values.end(); ++it)
{
    std::cout << *it << '\n';
}
```

Read that as a sentence:

```text
start at the first element
while not one-past-the-last
    use the current element
    advance to the next
```

When `it` finally equals `end()`, the loop stops *before* doing anything with that invalid position. The half-open range `[begin, end)` and the loop condition `it != end()` are two faces of the same design.

### Pointers were iterators all along

Iterators feel pointer-like because, historically, pointers *were* the original iterators. For a contiguous C-style array, a raw pointer supports exactly the three operations:

```cpp
#include <iostream>

int main()
{
    int values[] { 10, 20, 30 };

    int* begin { values };       // array decays to a pointer to its first element
    int* end { values + 3 };     // one past the last element

    for (int* ptr { begin }; ptr != end; ++ptr)
        std::cout << *ptr << '\n';

    return 0;
}
```

```text
values
  |
  v
[10] [20] [30]      values + 3 points here -> one-past-last
 ^
 begin
```

`*ptr` accesses the current element, `++ptr` advances, `ptr != end` tests the boundary — the same trio. This is no coincidence: the iterator interface was designed to generalize the pointer interface so that algorithms written for pointers would work for everything. In modern code you should still prefer container iterators over raw pointer arithmetic unless you're deliberately working at a low level — but understanding the kinship explains why iterators behave the way they do.

### Standard library iterator types, and `auto`

Every standard container defines its own iterator type. For `std::vector<int>` it's spelled `std::vector<int>::iterator` — a mouthful you almost never write out. Let `auto` deduce it:

```cpp
std::vector<int> values { 10, 20, 30 };

for (auto it { values.begin() }; it != values.end(); ++it)
    std::cout << *it << '\n';
```

When the container is `const`, `begin()` hands back an iterator that won't let you modify the elements through it:

```cpp
const std::vector<int> values { 10, 20, 30 };

for (auto it { values.begin() }; it != values.end(); ++it)
{
    // *it = 99;                  // error: can't modify through a const iterator
    std::cout << *it << '\n';     // reading is fine
}
```

You can also ask for read-only iterators explicitly, even on a non-const container, using `cbegin()` and `cend()` (the `c` is for "const"):

```cpp
for (auto it { values.cbegin() }; it != values.cend(); ++it)
    std::cout << *it << '\n';
```

> **Tip:** Reach for `cbegin()` / `cend()` when you're only reading. It documents your intent — "I will not modify this" — and lets the compiler catch you if you slip.

### Why `!=` and not `<`?

With indices, you naturally write `i < values.size()`. With iterators, the idiom is `it != values.end()`, and the reason is worth understanding rather than memorizing.

```cpp
for (auto it { values.begin() }; it < values.end(); ++it)   // works for vector, but less general
```

That `<` *does* work for a vector, because vector iterators can compare their relative positions. But not every iterator can. Iterators come in **categories** depending on what their container supports:

- A **vector** iterator can jump around and compare positions with `<` — it's *random-access*.
- A **`std::list`** iterator can step forward and backward but cannot do `<` — there's no notion of "less than" for positions in a linked list.
- A **stream** iterator may only move forward, one element at a time.

`!=` asks only "are these two positions the same?", which every iterator can answer. So `it != end` is the pattern that works everywhere, and it expresses the right mental model anyway: *keep moving until the cursor reaches the end sentinel.*

> **Best practice:** In iterator loops, test `it != end` rather than `it < end`. It works for every iterator category and reads as the sentinel pattern it is.

### Range-based `for`, demystified

You've been using range-based `for` since Chapter 16. Now you can see what it's built on. This:

```cpp
for (int value : values)
    std::cout << value << '\n';
```

is, in spirit, shorthand for this:

```cpp
for (auto it { values.begin() }; it != values.end(); ++it)
{
    int value { *it };
    std::cout << value << '\n';
}
```

That's *why* range-based `for` works on any type that provides `begin()` and `end()` — under the hood it's just the iterator loop. Knowing this also explains the reference forms you've been told to use. Want to *modify* each element? Bind a reference, so you're writing through the iterator instead of into a copy:

```cpp
for (int& value : values)
    value *= 2;
```

Want to read large elements without paying to copy them? Bind a `const` reference:

```cpp
for (const std::string& seed : seeds)
    std::cout << seed << '\n';
```

### Iterator invalidation: the dangling-position trap

Here is the one piece of this lesson that will eventually bite you if you forget it. **Iterator invalidation** means an iterator that used to point safely into a container no longer does — it has become a *dangling position*.

```cpp
#include <iostream>
#include <vector>

int main()
{
    std::vector<int> values { 1, 2, 3 };

    auto it { values.begin() };

    values.push_back(4);          // may reallocate the vector's storage

    std::cout << *it << '\n';     // UNSAFE if push_back moved the elements

    return 0;
}
```

Why can a humble `push_back` poison an iterator? Recall from Chapter 16 that `std::vector` stores its elements in one contiguous block. When the block fills up, `push_back` allocates a *bigger* block somewhere else, copies or moves the elements into it, and frees the old block. Your iterator still points at the old, now-abandoned memory:

```text
before push_back:                 after reallocation:

it                                it  (still pointing at freed memory!)
 |                                 |
 v                                 v
[1] [2] [3]   <- vector here      [?] [?] [?]   <- old block, gone

                                  new block elsewhere:
                                  [1] [2] [3] [4]
```

The iterator didn't follow the data to its new home — it has no way to know the data moved. Operations that resize or reorder a container are the dangerous ones: `push_back`, `insert`, `erase`, `resize`, and sorting can all invalidate iterators.

> **Warning:** Do not hold an iterator across an operation that may resize or reorder its container. If you must mutate and then keep iterating, **reacquire** the iterator afterward.

The safe habit is to grab the iterator *after* you're done mutating:

```cpp
std::vector<int> values { 1, 2, 3 };

values.push_back(4);

auto it { values.begin() };       // acquired after the mutation -> valid
std::cout << *it << '\n';
```

### The pattern you'll see constantly

Search algorithms return an iterator, and you'll read code like this all the time:

```cpp
auto it { std::find(seeds.begin(), seeds.end(), target) };
```

Read it as: *search the half-open range from `seeds.begin()` up to (but not including) `seeds.end()`; return an iterator to the first match, or `seeds.end()` if there is no match.* The return value tells you whether the search succeeded — and you must check it before dereferencing:

```cpp
auto it { std::find(seeds.begin(), seeds.end(), target) };

if (it != seeds.end())
    std::cout << "found: " << *it << '\n';
```

Skipping the check is exactly the `end()`-dereference mistake from earlier, just hidden inside a search:

```cpp
auto it { std::find(seeds.begin(), seeds.end(), target) };
std::cout << *it << '\n';          // UNSAFE if target wasn't found -> *end()
```

> **Key insight:** Any algorithm that returns an iterator returns `end()` to mean "not found." Compare against `end()` *before* you dereference, every single time. This is the single most important reflex for working with the algorithms in the next lesson.

---

## 18.3 — Introduction to standard library algorithms

### A toolbox of named operations

The `<algorithm>` header is a toolbox of reusable operations for the things you do to collections over and over:

- find an element
- count elements
- sort elements
- apply a function to each element
- copy, transform, partition, accumulate, and dozens more

What they all share is the iterator-range interface from the last lesson. The shape is always the same — hand the algorithm a range, plus whatever extra arguments it needs:

```cpp
algorithm(container.begin(), container.end(), extra_arguments...)
```

The division of labor is clean and worth naming explicitly:

```text
the container        owns the data
the iterator pair    describes the range to operate on
the algorithm        performs the operation
```

The container stores; the iterators delimit; the algorithm acts. Keep that three-way split in mind and every algorithm in this lesson will feel familiar before you've read its description.

### Why an algorithm beats a hand-written loop

A loop you write yourself can be perfectly correct — and still be worse, because it *hides its intent*. Compare a manual search:

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

against the algorithm:

```cpp
auto it { std::find(seeds.begin(), seeds.end(), "crash") };
bool found { it != seeds.end() };
```

The second version *says what it does*: find this value. A reader doesn't have to trace a loop, spot the `break`, and reverse-engineer the goal. The benefits compound:

- less boilerplate to write and read
- no room for off-by-one or forgotten-`break` bugs
- intent stated directly in the name
- behavior every C++ programmer already recognizes
- often hand-tuned by the library implementers

> **Best practice:** Prefer a standard algorithm to a hand-written loop whenever one fits. You write less, you say more, and you inherit code that's already been debugged by everyone who came before you.

Let's meet the workhorses.

### `std::find` — is this value here?

`std::find` scans a range for a value and returns an iterator to the **first** match, or `end()` if there is none.

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

```text
seeds:

begin()                                      end()
  |                                           |
  v                                           v
["tiny"] ["medium"] ["crash"] ["large"]     one-past
                     ^
                     returned iterator
```

Search for something absent — `"missing"` — and the returned iterator equals `seeds.end()`. This is exactly the not-found protocol from 18.2: always check against `end()` before you dereference.

### `std::find_if` — the first element matching a *condition*

Often you're not looking for one specific value but for the first element that *satisfies some test*. That's `std::find_if`. Instead of a value, you hand it a **predicate**: a callable that takes an element and returns `true` or `false`.

In this chapter your predicate is a plain **free function** — a normal function whose name you pass to the algorithm. (Lambdas, the more compact way to write the same thing inline, arrive in Chapter 20. Until then, a named function is the tool.)

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
        std::cout << "found an empty seed\n";

    return 0;
}
```

When you write `isEmpty` as the third argument, its name decays to a **function pointer** — the address of the function — which the algorithm calls once per element. Read the call as: *for each seed, if `isEmpty(seed)` is true, stop and return an iterator to it.* `std::find_if` walks the range, calls your predicate on each element, and hands back the first one for which it returns `true` (or `end()` if none qualify).

> **Note:** A **predicate** is just a callable that answers a yes/no question about an element. In Chapter 18 it's a free function; the address of that function (a function pointer) is what you pass. Chapter 20's lambdas will let you write the same predicate inline at the call site.

The predicate can test anything. Here it looks for the first oversized input — perhaps to stress a parser:

```cpp
bool tooLong(const std::string& seed)
{
    return seed.size() > 1024;
}

auto it { std::find_if(seeds.begin(), seeds.end(), tooLong) };
```

And it works just as well on your own struct types — find the first crash record with a particular signal:

```cpp
struct Crash
{
    std::string input;
    int signal {};
};

bool isSegfault(const Crash& crash)
{
    return crash.signal == 11;
}

auto it { std::find_if(crashes.begin(), crashes.end(), isSegfault) };
```

### `std::count` and `std::count_if` — how many?

`std::count` returns how many elements in a range equal a given value:

```cpp
std::vector<int> outcomes { 0, 1, 0, 2, 0 };

auto zeros { std::count(outcomes.begin(), outcomes.end(), 0) };   // 3
```

`std::count_if` is its predicate-driven sibling: it counts how many elements satisfy a condition.

```cpp
bool isEmpty(const std::string& seed)
{
    return seed.empty();
}

std::vector<std::string> seeds { "a", "", "abc", "", "abcdef" };

auto emptyCount { std::count_if(seeds.begin(), seeds.end(), isEmpty) };   // 2
```

The pattern shows up constantly when you want a tally rather than a single hit. Counting records that crashed reads far better as an algorithm than as a hand-rolled counter loop:

```cpp
struct RunResult
{
    std::string input;
    bool crashed {};
    int coveragePoints {};
};

bool didCrash(const RunResult& result)
{
    return result.crashed;
}

auto crashCount { std::count_if(results.begin(), results.end(), didCrash) };
```

> **Note:** The count algorithms return a signed integer type (`std::ptrdiff_t`), not `int` or `std::size_t`. If you need an `int` — say, to match a function's return type — convert explicitly with `static_cast<int>(...)`.

### `std::sort` with a custom order

By default `std::sort` orders elements with `<`, giving ascending order:

```cpp
std::vector<int> values { 9, 1, 4 };
std::sort(values.begin(), values.end());      // 1, 4, 9
```

To sort by anything else, pass a third argument: a **comparator**. Like a predicate, it's a callable — here one that takes *two* elements and answers a single question: *should the first come before the second?* Return `true` for yes.

Sort strings shortest-first by comparing their lengths:

```cpp
bool byLength(const std::string& a, const std::string& b)
{
    return a.size() < b.size();
}

std::vector<std::string> seeds { "abcdef", "x", "abc" };

std::sort(seeds.begin(), seeds.end(), byLength);   // x, abc, abcdef
```

Flip the comparison to `>` and you get descending order:

```cpp
bool descending(int a, int b)
{
    return a > b;
}

std::sort(values.begin(), values.end(), descending);
```

Or rank your run results so the highest coverage floats to the top:

```cpp
bool byCoverageDesc(const RunResult& a, const RunResult& b)
{
    return a.coveragePoints > b.coveragePoints;
}

std::sort(results.begin(), results.end(), byCoverageDesc);
```

### The comparator rule you must not break

There is one rule about comparators that, if you violate it, leads to undefined behavior — meaning a crash, a scrambled result, or nothing at all, unpredictably. The comparator must define a **strict weak ordering**, and the part of that you need to internalize is simple: *equal elements must not claim to come before each other.*

Concretely, `comp(x, x)` must always return `false`. An element is never "before" itself. This is why you use strictly `<` (or `>`), never `<=` (or `>=`):

```cpp
// BROKEN comparator
bool bad(int a, int b)
{
    return a <= b;        // when a == b, this is true -> violates the rule
}
```

When `a` and `b` are equal, `a <= b` is `true` *and* `b <= a` is `true` — the comparator simultaneously claims `a` comes before `b` and `b` comes before `a`. That contradiction is exactly what `std::sort` is forbidden to receive, and it can corrupt memory when it does.

```cpp
// CORRECT comparator
bool good(int a, int b)
{
    return a < b;         // when a == b, this is false -> neither comes first
}
```

> **Warning:** A `std::sort` comparator must use strict comparison (`<` or `>`), never `<=` or `>=`. When two elements are equal, the comparator must return `false`. Breaking this rule is undefined behavior, not a harmless quirk.

### `std::for_each` — do something to every element

`std::for_each` applies a callable to each element of a range:

```cpp
#include <algorithm>
#include <iostream>
#include <vector>

void print(int value)
{
    std::cout << value << '\n';
}

int main()
{
    std::vector<int> values { 1, 2, 3 };

    std::for_each(values.begin(), values.end(), print);

    return 0;
}
```

Be honest about when this earns its keep. For a simple "do X to each element," a range-based `for` is usually *clearer*:

```cpp
for (int value : values)
    std::cout << value << '\n';
```

`std::for_each` shines when it slots into a pipeline of algorithms, or when you specifically want the algorithm-style uniformity. Don't force it where a plain loop reads better.

> **Best practice:** Use a range-based `for` for ordinary per-element work. Reach for `std::for_each` only when the algorithm form genuinely improves clarity or fits an existing algorithm pipeline.

### Order of work: match the algorithm to the question

Different algorithms do different amounts of work, and choosing the right one is a small but real performance decision. The key distinction is **early exit** versus **full scan**:

- `std::find` and `std::find_if` stop at the **first** match — they can quit early.
- `std::count` and `std::count_if` must inspect the **entire** range — they need the total.
- `std::sort` reorders the whole range.

So if all you need to know is *whether* a value is present, ask with `find`, not `count`:

```cpp
bool hasCrash { std::find(outcomes.begin(), outcomes.end(), "crash") != outcomes.end() };
```

That stops the instant it sees `"crash"`. Using `count` to answer the same yes/no question forces a scan of every element, because counting can't know it's done until it's seen them all:

```cpp
auto crashCount { std::count(outcomes.begin(), outcomes.end(), "crash") };  // scans everything
```

> **Tip:** Pick the algorithm that matches the *question*. "Is it there?" wants `find` (early exit). "How many?" wants `count` (full scan). Asking the wrong one still gives the right answer — just slower.

### A glimpse of C++20 ranges

If you work in a newer codebase, you may see a more concise form. C++20 added **ranges**, which let many algorithms take a container directly instead of a `begin()`/`end()` pair:

```cpp
#include <algorithm>
#include <ranges>
#include <vector>

int main()
{
    std::vector<int> values { 9, 1, 4 };

    std::ranges::sort(values);            // instead of std::sort(values.begin(), values.end())

    return 0;
}
```

Besides being shorter, range algorithms remove a whole class of bug: you can't accidentally pass `begin()` from one container and `end()` from another. They're a genuine improvement where available — but plenty of course environments and existing code still use the classic iterator-pair form, so that's the one to be fluent in first:

```cpp
std::find(v.begin(), v.end(), value)
std::sort(v.begin(), v.end())
```

Treat ranges as a pleasant modern layer on top once the classic style is second nature.

### Algorithm selection cheat sheet

When you know the question, the algorithm follows:

| Question | Algorithm |
| --- | --- |
| Is this value present? | `std::find` |
| Where is the first occurrence of a value? | `std::find` |
| Where is the first element matching a condition? | `std::find_if` |
| How many equal this value? | `std::count` |
| How many match a condition? | `std::count_if` |
| Put the elements in order? | `std::sort` |
| Do something to every element? | range-based `for` or `std::for_each` |

The same table, in the language of the lab you're about to do:

| Task | Reach for |
| --- | --- |
| Find whether a value is already in the vector | `std::find` |
| Find the first element passing a test | `std::find_if` |
| Count elements passing a test | `std::count_if` |
| Rank elements by some key | `std::sort` with a comparator |
| Find the largest element | `std::max_element` |
| Print every element | range-based `for` |

That last row introduces one more algorithm worth knowing, because the lab uses it.

### `std::max_element` — and the empty-range guard

`std::max_element` returns an iterator to the **largest** element in a range (using `<` by default). Because it returns an *iterator*, it follows the same protocol as `std::find` — including what happens on an empty range.

```cpp
#include <algorithm>
#include <vector>

int largest(const std::vector<int>& vec)
{
    auto it { std::max_element(vec.begin(), vec.end()) };

    if (it == vec.end())          // empty range: begin() == end()
        return 0;                 // fallback; do NOT dereference

    return *it;
}
```

When the vector is empty, `vec.begin()` already equals `vec.end()`, so there's no maximum to point at and `std::max_element` returns `end()`. Dereferencing it would be the same undefined behavior as `*end()`. **Guard the empty case first**, then dereference. This guard is the exact pattern the lab's `largest` task tests — and the reason the lab gives it its own task is that forgetting the guard is the classic beginner crash.

> **Key insight:** Every algorithm that returns an iterator can return `end()` — and on an empty range, it *will*. `std::find`, `std::find_if`, and `std::max_element` all share the same reflex: check against `end()` before dereferencing. Learn it once; it protects you everywhere.

---

## 18.4 — Timing your code

### Why measure at all

Now that you can choose between a hand-written `O(n²)` sort and `std::sort`, a natural question follows: *how much does the choice actually cost?* **Timing** — measuring how long code takes to run — turns that from a guess into a number. You'll want it whenever:

- a loop feels too slow and you want to know where the time goes
- you're comparing two implementations of the same task
- a release build behaves very differently from a debug build
- an algorithm that was fine on small inputs becomes painful on large ones

But a warning up front, because it's the most common mistake people make with timing: **a single measurement is rarely the truth.** Timing is noisy. Run the same code twice and you'll get two different numbers.

### What makes timings noisy

Your measurement is at the mercy of the whole machine, not just your code:

- CPU speed and current load
- debug versus release build
- compiler optimizations
- input size and shape
- memory allocation patterns and cache behavior
- disk and other I/O
- how much you're printing
- the operating system scheduling other work

That printing point deserves emphasis, because it's a trap students fall into constantly. A stray line like

```cpp
std::cout << "testing " << x << '\n';
```

inside a tight loop can cost *more than the work you're trying to measure* — output is slow. If you instrument a loop with prints and then time it, you're often timing the prints.

So, to get a meaningful number: avoid timing debug spew, prefer release builds when you care about real speed, run enough iterations that the total is comfortably measurable, repeat the measurement, keep your inputs comparable, and stay especially wary of I/O.

### Measuring with `<chrono>`

The standard timing library is `<chrono>`. The pattern is: read the clock, do the work, read the clock again, subtract.

```cpp
#include <chrono>
#include <iostream>

int main()
{
    auto start { std::chrono::steady_clock::now() };

    // ... the code you want to measure ...

    auto stop { std::chrono::steady_clock::now() };

    auto elapsed { stop - start };
    std::cout << std::chrono::duration<double>(elapsed).count() << " seconds\n";

    return 0;
}
```

The `std::chrono::duration<double>(elapsed).count()` part converts the raw elapsed time into a plain `double` number of seconds you can print.

Notice the clock: `std::chrono::steady_clock`. Use it for elapsed-time measurements because it is **monotonic** — it only ever moves forward at a steady rate. The system clock, by contrast, can jump (a user adjusts it, an NTP sync nudges it), which would make a subtraction across that jump meaningless or even negative.

> **Best practice:** Use `std::chrono::steady_clock` to measure how long something takes. It can't be yanked backward or forward the way the wall-clock time can, so the difference between two readings is always a sane elapsed duration.

### A reusable `Timer`

Reading the clock twice by hand gets tedious. A tiny class wraps it up:

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

The timer starts itself the moment you create it; `elapsed()` tells you how many seconds have passed since then; `reset()` restarts the clock. Using it to time a real `std::sort` on a million random integers:

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

### Measure enough work — and outsmart the optimizer

Tiny operations are *too fast to measure*. The overhead of reading the clock dwarfs the thing you're timing:

```cpp
Timer timer {};
int x { 1 + 2 };                  // far faster than the measurement itself
std::cout << timer.elapsed() << '\n';
```

The fix is to repeat the work enough times that the total is significant:

```cpp
Timer timer {};

int total {};
for (int i { 0 }; i < 10'000'000; ++i)
    total += i % 7;

std::cout << "elapsed: " << timer.elapsed() << '\n';
std::cout << "total: " << total << '\n';      // print the result on purpose
```

That last line isn't decoration. An optimizing compiler is allowed to *delete* a loop whose result you never use — if nothing reads `total`, the entire loop is dead code and may vanish, leaving you timing nothing. Printing `total` forces the compiler to actually compute it.

> **Tip:** If you don't use a computed result, the optimizer may legally remove the code that produced it — and your timing measures zero work. "Use" the result (print it, accumulate it into a return value) so the work can't be optimized away.

### Debug versus release

This one catches everyone at least once. **Debug builds are much slower than release builds** — often several times slower. Debug builds keep extra information for the debugger and disable most optimizations; release builds optimize aggressively and strip that overhead.

```text
Debug build:    easy to step through, full of checks, slow
Release build:  optimized, hard to single-step, fast
```

Two rules follow. Use debug builds while you're hunting correctness bugs, where being able to step through matters more than speed. Switch to a release (optimized) build whenever you're measuring performance. And never, ever compare a debug build of one implementation against a release build of another — the build mode dominates the difference and tells you nothing about the code.

> **Warning:** Never compare timings across different build modes. A slow algorithm in release can easily beat a fast one in debug. Time both contenders under the *same*, optimized build, or your conclusion is noise.

### Timing a loop, the right unit of work

When the thing you care about is throughput — say, how fast you can generate transformed copies of some data — time the *batch*, not a single call. One call is too quick to measure reliably; ten thousand is informative:

```cpp
#include <chrono>
#include <iostream>
#include <string>
#include <vector>

std::string bump(std::string s)
{
    if (!s.empty())
        s[0] = static_cast<char>(s[0] + 1);
    return s;
}

int main()
{
    std::vector<std::string> inputs { "abc", "xyz", "hello" };
    std::vector<std::string> outputs {};

    Timer timer {};

    for (int round { 0 }; round < 100'000; ++round)
    {
        for (const std::string& s : inputs)
            outputs.push_back(bump(s));
    }

    std::cout << "produced " << outputs.size() << " strings\n";
    std::cout << "elapsed " << timer.elapsed() << " seconds\n";

    return 0;
}
```

This measures the loop's real throughput far better than timing one call to `bump`. But interpret it honestly — what you've measured isn't *only* the transformation:

- `push_back` may reallocate the growing `outputs` vector repeatedly
- copying strings has its own cost
- `outputs` consumes a lot of memory

If you want to isolate the transformation cost from the allocation cost, reserve the vector's capacity up front so it stops reallocating mid-loop (a Chapter 16 trick paying off here):

```cpp
outputs.reserve(inputs.size() * 100'000);
```

> **Note:** When you time a loop that builds a container, you may be measuring memory allocation as much as the work itself. `reserve()` removes the reallocation noise so the number reflects the operation you actually care about.

### Timing and the choice of algorithm

This brings the chapter full circle. Recall the two sorts:

```text
selection sort:  easy to understand, O(n^2), bad for large n
std::sort:       standard, far better asymptotically, heavily optimized
```

On ten elements you'd never notice the difference. On a hundred thousand, `std::sort` is the difference between "instant" and "go get a coffee." Timing is how you *see* that difference instead of merely being told about it — and now you can run the experiment yourself.

---

## Chapter 18 summary

You arrived at this chapter writing index-based loops by hand. You leave it with a vocabulary for talking about collections at a higher level.

**Sorting.** Sorting rearranges elements into an order without creating new values; copy first if you need the original order. **Selection sort** repeatedly picks the smallest remaining element and swaps it into place — a fine way to *understand* sorting, but `O(n²)` and far too slow for real use. In practice, always reach for **`std::sort`**.

**Iterators.** An iterator is an object representing a *position* in a sequence. The three operations are `*it` (access), `++it` (advance), and `it != end` (test). `begin()` points at the first element; `end()` points *one past* the last and **must never be dereferenced**. Algorithms work on the **half-open range** `[begin, end)` — start included, end excluded — a convention that runs through the entire standard library. Pointers are iterators for C-style arrays, which is why the two feel so alike. And **iterator invalidation** is the trap to respect: operations that resize or reorder a container (`push_back`, `insert`, `erase`, `resize`, sorting) can leave existing iterators dangling — reacquire them after mutating.

**Algorithms.** From `<algorithm>`: `std::find` finds a value; `std::find_if` finds the first element matching a **predicate**; `std::count` and `std::count_if` tally exact values and predicate matches; `std::sort` orders a range, optionally with a **comparator** (which must be a strict comparison — `<` or `>`, never `<=`/`>=`); `std::max_element` returns an iterator to the largest element. Every algorithm that returns an iterator returns `end()` to mean "not found" or "empty range" — **check against `end()` before dereferencing**, always. In this chapter your predicates and comparators are **free functions** passed by name (they decay to function pointers); Chapter 20's lambdas will make them more concise.

**Timing.** Use `<chrono>` with `std::chrono::steady_clock` to measure elapsed time. Time enough work to be significant, "use" computed results so the optimizer can't delete them, repeat measurements, and never compare across debug and release builds.

### What this sets up: the VecTools lab

The chapter's lab, **VecTools**, makes the central lesson physical. For each of five operations you write it **two ways**: a `hand*` version using an explicit iterator loop (`begin()`, `end()`, `++it`, `*it`, the `it != end` test) and an `algo*` version that delegates to a standard algorithm. The grader checks that the two versions *agree on every input* — which is exactly the point of this chapter. The standard algorithms aren't magic; they run the same iterator loop you'd write by hand, packaged under a name. As you fill in the tasks, watch for the four reflexes this chapter drilled:

- **Convert an iterator to an index** with `std::distance(vec.begin(), it)` — for a vector iterator this is `O(1)`.
- **Pass a free function** (`isEven`, `greaterThan`) by name to `std::count_if` and `std::sort`; it decays to a function pointer.
- **Use strict `>`** in your `sortDescending` comparator, never `>=` — the strict-weak-ordering rule, or it's undefined behavior.
- **Guard the empty range** before dereferencing `std::max_element`'s result — `end()` on an empty vector is the crash this chapter taught you to expect.

Get those four right and the rest is the iterator loop you now understand from the inside out.
