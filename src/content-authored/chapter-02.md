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

In Chapter 1, every program you wrote lived inside a single `main()`. That works
when a program does one short thing. But real programs grow, and a program built
as one long list of statements becomes impossible to read, test, or change. You
need a way to take a chunk of work, give it a name, and set it aside as its own
self-contained unit. That unit is a **function**.

A **function** is a reusable sequence of statements designed to do a particular
job. Think of it as a recipe card you've written once and filed away: whenever
you need that dish, you don't re-derive the steps — you pull the card and follow
it. Your program does the same thing every time it **calls** a function.

### Callers, callees, and the call

When one function runs another, we have names for the two roles. The function
doing the calling is the **caller**; the function being run is the **callee**.
The act itself — running the callee — is a **function call** (you'll also hear it
called an *invocation*). You've already been a caller: every time you wrote
`std::cout`, you were ultimately driving library code someone else wrote.

### What actually happens during a call

Here's the mechanism, and it's worth getting right because everything later in
this chapter rests on it. The CPU executes statements in the caller one after
another. When it reaches a function call, it **suspends** the caller — it sets a
kind of bookmark at that exact spot — and jumps to the top of the callee. It runs
the callee's statements to completion, then **returns** to the bookmark and
resumes the caller from precisely where it left off.

> **Key insight:** a function call is not a detour you have to manually find your
> way back from. The CPU remembers where it was and returns there automatically.
> This is what lets you compose programs out of small pieces without bookkeeping.

### Syntax

A function definition has two parts. The **header** names the function and says
what type of value it returns; the **body** is the braces and the statements
inside them.

```cpp
returnType functionName() // function HEADER (return type + name)
{                         // function BODY begins
    // statements
}                         // function BODY ends
```

### Two sources of functions

Every function you call comes from one of two places:

- **Library functions** ship with the C++ standard library — you call them, you
  don't write them.
- **User-defined functions** are the ones you write yourself.

Either way, you call a function by writing its name followed by parentheses:
`name()`. The parentheses are not optional decoration — they are what tells the
compiler "call this now."

### A worked example

This program defines a function and watches control move into it and back:

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

```text
Starting main()
In doPrint()
Ending main()
```

Read that output as a trace of control flow. `main` prints its first line, the
call to `doPrint()` hands control over (so "In doPrint()" appears), `doPrint`
finishes and returns, and `main` resumes to print its last line.

A few facts that fall out of this model:

- A function can be **called as many times as you like**. Each call runs the
  whole body again, from the top.
- Functions can **call other functions**, which can call still others, forming a
  chain (`main` → `doA` → `doB`). Control returns back up the chain in reverse,
  unwinding one call at a time.
- **Nested function definitions are not allowed in C++.** You cannot define one
  function inside another. Every function is defined on its own, at the top
  level of a file.

> **Warning:** forgetting the parentheses — writing `doPrint;` instead of
> `doPrint();` — does *not* call the function. At best it does nothing useful; in
> most contexts it won't even compile. If a function "isn't running," check that
> you actually called it.

---

## 2.2 — Function Return Values (Value-Returning Functions)

A function that just performs an action is useful, but often you want a function
to *compute something and hand the answer back* — "what's the user's age?", "what
is 3 + 4?". A **value-returning function** does exactly that: it produces a value
and sends it to the caller.

Two pieces make this work.

The **return type**, written before the function name, declares *what kind* of
value the function gives back. `int getValueFromUser()` promises to return an
`int`.

The **`return` statement**, written `return expr;`, is where the function
actually hands the value over. It evaluates `expr`, sends a **copy** of the
result — the **return value** — back to the caller, and immediately exits the
function. Any statements after a `return` that runs don't execute.

```cpp
#include <iostream>

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

Notice the third call. **The caller decides what to do with the return value.**
You can drop it into an expression (`returnFive() + 2`), store it in a variable,
print it, or ignore it entirely. The function's job is to produce the value; what
happens next is up to you.

### `main`'s return value is a status code

You've been writing `return 0;` at the bottom of `main` without much explanation.
Here it is. `main` returns an `int` to the operating system — a **status code**
that reports how the program did. By universal convention, **`0` means success**
and any non-zero value means something went wrong.

The header `<cstdlib>` defines two portable named constants for this:
**`EXIT_SUCCESS`** and **`EXIT_FAILURE`**. Returning one of those reads more
clearly than a bare number. Note that only `0`, `EXIT_SUCCESS`, and
`EXIT_FAILURE` are guaranteed portable return values from `main` — other numbers
may mean different things on different systems.

`main` is special in one more way: if you leave off the `return` statement
entirely, it **implicitly returns 0**. Even so:

> **Best practice:** explicitly write `return 0;` at the bottom of `main`. It
> costs one line and makes your intent unmistakable.

One thing you may *not* do: **you cannot call `main()` yourself.** Only the
operating system starts your program by calling `main`.

### The rule that bites beginners

> **Warning:** a value-returning function must return a value on **every** path
> through the function. If control reaches the end of a non-`void` function
> without hitting a `return` (for example, an `int` function that "falls off the
> bottom"), the behavior is **undefined** — the program may produce garbage,
> appear to work, or crash. Modern compilers warn about this; do not ignore the
> warning.

Also worth fixing in your mind early: a function returns **exactly one value**
per call. When you eventually need to send back several pieces of data, you'll
learn techniques for it — but the one-value-per-call rule is the baseline.

### DRY: Don't Repeat Yourself

Functions are the main tool for obeying a principle you'll hear constantly:
**Don't Repeat Yourself**. If the same logic appears in two places and you later
need to fix it, you have to remember to fix *both* — and you won't. Factor the
repeated logic into a single function and call it wherever you need it.

```cpp
#include <iostream>

// Prompts the user for an integer and returns the value they entered.
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

The prompt-and-read logic is written once and called twice. Notice the comment
above `getValueFromUser` — it documents *what the return value means*, which is
exactly the kind of comment that earns its keep.

> **Tip:** DRY is a guideline, not a law. Splitting code into too many
> hair-thin functions can hurt readability as much as repetition does. Aim for
> functions that name a meaningful unit of work. (The tongue-in-cheek opposite of
> DRY is **WET** — "Write Everything Twice.")

---

## 2.3 — Void Functions (Non-Value-Returning Functions)

Not every function has a value to give back. A function whose whole job is to
*do something* — print a banner, save a file — has no meaningful return value.
For those, the return type is **`void`**: the keyword that says "this function
returns nothing."

```cpp
#include <iostream>

void printHi()
{
    std::cout << "Hi" << '\n';
}   // reaches the end and returns to the caller automatically — no return needed
```

A void function **returns automatically** when control reaches the closing brace.
You don't write a `return` to make that happen.

You *may* use a bare **`return;`** (with no value) to exit a void function early —
say, to bail out partway through. But a `return;` as the very last line does
nothing the closing brace wouldn't do anyway.

> **Best practice:** don't put a `return;` at the end of a void function. It's
> redundant noise.

Two things a void function cannot do, both of which produce compile errors:

```cpp
void printHi()
{
    std::cout << "Hi" << '\n';
    return 5;               // ERROR — a void function can't return a value
}
```

```cpp
std::cout << printHi(); // ERROR — printHi() produces no value to print
printHi();              // OK — call it as its own statement
```

> **Key insight:** some contexts *require* a value — the right-hand side of `<<`,
> the right-hand side of an assignment, an argument to another function. A void
> function has no value to supply there. A value-returning function always
> produces a value (which you may then choose to ignore); a void function simply
> has nothing to give.

---

## 2.4 — Introduction to Function Parameters and Arguments

So far data has flowed *out* of functions through return values. Now for the
other direction: getting data *into* a function. That's what parameters and
arguments are for.

A **parameter** is a variable declared in the function's header. An **argument**
is the actual value the caller supplies for that parameter when calling.

```cpp
#include <iostream>

void printValue(int x)  // x is a PARAMETER
{
    std::cout << x << '\n';
}

int main()
{
    printValue(6);      // 6 is an ARGUMENT
    return 0;
}
```

The vocabulary matters: *parameters* live in the definition, *arguments* live at
the call site. The same function called with different arguments does the same
job on different data.

### Pass by value

How does the argument reach the parameter? When you make the call, **the value of
each argument is copied into the matching parameter**. The parameter is
initialized from the argument, and from then on it's an independent **copy**.
This default mechanism is called **pass by value**. (Because the parameter is a
copy, changes to it inside the function don't affect the caller's original — a
property we'll lean on heavily later.)

Multiple parameters are separated by commas, and arguments are matched to them
left to right:

```cpp
#include <iostream>

int add(int x, int y)
{
    return x + y;
}

int main()
{
    std::cout << add(4, 5) << '\n';          // 9
    std::cout << add(1, add(2, 3)) << '\n';  // inner add(2,3)=5, then add(1,5)=6
    return 0;
}
```

The second line shows that a function call is itself an expression: `add(2, 3)`
evaluates to `5`, and that `5` becomes the second argument to the outer `add`.

### Parameters plus return values give you reusability

> **Key insight:** parameters (data in) and return values (data out) together let
> you write functions that perform a task and return a result **without knowing
> the specific inputs or outputs ahead of time**. `add` doesn't care which two
> numbers it gets — it works for all of them. This is the heart of writing
> general, reusable code.

### Unused parameters

Sometimes a function must *accept* a parameter (to match a required shape) but
doesn't actually use it. Naming such a parameter triggers an "unused variable"
warning. The fix: a parameter name is **optional**, so you can simply omit it,
leaving the type and a documenting comment.

```cpp
void doSomething(int /*count*/) // unnamed parameter; comment records its intent
{
}
```

> **Best practice:** when a parameter must exist but isn't used, leave it
> unnamed (a comment with the would-be name keeps it readable).

---

## 2.5 — Introduction to Local Scope

You've now seen variables defined inside functions and variables that are
function parameters. Both are examples of **local variables** — variables defined
inside a function body. (Function parameters count as local variables too: they
behave exactly like locals you defined yourself.)

```cpp
int add(int x, int y) // x and y are local variables (parameters)
{
    int z{ x + y };   // z is a local variable
    return z;
}
```

Two distinct ideas govern local variables, and beginners conflate them
constantly. Keep them separate.

### Lifetime — *when* a variable exists (a runtime property)

A local variable is **created** when its definition executes at runtime
(parameters are created when the function is entered). It is **destroyed** at the
end of the set of curly braces that contains it — for a function's locals, that's
when the function ends. When several locals are destroyed together, they go in
**reverse order of creation** (last created, first destroyed). A variable's
**lifetime** is the span from its creation to its destruction.

### Scope — *where* the name is usable (a compile-time property)

A local variable has **local scope** (also called **block scope**): its name is
usable from its point of definition to the end of the innermost pair of curly
braces containing it. Within that region the name is **in scope** (you can use
it); outside it the name is **out of scope**, and trying to use it is a compile
error.

> **Key insight:** *lifetime* answers "when does this variable exist?" and is a
> runtime fact. *Scope* answers "where can I refer to this name?" and is a
> compile-time fact. They usually line up for simple locals, but they are
> different questions — keep the two words straight.

### Locals in different functions are independent

Because each function's locals are confined to that function, two functions can
use the same variable names without any conflict at all:

```cpp
#include <iostream>

int add(int x, int y)   // add's own x and y
{
    return x + y;
}

int main()
{
    int x{ 5 };
    int y{ 6 };         // main's x and y — completely distinct from add's
    std::cout << add(x, y) << '\n';
    return 0;
}
```

`main`'s `x` and `add`'s `x` are unrelated variables that merely share a name.
When `main` calls `add(x, y)`, the *values* are copied across; the variables
themselves stay separate.

> **Best practice:** define each local variable **as close to its first use as is
> reasonable**, and in the smallest scope that works. The old C habit of
> declaring everything at the top of a function makes code harder to follow —
> modern C++ prefers definitions right where they're needed.

> **Tip:** to decide between a parameter and a local: use a **parameter** when the
> caller supplies the value, and a **local** when the function computes the value
> itself.

A few finer points you'll appreciate more later: class objects (Chapter 14
onward) run a special cleanup routine called a **destructor** just before they're
destroyed, and using such an object after destruction is undefined behavior. The
compiler is also allowed to shift the exact timing of creation and destruction
for optimization, as long as your program can't tell. And a value with no name —
a **temporary object**, such as the one a `return` by value produces — has no
scope and lives only until the end of the full expression it appears in. You
don't need to act on any of this now; just know the words exist.

---

## 2.6 — Why Functions Are Useful, and How to Use Them Effectively

You've seen *how* functions work. This lesson steps back to *why* you'd reach for
one, because knowing when to write a function is a skill in itself.

### What functions buy you

1. **Organization.** A function is almost like a mini-program with a clear job,
   written apart from `main`. Breaking a big task into named pieces makes the
   whole far easier to understand.
2. **Reusability.** Write the logic once, call it as often as you like — even
   share it across different programs.
3. **Testing.** Test a function once and you can trust it; you don't have to
   re-verify it every time you use it, only when you change it.
4. **Extensibility.** Need the behavior to change? Edit the one function, and
   every caller gets the improvement for free.
5. **Abstraction.** To use a function, a caller needs only its name, its inputs,
   its outputs, and where it lives — not how it works inside. That's what lets you
   build on top of code (yours and the library's) without drowning in detail.

### When to write a function

- A group of statements appears **more than once** — pull it into a function
  (DRY).
- A chunk of code performs a **well-defined task** with clear inputs and outputs —
  give it a name, even if you only use it once. A good name documents intent.
- Keep to **one function, one task** (the single-responsibility idea). A function
  that does three things is three functions wearing a trench coat.
- When a function grows too long or too tangled, **refactor** it — split it into
  smaller, well-named helpers.

> **Note:** resist mixing *calculation* with *output* in one function. A function
> that computes a result should **return** that result and let the caller decide
> whether to print it, store it, or feed it onward. Separating "figure it out"
> from "show it" keeps both halves reusable — and it's exactly the discipline the
> chapter's `geo` library exercise asks of you: each shape function *computes and
> returns* a number; nothing inside the library prints.

---

## 2.7 — Forward Declarations and Definitions

Here's a problem you'll hit the moment you have two functions. The compiler reads
a file **top to bottom**, in one pass. If `main` calls `add` but `add` is defined
*below* `main`, then at the point of the call the compiler has never heard of
`add` and reports an error like *"identifier not found."*

One fix is to always define functions before you use them. But that forces you to
order your whole file by dependency, which is awkward, and it's flat-out
impossible when functions call each other in a cycle. The real fix is a **forward
declaration**.

### The solution: a forward declaration (function prototype)

A **forward declaration** tells the compiler that a function *exists* — its name,
its return type, and its parameter types — before you actually define it. It's a
promise: "this function is defined somewhere; trust me and let the calls through."
A forward declaration of a function is also called a **function prototype**.

```cpp
int add(int x, int y); // forward declaration: return type, name, parameter
                       // TYPES, and a semicolon — no body
```

The trailing semicolon (where a body's `{ }` would go) is what makes it a
declaration rather than a definition. With this near the top of the file, `main`
can call `add` even though `add`'s body appears later — or in another file
entirely.

Parameter **names are optional** in a prototype; `int add(int, int);` is legal.
But:

> **Best practice:** keep the parameter names in your prototypes. `int add(int x,
> int y);` documents what the arguments mean far better than bare types do.

> **Tip:** the easy way to write a prototype is to copy the function's header and
> add a semicolon.

### Declaration versus definition

These two words look similar and are worth pinning down:

- A **declaration** tells the compiler that a name exists, along with its type
  information.
- A **definition** actually implements the thing — it provides the function's
  body, or creates the variable. **Every definition is also a declaration** (to
  define something, you necessarily declare it).
- A **pure declaration** is a declaration that is *not* a definition. A forward
  declaration is the prime example.

### The One Definition Rule (ODR)

C++ enforces a foundational rule with three parts:

1. **Within a single file**, a function, variable, type, or template may have at
   most one definition in a given scope. Break this and you get a **compiler
   error**.
2. **Within a whole program**, every function and variable may have at most one
   definition across all files. Break this and you get a **linker error**.
3. There's an **exception**: types, templates, and inline functions/variables may
   be defined identically in multiple files. If the definitions aren't truly
   identical, the result is undefined behavior.

You can declare a thing as many times as you want; you may define it only once.

### Why forward declarations earn their keep

- The function you're calling lives in **another file** — you can't reorder code
  across files, so a declaration is the only way to make the call.
- Two functions call **each other** (A calls B, B calls A) — no single ordering
  satisfies both, so at least one needs a forward declaration.
- They let you organize a file by **topic** (put `main` first, helpers below)
  rather than being held hostage by dependency order.

> **Best practice:** when the compiler reports a cascade of errors, fix the
> **first** one and recompile. Later errors are frequently just fallout from the
> first, and may vanish on their own.

One subtlety to file away: a forward declaration satisfies the *compiler*, but
the *linker* still needs to find an actual definition. If you declare and call a
function but never define it anywhere, the code **compiles** and then the linker
fails with *"unresolved external symbol."* Declaration and definition are
checked at different stages.

---

## 2.8 — Programs with Multiple Code Files

A program of any size doesn't live in one file. You split it across many, grouped
by topic, and the build system stitches them together. This lesson is your first
look at how that works — and it's the exact shape of the chapter's `geo` exercise.

> **Best practice:** new code files should use the `.cpp` extension.

### A two-file program

Put `add` in its own file, and call it from `main` in another:

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

You compile both at once, for example:

```sh
g++ main.cpp add.cpp -o main
```

### Why the forward declaration is mandatory here

This is the part that surprises people, so slow down for it. The compiler
compiles **each file independently**. While it's working on `main.cpp`, it does
*not* know anything about the contents of `add.cpp` — it doesn't peek at other
files, and it doesn't remember anything from files it compiled earlier.

So `main.cpp`, on its own, has no idea that `add` exists. The forward declaration
supplies that knowledge: it tells the compiler `add`'s signature so the call type-
checks. The *definition* stays over in `add.cpp`. After both files compile into
separate object files, the **linker** runs and connects the call in `main.cpp` to
the definition in `add.cpp`.

> **Key insight:** independent compilation is a feature, not a quirk. Files can be
> compiled in any order, only changed files need recompiling (a big speed win on
> large projects), and names in one file are insulated from names in another.

### Identifier resolution, in one place

When you use an identifier, it must ultimately connect to a definition. Where that
connection is made — and what error you get if it can't be — depends on where the
definition is:

- No declaration *or* definition in this file → **compiler error** ("identifier
  not found").
- Definition in the **same** file → the compiler connects it directly.
- Definition in **another** file → the **linker** connects it.
- No definition **anywhere** → **linker error** ("unresolved external symbol").

Memorize this table and most "why won't it build?" mysteries become a quick
diagnosis: *which stage failed tells you what's missing.*

### Common mistakes

- **Forgot the forward declaration** → compiler error, *identifier not found*.
- **Forgot to add `add.cpp` to the build** → it compiles but the linker can't find
  `add` → *unresolved external symbol*.
- **`#include "add.cpp"`** → don't. Including a `.cpp` file pastes its definitions
  in and causes duplicate-definition errors. Add the `.cpp` to the build instead.
- **Missing an `#include`** → every file that uses `std::cout` or `std::cin` needs
  its own `#include <iostream>`, because each file is compiled on its own with no
  memory of the others.

---

## 2.9 — Naming Collisions and an Introduction to Namespaces

As programs grow and pull in other people's code, two different things end up with
the same name. When the compiler or linker can't tell them apart, you get a
**naming collision** (or *naming conflict*).

The classic analogy: imagine a city with two different "Front Street"s. As long as
nobody needs to navigate between them it's fine — but the moment a delivery is
addressed to "Front Street," the system can't tell which one you meant.

Collisions surface at one of two stages:

- Two identical names in the **same file** → **compiler error**.
- Two identical definitions in **separate files** of one program → **linker
  error**. This happens even if the function is never called:

```cpp
// a.cpp
#include <iostream>
void myFcn(int x) { std::cout << x; }

// main.cpp
#include <iostream>
void myFcn(int x) { std::cout << 2 * x; } // linker: myFcn defined twice
int main() { return 0; }
```

### Scope regions and namespaces

To prevent collisions we need a way to keep names apart. The tool is the **scope
region**: an area within which all declared identifiers must be distinct. Two
identical names in *different* scope regions don't conflict at all — like the same
street name in two different cities.

A **namespace** gives you a named scope region. Declarations and definitions
placed inside it are isolated from names elsewhere, so they can't collide with
identically named things outside.

> **Key insight:** a namespace may only contain **declarations and definitions** —
> not bare executable statements. Statements like `std::cout << x;` must live
> inside a function body, not loose in a namespace.

```cpp
namespace math {
    double pi { 3.14 };    // OK — a definition
    void calculate() { }   // OK — a definition
}
```

This is precisely what the `geo` exercise does: it wraps every shape function in
`namespace geo { ... }` so that `geo::rectangleArea` can never collide with some
other library's `rectangleArea`.

### The global namespace

Anything you define that *isn't* inside a function, class, or namespace lives in
the **global namespace** (also called global scope). A global name is visible from
its point of declaration to the end of the file. Global *variables* are allowed
but **generally discouraged** — you'll see why in later chapters.

### The `std` namespace

The entire C++ standard library lives in a namespace called **`std`** (short for
"standard"). That's why those names don't collide with yours: `cout`, `cin`,
`string`, and the rest are tucked inside `std`. The flip side is that you have to
*tell the compiler* a name lives in `std`. There are three ways.

**1. The scope resolution operator `::` (recommended).** Write
`namespace::name` to mean "the `name` that lives in `namespace`":

```cpp
std::cout << "Hello world!"; // the cout that lives in std
```

`std::cout` reads literally as "`cout`, which is in `std`." A name written with
its namespace prefix is a **qualified name**. (With nothing on the left of `::`,
the name is taken from the global namespace.)

> **Best practice:** prefer explicit `std::` prefixes. They make it unambiguous
> where each name comes from, and they never break when the library grows.

**2. A using-declaration** (`using std::cout;`) imports a *single* name so you can
write it unqualified. Useful in moderation; details come in a later chapter.

**3. A using-directive** (`using namespace std;`) imports *everything* from a
namespace at once. Avoid it. It drags every name in `std` into your scope and
re-creates the very collisions namespaces were invented to prevent:

```cpp
using namespace std;
int cout() { return 5; }     // your own cout
cout << "Hello, world!";     // ERROR — ambiguous: std::cout or your cout()?
```

> **Warning:** don't put `using namespace std;` at the top of a program, and
> *never* in a header. Beyond today's ambiguity, future additions to `std` can
> silently collide with your names and break code that used to compile. The small
> typing it saves is not worth it.

---

## 2.10 — Introduction to the Preprocessor

Before the compiler ever sees your code, another program edits it first: the
**preprocessor**. For each `.cpp` file, the preprocessor scans the text and makes
a series of **text substitutions** in memory. Your file on disk is never changed —
the preprocessor produces a modified copy and feeds *that* to the compiler.

The output — your `.cpp` plus the full text of every header it pulled in — is
called a **translation unit**. That translation unit is the actual input to the
compiler.

> **Key insight:** the preprocess, compile, and link steps together are called
> **translation**, which is why the compiler's input is a "translation unit."

### Preprocessor directives

The preprocessor is driven by **directives**: lines that begin with `#` and run to
the **end of the line** (no semicolon). Directives are a separate little language
from C++ itself.

> **Key insight:** the preprocessor's output contains **no directives** — only the
> *result* of carrying them out is handed to the compiler. By the time the
> compiler runs, every `#include` and `#define` is gone.

### `#include`

You've used this since Chapter 1. `#include` replaces the directive with the
**entire contents** of the named file:

```cpp
#include <iostream> // replaced by the full text of the iostream header
```

That's all `#include` does — it's a copy-paste. Everything you can call from
`<iostream>` is available afterward because its declarations got pasted in.

### `#define` and macros

`#define` creates a **macro** — a named rule the preprocessor applies. There are a
few flavors:

- **Object-like macro with substitution text.** Every later occurrence of the name
  is replaced by the text:

  ```cpp
  #define MY_NAME "Alex"
  std::cout << MY_NAME; // becomes std::cout << "Alex"
  ```

  > **Best practice:** avoid substitution-text macros unless there's genuinely no
  > alternative. They bypass the type system and the scope rules, which causes
  > surprises; named constants (Chapter 5) are almost always better.

- **Object-like macro with no substitution text.** It defines a name that stands
  for nothing — it just exists, as an on/off flag for conditional compilation
  (below). This use is generally fine.

  ```cpp
  #define PRINT_JOE
  ```

- **Function-like macros** look like functions but are macros. They're considered
  unsafe — use real functions instead.

> **Best practice:** name macros in `ALL_UPPERCASE_WITH_UNDERSCORES`, so they're
> visually distinct from normal identifiers.

### Conditional compilation

The most useful job macros do is *deciding which code gets compiled at all*. The
conditional-compilation directives ask whether a macro is defined and include or
exclude a block accordingly:

```cpp
#define PRINT_JOE

#ifdef PRINT_JOE
    std::cout << "Joe\n";   // included — PRINT_JOE is defined
#endif

#ifndef PRINT_BOB
    std::cout << "Bob\n";   // included — PRINT_BOB is not defined
#endif
```

- `#ifdef X` includes its block if `X` is defined; `#ifndef X` includes its block
  if `X` is *not* defined. The equivalent C++-style forms are `#if defined(X)` and
  `#if !defined(X)`.
- `#if 0 ... #endif` excludes a block from compilation entirely. It's a handy way
  to "comment out" a region that itself contains `/* */` comments (which can't be
  nested). Flip the `0` to `1` to switch the block back on.

### A `#define` only affects its own file

The preprocessor has no notion of C++ scope. A `#define` is in effect from the line
it appears on to the **end of that file** — no further. It does **not** leak into
other files unless those files `#include` the file that defined it.

> **Note:** macro substitution generally does *not* happen inside other
> directives — for instance, the `X` in `#ifdef X` is not itself macro-expanded —
> with `#if` and `#elif` as exceptions. You rarely need to think about this, but
> it explains some otherwise-baffling behavior.

---

## 2.11 — Header Files

In a multi-file program, every file that calls `add` needs `add`'s forward
declaration. Copying that prototype by hand into ten files — and keeping all ten
in sync when the signature changes — is hopeless. **Header files** solve this: you
write the declarations *once* in a header and `#include` it wherever they're
needed.

### The convention

A header file (`.h`) is **paired** with a source file of the same base name:
`add.h` goes with `add.cpp`. The header holds the **declarations** (the public
promises); the `.cpp` holds the **definitions** (the implementations). This is the
single most important structural idea in the chapter — and exactly how the `geo`
exercise is laid out: `geo.h` declares the five functions, `starter/geo.cpp`
defines them.

### The full pattern

**add.h**

```cpp
int add(int x, int y); // declaration only (a prototype)
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
#include <iostream>     // angle brackets → a library header

int main()
{
    std::cout << "Sum: " << add(3, 4) << '\n';
    return 0;
}
```

Now `main.cpp` gets `add`'s declaration by including one line, and so would any
other file that needs it.

### Quotes versus angle brackets

The two `#include` forms tell the preprocessor *where to look*:

- `#include "add.h"` — for **your own** headers. The preprocessor searches the
  current directory first, then the system include paths.
- `#include <iostream>` — for **system/library** headers. The preprocessor
  searches only the system include paths.

Rule of thumb: angle brackets for things that came with your compiler or a
library, quotes for things you wrote.

### The rules that keep it working

- **Headers contain declarations only** (for now). A *definition* placed in a
  header that's included by multiple `.cpp` files produces multiple definitions
  across the program — an ODR violation, and you'll get linker errors.
- **A source file should include its own paired header.** This isn't just tidy; it
  lets the compiler catch mismatches at *compile* time instead of link time:

  ```cpp
  // add.h:   int add(int x, int y);
  // add.cpp:
  #include "add.h"
  double add(int x, int y) { /* ... */ } // ERROR caught now — return type mismatch
  ```

  Because `add.cpp` sees the declaration and the definition together, the compiler
  notices they disagree immediately. The `geo` exercise relies on exactly this: a
  signature mismatch in your `geo.cpp` is a compile error precisely because
  `geo.cpp` includes `geo.h`.
- **Never `#include` a `.cpp` file.** Add it to the build instead. Including it
  causes duplicate definitions, recompilation surprises, and collisions.

### Include order (a best practice)

When a file has several includes, order them like this:

1. This file's **paired header**
2. Other **project** headers (alphabetical)
3. **Third-party library** headers (alphabetical)
4. **Standard library** headers (alphabetical)

Putting your own paired header *first* maximizes the chance that a missing
`#include` *inside* that header gets caught — if a later system header happened to
supply the missing piece, you'd never notice the gap.

### Transitive includes

When you include a header, you also get whatever *it* includes — and whatever those
include — all the way down. This is called a **transitive include**. It's
convenient, but don't lean on it:

> **Best practice:** every file should explicitly `#include` everything it directly
> uses. Don't rely on a name being available "for free" through a transitive
> include — which header pulls in what varies across compilers and standard-library
> versions, so code that builds on your machine may fail on someone else's. If
> something compiles without an include you'd expect to need, that's a transitive
> include quietly doing the work, not a license to omit it.

### `.h` versus no extension

- **Standard library** headers have **no extension** (`<iostream>`, `<cstdlib>`)
  and put their names in `std`. (Very old `.h` versions of these existed and put
  names in the global namespace; don't use them.)
- **Your own** headers should still use the **`.h`** extension.

---

## 2.12 — Header Guards

Header files introduce a new way to trip over the One Definition Rule. Here's the
trap and the standard tool that disarms it.

### The duplicate-inclusion problem

If a header gets included **more than once** into the same translation unit, its
contents appear twice — and if it contains anything that can only be defined once,
that's an ODR violation. This is alarmingly easy to cause by accident through
transitive includes:

```cpp
// square.h: contains a definition (shown here for illustration)
//           int getSquareSides() { return 4; }
// wave.h:   #include "square.h"
// main.cpp: #include "square.h"
//           #include "wave.h"   // square.h gets pulled in a SECOND time → redefinition
```

`main.cpp` includes `square.h` directly, then includes `wave.h`, which *also*
includes `square.h`. Now `square.h`'s contents land in the translation unit twice.

### The fix: a header guard

A **header guard** (or *include guard*) wraps the header's contents so that even if
the file is `#include`d many times, its body is processed only once per
translation unit:

```cpp
#ifndef SQUARE_H   // if SQUARE_H has not been defined yet...
#define SQUARE_H   // ...define it now, and include everything below

int getSquareSides() { return 4; }

#endif             // end of SQUARE_H guard
```

Trace it. The **first** time the file is included, `SQUARE_H` isn't defined yet, so
the `#ifndef` is true: the preprocessor defines `SQUARE_H` and keeps the body. The
**second** time the same translation unit pulls the file in, `SQUARE_H` *is* now
defined, the `#ifndef` is false, and the entire body is skipped. The contents
appear exactly once.

### Naming convention

The guard macro is conventionally the **filename in all caps**, with punctuation
turned into underscores: `square.h` → `SQUARE_H`, `my_math.h` → `MY_MATH_H`. This
is the form you'll see in `geo.h` for the exercise.

> **Note:** two different files both named `config.h` in different directories
> would both pick the guard `CONFIG_H` and accidentally block each other. In large
> projects a more unique form like `PROJECT_PATH_FILE_H` avoids that. For small
> programs the simple form is fine.

### What guards do — and don't — do

> **Key insight:** a header guard prevents duplicate inclusion **within a single
> translation unit**. It does *not* — and should not — prevent a header from being
> included into *different* `.cpp` files. Each `.cpp` is its own translation unit
> with its own pass of the preprocessor, and each one getting a copy of the
> declarations is exactly what you want.

That distinction has a consequence. A header containing a **definition** still
breaks across multiple `.cpp` files even with a guard: each translation unit
legitimately gets one copy of the definition, and the *program* ends up with
several — a linker error. The guard can't help, because nothing was duplicated
*within* a unit. The real fix is the rule you already know:

```cpp
// square.h
#ifndef SQUARE_H
#define SQUARE_H
int getSquareSides();          // declaration only
#endif

// square.cpp
#include "square.h"
int getSquareSides() { return 4; } // the one and only definition
```

Declarations in the header, the single definition in one `.cpp`. Guards handle
accidental double-inclusion; the header/source split handles the program-wide
single-definition rule.

### Why build the habit now

Soon you'll define your own types (Chapter 13 onward), and unlike functions, those
type definitions genuinely *must* live in headers — which makes header guards
essential, not optional. Building the habit now, on simple headers, means it's
automatic by the time it's mandatory.

### `#pragma once`, the alternative

Most compilers support a one-line alternative:

```cpp
#pragma once
// header contents
```

It's simpler and harder to get wrong than a traditional guard (no macro name to
duplicate by accident). The catch: `#pragma once` is **not** part of the official
C++ standard, though it's supported nearly everywhere. There's also a rare edge
case — if the *same* header is physically duplicated at two paths on disk and both
copies get included, traditional guards (sharing one macro name) de-duplicate while
`#pragma once` may treat them as different files. The exercise uses traditional
`#ifndef`/`#define`/`#endif` guards, as the most conventional and portable choice.

---

## 2.13 — How to Design Your First Programs

You now have the building blocks — functions, files, headers, namespaces. The last
lesson of the chapter is about *thinking before typing*. Programs you sketch on the
fly tend to sprawl; a few minutes of design up front saves hours of debugging.

> **Key insight:** spending a little time up front thinking about how to structure
> your program leads to better code and far less time spent finding and fixing
> errors.

### Design steps (before you write code)

1. **Define your goal.** State, in a sentence or two and in user-facing terms, what
   the program is *for*. ("Let the user organize a list of names and phone
   numbers.")
2. **Define your requirements.** List the constraints (time, memory, budget) and
   the capabilities the program must have. Stay focused on *what* it must do, not
   *how*. ("Phone numbers must be saved for later retrieval"; "must run in under
   ten seconds.")
3. **Define your tools, targets, and backup plan.** For now your tool is your
   editor or IDE, and your target is your own machine — so the practical advice
   here is simply: **back up your code in more than one place** (a cloud drive,
   GitHub, an external disk). Losing a day's work to a dead laptop teaches this
   lesson the hard way.
4. **Break hard problems into easy ones.** Decompose the task into subtasks, either
   **top-down** (start with the big goal and recursively split it) or **bottom-up**
   (list the small tasks you know how to do and group them into a hierarchy). The
   resulting hierarchy maps straight onto code: the main task becomes `main()`, and
   each subtask becomes a function.
5. **Figure out the sequence of events** — the order the tasks run in. For a
   calculator: get the first number → get the operator → get the second number →
   compute → print the result.

### Implementation steps

1. **Outline `main` as a skeleton** of commented-out function calls — the shape of
   the program before any of it works:

   ```cpp
   int main()
   {
       // getUserInput();
       // calculate();
       // printResult();
       return 0;
   }
   ```

2. **Implement each function in turn.** For each one: (a) write its prototype,
   nailing down its inputs and outputs; (b) write its body; (c) **test it
   immediately** with a little throwaway code before moving on. Don't write the
   whole program and *then* run it for the first time — that buries ten bugs in one
   pile.

3. **Do final testing** of the finished program against every requirement from
   step 2.

### Words of advice

- **Keep your first version simple.** Get a minimal thing working, then add
  features one at a time.
- **Focus on one area at a time.** Finish a task before starting the next; a dozen
  half-built pieces is worse than three finished ones.
- **Test each piece as you go.** Bugs caught early are cheap; bugs that compound on
  top of each other are expensive.
- **Don't polish early code.** Programs evolve and early code often gets rewritten —
  perfecting it is wasted effort.
- **Optimize for maintainability, not raw performance.** As Knuth put it,
  "premature optimization is the root of all evil." Reduce redundancy and improve
  readability first; make it fast only when you've measured that you need to.

> **Note:** "A complex system that works is invariably found to have evolved from a
> simple system that worked." (John Gall) Start small, get it working, grow it.

This design loop — decompose into functions, prototype, implement, test each piece —
is exactly the rhythm the `geo` exercise walks you through: five small functions,
each with a clear input and output, written and checked one at a time until the
grader turns green.

---

## 2.x — Chapter 2 summary and quiz

### Core takeaways

- **Functions** package a reusable sequence of statements behind a name. A
  **function call** temporarily transfers control to the callee, which runs and
  then returns to the caller right where it left off.
- A **value-returning function** declares a return type and uses `return expr;` to
  hand a value back; it must return a value on **every** path, or the behavior is
  undefined. A **void** function returns nothing and returns automatically at its
  closing brace.
- **Parameters** are the variables in a function's header; **arguments** are the
  values the caller passes. By default arguments are **passed by value** — copied
  into the parameters. Parameters in, return values out, gives you reusable code.
- **Local variables** (parameters included) have **block scope** (a compile-time
  property: *where* a name is usable) and a **lifetime** running to the end of
  their enclosing braces (a runtime property: *when* the variable exists). Locals
  in different functions are independent even when identically named.
- A **declaration** says a name exists; a **definition** implements it. A **forward
  declaration** lets the compiler accept a call before the body appears. The **One
  Definition Rule** allows many declarations but only one definition.
- Multi-file programs compile each `.cpp` **independently** into a translation
  unit, then the **linker** joins them. The error stage (compiler vs linker) tells
  you what's missing.
- **Namespaces** create named scope regions that prevent naming collisions; the
  standard library lives in `std`, reached with the scope resolution operator
  (`std::cout`). Prefer explicit `std::`; avoid `using namespace std;`.
- The **preprocessor** runs first, performing text substitutions for directives
  like `#include` and `#define`; its output is the translation unit the compiler
  sees.
- **Header files** hold declarations and are paired with a `.cpp` that holds the
  definitions; a source file includes its own header so mismatches are caught at
  compile time. **Header guards** prevent duplicate inclusion within one
  translation unit.
- Good program design starts with a clear **goal** and **requirements**,
  **decomposes** the work into functions, and **tests each piece incrementally**.

### Putting it to work: the `geo` mini-library

The chapter's exercise is the whole chapter in miniature. You're handed `geo.h`, a
header that — behind a header guard — declares five functions inside
`namespace geo`. Your job is to write their bodies in `starter/geo.cpp`. The demo
program and the grader both `#include "geo.h"` and call your functions through the
`geo::` prefix; they never see the implementations, only what the header promises.

Watch for the ideas as they show up:

- **Functions with parameters and return values** — each shape function takes
  dimensions and returns a number.
- **The header/source split** — declarations in `geo.h`, definitions in `geo.cpp`,
  compiled as two files and joined by the **linker** (`make test` compiles the
  tests *together with* your `geo.cpp`).
- **The `.cpp`-includes-its-`.h` safety net** — a signature mismatch becomes a
  *compile* error because `geo.cpp` includes `geo.h`.
- **A user-defined namespace and `::`** — your bodies must live inside
  `namespace geo { ... }` so their full names match the prototypes; inside the
  namespace you call sibling functions unqualified.
- **Functions calling functions (DRY)** — `rectangleDiagonal` is the hypotenuse of
  the rectangle's two sides, so it *calls* your own `hypotenuse` rather than
  re-deriving the square root. One definition of the Pythagoras math, reused.

One type-watch worth flagging now (it's the trap the grader checks): in
`rightTriangleArea`, the legs are `int`, so `(a * b) / 2` does **integer
division** and throws away the half *before* the value ever becomes a `double`.
`15 / 2` is `7`, not `7.5`. Divide by `2.0` so the division happens in floating
point. You'll meet the full story of integer-versus-floating-point arithmetic in
Chapter 4; for now, the fix is simply the `2.0`.

### CS6340 lens

This file layout — a `.h` that declares an API inside a namespace, a `.cpp` that
defines it — is the exact shape of every LLVM pass you'll write later. When you see

```cpp
#include "Instrument.h"
```

read it as: *this source file needs the declarations from `Instrument.h`.* And when
a linker error reports an undefined symbol, ask the question this chapter taught you
to ask:

```text
Did I declare the function but forget to provide exactly one compiled definition?
```

That single question resolves the large majority of multi-file build failures.
