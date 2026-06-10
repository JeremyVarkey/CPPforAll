# Chapter 4 — Fundamental Data Types

> Source: <https://www.learncpp.com/> (Chapter 4)

This is the chapter where numbers stop being innocent. So far you have treated
`int` as "a number" and moved on. But C++ does not have one kind of number — it
has a whole family of numeric types, each with its own size, its own range, and
its own personality. Choose the wrong one and your program will compile cleanly,
run happily, and quietly give you the wrong answer: a counter that wraps to zero,
a sum of dimes that refuses to equal a dollar, an index that turns negative and
then enormous.

This chapter is about meeting those types deliberately, before they surprise you.
By the end you will know how big each type is, what range it can hold, what
happens at the edges, and how to convert between types *on purpose* instead of by
accident.

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

Every object in your program lives in memory, and memory is nothing but bits —
on/off switches, `0`s and `1`s. A bit pattern by itself means nothing. The
sequence `01000001` is just eight switches. What turns those switches into the
letter `'A'`, or the number `65`, or the truth value `true`, is the object's
**type**. The type is the lens through which C++ reads the bits.

C++ ships with a set of **fundamental data types** built directly into the
language. You do not include a header to use them; they are always there. They
are the raw material from which everything else — strings, vectors, your own
classes — is eventually built.

| Category | Examples | Meaning |
| --- | --- | --- |
| Boolean | `bool` | true/false values |
| Character | `char`, `wchar_t`, `char8_t`, `char16_t`, `char32_t` | character / code-unit storage |
| Integer | `short`, `int`, `long`, `long long` | whole numbers |
| Floating point | `float`, `double`, `long double` | approximate real numbers |
| Void | `void` | "no value" in certain contexts |
| Null pointer | `std::nullptr_t` | the type of `nullptr` |

For now you will spend almost all your time with four of them:

```cpp
bool done { false };
char letter { 'A' };
int count { 42 };
double ratio { 0.5 };
```

### The type decides how the bits are read

Here is the central idea, made concrete. Suppose memory holds this one byte:

```text
bits in memory:
    01000001

read as a char:  'A'
read as an int:  65
read as a bool:  true   (any nonzero pattern is true)
```

The bits never changed. Only the interpretation did. This is why type matters so
much in C++: the same storage means different things depending on the type you
declared. Get the type right and the bits tell the truth; get it wrong and they
lie convincingly.

> **Key insight:** A type is not a label you stick on a value for documentation.
> It is the instruction the compiler follows to interpret bits, choose
> operations, and decide what conversions are legal.

### Every object has a type, a size, and a value

When you write:

```cpp
int x { 5 };
```

you create an object that the compiler tracks with several properties:

```text
name:  x
type:  int
size:  implementation-defined, commonly 4 bytes
value: 5
```

From the type, the compiler works out:

- how much storage to reserve,
- which operations are allowed (you can divide two `int`s; you cannot divide two `bool`s meaningfully),
- how expressions involving the object behave,
- which function overload to call when several are available,
- which conversions may quietly happen.

That is a lot of decisions riding on one keyword. Choosing types thoughtfully is
not pedantry — it is how you steer the compiler.

### Whole numbers vs real numbers

The first fork in the road is *integer* vs *floating point*.

Use an integer type like `int` when the quantity is a whole number — a count, an
index, a year:

```cpp
int branches { 12 };
```

Use a floating-point type like `double` when fractional values are meaningful — a
ratio, a measurement, a price-per-unit:

```cpp
double coverageRatio { 0.875 };
```

A common beginner reflex is to reach for `double` whenever a number "might get
big." Resist it. Floating point buys range by giving up exactness (you will see
exactly how in 4.8). Pick floating point because you genuinely need fractions,
not as a hedge against large values — for that, there are larger integer types.

### "At least," not "exactly"

One uncomfortable truth about C++: the standard does not nail down the exact size
of most fundamental types. It guarantees *minimums*:

```text
int        is at least 16 bits
long       is at least 32 bits
long long  is at least 64 bits
```

"At least 16 bits" is not "exactly 16 bits." On the machine in front of you,
`int` is very likely 32 bits — but the language does not promise it, and code
that assumes an exact width can break when moved to another platform. When the
exact bit-width genuinely matters (file formats, network protocols), C++ offers
**fixed-width integer types** from `<cstdint>`, which we reach in 4.6.

> **Note:** This is your first taste of a recurring C++ theme: the language
> trades a little portability guarantee for a lot of performance flexibility,
> letting each platform pick the type sizes that run fastest on its hardware.

## 4.2 — Void

`void` is the odd one out. Every other fundamental type describes a kind of
value; `void` describes the *absence* of a value. It is C++'s way of saying
"nothing here." You meet it in a few distinct roles.

### A function that returns nothing

The most common use: a function whose job is to *do* something, not to *compute*
something, declares its return type as `void`.

```cpp
void printStatus()
{
    std::cout << "running\n";
}
```

You call it as a standalone statement:

```cpp
printStatus();
```

Because it yields no value, you cannot use the call where a value is expected:

```cpp
int x { printStatus() }; // error: printStatus() has no value to give x
```

There is simply nothing for `x` to be initialized from.

### Leaving a void function early

A `void` function can still use `return` — but with no expression after it. A
bare `return;` means "stop here and go back to the caller."

```cpp
void printIfPositive(int x)
{
    if (x <= 0)
        return;          // bail out early; nothing follows return

    std::cout << x << '\n';
}
```

This is an early exit, not a returned value. The keyword is the same; the meaning
is "I'm done," not "here is your answer."

### An empty parameter list already means "no parameters"

In C++, a function that takes nothing is written with empty parentheses:

```cpp
int getValue()
{
    return 5;
}
```

You may run into older, C-style code that writes `void` inside the parentheses to
say the same thing:

```cpp
int getValue(void) // C-style; means the same as ()
{
    return 5;
}
```

Both compile, but in modern C++ the empty list is the idiom.

> **Best practice:** Use an empty parameter list `()` to declare a function that
> takes no parameters. Reserve `void` for return types.

### You cannot make a `void` object

Because `void` represents no value, you cannot declare a variable of type `void`:

```cpp
// void x; // error: cannot create an object that holds no value
```

There is nothing to store, so there is nothing to declare.

### A glimpse ahead: `void*`

Much later you will meet `void*`, a *pointer* to memory of unknown object type:

```cpp
void* ptr {};
```

Do not confuse this with a `void` object. `void*` is a genuine, storable pointer
type with its own rules; the `void` in it just means "I won't tell you what's at
the other end." Modern C++ rarely needs raw `void*` outside of low-level or
C-interfacing code, so file it away as "exists, mostly avoid" for now.

## 4.3 — Object sizes and the `sizeof` operator

Objects take up room. How much room a type needs is something you can ask the
compiler directly, using the **`sizeof`** operator. `sizeof` reports the size of
a type or object in *bytes*.

```cpp
#include <iostream>

int main()
{
    std::cout << sizeof(int) << '\n'; // size of the type int

    int x {};
    std::cout << sizeof(x) << '\n';   // size of the object x

    return 0;
}
```

You can hand `sizeof` either a type name (in parentheses) or an expression.

### Bits and bytes

A quick vocabulary check, because the rest of this chapter leans on it:

- A **bit** is a single binary digit: `0` or `1`.
- A **byte** is the smallest individually addressable chunk of memory. On
  essentially every modern system, a byte is 8 bits.

```text
1 byte = 8 bits

bit index: 7 6 5 4 3 2 1 0
value:     0 1 0 0 0 0 0 1
```

So when `sizeof(int)` prints `4`, that `int` occupies 4 bytes, which is:

```text
4 bytes × 8 bits/byte = 32 bits
```

### Typical sizes — but don't hard-code them

On many mainstream desktop platforms you will see these sizes:

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

Treat this as "what you'll usually see," not "what the standard guarantees."
Notice `long` already varies. If your logic depends on an exact size, measure it
with `sizeof` or use a fixed-width type (4.6) — never bake a magic number like `4`
into your reasoning and hope.

### More bits means a wider range

Size and range are two views of the same fact: the more bits a type has, the more
distinct values it can represent. For an **unsigned** integer with `n` bits:

```text
number of representable values = 2^n
range                          = 0 through 2^n − 1
```

So an 8-bit unsigned integer can hold:

```text
2^8 = 256 distinct values
range: 0 through 255
```

A **signed** integer spends some of its representable values on negatives, so its
largest positive value is correspondingly smaller. We unpack signed and unsigned
ranges properly in 4.4 and 4.5.

### `sizeof` looks at the type, not the result

Here is a subtlety worth knowing now, because it surprises people. When you apply
`sizeof` to an expression, it inspects the *type* of that expression — it does not
actually run it.

```cpp
int x { 1 };

std::cout << sizeof(x++) << '\n'; // prints sizeof(int); x++ is NOT executed
std::cout << x << '\n';           // still 1 — the increment never happened
```

`sizeof` only needs to know the type of `x++`, which is `int`. It can answer
without evaluating the expression, so the side effect (`++`) never occurs.

### `sizeof` gives back a `std::size_t`

The value `sizeof` produces has type **`std::size_t`** — an *unsigned* integer
type used throughout the standard library for sizes and counts. That detail
matters: as you will see in 4.5 and 4.6, mixing an unsigned `std::size_t` with
ordinary signed integers can trigger surprising conversions. Keep it in the back
of your mind; it returns in the chapters on strings, vectors, and arrays.

## 4.4 — Signed integers

A **signed** integer can represent negative numbers, zero, and positive numbers.
This is what most people mean by "an integer," and it is the default behavior of
every plain integer type in C++. The signed integer family:

```cpp
short s {};
int i {};
long l {};
long long ll {};
```

They differ only in their guaranteed minimum range — `short` ≤ `int` ≤ `long` ≤
`long long`.

### Reach for `int` first

For ordinary whole-number values — counts, scores, offsets — `int` is the right
default. It is typically the size the platform handles most efficiently, and it
has plenty of range for everyday work.

```cpp
int count { 10 };
int score { -1 };
```

Step up to a wider type only when you actually need a bigger range:

```cpp
long long totalBytes {};
```

> **Best practice:** Use `int` for ordinary integer arithmetic. Move to a larger
> signed type only when you have a concrete reason to expect values beyond an
> `int`'s range.

### Knowing the range exactly

For a typical 32-bit `int`, the range is roughly:

```text
−2,147,483,648  through  +2,147,483,647
```

Rather than memorizing that, ask the library. `<limits>` reports the exact
boundaries for any numeric type on your platform:

```cpp
#include <iostream>
#include <limits>

int main()
{
    std::cout << std::numeric_limits<int>::min() << '\n';
    std::cout << std::numeric_limits<int>::max() << '\n';
}
```

This is the honest, portable way to discover a type's limits — it adapts to
whatever platform you compile on.

### Signed overflow is undefined behavior

What happens when you push a signed integer past its maximum? The intuitive guess
is "it wraps around to the minimum." **Do not rely on that.** When a signed
integer goes outside its representable range, the C++ standard declares the result
**undefined behavior** — the language makes *no* promise about what happens next.

```cpp
int x { std::numeric_limits<int>::max() };
++x; // undefined behavior — anything is permitted to happen
```

Undefined behavior is not the same as "an unpredictable but bounded value." It
means the standard washes its hands entirely: your program might wrap, might
crash, might let the compiler optimize the surrounding code in ways that look
nonsensical. So this reasoning is a trap:

```text
"max int + 1 must wrap to min int"   ← NOT guaranteed for signed types
```

It might do exactly that on your machine today. The language still does not
promise it, which means you cannot build correct programs on top of it.

> **Warning:** Never depend on signed integer overflow "wrapping." It is
> undefined behavior. If you need values to stay in range, check before you
> compute, or choose a wider type.

### Integer division truncates

When you divide one integer by another, the result is an integer — the fractional
part is simply discarded (truncated toward zero), not rounded.

```cpp
int x { 7 / 2 }; // 3, not 3.5 and not 4
```

`7 / 2` is mathematically `3.5`, but integer division throws away the `.5`. If you
want the fraction, at least one operand must be floating point:

```cpp
double x { 7.0 / 2.0 }; // 3.5
```

When the values live in `int` variables, convert one of them explicitly (the cast
is your topic for 4.12):

```cpp
int a { 7 };
int b { 2 };

double x { static_cast<double>(a) / b }; // 3.5 — a becomes double, so the division is floating point
```

> **Warning:** A very common bug is writing `double avg { total / count };` where
> both operands are `int`. The integer division happens *first*, truncating, and
> only then is the already-wrong result widened to `double`. Convert one operand
> *before* dividing.

### When signed integers shine

```text
good uses:
  counts that can legitimately go below zero
  differences (a − b, which may be negative)
  sentinel values like −1 ("not found")
  ordinary day-to-day arithmetic

watch out for:
  overflow past the max or min is undefined
  mixing with unsigned types can flip a comparison (see 4.5)
```

## 4.5 — Unsigned integers, and why to avoid them

An **unsigned** integer gives up the ability to be negative. It represents only
zero and positive values — and in exchange, it roughly doubles its positive range,
since none of its bit patterns are spent on negatives.

```cpp
unsigned int u { 5 };
```

For an 8-bit unsigned type, the range is:

```text
0 through 255
```

That sounds harmless, even useful. The trouble is what happens at the edges.

### Unsigned arithmetic wraps around

When an unsigned value would go below `0` or above its maximum, it does not
overflow into undefined behavior the way signed types do. Instead it **wraps**,
modulo `2^n` — it cycles around like an odometer.

```text
for an 8-bit unsigned value:

255 + 1  →  0       (rolled over the top)
  0 − 1  →  255     (rolled under the bottom)
```

Unlike signed overflow, this behavior is fully *defined*: the standard guarantees
the wrap. That is precisely what makes it dangerous — it does not crash or warn.
It silently produces a number that is mathematically absurd but technically legal.

### The classic trap

```cpp
unsigned int x { 0 };
--x;

std::cout << x << '\n'; // NOT −1 — a huge value (4294967295 for 32-bit unsigned)
```

You wanted "one less than zero." Unsigned has no concept of "less than zero," so
it wrapped to the top of its range instead. This bites people constantly in
loops and array indexing, where a counter dips below zero and suddenly the loop
runs four billion more times than intended.

### Mixing signed and unsigned

It gets worse when the two kinds meet in one expression. When a signed and an
unsigned integer appear together, C++ often converts the *signed* value to
unsigned to find a common type — and a negative signed value becomes an enormous
unsigned one.

```cpp
int s { -1 };
unsigned int u { 1 };

if (s < u)
{
    std::cout << "s is smaller\n"; // you'd expect this to print...
}
```

It might not print. To compare `s` and `u`, C++ needs a common type, and it picks
unsigned:

```text
s = −1  (signed)        u = 1  (unsigned)

         comparison needs a common type
                     │
                     ▼
         −1 is converted to unsigned
                     │
                     ▼
         a very large value (e.g. 4294967295)

         so "−1 < 1" can become "4294967295 < 1" → false
```

The comparison you wrote is not the comparison that runs. A good compiler will
warn you about this — heed the warning.

### Don't use unsigned just to "forbid negatives"

It is tempting to reason: "an age can't be negative, so I'll make it unsigned."

```cpp
unsigned int age {};
```

But unsigned does not *prevent* invalid logic — it *changes the arithmetic* in a
way that hides bugs. A stray subtraction does not produce an error; it produces a
gigantic positive number:

```cpp
unsigned int age { 0 };
--age; // wraps to a huge value, no warning, no crash
```

The honest approach is to use a signed type and validate the range yourself, so a
bad value is *visible* rather than disguised:

```cpp
int age {};

if (age < 0)
{
    std::cerr << "invalid age\n"; // a negative age can be detected and rejected
}
```

### When unsigned is the right tool

Unsigned types are not forbidden — they have legitimate jobs:

- bit manipulation, where you care about individual bits,
- modular arithmetic, where wraparound is *exactly* what you want,
- interfacing with APIs that require unsigned arguments,
- sizes and indices via standard-library types like `std::size_t` (4.6).

> **Best practice:** Prefer signed integers for general-purpose arithmetic. Reach
> for unsigned only when you specifically want bit-level, modular, or
> size/index semantics — not merely to express "this shouldn't be negative."

### A heads-up for the lab

Standard-library sizes are unsigned, and that will reach into your own loops:

```cpp
std::string seed {};
auto n { seed.size() }; // n is std::size_t — unsigned
```

So a perfectly natural-looking loop can trigger a signed/unsigned comparison
warning:

```cpp
for (int i { 0 }; i < seed.size(); ++i) // signed i vs unsigned seed.size()
{
}
```

Don't paper over that warning — understand the boundary. In this chapter's lab,
one task hands you exactly this situation (a signed index meeting an unsigned
length) and asks you to convert *consciously*, after checking the sign. That
guard is the whole lesson; 4.12 shows the pattern.

## 4.6 — Fixed-width integers and `size_t`

Recall from 4.3 that the standard integer types come with *minimum* sizes, not
exact ones — an `int` is "at least 16 bits," a `long` "at least 32 bits." Most of
the time that flexibility is fine. But sometimes you need to know *exactly* how
many bits you are working with: when you read a binary file format, parse a
network packet, or lay out data that another program will read byte-for-byte.

For those cases, `<cstdint>` provides **fixed-width integer types** whose size is
spelled out in the name:

```cpp
#include <cstdint>

std::int32_t x { 42 };  // exactly 32 bits, signed
std::uint64_t y { 100 }; // exactly 64 bits, unsigned
```

### The fixed-width family

| Type | Meaning |
| --- | --- |
| `std::int8_t` | signed 8-bit integer (if available) |
| `std::int16_t` | signed 16-bit integer (if available) |
| `std::int32_t` | signed 32-bit integer (if available) |
| `std::int64_t` | signed 64-bit integer (if available) |
| `std::uint8_t` | unsigned 8-bit integer (if available) |
| `std::uint16_t` | unsigned 16-bit integer (if available) |
| `std::uint32_t` | unsigned 32-bit integer (if available) |
| `std::uint64_t` | unsigned 64-bit integer (if available) |

The "if available" caveat is real: these exact-width types are guaranteed only on
platforms that can provide them. In practice the 8/16/32/64-bit versions exist
nearly everywhere you will compile, which is why the lab can safely assert that
`sizeof(std::int32_t)` is `4` bytes on every platform that has the type.

### The `int8_t` / `uint8_t` gotcha

There is one trap worth flagging. On most platforms, `std::int8_t` and
`std::uint8_t` are aliases for `signed char` and `unsigned char` — they *are*
character types under the hood. So streaming one to `std::cout` may print a
*character*, not a number:

```cpp
std::uint8_t byte { 65 };
std::cout << byte << '\n'; // may print 'A' (code 65), not 65
```

When you want the numeric value printed, convert to `int` first:

```cpp
std::cout << static_cast<int>(byte) << '\n'; // prints 65
```

> **Warning:** `std::int8_t` and `std::uint8_t` often behave like `char` in I/O.
> If you want to see the number, `static_cast` it to `int` before printing.

### "least" and "fast" variants

For completeness: `<cstdint>` also offers types that relax the *exact*-width
requirement.

```cpp
std::int_least32_t a {}; // smallest type with AT LEAST 32 bits
std::int_fast32_t  b {}; // a type with at least 32 bits, chosen to be fast
```

`least` minimizes storage; `fast` favors speed. You rarely need them as a
beginner. The practical guidance for course work:

- use `int` for ordinary numbers,
- use `std::int64_t` when you need a large signed range with an exact width,
- use fixed-width *unsigned* types for raw bytes and bit patterns.

### `std::size_t`, the size type

You met `std::size_t` in 4.3 as the result type of `sizeof`. It is an unsigned
integer type the standard library uses for object sizes and for the lengths of
containers and strings:

```cpp
sizeof(int)                 // yields a std::size_t
std::string{}.size()        // yields a std::size_t
std::vector<int>{}.size()   // yields a size type, commonly std::size_t
```

Printing one is unremarkable:

```cpp
std::size_t length { seed.size() };
std::cout << length << '\n';
```

### Why `size_t` causes friction

Because `std::size_t` is *unsigned*, it inherits all the wraparound hazards from
4.5. Subtracting below zero does not give you a negative number — it gives you a
gigantic one:

```cpp
std::size_t length { 0 };
std::cout << length - 1 << '\n'; // huge value, not −1
```

This makes the obvious "count down to zero" loop an infinite loop, because an
unsigned value is *always* `>= 0` — the condition can never become false:

```cpp
for (std::size_t i { length - 1 }; i >= 0; --i) // BUG: i >= 0 is always true
{
    // never terminates
}
```

A correct reverse loop over an unsigned index needs a different shape — one that
tests *before* it wraps:

```cpp
for (std::size_t i { length }; i-- > 0; )
{
    // body runs with i = length-1, length-2, ..., 0
    // i-- yields the old value for the test, THEN decrements
}
```

(If that loop looks slippery, you're right — it is exactly the kind of thing
unsigned types make awkward, which is why 4.5 steers you toward signed indices
unless you have a reason.)

### Choosing a type at a glance

| Situation | Reasonable type |
| --- | --- |
| ordinary count / math | `int` |
| very large signed count | `long long` or `std::int64_t` |
| exact 32-bit signed field | `std::int32_t` |
| a raw byte | `std::uint8_t` (or `std::byte`) |
| a container's size result | `std::size_t` (or the container's `size_type`) |
| difference between two positions | a signed difference type, often `std::ptrdiff_t` |

## 4.7 — Introduction to scientific notation

Before we get to floating point, a short detour into the notation it is built on.
**Scientific notation** writes a number as a *significand* times a power of ten:

```text
significand × 10^exponent
```

A couple of worked examples:

```text
1.23e4  =  1.23 × 10^4   =  12300
5.0e-3  =  5.0  × 10^-3  =  0.005
```

C++ lets you write floating-point literals in this `e` form directly — the `e`
stands for "times ten to the":

```cpp
double large { 1.2e6 };  // 1.2 × 10^6  = 1,200,000
double small { 3.5e-4 }; // 3.5 × 10^-4 = 0.00035
```

### Why it's worth knowing

Two reasons. First, it is compact for values with many zeros, which are painful
and error-prone to type out:

```text
0.000000001   →  1e-9
1000000000    →  1e9
```

Second — and this is the practical one — C++ *prints* large or small
floating-point values in this form, so you need to be able to read it:

```text
1.23456e+06   means   1.23456 × 10^6
```

### Normalized form

By convention, "normalized" scientific notation keeps exactly one nonzero digit
to the left of the decimal point:

```text
3.14e2   normalized   (one digit before the point)
31.4e1   same value, but not normalized
```

This is more than a tidiness rule. Floating-point hardware stores numbers using a
*binary* version of exactly this idea — a significand and an exponent — which is
why understanding the notation helps you understand the next section's
approximations.

## 4.8 — Floating point numbers

**Floating-point** types represent approximate real numbers — values with
fractional parts, and values of enormous or tiny magnitude. C++ offers three:

```cpp
float f {};
double d {};
long double ld {};
```

For everyday floating-point work, `double` is the right default — it offers more
precision than `float` at little practical cost on modern hardware.

> **Best practice:** Use `double` for ordinary floating-point values. Choose
> `float` only when you have a specific reason (e.g. large arrays where halving
> the memory matters).

### Floating point is approximate — on purpose

Here is the fact that trips up everyone at first: **most decimal fractions cannot
be represented exactly in binary.** Just as `1/3` has no exact decimal form
(`0.3333…`), values like `0.1` have no exact *binary* form. The computer stores
the closest value it can, which is very close to `0.1` but not mathematically
identical to one-tenth.

```cpp
double x { 0.1 }; // stored value is the nearest binary approximation, not exact
```

Push a couple of those approximations through arithmetic and the tiny errors
become visible:

```cpp
#include <iomanip>
#include <iostream>

int main()
{
    double sum { 0.1 + 0.2 };
    std::cout << std::setprecision(17) << sum << '\n'; // 0.30000000000000004
}
```

The mathematically correct answer is `0.3`. Floating point gives you something
*almost* equal — close enough for most purposes, but not exactly equal, and that
difference is the source of a famous class of bugs.

### The trade: range for exactness

Integers and floating-point types make opposite bargains:

```text
integer types:
  exact within their range
  no fractions at all

floating-point types:
  fractions, plus huge and tiny magnitudes
  but only approximate, with limited significant digits
```

So a `double` can hold `1e308` *and* `0.00001`, but it cannot hold every value in
between exactly. It keeps a fixed number of significant digits and accepts a small
error on the rest.

### Errors accumulate

Because each operation can introduce a tiny rounding error, those errors can pile
up across many operations:

```cpp
double total {};

for (int i { 0 }; i < 10; ++i)
{
    total += 0.1;
}
// mathematically total should be 1.0; in floating point it may be slightly off
```

Ten additions of an inexact `0.1` need not land precisely on `1.0`.

### Don't compare floating-point values with `==`

This is the headline rule of the section. Because two floating-point computations
that *should* be equal often differ in the last few digits, comparing them with
`==` is unreliable:

```cpp
if (total == 1.0) // fragile — total may be 0.9999999999999999
{
    std::cout << "exactly one\n";
}
```

Instead, test whether the two values are *close enough* — within a small tolerance
called an **epsilon**:

```cpp
#include <cmath>

bool nearlyEqual(double a, double b, double epsilon)
{
    return std::abs(a - b) <= epsilon; // true when a and b differ by at most epsilon
}
```

and use it like this:

```cpp
if (nearlyEqual(total, 1.0, 1e-9))
{
    std::cout << "close enough\n";
}
```

> **Warning:** Avoid `==` and `!=` on floating-point values you computed. Compare
> with an epsilon tolerance instead. You will write exactly this `nearlyEqual`
> function in the lab — and reuse it inside a classifier — so the rule becomes
> muscle memory.

A useful rule of thumb for your course work: keep *exact* quantities (counts,
branch totals) in integer types, and treat genuinely *fractional* quantities
(ratios, percentages) as approximate floating point. Don't mix the two
expectations.

### Special values: infinity and NaN

Floating-point hardware reserves a few patterns for results that aren't ordinary
numbers:

- positive infinity,
- negative infinity,
- **NaN**, "not a number" (e.g. the result of `0.0 / 0.0`).

```cpp
double inf { 1.0 / 0.0 }; // may produce positive infinity
double nan { 0.0 / 0.0 }; // may produce NaN
```

These exist, but they are a sign that something went sideways. NaN in particular
is contagious — it propagates through arithmetic and even compares unequal to
itself. Don't build normal control flow on them; validate your inputs *before*
performing operations that could produce them.

### Controlling output precision

By default, `std::cout` prints only a handful of significant digits, which can
*hide* the approximation you just learned about. To see the full stored value, ask
for more digits with `std::setprecision` from `<iomanip>`:

```cpp
#include <iomanip>
#include <iostream>

int main()
{
    double x { 1.0 / 3.0 };

    std::cout << x << '\n';                          // a rounded view, e.g. 0.333333
    std::cout << std::setprecision(17) << x << '\n'; // the full precision view
}
```

`std::setprecision` controls how many significant digits are shown — handy when
you want to *see* exactly what floating point is doing rather than the polite
rounded summary.

## 4.9 — Boolean values

A **`bool`** holds one of exactly two values: `true` or `false`. It is the type of
yes/no, on/off, satisfied-or-not. After the numeric complexity of the last few
sections, `bool` is a relief — there are no ranges to worry about and no overflow
to fear.

```cpp
bool done { false };
bool valid { true };
```

Booleans are the natural fuel for conditions, which is why this section sits right
before `if`:

```cpp
if (valid)
{
    std::cout << "valid\n";
}
```

### Printing booleans

By default, `std::cout` prints a `bool` as a number: `1` for `true`, `0` for
`false`.

```cpp
std::cout << true  << '\n'; // 1
std::cout << false << '\n'; // 0
```

That is rarely what you want in human-facing output. Switch to words with
`std::boolalpha`:

```cpp
std::cout << std::boolalpha;
std::cout << true  << '\n'; // true
std::cout << false << '\n'; // false
```

`std::boolalpha` is a *manipulator* — once you send it to the stream, it stays in
effect, so every later `bool` prints as a word until you turn it off with
`std::noboolalpha`.

### Reading booleans

Input mirrors output. By default, `std::cin >> someBool` expects the digit `0` or
`1`. If you have set `std::boolalpha` on the input stream, it instead expects the
words `true` or `false`.

### Name booleans like questions

A boolean variable reads best when its name is a yes/no question, so that the
condition `if (name)` reads as plain English:

```cpp
bool isEmpty {};
bool hasCrash {};
bool shouldStop {};
bool foundTarget {};
```

Compare those to a vague name that tells the reader nothing about what `true`
would mean:

```cpp
bool data {}; // true means... what, exactly?
```

> **Best practice:** Prefix boolean names with `is`, `has`, `should`, or a similar
> word so the variable reads as a question with a yes/no answer.

### Integers convert to `bool`

There is one conversion worth understanding now, because it explains a common
idiom. Any integer converts to `bool` by a simple rule:

```text
0         →  false
anything nonzero  →  true
```

This is why you will see conditions written as a bare value:

```cpp
if (x) // means: if x is nonzero
{
}
```

It works, but it can obscure intent. When the value is not *inherently* a truth
value — when `x` is a count, say — an explicit comparison reads more clearly and
says what you actually mean:

```cpp
if (count != 0) // clearer than `if (count)` for a count
{
}
```

## 4.10 — Introduction to `if` statements

An **`if` statement** lets your program make a decision: run a piece of code only
when some condition holds. This is your first taste of *control flow* — code that
no longer marches straight from top to bottom but chooses its path.

```cpp
if (condition)
{
    statement;
}
```

If `condition` is `true`, the body runs. If it is `false`, the body is skipped and
execution continues after it.

```text
condition?
   │
   ├── true  → run the body  → continue
   │
   └── false → skip the body → continue
```

### A first example

```cpp
int x { 5 };

if (x > 0)
{
    std::cout << "positive\n";
}
```

The condition here is `x > 0`. A relational expression like this evaluates to a
`bool` — `true` when `x` is greater than `0`, `false` otherwise — which is exactly
the kind of value `if` wants.

### Braces vs a single statement

Without braces, an `if` governs *only the one statement that follows it* — no more.
This is a notorious source of confusion:

```cpp
if (x > 0)
    std::cout << "positive\n"; // only THIS line is conditional
std::cout << "done\n";         // this ALWAYS runs, indentation notwithstanding
```

The indentation suggests both lines belong to the `if`, but only the first does.
`"done"` prints no matter what. The fix is to always use braces, so the structure
on the page matches the structure the compiler sees:

```cpp
if (x > 0)
{
    std::cout << "positive\n";
}

std::cout << "done\n";
```

> **Best practice:** Always wrap the body of an `if` in braces, even for a single
> statement. It prevents the "the indentation lied to me" bug and makes adding a
> second line later safe.

### `if` / `else`

Attach an `else` to run alternate code when the condition is false. Exactly one of
the two branches runs:

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

### Chaining with `else if`

To test several conditions in order, chain them. C++ checks each condition from
top to bottom and runs the body of the *first* one that is true, skipping the
rest:

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

Order matters here: because the first match wins, you arrange conditions so that
the right one is reached first. You will rely on exactly this ordered-chain
behavior in the lab's capstone classifier, where the first matching band decides
the answer.

### Why an analyzer cares about every `if`

A quick connection to the bigger picture. Each `if` introduces a *branch* — a fork
where execution can go one of two ways:

```text
if (condition)
    ├── the true branch
    └── the false branch
```

Tools that measure how thoroughly a program is tested (coverage analysis, a theme
of this course) count those branches and ask: did the tests exercise *both* sides?
An `if` whose `false` path never ran is a corner of your program no test has
visited. Keeping that in mind makes you write more testable code — and it is why
this chapter's lab ends with a branch-heavy classifier.

## 4.11 — Chars

A **`char`** stores a single character-sized value — one letter, digit, or
symbol.

```cpp
char letter { 'A' };
```

Character literals are written with **single quotes**. This is a hard distinction
from strings, which use double quotes:

```cpp
'A'   // a char literal
'\n'  // a char literal: the newline character
'0'   // a char literal: the digit zero
```

```cpp
"A"      // a string literal
"hello"  // a string literal
```

The difference is not cosmetic — the two have different *types*:

```text
'A'  →  a single char
"A"  →  a string literal: the character A plus a hidden null terminator
```

So `'A'` is one thing and `"A"` is another. Mixing them up is a classic
beginner error the compiler will usually catch.

### Characters are numbers in disguise

Here is the idea that makes `char` interesting: a character is stored as an
*integer code*. Character encodings (ASCII and its supersets) assign each
character a number. In any ASCII-compatible encoding:

```text
'A'  →  65
'a'  →  97
'0'  →  48      (the digit zero, not the value zero!)
'\n' →  10
```

You can see the underlying number by converting the `char` to an `int`:

```cpp
char ch { 'A' };
std::cout << static_cast<int>(ch) << '\n'; // 65 in ASCII
```

This numeric nature is genuinely useful — because `'A'` is `65` and `'B'` is `66`,
you can *shift* a character by doing arithmetic on its code and converting back.
That round-trip — char to int, do math, int back to char — is exactly one of the
lab's tasks, and it is why `static_cast` and `char` share this chapter.

### Escape sequences

Some characters can't be typed directly into a literal — a newline, a tab, or the
quote characters themselves. You write these with an **escape sequence**: a
backslash followed by a code.

| Escape | Meaning |
| --- | --- |
| `\n` | newline |
| `\t` | tab |
| `\\` | a literal backslash |
| `\"` | a literal double quote |
| `\'` | a literal single quote |
| `\0` | the null character |

You have been using `\n` since chapter 1:

```cpp
std::cout << "line 1\nline 2\n"; // prints two lines
```

### `char` may be signed or unsigned

A subtle portability point: plain `char` is *not* guaranteed to be signed or
unsigned — the implementation chooses. That ambiguity is fine when you use `char`
for *text*, but it makes plain `char` a poor choice for storing small *numbers*,
since you can't be sure of its range.

> **Best practice:** Use `char` for characters/text. If you need a small *integer*,
> say so explicitly with `signed char`, `unsigned char`, `std::int8_t`, or
> `std::uint8_t`, depending on whether you need negatives.

### Reading a char

```cpp
char ch {};
std::cin >> ch;
```

The `>>` extraction operator skips any leading whitespace and reads a single
non-whitespace character. That "skip whitespace" behavior is usually convenient,
but it means `>>` cannot read a space or newline *into* `ch`. To capture
whitespace characters too, you use lower-level input such as `std::cin.get(ch)`,
which the I/O chapter covers later.

## 4.12 — Introduction to type conversion and `static_cast`

A **type conversion** produces a value of one type from a value of another. You
have already triggered several without naming them; this section makes them
deliberate. The key distinction is between conversions C++ does *for* you and
conversions you ask for *explicitly*.

### Implicit conversions happen automatically

C++ performs many conversions silently when the context calls for one:

```cpp
double d { 5 }; // the int 5 is converted to the double 5.0
```

```cpp
int x {};
if (x) // x (an int) is converted to bool for the condition
{
}
```

These are convenient, and often exactly what you want. The danger is that an
implicit conversion can quietly *lose information* — and because it is silent, you
might not notice.

### Narrowing conversions lose data

A **narrowing conversion** is one where the destination type can't represent every
value of the source type — converting `double` to `int`, for example, throws away
the fractional part. Brace initialization is designed to *catch* these and refuse
to compile:

```cpp
int x { 3.5 }; // error: brace init rejects this narrowing — the .5 would be lost
```

That error is a feature: it stops a silent loss before it happens. Older
assignment-style initialization is more permissive and will perform the narrowing
anyway, sometimes with only a warning:

```cpp
int x = 3.5; // compiles; x becomes 3 — the .5 is silently discarded
```

> **Best practice:** Prefer brace initialization `{ }`. It turns dangerous
> narrowing conversions into compile errors instead of silent data loss — this is
> a major reason the whole course initializes with braces.

### `static_cast`: converting on purpose

When you genuinely *want* a conversion, say so explicitly with **`static_cast`**.
Reading `static_cast<T>(value)` tells anyone — including future you — "I know a
conversion is happening here, and I meant it."

The canonical use is forcing floating-point division between integers:

```cpp
int total { 7 };
int count { 2 };

double average { static_cast<double>(total) / count }; // 3.5
```

Walk through why the cast is necessary. Without it:

```cpp
double average { total / count }; // 3.0 — integer division happened first!
```

Both operands are `int`, so `total / count` is *integer* division: `7 / 2` is `3`,
and only that truncated `3` is widened to `3.0`. With the cast, the left operand
becomes a `double` *before* the division, so the whole operation is floating point:

```text
static_cast<double>(total)  →  7.0
7.0 / 2  →  (the int 2 is promoted to 2.0)  →  3.5
```

### char ↔ int

`static_cast` is also how you read a character's numeric code on demand:

```cpp
char ch { 'A' };
std::cout << static_cast<int>(ch) << '\n'; // 65, the code — not the glyph
```

and, going the other way, how you turn a computed code back into a character.

### A cast doesn't change the original

An important mental model: `static_cast` does not modify the object you cast. It
produces a *new, temporary* converted value for use in the surrounding expression.
The original keeps its type and value.

```cpp
int x { 5 };
double d { static_cast<double>(x) }; // x is unchanged
```

```text
x:
  type  int
  value 5            ← untouched

static_cast<double>(x):
  a temporary value
  type  double
  value 5.0          ← used to initialize d, then gone
```

### Prefer `static_cast` over C-style casts

You may see an older cast syntax in C-derived code:

```cpp
double d { (double)x }; // C-style cast — avoid
```

Prefer the named form:

```cpp
double d { static_cast<double>(x) }; // explicit and searchable
```

`static_cast` is more visible (it stands out when you skim code), easy to search
for, and *more constrained* — it permits only sensible conversions and rejects
nonsense, whereas a C-style cast will silently attempt almost anything. That extra
safety is exactly why we want it.

> **Best practice:** Use `static_cast` for explicit conversions. Avoid C-style
> casts like `(double)x` — they are harder to spot and far less safe.

### The conversion that earns its own lab task

One pattern deserves special attention because it combines everything in this
chapter — and because it is the lab's trickiest task. Suppose you have a signed
index and an unsigned length, and you want to use the index to read a character:

```cpp
std::size_t length { seed.size() }; // unsigned
int index { getRandomIndex() };     // signed; could be negative
```

To compare `index` against `length` you must convert one to the other's type. But
recall 4.5: casting a *negative* `int` to `std::size_t` does not give you a small
number — it wraps to a gigantic one, which sails right past any length check. So
the order of operations is the whole point:

```cpp
if (index >= 0 && static_cast<std::size_t>(index) < seed.size())
{
    char ch { seed[static_cast<std::size_t>(index)] };
}
```

Notice the cast comes *after* the `index >= 0` check. You confirm the value is
nonnegative first; only then is it safe to reinterpret it as unsigned.

> **Warning:** Always verify an `int` is nonnegative *before* you `static_cast` it
> to `std::size_t` (or any unsigned type). Cast a negative value and it becomes
> enormous, defeating the very bounds check you were trying to perform.

The lab's `isValidIndex` task is precisely this guard, isolated so you can feel why
the order matters. Get the sequence wrong and a negative index slips through as
"valid."

## 4.x — Chapter 4 summary

You now have the full vocabulary of C++'s built-in numeric world. The throughline
of the whole chapter: *types are not interchangeable labels — each one decides how
bits are read, what range is available, and what happens at the edges.* The
specifics worth keeping:

- **Fundamental types** are built into the language; a type tells the compiler how
  to interpret bits, what operations are legal, and which conversions may occur.
- **`void`** means "no value" — most often as the return type of a function that
  does work rather than computing a result. You cannot make a `void` object.
- **`sizeof`** reports a type's or object's size in *bytes*, and its result is a
  `std::size_t`. It inspects an expression's type without evaluating it.
- **Signed integers** (`int` and friends) cover negatives, zero, and positives;
  prefer `int` for ordinary work. **Signed overflow is undefined behavior** — never
  rely on it wrapping.
- **Integer division truncates** the fractional part toward zero.
- **Unsigned integers** wrap modulo `2^n`. The wrap is *defined* but routinely
  surprising; avoid unsigned for general arithmetic, and beware signed/unsigned
  mixing in comparisons.
- **Fixed-width integers** live in `<cstdint>` for when exact bit-widths matter.
  **`std::size_t`** is the unsigned type used for sizes and lengths.
- **Scientific notation** writes a value as a significand times a power of ten, and
  is how C++ prints very large and small floating-point numbers.
- **Floating-point** values are *approximate*. Don't compare them with `==`; test
  closeness with an epsilon instead.
- **`bool`** holds `true`/`false`; print readable words with `std::boolalpha`, and
  name booleans like yes/no questions.
- **`if`** statements run code conditionally and introduce *branches* — the unit a
  coverage analyzer measures. Always brace the body.
- **`char`** stores a single character whose value is an integer code; character
  literals use single quotes, strings use double quotes.
- **Escape sequences** like `\n` and `\t` write characters you can't type directly.
- Conversions are **implicit** or **explicit**; prefer brace initialization to
  catch narrowing, and use **`static_cast`** for conversions you mean.
- **Guard before you cast** a possibly-negative signed value to an unsigned type —
  check the sign first, then convert.

With that, you are ready for the **Numeric Types Lab**, where you will make every
one of these warnings *physical*: watch an 8-bit value wrap past `255`, prove that
`0.1 + 0.2` isn't `0.3`, round-trip a char through its integer code, and stop a
negative index at the gate before it can pretend to be valid.
