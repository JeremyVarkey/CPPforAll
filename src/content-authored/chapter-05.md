# Chapter 5 — Constants and Strings

> Source: <https://www.learncpp.com/> (Chapter 5)

So far, every value in your programs has been a moving part: variables you read into, update, and overwrite. This chapter is about the opposite instinct — values you promise *not* to change, and the compiler's willingness to hold you to that promise. We'll see why "this never changes" is one of the most useful things you can tell a compiler, how it lets the compiler do work *before your program ever runs*, and how the same idea reshapes the way we handle text. By the end you'll have two of the most important tools in modern C++: `constexpr` for compile-time constants, and the `std::string` / `std::string_view` pair for working with text safely and cheaply.

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

### Two kinds of "constant"

The word *constant* just means a value that cannot change while the program runs. But that one idea shows up in two different forms in your source code, and it's worth naming them so we can talk precisely.

A **literal constant** is a value written directly into the code — `42`, `3.14`, `'x'`, `"hello"`. It has no name; it's right there in the text. A **named constant** (also called a symbolic constant) is a constant value that you've attached an identifier to, like `maxStudents` or `gravity`. Same kind of unchanging value, but now it has a name you can refer to.

```cpp
// named constant: the value lives behind an identifier
constexpr int maxStudents { 30 };

// literal constant: the value is written directly, no name
if (count > 30) { /* ... */ }
//            ^^ that 30 is a literal
```

The split matters because the two have different strengths. A literal is perfect for a value that is obvious and throwaway — `0`, `1`, `true`, `'\n'`. A named constant earns its keep when the value carries *meaning* ("the maximum class size") or might need to change later. We'll keep coming back to this judgment call.

### Three ways C++ can name a constant

Historically, C++ has offered a few mechanisms for naming a constant value. For this course, only one of them is the modern default, but it helps to know the landscape:

- **`const` / `constexpr` variables** — the subject of this chapter, and the preferred tool today.
- **Object-like macros** with substitution text (`#define`) — a preprocessor trick from C. We'll see why to avoid it for typed constants.
- **Enumerated constants** — great for naming a *set* of related choices or states; you'll meet these properly in a later chapter.

For now: when you want to name a constant value, reach for a `const` or `constexpr` variable.

### `const` variables

A **constant variable** is a variable whose value cannot be changed after it's initialized. The phrase sounds like a contradiction — a "variable" that doesn't vary — but it describes the situation exactly: the object has a name and storage like any variable, but the value inside is locked.

```cpp
int retries { 3 };
retries = 4;            // ok: ordinary variable, free to reassign

const double gravity { 9.8 };
gravity = 1.6;          // compile error: cannot assign to a const object
```

The `const` keyword is a **const qualifier** — it qualifies (modifies) the type. C++ actually accepts it on either side of the type:

```cpp
const int maxUsers { 100 };  // const before the type (the common style)
int const maxFiles { 20 };   // "east const": equally legal, less common
```

Both mean the same thing. Pick one style and be consistent; this course writes `const` before the type:

```cpp
const Type name { initializer };
```

### A const variable must be initialized

Here's a consequence that trips people up. An ordinary variable can be created empty now and given a value later. A const variable cannot — because that "later assignment" would be a change after creation, and `const` forbids changes after creation.

```cpp
const int size;       // compile error: a const has no second chance to get a value
size = 10;            // ...and this assignment isn't allowed either

const int width { 10 }; // correct: give it its value at birth
```

A mental picture that sticks:

> **Key insight:** An ordinary variable is an open box — you can drop a value in now or later, and swap it whenever. A const variable is a *sealed* box: you put the value in as you seal it, and it can never be reopened. That's why it must be initialized on the same line it's declared.

### Naming constants

Use ordinary, readable variable-style names for your constants:

```cpp
const int maxStudents { 30 };
constexpr double taxRate { 0.0825 };
```

You'll see some codebases shout their constants in `ALL_CAPS`. That convention comes from C, where it signals a macro — and in C++ it can visually collide with actual macros, which is confusing. The only firm rule is to match whatever the code around you already does.

### const on function parameters

When a parameter is passed **by value**, the function gets its *own copy* of the argument. Marking that copy `const` only stops the function body from modifying *its local copy* — it does nothing to protect the caller's original value, because the caller's value was never in danger to begin with.

```cpp
void printTwice(const int x)
{
    // x = 5;   // compile error: can't modify x inside this function
    std::cout << x << ' ' << x << '\n';
}
```

So for a by-value parameter, `const` is mostly internal bookkeeping — a note to yourself that "I won't reassign this." It can be a helpful discipline inside a long function body, but it doesn't change the function's contract with its callers. (When we get to references and pointers in a later chapter, `const` on a parameter starts to matter a great deal more — because then the function really can reach back and touch the caller's data.)

### const on return-by-value

For the simple types you've used so far, returning `const` by value usually buys you nothing:

```cpp
const int getCount(); // the const here is essentially useless
int getCount();       // prefer this
```

The returned value is already a fresh temporary copy — the caller can't hurt anyone else's data through it. Worse, marking a by-value return `const` can interfere with optimizations the compiler would otherwise apply. Prefer plain non-const returns by value unless you have a specific reason not to.

### Why bother making things const?

It's a fair question. The value didn't *need* the keyword to behave correctly. The benefits are about catching mistakes and communicating intent:

- The compiler catches any accidental write — a typo that would silently corrupt your logic becomes a clear error at compile time.
- The name and the `const` together *document* that this value is fixed, so a reader doesn't have to scan the rest of the function wondering if it changes.
- The compiler can sometimes optimize more aggressively, knowing the value is stable.

```cpp
constexpr int columnsPerRow { 16 };

for (int col { 0 }; col < columnsPerRow; ++col)
{
    // columnsPerRow is guaranteed stable; only col moves
}
```

> **Best practice:** Make a variable const (or constexpr) whenever its value should not change after initialization. It costs one keyword and pays you back in caught bugs and clearer code.

### Object-like macros are not real variables

Before `const`, C programmers named constants with the preprocessor:

```cpp
#define MAX_STUDENTS 30
```

This is *not* a variable. It's a text-substitution rule. Before the compiler ever sees your code, the preprocessor finds every `MAX_STUDENTS` and pastes `30` in its place:

```text
what you wrote:        if (count > MAX_STUDENTS)
after preprocessing:   if (count > 30)
```

That blind find-and-replace causes real problems. Macros ignore C++'s scope rules — they leak everywhere. They have no type, so the compiler can't type-check them. Debuggers usually can't see them as named values. And a macro name can silently collide with an identifier elsewhere, producing baffling errors. Prefer the typed, scoped, debugger-visible alternative:

```cpp
constexpr int maxStudents { 30 };
```

### A note on type qualifiers

`const` is one of two **type qualifiers** in C++. The other is `volatile`, which tells the compiler that a value might change in ways it can't see — for example, a hardware register that the outside world updates — and so disables certain optimizations on it. It's rare, specialized, and you almost certainly won't need it in ordinary code. Mentioning it here just so the term doesn't surprise you later.

---

## 5.2 — Literals

### Literals have types

A **literal** is a value typed directly into your source code. You've been writing them all along:

```cpp
std::cout << 42 << '\n';
std::cout << "hello\n";
```

Each literal belongs to a category, and — this is the part that's easy to miss — each one has a *type*:

| Literal | Kind |
|---|---|
| `5` | integer literal |
| `3.14` | floating-point literal |
| `'a'` | character literal |
| `"hello"` | C-style string literal |
| `true` / `false` | boolean literal |

The type matters because C++ uses it to decide which operations are legal and how much precision or storage is involved. `5` is an `int`; `3.14` is a `double`. That's not a detail you usually think about, but it's quietly steering how your expressions behave.

### Literal suffixes

Sometimes the default type isn't what you want. A **suffix** changes the type of a literal:

```cpp
5      // int
5u     // unsigned int
5L     // long
5.0    // double
5.0f   // float
"hi"s  // std::string         (needs a namespace, see below)
"hi"sv // std::string_view    (needs a namespace, see below)
```

> **Tip:** Use an uppercase `L` for `long`. A lowercase `l` is far too easy to mistake for the digit `1` — `5l` versus `51` is a bug waiting to happen.

The `s` and `sv` suffixes are how you ask for a `std::string` or `std::string_view` literal rather than a plain C-style string. They require a small `using` declaration to bring the right literal namespace into scope; we'll use them carefully later in the chapter.

### Integer and floating-point literals

Integer literals are whole numbers:

```cpp
int students { 30 };
```

Floating-point literals carry a fractional part, and they can be written in **scientific notation**, which is invaluable for very large or very small numbers:

```cpp
double gravity { 9.8 };
double tiny { 1.2e-5 };   // 1.2 × 10⁻⁵
double huge { 4.0e12 };   // 4.0 × 10¹²
```

The `e` reads as "times ten to the power of":

```text
1.23e4   == 1.23 × 10⁴   == 12300
1.23e-4  == 1.23 × 10⁻⁴  == 0.000123
```

### C-style string literals and the hidden null

Any text in double quotes is, by default, a **C-style string literal**:

```cpp
"Hello"
```

These carry a hidden detail that will matter a lot in a few lessons: they are **null-terminated**. C++ silently appends a special `'\0'` character to mark where the string ends.

```text
"cat" in memory:

+-----+-----+-----+------+
| 'c' | 'a' | 't' | '\0' |
+-----+-----+-----+------+
```

So `"cat"` looks like three characters but occupies four slots in memory. Keep that null terminator in mind — it's the reason C-style strings can be passed around as a bare pointer (the `'\0'` tells code where to stop), and it's a subtlety that returns when we meet `std::string_view`.

### Where literals live

C-style string literals are special in another way: they're created when the program starts and exist for its *entire* lifetime. They don't come and go with a function call. That permanence is what makes this safe:

```cpp
std::string_view greeting { "hello" }; // viewing storage that lasts the whole program
```

The `s` and `sv` *suffixed* literals are a different story — they can create temporary objects that vanish at the end of the expression. We'll handle that carefully in lesson 5.9, because it's a classic source of bugs.

### Magic numbers

A **magic number** is a literal whose meaning isn't obvious, or whose value might need to change later. Magic numbers are where literals turn from convenient to dangerous:

```cpp
if (studentCount > 30)       // why 30? what does it mean?
{
    openSecondSection();
}
```

Give it a name, and two problems disappear at once:

```cpp
constexpr int maxStudentsPerSection { 30 };

if (studentCount > maxStudentsPerSection)
{
    openSecondSection();
}
```

Now the reader knows *what* `30` represents, and if the policy changes to 25, you edit one line instead of hunting through the file for every `30` (and guessing which ones meant "class size" versus something else).

Not every literal is magic, though — don't go overboard:

```cpp
count = 0;        // zero is usually self-explanatory
++count;          // incrementing by one needs no name
if (index == -1)  // a -1 "not found" sentinel is fine if that convention is clear locally
```

> **Best practice:** If a literal carries domain meaning — a limit, a rate, a special code — name it with a constant. If it's an obvious `0` or `1`, leave it alone.

This is the central habit the chapter lab will drill: the badge formatter's org name, separator, and width all become named constants instead of literals scattered through the logic.

---

## 5.3 — Numeral systems (decimal, binary, hexadecimal, and octal)

### Four ways to write the same number

A **numeral system** is just a convention for writing numbers using digits. We grow up with decimal, but a computer's natural language is binary, and programmers borrow a couple of others for convenience. C++ lets you write a literal in any of four bases, signaled by a prefix:

| System | Base | Digits | C++ prefix |
|---|---:|---|---|
| Decimal | 10 | `0`–`9` | none |
| Binary | 2 | `0`, `1` | `0b` |
| Octal | 8 | `0`–`7` | leading `0` |
| Hexadecimal | 16 | `0`–`9`, `A`–`F` | `0x` |

Crucially, these are all just different *spellings* of the same underlying value. `0b1101`, `015`, `0xD`, and `13` are the identical number to the computer — only the notation differs.

### Place value, the common thread

Every base works the same way: each digit's position is a power of the base. Decimal:

```text
427 = 4×100 + 2×10 + 7×1
    = 4×10² + 2×10¹ + 7×10⁰
```

Binary is identical, just with powers of two:

```text
0b1101 = 1×8 + 1×4 + 0×2 + 1×1
       = 13 (decimal)

place:   8   4   2   1
bits:    1   1   0   1
value:   8 + 4 + 0 + 1 = 13
```

### Octal — and a foot-gun

Octal literals are marked by a leading zero:

```cpp
int x { 012 }; // octal 12, which is decimal 10 — NOT twelve
```

That leading-zero rule is a trap. A stray `0` in front of a number silently changes its base, so `012` is *not* the twelve you might have meant.

> **Warning:** Avoid writing octal literals unless you genuinely need them. A number like `0700` does not equal seven hundred — and an accidental leading zero on `0123` quietly turns it into a different value than `123`.

### Hexadecimal — the programmer's favorite

Hex literals use the `0x` prefix:

```cpp
int color { 0xFF00AA };
int mask  { 0x0F };
```

Hex is everywhere in systems programming because of one elegant fact: **one hex digit maps to exactly four bits.** That makes it a compact, readable shorthand for binary.

```text
hex:     A          hex byte: 0xC5
decimal: 10         binary:  1100 0101
binary:  1010                 C    5
```

Because each hex digit is exactly a half-byte (a "nibble"), memory addresses, byte masks, and raw bit patterns are almost always shown in hex — it lines up cleanly with the underlying bits in a way decimal never can.

### Binary literals and digit separators

Since C++14 you can write binary directly:

```cpp
int flags { 0b1010'0101 };
```

That apostrophe is a **digit separator**. The compiler ignores it completely — it's purely a visual aid so your eyes can group long numbers, exactly like commas in `1,000,000`:

```cpp
int million { 1'000'000 };
int byte    { 0b1100'0101 };
int word    { 0x00CA'FEBA };
```

The separator works in any base, and you can place it anywhere between digits — group by thousands in decimal, by nibbles in binary or hex, whatever reads best. One caution: it only affects readability, never value or type. Keep your literal *in range* for its type as usual — a braced `int` initializer still rejects a value too large to fit (which is exactly why the hex example above uses a value that fits in an `int` rather than something larger).

### Printing in different bases

`std::cout` prints in decimal by default. To switch, you use *manipulators* — special tokens you stream in that change how subsequent numbers are formatted:

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

> **Warning:** These manipulators are *sticky*. Once you stream `std::hex`, every number after it prints in hex until you switch back with `std::dec`. Forgetting to switch back is a common source of "why is my output garbage" confusion.

### Printing binary with std::bitset

There's no built-in manipulator for binary, but `std::bitset<N>` fills the gap — it represents a fixed `N` bits and prints them directly:

```cpp
#include <bitset>
#include <iostream>

int main()
{
    std::bitset<8> bits { 0b1100'0101 };
    std::cout << bits << '\n'; // 11000101
}
```

The bit count `N` has to be known at compile time (it's baked into the type) — which is a small preview of why "known at compile time" is about to become a recurring theme.

> **Note:** You don't need to love base conversion to write good C++. But you do need to *recognize* `0x`, `0b`, and grouped bits when you see them — they're the standard notation for masks, addresses, and low-level data, and they'll appear the moment you touch anything close to the machine.

---

## 5.4 — The as-if rule and compile-time optimization

### What optimization actually means

**Optimization** is changing a program so it uses fewer resources — less time, less memory, a smaller executable, less power — while producing the same results. It comes from two directions:

| Kind | Who does it | Examples |
|---|---|---|
| High-level | You, the programmer | A better algorithm, a smarter data structure, fewer I/O calls |
| Low-level | The compiler's optimizer | Folding constants, deleting unused code, inlining small functions |

The part to internalize is that the compiler's optimizer rewrites the *generated machine code*, never your source file. Your `.cpp` stays exactly as you wrote it; the executable is what gets reshaped.

```text
source.cpp  ──compiler + optimizer──▶  executable
     ▲                                     ▲
     │                                     │
 you edit this                   the optimizer rewrites this
```

### Debug vs release builds

Optimization is something you can dial up or down, and the two common settings have a real trade-off:

| Build mode | Optimization | Debuggability |
|---|---|---|
| Debug | off / low | source lines map cleanly onto the executable |
| Release | high | faster and smaller, but harder to step through |

On the command line that looks like:

```bash
g++ -O0 -g main.cpp   # debug-ish: no optimization, keep debug symbols
g++ -O2 main.cpp      # release-ish: optimize
```

You optimize *less* while developing precisely because heavy optimization makes a debugger's job confusing — which we'll see in a moment.

### The as-if rule

Here's the principle that gives the optimizer its freedom. The **as-if rule** says the compiler may transform your program in any way it likes, *as long as the program's observable behavior is unchanged.* The compiler must produce a program that behaves *as if* it ran your code exactly as written — but how it gets there is its business.

"Observable behavior" is a precise idea. It includes things the outside world can detect: text printed to the console, input and output effects, accesses to `volatile` data, and how the program terminates. If none of that changes, the compiler can rearrange everything else.

```text
you write:           int x { 3 + 4 };
compiler may emit:   int x { 7 };

observable behavior: identical → the rewrite is allowed
```

### Doing the work early

By default your expressions are evaluated at *runtime* — when the program runs. But if the compiler can already figure out a result during compilation, the as-if rule lets it do that work early and bake the answer in.

```text
compile time:  source ──▶ compiler computes what it can ──▶ executable
runtime:       executable ──▶ user runs it ──▶ input-dependent work happens
```

This split — what's knowable now versus what's only knowable when the program runs — is the conceptual heart of the rest of the chapter.

### Constant folding

**Constant folding** is the simplest case: an expression built entirely from known literals gets replaced by its result.

```cpp
int x { 3 + 4 }; // the compiler can fold this to: int x { 7 };
```

The addition never happens at runtime — it happened once, in the compiler.

### Constant propagation

**Constant propagation** goes a step further: it replaces *uses* of a value the compiler knows can't change.

```cpp
const int x { 7 };
int y { x + 5 };
```

The compiler reasons: `x` is always `7`, so `x + 5` is `7 + 5`, which is `12` — and it can store `12` directly. This is exactly where `const` earns its keep beyond catching typos: the language-level promise that `x` is never reassigned is what *lets* the compiler substitute its value with confidence.

### Dead code elimination

**Dead code** is code that has no effect on observable behavior. The optimizer is free to delete it entirely.

```cpp
int x { 3 + 4 }; // if x is never used anywhere, this line can simply vanish
```

### Why not always crank optimization to the max?

Because the same transformations that make a release build fast make a debug session bewildering:

- A variable you want to inspect may have been optimized away — there's nothing to look at.
- A small function may be inlined, so "step into" lands somewhere surprising.
- Statements may execute in a different order than you wrote them.
- Generated instructions may not line up one-to-one with your source lines.

So debug builds prioritize *understandability* (keep the mapping honest) and release builds prioritize *output quality* (be fast). You pick based on whether you're hunting a bug or shipping the result.

### Compile-time constants vs runtime constants

This brings us to a distinction the next two lessons depend on. Both of the following are `const` — neither can be reassigned — but only one has a value the *compiler* knows:

| Term | Meaning | Example |
|---|---|---|
| Compile-time constant | Value known during compilation | `constexpr int n { 4 };` |
| Runtime constant | Fixed after initialization, but not known until the program runs | `const int age { readAge() };` |

```cpp
int readAge();

constexpr int columns { 16 };  // compile-time constant: the value is right there
const int age { readAge() };   // runtime constant: unchangeable, but not known early
```

`age` can't change once set — but its value depends on a function call that only happens at runtime, so the compiler can't fold or propagate it. That difference is the whole reason `constexpr` exists, which is exactly where we're headed.

---

## 5.5 — Constant expressions

### Runtime expressions vs constant expressions

An **expression** combines literals, variables, operators, and function calls to compute a value. Most expressions are **runtime expressions** — their value isn't known until the program runs:

```cpp
int age {};
std::cin >> age;      // value comes from the user, at runtime
int next { age + 1 }; // depends on age, so this is a runtime expression
```

A **constant expression** is one the compiler can fully evaluate *during compilation*:

```cpp
3 + 4                 // constant expression
sizeof(int)           // constant expression: the size is fixed at compile time
```

### Why we care about constant expressions

Pushing work to compile time buys three concrete things:

1. **Faster runtime** — the work is already done before the program ever starts.
2. **Simpler generated code** — there's nothing left to compute, just a baked-in value.
3. **Access to features that *require* compile-time values** — and this is the big one.

Some C++ constructs flatly demand a value the compiler already knows. The size of a `std::bitset`, fixed-size array bounds, template arguments — these are part of a *type*, and types are fixed at compile time. So they can only accept compile-time values:

```cpp
constexpr int size { 8 };
std::bitset<size> flags {}; // ok: size is known at compile time
```

### What's allowed inside a constant expression

A constant expression can only use ingredients the compiler is permitted to evaluate ahead of time: literals, `constexpr` variables, certain operators, certain functions specifically allowed in this context, and sizes/types known during compilation. What it *cannot* do is depend on anything that only exists at runtime:

```cpp
int getRuntimeValue();

constexpr int a { 2 + 3 };          // ok: pure compile-time arithmetic
const int b { getRuntimeValue() };  // const, yes — but NOT a constant expression
```

`b` is a perfectly valid const variable; it just can't be used anywhere a constant expression is required, because its value rides in from a runtime call.

### "Can be" vs "must be" compile-time

There's a subtle but important wrinkle. Many expressions *can* be evaluated at compile time, but the standard doesn't *force* the compiler to do so — unless you put them in a context that demands it.

```text
can be compile-time:    int x { 3 + 4 };          // compiler may fold it (as-if rule)
must be compile-time:   constexpr int x { 3 + 4 }; // the language requires it
```

The first line *might* get folded by the optimizer, depending on the build. The second line *guarantees* the initializer is evaluated at compile time — and the compiler will reject it if it can't be. That guarantee is what the next lesson is all about.

### Constant expression vs compile-time constant

Two closely related terms, worth keeping straight:

- A **constant expression** is an *expression* the compiler can evaluate at compile time.
- A **compile-time constant** is an *object or value* whose value is known at compile time.

```cpp
constexpr int x { 5 }; // x is a compile-time constant
x + 2                  // x + 2 is a constant expression
```

The object is a *thing*; the expression is a *computation*. They go together but name different parts of the picture.

---

## 5.6 — Constexpr variables

### The gap in `const`

We've seen the problem `constexpr` solves, so let's name it cleanly. `const` makes one promise: *the value cannot be changed after initialization.* It says nothing about *when* the value becomes known.

```cpp
const int a { 5 };           // happens to be a compile-time constant
int readValue();
const int b { readValue() }; // a runtime constant — value arrives at runtime
```

Both are `const`. Only `a` is known during compilation. So if you write `const` intending a compile-time constant, the compiler won't stop you when you accidentally write one that isn't — it'll just quietly become a runtime constant, and any compile-time use of it will fail later in a confusing way.

### `constexpr` closes the gap

`constexpr` is the stronger keyword. It says: *this variable must be initialized by a constant expression* — the value is required to be known at compile time, and the compiler will reject anything that isn't.

```cpp
constexpr int maxColumns { 16 };   // ok: 16 is a constant expression
constexpr int bad { readValue() };  // compile error: a runtime call can't initialize a constexpr
```

That rejection is a *feature*. With `constexpr`, "I meant this to be a compile-time constant" is checked by the compiler, not left to chance.

### `const` vs `constexpr` side by side

| Keyword | Promise |
|---|---|
| `const` | Value cannot change after initialization |
| `constexpr` | Value is constant **and** must be known at compile time |

```text
const
  ├─ compile-time constant   (possible)
  └─ runtime constant        (possible)

constexpr
  └─ compile-time constant   (required)
```

> **Best practice:** Prefer `constexpr` for any named constant whose value is known at compile time. It does everything `const` does *and* guarantees the compile-time property.

```cpp
constexpr double gravity { 9.8 };
constexpr int maxRetries { 3 };
constexpr std::string_view appName { "fuzzer" };
```

Reach for plain `const` only when the value can't be known until runtime:

```cpp
const int age { readAgeFromUser() }; // can't change, but only known at runtime
```

### Function parameters can't be `constexpr`

A function parameter receives its value from the *call*, and in general calls happen at runtime — a parameter's value isn't known when the function is compiled. So `constexpr` on a parameter is meaningless and isn't allowed:

```cpp
void print(constexpr int x); // not allowed
```

If you want a parameter the function body won't modify, use `const`:

```cpp
void printValue(const int x)
{
    std::cout << x << '\n';
}
```

### A glimpse ahead: constexpr functions

`constexpr` isn't limited to variables. You can mark a *function* `constexpr`, which tells the compiler it may be called inside a constant expression when its arguments are themselves compile-time known:

```cpp
constexpr int square(int x)
{
    return x * x;
}

constexpr int area { square(4) }; // computed at compile time
```

That's a preview, not the full rule set — but it shows where `constexpr` ultimately goes: ordinary-looking code that the compiler can run *for you*, before your program ever does.

---

## 5.7 — Introduction to std::string

### Why we need a real string type

You've been printing C-style string literals since your first program, and that's perfectly fine:

```cpp
std::cout << "hello";
```

The trouble starts when you want a string *variable* — text you can store, change, and pass around. C-style string variables are clumsy and easy to misuse (fixed-size buffers, manual null terminators, overflow risks). Modern C++ gives you two purpose-built types instead, and almost all of your string work will use one of them:

- **`std::string`** — when you need to *own* and *modify* text.
- **`std::string_view`** — when you only need to *read* text someone else owns, without copying it.

This lesson covers the owning type; the next two cover the viewer.

### Using std::string

`std::string` lives in the `<string>` header. It behaves like the string type you'd hope for: assignable, resizable, and it manages its own memory for you.

```cpp
#include <string>

int main()
{
    std::string name { "Alex" };
    name = "Jordan"; // free to reassign, even to a longer or shorter value
}
```

```text
std::string name
  ├─ owns its character data
  ├─ can grow or shrink as needed
  └─ can be reassigned freely
```

You never call anything to free that memory — when the `std::string` goes out of scope, it cleans up after itself. That ownership is the feature.

### Printing a std::string

It streams to `std::cout` exactly as you'd expect:

```cpp
#include <iostream>
#include <string>

int main()
{
    std::string language { "C++" };
    std::cout << "Learning " << language << '\n';
}
```

### Reading with std::cin

Reading a string with `>>` works, but with one catch worth knowing up front: `operator>>` reads a single *whitespace-delimited word* and stops at the first space.

```cpp
std::string first {};
std::cin >> first; // typing "Ada Lovelace" stores only "Ada"
```

### Reading a whole line with std::getline

When you want the entire line, spaces included, use `std::getline`:

```cpp
std::string fullName {};
std::getline(std::cin, fullName);
```

There's one classic pitfall. If you read a number (or a word) with `>>` *before* a `getline`, a leftover newline is still sitting in the input buffer — and `getline` will read that newline as an empty line and stop immediately. The fix is `std::ws`, a manipulator that skips leading whitespace before `getline` starts collecting:

```cpp
int age {};
std::cin >> age;

std::string name {};
std::getline(std::cin >> std::ws, name); // std::ws eats the leftover newline first
```

```text
input buffer after reading age:
    "42\nAda Lovelace\n"
       ▲
       leftover newline — without std::ws, getline reads "" and stops

std::ws consumes that whitespace, so getline collects "Ada Lovelace"
```

### Measuring length

Ask a string its length with `.length()` (or the equivalent `.size()`):

```cpp
std::string word { "fuzz" };
std::cout << word.length() << '\n'; // 4
```

> **Warning:** `.length()` returns an *unsigned* size type (`std::size_t`), not an `int`. Mixing it with signed integers in comparisons or arithmetic can produce surprising results — and the compiler will warn you under `-Wall -Wextra`. When you need it as an `int`, convert deliberately with `static_cast<int>(...)`.

The lab leans on exactly this: `nameLength` returns the full name's `.size()`, cast to `int` so the conversion is intentional and warning-free.

### std::string is expensive to copy

This is the single most important thing to understand about `std::string`, and it motivates the entire next lesson. An `int` is tiny — copying it is free. A `std::string` may own a heap buffer holding many characters, so *copying it can allocate memory and duplicate every character.*

```cpp
void printBad(std::string text) // takes the string BY VALUE — copies the whole thing
{
    std::cout << text << '\n';
}
```

If that function only reads `text`, the copy is pure waste. For read-only parameters there's a better tool, which is precisely `std::string_view` in lesson 5.8. For now, just register the cost: **passing a `std::string` by value copies it.**

### Returning a std::string

Returning a `std::string` by value, on the other hand, is completely normal and idiomatic. Modern C++ is very good at avoiding the copy here — it can often *move* or elide the returned value, so building a string and returning it is the right pattern:

```cpp
std::string makeGreeting(std::string_view name)
{
    std::string result { "Hello, " };
    result += name;   // append onto the owned string
    return result;    // returned efficiently, not deep-copied
}
```

This *build-and-return* shape — seed an owning `std::string`, append onto it with `+=`, return it — is exactly what the lab's functions do.

### std::string literals

With a little namespace setup, the `s` suffix produces a `std::string` literal instead of a C-style one:

```cpp
#include <string>
using namespace std::string_literals;

auto name { "Alex"s }; // name is a std::string, not a C-style string
```

> **Best practice:** Bring in `std::string_literals` for the suffix when you need it, but don't escalate to a blanket `using namespace std;`. The literal namespace is deliberately narrow — it exists only to enable these suffixes.

### Why not constexpr std::string?

You might want a compile-time string constant. `std::string` is generally the *wrong* tool for that — it's built to own and manage runtime memory. For a string symbolic constant, use `constexpr std::string_view` instead, which we're about to meet and which is purpose-built for the job.

---

## 5.8 — Introduction to std::string_view

### The copy problem, restated

We just saw that `std::string` owns its text, and that copying it can be expensive. So consider a function whose only job is to *read* a string:

```cpp
void printString(std::string str) // copies the entire string just to read it
{
    std::cout << str << '\n';
}
```

Every call duplicates all those characters for no reason — the function never modifies them. We want a way to *look at* existing text without taking ownership of it. That's `std::string_view`.

### What a string_view is

`std::string_view` lives in `<string_view>` and provides **read-only access to character data that already exists somewhere else.** It doesn't own the characters; under the hood it's essentially a *pointer plus a length* — a window onto text that someone else is responsible for.

```cpp
#include <iostream>
#include <string_view>

void print(std::string_view text) // a cheap, non-owning, read-only window
{
    std::cout << text << '\n';
}
```

```text
C-style literal storage:  "hello\0"
                           ▲
                           │
std::string_view ──────────┘   (stores pointer + length, owns nothing)
```

Because a view is just a pointer and a length, copying a `std::string_view` is cheap no matter how long the underlying text is — you're copying the window, not the view through it.

### Why parameters love string_view

The real payoff: a single `std::string_view` parameter cheaply accepts *all three* kinds of string, with no copy required to read them.

```cpp
print("literal");              // a C-style string literal

std::string owned { "owned" };
print(owned);                  // a std::string

std::string_view view { "v" };
print(view);                   // another std::string_view
```

This is exactly why the lab makes every read-only parameter a `std::string_view`: the badge functions can be called with literals *or* `std::string`s *or* views, and none of those calls pays for a copy.

> **Best practice:** For a function parameter that only *reads* a string and doesn't need to store it, prefer `std::string_view`. It's the modern default for read-only string parameters.

```cpp
void logMessage(std::string_view message);
```

### A view does not silently become a string

C++ deliberately allows conversions in only one direction, and the asymmetry is on purpose. A `std::string` or C-style string converts to a `std::string_view` *implicitly*, because making a view is cheap and harmless. But a `std::string_view` does **not** implicitly convert back to a `std::string`, because that would hide an expensive copy behind innocent-looking code.

```cpp
void takesString(std::string s);

std::string_view sv { "hello" };
// takesString(sv);              // compile error: would be a hidden copy
takesString(std::string { sv }); // ok: you asked for the copy explicitly
```

The lab's very first task hinges on this: `kOrgName` is a `std::string_view`, but `badgeHeader()` returns a `std::string`, so you write the copy out loud — `return std::string { kOrgName };`.

### Assigning to a view changes the window, not the text

Assigning to a `std::string_view` re-aims it at *different* text. It never modifies whatever it used to be looking at:

```cpp
std::string_view sv { "first" };
sv = "second"; // sv now views "second"; the "first" literal is untouched
```

### string_view literals

The `sv` suffix produces a `std::string_view` literal:

```cpp
#include <string_view>
using namespace std::string_view_literals;

auto label { "input"sv };
```

You rarely need it for simple initialization, though — a plain literal works fine, because C-style literals convert to views implicitly:

```cpp
std::string_view label { "input" }; // perfectly fine, no suffix needed
```

### constexpr string_view: the chapter's sweet spot

Here's where the chapter's two halves meet. `std::string_view` works beautifully as a *compile-time* string constant:

```cpp
constexpr std::string_view toolName { "fuzzer" };
```

Look at why all three properties line up:

- `constexpr` gives you a guaranteed compile-time named constant.
- `std::string_view` owns nothing, so there's no runtime memory to manage.
- The C-style literal it views lives for the entire program, so the view can never dangle.

That combination — `constexpr std::string_view` for a named string constant — is the idiom to reach for, and it's exactly how the lab declares `kOrgName` and `kUsernameSep`.

---

## 5.9 — std::string_view (part 2)

### Owner vs viewer — the model to hold in your head

Everything tricky about `std::string_view` follows from one fact: it doesn't own what it looks at. Side by side:

```text
std::string                std::string_view
  owns the characters        views characters owned elsewhere
  controls their lifetime    controls nothing
  can modify the text        read-only
  copy can be expensive      copy is always cheap
```

An analogy that captures the danger:

> **Key insight:** A `std::string` is like *owning a printed book* — it's yours, it stays as long as you keep it. A `std::string_view` is like *reading one page through a window*. The window is light and cheap. But if someone takes the book away, or swaps the page, the window doesn't magically update — it now shows whatever happens to be there, which may be garbage. The view is only as trustworthy as the thing it's pointed at.

### Dangling views

A `std::string_view` turns dangerous the moment the thing it views stops existing. That's a **dangling view**, and it's undefined behavior to use one.

```cpp
std::string_view sv {};

{
    std::string local { "temporary" };
    sv = local;   // sv now views local's characters
} // local is destroyed here — its characters are gone

std::cout << sv << '\n'; // undefined behavior: viewing destroyed storage
```

The view outlived the data it pointed at:

```text
time ───▶

local string:   [ alive ]✗ destroyed
sv view:        [────────── still exists ──────────]
                         ▲ dangling from here on
```

### Don't view a temporary string

This is the trap that catches everyone at least once:

```cpp
using namespace std::string_literals;

std::string_view name { "Alex"s }; // bad!
```

The `s` suffix creates a *temporary* `std::string`. The view latches onto that temporary's characters — and then the temporary is destroyed at the end of the statement, leaving `name` dangling immediately. The safe versions view long-lived storage instead:

```cpp
std::string_view a { "Alex" };   // views a C-style literal: lives the whole program
std::string_view b { "Alex"sv }; // sv literal: also fine, also long-lived
```

> **Warning:** Never initialize a `std::string_view` from a `std::string` *temporary* (including a `"..."s` literal). The temporary dies at the end of the expression and your view dangles on the very next line.

### Modifying a string can invalidate views of it

Even a view of a perfectly valid `std::string` can go stale if you *change* that string:

```cpp
std::string s { "short" };
std::string_view sv { s };

s = "a much longer string that may reallocate its buffer";
// sv may now be dangling or stale
```

Why? Because growing a string may force it to allocate a *new, larger* buffer and abandon the old one:

```text
before:
  s owns buffer A: "short"
  sv points into A

after assignment:
  s now owns buffer B: "a much longer..."
  buffer A may be freed
  sv still points into A → invalid
```

And even if the buffer is reused rather than reallocated, the view may keep its *old length*, so it shows a truncated piece or trailing garbage. The rule is simple: **don't trust a view after its underlying string changes.**

### Revalidating a view

A dangling or stale view isn't permanently broken — you just have to re-point it at something valid:

```cpp
sv = s;        // re-view the current contents of s
sv = "reset";  // or view a long-lived literal
```

### Returning a string_view safely

Returning views is where lifetime reasoning pays off. The danger:

```cpp
std::string_view getName()
{
    std::string name { "Ada" };
    return name; // BAD: name dies when the function returns; the view dangles
}
```

But it's safe to return a view of a *C-style literal*, because literals outlive everything:

```cpp
std::string_view boolName(bool value)
{
    return value ? "true" : "false"; // both literals live for the whole program
}
```

And it's safe-ish to return one of your own `std::string_view` *parameters* — you're handing back a window the caller already owned the data for, so the caller stays responsible for keeping it alive:

```cpp
std::string_view shorter(std::string_view a, std::string_view b)
{
    return (a.length() <= b.length()) ? a : b;
}
```

### Functions that shrink the view

`std::string_view` has a couple of functions that adjust *the window*, never the underlying text:

```cpp
std::string_view sv { "abcdef" };
sv.remove_prefix(2); // window now starts later: views "cdef"
sv.remove_suffix(1); // window now ends earlier: views "cde"
```

The original string or literal is completely unchanged — you've only moved the edges of the window:

```text
original:  a b c d e f \0
view 1:    [a b c d e f]
prefix-2:      [c d e f]
suffix-1:      [c d e]
```

### Views can look at substrings — and may not be null-terminated

Because a view is a pointer plus a length, it can point at the *middle* of a string with no copying. That's powerful, but it has a sharp edge: the text a view sees is **not guaranteed to end in a null terminator.**

```text
underlying storage:  h e l l o \0
view of "ell":         [e l l]
storage after view:          o \0
```

> **Warning:** A C-style API expects a null-terminated string. If you hand it a `std::string_view`'s `.data()`, it may keep reading *past* the end of your window — through whatever characters happen to follow — until it stumbles onto some `'\0'`. When you truly need a null-terminated C string, use a `std::string` and its `.c_str()`.

### Quick guide: string or string_view?

Reach for **`std::string`** when you need to:

- own the text,
- modify the text,
- store the text long-term, independent of where it came from,
- or hand a guaranteed null-terminated C string to an API via `.c_str()`.

Reach for **`std::string_view`** when:

- a function only needs read-only access,
- you want to accept literals, `std::string`s, and views cheaply through one parameter,
- the viewed data is guaranteed to outlive the view,
- or you need a compile-time string symbolic constant (`constexpr std::string_view`).

That last contrast is the whole lab in miniature: the badge functions *build* text — so they create and return owning `std::string`s — while every read-only input is a `std::string_view`, and the fixed labels are `constexpr std::string_view` constants.

---

## 5.x — Chapter 5 summary and quiz

### The big ideas

- A **named constant** attaches an identifier to a value that shouldn't change; a **literal** is a value written directly into the source.
- Prefer `constexpr` named constants over **magic numbers** — they give the value meaning and a single place to change it.
- `const` prevents reassignment after initialization. `constexpr` does that *and* requires the value to be known at compile time.
- A **compile-time constant** is known during compilation; a **runtime constant** is fixed after initialization but only known once the program runs.
- The **as-if rule** lets the compiler rewrite your program however it likes, as long as observable behavior is unchanged — which is what enables constant folding, propagation, and dead-code elimination.
- Literals have **types**, and **suffixes** (`u`, `L`, `f`, `s`, `sv`) can change them.
- C-style string literals are **null-terminated** and live for the whole program.
- `std::string` **owns** modifiable text but can be expensive to copy.
- `std::string_view` cheaply **views** existing text — but can **dangle** if that text changes or disappears.

### A decision table to keep

| You need | Reach for |
|---|---|
| A named numeric constant known at compile time | `constexpr int` / `constexpr double` |
| A named value fixed only after runtime input | `const` |
| Modifiable, owned text | `std::string` |
| A read-only string parameter | `std::string_view` |
| A compile-time string label | `constexpr std::string_view` |
| A raw literal whose meaning is unclear | Don't — name it |

### Mistakes worth recognizing on sight

```cpp
const int x;                     // error: a const must be initialized
```

```cpp
constexpr int age { readAge() };  // error: a runtime call can't initialize a constexpr
```

```cpp
std::string_view sv { "hello"s }; // bad: dangling view onto a temporary std::string
```

```cpp
std::string text { "abc" };
std::string_view view { text };
text = "a much longer value";     // view may now be stale or dangling
```

```cpp
int value { 012 };                // not twelve — octal literal, equals decimal 10
```

### Mini drill — the three tools together

This little program uses a compile-time constant, a string label, an owning `std::string`, and a `std::string_view` parameter all at once:

```cpp
#include <iostream>
#include <string>
#include <string_view>

constexpr int maxMutationsPerSeed { 10 };
constexpr std::string_view defaultCampaign { "MutationA" };

std::string makeInput(std::string_view seed)
{
    std::string input { seed }; // copy, because we plan to modify it
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

Why each choice is right:

- `defaultCampaign` is a compile-time string label → `constexpr std::string_view`.
- `seed` is only read → `std::string_view` parameter (no copy, accepts a literal).
- `input` must be owned and modified → `std::string`.
- `maxMutationsPerSeed` names a number that would otherwise be magic → `constexpr int`.

### On to the lab

The chapter exercise, the **Name-Badge Formatter**, is this whole chapter made concrete. You'll write six small const-correct functions that assemble badge text from a person's first and last name. Read-only inputs are `std::string_view`; every result you *build* is returned as an owning `std::string`; and the org name, separator, and width are `constexpr` named constants rather than literals sprinkled through the code. Two moments to watch for: copying a view into an owning string must be **explicit** (`std::string { kOrgName }`), and calling `.front()` on an **empty** view is undefined behavior — so guard each name with `if (name.size() > 0)` before reading its first letter. Turn the grader from red to green, and you'll have used every idea in this chapter for real.
