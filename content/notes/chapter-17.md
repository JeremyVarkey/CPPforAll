# Chapter 17 — Fixed-Size Arrays: std::array and C-Style Arrays

> Source: <https://www.learncpp.com/> (Chapter 17)
> One file per chapter. Each lesson is a section below.

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

### Fixed-size vs dynamic arrays

C++ has several array-like tools:

| Type | Size known when? | Can resize? | Best beginner use |
|---|---|---|---|
| `std::vector<T>` | runtime | yes | normal growable arrays |
| `std::array<T, N>` | compile time | no | fixed-size, constexpr-friendly arrays |
| C-style array | compile time | no | legacy/interoperability/global constexpr data |

`std::array` is a fixed-size array container from `<array>`.

```cpp
#include <array>

std::array<int, 4> counters {};
```

The two template arguments are:

| Argument | Meaning |
|---|---|
| `int` | element type |
| `4` | number of elements |

The length is part of the type:

```cpp
std::array<int, 4> four {};
std::array<int, 5> five {};
```

`four` and `five` have different types.

### Why use std::array?

Use `std::array` when:

- the length is known at compile time,
- the length should not change,
- you want a standard-library container interface,
- the array can be `constexpr`,
- you want safer behavior than raw C-style arrays.

Prefer `std::vector` when the number of elements is only known at runtime or the
collection may grow/shrink.

CS6340 tie-in:

```cpp
constexpr std::array<std::string_view, 3> mutationKindNames {
    "replace",
    "delete",
    "insert"
};
```

That table is small, fixed, and known at compile time, so `std::array` is a good
fit.

### Length must be a constant expression

```cpp
constexpr int maxKinds { 3 };
std::array<int, maxKinds> counts {};
```

This works because `maxKinds` is known at compile time.

This does not:

```cpp
int readCount();

int count { readCount() };
// std::array<int, count> values {}; // error: count is runtime data
```

Use `std::vector<int> values(count);` for runtime lengths.

### Zero-length std::array

`std::array<T, 0>` is allowed, but it has no elements.

```cpp
std::array<int, 0> empty {};

if (empty.empty())
{
    // no valid index exists
}
```

Do not subscript a zero-length array. There is no `empty[0]`.

### Aggregate initialization

`std::array` is an aggregate, so it uses aggregate initialization:

```cpp
std::array<int, 5> primes { 2, 3, 5, 7, 11 };
```

Elements are initialized in order:

```
index:   0  1  2  3   4
value:   2  3  5  7  11
```

If you omit some initializers, remaining elements are value-initialized:

```cpp
std::array<int, 5> values { 4, 8 }; // 4, 8, 0, 0, 0
```

If you provide too many initializers, compilation fails.

### Value-initialize when no values are supplied

Without braces:

```cpp
std::array<int, 4> bad; // int elements are default-initialized, not zeroed
```

With braces:

```cpp
std::array<int, 4> good {}; // int elements are value-initialized to 0
```

Use empty braces when creating an array without explicit values.

### const and constexpr arrays

```cpp
const std::array<int, 3> runtimeConst { 1, 2, 3 };
constexpr std::array<int, 3> compileTime { 1, 2, 3 };
```

`const` means the array cannot be modified after initialization.

`constexpr` means the array can participate in compile-time evaluation. This is
the main reason `std::array` matters in modern C++.

### CTAD

Class template argument deduction can infer the element type and length from the
initializers:

```cpp
constexpr std::array ids { 10, 20, 30 }; // std::array<int, 3>
```

CTAD is convenient when all values naturally have the desired type. If the
deduced type is not what you want, specify it:

```cpp
std::array<double, 3> weights { 1, 2, 3 }; // doubles, not ints
```

### std::to_array

C++20 has `std::to_array` for cases where you want the compiler to deduce length
but you want to specify the element type.

```cpp
constexpr auto smallValues { std::to_array<short>({ 1, 2, 3 }) };
```

Use it selectively. Direct `std::array` initialization is simpler and avoids the
temporary object that `std::to_array` may create.

### Basic indexing

```cpp
constexpr std::array labels { 'a', 'b', 'c' };
char second { labels[1] };
```

`operator[]` does not perform bounds checking. Invalid indices are undefined
behavior.

---

## 17.2 — std::array length and indexing

### Length type

`std::array` is essentially declared like:

```cpp
template <typename T, std::size_t N>
struct array;
```

The length parameter `N` is a non-type template parameter of type
`std::size_t`. That is an unsigned integer type.

This means standard-library container length/index APIs tend to return unsigned
types.

### Getting length

Common ways:

```cpp
constexpr std::array values { 4, 8, 15, 16 };

values.size();      // member function, unsigned size_type
std::size(values);  // C++17, unsigned size_type
std::ssize(values); // C++20, signed integer type
```

Use `std::ssize()` when you want to loop with a signed index.

```cpp
for (int i { 0 }; i < std::ssize(values); ++i)
{
    std::cout << values[static_cast<std::size_t>(i)] << '\n';
}
```

The cast marks the boundary between signed loop logic and unsigned indexing.

### Length can often be constexpr

The length of a `std::array` is known at compile time.

```cpp
std::array values { 1, 2, 3 };
constexpr int length { static_cast<int>(std::size(values)) };
```

However, there are historical language limitations around getting a constexpr
length from an array passed by reference before C++23. A common workaround is to
make the length a template parameter:

```cpp
template <typename T, std::size_t N>
void printLength(const std::array<T, N>&)
{
    std::cout << N << '\n';
}
```

Now `N` is directly available as a compile-time value.

### operator[] vs at() vs std::get

| Tool | Bounds check | Index known when? | Notes |
|---|---|---|---|
| `arr[index]` | no | runtime or compile time | fastest/common |
| `arr.at(index)` | runtime | runtime or compile time | throws on bad index |
| `std::get<I>(arr)` | compile time | compile time | catches invalid constexpr index |

Example:

```cpp
constexpr std::array values { 10, 20, 30 };

static_assert(std::get<1>(values) == 20);
```

`std::get<1>` uses the index as a non-type template argument. If the index is out
of bounds, compilation fails.

### Indexing strategy

Use `operator[]` when:

- you already know the index is valid,
- the index was checked before indexing,
- performance and ordinary container syntax matter.

Use `std::get<I>` when:

- the index is a compile-time constant,
- a compile-time error would be better than runtime undefined behavior.

Avoid relying on `at()` as your main safety strategy. Checking before indexing
usually leads to clearer control flow.

---

## 17.3 — Passing and returning std::array

### Passing std::array by value copies the whole array

```cpp
void printCopy(std::array<int, 100> values); // copies 100 ints
```

Prefer pass by const reference for read-only access:

```cpp
void printValues(const std::array<int, 100>& values)
{
    for (int value : values)
        std::cout << value << ' ';
}
```

Use non-const reference when the function intentionally modifies the caller's
array:

```cpp
void reset(std::array<int, 100>& values)
{
    for (int& value : values)
        value = 0;
}
```

### Element type and length are part of the parameter type

```cpp
void printThree(const std::array<int, 3>& values);
```

This function accepts `std::array<int, 3>`, not `std::array<int, 4>` or
`std::array<double, 3>`.

To accept multiple element types and lengths, write a function template:

```cpp
template <typename T, std::size_t N>
void printArray(const std::array<T, N>& values)
{
    for (const T& value : values)
        std::cout << value << ' ';
}
```

Now the compiler can instantiate:

```
printArray<int, 3>
printArray<double, 3>
printArray<int, 8>
```

### Template only what should vary

If only length should vary:

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

This accepts any length but only `int` elements.

If both type and length should vary:

```cpp
template <typename T, std::size_t N>
T sum(const std::array<T, N>& values)
{
    T total {};

    for (const T& value : values)
        total += value;

    return total;
}
```

### Static assertions on length

When a function requires a minimum length, use `static_assert` on the template
length:

```cpp
template <typename T, std::size_t N>
T thirdElement(const std::array<T, N>& values)
{
    static_assert(N > 2, "thirdElement requires at least three elements");
    return values[2];
}
```

That catches invalid fixed-size arrays at compile time.

### Returning std::array

Returning a `std::array` by value copies the array object. This can be fine when:

- the array is small,
- elements are cheap to copy,
- the function is not in a hot path,
- returning a value keeps the interface simple.

```cpp
std::array<int, 3> makeRgb(int red, int green, int blue)
{
    return { red, green, blue };
}
```

For large arrays or expensive elements, consider whether the caller should supply
the array to fill:

```cpp
void fillRgb(std::array<int, 3>& rgb, int red, int green, int blue)
{
    rgb[0] = red;
    rgb[1] = green;
    rgb[2] = blue;
}
```

Prefer clear return values until profiling or ownership requirements justify the
more awkward interface.

---

## 17.4 — std::array of class types, and brace elision

### std::array can hold objects

Elements can be fundamental types, pointers, structs, classes, or other arrays.

```cpp
struct CoverageCounter
{
    std::string_view name {};
    int hits {};
};

std::array<CoverageCounter, 2> counters {};
```

Assigning elements works naturally:

```cpp
counters[0] = { "entry", 1 };
counters[1] = { "exit", 0 };
```

The compiler knows each element is a `CoverageCounter`.

### Initializing arrays of structs

With explicit element type per initializer, CTAD can work clearly:

```cpp
constexpr std::array counters {
    CoverageCounter { "entry", 1 },
    CoverageCounter { "exit", 0 },
};
```

If you explicitly specify `std::array<CoverageCounter, 2>` and omit the element
type inside each row, you may need extra braces:

```cpp
constexpr std::array<CoverageCounter, 2> counters {{
    { "entry", 1 },
    { "exit", 0 },
}};
```

### Why double braces appear

Conceptually, `std::array<T, N>` is an aggregate that contains one underlying
C-style array member:

```cpp
template <typename T, std::size_t N>
struct array
{
    T storage[N];
};
```

So initialization may have two levels:

```
std::array object
    underlying storage array
        element 0
        element 1
```

Double braces make the levels explicit:

```cpp
std::array<CoverageCounter, 2> counters {{
    { "entry", 1 },
    { "exit", 0 },
}};
```

### Brace elision

C++ aggregate initialization has brace elision rules that allow some braces to
be omitted.

Scalar element arrays usually work with one brace layer:

```cpp
std::array<int, 4> values { 1, 2, 3, 4 };
```

Struct/class element arrays may need either:

```cpp
std::array items {
    CoverageCounter { "entry", 1 },
    CoverageCounter { "exit", 0 },
};
```

or:

```cpp
std::array<CoverageCounter, 2> items {{
    { "entry", 1 },
    { "exit", 0 },
}};
```

When in doubt, double braces for `std::array` are acceptable and make the
aggregate shape explicit.

---

## 17.5 — Arrays of references via std::reference_wrapper

### Arrays cannot store references directly

References are not objects and cannot be reseated.

```cpp
int a { 1 };
int b { 2 };

// std::array<int&, 2> refs { a, b }; // error
```

If you use CTAD with references, the references are usually copied as values:

```cpp
int& ra { a };
int& rb { b };

std::array values { ra, rb }; // std::array<int, 2>, not references
```

### std::reference_wrapper

`std::reference_wrapper<T>` is a small object that behaves like a reseatable
reference wrapper. It lives in `<functional>`.

```cpp
#include <array>
#include <functional>

int hot { 10 };
int cold { 2 };

std::array<std::reference_wrapper<int>, 2> refs { hot, cold };
```

To modify the referenced object:

```cpp
refs[0].get() = 11; // modifies hot
```

`get()` returns a real `T&`.

### Reseating vs modifying

`std::reference_wrapper` assignment reseats the wrapper:

```cpp
int x { 1 };
int y { 2 };
int z { 3 };

std::reference_wrapper<int> ref { x };

ref.get() = 10; // modifies x
ref = z;        // ref now refers to z
```

That is different from a true C++ reference, which cannot be reseated.

### std::ref and std::cref

Helper functions:

```cpp
auto ref { std::ref(hot) };    // reference_wrapper<int>
auto cref { std::cref(cold) }; // reference_wrapper<const int>
```

Use `std::reference_wrapper` when you need a container of references. Use
pointers when nullability or pointer-style APIs make the ownership story clearer.

---

## 17.6 — std::array and enumerations

### Enumerators as array indices

Enums are useful for naming positions in fixed tables.

```cpp
namespace Counter
{
    enum Kind
    {
        entry,
        branch,
        exit,
        maxKinds
    };
}

std::array<int, Counter::maxKinds> hits {};
hits[Counter::branch] = 3;
```

This is clearer than remembering that `1` means branch.

### Protect table length with static_assert

When CTAD deduces the array length, a missing initializer can silently create a
shorter array than intended.

```cpp
namespace Counter
{
    enum Kind
    {
        entry,
        branch,
        exit,
        maxKinds
    };

    using namespace std::string_view_literals;

    constexpr std::array names { "entry"sv, "branch"sv, "exit"sv };
    static_assert(std::size(names) == maxKinds);
}
```

If someone adds `function` before `maxKinds` but forgets to add a name, the
compile-time assertion catches the mismatch.

### Mapping enum to string

For sequential enum values starting at 0, a table can replace repetitive switch
logic.

```cpp
constexpr std::string_view nameOf(Counter::Kind kind)
{
    return Counter::names[static_cast<std::size_t>(kind)];
}
```

Use a cast at the indexing boundary when needed. With unscoped enums, implicit
conversion to an integer often happens, but an explicit cast makes the boundary
obvious.

### Mapping string back to enum

```cpp
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

The array keeps the source of truth centralized.

### Iterating enum values

Enums themselves are not ranges. A range-based for-loop cannot directly iterate
over all enumerators.

Create a fixed array of enumerators:

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

This works best when enumerators have unique values and the table is maintained
with `static_assert`.

---

## 17.7 — Introduction to C-style arrays

### What a C-style array is

A C-style array is the built-in array type inherited from C.

```cpp
int scores[5] {};
```

This defines five `int` elements:

```
scores[0] scores[1] scores[2] scores[3] scores[4]
```

C-style arrays require no header because they are part of the core language.

### Length rules

For ordinary stack/static C-style arrays:

- length must be a constant expression,
- length must be at least 1,
- element type must be explicit,
- variable-length arrays are not standard C++.

```cpp
constexpr int maxScores { 5 };
int scores[maxScores] {};
```

Avoid compiler extensions that allow runtime lengths:

```cpp
int n {};
std::cin >> n;
// int values[n]; // not standard C++
```

Use `std::vector` for runtime-sized arrays.

### Indexing

```cpp
int values[3] { 10, 20, 30 };

std::cout << values[1] << '\n'; // 20
```

Unlike standard-library containers, C-style arrays accept signed or unsigned
integral indices and unscoped enumerations. There is still no bounds checking.

```cpp
values[3]; // out of bounds: undefined behavior
```

### Initialization

```cpp
int a[4] {};             // 0, 0, 0, 0
int b[4] { 1, 2 };       // 1, 2, 0, 0
int c[] { 1, 2, 3, 4 };  // length deduced as 4
```

When all elements are explicitly listed, prefer omitting the length:

```cpp
constexpr int opcodes[] { 10, 20, 30 };
```

If an initializer is added or removed, the array length updates automatically.

### No CTAD or auto array deduction

C-style arrays are not class templates.

```cpp
// auto values[3] { 1, 2, 3 }; // not allowed
```

If you want deduction from initializers, prefer `std::array`.

### const and constexpr C-style arrays

```cpp
constexpr int lookup[] { 1, 4, 9, 16 };
```

This can be useful for global compile-time tables, especially where C-style
array syntax is less noisy or signed indexing behavior matters.

### Getting length

In C++17:

```cpp
#include <iterator>

constexpr int lookup[] { 1, 4, 9, 16 };

std::size(lookup);  // 4, unsigned
std::ssize(lookup); // C++20, signed
```

Prefer `std::size()` / `std::ssize()` over old `sizeof(array) /
sizeof(array[0])` tricks. The old formula fails after array decay.

### C-style arrays cannot be assigned

```cpp
int values[] { 1, 2, 3 };

values[0] = 9;       // ok: element assignment
// values = { 4, 5 }; // error: array assignment not supported
```

Another reason modern code usually prefers `std::array` or `std::vector`.

---

## 17.8 — C-style array decay

### Array decay

In most expressions, a C-style array converts to a pointer to its first element.
This is called **array decay**.

```cpp
int values[] { 10, 20, 30 };

int* ptr { values }; // values decays to &values[0]
```

Diagram:

```
values object: [10][20][30]
                ^
                |
ptr ------------+
```

The array object has type `int[3]`. The decayed pointer has type `int*`.

### Decay loses length information

After decay:

```cpp
int values[] { 10, 20, 30 };
int* ptr { values };
```

`values` knows its length as part of its type. `ptr` only stores an address.

```
array type:    int[3]  -> element type + length
pointer type:  int*    -> element type only
```

This length loss is the central danger.

### Cases where arrays do not decay

Common no-decay contexts:

- operand to `sizeof`,
- operand to `typeid`,
- address-of the array itself,
- passed by reference,
- stored as a class member.

Most other expression uses decay.

### Function parameters and array syntax

This parameter:

```cpp
void printFirst(const int values[])
{
    std::cout << values[0] << '\n';
}
```

is treated like:

```cpp
void printFirst(const int* values);
```

The `[]` form communicates that the pointer is expected to point at the first
element of an array, but it does not preserve length.

If a length appears in the parameter brackets, it is ignored:

```cpp
void printFirst(const int values[100]); // still effectively const int*
```

### Decay enables one function to accept many lengths

```cpp
void printFirst(const int values[])
{
    std::cout << values[0] << '\n';
}

int a[] { 1, 2, 3 };
int b[] { 9, 8, 7, 6, 5 };

printFirst(a);
printFirst(b);
```

Both arrays decay to `const int*`.

### Decay problems

Problem 1: length functions no longer work.

```cpp
void printLength(const int values[])
{
    // std::size(values); // error: values is a pointer here
}
```

Problem 2: invalid lengths are not detectable from the pointer alone.

```cpp
void printThird(const int values[])
{
    std::cout << values[2] << '\n';
}

int shortArray[] { 4, 5 };
printThird(shortArray); // compiles, undefined behavior
```

Problem 3: refactoring can silently change meaning. Code that worked with a real
array may fail or misbehave after being moved into a function where the parameter
has decayed.

### Workarounds

Pass a separate length:

```cpp
void printAll(const int values[], int length)
{
    for (int i { 0 }; i < length; ++i)
        std::cout << values[i] << ' ';
}
```

This is fragile because caller and callee must agree that `length` matches the
actual array.

Use a terminating sentinel:

```cpp
constexpr int stop { -1 };
int values[] { 4, 8, 15, stop };
```

This only works when a sentinel value is impossible as normal data. C-style
strings use this idea with the null terminator.

### Prefer modern containers

Avoid C-style arrays when practical:

| Need | Prefer |
|---|---|
| Read-only string parameter | `std::string_view` |
| Modifiable text | `std::string` |
| Fixed constexpr local array | `std::array` |
| Runtime-sized array | `std::vector` |
| Legacy API / global constexpr table | C-style array may be acceptable |

---

## 17.9 — Pointer arithmetic and subscripting

### Pointer arithmetic scales by pointed-to type

Pointer arithmetic moves by elements, not raw bytes.

```cpp
int values[] { 10, 20, 30 };
int* ptr { values };

int* next { ptr + 1 }; // points at values[1]
```

If `int` is 4 bytes, `ptr + 1` is 4 bytes after `ptr`. If the pointer were a
`double*`, `ptr + 1` would move by `sizeof(double)` bytes.

### Valid pointer arithmetic bounds

For an array:

```cpp
int values[] { 10, 20, 30 };
```

Valid pointer positions for arithmetic:

```
&values[0]  &values[1]  &values[2]  one-past-end
```

The one-past-end pointer is valid to form and compare, but not to dereference.

```cpp
int* begin { values };
int* end { values + std::size(values) };

for (int* current { begin }; current != end; ++current)
{
    std::cout << *current << '\n';
}
```

### Subscripting is pointer arithmetic plus dereference

For a pointer `ptr`:

```cpp
ptr[n]
```

means:

```cpp
*(ptr + n)
```

So:

```cpp
int values[] { 10, 20, 30 };
int* ptr { values };

std::cout << ptr[2] << '\n'; // *(ptr + 2), prints 30
```

This also explains why arrays are 0-indexed:

```cpp
values[0] == *(values + 0) == *values
```

No offset is needed for the first element.

### Relative indexing

An index on a pointer is relative to the pointer's current position.

```cpp
int values[] { 10, 20, 30, 40 };
int* middle { &values[2] };

middle[0];  // values[2], 30
middle[1];  // values[3], 40
middle[-1]; // values[1], 20
```

This is valid only when the resulting address stays inside the array or one past
the end where allowed.

Use subscripting when indexing from the start of an array. Use pointer arithmetic
when relative positioning is the point.

### Begin/end pairs

A begin/end pair can describe a half-open range:

```
[begin, end)
```

`begin` points to the first element. `end` points one past the last element.

```cpp
void printRange(const int* begin, const int* end)
{
    for (const int* current { begin }; current != end; ++current)
        std::cout << *current << ' ';
}
```

Call:

```cpp
int values[] { 1, 2, 3 };
printRange(values, values + std::size(values));
```

This pattern is the conceptual ancestor of iterators and many standard-library
algorithms.

### Range-for connection

A range-based for-loop over an array is roughly built from begin/end traversal:

```cpp
for (int value : values)
{
    std::cout << value << '\n';
}
```

Conceptually:

```cpp
auto begin { values };
auto end { values + std::size(values) };

for (; begin != end; ++begin)
{
    int value { *begin };
    std::cout << value << '\n';
}
```

---

## 17.10 — C-style strings

### A C-style string is a char array with a terminator

A C-style string is a C-style array of `char` or `const char` that ends with a
null terminator, `'\0'`.

```cpp
char text[] { "log" };
```

Memory shape:

```
'l' 'o' 'g' '\0'
```

The terminator is how functions know where the string ends after the array
decays to a pointer.

### String literals include the terminator

```cpp
const char text[] { "hello" };
```

This array has length 6, not 5:

```
h e l l o \0
```

When defining a C-style string with a literal initializer, omit the explicit
length:

```cpp
char buffer[] { "abc" }; // compiler includes room for '\0'
```

### C-style strings decay

```cpp
void print(const char text[])
{
    std::cout << text << '\n';
}

char message[] { "ok" };
print(message); // message decays to char*
```

`std::cout` treats `char*` and `const char*` specially: it prints characters
until the null terminator.

If the terminator is missing, output can continue into unrelated memory, which
is undefined behavior.

### Input and buffer overflow

C-style strings are fixed-size arrays. If too much input is copied into them,
the program writes past the end of the buffer.

```cpp
char name[8] {};
std::cin >> name; // historically dangerous for long input
```

Safer C-style input uses a length:

```cpp
char name[8] {};
std::cin.getline(name, std::size(name));
```

This still requires careful fixed-buffer management. Prefer `std::string` for
user-entered text:

```cpp
std::string name {};
std::getline(std::cin, name);
```

### Modifying C-style strings

The array cannot be assigned as a whole:

```cpp
char text[] { "cat" };
// text = "dog"; // error
```

Individual elements can be changed if the array is not const:

```cpp
text[0] = 'b'; // "bat"
```

Be careful not to overwrite the null terminator unless you deliberately maintain
another one later in the array.

### Length of array vs length of string

```cpp
char text[16] { "hello" };
```

Array length:

```cpp
std::size(text); // 16
```

String length:

```cpp
std::strlen(text); // 5
```

`std::strlen` from `<cstring>` counts characters until `'\0'`. It works on
decayed C-style strings, but it is O(n) because it must traverse the string.

### Avoid non-const C-style string objects

Non-const C-style string objects are awkward and easy to overflow. Use:

- `std::string` for owned modifiable text,
- `std::string_view` for read-only non-owning text,
- C-style strings only when interoperating with C APIs or old code.

---

## 17.11 — C-style string symbolic constants

### Two C-style ways to name string constants

Array form:

```cpp
const char toolName[] { "mutator" };
```

Pointer form:

```cpp
const char* const phaseName { "instrument" };
```

The array form creates an array initialized from the literal. The pointer form
stores a pointer to a string literal, typically in read-only storage.

### Type deduction with string literals

```cpp
auto a { "abc" };  // const char*
auto* b { "abc" }; // const char*
auto& c { "abc" }; // const char (&)[4]
```

Most uses decay to pointer-to-const-char.

### Printing char pointers

`std::cout` treats character pointers as C-style strings:

```cpp
const char* text { "abc" };
std::cout << text << '\n'; // prints abc
```

For non-character pointers, `std::cout` prints the address.

This can surprise you:

```cpp
char ch { 'x' };
std::cout << &ch; // char*, so cout tries to print a C-style string
```

`&ch` is not a null-terminated string. Printing it as one is undefined behavior.

To print a character pointer address, cast to `const void*`:

```cpp
const char* text { "abc" };
std::cout << static_cast<const void*>(text) << '\n';
```

### Prefer string_view constants

Modern C++ should usually use:

```cpp
constexpr std::string_view phaseName { "instrument" };
```

Advantages:

- explicit length is stored,
- no reliance on null-termination for view length,
- works naturally with string-view-taking functions,
- no mutable buffer hazards.

Use C-style string constants only when an API specifically expects
null-terminated `const char*`.

---

## 17.12 — Multidimensional C-style Arrays

### Dimensions

An array's dimension is the number of indices needed to select an element.

One-dimensional:

```cpp
int line[4] {};
line[2] = 7;
```

Two-dimensional:

```cpp
int grid[3][4] {};
grid[1][2] = 7;
```

`grid` is a 3-element array where each element is a 4-element array of `int`.

### Rows and columns

For 2D arrays, read the left index as row and the right index as column:

```cpp
int grid[3][4] {};
```

Layout:

```
          col 0   col 1   col 2   col 3
row 0   [0][0]  [0][1]  [0][2]  [0][3]
row 1   [1][0]  [1][1]  [1][2]  [1][3]
row 2   [2][0]  [2][1]  [2][2]  [2][3]
```

### Row-major order

C++ stores multidimensional array elements row by row.

For `grid[3][4]`, memory order is:

```
[0][0] [0][1] [0][2] [0][3] [1][0] [1][1] ... [2][3]
```

Traversing row outer, column inner follows memory order:

```cpp
for (std::size_t row { 0 }; row < std::size(grid); ++row)
{
    for (std::size_t col { 0 }; col < std::size(grid[0]); ++col)
        std::cout << grid[row][col] << ' ';

    std::cout << '\n';
}
```

### Initialization

Use nested braces for readability:

```cpp
int grid[2][3] {
    { 1, 2, 3 },
    { 4, 5, 6 },
};
```

Missing inner values are value-initialized:

```cpp
int grid[2][3] {
    { 1 },
    { 4, 5 },
};
```

Result:

```
1 0 0
4 5 0
```

When initializing, the leftmost dimension can be omitted:

```cpp
int grid[][3] {
    { 1, 2, 3 },
    { 4, 5, 6 },
};
```

The compiler can count rows from the initializer list. Non-leftmost dimensions
cannot be omitted because the compiler needs them to compute element layout.

### Nested range-for loops

```cpp
for (const auto& row : grid)
{
    for (int value : row)
        std::cout << value << ' ';

    std::cout << '\n';
}
```

Use `const auto& row` so each row array is not copied.

### Cartesian coordinates vs array indices

Geometry often says `(x, y)`, where `x` is horizontal and `y` is vertical.

C++ array indexing usually says `[row][col]`:

```
row -> vertical position
col -> horizontal position
```

If a program uses coordinate names, convert deliberately:

```cpp
grid[y][x] = value;
```

That is often correct for screen/board data, but the order should be obvious in
the code.

---

## 17.13 — Multidimensional std::array

### std::array is one-dimensional

There is no dedicated standard-library multidimensional `std::array` class.
The usual representation is an array of arrays:

```cpp
std::array<std::array<int, 4>, 3> grid {{
    { 1, 2, 3, 4 },
    { 5, 6, 7, 8 },
    { 9, 10, 11, 12 },
}};
```

Interpretation:

```
std::array<
    std::array<int, 4>, // each row has 4 ints
    3                   // there are 3 rows
>
```

Indexing:

```cpp
grid[1][2]; // row 1, column 2
```

### Function templates for 2D std::array

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

The parameter type is verbose because both dimensions are part of the type.

### Alias templates reduce noise

```cpp
template <typename T, std::size_t Rows, std::size_t Cols>
using Array2d = std::array<std::array<T, Cols>, Rows>;
```

Now:

```cpp
Array2d<int, 3, 4> grid {{
    { 1, 2, 3, 4 },
    { 5, 6, 7, 8 },
    { 9, 10, 11, 12 },
}};
```

And:

```cpp
template <typename T, std::size_t Rows, std::size_t Cols>
void printGrid(const Array2d<T, Rows, Cols>& grid);
```

This is much easier to read.

### Getting dimensions

For a non-empty 2D array:

```cpp
grid.size();    // rows
grid[0].size(); // columns, but requires row 0 to exist
```

Safer template-based helpers use non-type template parameters directly:

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

These do not inspect `grid[0]`, so they avoid zero-row pitfalls.

### Flattening

Nested arrays get verbose as dimensions increase. Another representation is a
flat one-dimensional array with a coordinate mapping.

```cpp
template <typename T, std::size_t Rows, std::size_t Cols>
using FlatGrid = std::array<T, Rows * Cols>;
```

Mapping:

```cpp
constexpr std::size_t index2d(std::size_t row,
                              std::size_t col,
                              std::size_t cols)
{
    return row * cols + col;
}
```

Usage:

```cpp
FlatGrid<int, 3, 4> grid {
    1, 2, 3, 4,
    5, 6, 7, 8,
    9, 10, 11, 12,
};

int value { grid[index2d(1, 2, 4)] };
```

Flattening is common in performance-sensitive code and in APIs that expect
contiguous memory. The cost is that you must maintain the coordinate mapping
correctly.

### Views and mdspan

A view can expose 2D indexing over flat storage without owning the storage. C++23
introduces `std::mdspan` for this general idea: a multidimensional view over a
contiguous sequence.

For this course, the main idea is enough:

```
flat storage:   [1 2 3 4 5 6 7 8 9 10 11 12]
2D view:         row/col interface maps to flat index
```

If you need a simple fixed table, nested `std::array` is fine. If the table is
large, performance-sensitive, or passed to APIs expecting contiguous flat data,
consider flattening.

---

## 17.x — Chapter 17 summary and quiz

### Core takeaways

- `std::array<T, N>` is a fixed-size standard-library array.
- The length `N` is part of the type and must be a compile-time value.
- Use `std::array` for constexpr arrays and fixed-size local tables.
- Use `std::vector` when length is runtime-sized or needs to change.
- Value-initialize arrays with `{}` when no explicit values are supplied.
- CTAD can infer `std::array` element type and length from initializers.
- `operator[]` does not bounds check.
- `std::get<I>(arr)` provides compile-time checking for compile-time indices.
- Pass `std::array` by const reference for read-only use.
- Use function templates to accept arrays of varying element type or length.
- Arrays of references are not allowed; use `std::reference_wrapper` when needed.
- Enum-indexed arrays can replace repetitive switch/string conversion logic.
- Protect enum-backed tables with `static_assert`.
- C-style arrays are built into the language but decay to pointers in most
  expressions.
- Decay loses length information, which is the main reason C-style arrays are
  risky.
- Pointer arithmetic moves by elements, not raw bytes.
- `ptr[n]` means `*(ptr + n)`.
- C-style strings are char arrays ending with `'\0'`.
- Prefer `std::string` and `std::string_view` over C-style string objects.
- Multidimensional C-style arrays and nested `std::array` use row-major layout.
- Flattened arrays store multidimensional data in one contiguous sequence with a
  manual index mapping.

### Decision table

| Need | Prefer |
|---|---|
| Fixed compile-time table | `constexpr std::array` |
| Runtime-sized list | `std::vector` |
| Read-only text parameter | `std::string_view` |
| Owned modifiable text | `std::string` |
| Legacy API expects `char*` / `const char*` | C-style string carefully |
| Container of references | `std::reference_wrapper` or pointers |
| 2D fixed table with simple code | nested `std::array` |
| 2D table with flat storage needs | flat `std::array` plus mapping |

### CS6340 bridge

Fixed-size arrays show up anywhere the set of states is known:

```cpp
enum class ResultKind
{
    passed,
    failed,
    crashed,
    count
};
```

With a table:

```cpp
using namespace std::string_view_literals;

constexpr std::array resultNames {
    "passed"sv,
    "failed"sv,
    "crashed"sv,
};

static_assert(std::size(resultNames) ==
              static_cast<std::size_t>(ResultKind::count));
```

That combination is useful for coverage counters, mutation categories, result
labels, and small lookup tables. The key safety habit is to keep the enum and
the table tied together with compile-time checks.

### Mental model

```
std::array<T, N>
    owns N elements
    keeps length in its type
    passes normally by reference

C-style array T[N]
    owns N elements
    often decays to T*
    loses length when decayed

C-style string char[N]
    C-style array of char
    relies on '\0' to recover string length after decay
```

When confused, ask: "Do I still have the array object, or has it decayed to a
pointer?"
