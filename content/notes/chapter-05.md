# Chapter 5 — Constants and Strings

> Source: <https://www.learncpp.com/> (Chapter 5)
> One file per chapter. Each lesson is a section below.

## Contents
- [5.1 — Constant variables (named constants)](#51--constant-variables-named-constants)
- [5.2 — Literals](#52--literals)
- [5.3 — Numeral systems (decimal, binary, hexadecimal, and octal)](#53--numeral-systems-decimal-binary-hexadecimal-and-octal)
- [5.4 — The as-if rule and compile-time optimization](#54--the-as-if-rule-and-compile-time-optimization)
- [5.5 — Constant expressions](#55--constant-expressions)
- [5.6 — Constexpr variables](#56--constexpr-variables)
- [5.7 — Introduction to std::string](#57--introduction-to-stdstring)
- [5.8 — Introduction to std::string_view](#58--introduction-to-stdstring_view)
- [5.9 — std::string_view (part 2)](#59--stdstring_view-part-2)
- [5.x — Chapter 5 summary and quiz](#5x--chapter-5-summary-and-quiz)

---

## 5.1 — Constant variables (named constants)

### Constants: the big split

A **constant** is a value that cannot be changed while the program is running.
C++ has two broad categories:

| Category | Meaning | Example |
|---|---|---|
| **Named constant** / symbolic constant | Constant value attached to an identifier | `maxStudents`, `gravity`, `pi` |
| **Literal constant** | Constant value written directly in source code | `42`, `3.14`, `'x'`, `"hello"` |

```
named constant:     constexpr int maxStudents { 30 };
literal constant:   if (count > 30) { ... }
                                    ^^ direct value, no name
```

Named constants are better for values that carry meaning or may need to change.
Literal constants are fine for obvious throwaway values like `0`, `1`, `true`,
or `'\n'`.

### Three ways C++ can make named constants

| Mechanism | Where covered | Use today? |
|---|---|---|
| `const` / `constexpr` variables | Chapter 5 | Yes, preferred |
| Object-like macros with substitution text | Chapter 2.10 and this lesson | Avoid for typed constants |
| Enumerated constants | Chapter 13 | Useful for named choices/states |

### `const` variables

A **constant variable** is a variable whose value cannot be changed after it is
initialized. The phrase sounds contradictory, but it means: the object has a
name and storage like a variable, but the value is locked.

```cpp
int retries { 3 };
retries = 4;          // ok: non-const variable

const double gravity { 9.8 };
gravity = 1.6;        // compile error: cannot assign to const object
```

The `const` keyword is a **const qualifier**. LearnCpp notes that C++ accepts
both positions:

```cpp
const int maxUsers { 100 }; // common style: const before the type
int const maxFiles { 20 };  // "east const": legal, less common here
```

For these notes, use the normal beginner-readable style:

```cpp
const Type name { initializer };
```

### Const variables must be initialized

Unlike a normal variable, a const variable cannot be defined empty and assigned
later. Assignment would be a post-creation change, and `const` forbids that.

```cpp
const int size;       // compile error: no initializer
size = 10;            // also not allowed

const int width { 10 }; // ok
```

Mental model:

```
normal variable:
    create box -> maybe put value later -> can replace value

const variable:
    create sealed box with value inside -> cannot reopen
```

### Naming constants

LearnCpp's style is ordinary variable-style names for local constants:

```cpp
const int maxStudents { 30 };
constexpr double taxRate { 0.0825 };
```

Some codebases use all caps for constants, but in C++ that can visually collide
with macro style. The important thing is consistency with the local codebase.

### Const function parameters

If a parameter is passed **by value**, the function receives its own copy. Making
that copy `const` only prevents the function body from modifying its local copy.
It does not protect the caller's original value.

```cpp
void printTwice(const int x)
{
    // x = 5;       // compile error inside this function only
    std::cout << x << ' ' << x << '\n';
}
```

For by-value parameters, `const` is often noise in the function declaration. It
can be useful inside a long implementation, but it does not change the function's
external contract.

CS6340 tie-in:

```cpp
void instrumentCoverage(Module *M, Function &F, Instruction &I);
```

Those parameters are not simple by-value copies. `Module *`, `Function &`, and
`Instruction &` point/refer to existing LLVM objects, so const-correctness there
would matter much more.

### Const return values

For simple return-by-value types, returning `const` usually does not help:

```cpp
const int getCount(); // usually not useful
int getCount();       // prefer this
```

The returned value is already a temporary copy. Marking it const can interfere
with later optimizations for class types. Prefer non-const returns by value
unless there is a specific reason.

### Why make variables constant?

Make a variable constant when its value should not change. Benefits:

- The compiler catches accidental writes.
- The name documents intent.
- The compiler can sometimes optimize more easily.
- Readers know they do not have to track future mutations.

```cpp
constexpr int columnsPerRow { 16 };

for (int col { 0 }; col < columnsPerRow; ++col)
{
    // columnsPerRow is stable; only col changes
}
```

### Object-like macros are not real variables

An object-like macro with substitution text is handled by the preprocessor:

```cpp
#define MAX_STUDENTS 30
```

Before compilation, the preprocessor does textual replacement:

```
source you wrote:      if (count > MAX_STUDENTS)
after preprocessing:   if (count > 30)
```

Problems:

- Macros do not obey normal C++ scope rules.
- They have no type.
- Debuggers usually do not see them as variables.
- Text substitution can produce confusing errors.
- A macro name can collide with other identifiers.

Prefer:

```cpp
constexpr int maxStudents { 30 };
```

### Type qualifiers

`const` and `volatile` are **type qualifiers**. `const` is common. `volatile` is
rare and tells the compiler that a value may change outside the compiler's
normal view, which disables some optimizations. Most ordinary C++ code does not
need `volatile`.

---

## 5.2 — Literals

### Literal constants

A **literal** is a value written directly into source code.

```cpp
std::cout << 42 << '\n';
std::cout << "hello\n";
```

| Literal | Kind |
|---|---|
| `5` | integer literal |
| `3.14` | floating-point literal |
| `'a'` | character literal |
| `"hello"` | C-style string literal |
| `true` / `false` | boolean literal |

Literals have types. This matters because C++ uses the literal's type to decide
which operations are legal and how much storage/precision is involved.

### Literal suffixes

Suffixes adjust the type of a literal.

```cpp
5      // int
5u     // unsigned int
5L     // long
5.0    // double
5.0f   // float
"hi"s  // std::string literal, with namespace setup
"hi"sv // std::string_view literal, with namespace setup
```

Use uppercase `L` for long, because lowercase `l` is easy to confuse with `1`.

### Integral and floating-point literals

Integer literals represent whole numbers:

```cpp
int students { 30 };
```

Floating-point literals represent fractional values:

```cpp
double gravity { 9.8 };
double tiny { 1.2e-5 };   // scientific notation: 1.2 * 10^-5
double huge { 4.0e12 };   // 4.0 * 10^12
```

Scientific notation is useful when values are very large or very small. The
`e` means "times ten to the power of".

```
1.23e4  == 1.23 * 10^4  == 12300
1.23e-4 == 1.23 * 10^-4 == 0.000123
```

### C-style string literals

A string literal in double quotes is a **C-style string literal** by default:

```cpp
"Hello"
```

Important hidden detail: C-style strings are null-terminated. A trailing `'\0'`
marks where the string ends.

```
"cat"

memory-ish view:
+-----+-----+-----+------+
| 'c' | 'a' | 't' | '\0' |
+-----+-----+-----+------+
```

That null terminator is why `"cat"` looks like 3 characters but needs 4 slots in
memory. This becomes important for `std::string_view`, because string views can
look at existing character storage instead of owning a copy.

### Literal lifetimes

C-style string literals are created when the program starts and exist for the
whole program. That makes this safe:

```cpp
std::string_view greeting { "hello" }; // viewing long-lived literal storage
```

But string and string_view **literal suffixes** can create temporary objects:

```cpp
using namespace std::string_literals;

auto s { "hello"s }; // std::string temporary used to initialize s
```

The temporary-object rules matter more once `std::string_view` enters the room.

### Magic numbers

A **magic number** is a literal whose meaning is unclear or whose value may need
to change later.

```cpp
if (studentCount > 30)       // what is 30?
{
    openSecondSection();
}
```

Better:

```cpp
constexpr int maxStudentsPerSection { 30 };

if (studentCount > maxStudentsPerSection)
{
    openSecondSection();
}
```

The named constant solves two problems:

1. Meaning: the reader knows what the value represents.
2. Change control: update the value once at its definition.

Not every literal is magic:

```cpp
count = 0;        // zero is often obvious
++count;          // one-step increment is obvious
if (index == -1)  // often used as "not found", if local convention is clear
```

Best default: if the literal has domain meaning, name it.

---

## 5.3 — Numeral systems (decimal, binary, hexadecimal, and octal)

### Numeral systems

A **numeral system** is a way to represent numbers using digits. C++ commonly
uses four:

| System | Base | Digits | C++ literal prefix |
|---|---:|---|---|
| Decimal | 10 | `0`-`9` | none |
| Binary | 2 | `0`, `1` | `0b` |
| Octal | 8 | `0`-`7` | leading `0` |
| Hexadecimal | 16 | `0`-`9`, `A`-`F` | `0x` |

### Decimal vs binary place value

Decimal:

```
427 = 4*100 + 2*10 + 7*1
    = 4*10^2 + 2*10^1 + 7*10^0
```

Binary:

```
0b1101 = 1*8 + 1*4 + 0*2 + 1*1
       = 13 decimal
```

ASCII table:

```
binary place:   8   4   2   1
bits:           1   1   0   1
value:          8 + 4 + 0 + 1 = 13
```

### Octal literals

Octal uses a leading zero:

```cpp
int x { 012 }; // octal 12 == decimal 10
```

This is a foot-gun: a stray leading zero changes the base. In modern code, avoid
octal unless you intentionally need it.

### Hexadecimal literals

Hex uses `0x`:

```cpp
int color { 0xFF00AA };
int mask  { 0x0F };
```

Hex is popular because one hex digit maps exactly to four bits:

```
hex:     A
decimal: 10
binary:  1010

hex byte:  0xC5
binary:   1100 0101
           C    5
```

That is why memory addresses, byte masks, and raw binary-ish values are often
shown in hex.

### Binary literals and digit separators

C++14 added binary literals:

```cpp
int flags { 0b1010'0101 };
```

The apostrophe is a **digit separator**. It is ignored by the compiler but helps
humans read long numbers.

```cpp
int million { 1'000'000 };
int byte    { 0b1100'0101 };
int word    { 0xCAFE'BABE };
```

### Outputting bases

`std::cout` outputs numbers in decimal by default. Use manipulators for other
bases:

```cpp
#include <iostream>

int main()
{
    int value { 42 };

    std::cout << std::dec << value << '\n'; // 42
    std::cout << std::hex << value << '\n'; // 2a
    std::cout << std::oct << value << '\n'; // 52
}
```

Important: these manipulators stick until changed.

```
std::cout << std::hex;
// later outputs are still hexadecimal unless std::dec is restored
```

### Outputting binary with `std::bitset`

`std::cout` has no built-in binary manipulator. `std::bitset<N>` prints a fixed
number of bits.

```cpp
#include <bitset>
#include <iostream>

int main()
{
    std::bitset<8> bits { 0b1100'0101 };
    std::cout << bits << '\n'; // 11000101
}
```

The bit count `N` must be known at compile time.

CS6340 tie-in: bit masks and binary/hex notation show up in low-level systems,
compiler internals, and coverage/instrumentation work. You do not need to love
base conversion, but you do need to recognize what `0x`, `0b`, and bit grouping
mean.

---

## 5.4 — The as-if rule and compile-time optimization

### Optimization

**Optimization** means changing a program so it uses fewer resources: less time,
less memory, smaller executable size, or less power.

Two broad paths:

| Kind | Who does it? | Examples |
|---|---|---|
| High-level optimization | Programmer | Better algorithm, better data structure, less I/O |
| Low-level optimization | Compiler/optimizer | Fold constants, remove unused code, inline small functions |

Modern C++ compilers are optimizing compilers. The optimizer changes generated
machine code, not your source file.

```
source.cpp  --compiler + optimizer-->  executable
     ^                                      ^
     |                                      |
you edit this                       optimizer changes this
```

### Optimization levels

Debug builds usually disable or reduce optimization. Release builds usually turn
optimization on.

| Build mode | Optimization | Debuggability |
|---|---|---|
| Debug | off / low | source and executable line up closely |
| Release | high | faster, but harder to step through |

Command-line flavor:

```bash
g++ -O0 -g main.cpp   # debug-ish: no optimization, include debug symbols
g++ -O2 main.cpp      # release-ish: optimize
```

CS6340 tie-in: optimization can change the LLVM IR. A pass run over `-O0` IR may
see different instructions than a pass run after optimization.

### The as-if rule

The **as-if rule** lets the compiler transform your program however it wants, as
long as the program's **observable behavior** is unchanged.

Observable behavior includes things like:

- text printed to the console,
- input/output effects,
- volatile accesses,
- program termination behavior.

If the outside-visible behavior is the same, the compiler can rewrite freely.

```
you write:          int x { 3 + 4 };
compiler may emit:  int x { 7 };

observable behavior: same
```

### Compile-time evaluation

By default, expressions run at runtime. But if the compiler can determine a
result during compilation, it may do the work early.

```
compile time: source -> compiler does known work -> executable
runtime:      executable -> user runs it -> unknown/input work happens
```

### Constant folding

**Constant folding** replaces an expression made of known literals with its
result.

```cpp
int x { 3 + 4 }; // compiler can fold to 7
```

ASCII:

```
before:  3 + 4
         \___/
           |
after:     7
```

### Constant propagation

**Constant propagation** replaces uses of a value that is known not to change.

```cpp
const int x { 7 };
int y { x + 5 };
```

Optimizer can reason:

```
x is always 7
y = x + 5
y = 7 + 5
y = 12
```

Const variables make this easier because the compiler has a language-level
promise that the value will not be reassigned.

### Dead code elimination

**Dead code** is code that has no effect on observable behavior. The optimizer
can remove it.

```cpp
int x { 3 + 4 }; // if x is never used, this can disappear
```

### Why not always optimize?

Optimization can make debugging weird:

- A variable may be optimized away.
- A function may be inlined, so stepping into it looks strange.
- Lines may execute in a different order than expected.
- Generated instructions may not map neatly to source statements.

So: debug builds prioritize understandability; release builds prioritize output
quality and performance.

### Compile-time constants vs runtime constants

| Term | Meaning | Example |
|---|---|---|
| Compile-time constant | Value known during compilation | `constexpr int n { 4 };` |
| Runtime constant | Value fixed after initialization, but not known until runtime | `const int age { readAge() };` |

```cpp
int readAge();

constexpr int columns { 16 };  // compile-time constant
const int age { readAge() };   // runtime constant: cannot change, but not known early
```

This distinction matters because some C++ features require values known at
compile time.

---

## 5.5 — Constant expressions

### Runtime expressions vs constant expressions

An **expression** combines literals, variables, operators, and function calls to
produce a value.

Most expressions are runtime expressions:

```cpp
int age {};
std::cin >> age;      // only known when the program runs
int next { age + 1 }; // runtime expression
```

A **constant expression** is an expression that can be evaluated at compile time.

```cpp
3 + 4                 // constant expression
sizeof(int)           // constant expression
```

### Why constant expressions matter

Compile-time computation gives three major benefits:

1. Faster runtime: work is already done.
2. Smaller/simpler generated code.
3. Access to language features that require compile-time values.

Example: fixed-size arrays and template arguments often need compile-time
constants.

```cpp
constexpr int size { 8 };
std::bitset<size> flags {}; // ok: size known at compile time
```

### Constant expression ingredients

Constant expressions can use things the compiler is allowed to know at compile
time:

- literals,
- `constexpr` variables,
- certain operators,
- certain functions that are allowed in constant expressions,
- types and sizes known during compilation.

They cannot depend on runtime input or runtime-only work:

```cpp
int getRuntimeValue();

constexpr int a { 2 + 3 };       // ok
const int b { getRuntimeValue() }; // const, but not a constant expression
```

### Compile-time evaluable vs required compile-time

Some expressions **can** be evaluated at compile time, but the standard does not
always require the compiler to do so unless the expression is used in a context
that demands it.

```
can be compile-time:       int x { 3 + 4 };
must be compile-time:      constexpr int x { 3 + 4 };
```

The optimizer may fold the first. The language requires the second initializer
to be valid at compile time.

### Constant expression vs compile-time constant

These terms are close:

- **Constant expression**: expression that can be evaluated at compile time.
- **Compile-time constant**: object/value whose value is known at compile time.

```cpp
constexpr int x { 5 }; // x is a compile-time constant
x + 2                  // constant expression
```

### CS6340 tie-in

LLVM and compiler tooling cares deeply about what happens before runtime. Chapter
5 is where that door opens:

```
C++ source
  -> compile-time checks/evaluation/optimization
  -> LLVM IR / object code
  -> runtime behavior
```

Lab 1's instrumentation inserts runtime checks, but the compiler still controls
the IR you get to instrument. Understanding compile-time vs runtime will keep
you from treating source code, IR, and execution as the same layer.

---

## 5.6 — Constexpr variables

### The challenge with `const`

`const` means "cannot be changed after initialization." It does **not**
automatically mean "known at compile time."

```cpp
const int a { 5 };          // compile-time constant

int readValue();
const int b { readValue() }; // runtime constant
```

Both are const. Only `a` is known during compilation.

### `constexpr`

`constexpr` means the variable must be initialized by a constant expression.

```cpp
constexpr int maxColumns { 16 }; // ok
constexpr int bad { readValue() }; // compile error: runtime call
```

Use `constexpr` when you need a compile-time named constant.

### `const` vs `constexpr`

| Keyword | Promise |
|---|---|
| `const` | Value cannot be changed after initialization |
| `constexpr` | Value is constant and must be known at compile time |

ASCII:

```
const
  ├─ compile-time const possible
  └─ runtime const possible

constexpr
  └─ compile-time const required
```

### Best practice

Prefer `constexpr` for named constants whose values are known at compile time:

```cpp
constexpr double gravity { 9.8 };
constexpr int maxRetries { 3 };
constexpr std::string_view appName { "fuzzer" };
```

Use `const` for values that should not change but are only known at runtime:

```cpp
const int age { readAgeFromUser() };
```

### Function parameters cannot be `constexpr`

A function parameter receives a value when the function is called. Calls happen
at runtime in the general case, so parameters cannot be `constexpr`.

```cpp
void print(constexpr int x); // not allowed
```

Use `const` for a parameter if the function should not modify it internally:

```cpp
void printValue(const int x)
{
    std::cout << x << '\n';
}
```

### Brief preview: constexpr functions

Later C++ allows functions to be marked `constexpr`, meaning they can be used in
constant expressions when called with compile-time arguments.

```cpp
constexpr int square(int x)
{
    return x * x;
}

constexpr int area { square(4) }; // compile-time result
```

This is a preview, not the full rules.

---

## 5.7 — Introduction to std::string

### Why not C-style string variables?

C-style string literals are common and fine:

```cpp
std::cout << "hello";
```

But C-style string variables are awkward and dangerous. Modern C++ usually uses:

- `std::string` when you need an owning, modifiable string.
- `std::string_view` when you need read-only access without copying.

### `std::string`

`std::string` lives in `<string>`.

```cpp
#include <string>

int main()
{
    std::string name { "Alex" };
    name = "Jordan";
}
```

It can represent text of different lengths and handles memory management for
you.

```
std::string name
  owns its character data
  can grow/shrink
  can be reassigned
```

### Output

```cpp
#include <iostream>
#include <string>

int main()
{
    std::string language { "C++" };
    std::cout << "Learning " << language << '\n';
}
```

### Input with `std::cin`

`std::cin >> name` reads one whitespace-delimited word:

```cpp
std::string first {};
std::cin >> first; // "Ada Lovelace" stores only "Ada"
```

### Input with `std::getline`

Use `std::getline()` to read a full line including spaces:

```cpp
std::string fullName {};
std::getline(std::cin, fullName);
```

If you previously used `operator>>`, a newline may still be waiting in the input
buffer. `std::ws` consumes leading whitespace:

```cpp
int age {};
std::cin >> age;

std::string name {};
std::getline(std::cin >> std::ws, name);
```

Mental model:

```
input buffer after reading age:
    "42\nAda Lovelace\n"
       ^
       leftover newline can make getline return empty

std::ws eats leading whitespace before getline starts collecting text
```

### String length

Use `.length()` to get the number of characters:

```cpp
std::string word { "fuzz" };
std::cout << word.length() << '\n'; // 4
```

The return type is an unsigned size type (`std::size_t`), not `int`. Be mindful
when comparing lengths with signed integers.

```cpp
auto len { word.length() }; // often simplest for now
```

### `std::string` is expensive to copy

Unlike an `int`, a string may manage heap memory and many characters. Copying it
can allocate memory and duplicate all characters.

```cpp
void printBad(std::string text) // copies argument
{
    std::cout << text << '\n';
}
```

For read-only function parameters, prefer `std::string_view` after lesson 5.8.
For now, understand that by-value string parameters copy.

### Returning `std::string`

Returning a `std::string` by value is normal. Modern C++ can often elide or move
the return efficiently.

```cpp
std::string makeGreeting(std::string_view name)
{
    std::string result { "Hello, " };
    result += name;
    return result;
}
```

### `std::string` literals

With namespace support, the `s` suffix creates a `std::string` literal:

```cpp
#include <string>
using namespace std::string_literals;

auto name { "Alex"s }; // std::string
```

Do not turn this into a habit of global `using namespace std;`. The literal
namespace is narrow and specifically for literal suffixes.

### Constexpr strings

`std::string` generally is not the right type for compile-time string constants.
Use `constexpr std::string_view` for string symbolic constants when possible.

---

## 5.8 — Introduction to std::string_view

### The copy problem

`std::string` owns its text. Ownership is safe, but copying can be expensive:

```cpp
void printString(std::string str) // copy
{
    std::cout << str << '\n';
}
```

If all the function wants to do is read the text, copying is wasteful.

### `std::string_view`

`std::string_view` lives in `<string_view>` and provides read-only access to
string data that already exists somewhere else.

```cpp
#include <iostream>
#include <string_view>

void print(std::string_view text)
{
    std::cout << text << '\n';
}
```

Diagram:

```
C-style literal storage:  "hello\0"
                           ^
                           |
std::string_view ----------+

The view does not own the characters.
It stores something like: pointer + length.
```

### Why function parameters love string_view

A `std::string_view` parameter can accept:

```cpp
print("literal");             // C-style string literal

std::string owned { "owned" };
print(owned);                 // std::string

std::string_view view { "v" };
print(view);                  // std::string_view
```

No string copy is required just to read the text.

Best default for read-only string parameters:

```cpp
void logMessage(std::string_view message);
```

### string_view does not implicitly become string

C++ allows `std::string` or C-style strings to become `std::string_view`
implicitly, because viewing is cheap. It does not implicitly convert
`std::string_view` to `std::string`, because that would make an expensive copy
too easily.

```cpp
void takesString(std::string s);

std::string_view sv { "hello" };
// takesString(sv);              // compile error
takesString(std::string { sv }); // explicit copy
```

### Assignment changes the view

Assigning to a `std::string_view` changes what it views. It does not modify the
old string.

```cpp
std::string_view sv { "first" };
sv = "second"; // sv now views "second"; "first" is unchanged
```

### string_view literals

The `sv` suffix creates a `std::string_view` literal:

```cpp
#include <string_view>
using namespace std::string_view_literals;

auto label { "input"sv };
```

But this is not required for simple initialization:

```cpp
std::string_view label { "input" }; // fine
```

### constexpr string_view

`std::string_view` works well for compile-time string constants:

```cpp
constexpr std::string_view toolName { "fuzzer" };
```

This is the Chapter 5 sweet spot:

- `constexpr` gives compile-time named constant.
- `std::string_view` avoids owning/copying string data.
- String literal storage lasts for the whole program.

---

## 5.9 — std::string_view (part 2)

### Owner vs viewer

The key mental model:

```
std::string
  owns characters
  controls lifetime
  can modify text
  copy can be expensive

std::string_view
  views characters owned elsewhere
  does not control lifetime
  read-only
  cheap to copy
```

Analogy:

```
std::string      = owning a printed book
std::string_view = reading a page through a window

If the book is destroyed or the page is replaced, the window is not magic.
It may now show invalid or surprising contents.
```

### Dangling views

A `std::string_view` becomes dangerous when the thing it views no longer exists.
That is called a **dangling view**.

Bad:

```cpp
std::string_view sv {};

{
    std::string local { "temporary" };
    sv = local;
} // local destroyed here

std::cout << sv << '\n'; // undefined behavior
```

The view outlives the owner.

ASCII lifetime:

```
time ---->

local string:   [ alive here ] X destroyed
sv view:          [------------- still exists -------------]
                                ^ dangling after this point
```

### Do not view temporary std::string objects

This is a classic trap:

```cpp
using namespace std::string_literals;

std::string_view name { "Alex"s }; // bad
```

`"Alex"s` creates a temporary `std::string`. The view points at that temporary,
then the temporary is destroyed at the end of the full expression.

Safe:

```cpp
std::string_view a { "Alex" };   // views C-style literal, long-lived
std::string_view b { "Alex"sv }; // also ok with sv literal
```

### Modifying a string can invalidate views

If a `std::string_view` views a `std::string`, modifying the string may
invalidate the view.

```cpp
std::string s { "short" };
std::string_view sv { s };

s = "a much longer string that may reallocate";
// sv may now be dangling or stale
```

Why:

```
before:
s owns buffer A:  "short"
sv points to A

after assignment:
s may own buffer B: "a much longer..."
buffer A may be gone
sv still points to A -> invalid
```

If the buffer is reused instead of reallocated, the view may still have the old
length, producing a substring or garbage-looking superstring. Either way, do not
trust the view after the owner changes.

### Revalidating a view

You can make a view valid again by assigning it something valid to view:

```cpp
sv = s;        // re-view the current string
sv = "reset";  // view long-lived literal
```

### Returning string_view safely

Dangerous:

```cpp
std::string_view getName()
{
    std::string name { "Ada" };
    return name; // bad: returns view to local string
}
```

Safe when returning a C-style string literal:

```cpp
std::string_view boolName(bool value)
{
    return value ? "true" : "false";
}
```

Safe-ish when returning one of the function's `std::string_view` parameters,
because the caller is responsible for ensuring the original viewed data lives
long enough:

```cpp
std::string_view shorter(std::string_view a, std::string_view b)
{
    return (a.length() <= b.length()) ? a : b;
}
```

### View modification functions

`std::string_view` has functions that modify the view, not the underlying string:

```cpp
std::string_view sv { "abcdef" };
sv.remove_prefix(2); // now views "cdef"
sv.remove_suffix(1); // now views "cde"
```

The original string/literal is unchanged.

Diagram:

```
original:  a b c d e f \0
view 1:    [a b c d e f]
prefix-2:      [c d e f]
suffix-1:      [c d e]
```

### string_view can view substrings

A `std::string_view` stores a pointer and a length, so it can view the middle of
a string without copying.

That is powerful, but it means the viewed text is not necessarily null-terminated
at the end of the view.

```
underlying storage:  h e l l o \0
view of "ell":         [e l l]
storage after view:          o \0
```

If you pass `.data()` to a C API expecting a null-terminated string, it may read
past the view. Be careful.

### Quick guide: string vs string_view

Use `std::string` when:

- you need to own the text,
- you need to modify the text,
- you need to store the text long-term independent of its source,
- you need a guaranteed null-terminated string for APIs via `.c_str()`.

Use `std::string_view` when:

- a function only needs read-only access,
- you want to accept literals, `std::string`, and `std::string_view` cheaply,
- the viewed data is guaranteed to outlive the view,
- you need a compile-time string symbolic constant.

CS6340 fuzzer tie-in:

```cpp
std::string mutateA(std::string input);       // owns and changes a mutant
void log(std::string_view message);           // reads only, no ownership needed
constexpr std::string_view campaign { "A" };  // named string constant
```

For Lab 1 mutation functions, `std::string` is the right workhorse because you
will create and modify candidate inputs. For logging, names, labels, and read-only
parameters, `std::string_view` is often better.

---

## 5.x — Chapter 5 summary and quiz

### Core takeaways

- A **named constant** attaches an identifier to a value that should not change.
- A **literal** is a value written directly into source code.
- Prefer `constexpr` variables over magic numbers.
- `const` prevents reassignment after initialization.
- `constexpr` requires compile-time initialization.
- A compile-time constant is known during compilation.
- A runtime constant is fixed after initialization but only known when the
  program runs.
- Literals have types, and suffixes can change those types.
- C-style string literals are null-terminated and live for the whole program.
- `std::string` owns modifiable text but can be expensive to copy.
- `std::string_view` cheaply views existing text but can dangle.

### Decision table

| Need | Prefer |
|---|---|
| Named numeric constant known at compile time | `constexpr int` / `constexpr double` |
| Named value fixed after runtime input | `const` |
| Modifiable text | `std::string` |
| Read-only string parameter | `std::string_view` |
| Compile-time string label | `constexpr std::string_view` |
| Raw literal with unclear meaning | Do not: name it |

### Common mistakes to catch

```cpp
const int x; // error: const must be initialized
```

```cpp
constexpr int age { readAge() }; // error unless readAge is constexpr-valid
```

```cpp
std::string_view sv { "hello"s }; // bad: dangling view to temporary string
```

```cpp
std::string text { "abc" };
std::string_view view { text };
text = "a much longer value"; // view may be invalid now
```

```cpp
int value { 012 }; // not twelve: octal literal, decimal 10
```

### Mini drill

Write a tiny program that uses all three:

```cpp
#include <iostream>
#include <string>
#include <string_view>

constexpr int maxMutationsPerSeed { 10 };
constexpr std::string_view defaultCampaign { "MutationA" };

std::string makeInput(std::string_view seed)
{
    std::string input { seed }; // copy because we plan to modify
    input += "!";
    return input;
}

int main()
{
    std::string mutant { makeInput("abc") };

    std::cout << defaultCampaign << ": "
              << mutant << " with budget "
              << maxMutationsPerSeed << '\n';
}
```

What this reinforces:

- `defaultCampaign` is a compile-time string label.
- `seed` is read-only, so `std::string_view` is enough.
- `input` must be mutable and owned, so it is `std::string`.
- `maxMutationsPerSeed` names a number that would otherwise be magic.
