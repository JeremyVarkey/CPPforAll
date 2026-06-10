# Chapter 7 — Scope, Duration, and Linkage

> Source: <https://www.learncpp.com/> (Chapter 7)
> One file per chapter. Each lesson is a section below.

## Contents
- [7.1 — Compound statements (blocks)](#71--compound-statements-blocks)
- [7.2 — User-defined namespaces and the scope resolution operator](#72--user-defined-namespaces-and-the-scope-resolution-operator)
- [7.3 — Local variables](#73--local-variables)
- [7.4 — Introduction to global variables](#74--introduction-to-global-variables)
- [7.5 — Variable shadowing (name hiding)](#75--variable-shadowing-name-hiding)
- [7.6 — Internal linkage](#76--internal-linkage)
- [7.7 — External linkage and variable forward declarations](#77--external-linkage-and-variable-forward-declarations)
- [7.8 — Why (non-const) global variables are evil](#78--why-non-const-global-variables-are-evil)
- [7.9 — Inline functions and variables](#79--inline-functions-and-variables)
- [7.10 — Sharing global constants across multiple files (using inline variables)](#710--sharing-global-constants-across-multiple-files-using-inline-variables)
- [7.11 — Static local variables](#711--static-local-variables)
- [7.12 — Scope, duration, and linkage summary](#712--scope-duration-and-linkage-summary)
- [7.13 — Using declarations and using directives](#713--using-declarations-and-using-directives)
- [7.14 — Unnamed and inline namespaces](#714--unnamed-and-inline-namespaces)
- [7.x — Chapter 7 summary and quiz](#7x--chapter-7-summary-and-quiz)

---

## 7.1 — Compound statements (blocks)

### Blocks let multiple statements act like one statement

A **compound statement**, usually called a **block**, is a group of zero or more
statements enclosed in braces.

```cpp
{
    int x { 5 };
    int y { 7 };
    std::cout << x + y << '\n';
}
```

C++ treats the block as one statement. Blocks are used for function bodies,
`if`/`else` bodies, loop bodies, and manually introduced nested scopes.

```cpp
int add(int left, int right)
{
    return left + right;
}
```

There is no semicolon after the closing brace of an ordinary block.

### Blocks inside blocks

Blocks can be nested.

```cpp
int main()
{
    int outer { 10 };

    {
        int inner { 3 };
        std::cout << outer + inner << '\n';
    }

    return 0;
}
```

The inner block can see names from the outer block. The outer block cannot see
names that are declared only in the inner block.

```
main block
|
+-- outer is visible here
|
+-- nested block
|   |
|   +-- outer and inner are visible here
|
+-- inner is gone here
```

### Blocks with conditionals

An `if` statement controls one statement. A block is one statement, so braces let
an `if` control multiple statements.

```cpp
void printStatus(int failures)
{
    if (failures == 0)
    {
        std::cout << "all tests passed\n";
        std::cout << "safe to continue\n";
    }
    else
    {
        std::cout << failures << " tests failed\n";
        std::cout << "inspect before continuing\n";
    }
}
```

Even when the body has one statement, braces are often clearer and reduce future
edit mistakes.

### Nesting depth

Deep nesting makes code harder to understand.

```cpp
void classify(int value)
{
    if (value > 0)
    {
        if ((value % 2) == 0)
        {
            std::cout << "positive even\n";
        }
    }
}
```

If a function needs more than a few nested levels, consider extracting helper
functions or using early returns.

```cpp
bool isPositiveEven(int value)
{
    return (value > 0) && ((value % 2) == 0);
}

void classify(int value)
{
    if (isPositiveEven(value))
        std::cout << "positive even\n";
}
```

CS6340 tie-in:

LLVM pass code can become nested quickly because it often walks modules,
functions, basic blocks, and instructions:

```cpp
for (Function& function : module)
{
    if (function.isDeclaration())
        continue;

    for (BasicBlock& block : function)
    {
        for (Instruction& instruction : block)
        {
            inspect(instruction);
        }
    }
}
```

Early `continue` statements and helper functions keep nesting readable.

---

## 7.2 — User-defined namespaces and the scope resolution operator

### Why namespaces exist

As a program grows, more identifiers are introduced. More identifiers means more
opportunities for name collisions.

```cpp
int countTests();
int countTests(); // same scope and same signature: collision
```

Namespaces create named regions for declarations.

```cpp
namespace Coverage
{
    int countTests()
    {
        return 12;
    }
}

namespace Mutation
{
    int countTests()
    {
        return 4;
    }
}
```

The two functions can have the same unqualified name because their full names
are different:

```cpp
Coverage::countTests()
Mutation::countTests()
```

### Defining a namespace

```cpp
namespace ProjectTools
{
    void printBanner()
    {
        std::cout << "CS6340 tools\n";
    }
}
```

Namespaces must be defined in global scope or inside another namespace. You
normally indent the declarations inside the namespace one level.

### Scope resolution operator

The `::` operator qualifies a name with a scope.

```cpp
ProjectTools::printBanner();
```

Read it as:

```
look inside ProjectTools for printBanner
```

The same operator is used constantly in standard library and LLVM code:

```cpp
std::cout
std::string_view
llvm::Function
llvm::Instruction
```

### The global namespace form

If `::` appears without a left-hand scope, it means the global namespace.

```cpp
int value { 1 };

namespace Demo
{
    int value { 2 };

    void printBoth()
    {
        std::cout << value << '\n';   // Demo::value
        std::cout << ::value << '\n'; // global value
    }
}
```

Use this sparingly. It can clarify a rare shadowing case, but relying on global
variables is usually the deeper problem.

### Forward declarations must be in the same namespace

If a function lives in a namespace, its declaration and definition must agree.

```cpp
// math_tools.h
namespace MathTools
{
    int clampToZero(int value);
}
```

```cpp
// math_tools.cpp
#include "math_tools.h"

namespace MathTools
{
    int clampToZero(int value)
    {
        return (value < 0) ? 0 : value;
    }
}
```

If the declaration were written in the global namespace but the definition were
inside `MathTools`, the compiler and linker would be talking about different
functions.

### Namespace blocks can be split

You can define the same namespace in multiple files or multiple places.

```cpp
namespace Config
{
    inline constexpr int maxMutants { 1000 };
}

namespace Config
{
    inline constexpr int defaultSeed { 1 };
}
```

C++ treats both blocks as contributing to the same namespace. This is how
standard library headers all add declarations to `std`.

Do not add your own declarations to `std`. That namespace is reserved for the
standard library except for a few specialized customization cases that are not
part of this beginner workflow.

### Nested namespaces

Namespaces can be nested.

```cpp
namespace Cs6340
{
    namespace Coverage
    {
        int defaultCounter()
        {
            return 0;
        }
    }
}
```

C++17 allows a compact spelling:

```cpp
namespace Cs6340::Coverage
{
    int defaultCounter()
    {
        return 0;
    }
}
```

Use nested namespaces when the names model real ownership. Avoid creating deep
namespace trees just to look organized.

---

## 7.3 — Local variables

### Local variables are defined inside functions

A **local variable** is defined inside a function body or block. Function
parameters behave like local variables for scope and lifetime purposes.

```cpp
int maxOf(int left, int right) // left and right are local to this function
{
    int larger { (left > right) ? left : right };
    return larger;
}
```

### Block scope

A local variable has **block scope**. It is visible from its definition to the
end of the block where it was defined.

```cpp
int main()
{
    int x { 5 }; // x enters scope here

    {
        int y { 7 }; // y enters scope here
        std::cout << x + y << '\n';
    } // y leaves scope here

    // y is not usable here

    return 0;
} // x leaves scope here
```

Scope is a compile-time rule. If you use a name outside its scope, the compiler
rejects the program.

### Automatic storage duration

Most local variables have **automatic storage duration**. They are created when
execution reaches their definition and destroyed when the block exits.

```cpp
void printOnce()
{
    int callsThisTime { 1 };
    std::cout << callsThisTime << '\n';
} // callsThisTime is destroyed here
```

Each call creates a fresh local object.

### No linkage

Local variables have **no linkage**. Another declaration with the same name in a
different scope is a different object.

```cpp
void first()
{
    int count { 1 };
}

void second()
{
    int count { 2 }; // unrelated to first()'s count
}
```

This is a feature: local variables are naturally isolated.

### Define variables in the most limited existing scope

Prefer the smallest existing scope that still covers all uses.

```cpp
void printIfCovered(bool isCovered)
{
    if (isCovered)
    {
        std::string_view label { "covered" };
        std::cout << label << '\n';
    }
}
```

Do not create a new block only to shrink a variable's scope if that makes the
function harder to read. If a chunk of code wants its own local world, a helper
function is often better.

CS6340 tie-in:

```cpp
bool shouldInstrument(const Instruction& instruction)
{
    bool isTerminator { instruction.isTerminator() };
    bool mayThrow { instruction.mayThrow() };

    return isTerminator || mayThrow;
}
```

The variables are local to the decision. Nothing else in the pass can accidentally
reuse or mutate them.

---

## 7.4 — Introduction to global variables

### Global variables are declared outside functions

A **global variable** is declared outside a function. It can be in the global
namespace or inside a namespace.

```cpp
int g_runCount {}; // global namespace

namespace Settings
{
    int maxRuns { 10 }; // also a global variable
}
```

Variables inside namespaces are still global variables because they live outside
functions and have static duration.

### Global scope / file scope

A global identifier can be used from its point of declaration to the end of the
file.

```cpp
int g_counter {};

void increment()
{
    ++g_counter;
}

int main()
{
    increment();
    std::cout << g_counter << '\n';
}
```

If a global is in a namespace, qualify it:

```cpp
namespace Counters
{
    int hits {};
}

void recordHit()
{
    ++Counters::hits;
}
```

### Static duration

Global variables have **static duration**. They are created when the program
starts and destroyed when the program ends.

```cpp
int g_totalEvents {}; // zero-initialized before main starts
```

Unlike ordinary local variables, globals without explicit initializers are
zero-initialized.

### Naming globals

Some codebases prefix globals with `g_`:

```cpp
int g_activeMutants {};
```

If globals are inside a namespace, the namespace qualifier often makes their
global nature visible:

```cpp
Config::maxMutants
```

The larger rule is more important than the prefix: keep global variables rare,
obvious, and documented.

### Constant globals

Global constants are common and usually much safer than mutable globals.

```cpp
namespace Limits
{
    inline constexpr int maxGeneratedMutants { 1000 };
    inline constexpr double defaultTimeoutSeconds { 2.5 };
}
```

For C++17 and newer, `inline constexpr` in a header is the normal way to share
global constants. Chapter 7.10 covers this in detail.

---

## 7.5 — Variable shadowing (name hiding)

### Shadowing means an inner name hides an outer name

If a nested scope declares a name that already exists in an outer scope, the
inner declaration **shadows** the outer one.

```cpp
int main()
{
    int count { 5 };

    {
        int count { 10 }; // shadows outer count
        std::cout << count << '\n'; // 10
    }

    std::cout << count << '\n'; // 5
}
```

The two variables are different objects. Assigning to the inner one does not
modify the outer one.

### Shadowing globals

Local variables can shadow global variables.

```cpp
int value { 5 };

int main()
{
    int value { 7 };

    std::cout << value << '\n';   // local value
    std::cout << ::value << '\n'; // global value
}
```

The `::value` spelling reaches the global namespace, but needing it often means
the names should be improved.

### Avoid shadowing

Shadowing makes code harder to audit because a reader has to ask "which object
does this name mean here?"

```cpp
void analyze(int count)
{
    for (int count { 0 }; count < 10; ++count) // bad: shadows parameter
    {
        std::cout << count << '\n';
    }
}
```

Prefer distinct names:

```cpp
void analyze(int requestedCount)
{
    for (int index { 0 }; index < requestedCount; ++index)
    {
        std::cout << index << '\n';
    }
}
```

Many compilers can warn on shadowing. GCC and Clang support `-Wshadow` variants.

CS6340 tie-in:

In LLVM pass code, avoid reusing names like `I`, `BB`, `F`, or `M` in nested
scopes if it makes references ambiguous.

```cpp
for (Function& function : module)
{
    for (BasicBlock& block : function)
    {
        for (Instruction& instruction : block)
        {
            inspect(function, block, instruction);
        }
    }
}
```

Verbose names are often better while learning.

---

## 7.6 — Internal linkage

### Linkage answers whether declarations refer to the same entity

An identifier's **linkage** determines whether the same name in another scope or
translation unit can refer to the same object or function.

Three beginner categories:

| Linkage | Meaning |
|---|---|
| none | every declaration is a separate entity |
| internal | usable only inside one translation unit |
| external | can be connected across translation units |

A **translation unit** is roughly one `.cpp` file after preprocessing has pulled
in included headers.

### Internal linkage keeps names local to one translation unit

An identifier with internal linkage can be used within its translation unit but
is not accessible from other translation units.

```cpp
// coverage_helpers.cpp
static int g_nextCounterId { 0 };

static int allocateCounterId()
{
    return g_nextCounterId++;
}
```

Another `.cpp` file can have its own `g_nextCounterId`; the two variables are
independent.

### Globals and internal linkage

Non-const global variables have external linkage by default, but `static` gives
them internal linkage.

```cpp
static int g_fileLocalCounter {};
```

Const and constexpr globals have internal linkage by default:

```cpp
const int maxLocalPasses { 4 };
constexpr int defaultStride { 8 };
```

The reason is practical: constants often need to live in headers and be usable
as compile-time values in each translation unit that includes them.

### Functions and internal linkage

Functions have external linkage by default. `static` can make a free function
file-local:

```cpp
static bool isInterestingOpcode(unsigned opcode)
{
    return opcode == 42;
}
```

Modern C++ often prefers unnamed namespaces for groups of file-local helpers,
covered in 7.14.

### Why use internal linkage?

Internal linkage is useful when:

- A helper should not be part of the file's public interface.
- A name is generic and should not collide across the program.
- You want the linker to reject accidental cross-file use.

CS6340 tie-in:

```cpp
namespace
{
    bool shouldSkipInstruction(const Instruction& instruction)
    {
        return instruction.isDebugOrPseudoInst();
    }
}
```

This helper belongs to one pass implementation file. Keeping it internal makes
that ownership explicit.

---

## 7.7 — External linkage and variable forward declarations

### External linkage crosses translation units

An identifier with **external linkage** can be connected across source files.

```cpp
// printer.cpp
#include <iostream>

void printReady()
{
    std::cout << "ready\n";
}
```

```cpp
// main.cpp
void printReady(); // function forward declaration

int main()
{
    printReady(); // linker connects this call to printer.cpp
}
```

Functions have external linkage by default.

### Global variables with external linkage

Non-const globals have external linkage by default.

```cpp
int g_totalRuns { 0 }; // definition, external by default
```

To use this variable from another `.cpp` file, declare it with `extern` and no
initializer:

```cpp
extern int g_totalRuns; // declaration, definition lives elsewhere
```

The definition creates the object. The `extern` declaration tells the compiler
the object exists and asks the linker to connect it.

### `extern` has context-sensitive meaning

```cpp
extern int g_value;      // declaration only; defined elsewhere
extern const int g_limit { 10 }; // definition with external linkage
```

For non-const globals, avoid `extern` on the definition:

```cpp
int g_value { 3 };        // normal definition
extern int g_value { 3 }; // legal-ish but suspicious style
```

Use `extern` for global variable forward declarations and for const global
definitions that intentionally need external linkage.

### Constexpr globals cannot be usefully forward-declared as constexpr

`constexpr` means the compiler needs the value at compile time. A forward
declaration does not provide the value.

```cpp
extern constexpr int maxItems; // not a useful pattern
```

If a constant must be usable in compile-time contexts across files, define it in
a header as `inline constexpr` in C++17 or newer.

### Header pattern for external variables

For mutable globals, a header may contain only declarations:

```cpp
// counters.h
#ifndef COUNTERS_H
#define COUNTERS_H

extern int g_totalRuns;

#endif
```

And exactly one `.cpp` file contains the definition:

```cpp
// counters.cpp
#include "counters.h"

int g_totalRuns { 0 };
```

This pattern is mechanically valid, but mutable globals are still usually a
design smell. Prefer passing state explicitly or wrapping state in an object.

---

## 7.8 — Why (non-const) global variables are evil

### The main problem is hidden mutable state

Mutable globals can be read or modified from many places. That makes a function
call less predictable.

```cpp
int g_mode { 0 };

void configureForDebug()
{
    g_mode = 2;
}

void run()
{
    g_mode = 1;
    configureForDebug();

    if (g_mode == 1)
        std::cout << "normal\n";
    else
        std::cout << "debug\n";
}
```

The call to `configureForDebug()` silently changes `run()`'s later decision.
With local variables and parameters, the data flow is easier to see.

### Debugging globals scales poorly

If a global has the wrong value, every function that can access it becomes a
suspect.

```
local variable:
    inspect the current function and callers

global variable:
    search the whole program for every write
```

Globals also make tests harder because one test can leave state that affects the
next test.

### Globals reduce modularity

A function that uses only parameters and return values is easier to reuse.

```cpp
bool shouldStop(int failures, int maxFailures)
{
    return failures >= maxFailures;
}
```

This is better than:

```cpp
int g_failures {};
int g_maxFailures { 10 };

bool shouldStop()
{
    return g_failures >= g_maxFailures;
}
```

The first function can be tested with any inputs. The second function depends on
ambient program state.

### Static initialization order

Objects with static duration are initialized before `main()`. Simple constants
and zero-initialized globals are straightforward. Dynamic initialization is more
dangerous, especially across translation units.

```cpp
int makeLimit();
int g_limit { makeLimit() }; // dynamic initialization before main
```

If one global's initializer depends on another global in a different `.cpp`
file, the initialization order may not be what you expect. This family of bugs
is often called the static initialization order problem.

Prefer compile-time constants, local variables, or function-local statics when
you need controlled initialization.

### When globals are acceptable

Non-const globals are rare but not impossible to justify. Common examples:

- a process-wide logger,
- a global random number engine,
- a single configuration registry in a small tool.

Even then, reduce damage:

```cpp
namespace Logging
{
    void setVerbose(bool enabled);
    bool isVerbose();
}
```

Hide the raw mutable object and expose a narrow interface.

CS6340 tie-in:

For a lab pass, mutable globals can feel convenient for counters. Prefer pass
members, local state, or explicit objects first. If a counter truly must be
file-wide, make that ownership obvious and keep the variable internal to the
implementation file.

---

## 7.9 — Inline functions and variables

### Inline expansion is an optimization

Function calls have overhead: arguments are prepared, control jumps to the
function, and control returns. For tiny functions, the overhead can matter more
than the body.

**Inline expansion** is when the compiler replaces a function call with the
function body.

```cpp
int minValue(int left, int right)
{
    return (left < right) ? left : right;
}

int x { minValue(3, 4) };
```

The compiler might optimize as though the call were written directly:

```cpp
int x { (3 < 4) ? 3 : 4 };
```

Modern compilers decide this for themselves. Do not use `inline` merely to ask
for performance.

### Modern `inline` means multiple identical definitions are allowed

The modern practical meaning of `inline` is ODR-related:

> this function or variable may be defined in multiple translation units, as
> long as every definition is identical.

This enables definitions in headers.

```cpp
// math_helpers.h
#ifndef MATH_HELPERS_H
#define MATH_HELPERS_H

inline int clampNegativeToZero(int value)
{
    return (value < 0) ? 0 : value;
}

#endif
```

Every `.cpp` file that includes the header gets the same definition. The linker
deduplicates the inline definitions.

### Requirements for inline functions

For an inline function with external linkage:

- the full definition must be visible in each translation unit where it is used,
- there can be only one definition per translation unit,
- every definition across the program must be identical.

If definitions differ, the program has undefined behavior.

### Header-only libraries

Inline functions are common in header-only libraries because there is no `.cpp`
file to compile and link separately.

```cpp
// bit_count.h
#ifndef BIT_COUNT_H
#define BIT_COUNT_H

inline bool isPowerOfTwo(unsigned value)
{
    return value != 0 && ((value & (value - 1)) == 0);
}

#endif
```

The tradeoff is build time. If a header changes, every translation unit that
includes it may need recompilation.

### Inline variables

C++17 added inline variables. They follow the same "multiple identical
definitions are allowed" idea.

```cpp
namespace BuildInfo
{
    inline constexpr int courseYear { 2026 };
}
```

`constexpr` functions are implicitly inline, but ordinary `constexpr` variables
are **not** automatically inline. Use `inline constexpr` when a header constant
should be one shared entity across translation units.

---

## 7.10 — Sharing global constants across multiple files (using inline variables)

### The problem

Some constants belong in one central place:

```cpp
namespace MutationConfig
{
    constexpr int defaultSeed { 1 };
    constexpr int maxGeneratedMutants { 1000 };
}
```

If many `.cpp` files need these constants, duplicating definitions manually
breaks the "don't repeat yourself" rule. But putting definitions in headers has
linkage consequences.

### Option 1: constexpr constants in a header, internal linkage

Before C++17, a common solution was a header containing `constexpr` constants.

```cpp
// constants.h
#ifndef CONSTANTS_H
#define CONSTANTS_H

namespace Constants
{
    constexpr int maxAttempts { 3 };
    constexpr double timeoutSeconds { 1.5 };
}

#endif
```

Because non-inline `constexpr` globals have internal linkage by default, every
translation unit gets its own copy. This avoids ODR violations and lets the
compiler use the constants in compile-time contexts.

Downsides:

- every including translation unit may get a copy,
- changing the header causes recompilation of all includers.

For small constants, this is often acceptable.

### Option 2: extern const in a `.cpp` file

You can put one definition in a `.cpp` file and declarations in a header.

```cpp
// constants.h
#ifndef CONSTANTS_H
#define CONSTANTS_H

namespace Constants
{
    extern const int maxAttempts;
}

#endif
```

```cpp
// constants.cpp
#include "constants.h"

namespace Constants
{
    extern const int maxAttempts { 3 };
}
```

This creates one object for the whole program. But other translation units see
only a declaration, not the value. That means the constant usually cannot be
used as a compile-time constant outside `constants.cpp`.

### Option 3: inline constexpr in a header

For C++17 and newer, this is the usual best answer:

```cpp
// constants.h
#ifndef CONSTANTS_H
#define CONSTANTS_H

namespace Constants
{
    inline constexpr int maxAttempts { 3 };
    inline constexpr double timeoutSeconds { 1.5 };
}

#endif
```

Benefits:

- the value is visible in every translation unit that includes the header,
- it can be used in compile-time contexts,
- the linker can deduplicate the definitions,
- the constants live in one source location.

Downside:

- changing the header still triggers recompilation of includers.

### String constants

Use `std::string_view` for compile-time string constants.

```cpp
#include <string_view>

namespace Labels
{
    inline constexpr std::string_view covered { "covered" };
    inline constexpr std::string_view uncovered { "uncovered" };
}
```

### CS6340 header pattern

For a small lab helper header:

```cpp
// lab_constants.h
#ifndef LAB_CONSTANTS_H
#define LAB_CONSTANTS_H

#include <string_view>

namespace LabConstants
{
    inline constexpr int defaultMutationLimit { 100 };
    inline constexpr std::string_view counterPrefix { "__cs6340_counter_" };
}

#endif
```

This is safer than macros and easier to audit than scattered literals.

---

## 7.11 — Static local variables

### `static` local variables keep state across calls

Ordinary local variables have automatic duration:

```cpp
void printAutomatic()
{
    int value { 1 };
    ++value;
    std::cout << value << '\n';
} // value is destroyed here
```

Every call creates a fresh `value`, so this prints the same number each time.

A `static` local variable has block scope but static duration:

```cpp
void printStatic()
{
    static int s_value { 1 };
    ++s_value;
    std::cout << s_value << '\n';
} // s_value becomes inaccessible, but is not destroyed here
```

The variable is initialized once and retains its value between calls.

```
scope:
    visible only inside printStatic()

duration:
    exists until program termination
```

### Initialize static locals

Always initialize static local variables.

```cpp
int nextId()
{
    static int s_nextId { 0 };
    return s_nextId++;
}
```

The initializer runs once. Later calls reuse the existing object.

The `s_` prefix is a common convention for static-duration local variables.

### Unique ID generation

Static locals are useful when a function needs private remembered state.

```cpp
int allocateCounterId()
{
    static int s_nextCounterId { 0 };
    return s_nextCounterId++;
}
```

Only `allocateCounterId()` can access `s_nextCounterId`, but the value persists
between calls.

This is often safer than a mutable global because the name is not visible across
the file or program.

### Static local constants

Static local constants can avoid repeated expensive construction while keeping a
name local to the function.

```cpp
const std::vector<std::string>& knownPassNames()
{
    static const std::vector<std::string> s_names {
        "coverage",
        "mutation",
        "fuzzing"
    };

    return s_names;
}
```

Use this pattern when:

- the object is expensive to build,
- the same object can be reused,
- global visibility is not needed.

### Hidden state can still surprise readers

Static locals are not automatically good. They are hidden mutable state if the
value changes.

```cpp
int calls()
{
    static int s_count { 0 };
    return ++s_count;
}
```

This may be exactly right for an ID generator. It is risky for general business
logic because repeated calls no longer depend only on arguments.

For testable code, prefer explicit state unless persistence is the purpose.

---

## 7.12 — Scope, duration, and linkage summary

### Scope

**Scope** answers: where can this name be used in source code?

| Scope | Meaning | Examples |
|---|---|---|
| block scope | from declaration to end of block | local variables, parameters |
| global scope | from declaration to end of file | functions, namespace-scope variables |

```cpp
void f(int parameter) // parameter has block scope for the function
{
    int local { 0 }; // local has block scope
}
```

### Duration

**Duration** answers: when is the object created and destroyed?

| Duration | Created | Destroyed | Examples |
|---|---|---|---|
| automatic | when definition is reached | when block exits | ordinary locals, parameters |
| static | program startup | program termination | globals, static locals |
| dynamic | programmer request | programmer request | dynamically allocated objects |

Most beginner code should strongly prefer automatic duration.

### Linkage

**Linkage** answers: can declarations in different scopes or translation units
refer to the same entity?

| Linkage | Meaning | Examples |
|---|---|---|
| none | same name elsewhere is a separate entity | local variables |
| internal | same name can refer to same entity only within one translation unit | `static` globals, const globals, unnamed namespaces |
| external | declarations can refer to same entity across the program | ordinary functions, non-const globals, inline variables |

### Combined variable table

| Kind | Example | Scope | Duration | Linkage |
|---|---|---|---|---|
| ordinary local | `int x {};` | block | automatic | none |
| static local | `static int s_x {};` | block | static | none |
| non-const global | `int g_x {};` | global | static | external |
| internal global | `static int g_x {};` | global | static | internal |
| const global | `constexpr int x { 1 };` | global | static | internal |
| inline constant | `inline constexpr int x { 1 };` | global | static | external |
| extern declaration | `extern int g_x;` | global | none by itself | refers to external definition |

### Translation-unit mental model

```
source file:
    main.cpp

preprocessor:
    expands #include directives

translation unit:
    main.cpp plus included header text

compiler:
    compiles this unit mostly independently

linker:
    connects external names across units
```

Most header/linker confusion comes from forgetting that headers are copied into
each including translation unit before compilation.

---

## 7.13 — Using declarations and using directives

### Qualified and unqualified names

A **qualified name** includes a scope.

```cpp
std::cout
llvm::Function
::globalValue
```

An **unqualified name** does not.

```cpp
cout
Function
globalValue
```

Qualified names are more explicit and reduce naming collisions.

### Using-declarations

A using-declaration introduces one qualified name so it can be used unqualified
in the current scope.

```cpp
#include <iostream>

int main()
{
    using std::cout;

    cout << "hello\n";
}
```

The using-declaration is active from its declaration to the end of the scope.
It is reasonably safe inside `.cpp` files when narrowly scoped.

```cpp
void printResult()
{
    using std::cout;
    using std::endl;

    cout << "done" << endl;
}
```

For these notes and most LLVM code, explicit `std::` is usually clearer.

### Using-directives

A using-directive makes all names from a namespace available for unqualified
lookup in the directive's scope.

```cpp
using namespace std;
```

This is the famous line many older tutorials put near the top of a file. Avoid
it. It imports many names and can create collisions or surprising overload
choices as the standard library evolves.

```cpp
namespace Student
{
    int size() { return 1; }
}

using namespace Student;
using namespace std; // now unqualified names are harder to reason about
```

### Never put using-statements in headers

Headers are copied into every translation unit that includes them. A using
directive in a header leaks into users' files.

```cpp
// bad_header.h
using namespace std; // bad: affects every file that includes this header
```

Best practice:

- Prefer explicit namespace qualifiers.
- Avoid using-directives.
- If using-declarations help readability, put them in narrow `.cpp` scopes.
- Do not place using-statements in headers, especially at global scope.

### Common exception

The `std::literals` namespace is sometimes imported to enable literal suffixes
such as `sv` for `std::string_view`.

```cpp
using namespace std::literals;

auto name { "coverage"sv };
```

Even then, keep the using-directive scoped and intentional.

CS6340 tie-in:

Prefer:

```cpp
llvm::Function& function
std::string_view label
```

over global using-directives that hide where names come from. LLVM code already
has many names; explicit qualification reduces search space for a reader.

---

## 7.14 — Unnamed and inline namespaces

### Unnamed namespaces

An unnamed namespace has no identifier:

```cpp
namespace
{
    bool shouldSkip(int opcode)
    {
        return opcode == 0;
    }
}
```

Names inside an unnamed namespace are accessible within the enclosing scope in
the same translation unit, but they have internal linkage. Other translation
units cannot use them directly.

This is a modern way to make a group of helper functions and variables
file-local.

```cpp
namespace
{
    constexpr int maxRetries { 3 };

    bool isRetryable(int code)
    {
        return code != 0;
    }
}
```

Prefer unnamed namespaces when you have implementation details that should stay
local to one `.cpp` file.

Do not put unnamed namespaces in headers. Each translation unit would get its
own separate entities, which can be surprising and can create subtle bugs.

### `static` vs unnamed namespace

For one function, both patterns can express file-local intent:

```cpp
static bool helperA()
{
    return true;
}

namespace
{
    bool helperB()
    {
        return true;
    }
}
```

Unnamed namespaces scale better for groups and can also cover user-defined
types. Many modern C++ codebases prefer unnamed namespaces for `.cpp`-local
implementation details.

### Inline namespaces

An inline namespace is a namespace whose members are treated as part of the
parent namespace for unqualified lookup. Inline namespaces are commonly used for
versioning.

```cpp
namespace Api
{
    inline namespace V1
    {
        int parse()
        {
            return 1;
        }
    }

    namespace V2
    {
        int parse()
        {
            return 2;
        }
    }
}
```

Calls:

```cpp
Api::parse();     // V1, because V1 is inline
Api::V1::parse(); // explicit old version
Api::V2::parse(); // explicit newer version
```

Inline namespaces do not give internal linkage. They are a versioning and API
selection tool, not a file-locality tool.

For CS6340 lab work, you are much more likely to use unnamed namespaces than
inline namespaces.

---

## 7.x — Chapter 7 summary and quiz

### Summary

- A block is a group of statements enclosed in braces and treated as one
  statement.
- Namespaces reduce naming collisions and are accessed with `::`.
- Local variables have block scope, automatic duration, and no linkage.
- Global variables have global scope and static duration. Mutable globals should
  be rare.
- Shadowing hides an outer name with an inner name. Avoid it.
- Internal linkage keeps an identifier local to one translation unit.
- External linkage allows declarations in different translation units to refer
  to the same entity.
- Use `extern` carefully for global variable declarations. Prefer not needing
  mutable globals at all.
- Modern `inline` means multiple identical definitions are allowed, which is why
  inline functions and inline variables can live in headers.
- In C++17+, `inline constexpr` is the preferred pattern for shared header
  constants.
- Static local variables have block scope but static duration.
- Using-declarations can be acceptable in narrow source-file scopes; using
  directives should generally be avoided.
- Unnamed namespaces are useful for `.cpp`-local helpers. Inline namespaces are
  mainly for versioning.

### Header/source-file checklist

When reading or writing multi-file C++:

1. If it is a normal function declaration in a header, put the definition in one
   `.cpp` file unless it is intentionally inline.
2. If it is a small header-defined function, mark it `inline` unless another
   rule already makes it inline.
3. If it is a shared compile-time constant in C++17+, prefer `inline constexpr`
   in a namespace in a header.
4. If it is helper code used only by one `.cpp`, put it in an unnamed namespace
   in that `.cpp`.
5. If it is mutable global state, ask whether it can be local state, a parameter,
   a return value, or an object member instead.

### CS6340-facing mental model

LLVM lab code will often have:

```cpp
namespace
{
    bool shouldInstrument(const Instruction& instruction)
    {
        return !instruction.isDebugOrPseudoInst();
    }
}

bool Instrument::runOnFunction(Function& function)
{
    int insertedCounters { 0 };

    for (BasicBlock& block : function)
    {
        for (Instruction& instruction : block)
        {
            if (!shouldInstrument(instruction))
                continue;

            ++insertedCounters;
        }
    }

    return insertedCounters > 0;
}
```

Read it with this chapter's vocabulary:

- `shouldInstrument` has internal linkage because it is in an unnamed namespace.
- `insertedCounters` is a local variable with block scope and automatic
  duration.
- `function`, `block`, and `instruction` are local reference variables in nested
  blocks.
- No mutable global state is needed for the pass decision.
