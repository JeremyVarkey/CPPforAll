# Chapter 7 — Scope, Duration, and Linkage

> Source: <https://www.learncpp.com/> (Chapter 7)

Every name you write in a C++ program — every variable, every function — lives somewhere, exists for some span of time, and is either private to one file or shared across many. Up to now you've used these properties without naming them: a local variable disappears at the end of its function, two functions in different files can call each other, a constant is visible from where you declared it. This chapter pulls those three ideas into the open and gives them precise names — **scope**, **duration**, and **linkage** — because once a program grows past a single file, these are exactly the questions that decide whether your code compiles, links, and behaves. We'll also meet the tools that let you control them: namespaces to carve up the world of names, `static` and `extern` to tune linkage, and `inline` to share definitions across files. The payoff is real: by the end you'll be able to organize a multi-file program the way professionals do, and you'll understand the rules that make headers work.

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

### Many statements, treated as one

In a lot of places, C++ syntax expects exactly *one* statement. An `if` controls one statement. A `while` loop body is one statement. A function body is, syntactically, one thing. But real code almost always needs to do several things in those spots — and that's where the block comes in.

A **compound statement**, almost always just called a **block**, is a group of zero or more statements enclosed in curly braces. The key insight is what the braces buy you:

> **Key insight:** C++ treats an entire block as a single statement. So anywhere the grammar allows one statement, a block lets you supply many. That's the whole trick — braces are how "do several things" fits into a slot that wanted "do one thing."

```cpp
{
    int x { 5 };
    int y { 7 };
    std::cout << x + y << '\n';
}
```

You've already been writing blocks all along — every function body is one:

```cpp
int add(int left, int right)
{
    return left + right;
}
```

One small but persistent gotcha: an ordinary block does **not** take a semicolon after its closing brace. A block *is* a statement; it doesn't need a `;` to become one.

### Blocks inside blocks

Blocks can nest inside other blocks, as deeply as you like. When they do, an important visibility rule kicks in.

```cpp
int main()
{
    int outer { 10 };

    {
        int inner { 3 };
        std::cout << outer + inner << '\n'; // can see both
    }

    // inner is gone here; outer is still fine

    return 0;
}
```

The rule is one-directional. An **inner block can see names from the enclosing block** — that's why `inner` can reach out and use `outer`. But the **outer block cannot see names declared only inside the inner block** — once that inner pair of braces closes, `inner` is gone.

Picture the nesting as a set of fences. Each name lives inside the fence where it was born, and can see everything in its own fence and every fence around it — but nothing in a fence it's enclosed by can be seen from outside:

```text
main block
|
+-- outer is visible here
|
+-- nested block
|   |
|   +-- outer and inner are both visible here
|
+-- inner is gone here
```

### Blocks make conditionals do more

This is the everyday reason you reach for a block. An `if` statement governs a single statement — but you usually want a branch to do several things. Wrap them in braces, and the whole group becomes the one statement the `if` controls:

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

> **Best practice:** Use braces on the bodies of `if`/`else` and loops even when the body is a single statement today. It costs two characters and prevents a classic bug — the day someone adds a second line to the branch and forgets that, without braces, only the *first* line was ever controlled by the `if`.

### Keep nesting shallow

Blocks nest freely, but *deeply* nested code is genuinely hard to read — each level of indentation is another condition you have to hold in your head to understand what runs.

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

When a function starts stacking up nesting levels, two cures usually help: combine the conditions into a single expression, or pull a named helper function out so the intent reads in one line.

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

The second version is shorter *and* says what it means. As a rough rule of thumb, if a function needs more than a few levels of nesting, that's a signal to extract a helper or restructure with early returns.

> **Note (CS6340):** This matters more than it looks in the labs. LLVM pass code naturally nests deeply, because it walks a module, then each function inside it, then each basic block, then each instruction. An early `continue` to skip uninteresting cases, plus small named predicates, are what keep that four-deep walk readable rather than a pyramid of braces.

```cpp
for (Function& function : module)
{
    if (function.isDeclaration())
        continue; // bail early instead of nesting the rest

    for (BasicBlock& block : function)
    {
        for (Instruction& instruction : block)
            inspect(instruction);
    }
}
```

---

## 7.2 — User-defined namespaces and the scope resolution operator

### Why namespaces exist

Here's a problem that's invisible in a tiny program and unavoidable in a big one. Every name you introduce — every function, every global — competes for the same pool of identifiers. The bigger the program (and the more libraries it pulls in), the higher the chance that two unrelated pieces of code both want the name `count`, or `init`, or `parse`. When two declarations with the same name and signature land in the same scope, that's a **naming collision**, and the compiler can't let it stand:

```cpp
int countTests();
int countTests(); // same scope, same signature — collision
```

A **namespace** is the cure. It's a named region you put declarations inside, so that names in one namespace don't clash with the same names in another. Think of it like an area code on a phone number: two people can both have the number 5550100 as long as they're in different area codes — the full number is what has to be unique.

```cpp
namespace Coverage
{
    int countTests() { return 12; }
}

namespace Mutation
{
    int countTests() { return 4; }
}
```

Both functions are spelled `countTests`, and that's fine — their *full* names differ:

```cpp
Coverage::countTests();
Mutation::countTests();
```

You've already been on the receiving end of this: the entire standard library lives in the namespace `std`, which is exactly why it's `std::cout` and not just `cout`. That `std::` prefix isn't noise — it's the area code that keeps the library's hundreds of names from colliding with yours.

### Defining a namespace

You define a namespace with the `namespace` keyword and a name, then put your declarations in the braces:

```cpp
namespace ProjectTools
{
    void printBanner()
    {
        std::cout << "CS6340 tools\n";
    }
}
```

A namespace must be defined at **global scope** (outside any function) or nested inside another namespace — you can't open one inside a function body. By convention, indent the declarations one level inside the braces, the way you would a block.

### The scope resolution operator

The `::` symbol is the **scope resolution operator**. It says "look inside the scope on my left for the name on my right."

```cpp
ProjectTools::printBanner(); // look inside ProjectTools for printBanner
```

Read `Coverage::countTests` aloud as "the `countTests` that lives in `Coverage`." This is the same operator you've been reading in every standard-library and (in the labs) LLVM name:

```cpp
std::cout
std::string_view
llvm::Function
llvm::Instruction
```

### The global namespace and the leading `::`

There's also an unnamed, all-encompassing scope called the **global namespace** — it's where everything declared outside any namespace lives. If you write `::` with *nothing* on its left, it means "the global namespace." That gives you a way to reach a global name even when a local name of the same spelling is in the way:

```cpp
int value { 1 }; // in the global namespace

namespace Demo
{
    int value { 2 };

    void printBoth()
    {
        std::cout << value << '\n';   // Demo::value (the nearer one) -> 2
        std::cout << ::value << '\n'; // the global value             -> 1
    }
}
```

Use the leading `::` sparingly. It's the right tool for the rare, deliberate case where you must reach past a closer name — but if you find yourself reaching for it often, the deeper issue is usually that you're leaning on globals. (We'll meet this `::name` form again in the chapter lab, where you'll use it on purpose.)

### A declaration and its definition must share a namespace

If a function belongs to a namespace, *both* its forward declaration and its definition have to be written inside that same namespace. Otherwise the compiler thinks you're talking about two different functions.

```cpp
// math_tools.h
namespace MathTools
{
    int clampToZero(int value); // declaration, inside MathTools
}
```

```cpp
// math_tools.cpp
#include "math_tools.h"

namespace MathTools
{
    int clampToZero(int value) // definition, inside the same namespace
    {
        return (value < 0) ? 0 : value;
    }
}
```

Declare `clampToZero` in the global namespace but define it inside `MathTools`, and you'd have a declaration the linker can never satisfy — they name different entities.

### A namespace can be reopened

You don't have to define a whole namespace in one place. You can open the same namespace in several spots — even across different files — and every block contributes to the one shared namespace:

```cpp
namespace Config
{
    inline constexpr int maxMutants { 1000 };
}

// ...elsewhere, or in another file...

namespace Config
{
    inline constexpr int defaultSeed { 1 };
}
```

This is precisely how the standard library works — dozens of headers each reopen `std` to add their own pieces. One firm rule comes with this power, though:

> **Warning:** Never add your own declarations to namespace `std`. It is reserved for the standard library, and adding to it is undefined behavior (apart from a few advanced customization points that are well outside a beginner's workflow).

### Nested namespaces

Namespaces can live inside other namespaces, which lets your name structure mirror real ownership:

```cpp
namespace Cs6340
{
    namespace Coverage
    {
        int defaultCounter() { return 0; }
    }
}
```

Since C++17 there's a compact spelling for the same thing:

```cpp
namespace Cs6340::Coverage
{
    int defaultCounter() { return 0; }
}
```

> **Tip:** Nest namespaces when the layers model something real — `Cs6340::Coverage` reads like an actual ownership relationship. Don't build deep namespace trees just to look organized; each level a reader has to type and remember should earn its place.

---

## 7.3 — Local variables

### Variables that live inside functions

A **local variable** is any variable defined inside a function body or inside a block. You've been writing them since Chapter 1; now we name them and pin down their three properties precisely. One detail worth stating up front: **function parameters are local variables too** — they behave exactly like locals for the purposes of scope and lifetime.

```cpp
int maxOf(int left, int right) // left and right are local to maxOf
{
    int larger { (left > right) ? left : right }; // also local
    return larger;
}
```

### Block scope

A local variable has **block scope**: it is in scope (usable by name) from the point of its definition to the closing brace of the block it lives in — and nowhere else.

```cpp
int main()
{
    int x { 5 }; // x enters scope here

    {
        int y { 7 }; // y enters scope here
        std::cout << x + y << '\n';
    } // y leaves scope here

    // using y here would not compile

    return 0;
} // x leaves scope here
```

Scope is a **compile-time** concept — it's about which names the compiler will let you *write*. Use a name outside its scope and the program simply won't compile; nothing happens at run time because the program never runs.

### Automatic storage duration

Scope is about *where in the source* a name is usable. **Duration** is a separate question: *when does the object exist in memory?* Most local variables have **automatic storage duration**, which means the object is created when execution reaches its definition and automatically destroyed when its block exits.

```cpp
void printOnce()
{
    int callsThisTime { 1 };
    std::cout << callsThisTime << '\n';
} // callsThisTime is destroyed here
```

A direct consequence: **every call gets a fresh object.** Call `printOnce()` a hundred times and you've created and destroyed a hundred separate `callsThisTime` variables, each starting from scratch. Remember this — it's exactly the behavior that `static` (lesson 7.11) will change.

### No linkage

Local variables have **no linkage**. Linkage is the question of whether two declarations of the same name refer to the *same* entity; "no linkage" means each declaration is its own separate object, full stop. Two locals named `count` in two different functions are simply unrelated:

```cpp
void first()  { int count { 1 }; }
void second() { int count { 2 }; } // a different object entirely
```

This isn't a limitation — it's a feature. Local variables are naturally isolated, so you never have to worry that naming a loop counter `i` here might disturb an `i` somewhere else.

### Define variables in the smallest scope that works

> **Best practice:** Define each variable in the most limited *existing* scope that still covers everywhere you use it.

The narrower a variable's scope, the less code can touch it, and the less a reader has to track. If a value is only needed inside an `if`, define it inside the `if`:

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

Two cautions on the "smallest scope" rule. Don't invent an artificial nested block *just* to shrink a variable's scope if doing so makes the function harder to follow — readability wins. And if a chunk of code really wants its own little world of local variables, that's often a hint that it should become its own helper function instead.

> **Note (CS6340):** In pass code you'll write tiny predicates whose locals exist only to make one decision — and that's exactly where this rule pays off:
> ```cpp
> bool shouldInstrument(const Instruction& instruction)
> {
>     bool isTerminator { instruction.isTerminator() };
>     bool mayThrow { instruction.mayThrow() };
>     return isTerminator || mayThrow;
> }
> ```
> Nothing outside this function can reuse or mutate `isTerminator` or `mayThrow` — they live and die inside the decision.

---

## 7.4 — Introduction to global variables

### Variables that live outside every function

A **global variable** is a variable defined *outside* of any function. Where a local lives inside a function's braces, a global lives at file scope — either in the global namespace or inside a namespace you've opened.

```cpp
int g_runCount {}; // global, in the global namespace

namespace Settings
{
    int maxRuns { 10 }; // also a global variable
}
```

That second one is the part beginners often miss: **a variable inside a namespace is still a global variable.** It lives outside any function and has static duration (we'll get there in a moment). Being in a namespace changes its *name*, not its global nature.

> **Tip:** Many codebases prefix global variable names with `g_` (as in `g_runCount`) so that, at every use site, you can see "this is global" without scrolling up to find the definition. It's a convention, not a rule — but a helpful one.

### Global scope (file scope)

A global identifier is in scope from its point of declaration to the end of the file. That's why a function defined *below* a global can use it freely:

```cpp
int g_counter {};

void increment()
{
    ++g_counter; // fine: g_counter is in scope here
}

int main()
{
    increment();
    std::cout << g_counter << '\n'; // prints 1
}
```

If the global lives in a namespace, you qualify it with `::` like any other namespaced name:

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

### Static duration — and zero-initialization

Global variables have **static storage duration**: they're created once, when the program starts (before `main` runs), and destroyed once, when the program ends. They exist for the entire life of the program — which is the deep reason they can be seen and shared so widely.

```cpp
int g_totalEvents {}; // created at program startup, lives until shutdown
```

Static duration brings one convenient difference from locals. An uninitialized *local* holds garbage, but a global variable with no explicit initializer is **zero-initialized** — numeric globals start at `0`, automatically. (Writing `{}` as we do above makes that explicit, which is the clearer habit.)

### Constant globals are the good kind

Most of this chapter will argue against *mutable* globals. But constant globals — values that never change — are common, useful, and far safer, because the danger of a global is that someone *modifies* it from afar, and a constant can't be modified at all.

```cpp
namespace Limits
{
    inline constexpr int maxGeneratedMutants { 1000 };
    inline constexpr double defaultTimeoutSeconds { 2.5 };
}
```

For C++17 and newer, `inline constexpr` inside a namespace is the standard way to declare a shared global constant — and lesson 7.10 is devoted entirely to *why* that exact incantation, and what each piece of it does. For now, just register the shape: constant globals are normal and welcome; mutable ones are the ones we'll be careful with.

---

## 7.5 — Variable shadowing (name hiding)

### An inner name can hide an outer one

What happens if a nested scope declares a name that *already exists* in an enclosing scope? The inner declaration wins locally — it **shadows** (also called *name hiding*) the outer one. Within the inner scope, the name refers to the inner variable, and the outer variable is temporarily unreachable by that name.

```cpp
int main()
{
    int count { 5 };

    {
        int count { 10 };           // shadows the outer count
        std::cout << count << '\n'; // 10 — the inner one
    }

    std::cout << count << '\n';     // 5 — the outer one is back, untouched
}
```

The crucial point: these are **two different objects.** Assigning to the inner `count` does nothing to the outer `count`. The inner one simply ceases to exist when its block ends, and the name `count` goes back to meaning the outer variable.

### Shadowing a global

A local can shadow a global the same way. Here the leading `::` from lesson 7.2 earns its keep — it's the escape hatch that still reaches the global past the local that's hiding it:

```cpp
int value { 5 }; // global

int main()
{
    int value { 7 }; // local, shadows the global

    std::cout << value << '\n';   // 7 — the local
    std::cout << ::value << '\n'; // 5 — the global, reached past the local
}
```

> **Note:** Needing `::value` to reach your own global is usually a sign the names should be improved, not a pattern to celebrate. The chapter lab will have you write exactly this — a local that deliberately shadows a global, returned via `::` — specifically so you can *feel* the mechanism. In real code, you'd rename one of them.

### Just don't do it

Shadowing is legal, but it makes code hard to audit: a reader has to stop and ask "which `count` is this, here?" every time the name appears. The cost is highest when the shadowed name is a parameter, because the two are so close together:

```cpp
void analyze(int count)
{
    for (int count { 0 }; count < 10; ++count) // bad: shadows the parameter
        std::cout << count << '\n';
}
```

> **Best practice:** Avoid shadowing variables. Give the inner variable a distinct, more specific name instead.

```cpp
void analyze(int requestedCount)
{
    for (int index { 0 }; index < requestedCount; ++index)
        std::cout << index << '\n';
}
```

You don't have to police this entirely by hand. GCC and Clang can warn on shadowing with `-Wshadow` (and its variants) — turning it on catches accidental shadows before they bite.

> **Note (CS6340):** LLVM code is full of short names like `I`, `BB`, `F`, and `M` (instruction, basic block, function, module). Reusing those across nested loops is a fast road to ambiguous, shadow-prone code. While you're learning, prefer the verbose names — `instruction`, `block`, `function`, `module` — so a reference is never in doubt.

---

## 7.6 — Internal linkage

### Linkage, finally defined

We've now met "no linkage" in passing. It's time to define the term properly, because the next several lessons turn on it. An identifier's **linkage** answers one question:

> **Key insight:** Linkage asks — *can another declaration of this same name refer to the same entity?* No linkage: no, each is separate. Internal linkage: yes, but only within one file. External linkage: yes, even across different files.

There are three categories you need at this stage:

| Linkage | Meaning |
|---|---|
| none | every declaration is a separate entity |
| internal | the entity is usable only within one translation unit |
| external | declarations across translation units can refer to the same entity |

That phrase **translation unit** is the unit linkage is measured in, so it's worth nailing down. A translation unit is, roughly, one `.cpp` file *after the preprocessor has run* — meaning after every `#include` has been physically pasted in. The compiler processes one translation unit at a time, largely in isolation; the **linker** is the later step that stitches them together. Keep that picture handy: it explains nearly everything about linkage.

### Internal linkage keeps a name file-local

An identifier with **internal linkage** can be used freely within its own translation unit, but is *invisible* to every other translation unit. A different `.cpp` file can declare its own name with the same spelling, and the two are completely independent objects.

```cpp
// coverage_helpers.cpp
static int g_nextCounterId { 0 }; // internal linkage: private to this file

static int allocateCounterId()    // internal linkage too
{
    return g_nextCounterId++;
}
```

Another `.cpp` file could have its *own* `g_nextCounterId`, and the two would never interfere — each is sealed inside its own translation unit.

### How to get internal linkage

The rules differ by what kind of entity you have:

- **Non-const global variables** have *external* linkage by default, but the `static` keyword gives them internal linkage:
  ```cpp
  static int g_fileLocalCounter {}; // now internal
  ```
- **Const and constexpr global variables** have internal linkage *by default* — no `static` needed:
  ```cpp
  const int maxLocalPasses { 4 };
  constexpr int defaultStride { 8 };
  ```
  Why the difference? Because constants frequently need to live in headers and be usable as compile-time values in every file that includes them. Default internal linkage means each translation unit gets its own private copy, which avoids cross-file conflicts. (Lesson 7.10 revisits this and shows when you'd want to override it.)
- **Functions** have external linkage by default, but `static` makes a free function file-local:
  ```cpp
  static bool isInterestingOpcode(unsigned opcode)
  {
      return opcode == 42;
  }
  ```

> **Note:** Modern C++ often prefers an *unnamed namespace* over `static` for grouping file-local helpers — it scales better to many helpers and works for types too. That's lesson 7.14; for now, know that `static` is the older, single-item way to say the same thing.

### Why bother making things internal?

Internal linkage is the right call when:

- a helper is an *implementation detail* and has no business being part of the file's public interface,
- a name is generic (`init`, `helper`, `count`) and you don't want it colliding with the same name elsewhere in the program, or
- you actively *want* the linker to reject any accidental cross-file use, as a safety net.

> **Note (CS6340):** A pass's private helpers belong to one implementation file. Sealing them with internal linkage makes that ownership explicit — and is exactly what the chapter lab has you do with an unnamed namespace:
> ```cpp
> namespace
> {
>     bool shouldSkipInstruction(const Instruction& instruction)
>     {
>         return instruction.isDebugOrPseudoInst();
>     }
> }
> ```

---

## 7.7 — External linkage and variable forward declarations

### External linkage crosses translation units

The flip side of internal linkage is **external linkage**: an entity that *can* be connected across translation units. This is what makes multi-file programs possible at all. When you call a function defined in another `.cpp` file, external linkage is the property that lets the linker connect the call to the definition.

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
void printReady(); // forward declaration: "this exists, defined elsewhere"

int main()
{
    printReady(); // the linker connects this to printer.cpp's definition
}
```

`main.cpp` never sees the *body* of `printReady` — only a forward declaration. The compiler trusts that declaration to type-check the call, and the linker does the real work of matching it up. **Functions have external linkage by default**, which is exactly why this just works.

### Sharing a global variable across files

Non-const global variables *also* have external linkage by default. So you can share one across files — but the mechanics are a little fussier than for functions, and it's worth being precise.

You write the **definition** in exactly one file. The definition is the part with an initializer; it's what actually creates the object:

```cpp
// counters.cpp
int g_totalRuns { 0 }; // definition — creates the object, external by default
```

To *use* that same variable from another file, you write a **forward declaration** with `extern` and **no initializer**:

```cpp
// main.cpp
extern int g_totalRuns; // declaration — "this object exists, defined elsewhere"
```

The distinction is the whole game: the definition (with initializer) makes the object; the `extern` declaration (no initializer) only promises the object exists somewhere and asks the linker to connect to it. Have an initializer and it's a definition; omit it and add `extern` and it's a declaration.

### `extern` is context-sensitive

The `extern` keyword does slightly different things depending on whether there's an initializer, which trips people up:

```cpp
extern int g_value;              // no initializer -> a declaration only
extern const int g_limit { 10 }; // with initializer -> a DEFINITION,
                                 // forcing external linkage onto a const
```

That second line is the genuinely useful "advanced" use: a const normally has *internal* linkage, and `extern` on its definition overrides that to make it shared.

For *non-const* globals, you don't need `extern` on the definition — a plain `int g_value { 3 };` is already an external definition. Putting `extern` on a non-const definition is legal but stylistically confusing, so don't:

```cpp
int g_value { 3 };        // good: the normal definition
extern int g_value { 3 }; // legal but suspicious — avoid
```

> **Best practice:** Use `extern` for two things only — a global variable *forward declaration* (no initializer), and a *const* global definition you intentionally want to have external linkage. For an ordinary non-const global definition, leave `extern` off.

### `constexpr` resists forward declaration

There's one combination that simply doesn't work usefully. `constexpr` means *the compiler must know this value at compile time*. But a forward declaration carries no value — by design, it points elsewhere. So you can't forward-declare a `constexpr` and still use it as a compile-time constant in another file:

```cpp
extern constexpr int maxItems; // not a useful pattern — no value here
```

The proper fix, if a constant must be usable in compile-time contexts across multiple files, is to *define* it in a header as `inline constexpr` — which is exactly what the next lesson is about.

### The header pattern for shared mutable globals

When you genuinely must share a mutable global, the clean arrangement is: declaration in the header, definition in one `.cpp`.

```cpp
// counters.h
#ifndef COUNTERS_H
#define COUNTERS_H

extern int g_totalRuns; // declaration only — safe to include anywhere

#endif
```

```cpp
// counters.cpp
#include "counters.h"

int g_totalRuns { 0 }; // the single definition for the whole program
```

Now any file that includes `counters.h` can use `g_totalRuns`, and there's still exactly one object behind it. The chapter lab uses precisely this split: a global declared `extern` in the header, defined once in your `.cpp`.

> **Warning:** This pattern is mechanically correct, but a *mutable* global is still usually a design smell. Before reaching for one, ask whether you could pass the state as a parameter, return it, or wrap it in an object instead. The next lesson explains why.

---

## 7.8 — Why (non-const) global variables are evil

"Evil" is strong language for a language feature, and it's used half in jest — but the warning behind it is serious. Mutable global variables are one of the most reliable sources of hard-to-find bugs in larger programs, and it's worth understanding *exactly* why, so the rule sticks.

### The core problem: hidden, action-at-a-distance state

A mutable global can be read or written from *anywhere* in the program. That means a function call can quietly change state that some *other*, unrelated-looking function depends on — and nothing at the call site warns you:

```cpp
int g_mode { 0 };

void configureForDebug()
{
    g_mode = 2;
}

void run()
{
    g_mode = 1;
    configureForDebug(); // looks harmless...

    if (g_mode == 1)
        std::cout << "normal\n";
    else
        std::cout << "debug\n"; // ...but THIS branch runs, surprisingly
}
```

Reading `run()` top to bottom, you'd swear `g_mode == 1` when the `if` is reached. It isn't, because `configureForDebug()` reached out and changed it. With local variables and parameters, data flow is visible — you can *see* where a value comes from. With globals, it's invisible.

### Debugging globals doesn't scale

When a *local* has the wrong value, your search is small: this function and its callers. When a *global* has the wrong value, every function that can touch it is a suspect — which, for a global, is potentially the whole program.

```text
local with a bad value:   inspect the current function and its callers
global with a bad value:  search the entire program for every write
```

Globals also poison tests. One test can leave a global in a dirty state that silently changes the outcome of the next test, producing failures that depend on *test order* — a maddening class of bug.

### Globals reduce modularity

A function that talks to the world *only* through its parameters and return value is self-contained: you can lift it out, reuse it, and test it with any inputs you like.

```cpp
bool shouldStop(int failures, int maxFailures) // everything is explicit
{
    return failures >= maxFailures;
}
```

Compare that to the version wired to ambient state:

```cpp
int g_failures {};
int g_maxFailures { 10 };

bool shouldStop() // depends on whatever the globals happen to hold
{
    return g_failures >= g_maxFailures;
}
```

The first is testable with any pair of numbers and reusable anywhere. The second only works inside a program that happens to set up those exact globals first.

### The static initialization order problem

There's a subtler hazard too. Objects with static duration are initialized before `main` runs. For zero-initialized globals and simple constants, that's straightforward. But when a global's initializer is a *function call* — **dynamic initialization** — order starts to matter:

```cpp
int makeLimit();
int g_limit { makeLimit() }; // dynamic initialization, runs before main
```

The catch: when two such globals live in *different* `.cpp` files and one's initializer depends on the other, **the order they initialize in is not guaranteed.** You can end up reading a global that hasn't been initialized yet. This is the notorious **static initialization order problem** — also called the "static initialization order fiasco" — and it's nearly impossible to debug because it has nothing to do with your program's logic.

> **Best practice:** When you need controlled initialization, prefer compile-time constants, ordinary local variables, or function-local statics (lesson 7.11) over globals with non-trivial initializers.

### When a global is actually justified

Mutable globals are *rare*, not *forbidden*. A handful of cases genuinely call for one — a process-wide logger, a single global random-number engine, a configuration registry in a small tool. Even then, contain the damage: hide the raw mutable object and expose only a narrow, deliberate interface to it.

```cpp
namespace Logging
{
    void setVerbose(bool enabled);
    bool isVerbose();
}
```

Users go through `setVerbose` and `isVerbose`; nobody pokes the underlying boolean directly. The global still exists, but its blast radius is small and its access points are searchable.

> **Note (CS6340):** A counter in a lab pass can *feel* like it wants to be a global. Resist first: prefer a pass member, local state, or an explicit object. If a counter truly must be file-wide, keep it *internal* to the implementation file (lesson 7.6) so its reach is one file, not the whole program — which is exactly the discipline the chapter lab builds.

---

## 7.9 — Inline functions and variables

The keyword `inline` is one of the more confusing in C++, because its *original* meaning and its *modern, practically-useful* meaning are two different things. We'll meet both — and then spend most of our time on the one that matters for you.

### The original meaning: inline expansion

Calling a function isn't free. The program has to prepare the arguments, jump to the function's code, and jump back when it returns. For a *tiny* function, that overhead can cost more than the work the function actually does.

**Inline expansion** is the optimization where the compiler replaces a call with a copy of the function's body, eliminating the call overhead:

```cpp
int minValue(int left, int right)
{
    return (left < right) ? left : right;
}

int x { minValue(3, 4) };
```

The compiler is free to optimize that as though you'd written the body directly:

```cpp
int x { (3 < 4) ? 3 : 4 };
```

Historically, the `inline` keyword was a *hint* requesting this. Today that's obsolete:

> **Key insight:** Modern compilers decide inline expansion entirely on their own — they're far better at it than a human guessing. Do **not** add `inline` to ask for performance. That's not what it's for anymore.

### The modern meaning: multiple identical definitions are allowed

So what *is* `inline` for today? It relaxes a rule you've quietly relied on since you first wrote a header — the **one-definition rule** (ODR), which normally says a given function or variable may be defined only once in the whole program. The modern meaning of `inline` is:

> This entity may be defined in **multiple** translation units, as long as **every definition is identical.**

That single permission is what makes it legal to put a function *definition* (not just a declaration) directly in a header:

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

Every `.cpp` file that includes this header gets its *own* copy of the definition. Without `inline`, that would be multiple definitions of the same function across the program — an ODR violation, and a linker error. With `inline`, the linker knows the duplicates are intentional and deduplicates them down to one.

### The rules for an inline function

For an `inline` function with external linkage, three conditions must hold:

- the full definition must be visible in every translation unit that *uses* it (so put it in the header, not a `.cpp`),
- there is still only one definition allowed *per* translation unit, and
- every definition across the whole program must be **byte-for-byte identical**.

> **Warning:** If two translation units see *different* definitions of the same `inline` function, the program has undefined behavior — and the linker usually won't catch it. This is why the "identical" requirement matters: the safe way to guarantee it is to write the definition once, in a header, and include that header everywhere.

### Header-only libraries

This is why **header-only libraries** exist. If every function is `inline` and defined in a header, there's no separate `.cpp` to compile and link — you just `#include` the header and you're done:

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

The tradeoff is build time: because the definition lives in a header, editing it forces every translation unit that includes it to recompile.

### Inline variables

C++17 extended the same idea to *variables*. An **inline variable** may, like an inline function, be defined in multiple translation units as long as the definitions match — which finally makes it clean to define a shared variable in a header:

```cpp
namespace BuildInfo
{
    inline constexpr int courseYear { 2026 };
}

```

Two related facts are worth memorizing, because they catch everyone:

- `constexpr` *functions* are implicitly `inline` — you never write `inline` on them.
- Ordinary `constexpr` *variables* are **not** automatically inline. If you want a header constant to be a single shared entity across translation units, you must write `inline constexpr` explicitly.

That second point is the entire reason the next lesson exists.

---

## 7.10 — Sharing global constants across multiple files (using inline variables)

### The problem

Real programs have constants that belong in *one* central place — limits, defaults, magic numbers given names:

```cpp
namespace MutationConfig
{
    constexpr int defaultSeed { 1 };
    constexpr int maxGeneratedMutants { 1000 };
}
```

If many `.cpp` files need these, you don't want to copy the definitions into each one — that violates "don't repeat yourself," and the day the value changes you'll miss a copy. The natural move is to put them in a header and include it everywhere. But *how* you write the constant in that header has real linkage consequences, and there are three approaches worth knowing. The third is the answer for modern C++; the first two explain why.

### Option 1 — `constexpr` constants in a header (internal linkage)

Before C++17, the common solution was a header full of plain `constexpr` constants:

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

Recall from lesson 7.6 that non-inline `constexpr` globals have *internal linkage by default*. So each translation unit that includes this header gets its **own private copy** of each constant. That's what makes it legal (no ODR violation), and each copy is usable in compile-time contexts within its own file.

The downside: it's literally one copy per including file. For a handful of small integers that's a non-issue, but the duplication is real, and changing the header recompiles every includer.

### Option 2 — `extern const` in one `.cpp`

The opposite approach: put a *single* definition in one `.cpp` and only declarations in the header.

```cpp
// constants.h
namespace Constants { extern const int maxAttempts; } // declaration
```

```cpp
// constants.cpp
namespace Constants { extern const int maxAttempts { 3 }; } // the one definition
```

Now there's exactly one object for the whole program. But there's a serious catch: every *other* translation unit sees only the declaration, not the value. Since the value isn't visible at compile time outside `constants.cpp`, you generally **cannot use this constant in compile-time contexts** elsewhere (array sizes, `constexpr` computations, and so on). That's often a dealbreaker for a constant.

### Option 3 — `inline constexpr` in a header (the modern answer)

C++17's inline variables resolve the tension. Mark the header constants `inline constexpr`, and you get the best of both options:

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

What you get:

- the **value is visible** in every file that includes the header, so it works in compile-time contexts everywhere,
- it's one logical entity — the linker deduplicates the definitions instead of giving you a copy-per-file or a link error,
- the constants live in exactly **one source location** — change it once, everyone updates.

The only real downside is the familiar one: editing the header recompiles its includers.

> **Best practice:** In C++17 and newer, `inline constexpr` inside a namespace in a header is the standard way to share global constants across files. Reach for it by default.

### String constants

For compile-time string constants, the right type is `std::string_view` (from Chapter 5) — it can name a string literal without owning or copying it:

```cpp
#include <string_view>

namespace Labels
{
    inline constexpr std::string_view covered { "covered" };
    inline constexpr std::string_view uncovered { "uncovered" };
}
```

### Putting it together — the lab's header pattern

A small shared-constants header for a lab helper looks exactly like this — and it's the shape you'll find in the chapter exercise's `telemetry.h`:

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

This is safer than a `#define` macro (it's typed, scoped, and respects namespaces) and far easier to audit than the same literals scattered through a dozen `.cpp` files.

---

## 7.11 — Static local variables

This is the most important lesson in the chapter for the lab, so let's take it slowly. It's also where the keyword `static` means something *completely different* from what it meant in lesson 7.6 — there it controlled linkage; here it controls *duration*. Same word, different job.

### The default: locals forget everything

Recall that an ordinary local variable has automatic duration — a fresh object every call. So this prints the *same* number every time, no matter how often you call it:

```cpp
void printAutomatic()
{
    int value { 1 };
    ++value;
    std::cout << value << '\n'; // always 2
} // value is destroyed here
```

Each call builds a brand-new `value`, initialized to `1`, bumps it to `2`, and throws it away. There's no memory between calls.

### `static` gives a local memory

Put `static` in front, and the local keeps its block scope but gains **static duration**:

```cpp
void printStatic()
{
    static int s_value { 1 };
    ++s_value;
    std::cout << s_value << '\n'; // 2, then 3, then 4, ...
} // s_value becomes inaccessible here, but is NOT destroyed
```

Now the variable is initialized *once*, the first time control reaches its definition, and it **survives between calls** — call this function repeatedly and you'll see `2, 3, 4, ...`. The object is not destroyed when the block exits; it simply becomes unreachable by name until the next call.

The cleanest way to hold the idea is to see that `static` splits scope and duration apart, which usually travel together:

```text
scope:     visible only inside printStatic()   (block scope, unchanged)
duration:  exists until the program ends        (static duration, the new part)
```

It's a variable only one function can *see*, but that *remembers* — private, but persistent.

> **Tip:** The `s_` prefix (as in `s_value`) is a widespread convention for static local variables, signaling at every use that "this one persists." The lab uses it throughout.

### Always initialize a static local

```cpp
int nextId()
{
    static int s_nextId { 0 };
    return s_nextId++;
}
```

The initializer runs exactly once — on the first call. Every later call skips it and reuses the existing object. (This "initialize once" guarantee is the whole point; an *uninitialized* static local would defeat it.)

### The classic use: a unique-ID dispenser

The static local's superpower is private, remembered state — and the textbook example is handing out unique IDs:

```cpp
int allocateCounterId()
{
    static int s_nextCounterId { 0 };
    return s_nextCounterId++; // hand out 0, then 1, then 2, ...
}
```

Only `allocateCounterId()` can see `s_nextCounterId` — it's not visible anywhere else in the file or program — yet the value climbs across calls. Compare that to a *global* counter doing the same job: the global is far worse, because the entire program could read or corrupt it, while this one is sealed inside a single function.

> **Note:** This is the exact mechanism behind the chapter lab's `nextCounterId()`, which dispenses `1000, 1001, 1002, ...`. The trick is `return s_nextId++;` — *post*-increment returns the current value and *then* advances, so the first call yields the starting value, not the starting value plus one.

### Static local constants for expensive objects

There's a second, calmer use. If a function needs a costly-to-build object that never changes, a `static const` local builds it once and reuses it on every later call — without exposing it globally:

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

This pattern fits when the object is expensive to construct, the *same* object can be reused, and you don't need it visible outside the function. (Returning by `const` reference here avoids copying the vector on every call — a technique you'll lean on more once you've met references properly.)

### A caution: hidden state can still surprise

`static` locals are not automatically virtuous. A static local whose value *changes* is hidden mutable state — the very thing lesson 7.8 warned about, just scoped down to one function:

```cpp
int calls()
{
    static int s_count { 0 };
    return ++s_count;
}
```

For an ID generator, that's *exactly right* — persistence is the entire point. For general business logic it's risky, because the function's result no longer depends only on its arguments: call it twice with "the same inputs" and you get different answers.

> **Best practice:** Use a static local when remembered state *is* the purpose (counters, ID dispensers, expensive-build caches). For ordinary logic you want to test, prefer explicit state passed in and out — predictability beats cleverness.

---

## 7.12 — Scope, duration, and linkage summary

This lesson is the map. The three concepts we've been building are genuinely independent — a variable has a scope *and* a duration *and* a linkage, chosen somewhat separately — and most confusion comes from blurring them. Here they are, side by side.

### Scope — *where* can the name be used?

Scope is a compile-time property about the *source text*: from where to where may you write this name?

| Scope | Meaning | Examples |
|---|---|---|
| block scope | from its declaration to the end of its block | local variables, parameters |
| global scope | from its declaration to the end of the file | functions, namespace-scope variables |

```cpp
void f(int parameter) // parameter has block scope (the function body)
{
    int local { 0 };  // local also has block scope
}
```

### Duration — *when* does the object exist?

Duration is a run-time property about the object's *lifetime*: when is it created and destroyed?

| Duration | Created | Destroyed | Examples |
|---|---|---|---|
| automatic | when its definition is reached | when its block exits | ordinary locals, parameters |
| static | at program startup | at program termination | globals, static locals |
| dynamic | on programmer request | on programmer request | dynamically allocated objects (a later chapter) |

> **Best practice:** Most beginner code should strongly prefer automatic duration. It's the safest default — objects clean themselves up exactly when their block ends.

### Linkage — can other declarations refer to the *same* entity?

| Linkage | Meaning | Examples |
|---|---|---|
| none | the same name elsewhere is a separate entity | local variables |
| internal | the same name refers to one entity, but only within a single translation unit | `static` globals, const/constexpr globals, unnamed-namespace members |
| external | declarations across translation units can refer to one shared entity | ordinary functions, non-const globals, inline variables |

### The combined cheat-sheet

When you can read this table fluently, you understand the chapter. Each row is a kind of variable and its three properties:

| Kind | Example | Scope | Duration | Linkage |
|---|---|---|---|---|
| ordinary local | `int x {};` | block | automatic | none |
| static local | `static int s_x {};` | block | static | none |
| non-const global | `int g_x {};` | global | static | external |
| internal global | `static int g_x {};` | global | static | internal |
| const global | `constexpr int x { 1 };` | global | static | internal |
| inline constant | `inline constexpr int x { 1 };` | global | static | external |
| extern declaration | `extern int g_x;` | global | (refers elsewhere) | refers to an external definition |

A couple of rows reward a second look. The *static local* and the *ordinary local* share scope and linkage but differ in duration — that one cell is the whole of lesson 7.11. And the *const global* versus *inline constant* differ only in linkage — internal versus external — which is the entire point of lesson 7.10.

### The translation-unit picture

Almost every header-and-linker puzzle dissolves once you hold this pipeline in mind:

```text
source file:       main.cpp
preprocessor:      pastes in every #include directive
translation unit:  main.cpp PLUS all the included header text
compiler:          compiles this unit largely on its own
linker:            connects external names across all the units
```

> **Key insight:** A header is *copied into* every file that includes it, before the compiler ever runs. Most confusion about "why is this defined twice" or "why can't the other file see this" comes from forgetting that single fact.

---

## 7.13 — Using declarations and using directives

By now you've typed `std::` hundreds of times, and you may be wondering whether there's a way to stop. There is — two ways, actually — and this lesson is about which one is safe and which one you should mostly avoid.

### Qualified vs unqualified names

A **qualified name** carries a scope with it, via `::`:

```cpp
std::cout
llvm::Function
::globalValue
```

An **unqualified name** stands alone:

```cpp
cout
Function
globalValue
```

Qualified names are more explicit and far less prone to collisions — you can always tell exactly where the name comes from. The cost is verbosity, and the two `using` forms below are the standard ways to buy back some of that brevity.

### Using-declarations (the safe one)

A **using-declaration** introduces *one specific* qualified name into the current scope, so you can then write it unqualified:

```cpp
#include <iostream>

int main()
{
    using std::cout; // bring just std::cout into this scope

    cout << "hello\n"; // now legal unqualified
}
```

The using-declaration is active from where it appears to the end of its scope. Because it imports exactly one name into a *narrow* scope, it's fairly low-risk:

```cpp
void printResult()
{
    using std::cout;
    using std::endl;

    cout << "done" << endl;
}
```

> **Tip:** A using-declaration inside a function is a reasonable way to cut noise when you use one or two names repeatedly. Keep it local — inside the function or block — not at file scope.

### Using-directives (the famous trap)

A **using-directive** imports *every* name from a namespace for unqualified lookup. This is the line you've seen at the top of countless beginner tutorials:

```cpp
using namespace std; // brings ALL of std into scope
```

It's convenient and it's a bad habit. It pulls hundreds of names into your scope at once, and as the standard library grows, those names can silently collide with — or quietly out-compete — your own:

```cpp
namespace Student
{
    int size() { return 1; }
}

using namespace Student;
using namespace std; // now unqualified `size` is genuinely ambiguous to reason about
```

> **Best practice:** Avoid using-directives (`using namespace ...;`), especially `using namespace std;`. Prefer explicit qualification, and when you want brevity, prefer a narrowly-scoped using-*declaration* instead.

### Never put a `using` statement in a header

This rule is non-negotiable, and the translation-unit picture from lesson 7.12 tells you exactly why. A header is *copied into* every file that includes it — so a `using` statement at the top of a header silently leaks into every one of those files, whether they wanted it or not:

```cpp
// bad_header.h
using namespace std; // BAD: now infects every file that includes this header
```

The full checklist:

- Prefer explicit namespace qualifiers (`std::cout`) as your default.
- Avoid using-directives almost everywhere.
- If a using-*declaration* helps readability, confine it to a narrow `.cpp` scope.
- **Never** place a `using` statement in a header — least of all at global scope.

### A common, narrow exception

One place a using-directive is idiomatic is enabling the standard library's literal suffixes, which live in `std::literals`. For instance, the `sv` suffix makes a `std::string_view` literal:

```cpp
using namespace std::literals;

auto name { "coverage"sv }; // a std::string_view, thanks to the sv suffix
```

Even here, keep the directive scoped and intentional — inside the function that needs the suffix, not blanketing the whole file.

> **Note (CS6340):** LLVM code already swims in names. Writing `llvm::Function& function` and `std::string_view label` explicitly keeps a reader's search space small — they never have to wonder which namespace a name escaped from. Explicit qualification is the house style there for exactly that reason.

---

## 7.14 — Unnamed and inline namespaces

This last lesson covers two namespace features that look similar but solve opposite problems. The first — unnamed namespaces — is one you'll use constantly, including in the chapter lab. The second — inline namespaces — is a specialist tool you should mostly just recognize.

### Unnamed namespaces

An **unnamed namespace** (also called an *anonymous namespace*) is a namespace with no name at all:

```cpp
namespace
{
    bool shouldSkip(int opcode)
    {
        return opcode == 0;
    }
}
```

Here's what it does: every name inside an unnamed namespace is usable in the enclosing scope (you don't qualify it — there's no name to qualify with), but each gets **internal linkage**. In other words, the contents are file-local — other translation units cannot reach them.

That makes the unnamed namespace the modern, preferred way to mark a *group* of helpers and variables as private to one `.cpp` file:

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

> **Best practice:** For implementation details that should stay local to one `.cpp` file, prefer an unnamed namespace.

> **Warning:** Never put an unnamed namespace in a *header*. Since the header is copied into every includer, each translation unit would get its *own separate* copies of everything inside — a recipe for subtle, surprising bugs where files disagree about "the same" entity.

### `static` vs the unnamed namespace

For a *single* file-local function, the older `static` keyword (lesson 7.6) and an unnamed namespace express the same intent:

```cpp
static bool helperA() { return true; } // file-local, the old way

namespace
{
    bool helperB() { return true; }    // file-local, the modern way
}
```

The unnamed namespace pulls ahead for *groups* of helpers (one set of braces instead of repeating `static`) and, crucially, it can also make **user-defined types** file-local, which `static` cannot. That's why modern codebases lean toward unnamed namespaces for their `.cpp`-local implementation details.

### Inline namespaces (a different tool entirely)

Don't let the word `inline` mislead you here — an **inline namespace** has nothing to do with linkage or file-locality. Its members are treated as though they belong to the *parent* namespace for unqualified lookup, and its primary job is **API versioning**:

```cpp
namespace Api
{
    inline namespace V1
    {
        int parse() { return 1; }
    }

    namespace V2
    {
        int parse() { return 2; }
    }
}
```

Because `V1` is `inline`, its members surface directly into `Api`. So an unqualified `Api::parse()` resolves to the `V1` version — the "default" — while callers can still pin a specific version explicitly:

```cpp
Api::parse();     // V1 — because V1 is the inline (default) version
Api::V1::parse(); // explicitly the old version
Api::V2::parse(); // explicitly the newer version
```

When you later make `V2` the inline namespace instead, every unqualified `Api::parse()` call upgrades to V2 with no change at the call sites — that's the versioning trick.

> **Note:** Inline namespaces do **not** give internal linkage — they are a versioning/API-selection tool, not a file-locality tool. For lab work you'll reach for *unnamed* namespaces constantly and *inline* namespaces almost never; just recognize the latter when you see it.

---

## 7.x — Chapter 7 summary and quiz

### Summary

- A **block** is statements in braces, treated as one statement — which is how "do several things" fits anywhere the grammar wanted one statement.
- **Namespaces** carve up the world of names to prevent collisions; reach into one with the scope-resolution operator `::`, and use a leading `::` to reach the global namespace.
- **Local variables** have block scope, automatic duration, and no linkage — fresh and isolated on every call.
- **Global variables** have global scope and static duration. *Constant* globals are fine and common; *mutable* globals should be rare.
- **Shadowing** is an inner name hiding an outer one. It's legal but confusing — avoid it.
- **Internal linkage** keeps a name local to one translation unit; **external linkage** lets declarations across files refer to one shared entity. `static`, `const`/`constexpr`, and unnamed namespaces give internal linkage; functions, non-const globals, and inline variables are external.
- Use **`extern`** for a global variable forward declaration (no initializer) or a deliberately-external const definition — and ideally arrange not to need mutable globals at all.
- Modern **`inline`** means "multiple identical definitions are allowed," which is what lets inline functions and inline variables live in headers.
- In C++17+, **`inline constexpr`** in a namespace in a header is the standard way to share constants across files.
- **Static local variables** have block scope but static duration — private to one function, yet they remember across calls.
- A **using-declaration** in a narrow scope is acceptable; **using-directives** (`using namespace`) should generally be avoided, and `using` statements must never appear in headers.
- **Unnamed namespaces** are the modern way to make `.cpp`-local helpers; **inline namespaces** are for versioning, not file-locality.

### A header/source checklist

When you write or read multi-file C++, walk this list:

1. A normal function *declared* in a header → put its *definition* in one `.cpp`, unless it's intentionally `inline`.
2. A small function *defined* in a header → mark it `inline` (unless another rule already makes it inline, like `constexpr`).
3. A shared compile-time constant (C++17+) → `inline constexpr` in a namespace in a header.
4. A helper used by only one `.cpp` → put it in an unnamed namespace in that file.
5. Mutable global state → stop and ask whether it could be local state, a parameter, a return value, or an object member instead.

### Tying it together — the lab

The chapter exercise, **Pass Telemetry**, is this chapter made concrete. You'll build a tiny multi-file library that a pretend LLVM pass uses to keep score — and *every* major idea here shows up in it:

- A nested namespace (`telemetry::config`) of `inline constexpr` constants, reached with `::` (7.2, 7.10).
- A **static local** call-counter inside `recordRun()` that climbs `1 → 2 → 3` across calls, and a **static-local** unique-ID dispenser handing out `1000 → 1001 → 1002` — persistence you can *watch*, because the tests call your functions several times in a row (7.11).
- An internal-linkage helper sealed in an **unnamed namespace**, so only that `.cpp` can touch it (7.6, 7.14).
- A non-const global *declared* `extern` in the header and *defined exactly once* in your `.cpp` (7.7).
- A function that deliberately **shadows** that global with a local and reaches the real one via `::g_buildTag` (7.5).
- A provided `static_assert` in the header — a small preview of Chapter 9.

There are no loops yet (that's Chapter 8). The whole exercise is to read the chapter's vocabulary off the code and turn `make test` from red to green. Here's the mental model to carry in — read each line with this chapter's terms:

```cpp
namespace // unnamed: everything inside has internal linkage
{
    bool shouldInstrument(const Instruction& instruction)
    {
        return !instruction.isDebugOrPseudoInst();
    }
}

bool Instrument::runOnFunction(Function& function)
{
    int insertedCounters { 0 }; // local: block scope, automatic duration

    for (BasicBlock& block : function)       // local reference, nested block
    {
        for (Instruction& instruction : block) // local reference, deeper block
        {
            if (!shouldInstrument(instruction))
                continue;

            ++insertedCounters;
        }
    }

    return insertedCounters > 0;
}
```

- `shouldInstrument` has **internal linkage** — it lives in an unnamed namespace, private to this file.
- `insertedCounters` is a **local variable**: block scope, automatic duration, no linkage.
- `function`, `block`, and `instruction` are **local references** in progressively nested blocks.
- No **mutable global state** is needed anywhere — the decision flows entirely through parameters, locals, and the return value.

That's the goal of the whole chapter: to look at code like this and *see* the scope, duration, and linkage of every name in it.
