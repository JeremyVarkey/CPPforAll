# Chapter 20 — Functions

> Source: <https://www.learncpp.com/> (Chapter 20)
> One file per chapter. Each lesson is a section below.

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

A function is compiled code stored somewhere in the program. A function pointer
stores the address of a function with a particular signature.

```cpp
int add(int x, int y)
{
    return x + y;
}

int (*operation)(int, int) { &add };

std::cout << operation(2, 3) << '\n'; // calls add(2, 3)
```

The syntax is noisy because the `*operation` must be grouped:

```cpp
int (*operation)(int, int)
//  ^ return type
//      ^ pointer variable name
//                 ^ parameter types
```

Without parentheses, the declaration means something else:

```cpp
int* operation(int, int); // function returning int*
```

### Assigning functions

The address-of operator is optional in many function-pointer assignments, but
using it can make the intent obvious:

```cpp
int multiply(int x, int y)
{
    return x * y;
}

int (*op)(int, int) { &multiply };
```

The pointed-to function must match the function pointer type:

```cpp
double divide(double, double);

// int (*op)(int, int) { &divide }; // not the same signature
```

Overloaded functions may require help because the name alone can refer to
multiple functions:

```cpp
int score(int);
int score(int, int);

int (*singleScore)(int) { &score }; // context selects score(int)
```

### Calling through a pointer

Both spellings are common:

```cpp
int result1 { operation(4, 5) };
int result2 { (*operation)(4, 5) };
```

The first reads like an ordinary function call. The second emphasizes that a
pointer is being dereferenced.

Check for null before calling if the pointer may be empty:

```cpp
int (*op)(int, int) {};

if (op)
    std::cout << op(1, 2) << '\n';
```

Calling a null function pointer is undefined behavior.

### Default arguments do not travel through function pointers

Default arguments belong to the function declaration visible at the call site.
When a function is called through a pointer, the pointer type does not carry
those defaults in a useful way.

```cpp
void logLevel(int level = 1);

void (*logger)(int) { &logLevel };

logger(2);  // ok
// logger(); // not ok: pointer call needs the argument
```

If a callback needs a default behavior, wrap the call in a normal function or
lambda that supplies the missing argument.

### Passing behavior into a function

Function pointers are useful for callbacks: one function can receive behavior as
an argument.

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

The algorithm stays the same; the comparison rule changes.

CS6340 tie-in: callbacks are a conceptual ancestor of passing predicates into
algorithms. In lab-style LLVM code, this shows up more often as a lambda passed
to `std::find_if`, `std::any_of`, or a custom traversal.

### Default callback functions

A function can accept a function pointer with a default value:

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

This says, "if the caller does not provide a predicate, use this ordinary
function."

### Type aliases make this readable

Function pointer types are much easier to read through an alias:

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

The alias names the role instead of forcing the reader to parse punctuation.

### `std::function`

`std::function` is a general callable wrapper. It can store ordinary functions,
function pointers, lambdas, and function objects with a compatible call shape.

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

Tradeoff:

| Tool | Strength | Cost |
|---|---|---|
| raw function pointer | simple, cheap, can point to non-capturing functions | cannot store capturing lambdas |
| `std::function` | flexible, stores many callable types | type erasure overhead and possible allocation |
| template callable parameter | fastest and most flexible at compile time | function body usually must live in header |

### `auto` and callable types

`auto` can reduce clutter when the exact function pointer type is clear:

```cpp
auto op { &add };
std::cout << op(1, 2) << '\n';
```

For function parameters, prefer a named alias, `std::function`, or a template
parameter depending on the design goal.

---

## 20.2 — The stack and the heap

### Program memory is divided by purpose

The exact layout is implementation-dependent, but a useful mental model has
several regions:

```
program memory
+-----------------------------+
| code / text segment          |
| static and global storage    |
| heap / free store            |
| ...                          |
| stack                        |
+-----------------------------+
```

The stack and heap are the two regions most visible in ordinary C++ code.

### The call stack

Each active function call gets a stack frame. The frame stores local automatic
objects, parameters, return bookkeeping, and other implementation details.

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

During `square(5)`:

```
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

When `square` returns, its frame is removed. Its local variables no longer
exist.

### Stack properties

The stack is:

- fast to allocate and deallocate,
- automatically cleaned up when scopes end,
- limited in size,
- tied to nested function-call structure.

Large local arrays can exhaust stack space:

```cpp
void risky()
{
    int values[1'000'000] {};
}
```

Prefer `std::vector<int>` when the size is large or runtime-dependent.

### Heap properties

The heap is used for dynamic allocation:

```cpp
int* value { new int { 42 } };
delete value;
```

The heap is:

- more flexible than the stack,
- able to hold objects whose lifetime crosses function boundaries,
- manually managed unless wrapped by RAII,
- usually slower to allocate from than the stack,
- vulnerable to leaks and dangling pointers in raw-pointer code.

### Stack vs heap lifetimes

```cpp
int* makeBadPointer()
{
    int local { 5 };
    return &local; // wrong: local dies when function returns
}

int* makeDynamicPointer()
{
    return new int { 5 }; // caller must delete
}
```

The first function returns an address to a dead stack object. The second returns
an address to a live heap object, but now ownership has been pushed onto the
caller.

Modern version:

```cpp
#include <memory>

std::unique_ptr<int> makeValue()
{
    return std::make_unique<int>(5);
}
```

### Why this matters for recursion and callbacks

Every recursive call consumes another stack frame. Every function pointer or
lambda call still creates normal function-call activity. When code crashes from
"stack overflow," it often means the call chain grew too deep or local stack
objects were too large.

CS6340 tie-in: program analysis often reasons about call stacks, stack frames,
heap objects, and aliasing. These are not just runtime implementation details;
they are also the concepts static analysis tries to approximate.

---

## 20.3 — Recursion

### A function can call itself

Recursion means a function solves a problem by calling itself on a smaller or
simpler version of the same problem.

```cpp
int sumTo(int n)
{
    if (n <= 0)
        return 0;

    return n + sumTo(n - 1);
}
```

Two pieces are required:

| Piece | Purpose |
|---|---|
| Base case | stops the recursion |
| Recursive case | moves toward the base case |

Without a reachable base case, recursion keeps adding stack frames until the
program fails.

### Stack-frame view

For `sumTo(3)`:

```
sumTo(3)
  -> 3 + sumTo(2)
          -> 2 + sumTo(1)
                  -> 1 + sumTo(0)
                          -> 0
```

Then the calls return in reverse:

```
sumTo(0) = 0
sumTo(1) = 1 + 0 = 1
sumTo(2) = 2 + 1 = 3
sumTo(3) = 3 + 3 = 6
```

### Recursion is natural for recursive structures

Recursive code is often clearest for trees, nested syntax, graphs with visited
sets, and divide-and-conquer algorithms.

```cpp
struct Node
{
    int value {};
    Node* left {};
    Node* right {};
};

int countNodes(const Node* node)
{
    if (!node)
        return 0;

    return 1 + countNodes(node->left) + countNodes(node->right);
}
```

The data shape is recursive, so the traversal is naturally recursive.

### Recursion can be inefficient

Some recursive definitions recompute the same work many times:

```cpp
int fibSlow(int n)
{
    if (n <= 1)
        return n;

    return fibSlow(n - 1) + fibSlow(n - 2);
}
```

This is mathematically tidy but computationally expensive. A loop or memoization
is usually better:

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

### Recursive checklist

Before trusting recursive code, ask:

- What is the base case?
- Does every path move closer to that base case?
- How deep can the recursion get?
- Is any work duplicated exponentially?
- Would an iterative loop or explicit stack be safer?

CS6340 tie-in: many analysis algorithms are graph traversals. Recursion can make
DFS-style algorithms concise, but for large program graphs an explicit worklist
is often easier to bound and debug.

---

## 20.4 — Command line arguments

### `main` can receive arguments

Command-line arguments let the shell pass text into a program at startup:

```cpp
int main(int argc, char* argv[])
{
    for (int i { 0 }; i < argc; ++i)
        std::cout << i << ": " << argv[i] << '\n';
}
```

The conventional names are:

| Parameter | Meaning |
|---|---|
| `argc` | argument count |
| `argv` | argument vector: array of C-style string pointers |

`argv[0]` is usually the program name or path. Actual user-provided arguments
begin at `argv[1]`.

Example command:

```bash
./runner input.ll --verbose
```

Possible argument layout:

```
argc = 3
argv[0] -> "./runner"
argv[1] -> "input.ll"
argv[2] -> "--verbose"
```

### `argv` is pointer-heavy

These forms are equivalent as parameters:

```cpp
int main(int argc, char* argv[])
int main(int argc, char** argv)
```

Both mean `argv` points to the first element of an array of character pointers.
Each `argv[i]` points to a null-terminated C-style string.

### Convert and validate

Arguments arrive as text. Convert only after checking that the expected
argument exists.

```cpp
#include <iostream>
#include <string>

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cerr << "usage: runner <repeat-count>\n";
        return 1;
    }

    int repeats {};

    try
    {
        repeats = std::stoi(argv[1]);
    }
    catch (...)
    {
        std::cerr << "repeat count must be an integer\n";
        return 1;
    }

    std::cout << "running " << repeats << " times\n";
}
```

For simple comparisons, `std::string_view` can avoid constructing a full
`std::string`:

```cpp
#include <string_view>

std::string_view arg { argv[1] };

if (arg == "--help")
    std::cout << "usage...\n";
```

### Command-line parsing is a boundary

Treat CLI arguments like user input:

- check counts,
- reject unknown modes,
- validate numeric conversions,
- print useful errors,
- avoid indexing `argv[n]` before checking `argc`.

CS6340 tie-in: LLVM passes, fuzzers, and instrumentation tools often expose
their mode through CLI flags. The C++ syntax is just `argc` / `argv`, but the
engineering issue is input validation.

---

## 20.5 — Ellipsis (and why to avoid them)

### C-style variadic functions

An ellipsis parameter allows a function to accept extra arguments with no type
information in the function signature:

```cpp
void logMany(int count, ...);
```

This is the old C-style variadic mechanism behind functions such as `printf`.
The function has to use macros from `<cstdarg>` to walk through the arguments.

```cpp
#include <cstdarg>

int sumMany(int count, ...)
{
    std::va_list args {};
    va_start(args, count);

    int total {};

    for (int i { 0 }; i < count; ++i)
        total += va_arg(args, int);

    va_end(args);
    return total;
}
```

The caller is responsible for passing exactly what the callee expects:

```cpp
std::cout << sumMany(3, 10, 20, 30) << '\n';
```

### Why ellipsis is dangerous

The compiler cannot fully check the extra arguments:

```cpp
sumMany(3, 10, 20);        // count says 3, but only 2 values passed
sumMany(2, 10, "twenty");  // function expects int values
```

The callee will read bytes according to the types it asks for, even if the
caller passed something else. That is a direct route to undefined behavior.

Other problems:

- no names for the extra arguments,
- no normal overload resolution inside the ellipsis portion,
- limited support for non-trivial class types,
- fragile count/sentinel conventions,
- poor self-documentation.

### Prefer typed alternatives

Modern C++ has better options:

| Need | Prefer |
|---|---|
| a known finite set of signatures | overloads |
| many values of the same type | `std::span`, `std::vector`, initializer list |
| generic compile-time argument list | variadic templates |
| formatted text | type-safe formatting library / stream composition |

Example with initializer list:

```cpp
#include <initializer_list>

int sum(std::initializer_list<int> values)
{
    int total {};

    for (int value : values)
        total += value;

    return total;
}

std::cout << sum({ 10, 20, 30 }) << '\n';
```

The argument type is now visible and checked.

### When you still see ellipsis

You may encounter ellipsis in:

- C APIs,
- low-level logging wrappers,
- legacy code,
- compiler or platform interfaces.

Read these functions cautiously. The function contract is partly outside the
type system, so tests and documentation have to carry more weight.

---

## 20.6 — Introduction to lambdas (anonymous functions)

### Lambdas define local callable objects

A lambda is an unnamed function-like object written inline.

```cpp
auto isEven = [](int value)
{
    return value % 2 == 0;
};

std::cout << isEven(6) << '\n';
```

Basic shape:

```cpp
[captures](parameters) -> return_type
{
    body
}
```

Many lambdas omit the return type because the compiler can deduce it:

```cpp
auto square = [](int x)
{
    return x * x;
};
```

### Why lambdas are useful

Lambdas are ideal when behavior is small and local to the call site:

```cpp
#include <algorithm>
#include <vector>

std::vector<int> values { 4, 1, 9, 2 };

auto firstLarge = std::find_if(values.begin(), values.end(), [](int value)
{
    return value > 5;
});
```

Without the lambda, you would need to name a separate function for a one-use
predicate.

CS6340 tie-in:

```cpp
auto isCallInstruction = [](const Instruction& instruction)
{
    return instruction.getOpcodeName() == std::string_view { "call" };
};
```

The actual LLVM code will use LLVM's type/query APIs, but the idea is the same:
write a tiny predicate near the traversal that uses it.

### Lambdas have unique closure types

Every lambda expression creates a unique compiler-generated type. That type is
called a closure type. Store it with `auto` when you do not need type erasure:

```cpp
auto lessByAbs = [](int a, int b)
{
    return std::abs(a) < std::abs(b);
};
```

Use `std::function` when you need one variable or parameter type that can hold
different compatible callables:

```cpp
std::function<bool(int, int)> compare { lessByAbs };
```

### Generic lambdas

An `auto` parameter makes a lambda generic:

```cpp
auto printTwice = [](const auto& value)
{
    std::cout << value << ' ' << value << '\n';
};

printTwice(7);
printTwice(std::string { "token" });
```

This behaves like a tiny function template. The compiler generates the needed
call operator for each argument type used.

### Constexpr lambdas

A lambda can be usable in constant expressions when its body and arguments allow
compile-time evaluation:

```cpp
constexpr auto cube = [](int x)
{
    return x * x * x;
};

static_assert(cube(3) == 27);
```

Do not force `constexpr` everywhere. Use it when compile-time evaluation is part
of the design.

### Static locals in generic lambdas

Because a generic lambda behaves like a templated call operator, a static local
inside it is separate for each instantiated argument type:

```cpp
auto countCalls = [](auto)
{
    static int calls {};
    return ++calls;
};

std::cout << countCalls(1) << '\n';    // int specialization
std::cout << countCalls(2) << '\n';    // same int specialization
std::cout << countCalls(1.5) << '\n';  // double specialization
```

This can surprise readers. Avoid hidden state in generic lambdas unless the
separate-per-type behavior is intentional.

### Trailing return types

Use a trailing return type when deduction would be unclear or when different
branches need one agreed return type:

```cpp
auto classify = [](int score) -> std::string_view
{
    if (score >= 90)
        return "high";

    return "normal";
};
```

The `-> std::string_view` makes the contract visible.

### Standard library function objects

The standard library also provides reusable callable objects:

```cpp
#include <functional>

std::plus<int> add {};
std::cout << add(2, 3) << '\n';
```

In everyday code, lambdas are usually clearer for local behavior, while named
function objects matter more in generic library code.

---

## 20.7 — Lambda captures

### The capture clause

The square brackets at the front of a lambda decide which outside variables the
lambda can use.

```cpp
int threshold { 10 };

auto isLarge = [threshold](int value)
{
    return value > threshold;
};
```

`threshold` is captured by value. The lambda stores its own copy.

Capture by reference uses `&`:

```cpp
int matches {};

auto countIfLarge = [&matches, threshold](int value)
{
    if (value > threshold)
        ++matches;
};
```

Here `matches` is modified in the surrounding scope, while `threshold` is copied.

### Captures become data members

A capturing lambda is best understood as a small object:

```cpp
int threshold { 10 };

auto isLarge = [threshold](int value)
{
    return value > threshold;
};
```

Mental model:

```cpp
class CompilerMadeClosure
{
private:
    int thresholdCopy;

public:
    bool operator()(int value) const
    {
        return value > thresholdCopy;
    }
};
```

That is not the exact generated code, but it explains why captures have lifetime
and copying behavior.

### By-value captures are const by default

The lambda call operator is `const` by default, so copied captures cannot be
modified inside the lambda body:

```cpp
int calls {};

auto f = [calls]()
{
    // ++calls; // not allowed: captured copy is const inside operator()
};
```

Use `mutable` if the lambda's own copy should change:

```cpp
int calls {};

auto next = [calls]() mutable
{
    return ++calls;
};

std::cout << next() << '\n';
std::cout << next() << '\n';
```

The outside `calls` is still unchanged. Only the closure object's copy changes.

### Capture by reference

Reference capture lets the lambda work with the original object:

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

This is powerful and dangerous. The referenced object must outlive the lambda.

### Dangling captures

Returning a lambda that captured a local variable by reference creates a dangling
reference:

```cpp
auto makeBadPredicate()
{
    int limit { 5 };

    return [&limit](int value)
    {
        return value > limit;
    };
} // limit is destroyed here
```

The returned lambda refers to an object that no longer exists. Capture by value
is usually the right choice when a lambda escapes the current scope.

```cpp
auto makePredicate(int limit)
{
    return [limit](int value)
    {
        return value > limit;
    };
}
```

### Multiple captures

Captures can be mixed:

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

Prefer explicit captures for teaching and lab code. They make the data
dependencies visible.

### Default captures

Default capture by value:

```cpp
[=](int value) { return value > threshold; }
```

Default capture by reference:

```cpp
[&](int value) { total += value; }
```

Default captures are concise, but they hide which outside variables are used.
For small local examples they are fine. For code that may evolve, explicit
captures are easier to audit.

### Init captures

An init capture creates a new captured variable:

```cpp
auto ownsName = [name = std::string { "coverage" }]()
{
    std::cout << name << '\n';
};
```

This is useful for moving values into a lambda:

```cpp
auto ptr = std::make_unique<int>(42);

auto print = [value = std::move(ptr)]()
{
    std::cout << *value << '\n';
};
```

After the move, `ptr` is empty and the lambda owns the `unique_ptr`.

### Capturing `this`

Inside a member function, lambdas often need access to the current object:

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
            return m_base + value;
        };
    }
};
```

The lambda stores access to the current object. The object must outlive the
lambda, or the lambda will refer to a destroyed object.

### Copies of mutable lambdas

Lambdas are objects and can be copied. If a mutable lambda has internal state,
each copy has its own state:

```cpp
auto counter = [n = 0]() mutable
{
    return ++n;
};

auto copy { counter };

std::cout << counter() << '\n'; // 1
std::cout << copy() << '\n';    // 1, separate copy
```

This matters when passing stateful lambdas into functions or storing them in
`std::function`.

### Capture rules of thumb

- Capture nothing when the lambda does not need outside state.
- Capture by value when the lambda may outlive the current scope.
- Capture by reference only when the lambda is short-lived and the referenced
  objects clearly outlive it.
- Prefer explicit captures in code you expect to maintain.
- Treat `this` capture as a lifetime promise.

---

## 20.x — Chapter 20 summary and quiz

### Feature map

| Feature | Use it for | Watch out for |
|---|---|---|
| function pointer | simple callback to ordinary function | exact signature, null pointer, no captures |
| `std::function` | store/pass many callable kinds uniformly | overhead, type erasure |
| recursion | naturally recursive problems | base case, stack depth, duplicate work |
| `argc` / `argv` | startup configuration | text conversion and validation |
| ellipsis | legacy C variadic APIs | weak type checking; avoid in new code |
| lambda | local callable behavior | capture lifetime and copy behavior |

### One mental model

Chapter 20 is about treating behavior and calls as data:

```
function pointer:   address of behavior
lambda:             local behavior object
std::function:      type-erased behavior wrapper
recursion:          behavior calling itself
argc / argv:        startup data entering main
ellipsis:           untyped extra data entering a function
```

For CS6340, this matters because program-analysis code is full of traversal
logic: "visit each function", "keep instructions matching this predicate",
"run this callback when a branch is found", "recurse through this graph unless
already seen." Lambdas and typed callables are the modern way to express those
small, local decisions.
