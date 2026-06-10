# Chapter 17 — Fixed-Size Arrays: std::array and C-Style Arrays

> Source: <https://www.learncpp.com/> (Chapter 17)

In the last chapter you met `std::vector`, a container that grows and shrinks as your program runs. That flexibility is exactly what you want when you don't know up front how many elements you'll have — readings streaming off a sensor, words typed by a user, rows pulled from a file. But a surprising amount of the data you'll work with isn't like that. The days of the week are always seven. An RGB color is always three channels. A tic-tac-toe board is always nine cells. The set of error categories your tool reports is fixed the day you write it. When the size is baked into the problem, paying for a vector's runtime flexibility — its heap allocation, its capacity bookkeeping — is paying for something you'll never use.

This chapter is about the tools for *fixed-size* arrays, where the length is known when you compile. There are two of them, and they bracket the history of the language. **`std::array`** is the modern one: a thin, safe, standard-library wrapper that remembers its own length and can live entirely at compile time. The **C-style array** is the original, inherited straight from C — fast, ubiquitous in old code and system APIs, and carrying a famous trap called *decay* that has bitten every C programmer at least once. We'll teach `std::array` first because it's what you should reach for, then C-style arrays so you understand the machinery underneath — pointers, pointer arithmetic, null-terminated strings — that the rest of C++ is quietly built on. By the end you'll know which tool fits which job, and why "do I still have the array, or has it turned into a pointer?" is a question worth asking out loud.

## Contents
- [17.1 — Introduction to std::array](#171--introduction-to-stdarray)
- [17.2 — std::array length and indexing](#172--stdarray-length-and-indexing)
- [17.3 — Passing and returning std::array](#173--passing-and-returning-stdarray)
- [17.4 — std::array of class types, and brace elision](#174--stdarray-of-class-types-and-brace-elision)
- [17.5 — Arrays of references via std::reference_wrapper](#175--arrays-of-references-via-stdreference_wrapper)
- [17.6 — std::array and enumerations](#176--stdarray-and-enumerations)
- [17.7 — Introduction to C-style arrays](#177--introduction-to-c-style-arrays)
- [17.8 — C-style array decay](#178--c-style-array-decay)
- [17.9 — Pointer arithmetic and subscripting](#179--pointer-arithmetic-and-subscripting)
- [17.10 — C-style strings](#1710--c-style-strings)
- [17.11 — C-style string symbolic constants](#1711--c-style-string-symbolic-constants)
- [17.12 — Multidimensional C-style Arrays](#1712--multidimensional-c-style-arrays)
- [17.13 — Multidimensional std::array](#1713--multidimensional-stdarray)
- [17.x — Chapter 17 summary and quiz](#17x--chapter-17-summary-and-quiz)

---

## 17.1 — Introduction to std::array

### Three kinds of array, one decision

C++ gives you a small family of array-like tools, and the first skill is knowing which one a given situation calls for. The deciding question is almost always: *when do you know how many elements there are?*

| Type | Size known when? | Can resize? | Reach for it when… |
|---|---|---|---|
| `std::vector<T>` | runtime | yes | the count is only known while running, or it changes |
| `std::array<T, N>` | compile time | no | the count is fixed and known as you write the code |
| C-style array `T[N]` | compile time | no | interoperating with C, or a global compile-time table |

`std::vector` was the last chapter. This chapter is the bottom two rows. We start with `std::array`, the one you'll actually want most of the time.

`std::array` is a fixed-size array container that lives in the `<array>` header:

```cpp
#include <array>

std::array<int, 4> counters {};
```

That declaration creates an array of four `int`s, all set to zero. Notice the two pieces of information in the angle brackets:

| Template argument | Meaning |
|---|---|
| `int` | the element type |
| `4` | the number of elements |

Both of those are part of the type. This is the single most important idea in the chapter, so let it land now: an array of four `int`s and an array of five `int`s are *different types*, as different as `int` and `double`.

```cpp
std::array<int, 4> four {};
std::array<int, 5> five {};
```

`four` and `five` cannot be assigned to one another, cannot be passed to the same plain function, cannot be mixed up. The length is welded to the type, and that is precisely the safety C-style arrays will turn out to lack.

### Why std::array exists

You might reasonably ask: if `std::vector` can do everything a fixed-size array can do and more, why bother? Three reasons.

First, **honesty**. When you write `std::array<int, 3>`, the type itself announces "exactly three, never more, never fewer." A reader — and the compiler — knows the size can't drift.

Second, **no heap**. A `std::vector` stores its elements in dynamically allocated memory; constructing one means a trip to the allocator. A `std::array` stores its elements right inside itself, on the stack (or wherever the object lives). For a small fixed collection that's both faster and simpler.

Third, and most importantly for modern C++, **`constexpr`**. A `std::array` can be a compile-time constant — computed, indexed, and reasoned about entirely before your program ever runs. A `std::vector` cannot be a `constexpr` object. This is the feature that makes `std::array` indispensable rather than merely convenient.

> **Best practice:** Use `std::array` when the length is fixed and known at compile time. Use `std::vector` when the number of elements is determined at runtime or the collection may grow or shrink.

A small, fixed lookup table is the textbook case:

```cpp
#include <array>
#include <string_view>

constexpr std::array<std::string_view, 3> mutationKindNames {
    "replace",
    "delete",
    "insert"
};
```

That table is small, fixed, and known as you write it — three properties that all point at `std::array`.

### The length must be a constant expression

Because the length is part of the type, the compiler has to know it while compiling. So the length must be a **constant expression** — something with a value fixed at compile time.

```cpp
constexpr int maxKinds { 3 };
std::array<int, maxKinds> counts {};   // fine: maxKinds is a compile-time constant
```

A value that only exists at runtime won't do:

```cpp
int readCount();                 // reads a number while the program runs

int count { readCount() };
// std::array<int, count> values {};   // error: count is runtime data
```

If your length comes from a file, the keyboard, or any other runtime source, `std::array` is the wrong tool — that's `std::vector`'s job:

```cpp
std::vector<int> values(count);   // runtime length: use vector
```

### A length of zero is allowed

Here's a small surprise that's worth knowing before you meet its opposite in the C-style world: a `std::array` is permitted to have *zero* elements. `std::array<T, 0>` is a legal, well-formed type — it simply holds nothing.

```cpp
std::array<int, 0> empty {};
```

An empty array has no valid index at all, so there is no `empty[0]` — subscripting it is undefined behavior, exactly as it would be for any out-of-range index. Because "is this array empty?" is now a real question you might need to ask, `std::array` provides `.empty()`, which returns `true` precisely when the length is zero:

```cpp
std::array<int, 0> empty {};

if (empty.empty())
{
    // length is 0 — there is no element to read
}
```

> **Warning:** A zero-length `std::array` is legal but has no elements. Never subscript it — there is no `empty[0]`. Use `.empty()` to test for the zero-length case.

Keep this in your back pocket. When we reach C-style arrays in 17.7 you'll find they are *not* allowed to have length zero — one of several small ways `std::array` is the more regular, better-behaved tool.

### Initializing a std::array

`std::array` is an **aggregate** — a simple type with no hidden machinery — which means it uses *aggregate initialization*: you list the element values in braces, in order.

```cpp
std::array<int, 5> primes { 2, 3, 5, 7, 11 };
```

```text
index:   0  1  2  3   4
value:   2  3  5  7  11
```

If you supply fewer initializers than the array has room for, the leftovers are **value-initialized** — zero for numeric types:

```cpp
std::array<int, 5> values { 4, 8 };   // 4, 8, 0, 0, 0
```

If you supply *too many*, compilation fails — a nice safety net, since it catches a miscounted list at compile time.

There's one initialization mistake worth singling out, because it's silent. If you leave the braces off entirely, the elements are **default-initialized**, which for fundamental types like `int` means *left with garbage*:

```cpp
std::array<int, 4> bad;    // int elements hold indeterminate values
std::array<int, 4> good {}; // int elements are value-initialized to 0
```

> **Warning:** A `std::array` declared with no braces leaves fundamental-type elements uninitialized. Always use `{}` when you create an array without explicit values — `good`, not `bad`.

### const and constexpr arrays

You can make an array immutable two ways, and the difference matters:

```cpp
const std::array<int, 3> runtimeConst { 1, 2, 3 };
constexpr std::array<int, 3> compileTime { 1, 2, 3 };
```

`const` means *you can't modify it after initialization*. `constexpr` means all of that **plus** the array can participate in compile-time evaluation — you can index it in a `static_assert`, use its values to size other arrays, and so on. As mentioned, this is the headline reason `std::array` matters in modern C++.

> **Best practice:** If a `std::array`'s contents are known at compile time and never change, make it `constexpr`, not just `const`.

### Letting the compiler deduce the type (CTAD)

Spelling out `std::array<int, 3>` when the initializers already make the type obvious is a bit redundant. **Class template argument deduction** (CTAD) lets the compiler figure out both the element type and the length from the values you give it:

```cpp
constexpr std::array ids { 10, 20, 30 };   // deduced as std::array<int, 3>
```

CTAD is convenient when the values naturally have the type you want. When they don't — say you want `double` but wrote integer literals — name the type explicitly so no surprising deduction happens:

```cpp
std::array<double, 3> weights { 1, 2, 3 };   // doubles, not ints
```

There's also `std::to_array` (C++20) for the case where you want the compiler to count the elements but you want to pin the element type:

```cpp
constexpr auto smallValues { std::to_array<short>({ 1, 2, 3 }) };
```

Use it sparingly. Plain `std::array` initialization is simpler and avoids a temporary object that `std::to_array` may have to build.

### Reading an element

Indexing looks exactly like every other container you've used:

```cpp
constexpr std::array labels { 'a', 'b', 'c' };
char second { labels[1] };   // 'b'
```

One caution, and it's the same caution as `std::vector`'s `operator[]`: subscripting does **not** check whether your index is valid. Hand it a bad index and you get undefined behavior, not an error message. We'll look at safer indexing options in the next lesson.

> **Warning:** `operator[]` performs no bounds checking. An out-of-range index is undefined behavior.

---

## 17.2 — std::array length and indexing

### The length is an unsigned number

To understand the small annoyances of array lengths, it helps to see how `std::array` is declared in the standard library. Conceptually it's this:

```cpp
template <typename T, std::size_t N>
struct array;
```

The length `N` is a **non-type template parameter** of type `std::size_t` — an *unsigned* integer type. That single fact ripples outward: the functions that report an array's length, and the type used to index it, are unsigned too. You met the same signed/unsigned friction with `std::vector` last chapter; it's the same story here.

### Three ways to ask for the length

```cpp
constexpr std::array values { 4, 8, 15, 16 };

values.size();      // member function, returns an unsigned size_type
std::size(values);  // C++17 free function, also unsigned
std::ssize(values); // C++20 free function, returns a SIGNED integer
```

All three give you 4. The difference is the *type* of that 4. The first two are unsigned; `std::ssize` is signed, which is handy when you want a signed loop counter and would rather not fight the compiler over comparisons:

```cpp
for (int i { 0 }; i < std::ssize(values); ++i)
{
    std::cout << values[static_cast<std::size_t>(i)] << '\n';
}
```

Notice the `static_cast` at the moment of indexing. Your loop logic runs on a signed `int`, but the subscript wants an unsigned `std::size_t`. The cast is the visible seam between the two worlds — it marks exactly where signed counting becomes unsigned indexing.

### The length is available at compile time

Because the length is part of the type, it is itself a compile-time fact:

```cpp
std::array values { 1, 2, 3 };
constexpr int length { static_cast<int>(std::size(values)) };
```

There is one historical wrinkle worth knowing. Before C++23, getting a `constexpr` length out of a `std::array` *that was passed into a function by reference* was awkward, because the parameter isn't itself a constant expression. The clean workaround — and a useful pattern in its own right — is to make the length a template parameter, so the compiler hands you `N` directly:

```cpp
template <typename T, std::size_t N>
void printLength(const std::array<T, N>&)
{
    std::cout << N << '\n';   // N is a compile-time constant here
}
```

You'll see more of this template-the-length idiom in lesson 17.3.

### Three ways to index: [], at(), and std::get

C++ gives you three subscripting tools, and they trade safety against when the check happens:

| Tool | Bounds check | Index known when? | Notes |
|---|---|---|---|
| `arr[index]` | none | runtime or compile time | fastest, most common |
| `arr.at(index)` | at runtime | runtime or compile time | throws an exception on a bad index |
| `std::get<I>(arr)` | at compile time | compile time only | rejects an invalid index while compiling |

`std::get` is the interesting one. Because it takes the index as a *template* argument rather than a function argument, the index has to be a compile-time constant — and the compiler can verify it's in range before the program ever runs:

```cpp
constexpr std::array values { 10, 20, 30 };

static_assert(std::get<1>(values) == 20);   // checked at compile time
// std::get<5>(values);                      // would not compile
```

If you write `std::get<5>` on a three-element array, that's a compile error, not a crash. A bug caught while compiling is the cheapest kind of bug.

### Which one to use

Reach for `operator[]` by default — when you already know the index is valid (you just checked it, or it came from a loop bounded by `size()`). Reach for `std::get<I>` when the index is a literal compile-time constant and you'd rather a typo became a compile error than undefined behavior.

`at()` exists, and it's safe, but leaning on it as your primary defense tends to scatter exception handling through code that would read more clearly if it simply validated the index before indexing. Check first, then subscript.

> **Tip:** Prefer checking an index for validity *before* you use it, over relying on `at()` to throw afterward. The control flow is clearer, and you pay no per-access checking cost on indices you already know are good.

---

## 17.3 — Passing and returning std::array

### Passing by value copies the whole array

A `std::array` holds its elements *inside* itself. That has a consequence the moment you pass one to a function by value:

```cpp
void printCopy(std::array<int, 100> values);   // copies all 100 ints, every call
```

Every call duplicates the entire array. For a hundred ints that's wasteful; for a thousand large structs it's a disaster. So the default for read-only access is the same as it was for vectors and strings: **pass by const reference**.

```cpp
void printValues(const std::array<int, 100>& values)
{
    for (int value : values)
        std::cout << value << ' ';
}
```

The `const` says "I only read this," and the `&` says "and I'll read the caller's array directly, no copy." When the function genuinely needs to *change* the caller's array, drop the `const` and pass by non-const reference:

```cpp
void reset(std::array<int, 100>& values)
{
    for (int& value : values)   // note: int&, so we modify in place
        value = 0;
}
```

> **Best practice:** Pass `std::array` by `const` reference for read-only access, and by non-`const` reference only when the function is meant to modify the caller's array. Pass by value only when you specifically want a copy.

### The length is part of the parameter type

Here is where the welded-on length both helps and constrains you. This function:

```cpp
void printThree(const std::array<int, 3>& values);
```

accepts *exactly* `std::array<int, 3>`. Not `std::array<int, 4>`, not `std::array<double, 3>`. The type system won't let a four-element array sneak in where a three-element one was promised. That's safe — but it also means a function written for one fixed size is useless for any other.

The escape hatch is a **function template**. Make the element type and the length into template parameters, and the compiler will stamp out a version of the function for whatever array you actually pass:

```cpp
template <typename T, std::size_t N>
void printArray(const std::array<T, N>& values)
{
    for (const T& value : values)
        std::cout << value << ' ';
}
```

Now a single definition serves them all — `printArray<int, 3>`, `printArray<double, 3>`, `printArray<int, 8>` — each generated on demand from the argument you give it.

### Template only what should vary

You don't have to make *everything* a template parameter — only the parts that should be free to change. If you want any length but only `int` elements, template just the length:

```cpp
template <std::size_t N>
int sumInts(const std::array<int, N>& values)
{
    int total { 0 };

    for (int value : values)
        total += value;

    return total;
}
```

If both should vary, template both, and give the accumulator the element's own type:

```cpp
template <typename T, std::size_t N>
T sum(const std::array<T, N>& values)
{
    T total {};   // value-initialize: 0 for numbers, "" for strings, etc.

    for (const T& value : values)
        total += value;

    return total;
}
```

### Enforcing a minimum length at compile time

Because `N` is a compile-time value inside the template, you can *assert* things about it. Suppose a function only makes sense for arrays of at least three elements:

```cpp
template <typename T, std::size_t N>
T thirdElement(const std::array<T, N>& values)
{
    static_assert(N > 2, "thirdElement requires at least three elements");
    return values[2];
}
```

Call it with a two-element array and you get a clear compile error with your own message — not undefined behavior at runtime. This is the kind of guarantee C-style arrays simply cannot offer, because, as you'll see in 17.8, they don't carry their length far enough for anyone to check it.

### Returning a std::array

Returning a `std::array` by value copies the array object out to the caller. That's perfectly fine when the array is small, the elements are cheap to copy, and the function isn't in a tight inner loop:

```cpp
std::array<int, 3> makeRgb(int red, int green, int blue)
{
    return { red, green, blue };
}
```

For a large array or expensive elements where the copy actually shows up in a profile, an alternative is to have the caller supply the array and let the function fill it:

```cpp
void fillRgb(std::array<int, 3>& rgb, int red, int green, int blue)
{
    rgb[0] = red;
    rgb[1] = green;
    rgb[2] = blue;
}
```

But don't reach for that pattern reflexively — it's a clumsier interface, and it sacrifices the clarity of a real return value. Return by value until measurement or an ownership requirement tells you otherwise.

> **Best practice:** Prefer returning a `std::array` by value for clear, simple interfaces. Only switch to a caller-supplied output parameter when profiling shows the copy is a real cost.

---

## 17.4 — std::array of class types, and brace elision

### Arrays can hold anything

Nothing about `std::array` restricts you to `int`s and `char`s. The element type can be anything — a fundamental type, a pointer, a struct, a class, even another array:

```cpp
#include <array>
#include <string_view>

struct CoverageCounter
{
    std::string_view name {};
    int hits {};
};

std::array<CoverageCounter, 2> counters {};
```

Each element is a full `CoverageCounter`, and you assign to it the way you'd assign any struct:

```cpp
counters[0] = { "entry", 1 };
counters[1] = { "exit", 0 };
```

### The double-brace puzzle

Initializing an array of structs *with their values inline* is where a famous bit of C++ syntax surprises people. Try to give the values directly and you may find you need an extra layer of braces:

```cpp
constexpr std::array<CoverageCounter, 2> counters {{
    { "entry", 1 },
    { "exit", 0 },
}};
```

Two opening braces. Why? The answer is in how `std::array` is built. Conceptually it's an aggregate wrapped around a single C-style array member:

```cpp
template <typename T, std::size_t N>
struct array
{
    T storage[N];   // one plain array, hidden inside
};
```

So there are genuinely *two* levels of structure to initialize — the `std::array` wrapper, and the `storage` array inside it:

```text
std::array object       <- outer brace
    storage array        <- inner brace
        element 0
        element 1
```

The double braces make both levels explicit. The outer pair initializes the `std::array`; the inner pair initializes its `storage` member; and each `{ "entry", 1 }` initializes one `CoverageCounter`.

### Brace elision, and why scalars look simpler

If `std::array` really has two levels, why did `std::array<int, 5> primes { 2, 3, 5, 7, 11 }` back in 17.1 work with a *single* brace? Because of a rule called **brace elision**: in aggregate initialization, the compiler will let you omit inner braces when it can unambiguously figure out where things go. For an array of plain scalars, one brace layer is enough and the compiler fills in the rest.

For an array of *class-type* elements, you have two clean choices that always work. Either name each element's type, which lets CTAD and brace elision cooperate:

```cpp
constexpr std::array items {
    CoverageCounter { "entry", 1 },
    CoverageCounter { "exit", 0 },
};
```

or specify the array type and use the explicit double braces:

```cpp
std::array<CoverageCounter, 2> items {{
    { "entry", 1 },
    { "exit", 0 },
}};
```

> **Tip:** When you're unsure whether brace elision applies, the double-brace form for `std::array` is always correct and makes the array's two-level shape explicit. When in doubt, double up.

This double-brace form is exactly what the chapter lab uses for its tic-tac-toe board (`Board {{ {row}, {row}, {row} }}`), so it's worth getting comfortable with now.

---

## 17.5 — Arrays of references via std::reference_wrapper

### You cannot make an array of references

Suppose you have a few existing variables and you'd like an array that *refers* to them, so that changing an array element changes the original. The natural first attempt fails:

```cpp
int a { 1 };
int b { 2 };

// std::array<int&, 2> refs { a, b };   // error: cannot have an array of references
```

The reason is fundamental: a reference is not an object. It has no storage of its own and cannot be reseated to refer to something else after it's bound. Arrays, on the other hand, are made of objects you can assign to and copy. References and arrays are oil and water.

If you try to sidestep the problem with CTAD, you don't get references — you get *copies*, because deducing from a reference deduces the underlying value type:

```cpp
int& ra { a };
int& rb { b };

std::array values { ra, rb };   // deduced as std::array<int, 2> — copies of a and b
```

### std::reference_wrapper: a reference you can put in a box

The standard library's answer is `std::reference_wrapper<T>`, from `<functional>`. It's a small *object* — so it can live in an array — that behaves like a reference you're allowed to reseat:

```cpp
#include <array>
#include <functional>

int hot { 10 };
int cold { 2 };

std::array<std::reference_wrapper<int>, 2> refs { hot, cold };
```

To reach through the wrapper to the real object, call `.get()`, which hands you a genuine `T&`:

```cpp
refs[0].get() = 11;   // modifies hot, through the wrapper
```

### Assigning reseats the wrapper

Here's the one behavior that catches people, because it differs from a real reference. Assigning *to the wrapper itself* doesn't change the referenced value — it **reseats** the wrapper to point somewhere new:

```cpp
int x { 1 };
int y { 2 };
int z { 3 };

std::reference_wrapper<int> ref { x };

ref.get() = 10;   // modifies x (reach through, then assign)
ref = z;          // reseats: ref now refers to z, x is untouched
```

`ref.get() = 10` writes *through* the wrapper to `x`. Plain `ref = z` rebinds the wrapper to `z`. A true C++ reference can't be reseated at all, which is exactly why a wrapper is needed for a container of references.

### std::ref and std::cref

Spelling out `std::reference_wrapper<int>` is a mouthful, so the library provides two helper functions that build one for you and pair nicely with `auto`:

```cpp
auto ref { std::ref(hot) };    // std::reference_wrapper<int>
auto cref { std::cref(cold) }; // std::reference_wrapper<const int>
```

`std::ref` gives a mutable wrapper; `std::cref` a read-only one.

> **Tip:** Reach for `std::reference_wrapper` when you specifically need a container of references. If your situation involves "this might point at nothing," or you're talking to a pointer-shaped API, a container of pointers tells that story more directly.

---

## 17.6 — std::array and enumerations

### Enumerators make excellent indices

Back in Chapter 13 you learned that an unscoped enumerator quietly converts to an integer. That conversion turns out to be a small superpower when paired with a fixed array: you can use *names* as indices instead of bare numbers.

```cpp
namespace Counter
{
    enum Kind
    {
        entry,
        branch,
        exit,
        maxKinds   // sits last; its value equals the count of real kinds
    };
}

std::array<int, Counter::maxKinds> hits {};
hits[Counter::branch] = 3;
```

`hits[Counter::branch]` reads like prose. Compare it to `hits[1]`, where a future reader has to remember that `1` means *branch* — and where renumbering the enum silently breaks every hard-coded index. The trailing `maxKinds` enumerator is a classic trick: because enumerators count up from zero, the last one automatically equals the number of real kinds, giving you a ready-made array size.

### Tie the table's length to the enum

When you let CTAD deduce a table's length, a forgotten initializer can silently produce a *shorter* table than you intended — and then an enum index walks off the end. A single `static_assert` slams that door:

```cpp
namespace Counter
{
    enum Kind { entry, branch, exit, maxKinds };

    using namespace std::string_view_literals;

    constexpr std::array names { "entry"sv, "branch"sv, "exit"sv };
    static_assert(std::size(names) == maxKinds);   // names and Kind must stay in sync
}
```

Now if someone inserts a `function` kind before `maxKinds` but forgets to add its name to the table, the assertion fails at compile time and tells them exactly what's wrong. The enum and its table are kept honest by the compiler instead of by vigilance.

> **Best practice:** Whenever an array is indexed by an enum, guard it with `static_assert(std::size(table) == enumCount)` so the table can never silently fall out of step with the enum.

### Mapping an enum to a string

This pattern replaces the repetitive `switch` you might otherwise write to turn an enum into text. The table *is* the mapping:

```cpp
constexpr std::string_view nameOf(Counter::Kind kind)
{
    return Counter::names[static_cast<std::size_t>(kind)];
}
```

The `static_cast<std::size_t>` at the subscript is the by-now-familiar signed/unsigned seam. An unscoped enumerator would convert on its own, but the explicit cast makes the boundary between "enum value" and "array index" visible and deliberate.

### Mapping a string back to an enum

The reverse direction is a small linear search over the same table, returning a `std::optional` (Chapter 12) so "no match" is expressible without a sentinel:

```cpp
#include <optional>

std::optional<Counter::Kind> kindFromName(std::string_view text)
{
    for (std::size_t i { 0 }; i < Counter::names.size(); ++i)
    {
        if (Counter::names[i] == text)
            return static_cast<Counter::Kind>(i);
    }

    return std::nullopt;
}
```

Both directions read from the one table, so there's exactly one place to edit when the set of kinds changes. That single-source-of-truth property is the whole point.

### Iterating over an enum's values

There's one thing an enum can't do on its own: you can't write a range-`for` loop directly over "all the enumerators." An enum isn't a range. But a small `std::array` *of* enumerators is, so make one — and guard it the same way:

```cpp
namespace Counter
{
    constexpr std::array kinds { entry, branch, exit };
    static_assert(std::size(kinds) == maxKinds);
}

for (Counter::Kind kind : Counter::kinds)
{
    std::cout << nameOf(kind) << '\n';
}
```

This is clean when the enumerators have unique, contiguous values and the `static_assert` keeps the list complete.


---

## 17.7 — Introduction to C-style arrays

### The original array

Everything so far has been `std::array`, the modern wrapper. Now we go underneath it to the **C-style array** — the built-in array type C++ inherited from C. It's the raw material; `std::array` is the polished tool built on top.

```cpp
int scores[5] {};
```

That declares five `int`s, side by side in memory, all zero-initialized:

```text
scores[0] scores[1] scores[2] scores[3] scores[4]
```

The syntax is different from `std::array`: the element type comes first, the name next, and the length sits in square brackets after the name. No `<array>` header is needed, because C-style arrays are part of the core language itself, not the library.

### Length rules

For an ordinary C-style array (one on the stack, or a global), the length follows a few hard rules:

- it must be a **constant expression** — known at compile time;
- it must be at least 1 (no zero-length C-style arrays);
- the element type must be stated explicitly;
- it cannot be a runtime value. *Variable-length arrays* are not standard C++.

```cpp
constexpr int maxScores { 5 };
int scores[maxScores] {};        // fine: maxScores is a constant expression
```

Some compilers offer an *extension* that lets you write a runtime length, and it will appear to work — but it isn't portable standard C++, so don't rely on it:

```cpp
int n {};
std::cin >> n;
// int values[n];   // NOT standard C++ — use std::vector for a runtime length
```

> **Warning:** Variable-length arrays (a C-style array sized by a runtime value) are not part of standard C++, even if your compiler accepts them as an extension. For a runtime length, use `std::vector`.

### Indexing

Subscripting looks the same as everywhere else:

```cpp
int values[3] { 10, 20, 30 };

std::cout << values[1] << '\n';   // 20
```

There's one small difference from the library containers: a C-style array will accept a signed *or* unsigned integral index, and even an unscoped enumerator, without complaint about signedness. What it shares with them is the dangerous part — **no bounds checking**:

```cpp
values[3];   // out of bounds: undefined behavior
```

### Initialization

The initialization forms mirror `std::array`'s, with one nice bonus — the compiler can count the elements for you:

```cpp
int a[4] {};             // 0, 0, 0, 0   (all value-initialized)
int b[4] { 1, 2 };       // 1, 2, 0, 0   (rest value-initialized)
int c[] { 1, 2, 3, 4 };  // length deduced as 4
```

That last form is worth a habit. When you're listing every element explicitly, leave the brackets empty and let the compiler count:

```cpp
constexpr int opcodes[] { 10, 20, 30 };
```

Now adding or removing an element updates the length automatically — there's no separate `3` to forget to bump.

> **Best practice:** When you initialize a C-style array with an explicit list of all its elements, omit the length (`int c[] { … }`) and let the compiler deduce it. One fewer number to keep in sync.

### No CTAD, no auto

A C-style array is *not* a class template, so the deduction conveniences from earlier don't apply here:

```cpp
// auto values[3] { 1, 2, 3 };   // not allowed
```

If you want the element type deduced from the initializers, that's a reason to choose `std::array`, which does support CTAD.

### const and constexpr C-style arrays

C-style arrays can be `const` or `constexpr` just like anything else:

```cpp
constexpr int lookup[] { 1, 4, 9, 16 };
```

A global `constexpr` C-style table is one of the few places C-style arrays still earn their keep in modern code — the syntax is compact, and signed indexing comes for free if that's what you want.

### Getting the length

In C++17 and later, ask the same free functions you'd use for `std::array`:

```cpp
#include <iterator>

constexpr int lookup[] { 1, 4, 9, 16 };

std::size(lookup);   // 4, unsigned
std::ssize(lookup);  // C++20, signed
```

You may see older code compute the length with the `sizeof(array) / sizeof(array[0])` trick — total bytes divided by per-element bytes. Avoid it. Not only is it noisier, it breaks silently the moment the array decays to a pointer, which is the topic of the very next lesson. `std::size` and `std::ssize` simply refuse to compile on a decayed pointer, turning that subtle bug into an obvious error.

> **Best practice:** Use `std::size()` / `std::ssize()` to get a C-style array's length, not the old `sizeof(array) / sizeof(array[0])` formula.

### You cannot assign a whole array

One last quirk that trips up newcomers: you can assign to individual *elements*, but you cannot assign one array to another:

```cpp
int values[] { 1, 2, 3 };

values[0] = 9;        // fine: element assignment
// values = { 4, 5 }; // error: whole-array assignment is not allowed
```

That limitation — together with decay, coming next — is a big part of why modern code reaches for `std::array` or `std::vector` instead.

---

## 17.8 — C-style array decay

### The defining behavior of C-style arrays

This lesson is the heart of the whole "C-style" half of the chapter. Get it, and the rest of the rough edges make sense.

In most expressions, a C-style array doesn't behave like an array at all — it quietly converts into a **pointer to its first element**. This conversion is called **array decay**, and it happens automatically, almost everywhere, usually without you noticing.

```cpp
int values[] { 10, 20, 30 };

int* ptr { values };   // values decays to &values[0]
```

```text
values object: [10][20][30]
                ^
                |
ptr ------------+
```

The array object's type is `int[3]`. The decayed pointer's type is plain `int*`. Same first address — but, crucially, *not* the same information.

### Decay throws away the length

Here is the cost, stated plainly. An array's type carries two facts: the element type *and* the length. A pointer's type carries only the element type. So the instant an array decays, the length is gone:

```text
array type:    int[3]   ->  element type + length
pointer type:  int*     ->  element type only
```

```cpp
int values[] { 10, 20, 30 };
int* ptr { values };
```

`values` knows it has three elements — that's baked into `int[3]`. `ptr` knows only an address; ask it how long the array is and it cannot answer, because that knowledge was never copied into it. This single loss is the root of nearly every C-style-array hazard you'll meet.

### Where arrays do *not* decay

Decay is the default, but a handful of contexts preserve the full array type. The ones worth remembering:

- as the operand of `sizeof` (so `sizeof(values)` gives the array's byte size, not a pointer's);
- as the operand of `typeid`;
- when you take the address of the whole array with `&`;
- when the array is passed **by reference**;
- when it's stored as a member of a class or struct.

Outside of those, assume decay.

### Function parameters silently become pointers

Now the practical sting. Look at this function:

```cpp
void printFirst(const int values[])
{
    std::cout << values[0] << '\n';
}
```

The `const int values[]` parameter *looks* like it takes an array. It does not. The compiler rewrites that parameter to:

```cpp
void printFirst(const int* values);   // exactly the same function
```

The `[]` in a parameter list is pure decoration — a hint to the human reader that "this pointer is expected to point at the first element of an array." It carries no length. And if you write a number inside those brackets, the compiler ignores it completely:

```cpp
void printFirst(const int values[100]);   // STILL just const int* — the 100 is a lie
```

> **Warning:** A C-style array parameter is always really a pointer. `const int values[]` and `const int values[100]` both mean `const int*`. Any length you write in the brackets is ignored.

### The upside, and then the three problems

There *is* a silver lining: because every array decays to the same pointer type, one function can accept arrays of any length.

```cpp
void printFirst(const int values[])
{
    std::cout << values[0] << '\n';
}

int a[] { 1, 2, 3 };
int b[] { 9, 8, 7, 6, 5 };

printFirst(a);   // both decay to const int*
printFirst(b);
```

But you pay for that flexibility with the loss of length, and it shows up as three concrete problems.

**Problem 1 — length functions stop working.** Inside the function, the parameter is a pointer, so `std::size` won't even compile:

```cpp
void printLength(const int values[])
{
    // std::size(values);   // error: values is a pointer here, not an array
}
```

**Problem 2 — bad lengths are invisible.** Nothing in a pointer says how many elements follow it, so an undersized array sails right through and reads out of bounds:

```cpp
void printThird(const int values[])
{
    std::cout << values[2] << '\n';   // assumes at least 3 elements
}

int shortArray[] { 4, 5 };
printThird(shortArray);   // compiles cleanly, then undefined behavior at runtime
```

Contrast this with the `std::array` template version in 17.3, where `static_assert(N > 2)` caught exactly this mistake at compile time. The C-style array can't, because `N` never made it across the boundary.

**Problem 3 — refactoring can change meaning.** Code that worked on a real array can break the moment you lift it into a function, because what was an array in the original spot is a decayed pointer in the new one. `sizeof` and `std::size` quietly start reporting something different.

### Living with decay

When you must use C-style arrays, there are two classic ways to recover the length the pointer forgot.

Pass the length as a second parameter — the approach the chapter lab's `sumScores` uses:

```cpp
void printAll(const int values[], int length)
{
    for (int i { 0 }; i < length; ++i)
        std::cout << values[i] << ' ';
}
```

It works, but it's fragile: caller and callee have only a *promise* that `length` matches the real array. Pass the wrong number and you're back to undefined behavior.

Or use a **sentinel** — a special terminating value that marks the end:

```cpp
constexpr int stop { -1 };
int values[] { 4, 8, 15, stop };
```

This only works when the sentinel can never appear as real data. It's exactly the trick C-style strings use, with `'\0'` marking the end — which is where we're headed in 17.10.

### When to just not

For most code, the cleanest fix is to avoid C-style arrays altogether. Here's the cheat sheet:

| Need | Prefer |
|---|---|
| Read-only string parameter | `std::string_view` |
| Modifiable text | `std::string` |
| Fixed compile-time local array | `std::array` |
| Runtime-sized array | `std::vector` |
| Legacy/C API, or a global `constexpr` table | a C-style array may be acceptable |

> **Key insight:** The one fact a C-style array loses on decay — its length — is the one fact you most need to use it safely. That's the whole case for `std::array`, which carries its length in its type all the way across function boundaries.

---

## 17.9 — Pointer arithmetic and subscripting

### Adding to a pointer moves by elements

Once an array has decayed to a pointer, how do you reach the *other* elements? Through **pointer arithmetic** — and the key surprise is that it counts in elements, not bytes.

```cpp
int values[] { 10, 20, 30 };
int* ptr { values };

int* next { ptr + 1 };   // points at values[1], not "one byte later"
```

When you write `ptr + 1`, the compiler scales the `1` by the size of the pointed-to type. If `int` is 4 bytes, `ptr + 1` is 4 bytes further along; if `ptr` were a `double*`, the same `+ 1` would step by `sizeof(double)`. You think in elements; the compiler handles the byte math. That's why pointer arithmetic "just lands" on the next element every time.

### The legal range

For an array, you may form pointers to each element *and* to one position past the last element:

```cpp
int values[] { 10, 20, 30 };
```

```text
&values[0]   &values[1]   &values[2]   one-past-the-end
```

That **one-past-the-end** pointer is special: it's legal to compute and to compare against, but you must never *dereference* it — there's no element there. It exists precisely so loops have a clean place to stop:

```cpp
int* begin { values };
int* end { values + std::size(values) };   // one past the last element

for (int* current { begin }; current != end; ++current)
{
    std::cout << *current << '\n';
}
```

The loop walks from `begin` and halts the moment `current` reaches `end`, dereferencing every element along the way but never `end` itself.

### Subscripting is arithmetic in disguise

Here's a fact that explains a great deal at once. For any pointer `ptr`, the subscript expression

```cpp
ptr[n]
```

is defined to mean exactly

```cpp
*(ptr + n)
```

— "step `n` elements forward, then dereference." Subscripting was never a separate feature; it's shorthand for pointer arithmetic plus a dereference.

```cpp
int values[] { 10, 20, 30 };
int* ptr { values };

std::cout << ptr[2] << '\n';   // *(ptr + 2), prints 30
```

This also finally explains *why arrays start at index 0*. The first element is at offset zero from the start:

```cpp
values[0] == *(values + 0) == *values
```

No offset, no surprise — index 0 is simply "the element right here, no stepping required." (This is the very fact that makes `scores[i]` work inside the lab's `sumScores`, even though `scores` is a decayed pointer there, not an array.)

### Indices can be relative, and even negative

Because `ptr[n]` is just `*(ptr + n)`, the index is relative to wherever the pointer currently points — which means it can be negative:

```cpp
int values[] { 10, 20, 30, 40 };
int* middle { &values[2] };

middle[0];   // values[2] -> 30
middle[1];   // values[3] -> 40
middle[-1];  // values[1] -> 20
```

This is valid only while the computed address stays within the array (or one past the end where that's allowed). Use plain subscripting when you're indexing from the start of an array; reach for explicit pointer arithmetic when relative positioning is genuinely the point.

### Begin/end: the shape of every algorithm to come

The `begin`/`end` pair from the loop above is worth dwelling on, because it's the single most important pattern in this lesson. Together they describe a **half-open range**, written `[begin, end)`: `begin` points at the first element, `end` points *one past* the last, and the range includes `begin` but excludes `end`.

```text
[begin, end)
 ^           ^
 first       one-past-last (not included)
```

A function can take a range exactly this way:

```cpp
void printRange(const int* begin, const int* end)
{
    for (const int* current { begin }; current != end; ++current)
        std::cout << *current << ' ';
}
```

and you call it by handing over the two endpoints:

```cpp
int values[] { 1, 2, 3 };
printRange(values, values + std::size(values));
```

Two properties make half-open ranges so pleasant. An *empty* range is just `begin == end` — no special case. And the number of elements is exactly `end - begin`. This `[begin, end)` convention is the direct conceptual ancestor of **iterators**, and every standard-library algorithm you'll meet in the next chapter is built on it. The lab's `firstNegative` function — walk `[begin, end)`, return the position of the first match or `end` if there's none — is this exact pattern, and "return `end` to mean *not found*" is the standard idiom you'll see again and again.

> **Key insight:** A half-open range `[begin, end)` — first element to one-past-the-last — is the foundation of C++ iteration. Empty means `begin == end`; the count is `end - begin`; "not found" is conventionally signaled by returning `end`. Chapter 18's iterators and algorithms are this idea, generalized.

### Why range-`for` works

You've been using range-`for` since Chapter 16 without seeing its insides. Now you can: a range-`for` over an array is essentially the begin/end walk, written for you.

```cpp
for (int value : values)
{
    std::cout << value << '\n';
}
```

is, conceptually, this:

```cpp
auto begin { values };
auto end { values + std::size(values) };

for (; begin != end; ++begin)
{
    int value { *begin };
    std::cout << value << '\n';
}
```

Same traversal, same half-open range — just with the bookkeeping hidden behind nicer syntax.


---

## 17.10 — C-style strings

### A string made of characters and one terminator

Long before `std::string`, C represented text as a plain array of characters with a single convention bolted on: the text ends at the first **null terminator**, the character `'\0'` (a byte whose value is zero). A **C-style string** is exactly that — a C-style array of `char` (or `const char`) ending in `'\0'`.

```cpp
char text[] { "log" };
```

In memory:

```text
'l' 'o' 'g' '\0'
```

That trailing `'\0'` is not decoration — it's load-bearing. Remember that arrays decay to pointers and lose their length; the null terminator is how a function recovers where the string ends after only a pointer survives. The terminator *is* the length information, encoded inside the data.

### String literals carry the terminator for you

When you initialize a char array from a string literal, the compiler appends the `'\0'` automatically — so the array is one element longer than the visible characters:

```cpp
const char text[] { "hello" };
```

```text
h  e  l  l  o  \0
0  1  2  3  4  5     <- six elements, not five
```

This is why you should let the compiler size these arrays. Write the literal and leave the brackets empty, and it reserves exactly the right room, terminator included:

```cpp
char buffer[] { "abc" };   // length 4: 'a' 'b' 'c' '\0'
```

### C-style strings decay too, and cout cooperates

A C-style string is a C-style array, so of course it decays to a pointer when passed along:

```cpp
void print(const char text[])   // really const char*
{
    std::cout << text << '\n';
}

char message[] { "ok" };
print(message);   // message decays to a char pointer
```

`std::cout` treats `char*` and `const char*` *specially*: instead of printing the address, it prints characters starting at the pointer and keeps going until it hits a `'\0'`. That's a convenience — but it's also a trap. If the terminator is missing, `std::cout` marches straight off the end of your data into whatever memory lies beyond, which is undefined behavior.

> **Warning:** Printing a `char*` makes `std::cout` read characters until it finds a `'\0'`. If there is no terminator, it reads past your data — undefined behavior. Every C-style string must end in `'\0'`.

### Input and buffer overflow

C-style strings are fixed-size arrays, and that fixedness is dangerous when text comes from outside the program. If more characters arrive than the buffer can hold, the extra characters spill past the end of the array — the classic **buffer overflow**, the source of countless security holes over the decades:

```cpp
char name[8] {};
std::cin >> name;   // historically dangerous: long input overruns the buffer
```

The safer C-style approach passes the buffer size, so input stops before it overflows:

```cpp
char name[8] {};
std::cin.getline(name, std::size(name));
```

Even that demands careful fixed-buffer discipline. For anything the user types, the right answer is to stop fighting fixed buffers and use `std::string`, which grows to fit whatever it's given:

```cpp
std::string name {};
std::getline(std::cin, name);
```

> **Best practice:** Use `std::string` (with `std::getline`) for user-entered text. C-style character buffers are easy to overflow and should be reserved for C-API interop.

### Modifying a C-style string

You already know a whole array can't be assigned at once, and C-style strings are no exception:

```cpp
char text[] { "cat" };
// text = "dog";   // error: cannot assign to a whole array
```

You *can* change individual characters, as long as the array isn't `const`:

```cpp
text[0] = 'b';   // now "bat"
```

Just respect the terminator. Overwrite the `'\0'` and you've destroyed the only marker of where the string ends — every function that scans for it will run past the end.

### Array length versus string length

These are two different numbers, and conflating them causes bugs:

```cpp
char text[16] { "hello" };
```

The **array length** is how many `char` slots exist:

```cpp
std::size(text);     // 16 — total capacity
```

The **string length** is how many characters precede the `'\0'`:

```cpp
#include <cstring>
std::strlen(text);   // 5 — actual text length
```

`std::strlen`, from `<cstring>`, counts characters up to the terminator. It works on a decayed pointer (it only needs the start and the `'\0'`), but note it's an O(n) walk every time you call it — it has to scan the whole string to count. `std::string`, by contrast, simply *knows* its length in constant time.

### The bottom line on C-style strings

Non-`const` C-style string *objects* are clumsy and overflow-prone. In modern code:

- use `std::string` for owned, modifiable text;
- use `std::string_view` for read-only, non-owning text;
- reach for C-style strings only when an API or older code requires a null-terminated `const char*`.

---

## 17.11 — C-style string symbolic constants

### Two ways to name a string constant

When the string is a *constant* — a fixed label that never changes — C gives you two spellings, and they're not quite the same under the hood.

The **array form** makes an array, initialized from the literal:

```cpp
const char toolName[] { "mutator" };
```

The **pointer form** makes a pointer that points at the literal, which the compiler typically places in read-only storage:

```cpp
const char* const phaseName { "instrument" };
```

(That `const … const` reads as "a constant pointer to constant characters" — neither the pointer nor what it points at can change.) The array form gives you a private, modifiable-if-not-const copy of the characters; the pointer form just aims at the shared literal. For a read-only constant, either works.

### How auto deduces from a string literal

String literals interact with `auto` in a way that surprises people, and it's worth seeing once so it doesn't catch you later:

```cpp
auto a { "abc" };   // const char*        (the literal decays to a pointer)
auto* b { "abc" };  // const char*        (same thing, spelled with *)
auto& c { "abc" };  // const char (&)[4]  (a reference: no decay, full array type)
```

The first two decay to a pointer-to-`const char`. Only `auto&`, which binds a *reference* and so prevents decay, preserves the full array type — length and all (`[4]`: three letters plus the terminator). Most everyday uses decay to the pointer.

### cout and character pointers: a sharp edge

You saw that `std::cout` prints a `char*` as a string. The flip side is that it prints a pointer to *any other* type as a plain address. That asymmetry produces two classic gotchas.

The first: taking the address of a single `char` gives a `char*`, so `cout` tries to read a string that isn't there:

```cpp
char ch { 'x' };
std::cout << &ch;   // char* — cout reads characters until a '\0' it may never find
```

`&ch` points at one character with no terminator after it, so printing it as a string is undefined behavior.

The second: when you genuinely *want* the address of a character pointer, you must hide its char-ness from `cout` by casting to `const void*`:

```cpp
const char* text { "abc" };
std::cout << static_cast<const void*>(text) << '\n';   // prints the address, not "abc"
```

> **Warning:** `std::cout` prints a `char*`/`const char*` as a null-terminated string, not as an address. To print the address of character data, cast to `const void*` first.

### Prefer std::string_view for string constants

In modern C++, the right tool for a string constant is usually neither C-style form — it's `std::string_view`:

```cpp
constexpr std::string_view phaseName { "instrument" };
```

It's better on every axis that matters here:

- it stores the length explicitly, so it never has to scan for a terminator;
- it works directly with the many functions that take a `std::string_view`;
- it carries no mutable buffer to accidentally overflow;
- it's happily `constexpr`.

Keep the C-style string constant in your pocket for the one case it's actually needed: an API that specifically wants a null-terminated `const char*`.

> **Best practice:** Prefer `constexpr std::string_view` for string constants. Use a C-style string constant only when an interface explicitly requires a null-terminated `const char*`.

---

## 17.12 — Multidimensional C-style Arrays

### What "dimension" means

So far every array has been a single row of elements — *one-dimensional*. An array's **dimension** is just the number of indices you need to pin down one element. One index, one dimension:

```cpp
int line[4] {};
line[2] = 7;
```

Give an array element type that is *itself* an array, and you get a second dimension — a grid:

```cpp
int grid[3][4] {};
grid[1][2] = 7;
```

Read `int grid[3][4]` as "a 3-element array, where each element is a 4-element array of `int`." Two indices select one cell: the first picks which inner array, the second picks within it.

### Rows and columns

The conventional reading of a 2D array is *left index is the row, right index is the column*:

```cpp
int grid[3][4] {};
```

```text
          col 0   col 1   col 2   col 3
row 0   [0][0]  [0][1]  [0][2]  [0][3]
row 1   [1][0]  [1][1]  [1][2]  [1][3]
row 2   [2][0]  [2][1]  [2][2]  [2][3]
```

### Row-major order

The grid looks two-dimensional, but memory is one long line of bytes, so the elements have to be laid out in *some* linear order. C++ uses **row-major order**: it stores the whole first row, then the whole second row, and so on.

For `grid[3][4]`, memory runs:

```text
[0][0] [0][1] [0][2] [0][3] [1][0] [1][1] ... [2][3]
```

This is why looping *row on the outside, column on the inside* is the cache-friendly order — it walks memory in the exact sequence it's stored:

```cpp
for (std::size_t row { 0 }; row < std::size(grid); ++row)
{
    for (std::size_t col { 0 }; col < std::size(grid[0]); ++col)
        std::cout << grid[row][col] << ' ';

    std::cout << '\n';
}
```

### Initialization

Nested braces mirror the grid's shape and make the layout legible — one inner brace per row:

```cpp
int grid[2][3] {
    { 1, 2, 3 },
    { 4, 5, 6 },
};
```

As with one-dimensional arrays, missing inner values are value-initialized:

```cpp
int grid[2][3] {
    { 1 },
    { 4, 5 },
};
```

```text
1 0 0
4 5 0
```

And the compiler can count the *leftmost* dimension from the initializer, so you can leave it blank:

```cpp
int grid[][3] {
    { 1, 2, 3 },
    { 4, 5, 6 },
};
```

But *only* the leftmost. Every other dimension must be spelled out, because the compiler needs them to compute where each element sits — without the column count, it can't tell where one row ends and the next begins.

> **Tip:** When initializing a multidimensional C-style array, you may omit the leftmost dimension (the compiler counts the rows) but never the others — they're needed to compute element layout.

### Iterating with nested range-`for`

Range-`for` nests cleanly over a 2D array, and there's one detail that matters for performance:

```cpp
for (const auto& row : grid)
{
    for (int value : row)
        std::cout << value << ' ';

    std::cout << '\n';
}
```

Bind the outer variable as `const auto& row`. Each `row` is itself an array; without the reference, the loop would *copy* an entire row on every iteration for no reason. The reference looks at the real row in place. (This is the same `const auto&` rule the lab insists on for its nested-`std::array` board.)

### Coordinates versus indices

A last subtlety that causes real bugs. Geometry usually names points `(x, y)` with `x` horizontal and `y` vertical. But array indexing is `[row][col]`, and a row runs *vertically* down the grid while a column runs *horizontally* across it:

```text
row -> vertical position    (often the y coordinate)
col -> horizontal position  (often the x coordinate)
```

So if your program thinks in `(x, y)`, the correct subscript is usually `grid[y][x]`, not `grid[x][y]`:

```cpp
grid[y][x] = value;   // y selects the row, x selects the column
```

That's frequently right for screen and board data — but the conversion should be deliberate and obvious in the code, never an accident.

---

## 17.13 — Multidimensional std::array

### std::array is one-dimensional, so you nest it

There is no dedicated multidimensional `std::array`. To get a grid, you do the same thing C-style arrays do under the hood — make an array *of* arrays:

```cpp
std::array<std::array<int, 4>, 3> grid {{
    { 1, 2, 3, 4 },
    { 5, 6, 7, 8 },
    { 9, 10, 11, 12 },
}};
```

Read the type from the inside out:

```text
std::array<
    std::array<int, 4>,   // each row is 4 ints
    3                     // there are 3 such rows
>
```

Indexing is the familiar two-subscript form, and the double braces are the brace-elision shape from 17.4 — outer braces for the wrapper, inner for the rows:

```cpp
grid[1][2];   // row 1, column 2
```

This nested-`std::array` type is exactly the lab's board: `std::array<std::array<char, 3>, 3>`, a 3×3 grid of characters.

### Writing functions over a 2D std::array

A function template can accept a 2D `std::array` of any dimensions, but the parameter type gets wordy, since both dimensions are part of the type:

```cpp
template <typename T, std::size_t Rows, std::size_t Cols>
void printGrid(const std::array<std::array<T, Cols>, Rows>& grid)
{
    for (const auto& row : grid)
    {
        for (const T& value : row)
            std::cout << value << ' ';

        std::cout << '\n';
    }
}
```

Note, again, the `const auto& row` and the `const std::array&` parameter — both the by-now-reflexive way to pass and traverse fixed-size arrays without needless copies.

### Tame the noise with an alias template

That parameter type is a lot to read and re-read. An **alias template** (Chapter 13) gives the shape a friendly name:

```cpp
template <typename T, std::size_t Rows, std::size_t Cols>
using Array2d = std::array<std::array<T, Cols>, Rows>;
```

Now both declarations and signatures read naturally:

```cpp
Array2d<int, 3, 4> grid {{
    { 1, 2, 3, 4 },
    { 5, 6, 7, 8 },
    { 9, 10, 11, 12 },
}};

template <typename T, std::size_t Rows, std::size_t Cols>
void printGrid(const Array2d<T, Rows, Cols>& grid);
```

### Getting the dimensions safely

You *can* ask the array for its dimensions directly:

```cpp
grid.size();      // number of rows
grid[0].size();   // number of columns — but only valid if row 0 exists
```

That `grid[0]` is a small landmine: on a zero-row grid there is no row 0. The robust approach pulls the dimensions straight out of the template parameters, which exist whether or not any rows do:

```cpp
template <typename T, std::size_t Rows, std::size_t Cols>
constexpr std::size_t rowCount(const Array2d<T, Rows, Cols>&)
{
    return Rows;
}

template <typename T, std::size_t Rows, std::size_t Cols>
constexpr std::size_t colCount(const Array2d<T, Rows, Cols>&)
{
    return Cols;
}
```

These never touch `grid[0]`, so they can't trip over an empty grid — and they're `constexpr` besides.

### Flattening: one array, manual coordinates

Nested arrays grow unwieldy as dimensions pile up, and some APIs insist on a single contiguous block of memory. The alternative is to **flatten**: store the grid as one long 1D array and compute the offset into it yourself.

```cpp
template <typename T, std::size_t Rows, std::size_t Cols>
using FlatGrid = std::array<T, Rows * Cols>;
```

The coordinate-to-index mapping is just row-major order written as a formula — `row * cols + col` — the same layout the compiler uses internally:

```cpp
constexpr std::size_t index2d(std::size_t row,
                              std::size_t col,
                              std::size_t cols)
{
    return row * cols + col;
}
```

In use:

```cpp
FlatGrid<int, 3, 4> grid {
    1, 2, 3, 4,
    5, 6, 7, 8,
    9, 10, 11, 12,
};

int value { grid[index2d(1, 2, 4)] };   // row 1, col 2 -> 7
```

Flattening shows up in performance-sensitive code and in interop with C APIs that expect a flat buffer. The price is that the coordinate mapping is now *your* responsibility — get `row * cols + col` wrong and you silently read the wrong cell. (The lab's stretch goal asks you to build exactly this: a `FlatBoard = std::array<char, 9>` with `idx(r, c) = r * 3 + c`.)

### A glimpse ahead: views and mdspan

There's a tidier idea on the horizon. A **view** can present a 2D interface over flat storage *without owning* that storage — you keep one contiguous array, and the view does the `row * cols + col` arithmetic for you. C++23 standardizes this as `std::mdspan`, a general multidimensional view over a contiguous sequence.

```text
flat storage:   [1 2 3 4 5 6 7 8 9 10 11 12]
2D view:         a row/col interface that maps onto the flat indices
```

For this course the takeaway is the choice, not the C++23 machinery: for a small fixed table, nested `std::array` is perfectly fine and the most readable. When the table is large, performance-critical, or bound for an API that wants flat data, flatten it.


---

## 17.x — Chapter 17 summary and quiz

### The core ideas, gathered

You now have both fixed-size array tools and the machinery beneath them. The essentials:

- `std::array<T, N>` is a fixed-size, standard-library array. The length `N` is **part of the type** and must be a compile-time value.
- Reach for `std::array` when the length is fixed and known while compiling — especially when you want a `constexpr` array. Reach for `std::vector` when the length is a runtime fact or can change.
- Always value-initialize with `{}` when you don't supply explicit values; bare `std::array<int, 4> bad;` leaves fundamental-type elements as garbage.
- CTAD can deduce both the element type and the length from the initializers.
- `operator[]` does no bounds checking. `std::get<I>` checks a compile-time index *at compile time*; `at()` checks at runtime by throwing.
- Pass `std::array` by `const` reference for read-only access; use a function template to accept arrays of varying element type or length, and `static_assert` on `N` to enforce length requirements.
- Arrays can't hold references — use `std::reference_wrapper` (and `std::ref`/`std::cref`) for a container of references.
- Enum-indexed `std::array` tables replace repetitive `switch`/string-conversion code; guard them with `static_assert(std::size(table) == enumCount)`.
- **C-style arrays** are built into the language, but in most expressions they **decay** to a pointer to their first element, and decay **loses the length** — the source of nearly every C-array hazard.
- A C-style array parameter is always really a pointer; any length in the brackets is ignored.
- Pointer arithmetic moves by *elements*, not bytes; `ptr[n]` is defined as `*(ptr + n)`; and the half-open range `[begin, end)` is the model under all of C++ iteration.
- **C-style strings** are `char` arrays ending in `'\0'`; that terminator is how length survives decay. Prefer `std::string` and `std::string_view`.
- Multidimensional C-style arrays and nested `std::array` both use **row-major** layout. Flattening stores a grid in one contiguous 1D array with a manual `row * cols + col` mapping.

### Decision table

When you're unsure which tool to grab, this is the short version:

| Need | Prefer |
|---|---|
| Fixed compile-time table | `constexpr std::array` |
| Runtime-sized list | `std::vector` |
| Read-only text parameter | `std::string_view` |
| Owned, modifiable text | `std::string` |
| Legacy API expects `char*` / `const char*` | C-style string, carefully |
| Container of references | `std::reference_wrapper` (or pointers) |
| 2D fixed table, readability first | nested `std::array` |
| 2D table needing flat/contiguous storage | flat `std::array` + index mapping |

### The mental model

When C-style arrays confuse you, almost everything resolves to one question:

```text
std::array<T, N>
    owns N elements
    keeps its length in its type
    passes normally by reference, length intact

C-style array T[N]
    owns N elements
    decays to T* in most expressions
    loses its length the moment it decays

C-style string char[N]
    a C-style array of char
    recovers its length after decay via the '\0' terminator
```

> **Key insight:** When something about a C-style array surprises you, ask: *"Do I still have the array object, or has it decayed to a pointer?"* Almost every C-style-array gotcha is really a length that went missing at decay.

### Where this goes next: the chapter lab

The lab — a **stateless tic-tac-toe referee** — exercises both halves of the chapter in one place. **Part A** builds win-detection over a 2D `std::array` board (`std::array<std::array<char, 3>, 3>`, aliased to `Board`): you pass it by `const Board&`, traverse it with nested range-`for` and `const auto&` rows, and index it `board[r][c]`. **Part B** drops into C-style territory: `sumScores(const int scores[], int count)` makes you *feel* decay — `scores` is really a pointer, so `count` is the only surviving length — and `firstNegative(begin, end)` makes you walk a half-open `[begin, end)` range with `++p`, returning `end` for "not found." Everything you need is in 17.1–17.9; the lab is where these shapes stop being prose and start being muscle memory.

This same half-open `[begin, end)` convention is, not coincidentally, exactly how the iterators and algorithms of Chapter 18 work — so the pointer arithmetic you practice here is a direct on-ramp to the standard library's most powerful tools.
