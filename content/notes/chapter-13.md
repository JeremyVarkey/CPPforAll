# Chapter 13 — Compound Types: Enums and Structs

> Source: <https://www.learncpp.com/> (Chapter 13)
> One file per chapter. Each lesson is a section below.

## Contents
- [13.1 — Introduction to program-defined (user-defined) types](#131--introduction-to-program-defined-user-defined-types)
- [13.2 — Unscoped enumerations](#132--unscoped-enumerations)
- [13.3 — Unscoped enumerator integral conversions](#133--unscoped-enumerator-integral-conversions)
- [13.4 — Converting an enumeration to and from a string](#134--converting-an-enumeration-to-and-from-a-string)
- [13.5 — Introduction to overloading the I/O operators](#135--introduction-to-overloading-the-io-operators)
- [13.6 — Scoped enumerations (enum classes)](#136--scoped-enumerations-enum-classes)
- [13.7 — Introduction to structs, members, and member selection](#137--introduction-to-structs-members-and-member-selection)
- [13.8 — Struct aggregate initialization](#138--struct-aggregate-initialization)
- [13.9 — Default member initialization](#139--default-member-initialization)
- [13.10 — Passing and returning structs](#1310--passing-and-returning-structs)
- [13.11 — Struct miscellany](#1311--struct-miscellany)
- [13.12 — Member selection with pointers and references](#1312--member-selection-with-pointers-and-references)
- [13.13 — Class templates](#1313--class-templates)
- [13.14 — Class template argument deduction (CTAD) and deduction guides](#1314--class-template-argument-deduction-ctad-and-deduction-guides)
- [13.15 — Alias templates](#1315--alias-templates)
- [13.x — Chapter 13 summary and quiz](#13x--chapter-13-summary-and-quiz)
- [13.y — Using a language reference](#13y--using-a-language-reference)

---

## 13.1 — Introduction to program-defined (user-defined) types

### What user-defined types are

C++ gives you fundamental types (`int`, `double`, `bool`, etc.), but real
programs need domain-shaped types. A **program-defined type** is a type you
define in your own code.

Examples:

```cpp
enum class Campaign { MutationA, MutationB, MutationC };

struct CoveragePoint
{
    int line {};
    int column {};
};
```

These types let the type system carry meaning.

```
int, int              -> two generic numbers
CoveragePoint         -> line/column pair with intent
Campaign              -> one of a known set of fuzz strategies
```

### Defining types

Type definitions usually appear before use:

```cpp
struct Student
{
    std::string name {};
    int id {};
};
```

The definition tells the compiler:

- the type exists,
- what members or enumerators it has,
- how objects of that type can be laid out or interpreted.

### Multi-file programs

Program-defined types used across multiple files usually belong in header files.

```cpp
// CoveragePoint.h
#ifndef COVERAGE_POINT_H
#define COVERAGE_POINT_H

struct CoveragePoint
{
    int line {};
    int column {};
};

#endif
```

Then multiple `.cpp` files can include the same definition. Type definitions are
partially exempt from the One Definition Rule when the identical definition is
included in multiple translation units through headers.

### Naming

Use capitalized names for program-defined types in the LearnCpp style:

```cpp
struct CoveragePoint {};
enum class Campaign {};
```

CS6340 tie-in: type names like `Function`, `Module`, `Instruction`, `DebugLoc`,
and `FunctionCallee` are all program-defined/library-defined types. The syntax
is the same idea as your own structs/enums.

---

## 13.2 — Unscoped enumerations

### Enumeration basics

An **enumeration** is a compound type whose values are a fixed set of named
alternatives.

```cpp
enum Color
{
    red,
    green,
    blue,
};
```

`Color` is the enumeration type. `red`, `green`, and `blue` are enumerators.

### Why enums help

Without enum:

```cpp
int campaign { 1 }; // what does 1 mean?
```

With enum:

```cpp
enum Campaign
{
    mutationA,
    mutationB,
    mutationC,
};

Campaign campaign { mutationA };
```

The code says what the value means.

### Unscoped means enumerators leak into surrounding scope

```cpp
enum Color
{
    red,
    green,
    blue,
};

enum TrafficLight
{
    // red, // collision: red already in scope
    yellow,
};
```

Unscoped enumerator names are placed into the surrounding scope. This can cause
name collisions.

Common workaround:

```cpp
enum Color
{
    color_red,
    color_green,
    color_blue,
};
```

Better modern default: use scoped enumerations (`enum class`) from 13.6.

### Enumerated types are distinct

Even if the compiler stores enumerators as integers internally, `Color` is a
distinct type:

```cpp
Color c { red };
```

Use the named enumerators rather than raw integers.

---

## 13.3 — Unscoped enumerator integral conversions

### Unscoped enums convert to integers

Unscoped enumerators have underlying integer values:

```cpp
enum Color
{
    red,   // 0
    green, // 1
    blue,  // 2
};

std::cout << green << '\n'; // often prints 1
```

This implicit conversion can be convenient but also weakens type safety.

### Explicit values

```cpp
enum ErrorCode
{
    ok = 0,
    fileNotFound = 10,
    permissionDenied = 20,
};
```

Values can be assigned manually. If not, they increment from the previous value.

### Value initialization

Value-initializing an unscoped enum gives the zero enumerator value if one
exists:

```cpp
Color c {}; // underlying value 0 -> red in this enum
```

Best practice: make the zero value meaningful when reasonable.

```cpp
enum Status
{
    unknown,
    ready,
    failed,
};
```

### Underlying type

C++ stores enum values using an underlying integral type. You can specify it:

```cpp
enum Color : std::uint8_t
{
    red,
    green,
    blue,
};
```

Do this only when you have a reason, such as storage layout or interop.

### Integer to enum conversion

Converting arbitrary integers to enum values can create values that do not match
any named enumerator.

```cpp
Color c { static_cast<Color>(99) }; // possible, but not a real named color
```

Be careful when parsing external input.

---

## 13.4 — Converting an enumeration to and from a string

### Enum to string

C++ does not automatically know how to print an enum's name. You write the
mapping.

```cpp
enum class Campaign
{
    MutationA,
    MutationB,
    MutationC,
};

std::string_view toString(Campaign campaign)
{
    switch (campaign)
    {
    case Campaign::MutationA: return "MutationA";
    case Campaign::MutationB: return "MutationB";
    case Campaign::MutationC: return "MutationC";
    }

    return "Unknown";
}
```

### String to enum

```cpp
std::optional<Campaign> campaignFromString(std::string_view text)
{
    if (text == "MutationA") return Campaign::MutationA;
    if (text == "MutationB") return Campaign::MutationB;
    if (text == "MutationC") return Campaign::MutationC;

    return std::nullopt;
}
```

This is better than returning a fake default because parse failure is explicit.

### Why this matters

Fuzzer command-line arguments often come in as strings:

```bash
./fuzzer ./target fuzz_input fuzz_output MutationA
```

Internally, code usually wants a structured enum:

```
"MutationA" -> Campaign::MutationA
```

That boundary is exactly where conversion helpers belong.

---

## 13.5 — Introduction to overloading the I/O operators

### Operator overloading

C++ lets program-defined types define behavior for operators. For I/O, common
operators are:

- `operator<<` for output,
- `operator>>` for input.

### Outputting an enum

Instead of:

```cpp
std::cout << toString(campaign);
```

You can define:

```cpp
std::ostream& operator<<(std::ostream& out, Campaign campaign)
{
    return out << toString(campaign);
}
```

Then:

```cpp
std::cout << Campaign::MutationA << '\n';
```

### Inputting an enum

Input is trickier because parsing can fail:

```cpp
std::istream& operator>>(std::istream& in, Campaign& campaign)
{
    std::string text {};
    in >> text;

    if (auto parsed { campaignFromString(text) })
    {
        campaign = *parsed;
    }
    else
    {
        in.setstate(std::ios_base::failbit);
    }

    return in;
}
```

Important pattern: stream operators return the stream by reference so operations
can chain:

```cpp
std::cout << a << b << c;
```

For Lab 1, you probably do not need to overload operators, but reading these
patterns helps when starter code includes stream-heavy utilities.

---

## 13.6 — Scoped enumerations (enum classes)

### Scoped enum basics

`enum class` creates a scoped enumeration:

```cpp
enum class Campaign
{
    MutationA,
    MutationB,
    MutationC,
};
```

Enumerators are accessed through the enum type:

```cpp
Campaign campaign { Campaign::MutationA };
```

### Benefits over unscoped enums

| Feature | Unscoped enum | Scoped enum |
|---|---|---|
| Enumerator names leak into surrounding scope | Yes | No |
| Implicit conversion to int | Yes | No |
| Name collision risk | Higher | Lower |
| Type safety | Weaker | Stronger |

Example:

```cpp
enum class Color { red, green };
enum class TrafficLight { red, yellow, green };

Color c { Color::red };
TrafficLight t { TrafficLight::red };
```

No collision because each `red` is scoped.

### Converting to integer

Scoped enums do not implicitly convert to integers:

```cpp
// std::cout << Campaign::MutationA; // no built-in integer output
```

Use `static_cast` if you need the underlying value:

```cpp
auto value { static_cast<int>(Campaign::MutationA) };
```

### `using enum` preview

C++20 has `using enum` to bring scoped enumerators into scope:

```cpp
using enum Campaign;
Campaign c { MutationA };
```

Use sparingly. The explicit `Campaign::MutationA` spelling is often clearer.

Best default: prefer `enum class` over unscoped enum.

---

## 13.7 — Introduction to structs, members, and member selection

### Structs group related data

A `struct` defines a type with named members.

```cpp
struct CoveragePoint
{
    int line {};
    int column {};
};
```

Then create objects:

```cpp
CoveragePoint point { 27, 13 };
```

### Member access with `.`

```cpp
std::cout << point.line << ',' << point.column << '\n';
point.line = 28;
```

Diagram:

```
CoveragePoint point
+--------+--------+
| line   | column |
| 27     | 13     |
+--------+--------+
```

### Structs vs loose variables

Loose:

```cpp
int line { 27 };
int column { 13 };
```

Grouped:

```cpp
CoveragePoint location { 27, 13 };
```

Grouped data is easier to pass, return, store, and name.

### Non-static member initialization preview

#### Default member values

Struct members can have default initializers:

```cpp
struct CoveragePoint
{
    int line { 0 };
    int column { 0 };
};
```

This means a value-initialized object starts in a known state:

```cpp
CoveragePoint p {};
// p.line == 0, p.column == 0
```

Rule: member defaults are used when initialization does not provide a value for
that member.

#### Why defaults matter

Without defaults, default initialization can leave fundamental members
uninitialized in some contexts. Always giving members safe defaults prevents
garbage values.

---

## 13.8 — Struct aggregate initialization

### Data members are not automatically safe in every form

If you define:

```cpp
struct Point
{
    int x;
    int y;
};
```

Then some initialization forms can leave members uninitialized. Prefer default
member initializers or value/list initialization.

### Aggregates

A simple struct with public data members and no user-declared constructors is an
aggregate. Aggregates can be initialized with braces:

```cpp
Point p { 1, 2 };
```

Values initialize members in declaration order:

```
Point { 1, 2 }
        |  |
        v  v
        x  y
```

### Missing initializers

```cpp
Point p { 1 }; // x = 1, y value-initialized (0 for int)
```

If default member initializers exist, missing values use those defaults.

### Const structs

```cpp
const Point origin { 0, 0 };
// origin.x = 1; // error
```

A const struct object prevents modification of its members through that object.

### Designated initializers (C++20)

C++20 supports designated initializers:

```cpp
Point p { .x = 1, .y = 2 };
```

They improve readability, but fields must appear in declaration order in C++.

### Assignment with initializer list

```cpp
Point p {};
p = { 3, 4 };
```

This assigns new member values using aggregate-style syntax.

---

## 13.9 — Default member initialization

### Always provide default values for members

Best practice for simple structs:

```cpp
struct FuzzStats
{
    int testsRun {};
    int crashesFound {};
    double secondsElapsed {};
};
```

This gives a safe zero-ish default state.

### Explicit initializers override defaults

```cpp
struct Point
{
    int x { 0 };
    int y { 0 };
};

Point p { 5, 6 }; // x=5, y=6
```

### Missing values use defaults

```cpp
Point p { 5 }; // x=5, y=0
```

### Default initialization vs value initialization

```cpp
Point p1;  // default initialization
Point p2{}; // value/list initialization
```

With default member initializers, both are much safer. Prefer braces for clarity:

```cpp
Point p {};
```

---

## 13.10 — Passing and returning structs

### Passing structs

Small structs can be passed by value:

```cpp
void printPoint(CoveragePoint p);
```

Larger structs should usually be passed by const reference:

```cpp
void printStats(const FuzzStats& stats);
```

If the function modifies the struct:

```cpp
void resetStats(FuzzStats& stats);
```

### Temporary structs

You can pass a temporary aggregate:

```cpp
printPoint(CoveragePoint { 27, 13 });
```

### Returning structs

Returning structs by value is normal:

```cpp
CoveragePoint makePoint(int line, int column)
{
    return CoveragePoint { line, column };
}
```

Modern C++ handles many returns efficiently through copy elision/moves.

### Structs are building blocks

Structs let functions return more than one logical value without out parameters:

```cpp
struct ParseResult
{
    bool ok {};
    CoveragePoint point {};
};

ParseResult parseCoverageLine(std::string_view line);
```

This is clearer than mutating several reference output parameters.

---

## 13.11 — Struct miscellany

### Structs can contain program-defined members

```cpp
enum class Campaign
{
    MutationA,
    MutationB,
    MutationC,
};

struct FuzzResult
{
    Campaign campaign {};
    CoveragePoint location {};
    bool crashed {};
};
```

Types compose. This is how programs build domain models.

### Owners should own

If a struct owns data, its members should usually be owning types:

```cpp
struct SeedCorpus
{
    std::vector<std::string> seeds; // owns strings
};
```

Be cautious with raw pointers or `std::string_view` members because they do not
own the data they refer to. Those can be valid, but lifetime must be obvious.

Bad long-term storage:

```cpp
struct BadSeed
{
    std::string_view text; // may dangle if original string dies
};
```

Better if ownership is needed:

```cpp
struct Seed
{
    std::string text;
};
```

### Struct size and alignment

The compiler may insert padding between members so each member is properly
aligned in memory.

```cpp
struct Example
{
    char c; // 1 byte
    int i;  // 4 bytes, may require padding before it
};
```

Memory-ish view:

```
c pad pad pad i i i i
```

Member order can affect struct size. Usually prioritize clarity; optimize layout
only when size/performance actually matters.

---

## 13.12 — Member selection with pointers and references

### Struct object and reference: `.`

```cpp
CoveragePoint p { 27, 13 };
CoveragePoint& ref { p };

std::cout << p.line << '\n';
std::cout << ref.line << '\n';
```

References use the same `.` syntax as objects.

### Pointer: `->`

```cpp
CoveragePoint* ptr { &p };
std::cout << ptr->line << '\n';
```

`ptr->line` means:

```cpp
(*ptr).line
```

Use `->` for pointers because it is clearer and avoids parentheses.

### Chaining arrows

```cpp
node->next->value
```

Reads as:

```
node pointer
  -> next member, also pointer
      -> value member
```

### CS6340 decoder

```cpp
Module *M = F.getParent();
M->getOrInsertFunction(...);
```

`M` is a pointer, so use `->`.

```cpp
Instruction &I = ...;
I.getDebugLoc();
```

`I` is a reference, so use `.`.

---

## 13.13 — Class templates

### The problem templates solve

Suppose you want a pair of values:

```cpp
struct IntPair
{
    int first {};
    int second {};
};
```

Then you need `DoublePair`, `StringPair`, etc. That duplicates shape.

### Class template

A **class template** defines a pattern for generating class/struct types.

```cpp
template <typename T>
struct Pair
{
    T first {};
    T second {};
};
```

Use:

```cpp
Pair<int> p { 1, 2 };
Pair<double> d { 1.5, 2.5 };
```

Template mental model:

```
Pair<T> pattern
  T = int    -> Pair<int>
  T = double -> Pair<double>
```

### Multiple template parameters

```cpp
template <typename T, typename U>
struct Pair
{
    T first {};
    U second {};
};

Pair<int, std::string> item { 1, "one" };
```

### `std::pair`

The standard library already provides `std::pair<T, U>`:

```cpp
#include <utility>

std::pair<int, int> location { 27, 13 };
```

For domain meaning, a named struct is often clearer:

```cpp
CoveragePoint location { 27, 13 };
```

For generic utility code, `std::pair` is fine.

### Templates in multiple files

Templates usually need their full definitions visible where they are used, so
template definitions often live in headers. This is different from ordinary
non-template functions where declarations can live in headers and definitions in
`.cpp` files.

---

## 13.14 — Class template argument deduction (CTAD) and deduction guides

### CTAD

Class template argument deduction lets the compiler infer template arguments
from initializers.

```cpp
std::pair p { 1, 2.5 }; // deduces std::pair<int, double>
```

Without CTAD:

```cpp
std::pair<int, double> p { 1, 2.5 };
```

### Deduction guides

Some class templates need help telling the compiler how to deduce types.
Deduction guides provide that help. You probably will not write them soon, but
you may benefit from library-provided ones.

### CTAD limitations

CTAD does not work everywhere:

- not for function parameters,
- not reliably with all non-static member initialization patterns,
- not when there is not enough initializer information.

Be explicit when clarity matters:

```cpp
std::vector<int> values { 1, 2, 3 };
```

---

## 13.15 — Alias templates

### Alias template

An alias template is a templated type alias.

```cpp
template <typename T>
using Vec = std::vector<T>;

Vec<int> values { 1, 2, 3 };
```

More realistic:

```cpp
template <typename T>
using OptionalRef = std::optional<std::reference_wrapper<T>>;
```

Use alias templates to simplify repeated complex template types. Do not hide
types so aggressively that readers cannot tell what data structure is being used.

CS6340 possible helper:

```cpp
template <typename T>
using SeedMap = std::map<Campaign, std::vector<T>>;
```

---

## 13.x — Chapter 13 summary and quiz

### Core takeaways

- Program-defined types let the type system model your domain.
- Put reusable type definitions in headers.
- Unscoped enums leak enumerator names and convert to integers.
- Scoped enums (`enum class`) avoid name leakage and implicit integer conversion.
- Write helper functions to convert enums to/from strings.
- Operator overloads can make enums/structs printable, but are optional for
  learning code.
- Structs group related data under one type.
- Use `.` for objects/references and `->` for pointers.
- Give struct members default values.
- Aggregate initialization fills members in declaration order.
- Prefer returning structs over awkward out-parameter clusters.
- Structs that own data should use owning member types.
- Padding/alignment can make structs larger than the sum of members.
- Class templates define type patterns like `Pair<T>`.
- CTAD can infer template arguments from initializers.
- Alias templates simplify repeated templated type shapes.

### CS6340 type design examples

```cpp
enum class Campaign
{
    MutationA,
    MutationB,
    MutationC,
};

struct CoveragePoint
{
    int line {};
    int column {};
};

struct FuzzResult
{
    Campaign campaign {};
    std::string input {};
    bool crashed {};
};
```

This is better than passing around:

```cpp
int campaign;
int line;
int column;
std::string input;
bool crashed;
```

because the grouped version says what the values mean.

### Member access mini-map

```cpp
CoveragePoint point { 27, 13 };
CoveragePoint& ref { point };
CoveragePoint* ptr { &point };

point.line; // object
ref.line;   // reference
ptr->line;  // pointer
```

### Mini drill

```cpp
#include <iostream>
#include <optional>
#include <string>
#include <string_view>

enum class Campaign
{
    MutationA,
    MutationB,
    MutationC,
};

struct CoveragePoint
{
    int line {};
    int column {};
};

std::string_view toString(Campaign campaign)
{
    switch (campaign)
    {
    case Campaign::MutationA: return "MutationA";
    case Campaign::MutationB: return "MutationB";
    case Campaign::MutationC: return "MutationC";
    }

    return "Unknown";
}

void printCoverage(Campaign campaign, const CoveragePoint& point)
{
    std::cout << toString(campaign)
              << " reached "
              << point.line << ','
              << point.column << '\n';
}

int main()
{
    CoveragePoint point { 27, 13 };
    printCoverage(Campaign::MutationA, point);
}
```

What this reinforces:

- `enum class` for a small closed set of choices.
- `struct` for grouped domain data.
- `std::string_view` for returning long-lived string literals.
- `const CoveragePoint&` for read-only no-copy struct passing.
- `switch` over a scoped enum.

---

## 13.y — Using a language reference

LearnCpp includes this as a bridge from tutorial learning to reference-driven
learning. Tutorials explain concepts in a guided order; references are lookup
tools for exact rules, signatures, overloads, and library behavior.

### Tutorial vs reference

```text
tutorial:
  teaches a concept in sequence
  explains why the feature exists
  provides curated examples

reference:
  documents exact syntax and overloads
  lists constraints and edge cases
  helps when you already know what to look up
```

Both are useful, but they serve different moments.

### What to look for in a reference

When checking a C++ feature or standard library component, look for:

- required header
- namespace
- declaration/signature
- template parameters
- parameter meanings
- return type
- preconditions
- postconditions
- complexity notes
- invalidation rules
- examples
- language-standard version

Example for a standard library type:

```text
std::vector
  header: <vector>
  namespace: std
  key operations: push_back, size, operator[], at, begin/end
  invalidation: some operations can invalidate iterators/references
```

### Reading declarations

References often show declarations rather than friendly prose.

Example-style declaration:

```cpp
template <class T>
class vector;
```

Read this as:

```text
vector is a class template
T is the element type parameter
std::vector<int> means vector with T = int
```

Function declaration:

```cpp
std::size_t size() const noexcept;
```

Read:

- returns `std::size_t`
- takes no parameters
- can be called on const objects
- promises not to throw exceptions

### Why this matters for CS6340

LLVM and C++ standard-library code often require reference lookups.

For example, if you see:

```cpp
auto it { seeds.begin() };
```

and are unsure what `it` is, a reference can tell you:

```text
std::vector<T>::begin()
  returns iterator
  iterator points to first element
```

If you see an LLVM method you do not know, the same lookup habit applies:

```text
class name -> method name -> signature -> return type -> ownership/lifetime notes
```

### Practical lookup loop

```text
1. Identify the exact name.
2. Identify whether it is language syntax, standard library, or project API.
3. Find the declaration/signature.
4. Check required header/namespace.
5. Read return type and parameters.
6. Look for lifetime, ownership, invalidation, or error behavior.
7. Write a tiny local example if still uncertain.
```

This is how you move from "I need to memorize everything" to "I know how to
interrogate the toolchain and documentation."
