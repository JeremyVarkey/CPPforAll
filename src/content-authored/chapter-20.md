# Chapter 20 — Functions

> Source: <https://www.learncpp.com/> (Chapter 20)

You already know how to write a function: name it, give it parameters, return a value, call it. That much has been true since Chapter 2. What this chapter does is widen the lens. It asks a different set of questions — not "how do I write a function?" but "what *is* a function, really, once the program is running, and what new things become possible when I start treating functions themselves as data I can pass around?"

Three big ideas thread through the chapter, and they all point the same direction: **behavior as a thing you can hold, move, and hand off.**

The first is the address. A function lives somewhere in memory just like a variable does, which means it has an address, which means you can store that address in a pointer and call the function *through* the pointer. Suddenly a function can take another function as an argument — the caller decides *what* to do, your code decides *when* to do it. That is the callback, and it is everywhere in real software.

The second is **recursion** — a function that calls itself. It sounds like a parlor trick, but for problems that are themselves built out of smaller copies of the same problem (trees, nested structures, divide-and-conquer), recursion is often the clearest code you can write. To understand why it works — and how it can go wrong — you need a clear picture of the **call stack**, so we'll look at where local variables actually live, and why the stack and the heap behave so differently.

The third is the **lambda**: a small, unnamed function you write inline, right where you need it, that can *capture* variables from the surrounding code. Lambdas are the modern, ergonomic version of "pass behavior as data," and once you have them you'll reach for them constantly.

Along the way we'll cover how a program receives input from the command line, and one old mechanism — the ellipsis — that you should be able to recognize but almost never write yourself.

## Contents
- [20.1 — Function Pointers](#201--function-pointers)
- [20.2 — The stack and the heap](#202--the-stack-and-the-heap)
- [20.3 — Recursion](#203--recursion)
- [20.4 — Command line arguments](#204--command-line-arguments)
- [20.5 — Ellipsis (and why to avoid them)](#205--ellipsis-and-why-to-avoid-them)
- [20.6 — Introduction to lambdas (anonymous functions)](#206--introduction-to-lambdas-anonymous-functions)
- [20.7 — Lambda captures](#207--lambda-captures)
- [20.x — Chapter 20 summary and quiz](#20x--chapter-20-summary-and-quiz)

---

## 20.1 — Function Pointers

### Functions have addresses

Here is an idea that feels strange the first time you meet it: a function has an address. When your program is compiled, the body of each function becomes a block of machine instructions, and that block sits somewhere in memory. The function's name, when you use it without parentheses, *is* that address — much the way an array's name decays to the address of its first element.

If a function has an address, then there must be a kind of pointer that can hold it. There is. It is called a **function pointer**, and it stores the address of a function with a particular signature.

```cpp
int add(int x, int y)
{
    return x + y;
}

int (*operation)(int, int) { &add };

std::cout << operation(2, 3) << '\n'; // calls add(2, 3)
```

Read the declaration of `operation` slowly, because the syntax is genuinely awkward and the awkwardness is the point:

```cpp
int (*operation)(int, int)
//  ^ return type
//      ^ pointer variable name
//                 ^ parameter types
```

The parentheses around `*operation` are not decoration — they are load-bearing. They group the `*` with the name so the compiler reads "`operation` is a pointer, and the thing it points to is a function taking `(int, int)` and returning `int`." Drop those parentheses and the meaning flips entirely:

```cpp
int* operation(int, int); // a FUNCTION named operation, returning int*
```

That second line declares an ordinary function that happens to return a pointer-to-`int`. Same characters, almost — wildly different meaning. The grouping parentheses are what tell `*` to bind to the name instead of the return type.

> **Key insight:** A function pointer's type encodes the function's *signature* — its return type and parameter types. The parentheses around `(*name)` are mandatory; they're what make it a pointer to a function rather than a function returning a pointer.

### Assigning functions

To point a function pointer at a function, assign the function's name. The address-of operator `&` in front of the name is optional — the name already decays to an address — but writing it makes your intent unmistakable, and that's a habit worth keeping.

```cpp
int multiply(int x, int y)
{
    return x * y;
}

int (*op)(int, int) { &multiply };
```

The signature has to match exactly. A function pointer typed for `(int, int) -> int` cannot hold a function that takes `(double, double)`, even though "two numbers in, one number out" sounds close enough:

```cpp
double divide(double, double);

// int (*op)(int, int) { &divide }; // ERROR: signatures don't match
```

Overloaded functions create a wrinkle. When a name refers to several overloads, the name alone is ambiguous — the compiler needs the function-pointer type to decide which overload you mean. The type of the variable you're assigning to does that disambiguation for you:

```cpp
int score(int);
int score(int, int);

int (*singleScore)(int) { &score }; // the (int) type selects score(int)
```

### Calling through a pointer

Once a function pointer holds a valid address, you call through it. Two spellings are common and they do exactly the same thing:

```cpp
int result1 { operation(4, 5) };
int result2 { (*operation)(4, 5) };
```

The first reads like an ordinary function call — and that's the style you'll see most often, because once `operation` clearly *is* a function pointer, the extra punctuation just adds noise. The second spelling, with the explicit dereference, exists because it makes the "I am calling through a pointer" mechanism visible. Use whichever makes the code clearer at the call site.

There is one safety obligation. A function pointer can be null — either because you initialized it empty, or because some function handed you one. Calling a null function pointer is **undefined behavior**, so guard it:

```cpp
int (*op)(int, int) {}; // value-initialized to nullptr

if (op)
    std::cout << op(1, 2) << '\n';
```

> **Warning:** Calling a null function pointer is undefined behavior — typically a crash. If a pointer might be empty, check it (`if (op)`) before calling. The lab's `apply` function makes this guard its very first line.

### Default arguments do not travel through function pointers

Default arguments are a compile-time convenience supplied by the function *declaration* the compiler sees at the call site. A function pointer's type carries the parameter *types*, but not those defaults — so when you call through a pointer, you must supply every argument yourself.

```cpp
void logLevel(int level = 1);

void (*logger)(int) { &logLevel };

logger(2);  // ok
// logger(); // ERROR: the pointer call needs the argument; the default is gone
```

If a callback needs a default behavior, don't rely on the pointer to remember it. Wrap the call in an ordinary function or a lambda that fills in the missing argument.

### Passing behavior into a function

Now we arrive at the reason function pointers earn their keep: **callbacks**. A function can accept another function as a parameter, which lets you write one algorithm and vary a piece of its behavior from the outside.

Consider printing two numbers in order. The *algorithm* — compare, then print the smaller-looking one first — never changes. What changes is the rule for "comes before." So we pass that rule in as a function pointer:

```cpp
bool ascending(int a, int b)
{
    return a < b;
}

bool descending(int a, int b)
{
    return a > b;
}

void printOrdered(int left, int right, bool (*comesBefore)(int, int))
{
    if (comesBefore(right, left))
        std::cout << right << ' ' << left << '\n';
    else
        std::cout << left << ' ' << right << '\n';
}

printOrdered(4, 2, &ascending);
printOrdered(4, 2, &descending);
```

The body of `printOrdered` is fixed. Feed it `&ascending` and it prints small-first; feed it `&descending` and it prints large-first. You have factored the *policy* out of the *mechanism*. This is exactly the move that algorithms like `std::sort` and `std::find_if` (Chapter 18) make when they take a predicate.

> **Note (CS6340 tie-in):** Callbacks are the conceptual ancestor of passing predicates into algorithms. In LLVM-style analysis code you'll more often pass a *lambda* to `std::find_if`, `std::any_of`, or a custom traversal — but the underlying idea, "hand the algorithm a small decision function," is the one you're learning here.

### Default callback functions

Because a function pointer is just a parameter, it can have a default value like any other parameter. That lets you offer a sensible fallback behavior the caller can override:

```cpp
bool keepEverything(int)
{
    return true;
}

void printIf(int* values, int length, bool (*keep)(int) = &keepEverything)
{
    for (int i { 0 }; i < length; ++i)
    {
        if (keep(values[i]))
            std::cout << values[i] << '\n';
    }
}
```

This reads as: "if the caller doesn't supply a predicate, default to keeping everything." The caller who wants different behavior passes their own function; everyone else gets the fallback for free.

### Type aliases make this readable

Let's be honest about the syntax: `bool (*comesBefore)(int, int)` is hard to read, and it gets worse when it appears in a parameter list among other parameters. Chapter 10's `using` aliases are the cure. Give the function-pointer type a name that describes its *role*, and the punctuation disappears behind a noun:

```cpp
using Compare = bool (*)(int, int);

void printOrdered(int left, int right, Compare comesBefore)
{
    if (comesBefore(right, left))
        std::cout << right << ' ' << left << '\n';
    else
        std::cout << left << ' ' << right << '\n';
}
```

`Compare` tells the reader *why* the parameter exists; the raw form only tells them how it's spelled. The lab does exactly this with an alias called `BinaryIntOp` for `int (*)(int, int)` — when you write `apply` and `pickOp`, you'll be working with that alias rather than re-typing the pointer syntax each time.

> **Best practice:** Hide gnarly function-pointer types behind a `using` alias that names the role (`Compare`, `BinaryIntOp`). The declaration becomes readable and the type has a single place to change.

### `std::function`

A raw function pointer is lean and fast, but it has a hard limit you'll hit soon: it can only point at *plain* functions and *non-capturing* lambdas. As you'll see in 20.7, a capturing lambda is a little object with data inside it, and no plain pointer can hold one.

`std::function`, from the `<functional>` header, is the general-purpose answer. It is a callable *wrapper* that can store anything you can call with a given signature — ordinary functions, function pointers, lambdas (capturing or not), and function objects — as long as the call shape matches.

```cpp
#include <functional>

void runTwice(int value, std::function<void(int)> action)
{
    action(value);
    action(value);
}

runTwice(3, [](int x)
{
    std::cout << x * x << '\n';
});
```

The flexibility isn't free. `std::function` uses a technique called **type erasure** to hide the concrete callable behind a uniform interface, and that can cost an indirect call and sometimes a heap allocation. So choose deliberately:

| Tool | Strength | Cost |
|---|---|---|
| raw function pointer | simple, cheap; points to plain functions and non-capturing lambdas | cannot store capturing lambdas |
| `std::function` | flexible; stores many callable kinds uniformly | type-erasure overhead and possible allocation |
| template callable parameter | fastest and most flexible at compile time | the function body usually has to live in a header |

That third row — a template parameter — is the tool you'll meet properly in Chapter 26. For this chapter and the lab, `std::function<bool(int)>` is the right choice: the lab's `countMatching` needs to accept a plain function pointer *and* a capturing lambda through one parameter, and `std::function` is precisely the wrapper that makes that work.

### `auto` and callable types

When the exact function-pointer type is obvious from the initializer, `auto` (Chapter 10) can spare you from spelling it out:

```cpp
auto op { &add };
std::cout << op(1, 2) << '\n';
```

`auto` is great for *local variables* like this. It does not help for *function parameters*, though — a parameter still needs a concrete type. There, reach for a named alias, a `std::function`, or (later) a template parameter, depending on whether you're optimizing for readability, flexibility, or raw speed.

---

## 20.2 — The stack and the heap

### Program memory is divided by purpose

When your program runs, the operating system hands it a block of memory, and that block is not one undifferentiated pool. It's organized into regions, each with a job. The exact layout is implementation-dependent — you should not write code that depends on the specifics — but a useful mental model looks like this:

```text
program memory
+-----------------------------+
| code / text segment          |
| static and global storage    |
| heap / free store            |
| ...                          |
| stack                        |
+-----------------------------+
```

The **code segment** holds your compiled instructions (this is where functions live, which is why they have addresses). **Static and global storage** holds objects that exist for the whole run of the program. The two regions you'll think about most in everyday C++ are the last two: the **stack**, where local variables live, and the **heap** (also called the *free store*), where `new` gets its memory.

### The call stack

Every time a function is called, the program sets aside a chunk of stack memory for it called a **stack frame**. That frame holds the function's local automatic variables, its parameters, and bookkeeping the implementation needs to return to the caller afterward. When the function returns, its frame is removed and that memory is reclaimed.

The name "stack" is literal: frames pile up and come off in last-in, first-out order, exactly like a stack of plates. The most recently called function is on top; it must finish and be removed before the one beneath it resumes.

```cpp
int square(int x)
{
    int result { x * x };
    return result;
}

int main()
{
    int value { square(5) };
}
```

While `square(5)` is executing, the stack looks like this:

```text
top of stack
+------------------+
| square frame      |
| x = 5             |
| result = 25       |
+------------------+
| main frame        |
| value not set yet |
+------------------+
```

The instant `square` returns, its frame — and with it `x` and `result` — is gone. Those locals do not linger; they cease to exist. Hold that fact, because it is the single most important thing to understand about stack memory, and it's the source of one of the most common bugs in C++.

### Stack properties

The stack is:

- **fast** — allocating a frame is essentially moving a pointer; there's no searching for free space;
- **automatic** — frames are cleaned up for you the moment a scope ends, no `delete` required;
- **limited in size** — it's a fixed, fairly small region (often a few megabytes);
- **structured by call nesting** — it grows and shrinks strictly with how deeply functions call each other.

That third point has teeth. A large local array can blow past the stack's size limit and crash the program — a *stack overflow*:

```cpp
void risky()
{
    int values[1'000'000] {}; // ~4 MB on the stack — may overflow it
}
```

When a buffer is large, or its size isn't known until runtime, don't put it on the stack. Use `std::vector<int>` (Chapter 16), which stores its elements on the heap and grows as needed.

> **Best practice:** Keep large or runtime-sized data off the stack. A big local array risks a stack overflow; a `std::vector` keeps its storage on the heap and sidesteps the limit.

### Heap properties

The **heap** is the region you allocate from with `new` and return with `delete` (Chapter 19):

```cpp
int* value { new int { 42 } };
delete value;
```

Compared with the stack, the heap is:

- **flexible** — you decide exactly when memory is allocated and freed;
- **long-lived** — a heap object outlives the function that created it, so its lifetime can cross function boundaries;
- **manual** — *you* are responsible for freeing it, unless an RAII wrapper does it for you;
- **slower to allocate** — the allocator has to find a suitable free block, which is real work;
- **error-prone in raw form** — forget to free and you leak; free too early and you have a dangling pointer.

The trade is clear: the stack is fast and self-cleaning but rigid and bounded; the heap is flexible and durable but slower and demands discipline.

### Stack vs heap lifetimes

The difference between the two becomes concrete the moment you try to return a pointer to a local:

```cpp
int* makeBadPointer()
{
    int local { 5 };
    return &local; // BUG: 'local' dies when the function returns
}

int* makeDynamicPointer()
{
    return new int { 5 }; // ok, but the caller must delete it
}
```

`makeBadPointer` returns the address of `local` — but `local` lives in this function's stack frame, and that frame is destroyed at `return`. The caller receives an address pointing at memory that no longer holds a valid object: a **dangling pointer**, and using it is undefined behavior.

`makeDynamicPointer` is correct, because the heap object genuinely outlives the call. But notice it solved one problem by creating another: the caller is now on the hook to `delete` it, and if they forget, the memory leaks.

The modern way to express "make a value that outlives this function, and hand off responsibility cleanly" is a smart pointer (Chapter 22):

```cpp
#include <memory>

std::unique_ptr<int> makeValue()
{
    return std::make_unique<int>(5);
}
```

Here the heap object's lifetime is tied to a `unique_ptr` that frees it automatically — flexibility of the heap, with the automatic cleanup of the stack. You'll meet this properly later; for now, just register that the dangling-stack-pointer mistake has a known, clean fix.

> **Warning:** Never return the address (or a reference) of a local variable. The local's stack frame is gone the moment the function returns, leaving a dangling pointer. If a value must outlive the call, allocate it on the heap — and prefer a smart pointer to manage it.

### Why this matters for recursion and callbacks

This isn't abstract systems trivia — it directly explains the two other big topics in this chapter. Every recursive call pushes another frame onto the stack, so deep recursion is, quite literally, a tall stack. Every call through a function pointer or a lambda is still a normal function call that does normal stack work. When a program dies with "stack overflow," it almost always means one of two things: the call chain grew too deep (often runaway recursion with no reachable base case), or some local object was simply too big.

> **Note (CS6340 tie-in):** Program analysis reasons constantly about call stacks, stack frames, heap objects, and aliasing. These aren't just runtime details — they're the very concepts a static analyzer tries to model and approximate without running the program.

---

## 20.3 — Recursion

### A function can call itself

A **recursive** function is one that calls itself. Stated baldly that sounds either trivial or terrifying, but the idea is disciplined: a recursive function solves a problem by reducing it to a *smaller version of the same problem*, and solving that.

```cpp
int sumTo(int n)
{
    if (n <= 0)
        return 0;

    return n + sumTo(n - 1);
}
```

To sum the numbers from `n` down to `1`, observe that the answer is just `n` plus "the sum from `n - 1` down to `1`" — the same problem, one size smaller. Keep shrinking and you eventually reach a size so small the answer is obvious.

Every correct recursive function has exactly two parts, and you should learn to spot both before you trust any recursion:

| Piece | Purpose |
|---|---|
| **Base case** | the smallest sub-problem, answered directly — this is what *stops* the recursion |
| **Recursive case** | reduces the problem and calls itself, moving *toward* the base case |

In `sumTo`, the base case is `n <= 0` (return `0`, no further calls) and the recursive case is `n + sumTo(n - 1)` (each call uses a strictly smaller `n`). Miss the base case — or write a recursive case that doesn't actually move toward it — and the calls never stop. Each one pushes a new stack frame, the stack grows without bound, and the program dies of a stack overflow.

> **Key insight:** Every recursive function needs a reachable **base case** that returns without recursing, and a **recursive case** that moves strictly closer to that base case on every call. If either is missing or wrong, the recursion runs forever and overflows the stack.

### Stack-frame view

Recursion is easiest to believe when you watch the stack. Take `sumTo(3)`. The calls go *down* first, each one suspended while it waits for the smaller call beneath it:

```text
sumTo(3)
  -> 3 + sumTo(2)
          -> 2 + sumTo(1)
                  -> 1 + sumTo(0)
                          -> 0      <- base case, returns directly
```

Then, with the base case answered, the suspended calls resume and return *back up*, each completing its `n + ...`:

```text
sumTo(0) = 0
sumTo(1) = 1 + 0 = 1
sumTo(2) = 2 + 1 = 3
sumTo(3) = 3 + 3 = 6
```

Each arrow going down is a new stack frame; each value coming back up is a frame being removed as its function returns. The depth of the recursion is exactly the height of that temporary stack — which is why "how deep can this go?" is always a fair question to ask of recursive code.

### Recursion is natural for recursive structures

Why bother, when a loop can sum numbers just fine? Because some data is *itself* recursively shaped, and for that data, recursive code is dramatically clearer than the loop-and-manual-stack alternative. Trees, nested expressions, graphs explored with a visited set, divide-and-conquer algorithms — all of these are built out of smaller copies of themselves, and traversing them recursively just mirrors their shape.

A binary tree is the classic example. A node has a value and two children, each of which is itself a (possibly empty) tree:

```cpp
struct Node
{
    int value {};
    Node* left {};
    Node* right {};
};

int countNodes(const Node* node)
{
    if (!node)                 // base case: an empty subtree has 0 nodes
        return 0;

    return 1 + countNodes(node->left) + countNodes(node->right);
}
```

Count this node, then add the count of everything in the left subtree and the right subtree — and "count a subtree" is the very function we're writing. The data is recursive, so the traversal is recursive, and the code is about as short as a correct solution can be. An iterative version would have to manage an explicit stack of pending nodes by hand; the recursion lets the call stack *be* that stack for you.

### Recursion can be inefficient

Recursion is clear, but clear is not the same as fast. Some recursive definitions, written in the most obvious way, recompute the same answer over and over. The textbook offender is the naive Fibonacci:

```cpp
int fibSlow(int n)
{
    if (n <= 1)
        return n;

    return fibSlow(n - 1) + fibSlow(n - 2);
}
```

It's a faithful transcription of the math, and it works — but it's wildly wasteful. Computing `fibSlow(5)` requires `fibSlow(4)` and `fibSlow(3)`; computing `fibSlow(4)` *also* requires `fibSlow(3)`; and so on, with the same sub-problems recomputed exponentially many times. The number of calls roughly doubles each time `n` grows by one. At `n = 20` you're already at thousands of calls for an answer a loop would produce in twenty steps.

When the recursion overlaps like this, an iterative loop (or *memoization* — caching answers you've already computed) is almost always the better choice:

```cpp
int fibLoop(int n)
{
    if (n <= 1)
        return n;

    int prev { 0 };
    int curr { 1 };

    for (int i { 2 }; i <= n; ++i)
    {
        int next { prev + curr };
        prev = curr;
        curr = next;
    }

    return curr;
}
```

Same answers, but linear work and no risk of deep recursion. The lesson is not "avoid recursion" — it's "recognize when a recursive definition is doing redundant work, and reach for a loop or a cache when it is."

> **Tip:** Recursion that *branches* into overlapping sub-problems (like naive Fibonacci) does exponential work. When you see the same sub-problem recomputed, switch to a loop or add memoization. Recursion that simply *shrinks* one argument (like `factorial` or digit-sum) is fine as written.

### Recursive checklist

Before you trust any recursive function — including the ones you'll write in the lab — run it past these questions:

- What exactly is the **base case**, and is it reachable from every starting input?
- Does **every** recursive path move strictly closer to that base case?
- How **deep** can the recursion go? (Deep enough to overflow the stack?)
- Is any work **duplicated** exponentially, the way naive Fibonacci is?
- Would an **iterative loop** or an explicit stack be safer or clearer here?

> **Note (CS6340 tie-in):** Many program-analysis algorithms are graph traversals. Recursion makes DFS-style traversals beautifully concise, but for large program graphs the depth can be enormous — so analysis tools often use an explicit *worklist* instead, precisely because an explicit stack is easier to bound, pause, and debug than the call stack.

The lab leans directly on this section. You'll write `factorial` (a clean shrink-one-argument recursion), a recursive `sumDigitsRecursive` to contrast with Chapter 8's loop, and the naive `fibonacci` *specifically so you can watch the exponential call tree happen*. For each one, the first thing to nail down is the base case.

---

## 20.4 — Command line arguments

### `main` can receive arguments

Every program you've written so far has started with `int main()` — no parameters. But `main` is allowed a second form, one that lets the program receive input *from the command line* the moment it launches, before any `std::cin` is read. When you type a command like `./runner input.ll --verbose`, the shell splits that line into pieces and delivers them to your program through `main`'s parameters:

```cpp
int main(int argc, char* argv[])
{
    for (int i { 0 }; i < argc; ++i)
        std::cout << i << ": " << argv[i] << '\n';
}
```

The two parameters have conventional names you'll see everywhere:

| Parameter | Meaning |
|---|---|
| `argc` | **arg**ument **c**ount — how many arguments were passed |
| `argv` | **arg**ument **v**ector — an array of C-style string pointers, one per argument |

By convention, `argv[0]` is the program's name or path, and the *user's* arguments begin at `argv[1]`. So for this command:

```bash
./runner input.ll --verbose
```

your program receives this layout:

```text
argc = 3
argv[0] -> "./runner"
argv[1] -> "input.ll"
argv[2] -> "--verbose"
```

Three strings, so `argc` is `3`, and the valid indices are `argv[0]` through `argv[2]`.

### `argv` is pointer-heavy

The type of `argv` looks intimidating, and it draws together a lot of what you learned about arrays and pointers in Chapters 17 and 19. These two parameter spellings are exactly equivalent:

```cpp
int main(int argc, char* argv[])
int main(int argc, char** argv)
```

Both say the same thing: `argv` points to the first element of an array of `char*`, and each `char*` in that array points to a null-terminated C-style string. (Recall from Chapter 17 that an array parameter decays to a pointer to its first element — `char* argv[]` *is* `char** argv`.) So `argv[i]` is one C-string, and `argv[i][j]` is the `j`-th character of the `i`-th argument.

### Convert and validate

Command-line arguments arrive as **text**, always — even a number like `42` shows up as the string `"42"`. If you want a number, you must convert it, and you must not convert until you've checked that the argument you expect is actually there. Indexing `argv[1]` when the user gave no arguments reads off the end of the array — undefined behavior — so the count check comes *first*.

```cpp
#include <iostream>
#include <string>

int main(int argc, char* argv[])
{
    if (argc != 2)                       // expect exactly one user argument
    {
        std::cerr << "usage: runner <repeat-count>\n";
        return 1;                        // non-zero exit signals failure
    }

    int repeats {};

    try
    {
        repeats = std::stoi(argv[1]);    // text -> int; throws on bad input
    }
    catch (...)
    {
        std::cerr << "repeat count must be an integer\n";
        return 1;
    }

    std::cout << "running " << repeats << " times\n";
}
```

Two things are worth noticing. First, errors go to `std::cerr`, not `std::cout` — error messages belong on the error stream so they can be separated from real output. Second, `std::stoi` (Chapter 5) does the text-to-`int` conversion and *throws* if the text isn't a number, so we wrap it in a `try`/`catch` (a preview of Chapter 27's exception handling) to turn a bad argument into a clean error message instead of a crash.

If all you need is to *compare* an argument against a known flag, you don't have to build a full `std::string`. A `std::string_view` (Chapter 5) wraps the existing characters without copying them:

```cpp
#include <string_view>

std::string_view arg { argv[1] };

if (arg == "--help")
    std::cout << "usage...\n";
```

### Command-line parsing is a boundary

Here's the mindset that matters more than any syntax: the command line is **untrusted input**, exactly like a number typed into `std::cin`. A user can pass anything — too few arguments, too many, garbage where you expected a number, a flag you've never heard of. Treat the boundary defensively:

- check `argc` before indexing `argv`;
- reject unknown modes and flags with a clear message;
- validate every numeric conversion;
- print a useful usage line on error;
- never index `argv[n]` without first confirming `argc > n`.

> **Best practice:** Always check `argc` before reading `argv[n]`, and validate every conversion. Command-line arguments are user input — a missing or malformed argument should produce a helpful error and a non-zero exit code, never undefined behavior.

> **Note (CS6340 tie-in):** LLVM passes, fuzzers, and instrumentation tools usually select their mode through command-line flags. The C++ for reading them is just `argc`/`argv` — the engineering substance is the input validation around it.

The lab's `demo.cpp` shows this in action: it reads integers off the command line and prints a small table for each. The C++ involved is exactly the `argc`/`argv` loop above.

---

## 20.5 — Ellipsis (and why to avoid them)

### C-style variadic functions

Every function you've written has a fixed number of parameters, each with a declared type, all checked by the compiler. C++ inherits one mechanism from C that breaks this rule: the **ellipsis** parameter, written `...`. It lets a function accept *any number* of extra arguments, of *any* types, with no type information in the signature at all.

```cpp
void logMany(int count, ...);
```

This is the old C variadic mechanism — it's the machinery behind `printf`. The extra arguments are invisible to the type system, so the function has to walk through them manually at runtime using macros from the `<cstdarg>` header:

```cpp
#include <cstdarg>

int sumMany(int count, ...)
{
    std::va_list args {};
    va_start(args, count);          // begin after the last named parameter

    int total {};

    for (int i { 0 }; i < count; ++i)
        total += va_arg(args, int); // read the next argument AS an int

    va_end(args);
    return total;
}
```

The caller passes the count and then the values, and is entirely responsible for getting it right:

```cpp
std::cout << sumMany(3, 10, 20, 30) << '\n'; // 60
```

### Why ellipsis is dangerous

Look closely at `va_arg(args, int)`. You are *telling* the function what type to read — the compiler does not check it against what was actually passed. That's the heart of the problem, and it makes ellipsis functions genuinely unsafe:

```cpp
sumMany(3, 10, 20);        // count says 3, but only 2 values given
sumMany(2, 10, "twenty");  // function reads ints; "twenty" is a string
```

Neither line is a compile error. The first walks off the end of the supplied arguments and reads garbage; the second reinterprets a string pointer's bytes as an `int`. The function reads bytes according to the types *it* asks for, regardless of what the caller actually passed — a direct, unguarded route to **undefined behavior**.

The trouble doesn't stop there. Ellipsis arguments have:

- **no names** — nothing to document what each one means;
- **no overload resolution** — the type-based machinery that normally picks the right call simply doesn't apply to the variadic portion;
- **poor support for class types** — non-trivial objects don't pass safely through `...`;
- **fragile conventions** — correctness depends on a count parameter or a sentinel value the caller must supply exactly right;
- **weak self-documentation** — the signature `(int, ...)` tells a future reader almost nothing.

> **Warning:** An ellipsis function's arguments are not type-checked. The callee reads whatever bytes the caller passed, interpreted as whatever type the callee asks for — a mismatch is silent undefined behavior. Do not write ellipsis functions in new code.

### Prefer typed alternatives

Modern C++ gives you safe, type-checked tools for every job people historically reached for ellipsis to do:

| Need | Prefer |
|---|---|
| a known, finite set of signatures | function overloads (Chapter 11) |
| many values of the same type | `std::vector`, `std::span`, or an initializer list |
| a generic, compile-time argument list | variadic *templates* (a later topic) |
| formatted text output | a type-safe formatting facility or stream composition |

The initializer-list approach is the easiest to reach for and makes the argument type visible and checked:

```cpp
#include <initializer_list>

int sum(std::initializer_list<int> values)
{
    int total {};

    for (int value : values)
        total += value;

    return total;
}

std::cout << sum({ 10, 20, 30 }) << '\n'; // 60
```

Compare this with `sumMany`: there's no separate count to keep in sync, the element type `int` is right there in the signature, and passing a string would be a compile error rather than a runtime catastrophe.

### When you still see ellipsis

You won't write ellipsis functions, but you will *encounter* them — in C APIs, in low-level logging wrappers, in legacy code, and in compiler or platform interfaces where the convention is entrenched. When you read one, read it carefully: part of its contract lives outside the type system, in documentation and the caller's discipline rather than in anything the compiler enforces. That means tests and docs have to carry weight the type checker normally would.

---

## 20.6 — Introduction to lambdas (anonymous functions)

### Lambdas define local callable objects

Back in 20.1 you saw how to pass behavior into a function using a function pointer. It works, but it has an irritating cost: to pass a one-line predicate, you have to *stop*, scroll somewhere else, and define a whole named function — even though you'll use it exactly once and the logic is two characters of code. The behavior ends up far from where it's used.

A **lambda** fixes that. A lambda is an unnamed, function-like object you write *inline*, right at the point where you need it:

```cpp
auto isEven = [](int value)
{
    return value % 2 == 0;
};

std::cout << isEven(6) << '\n'; // 1 (true)
```

`isEven` behaves like a function — you call it with parentheses — but you defined it as an expression, in place, without ever giving it a return-type-and-name header. The general shape is:

```cpp
[captures](parameters) -> return_type
{
    body
}
```

The `[captures]` part is new and is the subject of the next lesson; for now just know that empty brackets `[]` mean "this lambda uses nothing from the surrounding scope." The `-> return_type` is usually optional, because the compiler deduces the return type from your `return` statements:

```cpp
auto square = [](int x)
{
    return x * x; // return type deduced as int
};
```

### Why lambdas are useful

The natural home of a lambda is right next to an algorithm that takes a callback. Recall `std::find_if` from Chapter 18: it walks a range and returns the first element for which a predicate is true. With a lambda, the predicate lives *at the call site*, where a reader can see it without hunting:

```cpp
#include <algorithm>
#include <vector>

std::vector<int> values { 4, 1, 9, 2 };

auto firstLarge = std::find_if(values.begin(), values.end(), [](int value)
{
    return value > 5;
});
```

In Chapter 18 you had to write a separate named function for that one-use predicate and pass its address. The lambda says the same thing in one place. The intent — "find the first value greater than 5" — is right where you're looking.

> **Note (CS6340 tie-in):** In LLVM-style code you'll constantly write tiny predicates next to a traversal — "is this a call instruction?", "does this block have a single successor?" A lambda lets that decision sit inline with the loop that uses it, instead of cluttering the file with single-use named functions.

### Lambdas have unique closure types

Here's a detail that explains a lot of later behavior: every lambda *expression* you write generates a brand-new, compiler-invented type, unique to that one lambda. That type is called a **closure type**, and the object the expression produces is a **closure**. You can't name the type — it has no spelling you could write — which is exactly why we store lambdas in `auto`:

```cpp
auto lessByAbs = [](int a, int b)
{
    return std::abs(a) < std::abs(b);
};
```

`auto` captures that unnameable type for you with zero overhead. Reach for `std::function` only when you specifically need *type erasure* — that is, when one variable or parameter must hold different callables that share a call signature:

```cpp
std::function<bool(int, int)> compare { lessByAbs };
```

The rule of thumb: `auto` to *store* a particular lambda cheaply; `std::function` when you need one slot that can hold *many different* compatible callables (which is exactly the lab's `countMatching` situation).

### Generic lambdas

Give a lambda an `auto` parameter and it becomes **generic** — it works on whatever type you call it with, like a tiny function template:

```cpp
auto printTwice = [](const auto& value)
{
    std::cout << value << ' ' << value << '\n';
};

printTwice(7);
printTwice(std::string { "token" });
```

The compiler generates a separate version of the call operator for each argument type you actually use — one for `int`, one for `std::string` here. It's the same mechanism as a function template (Chapter 11), just written inline. (The lab does *not* require generic lambdas — for its `std::function<bool(int)>` parameter, a plain `int` lambda is the right tool — but they're worth recognizing.)

### Constexpr lambdas

A lambda whose body and arguments are all eligible for compile-time evaluation can be used in a constant expression — handy when you want the compiler to do the work before the program even runs:

```cpp
constexpr auto cube = [](int x)
{
    return x * x * x;
};

static_assert(cube(3) == 27); // checked at compile time
```

Don't sprinkle `constexpr` everywhere out of habit. Use it when compile-time evaluation is genuinely part of the design.

### Static locals in generic lambdas

This one is a subtle trap worth flagging. A generic lambda is, under the hood, a *templated* call operator — so a `static` local variable inside it is **separate for each type** the lambda is instantiated with:

```cpp
auto countCalls = [](auto)
{
    static int calls {};
    return ++calls;
};

std::cout << countCalls(1) << '\n';    // 1  (int version)
std::cout << countCalls(2) << '\n';    // 2  (same int version)
std::cout << countCalls(1.5) << '\n';  // 1  (double version — its own counter!)
```

The `int` calls share one `calls`; the `double` call gets a fresh, independent one. That's almost never what a reader expects. Avoid hidden state in generic lambdas unless the separate-per-type behavior is exactly what you want.

> **Warning:** A `static` local inside a *generic* (`auto`-parameter) lambda is duplicated per instantiated type, so each type gets its own independent copy. This surprises almost everyone — keep mutable state out of generic lambdas unless you truly intend per-type counters.

### Trailing return types

The `-> type` after the parameter list is the lambda's **trailing return type**. You'll usually omit it and let deduction work, but it's the right tool when deduction would be ambiguous — most commonly when different `return` statements would otherwise deduce to *different* types and you need to pin down one:

```cpp
auto classify = [](int score) -> std::string_view
{
    if (score >= 90)
        return "high";

    return "normal";
};
```

Stating `-> std::string_view` makes the contract explicit and heads off any disagreement between the two return statements.

### Standard library function objects

Before lambdas existed, the standard library shipped a set of ready-made **function objects** for common operations, and they're still around in `<functional>`:

```cpp
#include <functional>

std::plus<int> add {};
std::cout << add(2, 3) << '\n'; // 5
```

In everyday application code, a lambda is usually clearer for a local operation. These named function objects earn their place mostly in generic library code, where having an actual named type to pass around is convenient.

---

## 20.7 — Lambda captures

### The capture clause

We've been writing `[]` and waving at it. Now it's time to use it. Those square brackets are the **capture clause**, and they decide which variables from the *surrounding* scope the lambda is allowed to use inside its body. A lambda cannot reach out and grab a local variable just because it's nearby — it can only use what it explicitly captures.

```cpp
int threshold { 10 };

auto isLarge = [threshold](int value)
{
    return value > threshold;
};
```

By naming `threshold` in the brackets, we **capture it by value**: the lambda stores its own private copy of `threshold`, taken at the moment the lambda is created. To capture by *reference* instead, prefix the name with `&`:

```cpp
int matches {};

auto countIfLarge = [&matches, threshold](int value)
{
    if (value > threshold)
        ++matches;
};
```

Here `matches` is captured by reference — the lambda touches the *original* variable in the enclosing scope, so incrementing it inside the lambda changes the real `matches` outside. Meanwhile `threshold` is still captured by value, a private copy. One lambda, two capture modes, chosen per variable.

### Captures become data members

Why do captures behave like this — why do copies persist, and why does lifetime suddenly matter? Because a capturing lambda is, quite literally, a small object, and the captured variables are its data members. The closure type the compiler invents for it is roughly equivalent to a hand-written class:

```cpp
int threshold { 10 };

auto isLarge = [threshold](int value)
{
    return value > threshold;
};
```

You can picture the generated type like this:

```cpp
class CompilerMadeClosure
{
private:
    int thresholdCopy;          // the captured value lives here

public:
    bool operator()(int value) const   // calling the lambda calls this
    {
        return value > thresholdCopy;
    }
};
```

This isn't the exact code the compiler emits, but it's an accurate mental model — and it explains everything that follows. Captures are *members*, so a by-value capture is a copy that has its own lifetime; calling the lambda *invokes `operator()`*, which is why the call operator's const-ness matters; and a by-reference capture is a member reference, which dangles if the referent dies first.

> **Key insight:** A capturing lambda is an object whose captures are data members and whose body is an `operator()`. Once you see it that way, every rule about captures — copies, const-ness, lifetimes, dangling — follows from ordinary object behavior.

### By-value captures are const by default

That `const` on `operator()` in the model above is real. The lambda's call operator is `const` by default, which means it may not modify its own by-value captures from inside the body:

```cpp
int calls {};

auto f = [calls]()
{
    // ++calls; // ERROR: the captured copy is const inside operator()
};
```

If you *want* the lambda's private copy to change between calls, add the `mutable` keyword. It removes the `const` from the call operator, letting the body modify its own captures:

```cpp
int calls {};

auto next = [calls]() mutable
{
    return ++calls;
};

std::cout << next() << '\n'; // 1
std::cout << next() << '\n'; // 2
```

Note carefully: `mutable` lets the lambda change *its own copy*. The outside `calls` is still `0` — it was never touched, because the capture was by value. `mutable` gives the closure internal, persistent state; it does not reach back into the enclosing scope.

### Capture by reference

A by-reference capture is the tool for the opposite goal: actually affecting the original object. It's the natural way to accumulate a result while a lambda runs:

```cpp
int total {};

auto addToTotal = [&total](int value)
{
    total += value;
};

addToTotal(5);
addToTotal(7);

std::cout << total << '\n'; // 12
```

This is powerful and it is dangerous in equal measure. The lambda holds a reference, so the captured object **must outlive the lambda**. As long as `addToTotal` is used right here, in the scope where `total` lives, all is well. The danger appears the moment the lambda escapes that scope.

### Dangling captures

This is the lambda version of the dangling-pointer mistake from 20.2, and it's just as easy to make. Capture a local by reference and then let the lambda outlive the function, and the reference is left pointing at a destroyed object:

```cpp
auto makeBadPredicate()
{
    int limit { 5 };

    return [&limit](int value) // captures a reference to a local...
    {
        return value > limit;
    };
} // ...but 'limit' is destroyed right here, when the function returns
```

The returned lambda carries a reference to `limit`, which no longer exists — every call through it is undefined behavior. The fix is to capture **by value**, so the lambda owns its own copy that survives the function's return:

```cpp
auto makePredicate(int limit)
{
    return [limit](int value) // by value: the lambda owns a copy of limit
    {
        return value > limit;
    };
}
```

> **Warning:** Capturing by reference is only safe while the captured object clearly outlives the lambda. If a lambda might escape the current scope — returned, stored, or handed to something that keeps it — capture by value so it owns its data. A returned lambda holding a reference to a local is a dangling reference.

### Multiple captures

You can list several captures, mixing modes freely, separated by commas:

```cpp
int minScore { 70 };
int accepted {};

auto accept = [minScore, &accepted](int score)
{
    if (score >= minScore)
    {
        ++accepted;
        return true;
    }

    return false;
};
```

`minScore` is read-only configuration, so it's captured by value; `accepted` is a running tally we want to update in the outer scope, so it's captured by reference. Listing each one explicitly makes the lambda's data dependencies plain to anyone reading it — which is exactly why it's the recommended style for teaching and lab code.

### Default captures

If naming every variable feels verbose, C++ offers **default captures** that grab whatever the body happens to use. `[=]` captures everything used by value; `[&]` captures everything used by reference:

```cpp
[=](int value) { return value > threshold; } // captures threshold by value
[&](int value) { total += value; }            // captures total by reference
```

They're concise, but concision here costs clarity: a default capture *hides* which outside variables the lambda depends on, so a reader has to scan the whole body to find out. For small, local, throwaway lambdas that's fine. For code you expect to read and maintain, prefer explicit captures — they document the data flow and they don't silently capture something new when you later edit the body.

> **Best practice:** Prefer explicit captures (`[x, &y]`) over default captures (`[=]`, `[&]`) in code you'll maintain. Explicit captures make the lambda's data dependencies visible and won't quietly pull in a new variable when the body changes later.

### Init captures

Sometimes you want the lambda to hold a value that *isn't* simply a copy of an existing variable. An **init capture** lets you create a new captured member and initialize it with an expression:

```cpp
auto ownsName = [name = std::string { "coverage" }]()
{
    std::cout << name << '\n';
};
```

The most important use of this is *moving* a value into a lambda — handing the lambda sole ownership of something that can't or shouldn't be copied:

```cpp
auto ptr = std::make_unique<int>(42);

auto print = [value = std::move(ptr)]() // move ptr's resource into the lambda
{
    std::cout << *value << '\n';
};
```

After the move, `ptr` is empty and the lambda owns the `unique_ptr` outright. (Move semantics and `unique_ptr` are Chapter 22's topic — this is a preview of where init captures become essential.)

### Capturing `this`

Inside a member function, a lambda often needs to reach the current object's members. Capturing `this` gives it that access:

```cpp
class Counter
{
private:
    int m_base {};

public:
    auto makeAdder() const
    {
        return [this](int value)
        {
            return m_base + value; // reaches the object's member via 'this'
        };
    }
};
```

Treat a `this` capture as a **lifetime promise**: the lambda now depends on the object staying alive. Capturing `this` is effectively capturing a pointer to the object by reference — if the `Counter` is destroyed while the returned lambda is still in use, that lambda refers to a dead object, the same dangling trap in a new costume.

### Copies of mutable lambdas

One last consequence of "a lambda is an object": lambdas can be copied, and a copy carries its *own* independent state. For a `mutable` lambda with internal state, this means each copy counts separately:

```cpp
auto counter = [n = 0]() mutable
{
    return ++n;
};

auto copy { counter };

std::cout << counter() << '\n'; // 1
std::cout << copy() << '\n';    // 1  — the copy has its own n
```

`copy` was duplicated while `counter`'s `n` was still `0`, so it starts its own count from there. This matters whenever you pass a stateful lambda *by value* into a function or store it in a `std::function` — you may be operating on a copy, not the original, and their states diverge.

### Capture rules of thumb

Pulling it together, here's how to decide what to capture and how:

- Capture **nothing** (`[]`) when the lambda needs no outside state — it's the simplest and safest.
- Capture **by value** when the lambda may outlive the current scope, so it owns its data.
- Capture **by reference** only when the lambda is short-lived *and* the referenced objects clearly outlive it.
- Prefer **explicit** captures in code you expect to maintain.
- Treat a **`this` capture** as a promise that the object stays alive.

The lab's final task, `countMatching`, exercises the payoff of this whole chapter. It takes a `std::function<bool(int)>` and must accept three kinds of callable through that one parameter: a plain function pointer (`&isEven`), a non-capturing lambda, and — the real test — a *capturing* lambda like `[threshold](int x){ return x >= threshold; }`. That last one is a closure object with `threshold` stored inside it, which is precisely why the parameter has to be a `std::function` and not a raw function pointer. Capture by value there, and the lambda safely owns its copy of `threshold` even after the surrounding variable is gone.

---

## 20.x — Chapter 20 summary and quiz

### Feature map

This chapter introduced several distinct tools that all serve one theme. Here's when to reach for each, and what to watch out for:

| Feature | Use it for | Watch out for |
|---|---|---|
| function pointer | a simple callback to a plain function | exact signature match, null-pointer guard, can't hold captures |
| `std::function` | storing or passing many callable kinds uniformly | type-erasure overhead, possible allocation |
| recursion | naturally recursive problems (trees, divide-and-conquer) | reachable base case, stack depth, duplicated work |
| `argc` / `argv` | startup configuration from the command line | text conversion and input validation |
| ellipsis | reading legacy C variadic APIs only | weak type checking — avoid writing in new code |
| lambda | local, inline callable behavior | capture lifetime and copy semantics |

### One mental model

If you remember one idea from this chapter, make it this: **Chapter 20 is about treating behavior and calls as data.** Each tool is a different way of capturing "something to do" and moving it around:

```text
function pointer:   the address of some behavior
lambda:             a local behavior object (with captured state)
std::function:      a type-erased wrapper that holds any of the above
recursion:          behavior that calls itself on a smaller input
argc / argv:        startup data entering main from the outside
ellipsis:           untyped extra data entering a function (avoid)
```

The stack-and-heap lesson underpins all of it: recursion is bounded by stack depth, captured references are bounded by object lifetimes, and a returned pointer or reference to a local is the same mistake whether it hides in a function or a lambda.

> **Note (CS6340 tie-in):** Program-analysis code is wall-to-wall traversal logic — "visit each function," "keep the instructions matching this predicate," "run this callback when a branch is found," "recurse through this graph unless we've already seen the node." Lambdas and typed callables are the modern, readable way to express those small, local decisions, and recursion (carefully bounded) is how you walk the structures they run over.

When you've finished the reading, the `calc-core` lab puts all three movements together: recursion (`factorial`, `sumDigitsRecursive`, `fibonacci`), function pointers (`apply`, and `pickOp` returning one), and lambdas through `std::function` (`countMatching`). For each piece, the chapter has given you the rule to check first — the base case, the null guard, the capture mode. Go make `make test` turn green.
