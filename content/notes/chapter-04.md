# Chapter 4 — Fundamental Data Types

> Source: <https://www.learncpp.com/> (Chapter 4)
>
> Goal for CS6340: understand the built-in C++ types well enough to avoid
> common lab bugs: signed/unsigned mistakes, integer overflow assumptions,
> accidental narrowing, floating-point equality traps, char/int confusion, and
> unclear casts.

## Contents

- [4.1 — Introduction to fundamental data types](#41--introduction-to-fundamental-data-types)
- [4.2 — Void](#42--void)
- [4.3 — Object sizes and the `sizeof` operator](#43--object-sizes-and-the-sizeof-operator)
- [4.4 — Signed integers](#44--signed-integers)
- [4.5 — Unsigned integers, and why to avoid them](#45--unsigned-integers-and-why-to-avoid-them)
- [4.6 — Fixed-width integers and `size_t`](#46--fixed-width-integers-and-size_t)
- [4.7 — Introduction to scientific notation](#47--introduction-to-scientific-notation)
- [4.8 — Floating point numbers](#48--floating-point-numbers)
- [4.9 — Boolean values](#49--boolean-values)
- [4.10 — Introduction to `if` statements](#410--introduction-to-if-statements)
- [4.11 — Chars](#411--chars)
- [4.12 — Introduction to type conversion and `static_cast`](#412--introduction-to-type-conversion-and-static_cast)
- [4.x — Chapter 4 summary](#4x--chapter-4-summary)

## 4.1 — Introduction to fundamental data types

C++ has fundamental data types built into the language.

These types are the raw material for objects and expressions:

| Category | Examples | Meaning |
| --- | --- | --- |
| Boolean | `bool` | true/false values |
| Character | `char`, `wchar_t`, `char8_t`, `char16_t`, `char32_t` | character/code unit storage |
| Integer | `short`, `int`, `long`, `long long` | whole numbers |
| Floating point | `float`, `double`, `long double` | approximate real numbers |
| Void | `void` | no value/type absence in certain contexts |
| Null pointer | `std::nullptr_t` | type of `nullptr` |

The most common beginner-level types:

```cpp
bool done { false };
char letter { 'A' };
int count { 42 };
double ratio { 0.5 };
```

### Types define interpretation

Memory is just bits. The type tells C++ how to interpret those bits.

```text
bits in memory:

01000001

as char: 'A'
as int-ish byte value: 65
as bool-ish nonzero: true
```

The same raw pattern can mean different things under different types.

### Objects have type, size, and value

```cpp
int x { 5 };
```

This creates an object with:

```text
name:  x
type:  int
size:  implementation-dependent, commonly 4 bytes
value: 5
```

The compiler uses the type to decide:

- how much storage is needed
- what operations are allowed
- how expressions involving the object behave
- how function overloads are selected
- what conversions may happen

### Integer vs floating point

`int` stores whole numbers:

```cpp
int branches { 12 };
```

`double` stores approximate real values:

```cpp
double coverageRatio { 0.875 };
```

Do not use floating-point types just because a number "might be big." Use them
when fractional values are meaningful.

### C++ type names are not all equally portable

The exact size of fundamental types can vary by implementation.

For example:

```text
int is at least 16 bits
long is at least 32 bits
long long is at least 64 bits
```

But "at least" is not "exactly."

If exact width matters, use fixed-width integer types from `<cstdint>` when
appropriate.

## 4.2 — Void

`void` means "no value" in several contexts.

### Void return type

A function with return type `void` does not return a value.

```cpp
void printStatus()
{
    std::cout << "running\n";
}
```

Call:

```cpp
printStatus();
```

You cannot use the call as a value:

```cpp
int x { printStatus() }; // invalid: printStatus returns no value
```

### Returning from a void function

A `void` function may use a bare `return;` to exit early.

```cpp
void printIfPositive(int x)
{
    if (x <= 0)
        return;

    std::cout << x << '\n';
}
```

No expression follows `return`.

### Void parameter list

In C++, an empty parameter list already means no parameters:

```cpp
int getValue()
{
    return 5;
}
```

You may see this in C-style code:

```cpp
int getValue(void)
{
    return 5;
}
```

In modern C++, prefer the empty parameter list.

### Void as an incomplete value type

You cannot create an object of type `void`:

```cpp
// void x; // invalid
```

`void` does not represent a storable value.

### Void pointers preview

Later C++ also has `void*`, a pointer to memory of unknown object type:

```cpp
void* ptr {};
```

That is not the same as a `void` object. It is a pointer type, and it has its own
rules. Modern C++ usually avoids raw `void*` unless interfacing with C-style APIs
or low-level systems.

## 4.3 — Object sizes and the `sizeof` operator

Objects occupy memory. `sizeof` tells you how many bytes a type or object uses.

```cpp
#include <iostream>

int main()
{
    std::cout << sizeof(int) << '\n';

    int x {};
    std::cout << sizeof(x) << '\n';

    return 0;
}
```

### Bytes and bits

A bit is a `0` or `1`.

A byte is the smallest addressable unit of memory. On modern systems, a byte is
almost always 8 bits.

```text
1 byte = 8 bits

byte:
bit index: 7 6 5 4 3 2 1 0
value:     0 1 0 0 0 0 0 1
```

### `sizeof` returns bytes

If `sizeof(int)` prints `4`, that means an `int` object uses 4 bytes.

```text
4 bytes * 8 bits/byte = 32 bits
```

Common sizes on many modern platforms:

| Type | Common size |
| --- | --- |
| `bool` | 1 byte |
| `char` | 1 byte |
| `short` | 2 bytes |
| `int` | 4 bytes |
| `long` | 4 or 8 bytes |
| `long long` | 8 bytes |
| `float` | 4 bytes |
| `double` | 8 bytes |

Do not hard-code these unless the standard or your platform contract guarantees
them.

### Size affects range

More bits allow more distinct values.

For an unsigned integer with `n` bits:

```text
number of values = 2^n
range = 0 through 2^n - 1
```

For an 8-bit unsigned integer:

```text
2^8 = 256 values
0 through 255
```

For a signed integer, some values represent negatives, so the positive maximum
is smaller than the unsigned maximum.

### `sizeof` does not evaluate some expressions

When used on an expression, `sizeof` usually inspects the type of the expression
without evaluating the expression.

```cpp
int x { 1 };

std::cout << sizeof(x++) << '\n';
std::cout << x << '\n'; // still 1
```

The expression `x++` is not executed here.

### `sizeof` result type

`sizeof` returns a value of type `std::size_t`, an unsigned type used for sizes.

This matters because mixing `std::size_t` with signed integers can produce
surprising conversions. That topic appears again in 4.6 and in the vector/array
chapters.

## 4.4 — Signed integers

Signed integers can represent negative, zero, and positive whole numbers.

Common signed integer types:

```cpp
short s {};
int i {};
long l {};
long long ll {};
```

### Prefer `int` for ordinary integer values

For most normal whole-number values:

```cpp
int count { 10 };
int score { -1 };
```

`int` is usually the natural integer type for the platform and is efficient.

Use a larger type when you need a larger range:

```cpp
long long totalBytes {};
```

### Signed range

For a typical 32-bit `int`, the range is roughly:

```text
-2,147,483,648 through 2,147,483,647
```

Exact ranges are available through `<limits>`:

```cpp
#include <iostream>
#include <limits>

int main()
{
    std::cout << std::numeric_limits<int>::min() << '\n';
    std::cout << std::numeric_limits<int>::max() << '\n';
}
```

### Signed integer overflow

If a signed integer goes outside its representable range, behavior is undefined.

```cpp
int x { std::numeric_limits<int>::max() };
++x; // undefined behavior
```

Undefined behavior means the C++ standard gives no guarantee about what happens.

Do not reason:

```text
max int + 1 must wrap to min int
```

That may happen on your machine, but the language does not promise it for signed
overflow.

### Integer division

Integer division drops the fractional part.

```cpp
int x { 7 / 2 }; // 3
```

It does not round to nearest.

If you need fractional results:

```cpp
double x { 7.0 / 2.0 }; // 3.5
```

or convert:

```cpp
int a { 7 };
int b { 2 };

double x { static_cast<double>(a) / b };
```

### Signed integer mental model

```text
good uses:
  counts that can go below zero
  differences
  sentinel values like -1
  ordinary arithmetic

danger:
  overflow is undefined
  mixing with unsigned can surprise you
```

## 4.5 — Unsigned integers, and why to avoid them

Unsigned integers represent only zero and positive values.

```cpp
unsigned int u { 5 };
```

For an 8-bit unsigned type:

```text
range: 0 through 255
```

### Unsigned wraparound

Unsigned arithmetic wraps modulo `2^n`.

If an 8-bit unsigned value is `255` and you add `1`, it becomes `0`.

```text
255 + 1 -> 0   for 8-bit unsigned
```

If it is `0` and you subtract `1`, it becomes `255`.

```text
0 - 1 -> 255   for 8-bit unsigned
```

This behavior is defined, unlike signed overflow.

### The wraparound trap

```cpp
unsigned int x { 0 };
--x;

std::cout << x << '\n'; // huge value, not -1
```

This is a classic source of bugs in loops and indexing.

### Signed/unsigned mixing

When signed and unsigned integers meet in an expression, C++ may convert the
signed value to unsigned.

```cpp
int s { -1 };
unsigned int u { 1 };

if (s < u)
{
    std::cout << "s is smaller\n";
}
```

This may not behave the way a human expects, because `s` can be converted to a
large unsigned value.

ASCII:

```text
s = -1       signed
u = 1        unsigned

comparison needs common type
    |
    v
-1 converted to unsigned
    |
    v
very large value
```

### Why not use unsigned to prevent negatives?

This seems tempting:

```cpp
unsigned int age {};
```

But unsigned does not prevent invalid logic. It changes arithmetic behavior.

```cpp
unsigned int age { 0 };
--age; // wraps to huge value
```

For user validation, use a signed type and check the range:

```cpp
int age {};

if (age < 0)
{
    std::cerr << "invalid age\n";
}
```

### When unsigned is appropriate

Unsigned types are useful for:

- bit manipulation
- modular arithmetic when wraparound is desired
- interacting with APIs that require unsigned types
- sizes and indices when using standard library types like `std::size_t`

But for general arithmetic, prefer signed integers unless you have a specific
reason.

### CS6340 warning

Lab code often touches sizes:

```cpp
std::string seed {};
auto n { seed.size() }; // std::size_t, unsigned
```

This can make loops and comparisons tricky:

```cpp
for (int i { 0 }; i < seed.size(); ++i) // signed/unsigned comparison warning
{
}
```

Handle the boundary consciously. Do not ignore the warning blindly.

## 4.6 — Fixed-width integers and `size_t`

The standard integer types have minimum sizes, not always exact sizes.

If exact width matters, use fixed-width types from `<cstdint>`.

```cpp
#include <cstdint>

std::int32_t x { 42 };
std::uint64_t y { 100 };
```

### Fixed-width types

Common fixed-width types:

| Type | Meaning |
| --- | --- |
| `std::int8_t` | signed 8-bit integer, if available |
| `std::int16_t` | signed 16-bit integer, if available |
| `std::int32_t` | signed 32-bit integer, if available |
| `std::int64_t` | signed 64-bit integer, if available |
| `std::uint8_t` | unsigned 8-bit integer, if available |
| `std::uint16_t` | unsigned 16-bit integer, if available |
| `std::uint32_t` | unsigned 32-bit integer, if available |
| `std::uint64_t` | unsigned 64-bit integer, if available |

Use fixed-width integers when file formats, network protocols, binary layouts,
or exact bit widths matter.

### `int8_t` and `uint8_t` caveat

`std::int8_t` and `std::uint8_t` may behave like character types in some output
contexts because they are often aliases of `signed char` and `unsigned char`.

```cpp
std::uint8_t byte { 65 };
std::cout << byte << '\n'; // may print 'A' instead of 65
```

Cast when you want numeric output:

```cpp
std::cout << static_cast<int>(byte) << '\n';
```

### Fast and least types

`<cstdint>` also provides:

```cpp
std::int_least32_t
std::int_fast32_t
```

`least` means at least that many bits, choosing the smallest available type.

`fast` means at least that many bits, choosing a type likely to be fast.

For most course code:

- use `int` for ordinary numbers
- use `std::int64_t` when you need a large exact signed range
- use fixed-width unsigned types for bytes/bit patterns

### `std::size_t`

`std::size_t` is an unsigned integer type used for object sizes and many
container/string lengths.

Examples:

```cpp
sizeof(int)             // returns std::size_t
std::string{}.size()    // returns std::size_t
std::vector<int>{}.size() // returns size type, often std::size_t
```

Printing:

```cpp
std::size_t length { seed.size() };
std::cout << length << '\n';
```

### Why `size_t` causes friction

This can be dangerous:

```cpp
std::size_t length { 0 };
std::cout << length - 1 << '\n'; // wraps to huge value
```

Because `length` is unsigned, subtracting below zero wraps.

Common bug:

```cpp
for (std::size_t i { length - 1 }; i >= 0; --i)
{
    // infinite loop: i >= 0 is always true for unsigned i
}
```

Correct reverse loop patterns require care:

```cpp
for (std::size_t i { length }; i-- > 0; )
{
    // use i
}
```

or use a signed index after checking/casting consciously.

### Type choice table

| Situation | Reasonable type |
| --- | --- |
| ordinary count/math | `int` |
| very large signed count | `long long` or `std::int64_t` |
| exact 32-bit signed field | `std::int32_t` |
| raw byte | `std::uint8_t` or `std::byte` |
| container size result | `std::size_t` or container's size type |
| difference between positions | signed difference type, often `std::ptrdiff_t` |

## 4.7 — Introduction to scientific notation

Scientific notation represents numbers as:

```text
significand * 10^exponent
```

Examples:

```text
1.23e4  = 1.23 * 10^4  = 12300
5.0e-3  = 5.0 * 10^-3  = 0.005
```

C++ supports `e` notation for floating-point literals:

```cpp
double large { 1.2e6 };  // 1,200,000
double small { 3.5e-4 }; // 0.00035
```

### Why scientific notation matters

It is compact for very large or small values:

```text
0.000000001  -> 1e-9
1000000000   -> 1e9
```

It also appears in program output:

```text
1.23456e+06
```

That means:

```text
1.23456 * 10^6
```

### Normalized scientific notation

Usually the significand is written with one nonzero digit before the decimal:

```text
3.14e2   normalized
31.4e1   same value, not normalized
```

Floating-point hardware uses a binary version of this idea internally.

## 4.8 — Floating point numbers

Floating-point types represent approximate real numbers.

Common types:

```cpp
float f {};
double d {};
long double ld {};
```

Prefer `double` for ordinary floating-point work.

### Floating point is approximate

Many decimal fractions cannot be represented exactly in binary.

```cpp
double x { 0.1 };
```

The stored value is very close to `0.1`, but not exactly mathematical one-tenth.

This can produce surprising output:

```cpp
double sum { 0.1 + 0.2 };
std::cout << std::setprecision(17) << sum << '\n';
```

You may see something like:

```text
0.30000000000000004
```

### Precision vs range

Floating-point types trade exactness for range.

They can represent extremely large and small magnitudes, but with limited
significant digits.

```text
integer:
  exact within range
  no fractions

floating point:
  fractions and huge/small magnitudes
  approximate
```

### Rounding errors accumulate

```cpp
double total {};

for (int i { 0 }; i < 10; ++i)
{
    total += 0.1;
}
```

Mathematically, total should be `1.0`.

In floating point, it may be slightly off.

### Do not compare most floating-point values with `==`

Bad:

```cpp
if (total == 1.0)
{
    std::cout << "exactly one\n";
}
```

Better:

```cpp
#include <cmath>

bool nearlyEqual(double a, double b, double epsilon)
{
    return std::abs(a - b) <= epsilon;
}
```

Use:

```cpp
if (nearlyEqual(total, 1.0, 1e-9))
{
    std::cout << "close enough\n";
}
```

For CS6340, exact integer metrics like branch counts should stay integers.
Floating-point ratios like coverage percentages should be treated as
approximate.

### Special values

Floating-point systems often support:

- positive infinity
- negative infinity
- NaN, "not a number"

Examples that may produce special values:

```cpp
double inf { 1.0 / 0.0 };
double nan { 0.0 / 0.0 };
```

Do not rely on these for normal control flow. Validate inputs before dangerous
operations.

### Output precision

Default output may hide precision issues.

```cpp
#include <iomanip>
#include <iostream>

double x { 1.0 / 3.0 };

std::cout << x << '\n';
std::cout << std::setprecision(17) << x << '\n';
```

`std::setprecision` affects how many significant digits are printed.

## 4.9 — Boolean values

`bool` represents truth values:

```cpp
bool done { false };
bool valid { true };
```

Boolean values are used in conditions:

```cpp
if (valid)
{
    std::cout << "valid\n";
}
```

### Boolean output

By default:

```cpp
std::cout << true << '\n';  // 1
std::cout << false << '\n'; // 0
```

Use `std::boolalpha`:

```cpp
std::cout << std::boolalpha;
std::cout << true << '\n';  // true
std::cout << false << '\n'; // false
```

### Boolean input

By default, `std::cin >> boolVariable` expects `0` or `1`.

With `std::boolalpha`, it expects textual `true` or `false`.

### Naming boolean variables

Good boolean names read like yes/no questions:

```cpp
bool isEmpty {};
bool hasCrash {};
bool shouldStop {};
bool foundTarget {};
```

Avoid vague names:

```cpp
bool data {}; // unclear
```

### Boolean conversions

Integers convert to bool:

```text
0 -> false
nonzero -> true
```

This is why:

```cpp
if (x)
{
}
```

means:

```text
if x is nonzero
```

For clarity, prefer explicit comparisons when the condition is not inherently
boolean:

```cpp
if (count != 0)
{
}
```

## 4.10 — Introduction to `if` statements

An `if` statement conditionally executes code.

```cpp
if (condition)
{
    statement;
}
```

If the condition is true, the body runs.

If false, the body is skipped.

ASCII:

```text
condition?
  |
  +-- true  -> run body -> continue
  |
  +-- false -> skip body -> continue
```

### Example

```cpp
int x { 5 };

if (x > 0)
{
    std::cout << "positive\n";
}
```

The condition:

```cpp
x > 0
```

evaluates to a boolean result.

### Single statement vs block

Without braces, only one statement belongs to the `if`:

```cpp
if (x > 0)
    std::cout << "positive\n";
std::cout << "done\n";
```

`"done"` always prints.

Prefer braces:

```cpp
if (x > 0)
{
    std::cout << "positive\n";
}

std::cout << "done\n";
```

This avoids visual lies from indentation.

### If/else

```cpp
if (x >= 0)
{
    std::cout << "nonnegative\n";
}
else
{
    std::cout << "negative\n";
}
```

Exactly one branch runs.

### Chained conditions

```cpp
if (score >= 90)
{
    std::cout << "A\n";
}
else if (score >= 80)
{
    std::cout << "B\n";
}
else
{
    std::cout << "lower\n";
}
```

Conditions are checked top to bottom until one is true.

### CS6340 tie-in

Coverage analysis cares about branches. Every `if` creates possible control-flow
paths.

```text
if condition
  true branch
  false branch
```

Testing both branches matters.

## 4.11 — Chars

`char` stores a single character-sized value.

```cpp
char letter { 'A' };
```

Character literals use single quotes:

```cpp
'A'
'\n'
'0'
```

String literals use double quotes:

```cpp
"A"
"hello"
```

These are different:

```text
'A'  -> char
"A"  -> string literal containing A and null terminator
```

### Character values are numeric underneath

Common character encodings assign integer codes to characters.

In ASCII-compatible encodings:

```text
'A' -> 65
'a' -> 97
'0' -> 48
'\n' -> 10
```

You can see the numeric value:

```cpp
char ch { 'A' };
std::cout << static_cast<int>(ch) << '\n'; // often 65
```

### Escape sequences

Some characters are written with escapes:

| Escape | Meaning |
| --- | --- |
| `\n` | newline |
| `\t` | tab |
| `\\` | backslash |
| `\"` | double quote |
| `\'` | single quote |
| `\0` | null character |

Example:

```cpp
std::cout << "line 1\nline 2\n";
```

### `char` signedness

Plain `char` may be signed or unsigned depending on the implementation.

If you need a small integer, do not use plain `char`.

Use:

```cpp
signed char
unsigned char
std::int8_t
std::uint8_t
```

depending on intent.

If you need text, `char` is fine.

### Character input

```cpp
char ch {};
std::cin >> ch;
```

Formatted extraction skips leading whitespace and reads one non-whitespace
character.

To read whitespace characters too, use lower-level input functions such as
`std::cin.get(ch)`, covered later in I/O.

## 4.12 — Introduction to type conversion and `static_cast`

Type conversion changes a value from one type to another.

### Implicit conversion

C++ performs some conversions automatically.

```cpp
double d { 5 }; // int 5 converted to double 5.0
```

```cpp
int x {};
if (x) // int converted to bool
{
}
```

Implicit conversions can be convenient, but they can also hide information loss.

### Narrowing conversion

A narrowing conversion can lose data or range.

```cpp
int x { 3.5 }; // list initialization rejects this
```

The fractional part would be lost.

Assignment-style initialization may allow more conversions, sometimes with
warnings:

```cpp
int x = 3.5; // x becomes 3, but this is a bad habit
```

Prefer brace initialization because it catches narrowing.

### Explicit conversion with `static_cast`

Use `static_cast` when you want to say:

```text
I know this conversion is happening.
```

Example:

```cpp
int total { 7 };
int count { 2 };

double average { static_cast<double>(total) / count };
```

Without the cast:

```cpp
double average { total / count }; // integer division first: 3.0
```

With the cast:

```text
total -> double
double / int -> double division
7.0 / 2 -> 3.5
```

### Char to int

```cpp
char ch { 'A' };
std::cout << static_cast<int>(ch) << '\n';
```

This prints the numeric code rather than the character.

### Casts do not change the original object

```cpp
int x { 5 };
double d { static_cast<double>(x) };
```

`x` is still an `int`. The cast creates a converted value used in the expression.

ASCII:

```text
x object:
  type int
  value 5

static_cast<double>(x):
  temporary converted value
  type double
  value 5.0
```

### Avoid C-style casts

Avoid:

```cpp
double d { (double)x };
```

Prefer:

```cpp
double d { static_cast<double>(x) };
```

`static_cast` is more visible, searchable, and constrained.

### Conversion and Lab 1

Common conversion pressure points:

```cpp
std::size_t length { seed.size() };
int index { getRandomIndex() };
```

If you need to use `index` to index `seed`, validate first:

```cpp
if (index >= 0 && static_cast<std::size_t>(index) < seed.size())
{
    char ch { seed[static_cast<std::size_t>(index)] };
}
```

The cast is placed after the nonnegative check. Casting a negative `int` to
`std::size_t` produces a huge unsigned value.

## 4.x — Chapter 4 summary

- Fundamental types are built into C++.
- Types determine storage, allowed operations, interpretation, and conversions.
- `void` means no value in contexts such as function return types.
- `sizeof` returns the size of a type/object in bytes.
- `sizeof` returns `std::size_t`.
- Signed integers represent negative, zero, and positive whole numbers.
- Signed integer overflow is undefined behavior.
- Integer division truncates fractional parts.
- Unsigned integers wrap modulo `2^n`.
- Unsigned wraparound is defined but often surprising.
- Avoid unsigned for ordinary arithmetic unless you specifically need unsigned
  behavior.
- Fixed-width integer types live in `<cstdint>`.
- `std::size_t` is an unsigned type used for sizes.
- Scientific notation writes values as a significand times a power of ten.
- Floating-point values are approximate and should usually not be compared with
  exact equality.
- `bool` stores `true` or `false`.
- `if` statements execute code conditionally and create control-flow branches.
- `char` stores character-sized values; character literals use single quotes.
- Escape sequences represent special characters such as newline and tab.
- Type conversions can be implicit or explicit.
- `static_cast` is the preferred explicit cast for ordinary checked-at-compile
  time conversions.
- Validate before casting signed values to unsigned types.

