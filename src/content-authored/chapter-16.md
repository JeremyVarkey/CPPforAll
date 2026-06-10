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

Up to now, every value you have stored has lived in its own named variable. That works beautifully when you have a handful of values. It falls apart the moment you have many.

### The variable scalability problem

Suppose you are writing a program that records exam scores. Three students? No problem:

```cpp
int score1 {};
int score2 {};
int score3 {};
```

Now imagine the class has 30 students. Or 300. Or that you do not know how many students there are until the program runs. You cannot declare three hundred named variables, and you certainly cannot write a variable name you do not yet know. Worse, even if you *could* declare them all, there would be no way to write a single loop that processes "all the scores" — each name is a separate identifier, opaque to iteration.

What you actually want is one object that holds *many* values, that you can grow as needed, and that you can walk through with a loop. That object is a **container**.

### Containers

A **container** is an object designed to hold a collection of other objects — its **elements**. The elements do not get individual variable names. Instead, the container manages them as a group, and you reach a particular element by its *position*.

A good mental model is an egg carton. The carton is the container; each pocket holds one egg (one element); and you refer to an egg not by name but by which slot it sits in — "the third egg." You can count the eggs, replace one, or carry the whole carton around as a single thing.

```cpp
std::vector<int> scores { 90, 85, 100 };
```

This one object holds three integers:

```
scores
+----+----+-----+
| 90 | 85 | 100 |
+----+----+-----+
  0    1     2     <- positions (indices)
```

None of those three values has a name of its own. You access them by their position — element `0`, element `1`, element `2` — or you iterate over the whole collection at once. That single change in mindset, from "named values" to "positions in a collection," is what makes large-scale data processing possible.

### Length

The **length** (or **size**) of a container is the number of elements it currently holds. Every standard container can tell you its own length:

```cpp
std::vector<int> scores { 90, 85, 100 };
std::cout << scores.size() << '\n'; // prints 3
```

### What you can do with a container

The whole point of containers is that they support a standard repertoire of operations. Across the collection you will typically want to:

- create the container,
- ask how many elements it holds,
- access an element by position,
- add or insert an element,
- remove an element,
- iterate over every element,
- search, sort, or count.

Different container types make different trade-offs about which of these are fast, but the vocabulary is shared.

### Element type

A container is **homogeneous**: every element has the same type. You declare that element type when you name the container, inside the angle brackets:

```cpp
std::vector<int> numbers;            // a vector of ints
std::vector<std::string> seeds;      // a vector of strings
std::vector<double> readings;        // a vector of doubles
```

A `std::vector<int>` holds only `int`s; a `std::vector<std::string>` holds only `std::string`s. The element type is part of the container's type — a fact that matters later when we pass vectors to functions.

### Arrays

An **array** is a container whose elements are laid out *sequentially in memory*, one right after another, so that any element can be reached directly from its index. C++ gives you several array-like types, each with a different trade-off:

| Type            | Size    | Notes                                          |
|-----------------|---------|------------------------------------------------|
| C-style array   | fixed   | inherited from C; powerful but full of sharp edges |
| `std::array`    | fixed   | a safer fixed-size container (Chapter 17)      |
| `std::vector`   | dynamic | a *resizable* array — the star of this chapter |

This chapter is about `std::vector`: the array that can grow and shrink while your program runs. It is the container you will reach for most often, and the one the rest of this chapter teaches in depth.

---

## 16.2 — Introduction to std::vector and list constructors

`std::vector` is the standard library's **dynamic array** — a contiguous sequence of elements that can grow and shrink at runtime. It is the workhorse container of C++, and once you are comfortable with it, a huge amount of practical programming opens up.

To use it, include `<vector>`:

```cpp
#include <vector>

std::vector<int> values {}; // an empty vector of ints
```

That `{}` initializes the vector to empty: length zero, no elements yet. The vector owns its storage; when the vector goes out of scope, that storage is automatically released. You never call `new` or `delete` — the vector manages memory for you.

### List initialization

Most of the time you want a vector that *starts* with some values. List initialization, with values inside braces, does exactly that:

```cpp
std::vector<int> primes { 2, 3, 5, 7, 11 };
```

Each value in the braces becomes one element, in order:

```
primes
index:  0  1  2  3   4
value:  2  3  5  7  11
```

### The subscript operator

To read or write an element by position, use the subscript operator `[]` with the element's index:

```cpp
std::cout << primes[0] << '\n'; // prints 2 (the first element)
primes[1] = 13;                 // overwrite the second element
```

Indices are **zero-based** — a convention worth burning into memory now, because every array-like type in C++ follows it:

```
first element  -> index 0
second element -> index 1
last element   -> index (length - 1)
```

So a vector of length 5 has valid indices `0, 1, 2, 3, 4`. There is no index `5`.

### Out-of-bounds access and `.at()`

Here is the first sharp edge. The subscript operator `[]` does **not** check whether your index is valid. If you ask for an element that does not exist, you get **undefined behavior** — the program might crash, might print garbage, might appear to work and corrupt something later:

```cpp
std::cout << primes[99]; // undefined behavior — index 99 doesn't exist
```

When you want a safety net, use the `.at()` member function instead. It checks the index at runtime and throws an exception (`std::out_of_range`) if the index is invalid, turning a silent disaster into a loud, debuggable failure:

```cpp
std::cout << primes.at(99); // throws std::out_of_range — a clean failure
```

> **Tip:** While you are learning or debugging, prefer `.at()` — a thrown exception is far easier to diagnose than the silent corruption of `[]`. In a performance-critical inner loop where you have already proven the index is valid, `[]` avoids the bounds check. Reach for the safe tool first and optimize only when you have a reason.

### Contiguous memory

The elements of a vector live in one unbroken block of memory, side by side:

```
std::vector<int> v { 10, 20, 30 };

memory:
+----+----+----+
| 10 | 20 | 30 |
+----+----+----+
 &v[0] &v[1] &v[2]   <- consecutive addresses
```

This **contiguity** is not just a curiosity. It is why vectors are fast: indexing is a single address calculation, and walking the elements in order is friendly to the CPU's cache. It is also the property that makes a vector interoperate with code expecting a plain array.

### Constructing a vector of a specific length

Sometimes you want a vector of a known length, pre-filled, without listing each value. For that, use **parentheses** (not braces):

```cpp
std::vector<int> zeros(5);     // 5 elements, each value-initialized to 0
std::vector<int> sevens(5, 7); // 5 elements, each set to 7
```

The parenthesized form invokes a constructor that takes a count (and optionally a fill value). And here is a trap worth memorizing, because it bites everyone once:

```cpp
std::vector<int> a { 5 }; // ONE element with value 5
std::vector<int> b ( 5 ); // FIVE elements, all value 0
```

Braces and parentheses mean different things here. Braces prefer **list initialization** — a list of element values — so `{ 5 }` is the single-element list "5." Parentheses invoke the **count constructor** — so `( 5 )` means "give me five elements."

> **Warning:** `{ 5 }` and `( 5 )` build completely different vectors. Use braces when you are listing the actual elements; use parentheses when you are specifying a count. Mixing them up is a classic, hard-to-spot bug.

### const vectors

You can make a vector `const`, which fixes both its contents and its length for its lifetime:

```cpp
const std::vector<int> values { 1, 2, 3 }; // read-only; cannot modify or resize
```

A `std::vector` manages its storage dynamically, at runtime, so treat it as a **runtime** container rather than something you compute at compile time. When you simply need a fixed read-only list, `const std::vector` is the natural choice.

---

## 16.3 — std::vector and the unsigned length and subscript problem

There is a wrinkle in how vectors report their size, and it produces one of the most common compiler warnings beginners meet. It is worth understanding clearly, because the fix is simple once you see *why* the problem exists.

### Length is an unsigned type

When you ask a vector for its length, you do not get back an `int`. You get back an **unsigned** integer type — `std::size_t`, or equivalently the vector's own `size_type`. Unsigned types cannot represent negative numbers; they trade away the negatives in exchange for a larger positive range.

```cpp
std::vector<int> values { 1, 2, 3 };
auto length { values.size() }; // length has an unsigned type, not int
```

This is a deliberate design choice — a length can never legitimately be negative — but it collides with the fact that most of us write loop counters as plain signed `int`.

### Where the friction shows up

The trouble appears the instant you compare a signed counter against an unsigned size:

```cpp
for (int i { 0 }; i < values.size(); ++i) // signed/unsigned comparison warning
{
    // ...
}
```

`i` is signed; `values.size()` is unsigned. Comparing them directly makes the compiler nervous, and with warnings turned on (which you should always have) it will say so.

### Why the compiler is nervous

The danger is not academic. When a signed value and an unsigned value meet in an expression, the signed one is **converted to unsigned** — and a negative signed value becomes an enormous positive one:

```cpp
int i { -1 };
std::size_t size { 3 };

// In a comparison, i converts to a huge unsigned value —
// so (i < size) can be FALSE even though -1 is "less than" 3.
```

A loop that relies on a counter dipping below zero, or an off-by-one that produces `-1`, can behave in a way that looks impossible until you remember this conversion.

### `std::ssize` — a signed length (C++20)

C++20 added `std::ssize(container)`, which returns the container's length as a **signed** type. This lets a signed loop counter compare against the length cleanly:

```cpp
#include <iterator> // for std::ssize

for (int i { 0 }; i < std::ssize(values); ++i)
{
    std::cout << values[static_cast<std::size_t>(i)] << '\n';
}
```

Note that you still cast `i` back to an unsigned type when you index, because `operator[]` expects an unsigned index. The signedness mismatch does not vanish — it just moves to the single, obvious place where you index, instead of polluting the comparison.

### Your three practical options

There is no single "right" answer, but three patterns cover essentially everything you will write. In rough order of preference:

**1. Avoid indexing entirely.** If you do not need the index, a range-based `for` loop sidesteps the whole sign problem — there is no counter to mismatch:

```cpp
for (const auto& value : values)
{
    std::cout << value << '\n';
}
```

**2. Use an unsigned counter.** When you do need the index, make the counter `std::size_t` so it matches the size type exactly — no warning, no cast:

```cpp
for (std::size_t i { 0 }; i < values.size(); ++i)
{
    std::cout << values[i] << '\n';
}
```

**3. Use a signed counter with `std::ssize` and cast at the index.** When the surrounding arithmetic genuinely needs signed math, keep the counter signed and cast only where you index:

```cpp
for (int i { 0 }; i < std::ssize(values); ++i)
{
    auto index { static_cast<std::size_t>(i) };
    std::cout << values[index] << '\n';
}
```

> **Best practice:** When you need an index, prefer an unsigned `std::size_t` counter so it matches `size()`'s type and there is nothing to cast. When you need signed arithmetic, cast *at the boundary* where the vector demands its index type — not casually scattered through the code.

---

## 16.4 — Passing std::vector

A vector can hold a lot of data, and that data lives in dynamically-allocated storage the vector owns. How you pass a vector to a function therefore matters — get it wrong and you silently copy the entire collection on every call.

### Do not pass by value unless you mean to copy

If you pass a vector **by value**, the function receives its own private copy. Every element is duplicated:

```cpp
void print(std::vector<int> values); // copies the ENTIRE vector on each call
```

For a small vector this is merely wasteful; for a large one it is a real performance bug. Pass by value only when the function genuinely needs its own independent copy to modify.

### Read-only: pass by const reference

When a function only *reads* the vector, pass it by **const reference**. A reference is an alias — no copy is made — and `const` promises the function will not modify the caller's data:

```cpp
void print(const std::vector<int>& values)
{
    for (const auto& value : values)
        std::cout << value << '\n';
}
```

This is the default you should reach for whenever a function inspects a vector without changing it.

### Mutation: pass by non-const reference

When a function needs to *change* the caller's vector — append to it, edit its elements — pass by **non-const reference**. Again no copy is made, and dropping `const` signals that modification is intended:

```cpp
void appendSeed(std::vector<std::string>& seeds, const std::string& seed)
{
    seeds.push_back(seed); // grows the caller's vector
}
```

> **Best practice:** Pass a vector by `const` reference for read-only access, and by non-`const` reference when the function must modify it. Reserve pass-by-value for the rare case where the function truly needs its own copy.

### Element type is part of the type

Remember that the element type is baked into the vector's type. These are three *different* types:

```cpp
std::vector<int>
std::vector<double>
std::vector<std::string>
```

A function declared to take `const std::vector<int>&` accepts *only* a vector of `int`. It will not accept a `std::vector<double>` — there is no implicit conversion between vector types.

### Accepting any element type with a template

If you want one function that works for vectors of *any* element type, make it a **function template**, with the element type as a type parameter:

```cpp
template <typename T>
void printVector(const std::vector<T>& values)
{
    for (const auto& value : values)
        std::cout << value << '\n';
}
```

Now `printVector` works on `std::vector<int>`, `std::vector<std::string>`, and so on — the compiler stamps out a version for each element type you use it with. You do not need to master templates here; just recognize the pattern as the standard way to write container code that is generic over its element type.

### Checking an expected length with assert

If a function assumes the vector has a particular length, make that assumption explicit with an assertion:

```cpp
#include <cassert>

void processRgb(const std::vector<int>& rgb)
{
    assert(rgb.size() == 3); // a programmer expectation, checked in debug builds
    // ... safe to use rgb[0], rgb[1], rgb[2] ...
}
```

> **Note:** `assert` documents and checks a programmer's assumption during development; it is not user-facing error handling. Use it to catch *your own* bugs, not to validate input a user might legitimately get wrong.

---

## 16.5 — Returning std::vector, and an introduction to move semantics

Passing a vector *in* without copying is straightforward — you use a reference. Returning a vector *out* of a function looks like it should be expensive, since the local vector is about to be destroyed. In modern C++ it usually is not, and the reason is worth understanding.

### Copying a vector copies its elements

First, the baseline. Copying a vector duplicates every element into fresh storage:

```cpp
std::vector<std::string> a { "x", "y" };
std::vector<std::string> b { a }; // copies both strings into b's own storage
```

For a vector of strings, that means allocating new storage *and* copying each string. The cost grows with the data. This is what we want to avoid when we can.

### Move semantics: transferring instead of duplicating

Here is the key idea. When the source of an initialization is a **temporary** — an object that is about to be destroyed anyway — there is no reason to laboriously copy its elements into the destination and then throw the original away. Instead, the destination can simply *take over* the source's existing storage. This is **move semantics**.

```
copy:  duplicate all elements into brand-new storage
move:  hand over ownership of the existing storage — no element copying
```

Picture a vector handing off its internal buffer:

```
before move:
  source -> [a][b][c]
  dest   -> (empty)

after move:
  source -> (valid but unspecified — typically empty)
  dest   -> [a][b][c]   (the very same buffer, now owned by dest)
```

A move is cheap and constant-time regardless of how many elements there are, because it transfers a handle to the storage rather than copying the contents. The moved-from source is left in a valid but unspecified state — safe to destroy or reassign, but you should not assume what it contains.

### Returning a vector by value is fine

Because of move semantics — and an even better optimization called **copy elision**, where the compiler constructs the result directly in the caller with no copy or move at all — returning a vector by value is a normal, efficient pattern:

```cpp
std::vector<std::string> makeSeeds()
{
    std::vector<std::string> seeds {};
    seeds.push_back("abc");
    seeds.push_back("xyz");
    return seeds; // moved or elided — not deep-copied
}
```

The local `seeds` is built up inside the function and handed back to the caller without an expensive copy. Write the function the obvious way and trust the compiler to do the right thing.

### Why "in" and "out" differ

It can seem inconsistent that we avoid passing *by value* but happily return *by value*. The distinction comes down to what each one copies.

Passing by value copies an object that *already exists and keeps existing* — the caller's vector is still theirs after the call, so the function needs a genuine, separate copy:

```cpp
void f(std::vector<int> values); // caller still owns their vector; f gets a copy
```

Returning by value produces a result from a *local* object that is ending its life anyway — exactly the situation move semantics and copy elision were designed for. The returned value can be moved or elided into the caller essentially for free.

> **Best practice:** Return vectors by value when a function produces one — it is clean and, thanks to move semantics and copy elision, efficient. Do not contort your design into "out parameters" out of a vague fear that vectors are expensive to return. They usually are not.

---

## 16.6 — Arrays and loops

This is where containers earn their keep. A loop and an array together let you write *one* piece of code that processes *any* number of elements — three or three thousand — without change. That combination is the foundation of nearly all data processing.

### The same code, any size

Without a container, processing values means writing out each one by hand, and the code's length is tied to the data's size:

```cpp
std::cout << a << b << c; // only ever handles exactly three values
```

With a vector and a loop, the *code* stays the same no matter how many elements there are:

```cpp
for (const auto& value : values)
{
    std::cout << value << '\n';
}
```

That loop prints every element whether `values` holds three readings or three million. Nothing in it mentions a specific count — the loop asks the container how far to go.

### Index-based loops

One way to walk a vector is by index: count from `0` up to (but not including) the length, using the counter to subscript each element:

```cpp
for (std::size_t i { 0 }; i < values.size(); ++i)
{
    std::cout << values[i] << '\n';
}
```

Use this form when you actually need the index — for example, to print a position alongside each value, or to compare an element with its neighbor.

### Range-based loops

When you do *not* need the index — you just want each element in turn — the range-based `for` loop is cleaner and harder to get wrong:

```cpp
for (const auto& value : values)
{
    std::cout << value << '\n';
}
```

There is no counter to manage, so there is no counter to mismanage. We will study this form in detail in lesson 16.8.

> **Best practice:** Prefer a range-based `for` loop when you do not need the index. Reserve index loops for when the position itself is part of the computation.

### Off-by-one errors and the half-open range

The most common bug with index loops is going one step too far. For a vector of length `n`, the valid indices are:

```
0 through n - 1
```

So the loop condition must use `<`, not `<=`. This version reads one element past the end on its last iteration — undefined behavior:

```cpp
for (std::size_t i { 0 }; i <= values.size(); ++i) // BUG: <= overshoots
{
    std::cout << values[i]; // on the last pass, i == size(): out of bounds
}
```

The correct condition stops *before* the length:

```cpp
for (std::size_t i { 0 }; i < values.size(); ++i) // correct
{
    std::cout << values[i] << '\n';
}
```

A useful way to think about this is the **half-open range** `[0, size)`: the start index `0` is included, the end index `size` is excluded. The loop visits `0, 1, ..., size - 1` and stops. Internalize "less than the size, not less than or equal," and a whole class of bugs disappears.

---

## 16.7 — Arrays, loops, and sign challenge solutions

Lesson 16.3 introduced the tension between signed loop counters and unsigned container lengths. Now that you are writing real loops, let us lay the options side by side so you can pick deliberately rather than fighting warnings.

### The challenge, stated plainly

You would like all of these at once:

- signed loop counters, because signed arithmetic is generally less surprising,
- unsigned indices, because standard containers index with unsigned types,
- no signed/unsigned comparison warnings,
- no accidental narrowing or conversion bugs.

You cannot have all four with zero effort, but each of the following patterns gives you a clean, defensible subset.

### Option 1 — unsigned loop variable

Make the counter `std::size_t` so it matches `size()` exactly. No warning, no cast:

```cpp
for (std::size_t i { 0 }; i < values.size(); ++i)
{
    std::cout << values[i] << '\n';
}
```

This is the simplest forward loop and the one the lab uses for its index-based task. One caveat: be careful with **reverse** loops, because an unsigned counter can never go below zero. This loop never terminates — when `i` is `0` and you decrement, it wraps around to a huge number instead of becoming `-1`:

```cpp
for (std::size_t i { values.size() - 1 }; i >= 0; --i) // BUG: i >= 0 is always true
{
    // ...
}
```

(There are correct ways to count down with an unsigned type, but the naive `>= 0` test is not one of them — the condition can never be false.)

### Option 2 — signed loop variable with a cast at the index

Keep the counter signed, compare against the signed `std::ssize`, and cast only at the point where you index:

```cpp
#include <iterator>

for (int i { 0 }; i < std::ssize(values); ++i)
{
    std::cout << values[static_cast<std::size_t>(i)] << '\n';
}
```

The cast is confined to the one spot where the vector insists on an unsigned index. Everything else is ordinary signed arithmetic, which makes reverse loops and signed math straightforward.

### Option 3 — avoid indexing altogether

When the index is not part of the computation, the cleanest option is to not have an index at all:

```cpp
for (const auto& value : values)
{
    std::cout << value << '\n';
}
```

There is no counter, so there is no sign problem, no off-by-one, and nothing to cast. Whenever this fits, it is the right choice.

> **Key insight:** The sign problem only exists when you introduce an index. If you do not need the index, do not invent one — let a range-based `for` carry you over the whole collection. When you genuinely need the index, an unsigned `std::size_t` counter is the simplest fix; reach for the signed-plus-cast pattern only when the surrounding math truly needs signed values.

---

## 16.8 — Range-based for loops (for-each)

The range-based `for` loop — often called the **for-each** loop — is the most pleasant way to iterate a container, and the one you will use most. It deserves careful study, because the small choice of *how you declare the loop variable* has real consequences for correctness and performance.

### The basic form

A range-based `for` declares a loop variable and the container to walk, separated by a colon:

```cpp
for (int value : values)
{
    std::cout << value << '\n';
}
```

Read it literally as: "for each `value` in `values`." On each iteration, the loop variable takes on the next element, and the body runs. No index, no condition, no increment to write.

### Empty containers need no special case

If the container is empty, the loop body simply runs zero times. You do not need to guard against the empty case — the loop already handles it. This is why the lab's `totalReading` can return `0` for an empty vector "for free": the accumulation loop never executes, and the running total stays at its initial value.

### Use `auto` for the element type

Rather than spell out the element type, let the compiler deduce it with `auto`:

```cpp
for (auto value : values)
{
    // ...
}
```

But notice what this does: `auto value` makes `value` a **copy** of each element. For an `int` that copy is trivial. For a `std::string` or a large struct, copying every element on every iteration is wasteful — and if you assign to `value`, you are modifying the copy, not the element in the container.

### Avoid copies with references

The fix is to make the loop variable a **reference** — an alias for the actual element — instead of a copy.

For read-only access to elements you will not modify, use `const auto&`. No copy is made, and `const` documents that you are only reading:

```cpp
for (const auto& seed : seeds)
{
    std::cout << seed << '\n'; // reads each string directly, no copy
}
```

To **modify** the elements in place, drop the `const` and use `auto&`. Now the loop variable is an alias for the real element, so writing through it changes the container:

```cpp
for (auto& seed : seeds)
{
    seed += "!"; // actually appends to each element in seeds
}
```

That single `&` is the entire difference between editing the container and quietly editing a throwaway copy. It is one of the most consequential characters in everyday C++.

Here is the decision table to keep in your head:

| Loop variable        | Meaning                                       |
|----------------------|-----------------------------------------------|
| `auto value`         | a **copy** of each element                    |
| `const auto& value`  | **read** each element with no copy            |
| `auto& value`        | **mutate** each element in place              |
| `auto&& value`       | advanced generic-/range-code (forwarding ref) |

> **Best practice:** For read-only iteration, default to `const auto&` — it avoids copies and clearly states your intent. Use `auto&` when you need to modify the elements. Use a plain `auto` copy only when you specifically want a per-iteration copy you can scribble on without touching the original.

### Getting at the index when you need it

A range-based `for` deliberately does not hand you an index — that is part of its simplicity. If you need one anyway, either fall back to an index loop or keep your own counter alongside the range-for:

```cpp
int index { 0 };
for (const auto& seed : seeds)
{
    std::cout << index << ": " << seed << '\n';
    ++index;
}
```

That said, if you find yourself reaching for the index inside nearly every range-for, that is a hint an index-based loop might fit the problem better.

### Iterating in reverse

A range-based `for` walks forward. Iterating a vector in reverse is genuinely useful sometimes, but the clean tools for it (reverse iterators, and C++20's range adaptors) come in later chapters. For now, if you need to go backward, a careful index loop will do. Do not reach for advanced machinery before you need it.

---

## 16.9 — Array indexing and length using enumerators

Raw numeric indices like `stats[0]` and `stats[1]` carry no meaning to a reader — you have to remember what lives at each position. When the positions of a vector stand for *named things*, enumerators can make the indexing self-documenting.

### Naming indices with an enum

Recall unscoped enumerators from Chapter 13: they are named integer constants that, by default, count up from `0`. That is exactly the shape of array indices — so an unscoped enum can give meaningful names to positions:

```cpp
enum Stat
{
    testsRun,      // 0
    crashesFound,  // 1
    statCount,     // 2
};

std::vector<int> stats(statCount); // a vector of 2 ints, value-initialized
stats[testsRun] = 100;
stats[crashesFound] = 2;
```

Compare that with the anonymous version:

```cpp
stats[0] = 100;
stats[1] = 2;   // which statistic is "1" again?
```

The enum version reads like prose. Because unscoped enumerators implicitly convert to integers, `stats[testsRun]` just works as an index — no cast needed.

### The "count" enumerator trick

Notice the last enumerator, `statCount`, in the example above. Placed at the end, it automatically takes the value equal to the *number* of preceding enumerators — a tidy way to keep a "how many are there" constant in sync with the list itself:

```cpp
enum Campaign
{
    mutationA,     // 0
    mutationB,     // 1
    mutationC,     // 2
    campaignCount, // 3 == number of real campaigns
};

std::vector<int> counts(campaignCount); // exactly 3 elements
```

Add a new campaign before `campaignCount`, and the count updates by itself. You never have to remember to bump a separate "3" somewhere.

### Asserting the length matches

If a vector must line up with the enum's count, say so with an assertion. It documents the invariant and catches a mismatch during development:

```cpp
assert(counts.size() == campaignCount);
```

### The friction with scoped enums

Scoped enumerators (`enum class`, from Chapter 13) are more type-safe — but that safety means they do **not** implicitly convert to integers, so you cannot use them directly as indices:

```cpp
enum class Campaign { A, B, C, Count };

// std::vector<int> counts(Campaign::Count); // error: no implicit conversion to an integer
auto index { static_cast<std::size_t>(Campaign::A) }; // must cast explicitly
```

You gain type safety and lose convenience: every use needs a `static_cast`. That trade-off is sometimes worth it and sometimes not.

> **Tip:** Unscoped enumerators are convenient as array indices precisely because they convert to integers freely. If the index-as-name pattern starts feeling clumsy — lots of casts, or positions that are not really sequential — consider whether a `struct` with named members, a `std::array`, or a `std::map` would model the data more clearly.

---

## 16.10 — std::vector resizing and capacity

The defining feature of `std::vector` is that it is *dynamic* — it can change size while your program runs. Understanding how it grows reveals two related-but-distinct concepts, **length** and **capacity**, and clears up a bug that catches almost everyone.

### Resizing at runtime

Unlike a fixed-size array, a vector can be resized after creation with `resize`:

```cpp
std::vector<int> values { 1, 2, 3 };
values.resize(5); // now length 5: { 1, 2, 3, 0, 0 } — new elements value-initialized
values.resize(2); // now length 2: { 1, 2 } — extra elements dropped
```

Growing with `resize` adds value-initialized elements at the end; shrinking removes elements from the end.

### Length versus capacity

Here is the crucial distinction. A vector tracks two different numbers:

- **Length** (`size()`) — the number of elements that currently exist and that you may access.
- **Capacity** (`capacity()`) — the number of elements the vector's *currently-allocated storage* could hold before it must allocate a bigger block.

Capacity is always at least as large as length, and often larger. The extra room is reserved space, sitting ready but not yet holding real elements:

```
capacity 8
+----+----+----+----+----+----+----+----+
| 10 | 20 | 30 |  ? |  ? |  ? |  ? |  ? |
+----+----+----+----+----+----+----+----+
  <-- length = 3 -->   <-- spare capacity -->
```

```cpp
values.size();     // how many elements exist (length)
values.capacity(); // how many it can hold before reallocating (capacity)
```

### Reallocation: what happens when you outgrow capacity

When you add an element and the length would exceed the capacity, the vector must **reallocate**: it requests a new, larger block of memory, moves (or copies) the existing elements into it, and releases the old block.

```
old storage: [a][b][c]          capacity 3, length 3  (full)
push_back(d):
new storage: [a][b][c][d][?][?] capacity ~6, length 4
old storage released
```

Reallocation is comparatively expensive — it allocates memory and relocates every element. It has one more important consequence:

> **Warning:** A reallocation invalidates every pointer, reference, and iterator that referred to the vector's elements. After the vector reallocates, those old handles point at freed memory. Never hold a raw pointer or reference to a vector element across an operation that might cause the vector to grow.

### Why capacity exists at all

If a vector allocated fresh storage on *every* `push_back`, building up a large vector would mean a reallocation per element — slow, and brutal on memory. Instead, when a vector reallocates it grabs *more* room than it immediately needs. That spare capacity absorbs many subsequent additions with no further allocation, making a long run of `push_back`s efficient on average.

### Capacity is not permission to index

This is the bug everyone hits once. Capacity is *reserved storage*, not *existing elements*. You may only index positions that are within the **length**:

```cpp
std::vector<int> values {};
values.reserve(10); // capacity becomes >= 10, but length is still 0

// values[0] = 1;   // WRONG: length is 0; there is no element 0 yet — undefined behavior
```

To actually create elements, change the **length** with `resize` (or grow it with `push_back`):

```cpp
values.resize(10); // now length 10; elements 0..9 exist
values[0] = 1;     // OK now
```

### Shrinking does not necessarily free memory

Shrinking the length with `resize` removes elements, but it does **not** guarantee the capacity shrinks too:

```cpp
values.resize(2); // length is now 2; capacity may still be 10
```

The vector may keep the larger allocation around in case you grow again. That is usually what you want — but if you ever truly need to reduce capacity, that is a separate operation, not a side effect of shrinking the length.

---

## 16.11 — std::vector and stack behavior

A **stack** is a last-in, first-out (LIFO) collection: the last item you add is the first one you take back. `std::vector` supports stack operations directly, which makes it a natural fit for "to-do" workloads where you keep pulling the most recent item and possibly adding more.

### What a stack is

Think of a stack of plates. You add to the top and remove from the top:

```
push A        push B        push C
[A]           [A][B]        [A][B][C]

pop -> C   then   pop -> B   then   pop -> A
```

The first plate in is the last one out. That ordering — LIFO — is the whole idea.

### Vector stack operations

Three member functions give a vector its stack interface, all operating on the **end** of the vector (which is the cheap end to modify, since no other elements shift):

```cpp
std::vector<std::string> stack {};

stack.push_back("seed1"); // add to the end (top of stack)
stack.push_back("seed2");

std::string top { stack.back() }; // read the last element (the top): "seed2"
stack.pop_back();                 // remove the last element
```

| Operation         | Meaning                          |
|-------------------|----------------------------------|
| `push_back(x)`    | add element `x` to the end       |
| `back()`          | access the last element          |
| `pop_back()`      | remove the last element          |

Note that `pop_back()` removes the top element but does **not** return it. The usual idiom is to read it with `back()` first, then `pop_back()`.

### `reserve`: capacity ahead of time

When you know roughly how many elements you will end up pushing, you can tell the vector to allocate that much capacity up front with `reserve`. This avoids the series of reallocations that would otherwise happen as the vector grows:

```cpp
std::vector<std::string> seeds {};
seeds.reserve(1000); // capacity becomes >= 1000; length is STILL 0
```

`reserve` changes **capacity only** — it does not create elements, and `size()` stays where it was. It is purely a performance hint: "I am about to add a lot; please allocate once instead of many times."

```
after reserve(1000):
  length   = 0
  capacity >= 1000
```

This is exactly what the lab's `buildRamp` does — `reserve(n)` first, then `push_back` in a loop — so all the pushes land in already-allocated storage with no mid-loop reallocation.

> **Best practice:** When you know about how many elements you will add, call `reserve` before the loop of `push_back`s. Remember it sets capacity, not length — you still need `push_back` (or `resize`) to actually add elements.

### `push_back` versus `emplace_back`

There are two ways to add an element to the end:

```cpp
seeds.push_back("abc");    // add a value (constructs a string, then moves it in)
seeds.emplace_back("abc"); // construct the element in place from these arguments
```

`push_back` takes an already-formed value (or one the compiler forms for you) and places it at the end. `emplace_back` forwards its arguments to the element's constructor and builds the element *directly* in the vector's storage, which can avoid a temporary for more complex element types.

> **Tip:** When you already have the object, `push_back` is clear and perfectly fine. When you would otherwise construct a temporary just to hand it to the vector — especially for heavier element types — `emplace_back` can construct it in place and save that step. For simple cases the difference is negligible; reach for clarity.

### Using a vector as a work stack

Putting it together, here is a vector driving a simple worklist — a pattern you will meet again and again in real systems software (a fuzzer's pool of interesting inputs is exactly this shape):

```cpp
std::vector<std::string> work {};
work.reserve(1000);            // pre-allocate; we expect to push a lot
work.push_back(initialSeed);

while (!work.empty())          // keep going until the stack is empty
{
    std::string item { work.back() }; // read the top
    work.pop_back();                  // remove it

    auto result { process(item) };
    if (isInteresting(result))
        work.push_back(result);       // schedule more work
}
```

Each iteration pops the most recent item, does something with it, and may push new items to be handled later — a complete LIFO worklist built entirely from `push_back`, `back`, `pop_back`, and `empty`.

---

## 16.12 — std::vector<bool>

There is one corner of `std::vector` that behaves differently from everything you have learned so far, and the practical advice is short: know it exists, and generally avoid it.

### The odd one out

`std::vector<bool>` is **not** a normal vector of `bool` objects. The standard library special-cases it into a *space-optimized* form that packs the boolean values into individual **bits** — eight booleans per byte — rather than storing each as a full `bool`.

```
a normal std::vector<T>:
  each element is a real T object you can take a reference or pointer to

std::vector<bool>:
  values are packed into bits; element access returns a PROXY object,
  not a true bool&
```

The intent was noble — save memory — but the consequence is that `std::vector<bool>` quietly breaks the usual rules. Because an individual bit has no address of its own, indexing does not yield a real `bool&`; it yields a stand-in **proxy** object. That makes references, pointers, and generic code that assumes "this acts like every other vector" behave in surprising ways.

### What to do instead

> **Best practice:** Avoid `std::vector<bool>`. When you need a growable sequence of boolean-ish flags that behaves like a normal vector, use a vector of a small integer type. When you need a fixed number of bits known at compile time, use `std::bitset`.

```cpp
#include <bitset>
#include <cstdint>
#include <vector>

std::vector<char> flags;          // a normal vector that behaves predictably
std::vector<std::uint8_t> bytes;  // also fine; each element is a real object
std::bitset<32> fixedFlags;       // fixed compile-time number of bits
```

Each of these behaves exactly as you now expect a container to behave — no proxy surprises. Save `std::vector<bool>` for the rare, deliberate case where its bit-packing genuinely matters and you have accounted for its quirks.

---

## 16.x — Chapter 16 summary and quiz

You have learned the container you will use more than any other. Let us consolidate.

### Core takeaways

- A **container** holds a collection of unnamed elements, accessed by position.
- `std::vector<T>` is a **dynamic, contiguous array** of `T` — it can grow and shrink at runtime and manages its own memory.
- Vector indices are **zero-based**: a length-`n` vector has valid indices `0` through `n - 1`.
- `operator[]` is **unchecked** (undefined behavior out of bounds); `.at()` is **bounds-checked** (throws on a bad index).
- **Length** (`size()`) and **capacity** (`capacity()`) are different: length is how many elements exist; capacity is how much storage is allocated.
- **Reallocation** (when length outgrows capacity) invalidates existing pointers, references, and iterators to elements.
- `reserve` changes **capacity**, not length. `resize` changes **length**.
- Pass vectors by **`const` reference** for read-only access, by **non-`const` reference** to modify them, and by value only when you truly need a copy.
- **Returning** a vector by value is normal and efficient in modern C++ (move semantics, copy elision).
- Prefer a **range-based `for`** when you do not need the index. Use **`const auto&`** to read elements without copying, **`auto&`** to modify them in place.
- Be deliberate about the **signed/unsigned** length-and-index issue: prefer an unsigned `std::size_t` counter, or avoid indexing entirely.
- `std::vector<bool>` is a special, bit-packed type — generally **avoid** it.

### Putting it to work

You are now ready for the chapter lab, the **Sensor-Readings Toolkit** — a small library of pure functions over `std::vector<int>` and `std::vector<double>` that exercises every idiom above. Watch for these patterns as you build it:

Read-only iteration with `const auto&`:

```cpp
int totalReading(const std::vector<int>& v)
{
    int sum { 0 };
    for (const auto& x : v) // no copies; empty vector -> loop runs 0 times -> returns 0
        sum += x;
    return sum;
}
```

In-place mutation with `auto&` — the `&` is the whole lesson:

```cpp
void normalizeInPlace(std::vector<double>& v, double scale)
{
    for (auto& x : v) // alias for the real element; without & the writes vanish
        x /= scale;
}
```

An index loop with an unsigned counter, when the position matters:

```cpp
int maxReading(const std::vector<int>& v)
{
    assert(!v.empty());
    int maxVal { v[0] };
    for (std::size_t i { 1 }; i < v.size(); ++i) // size_t matches v.size()
    {
        if (v[i] > maxVal)
            maxVal = v[i];
    }
    return maxVal;
}
```

Building and returning a vector — `reserve`, then `push_back`, then return by value:

```cpp
std::vector<int> buildRamp(int n)
{
    std::vector<int> result {};
    if (n <= 0)
        return result; // empty for non-positive n
    result.reserve(static_cast<std::size_t>(n)); // capacity, not length
    for (int i { 0 }; i < n; ++i)
        result.push_back(i);
    return result; // moved/elided, not deep-copied
}
```

> **Warning:** Remember the cast-before-divide rule from earlier chapters when you compute an average: `static_cast<double>(sum) / static_cast<double>(v.size())`. Casting *after* the division is too late — integer division has already truncated.

### Mini drill

A compact program that ties the chapter together — building a vector inside a function, returning it by value, and iterating it read-only:

```cpp
#include <iostream>
#include <string>
#include <vector>

std::vector<std::string> makeMutants(const std::string& seed)
{
    std::vector<std::string> mutants {};
    mutants.reserve(3); // we know we will add three

    mutants.push_back(seed + "!");
    mutants.push_back(seed + "?");
    mutants.push_back(seed + "#");

    return mutants; // return by value — efficient
}

int main()
{
    auto mutants { makeMutants("abc") };

    for (const auto& mutant : mutants) // const auto& — read each string, no copy
    {
        std::cout << mutant << '\n';
    }
}
```

This single example reinforces five chapter ideas at once: a vector owns a dynamic list; `reserve` prepares capacity without changing length; `push_back` grows the length; returning by value is the right, efficient default; and `const auto&` reads each element without copying it.
