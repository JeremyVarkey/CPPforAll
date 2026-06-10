# Chapter 10 — Type Conversion, Type Aliases, and Type Deduction

> Source: <https://www.learncpp.com/> (Chapter 10)
> One file per chapter. Each lesson is a section below.

## Contents
- [10.1 — Implicit type conversion](#101--implicit-type-conversion)
- [10.2 — Floating-point and integral promotion](#102--floating-point-and-integral-promotion)
- [10.3 — Numeric conversions](#103--numeric-conversions)
- [10.4 — Narrowing conversions, list initialization, and constexpr initializers](#104--narrowing-conversions-list-initialization-and-constexpr-initializers)
- [10.5 — Arithmetic conversions](#105--arithmetic-conversions)
- [10.6 — Explicit type conversion (casting) and static_cast](#106--explicit-type-conversion-casting-and-static_cast)
- [10.7 — Typedefs and type aliases](#107--typedefs-and-type-aliases)
- [10.8 — Type deduction for objects using the auto keyword](#108--type-deduction-for-objects-using-the-auto-keyword)
- [10.9 — Type deduction for functions](#109--type-deduction-for-functions)
- [10.x — Chapter 10 summary and quiz](#10x--chapter-10-summary-and-quiz)

---

## 10.1 — Implicit type conversion

### Why conversions exist

C++ is statically typed: objects and expressions have types. But programs often
combine values of different types:

```cpp
double d { 3 };      // int literal converted to double
int x { 2 };
double y { x + 0.5 }; // x participates in floating-point expression
```

A **type conversion** produces a value of one type from a value of another type.

```
source value:  int 3
conversion:    int -> double
result value:  double 3.0
```

### Implicit conversion

An **implicit conversion** happens automatically when C++ needs a value of a
different type.

Common places:

- initialization,
- assignment,
- function call argument binding,
- return statements,
- arithmetic expressions with mixed operand types,
- boolean conditions.

```cpp
void printDouble(double value);

int n { 5 };
printDouble(n); // n implicitly converted to double
```

### Standard conversions

C++ defines many built-in conversion rules. Important buckets:

| Conversion kind | Example |
|---|---|
| Numeric promotion | `short` -> `int`, `float` -> `double` |
| Numeric conversion | `double` -> `int`, `int` -> `double`, signed -> unsigned |
| Boolean conversion | `0` -> `false`, nonzero -> `true` |
| Qualification conversion | adding `const` |
| Pointer conversion | derived pointer -> base pointer, null pointer conversions |

This chapter focuses mostly on numeric conversions and type deduction.

### Conversion can fail conceptually

C++ may compile a conversion that loses information:

```cpp
int x { static_cast<int>(3.9) }; // x becomes 3
```

The compiler can convert the value, but the meaning may not be what you intended.
That is why warnings and brace initialization matter.

CS6340 tie-in: LLVM APIs often use very specific types (`unsigned`, `size_t`,
`Value*`, `Instruction*`, references). Learn to ask: "What type does this
expression actually become?"

---

## 10.2 — Floating-point and integral promotion

### Numeric promotion

A **numeric promotion** converts a smaller or less-preferred arithmetic type into
a type C++ prefers to compute with.

Promotions are generally safe because they preserve value.

Examples:

```cpp
short s { 3 };
int i { s };      // short promoted/converted to int

float f { 1.5f };
double d { f };   // float promoted to double
```

### Why promotions reduce overload/operator complexity

Without promotions, C++ would need separate operator implementations for every
tiny type combination:

```
char + char
char + short
short + short
bool + char
...
```

Instead, small integral types usually promote to `int`, so the compiler can use
the ordinary `int` operation.

### Integral promotions

Types like `bool`, `char`, `signed char`, `unsigned char`, `short`, and sometimes
small enums can promote to `int` if `int` can represent all values. Otherwise,
they promote to `unsigned int`.

```cpp
char a { 10 };
char b { 20 };

auto c { a + b }; // often int, not char
```

This surprises beginners: arithmetic on small types often produces `int`.

### Floating-point promotion

`float` can promote to `double`.

```cpp
float f { 0.1f };
double d { f };
```

Not every widening conversion is a formal promotion. LearnCpp distinguishes
promotions from other conversions because promotions are a special safe subset.

---

## 10.3 — Numeric conversions

### Numeric conversion

A **numeric conversion** converts one arithmetic type to another when it is not a
promotion.

Examples:

```cpp
double d { 5.0 };
int i { static_cast<int>(d) };

int n { 5 };
double x { n };
```

### Safe vs unsafe

Safe-ish conversion:

```cpp
int n { 5 };
double d { n }; // 5 exactly representable
```

Potentially unsafe conversions:

```cpp
int a { static_cast<int>(3.7) };       // fractional part lost
unsigned int b { static_cast<unsigned int>(-1) }; // wraps to large value
short c { static_cast<short>(100000) }; // may overflow/truncate
```

### Data loss categories

| Conversion | Risk |
|---|---|
| Floating -> integral | fractional part discarded |
| Wider integer -> narrower integer | value may not fit |
| Signed -> unsigned | negative values wrap to large positives |
| Floating -> narrower floating | precision/range loss |
| Integer -> floating | very large integers may lose precision |

ASCII:

```
double 3.9
   |
   | convert to int
   v
int 3       fractional .9 discarded
```

### Signed to unsigned trap

```cpp
int signedValue { -1 };
unsigned int unsignedValue { static_cast<unsigned int>(signedValue) };
```

`unsignedValue` is not `-1`; it becomes a large unsigned value according to the
unsigned representation rules.

This matters constantly with container sizes:

```cpp
std::string s { "abc" };
auto len { s.length() }; // unsigned size type
```

Mixing signed loop variables and unsigned lengths can trigger warnings and
sometimes bugs. Handle conversions deliberately.

---

## 10.4 — Narrowing conversions, list initialization, and constexpr initializers

### Narrowing conversions

A **narrowing conversion** is a conversion that may lose data or change value.

Examples:

```cpp
int a = 3.5;      // narrows double to int
int b = 100000L;  // may narrow long to int depending on platform
```

### Brace initialization blocks narrowing

List/brace initialization rejects many narrowing conversions:

```cpp
int x { 3.5 }; // compile error
```

This is one reason LearnCpp favors brace initialization.

Safer:

```cpp
int x { 3 };
double d { 3 };
```

### Intentional narrowing should be explicit

If you intend to narrow, say so:

```cpp
double raw { 3.8 };
int truncated { static_cast<int>(raw) };
```

That tells the reader:

```
I know this loses fractional data.
This is not an accidental conversion.
```

### Some constexpr conversions are allowed

If the compiler can prove a constexpr value fits, list initialization may allow
it:

```cpp
constexpr int small { 5 };
char c { small }; // okay if value fits in char on the platform
```

But if the value is not known until runtime, the compiler cannot prove safety:

```cpp
int runtime { 5 };
// char c { runtime }; // narrowing error under list initialization
```

### CS6340 tie-in

When converting indexes, sizes, opcodes, line numbers, and columns, ask:

1. Could the target type be smaller?
2. Could signedness change?
3. Is this conversion intentional?
4. Should `static_cast` document it?

---

## 10.5 — Arithmetic conversions

### Operators often need matching types

Many binary operators need both operands to share a common type before the
operation happens.

```cpp
int i { 2 };
double d { 3.5 };

auto result { i + d }; // i converted to double, result is double
```

Mental model:

```
int + double
  -> convert int to double
  -> double + double
  -> double result
```

### Usual arithmetic conversions

C++ applies a ranked set of rules to choose a common type. The exact full rules
are detailed, but the practical intuition:

- If a floating-point type is involved, the result often becomes the highest
  ranked floating type among the operands.
- Otherwise, integral promotions happen.
- Then signedness/rank rules decide the common integer type.

### Signed/unsigned issues

Classic problem:

```cpp
int x { -1 };
unsigned int y { 1 };

if (x < y)
{
    std::cout << "expected?\n";
}
```

Before comparison, `x` may convert to unsigned, becoming a huge value. The result
can surprise you.

ASCII:

```
signed -1
  -> converted to unsigned
  -> huge positive value
  -> comparison flips intuition
```

Best defense:

- avoid unnecessary signed/unsigned mixing,
- pay attention to compiler warnings,
- use explicit casts only when you know the range is safe,
- prefer signed loop counters for ordinary counting unless an API requires
  otherwise.

### `std::common_type`

`std::common_type_t<T, U>` can ask the type system what common type two types
would have.

```cpp
#include <type_traits>

using Common = std::common_type_t<int, double>; // double
```

You probably will not need this in Lab 1, but it is useful vocabulary.

---

## 10.6 — Explicit type conversion (casting) and static_cast

### Explicit conversion

An explicit conversion is one you request directly.

```cpp
double d { 3.7 };
int i { static_cast<int>(d) };
```

### Avoid C-style casts

C-style casts look like this:

```cpp
int i { (int)d };
```

Avoid them in C++ because they can perform several different kinds of casts. That
makes them less precise and harder to audit.

### Prefer `static_cast` for ordinary value conversions

`static_cast<T>(expr)` is the normal explicit cast for many safe-ish, checked at
compile-time conversions:

```cpp
double d { 3.7 };
int i { static_cast<int>(d) };
```

The syntax is loud on purpose:

```
static_cast<int>(d)
          ^ target type
               ^ source expression
```

### Use `static_cast` to document narrowing

```cpp
std::string s { "abc" };

int length { static_cast<int>(s.length()) };
```

This says: "I know `.length()` returns an unsigned size type; I intentionally
want an `int` here." That can be acceptable when the value range is obviously
small enough.

### Casting vs temporary construction

These can look similar:

```cpp
int a { static_cast<int>(3.7) };
int b { int { 3.7 } }; // list init blocks narrowing; not equivalent
```

Use `static_cast` when the point is conversion. Use direct/list initialization
when the point is constructing an object with safe initialization.

### CS6340 examples

```cpp
auto line { debugLoc.getLine() }; // maybe unsigned
int lineForRuntime { static_cast<int>(line) };
```

```cpp
char replacement {
    static_cast<char>('a' + randomOffset)
};
```

Do not scatter casts to silence warnings blindly. A cast is a promise. Make sure
you understand what it promises.

---

## 10.7 — Typedefs and type aliases

### Type aliases

A **type alias** gives an existing type another name.

Modern syntax:

```cpp
using StudentId = int;
using ErrorCode = int;
```

Then:

```cpp
StudentId id { 42 };
```

### Aliases do not create distinct types

This is important:

```cpp
using StudentId = int;
using CourseId = int;

StudentId s { 1 };
CourseId c { s }; // allowed: both are just int
```

Aliases improve readability, but they do not create type safety.

### Scope

Aliases obey scope rules:

```cpp
void f()
{
    using Count = int;
    Count x { 0 };
}

// Count not visible here
```

### `typedef`

Older C/C++ syntax:

```cpp
typedef int StudentId;
```

Prefer `using` in modern C++:

```cpp
using StudentId = int;
```

The `using` form reads left-to-right and works better with templates.

### When aliases help

1. Platform-independent names:

```cpp
using Distance = double;
```

2. Complex types:

```cpp
using SeedPool = std::vector<std::string>;
using CampaignSeeds = std::map<Campaign, SeedPool>;
```

3. Documentation:

```cpp
using LineNumber = int;
using ColumnNumber = int;
```

4. Maintenance:

```cpp
using Score = double; // can change to float/decimal type in one place
```

### Downsides

Too many aliases can obscure the real type. Use aliases when they clarify intent
or simplify ugly types, not just to rename everything.

CS6340 tie-in:

```cpp
using CoveragePoint = std::pair<int, int>; // line, column
using CoverageSet = std::set<CoveragePoint>;
```

This makes Part 3 coverage bookkeeping much more readable.

---

## 10.8 — Type deduction for objects using the auto keyword

### `auto`

`auto` asks the compiler to deduce the type from the initializer.

```cpp
auto x { 5 };       // int
auto y { 3.14 };    // double
auto name { "Ada" }; // const char*, not std::string
```

Type deduction must have an initializer:

```cpp
auto x; // error: nothing to deduce from
```

### `auto` drops top-level const

```cpp
const int a { 5 };
auto b { a }; // b is int, not const int
```

If you want const:

```cpp
const auto b { a };
```

### String literal deduction

This one matters:

```cpp
auto s { "hello" };
```

`s` is not `std::string`. It deduces to a C-style string pointer-ish type
(`const char*` after array-to-pointer adjustment in this context).

If you want `std::string`:

```cpp
std::string s { "hello" };
```

If you want `std::string_view`:

```cpp
std::string_view s { "hello" };
```

### `auto` and constexpr

```cpp
constexpr auto maxTests { 10000 }; // deduces int, requires constant expression
```

### Benefits

`auto` is helpful when:

- the type is obvious from the initializer,
- the exact type is long or ugly,
- the type could change without affecting logic,
- working with iterators or template-heavy APIs.

```cpp
auto it { seeds.begin() };
```

LLVM-style code often uses `auto *` to keep pointer-ness visible:

```cpp
auto *M { F.getParent() };
```

That says:

```
deduce the pointed-to type,
but make it clear this is a pointer
```

### Downsides

`auto` can hide important types:

```cpp
auto x { getValue() }; // what is x?
```

If the type matters for understanding, write it explicitly.

Best practice for learning:

- use explicit types when the type is part of the lesson,
- use `auto` when the initializer makes the type obvious or the real type is
  noisy.

---

## 10.9 — Type deduction for functions

### Return type deduction

Functions can use `auto` as the return type:

```cpp
auto add(int a, int b)
{
    return a + b; // deduces int
}
```

The compiler deduces the return type from return statements.

### Benefits

Return type deduction can reduce duplication for long types:

```cpp
auto makeSeeds()
{
    return std::vector<std::string>{ "a", "b", "c" };
}
```

### Downsides

It can hide important API information. Function declarations are contracts; the
caller often benefits from seeing the return type.

Less clear:

```cpp
auto readCoverage();
```

Clearer:

```cpp
std::set<std::pair<int, int>> readCoverage();
```

Or with alias:

```cpp
using CoveragePoint = std::pair<int, int>;
using CoverageSet = std::set<CoveragePoint>;

CoverageSet readCoverage();
```

### Trailing return type

C++ supports trailing return syntax:

```cpp
auto add(int a, int b) -> int
{
    return a + b;
}
```

This is useful in some template cases where the return type depends on parameter
types. You may see it in modern C++ and library code.

### Function parameter deduction

Normal functions cannot use plain `auto` parameters in the same simple way
covered here, unless using newer abbreviated function template syntax. For now,
write parameter types explicitly:

```cpp
void mutate(std::string input); // clear
```

---

## 10.x — Chapter 10 summary and quiz

### Core takeaways

- Type conversion creates a value of one type from a value of another type.
- Implicit conversions happen automatically.
- Promotions are safe standard conversions to preferred computation types.
- Numeric conversions may lose information.
- Narrowing conversions are dangerous; brace initialization catches many.
- Use `static_cast` for intentional explicit conversions.
- Avoid C-style casts in C++.
- Mixed arithmetic operands are converted to a common type.
- Signed/unsigned mixing can invert your intuition.
- Type aliases improve readability but do not create new distinct types.
- `auto` deduces object types from initializers.
- `auto` can hide important information if overused.
- Function return type deduction is useful, but public APIs often benefit from
  explicit return types.

### Lab-oriented decision table

| Situation | Good move |
|---|---|
| Need to convert `size_t` length to `int` for a small controlled string | `static_cast<int>(s.length())`, after thinking about range |
| Need readable coverage storage type | `using CoverageSet = std::set<std::pair<int, int>>;` |
| LLVM call returns pointer with noisy type | `auto *M { F.getParent() };` |
| String literal assigned with `auto` | Be careful: not `std::string` |
| Warning about signed/unsigned comparison | Do not silence blindly; align types deliberately |
| Possible fractional loss | Use explicit `static_cast<int>` if intentional |

### Mini drill

```cpp
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

using MutantList = std::vector<std::string>;

int main()
{
    MutantList mutants { "abc", "abcd", "abcde" };

    for (const auto& mutant : mutants)
    {
        auto length { mutant.length() }; // size type

        if (length > 4)
        {
            int lengthForLog { static_cast<int>(length) };
            std::string_view label { "long mutant" };

            std::cout << label << ": " << lengthForLog << '\n';
        }
    }
}
```

What this reinforces:

- `using` creates a readability alias.
- `const auto&` avoids copying each string.
- `.length()` does not return `int`.
- `static_cast<int>` documents an intentional conversion.
- `std::string_view` is good for read-only labels.
