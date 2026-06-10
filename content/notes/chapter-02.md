# Chapter 2 — Functions and Files

> Source: <https://www.learncpp.com/> (Chapter 2)
> One file per chapter. Each lesson is a section below.

## Contents
- [2.1 — Introduction to Functions](#21--introduction-to-functions)
- [2.2 — Function Return Values (Value-Returning Functions)](#22--function-return-values-value-returning-functions)
- [2.3 — Void Functions (Non-Value-Returning Functions)](#23--void-functions-non-value-returning-functions)
- [2.4 — Introduction to Function Parameters and Arguments](#24--introduction-to-function-parameters-and-arguments)
- [2.5 — Introduction to Local Scope](#25--introduction-to-local-scope)
- [2.6 — Why Functions Are Useful, and How to Use Them Effectively](#26--why-functions-are-useful-and-how-to-use-them-effectively)
- [2.7 — Forward Declarations and Definitions](#27--forward-declarations-and-definitions)
- [2.8 — Programs with Multiple Code Files](#28--programs-with-multiple-code-files)
- [2.9 — Naming Collisions and an Introduction to Namespaces](#29--naming-collisions-and-an-introduction-to-namespaces)
- [2.10 — Introduction to the Preprocessor](#210--introduction-to-the-preprocessor)
- [2.11 — Header Files](#211--header-files)
- [2.12 — Header Guards](#212--header-guards)
- [2.13 — How to Design Your First Programs](#213--how-to-design-your-first-programs)
- [2.x — Chapter 2 summary and quiz](#2x--chapter-2-summary-and-quiz)

---

## 2.1 — Introduction to Functions

- **Function** = *"a reusable sequence of statements designed to do a particular
  job."* The tool for organizing a growing program into modular, testable pieces.
- **Caller / callee:** the function doing the calling is the **caller**; the one
  being called is the **callee**. The act is a **function call** (or invocation).
- **Control flow metaphor (bookmark):** the CPU runs statements in one function,
  hits a call, **suspends** the caller, runs the callee, then **returns** to
  resume the caller right where it left off.

### Syntax

```cpp
returnType functionName() // function HEADER (name + return type)
{                         // function BODY (the braces + statements)
    // statements
}
```

### Two sources of functions
- **Library functions** — come with the C++ standard library.
- **User-defined functions** — you write them.
Call either with `name()` — *don't forget the parentheses.*

### Example (control transfer)

```cpp
#include <iostream>

void doPrint()
{
    std::cout << "In doPrint()\n";
}

int main()
{
    std::cout << "Starting main()\n";
    doPrint();                      // call transfers control to doPrint
    std::cout << "Ending main()\n";
    return 0;
}
```
Output:
```
Starting main()
In doPrint()
Ending main()
```

- Functions can be **called multiple times** (each call runs the whole body).
- Functions can **call other functions** → call chains (main → doA → doB), with
  control returning back up the chain.
- **Nested functions are NOT allowed in C++** — you cannot define a function
  inside another function.

> **Warning:** forgetting the `()` means the function isn't called (and often
> won't compile).

---

## 2.2 — Function Return Values (Value-Returning Functions)

A **value-returning function** computes a value and sends it back to the caller.

- **Return type** — written before the function name; says *what kind* of value
  comes back (e.g. `int getValueFromUser()`).
- **`return` statement** — `return expr;` evaluates `expr` and sends a **copy**
  of the result (the **return value**) back, then exits the function.

```cpp
int returnFive()
{
    return 5;
}

int main()
{
    std::cout << returnFive() << '\n';     // prints 5
    std::cout << returnFive() + 2 << '\n'; // prints 7
    returnFive();                          // legal — value just ignored
    return 0;
}
```

The **caller decides** what to do with the return value — use it in an
expression, store it, or ignore it.

### `main`'s return value = status code

- `main` returns an `int` **status code** to the OS: **`0` = success**, non-zero
  = failure.
- Portable named constants live in `<cstdlib>`: **`EXIT_SUCCESS`**,
  **`EXIT_FAILURE`**. Only `0`, `EXIT_SUCCESS`, `EXIT_FAILURE` are standard.
- `main` **implicitly returns 0** if you omit `return` — but
  > **Best practice:** explicitly `return 0;` at the **bottom** of `main`.
- You **cannot call `main()` yourself** in C++ (only the OS starts it).

### Critical rules

> **A value-returning function must return a value on ALL code paths.** Failing
> to (e.g. running off the end of an `int` function) is **undefined behavior**.
> Modern compilers warn about this.

- A function returns **exactly one value** per call (multiple-value workarounds
  come later).

### DRY

**Don't Repeat Yourself** — factor repeated logic into a function/variable.
```cpp
int getValueFromUser()
{
    std::cout << "Enter an integer: ";
    int input{};
    std::cin >> input;
    return input;          // send the value back
}

int main()
{
    int x{ getValueFromUser() };  // reuse instead of duplicating the prompt logic
    int y{ getValueFromUser() };
    std::cout << x + y << '\n';
    return 0;
}
```
> Document what a return value *means* in a comment above the function.
> **Caveat:** DRY is a guideline — splitting into too-tiny pieces hurts clarity.
> **Aside:** the (sarcastic) opposite of DRY is **WET** — "Write Everything
> Twice."

---

## 2.3 — Void Functions (Non-Value-Returning Functions)

- **`void`** return type = the function returns **nothing**.

```cpp
void printHi()
{
    std::cout << "Hi" << '\n';
}   // automatically returns to caller at the end — no return needed
```

- A void function **auto-returns** at the closing brace. You *may* use a bare
  **`return;`** to exit early, but a trailing one is **redundant**.
  > **Best practice:** *don't* put a `return;` at the end of a void function.
- **Cannot return a value:** `return 5;` in a void function = **compile error**.
- **Cannot be used where a value is required:**
  ```cpp
  std::cout << printHi(); // ERROR — printHi() produces no value
  printHi();              // OK — called on its own
  ```
> **Tip:** some contexts require a value (e.g. the right of `<<`), others don't
> (a standalone statement). Value-returning functions can be called and have
> their value ignored; void functions simply have no value to give.

---

## 2.4 — Introduction to Function Parameters and Arguments

This is how data flows **into** a function (return values flow data out).

- **Parameter** = a variable declared in the function header.
- **Argument** = the actual value supplied by the caller.

```cpp
void printValue(int x)  // x is a PARAMETER
{
    std::cout << x << '\n';
}

int main()
{
    printValue(6);      // 6 is an ARGUMENT
}
```

### Pass by value

On a call, *"the value of each argument is **copied** into the matching
parameter"* (via initialization). Each parameter is an independent **copy**.

Multiple parameters are comma-separated:
```cpp
int add(int x, int y) { return x + y; }

std::cout << add(4, 5) << '\n';          // 9
std::cout << add(1, add(2, 3)) << '\n';  // inner add(2,3)=5 → add(1,5)=6
```

### Parameters + return values = reusability

> **Key insight:** together they let you write functions that *"perform tasks
> and return results without knowing the specific inputs or outputs ahead of
> time."* This is the heart of modular/abstract code.

### Unused parameters

A parameter name is **optional**. If a parameter is required but unused, **omit
the name** to avoid an "unused" warning:
```cpp
void doSomething(int /*count*/) { } // unnamed; comment documents intent
```
> **Best practice:** when a parameter exists but isn't used, don't name it.

---

## 2.5 — Introduction to Local Scope

- **Local variable** = a variable defined inside a function body. **Function
  parameters are also local variables.**

```cpp
int add(int x, int y) // x, y are local
{
    int z{ x + y };   // z is local
    return z;
}
```

### Lifetime (a RUNTIME property)

- **Created** when its definition executes (parameters: on function entry).
- **Destroyed** at the end of the enclosing `{ }`, in **reverse order** of
  creation (function end for parameters).
- Lifetime = from creation to that closing brace.

### Scope (a COMPILE-TIME property)

- **Local (block) scope:** an identifier is usable *"from its point of definition
  to the end of the innermost pair of curly braces containing it."*
- **In scope** = usable; **out of scope** = using it is a compile error.

> **Key distinction:** **lifetime** = *when* it exists (runtime); **scope** =
> *where* its name is usable (compile time).

### Independence across functions

Each function's locals are **separate**, even with identical names:
```cpp
int add(int x, int y) { return x + y; } // add's x, y
int main()
{
    int x{ 5 }, y{ 6 };                  // main's x, y — totally distinct
    std::cout << add(x, y) << '\n';
}
```

> **Best practice:** define locals **as close to first use as reasonable** (and
> in the smallest scope). Modern C++ rejects the old C habit of declaring
> everything at the top of a function.
> Use a **parameter** when the caller supplies the value; use a **local** when
> it doesn't.

**Advanced notes:** class objects run a **destructor** before destruction
(using an object after destruction = UB). The compiler may shift
creation/destruction timing for optimization. A **temporary (anonymous) object**
holds a short-lived value, has *no scope*, and dies at the end of the **full
expression**. *"Return by value returns a temporary object holding a copy of the
return value."*

---

## 2.6 — Why Functions Are Useful, and How to Use Them Effectively

### Five benefits
1. **Organization** — a function is *"almost like a mini-program"* written apart
   from `main`.
2. **Reusability** — call repeatedly; even share across programs.
3. **Testing** — test once; no need to retest unless changed.
4. **Extensibility** — change the function once, every caller benefits.
5. **Abstraction** — callers need only the name, inputs, outputs, and where it
   lives — not the internals.

### When to write a function
- A group of statements appears **more than once**.
- Code has a **well-defined task** with clear inputs/outputs (even if used once).
- **One function = one task** (single responsibility).
- **Refactor**: split overly long/complex functions into smaller ones.

> **Note:** don't mix *calculation* with *output* in one function — compute and
> **return** a value, and let the caller decide what to do with it.

---

## 2.7 — Forward Declarations and Definitions

### The problem
The compiler reads **top-to-bottom**. Calling a function *before* it's defined →
*"identifier not found"* error.

### The solution: a forward declaration (function prototype)
Tell the compiler the function **exists** before defining it.
```cpp
int add(int x, int y); // forward declaration (prototype): return type, name,
                       // parameter TYPES, semicolon — NO body
```
- Parameter **names are optional** in a prototype (`int add(int, int);`), but
  > **Best practice:** keep the names for clarity.
- **Tip:** create one by copy/pasting the function header and adding a `;`.

### Declaration vs definition
- **Declaration** — tells the compiler a name exists (+ its type info).
- **Definition** — actually implements/creates it. **Every definition is also a
  declaration.**
- **Pure declaration** — a declaration that isn't a definition (a forward
  declaration).

### The One Definition Rule (ODR) — 3 parts
1. **Within a file:** one definition per function/variable/type per scope.
   *Violation → compiler error.*
2. **Within a program:** one definition per function/variable across all files.
   *Violation → linker error.*
3. **Exception:** types, templates, inline functions/variables may have
   identical definitions across files. *Violation (non-identical) → UB.*

### Why forward declarations matter
- The called function lives in **another file** (can't reorder across files).
- **Circular** calls (A calls B, B calls A) — no single valid ordering.
- Lets you organize code by topic, not by dependency order.

> **Best practice:** fix the **first** reported error, then recompile (later
> errors are often cascades of the first).

**Compile vs link:** if you forward-declare and *call* a function but never
**define** it anywhere, it **compiles** but the **linker** fails with
*"unresolved external symbol."*

---

## 2.8 — Programs with Multiple Code Files

> **Best practice:** new code files get a `.cpp` extension.

### Two-file example
**add.cpp**
```cpp
int add(int x, int y)
{
    return x + y;
}
```
**main.cpp**
```cpp
#include <iostream>

int add(int x, int y); // forward declaration — so main.cpp knows add exists

int main()
{
    std::cout << "The sum of 3 and 4 is: " << add(3, 4) << '\n';
    return 0;
}
```
Compile both: `g++ main.cpp add.cpp -o main`.

### Why this needs a forward declaration
The compiler compiles **each file independently** — *"it does not know about the
contents of other code files, or remember anything from previously compiled
files."* So `main.cpp` needs the forward declaration to know `add` exists; the
**definition** stays in `add.cpp`. The **linker** then connects `main.cpp`'s
call to `add.cpp`'s definition.

> Independent compilation is a feature: files compile in any order, only changed
> files recompile, and naming conflicts between files are reduced.

### Identifier resolution (key insight)
When an identifier is used, it must connect to its definition:
- no declaration/definition in this file → **compiler error**;
- definition in same file → compiler connects it;
- definition in another file → **linker** connects it;
- no definition anywhere → **linker error**.

### Common mistakes
- Forgot the forward declaration → **compiler** error (`identifier not found`).
- Forgot to add `add.cpp` to the project/compile line → **linker** error
  (`unresolved external symbol`).
- **Don't `#include "add.cpp"`** — add the `.cpp` to the build instead.
- Every file using `std::cout`/`std::cin` needs its own `#include <iostream>`
  (each file compiles independently).

---

## 2.9 — Naming Collisions and an Introduction to Namespaces

### Naming collision
Two identical identifiers the compiler/linker can't tell apart → error.
*(Postal analogy: two "Front Street"s in one city.)*
- Same file → **compiler error**.
- Separate files in one program → **linker error** (happens even if the function
  is never called):
```cpp
// a.cpp
void myFcn(int x) { std::cout << x; }
// main.cpp
void myFcn(int x) { std::cout << 2 * x; } // linker: duplicate definition
int main() { return 0; }
```

### Scope regions & namespaces
- A **scope region** = an area where all declared identifiers are distinct. Two
  identical names in *different* scope regions don't conflict.
- A **namespace** provides a named scope region for declarations/definitions, so
  names inside are isolated from names elsewhere.
- > **Key insight:** *"A namespace may only contain declarations and
  > definitions"* — no bare executable statements (those must be inside a
  > function definition).

```cpp
namespace math {
    double pi { 3.14 };    // OK (definition)
    void calculate() { }   // OK (definition)
}
```

### Global namespace
Anything not inside a class/function/namespace lives in the **global namespace**
(global scope). Global identifiers are visible from their declaration to end of
file. Global *variables* are allowed but **generally discouraged**.

### The `std` namespace
The C++ standard library lives in namespace **`std`** (so `cout` etc. don't
collide with your names). You must tell the compiler a name lives in `std`.

### Accessing namespace members
**1. Scope resolution operator `::`** — explicit and recommended:
```cpp
std::cout << "Hello world!"; // "cout, which lives in std"
```
`name::thing` = "`thing` inside namespace `name`." Nothing on the left of `::` =
the global namespace. A name with a namespace prefix is a **qualified name**.
> **Best practice:** use explicit `std::` prefixes.

**2. Using-declaration** (`using std::cout;`) — imports one name (details later).

**3. Using-directive** (`using namespace std;`) — imports *everything*; **avoid
it**:
```cpp
using namespace std;
int cout() { return 5; }     // your own cout
cout << "Hello, world!";     // ERROR — ambiguous: std::cout or your cout()?
```
> **Warning:** don't put `using namespace std;` at the top of a program or in a
> header. It re-creates the exact collision problem namespaces were invented to
> solve — and future additions to `std` can silently break your code.

---

## 2.10 — Introduction to the Preprocessor

The **preprocessor** runs on each `.cpp` file **before** the compiler, making
**text** edits in memory (originals untouched). Its output — a `.cpp` plus all
its `#include`d headers — is a **translation unit**.

> **Key insight:** preprocess + compile + link together = **translation**.

### Directives
- Start with `#`, end at the **newline** (no semicolon). Separate syntax from C++.
- > **Key insight:** the preprocessor's output contains **no directives** — only
  > the result of processing them goes to the compiler.

### `#include`
Replaces the directive with the **entire contents** of the named file.
```cpp
#include <iostream> // becomes the full text of iostream
```

### Macro `#define`
- **Object-like, with substitution text** — replaces every later occurrence:
  ```cpp
  #define MY_NAME "Alex"
  std::cout << MY_NAME; // → std::cout << "Alex"
  ```
  > **Best practice:** avoid substitution-text macros unless there's no
  > alternative.
- **Object-like, no substitution text** — defines a name with no replacement;
  used as an on/off flag for conditional compilation. Generally fine.
  ```cpp
  #define PRINT_JOE
  ```
- **Function-like macros** — look like functions; considered unsafe — **use real
  functions instead**.
> **Best practice:** macro names in `ALL_UPPERCASE_WITH_UNDERSCORES`.

### Conditional compilation
```cpp
#define PRINT_JOE

#ifdef PRINT_JOE
    std::cout << "Joe\n";   // included (PRINT_JOE defined)
#endif

#ifndef PRINT_BOB
    std::cout << "Bob\n";   // included (PRINT_BOB not defined)
#endif
```
- `#ifdef X` / `#ifndef X` — "if X is (not) defined." Equivalent C++-style forms:
  `#if defined(X)` / `#if !defined(X)`.
- `#if 0` … `#endif` — exclude a block from compilation (handy when the block
  contains `/* */` comments). Flip to `#if 1` to re-enable.

### Scope of `#define`s
The preprocessor doesn't understand C++ scope — a `#define` is active from its
line to the **end of that file only**, and does **not** cross into other files
unless they `#include` it.
```cpp
// function.cpp: chooses output based on whether PRINT is defined IN THIS FILE
// main.cpp: #define PRINT here does NOT affect function.cpp
```
> Note: macro substitution generally does **not** happen *inside* directives
> (e.g. the `X` in `#ifdef X`), with `#if`/`#elif` as exceptions.

---

## 2.11 — Header Files

**Problem:** in big programs, hand-copying forward declarations into every file
is impractical. **Header files** centralize declarations and you `#include` them
where needed.

### Convention
A header (`.h`) is **paired** with a source file of the **same base name**
(`add.h` ↔ `add.cpp`). Headers hold **declarations**; `.cpp` holds definitions.

### Full example
**add.h**
```cpp
int add(int x, int y); // declaration only (prototype)
```
**add.cpp**
```cpp
#include "add.h"        // a source file includes its own paired header

int add(int x, int y)
{
    return x + y;
}
```
**main.cpp**
```cpp
#include "add.h"        // quotes → your own header
#include <iostream>     // angle brackets → library header

int main()
{
    std::cout << "Sum: " << add(3, 4) << '\n';
    return 0;
}
```

### Quotes vs angle brackets
- `#include "add.h"` — **your** headers; searches the **current directory first**,
  then system paths.
- `#include <iostream>` — **system/library** headers; searches **only** system
  paths.

### Critical rules
- **Declarations only in headers** (for now). A *definition* in a header that's
  included by multiple `.cpp` files → ODR violation → linker errors.
- **A source file should `#include` its paired header.** This lets the compiler
  catch mismatches at **compile** time, not link time:
  ```cpp
  // add.h:  int add(int x, int y);
  // add.cpp:
  #include "add.h"
  double add(int x, int y) { ... } // ERROR caught now (return type mismatch)
  ```
- **Never `#include` a `.cpp` file** — add it to the build instead (else
  collisions/ODR/recompiles).

### Include order (best practice)
1. This file's **paired header**
2. Other **project** headers (alphabetical)
3. **Third-party** library headers (alphabetical)
4. **Standard library** headers (alphabetical)

Putting your own headers first maximizes the chance a missing `#include` inside
*your* header gets flagged (a later system header might otherwise hide it).

### Transitive includes
Including a header also pulls in whatever *it* includes (transitive). But:
> **Best practice:** each file must explicitly `#include` everything it uses —
> don't rely on transitive includes (they vary across systems/compilers). ("It
> worked without including `<X>`!" = something else included it transitively.)

### `.h` vs no extension
- **Standard library** headers have **no extension** (`<iostream>`), and put
  names in `std`. (Old `.h` versions put names in the global namespace.)
- **Your** headers should still use **`.h`**.

---

## 2.12 — Header Guards

### The duplicate-definition problem
If a header is included **more than once** into one translation unit, its
contents appear twice → ODR violation → error. This happens easily via transitive
includes:
```cpp
// square.h: int getSquareSides() { return 4; }   // (a definition — for illustration)
// wave.h:   #include "square.h"
// main.cpp: #include "square.h"
//           #include "wave.h"   // square.h pulled in TWICE → redefinition error
```

### The fix: a header guard (include guard)
```cpp
#ifndef SQUARE_H   // if SQUARE_H not yet defined...
#define SQUARE_H   // ...define it, and include the body once

int getSquareSides() { return 4; }

#endif             // SQUARE_H
```
First inclusion: `SQUARE_H` undefined → define it + include body. Second
inclusion in the **same** translation unit: `SQUARE_H` already defined → body
skipped.

### Naming convention
Guard macro = the **filename in ALL CAPS**, punctuation → underscores:
`square.h → SQUARE_H`, `my_math.h → MY_MATH_H`.
> **Advanced:** two `config.h` files in different dirs could collide on
> `CONFIG_H`; use a more unique form like `PROJECT_PATH_FILE_H`.

### What guards do and don't do
> **Key insight:** header guards stop duplicate inclusion **within a single
> translation unit** — they do **not** stop a header from being included into
> **different** `.cpp` files (each is a separate translation unit, and that's
> fine/desired).

So a header containing a **definition** still breaks across multiple `.cpp`
files (each unit gets one copy → linker error). The real fix is the same as
always: **declarations in the header, definition in one `.cpp`:**
```cpp
// square.h
#ifndef SQUARE_H
#define SQUARE_H
int getSquareSides();          // declaration only
#endif
// square.cpp
#include "square.h"
int getSquareSides() { return 4; } // the single definition
```

### Why bother now
Custom types (later) **require** definitions in headers, which makes guards
essential — so build the habit early.

### `#pragma once` (alternative)
```cpp
#pragma once
// contents
```
Simpler and less error-prone, but **not** officially in the C++ standard (though
near-universally supported). One edge case: if a header is **physically
duplicated** on disk and both copies get included, traditional guards de-dupe
(same macro) but `#pragma once` may not (different files).
> The tutorial uses **traditional header guards** as most conventional.

---

## 2.13 — How to Design Your First Programs

> **Key insight:** *"Spending a little time up front thinking about how to
> structure your program will lead to better code and less time spent finding and
> fixing errors."*

### Design steps (before coding)
1. **Define your goal** — one or two sentences, user-facing. ("Let users
   organize names and phone numbers.")
2. **Define requirements** — constraints (budget, time, memory) + capabilities;
   focus on **what**, not **how**. ("Numbers must be saved for later
   retrieval"; "run within 10s".)
3. **Define tools, targets, backup plan** — for now, your IDE; and **back up
   your code** in multiple places (cloud, GitHub, external drive).
4. **Break hard problems into easy ones** — decompose:
   - **Top-down:** start big, recursively split into subtasks.
   - **Bottom-up:** list easy tasks, group them up into a hierarchy.
   The hierarchy maps to code: main task → `main()`, subtasks → functions.
5. **Figure out the sequence of events** — order the tasks (calculator: get #1 →
   get op → get #2 → calculate → print).

### Implementation steps
1. **Outline `main`** as a skeleton of (commented) function calls:
   ```cpp
   int main()
   {
       // getUserInput();
       // calculate();
       // printResult();
       return 0;
   }
   ```
2. **Implement each function**: (a) define its prototype (inputs/outputs),
   (b) write the body, (c) **test it immediately** with temporary code.
   *"Don't implement your entire program in one go."*
3. **Final testing** against all requirements.

### Words of advice
- **Keep it simple to start;** add features over time.
- **Focus on one area at a time** (finish a task before starting another).
- **Test each piece as you go** (catch problems before they cascade).
- **Don't perfect early code** — programs evolve.
- **Optimize for maintainability, not performance** — *"premature optimization
  is the root of all evil"* (Knuth). Reduce redundancy; improve readability.

> *"A complex system that works is invariably found to have evolved from a simple
> system that worked."* — John Gall

---

## 2.x — Chapter 2 summary and quiz

### Core takeaways

- Functions package reusable sequences of statements behind a name.
- Function calls temporarily transfer control to the callee, then return to the
  caller.
- Value-returning functions should return a value on every meaningful path.
- `void` functions perform work without producing a return value.
- Parameters are variables in the function definition; arguments are the values
  supplied by the caller.
- Local variables have block scope and automatic storage duration.
- Small functions make programs easier to test, debug, and reason about.
- A declaration tells the compiler an entity exists; a definition actually
  creates the function/object/type.
- A forward declaration lets the compiler accept a call before seeing the
  function body.
- Multi-file programs compile each `.cpp` file separately into translation
  units, then link them together.
- Namespaces reduce naming collisions.
- The preprocessor runs before compilation and handles directives such as
  `#include`.
- Header files should contain declarations and reusable type definitions.
- Header guards prevent duplicate inclusion inside one translation unit.
- Good program design starts with a clear goal, requirements, decomposition, and
  incremental testing.

### CS6340 lens

Lab code is split across headers and source files. When you see:

```cpp
#include "Instrument.h"
```

read it as:

```text
this source file needs declarations from Instrument.h
```

When a linker error says a symbol is undefined, ask:

```text
Did I declare the function but forget to provide exactly one compiled definition?
```
