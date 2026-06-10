# Chapter 27 — Exceptions

> Source: <https://www.learncpp.com/> (Chapter 27)
>
> Goal for CS6340: understand exception syntax, stack unwinding, RAII cleanup,
> and `noexcept` well enough to read C++ error paths and avoid adding fragile
> cleanup or throwing behavior in the wrong place.

## Contents

- [27.1 — The need for exceptions](#271--the-need-for-exceptions)
- [27.2 — Basic exception handling](#272--basic-exception-handling)
- [27.3 — Exceptions, functions, and stack unwinding](#273--exceptions-functions-and-stack-unwinding)
- [27.4 — Uncaught exceptions and catch-all handlers](#274--uncaught-exceptions-and-catch-all-handlers)
- [27.5 — Exceptions, classes, and inheritance](#275--exceptions-classes-and-inheritance)
- [27.6 — Rethrowing exceptions](#276--rethrowing-exceptions)
- [27.7 — Function try blocks](#277--function-try-blocks)
- [27.8 — Exception dangers and downsides](#278--exception-dangers-and-downsides)
- [27.9 — Exception specifications and noexcept](#279--exception-specifications-and-noexcept)
- [27.10 — std::move_if_noexcept](#2710--stdmove_if_noexcept)
- [27.x — Chapter 27 summary and quiz](#27x--chapter-27-summary-and-quiz)
- [CS6340 patterns](#cs6340-patterns)

## 27.1 — The need for exceptions

### Return codes are simple until they spread everywhere

A return code is an ordinary value used to report failure:

```cpp
int findFirst(std::string_view text, char target)
{
    for (std::size_t i { 0 }; i < text.size(); ++i)
    {
        if (text[i] == target)
            return static_cast<int>(i);
    }

    return -1;
}
```

This is easy to write, but it creates questions:

```text
Does -1 mean "not found"?
Can -1 ever be a real value?
Will every caller remember to check?
```

Return codes are not bad. They are often the right answer for ordinary,
expected outcomes. The problem is that they become awkward when the failure is
serious, rare, or hard to handle locally.

### One return slot has to carry too much

Suppose a function naturally returns a useful result:

```cpp
double averageScore(const std::vector<int>& scores);
```

What should it return for an empty vector?

Options:

```text
return 0.0        -> ambiguous; maybe the real average is 0
return sentinel   -> every caller must know the sentinel
out parameter     -> clutters the signature
std::optional     -> good for expected "maybe no value" cases
exception         -> possible for serious contract failure
```

An out-parameter version is noisy:

```cpp
double averageScore(const std::vector<int>& scores, bool& ok)
{
    if (scores.empty())
    {
        ok = false;
        return 0.0;
    }

    ok = true;
    int sum {};
    for (int score : scores)
        sum += score;

    return static_cast<double>(sum) / scores.size();
}
```

Every caller must remember:

```cpp
bool ok {};
double average { averageScore(scores, ok) };

if (!ok)
{
    // handle failure before using average
}
```

### Error checks obscure the main path

Long return-code chains intermix normal work and error plumbing:

```cpp
if (!openInput(path))
    return Error::openFailed;

if (!readHeader())
    return Error::badHeader;

if (!readBody())
    return Error::badBody;

if (!validate())
    return Error::invalidData;
```

The function's core story gets harder to see because every step is surrounded by
"if failed, return."

Exceptions separate these concerns:

```text
normal path:      do the work
exception path:   jump to a handler that knows what to do
```

### Constructors cannot return status codes

Constructors have no return type:

```cpp
class Config
{
public:
    explicit Config(std::string_view path);
};
```

If construction cannot succeed, there is no `return Error::badConfig;` channel.
Possible approaches:

- Build an invalid object and require callers to check `isValid()`
- Use a factory function that returns `std::optional`, `std::expected`, or a
  status/result type
- Throw an exception to say construction failed

Exceptions fit constructors naturally because a thrown exception aborts object
construction.

### What exceptions buy you

Exception handling lets the code that detects an exceptional condition be
different from the code that handles it.

```text
low-level parser:
  detects "missing required field"
  throws ParseError

top-level tool:
  catches ParseError
  prints a diagnostic
  exits with failure status
```

This is useful when the low-level function cannot reasonably decide what the
whole program should do.

### Not every error should be an exception

Good candidates:

- Serious errors that prevent the requested operation from continuing
- Rare failures, not normal loop-control cases
- Failures that cannot be handled where they occur
- Cases where there is no clean return-value channel

Weak candidates:

- Ordinary "not found" results
- User input that is expected to be wrong often
- Fast inner-loop control flow
- Codebases that have explicitly chosen non-exception error handling

For LLVM-related work, check the local convention. LLVM itself often uses
assertions, status/result objects, and explicit error types instead of leaning
on C++ exceptions in core code.

## 27.2 — Basic exception handling

### The three keywords

Exception handling is built around:

| Keyword | Role |
| --- | --- |
| `throw` | raises an exception |
| `try` | marks code whose exceptions should be watched |
| `catch` | handles a matching exception type |

Minimal shape:

```cpp
try
{
    throw 5;
}
catch (int errorCode)
{
    std::cerr << "Caught int error: " << errorCode << '\n';
}
```

### Throwing

A `throw` statement raises an exception object:

```cpp
throw 1;
throw "missing input";
throw std::runtime_error { "missing input" };
```

C++ can throw many kinds of values, but in maintainable code, prefer standard
exception classes or custom exception classes over vague primitive values.

### Try blocks

A `try` block does not handle anything by itself. It marks a region of code:

```cpp
try
{
    loadConfig(path);
    runAnalysis();
}
```

Any exception thrown directly inside that block, or thrown by functions called
inside that block, is eligible for the attached `catch` blocks.

### Catch blocks

A `catch` block handles one kind of exception:

```cpp
catch (const std::runtime_error& error)
{
    std::cerr << error.what() << '\n';
}
```

Catch fundamental types by value:

```cpp
catch (int code)
```

Catch class types by `const` reference:

```cpp
catch (const std::exception& error)
```

That avoids unnecessary copies and avoids slicing derived exception objects.

If the caught value is not used, omit the name:

```cpp
catch (const std::runtime_error&)
{
    std::cerr << "runtime error\n";
}
```

### A complete example

```cpp
#include <iostream>
#include <stdexcept>

double reciprocal(double value)
{
    if (value == 0.0)
        throw std::runtime_error { "division by zero" };

    return 1.0 / value;
}

int main()
{
    try
    {
        std::cout << reciprocal(0.0) << '\n';
    }
    catch (const std::runtime_error& error)
    {
        std::cerr << "Could not compute reciprocal: " << error.what() << '\n';
    }

    std::cout << "program continues after handler\n";
}
```

Flow:

```text
main try block
  calls reciprocal(0)
    reciprocal throws std::runtime_error
  control jumps to matching catch block
catch block runs
execution resumes after all attached catch blocks
```

### Matching is type-based and strict

Catch matching does not do ordinary numeric promotions:

```cpp
try
{
    throw 'x';
}
catch (int)
{
    // not reached just because char can promote to int
}
catch (char)
{
    // reached
}
```

The important exception to remember for later: derived class exceptions can be
caught by handlers for their base class.

### Exceptions are handled immediately

Once `throw` executes, normal execution in that block stops:

```cpp
try
{
    std::cout << "before\n";
    throw std::runtime_error { "stop" };
    std::cout << "after\n"; // skipped
}
catch (const std::runtime_error&)
{
    std::cerr << "handled\n";
}
```

The `throw` is not a delayed status. It is an immediate control-flow transfer.

### What handlers usually do

Common catch-block actions:

- Log or print an error and continue if the program can still proceed
- Translate the failure into a return value at a boundary
- Throw a new exception with a different type or level of abstraction
- Rethrow the same exception after logging
- Catch a fatal error at `main()` and exit cleanly

An empty catch block still handles the exception:

```cpp
catch (const std::exception&)
{
}
```

Use that rarely. Swallowing errors silently usually makes debugging harder.

## 27.3 — Exceptions, functions, and stack unwinding

### Called functions can throw to the caller's try block

The `throw` does not need to be textually inside the `try` block:

```cpp
#include <stdexcept>
#include <string_view>

int parseRequiredPort(std::string_view text)
{
    if (text.empty())
        throw std::runtime_error { "missing port" };

    return 8080; // pretend parsing happened
}

void loadServer(std::string_view portText)
{
    int port { parseRequiredPort(portText) };
    (void)port;
}

int main()
{
    try
    {
        loadServer("");
    }
    catch (const std::runtime_error& error)
    {
        std::cerr << error.what() << '\n';
    }
}
```

The exception is thrown in `parseRequiredPort`, but the handler in `main` can
catch it because the call chain began inside `main`'s `try` block.

### Propagation up the call stack

Call stack before the throw:

```text
main
  loadServer
    parseRequiredPort
```

If `parseRequiredPort` throws and does not handle the exception:

```text
1. Check parseRequiredPort for matching handler.
2. None found.
3. Pop parseRequiredPort from the stack.
4. Check loadServer for matching handler.
5. None found.
6. Pop loadServer from the stack.
7. Check main's try/catch.
8. Matching handler found.
```

This process is stack unwinding.

### Unwinding destroys local objects

When a function is unwound, its local objects are destroyed as usual.

```cpp
#include <iostream>
#include <stdexcept>

struct Trace
{
    const char* name {};
    ~Trace() { std::cerr << "destroy " << name << '\n'; }
};

void inner()
{
    Trace t { "inner local" };
    throw std::runtime_error { "boom" };
}

void outer()
{
    Trace t { "outer local" };
    inner();
}
```

If `outer()` is called inside a matching `try`, both local `Trace` objects are
destroyed during unwinding.

This is why RAII matters. Cleanup belongs in destructors of member/local objects,
not in manual "remember to close this later" code.

### A matching catch must be attached to the right try block

This handler does not catch the exception:

```cpp
try
{
    // empty
}
catch (const std::runtime_error&)
{
}

loadServer(""); // not inside the try block above
```

The catch block only applies to exceptions thrown from the associated `try`
block.

### Once handled, the exception is done

If an exception is caught in a middle function, outer callers do not see it
unless it is rethrown:

```cpp
void middle()
{
    try
    {
        inner();
    }
    catch (const std::runtime_error&)
    {
        std::cerr << "middle handled it\n";
    }
}
```

After `middle` handles the exception, execution continues after the catch block
in `middle`. A caller's catch block will not run for that already-handled
exception.

## 27.4 — Uncaught exceptions and catch-all handlers

### Uncaught exceptions terminate the program

If no matching handler is found anywhere up the stack, the exception is
uncaught. The program calls `std::terminate()`.

Important detail:

```text
For an uncaught exception, the call stack may or may not be unwound.
```

That means local destructors may or may not run before termination. Do not rely
on ordinary cleanup after an exception reaches the uncaught state.

### Catch-all handlers

A catch-all handler catches any exception type:

```cpp
try
{
    runTool();
}
catch (...)
{
    std::cerr << "unhandled non-standard exception\n";
}
```

The `...` is the ellipsis. In this context, it means "any exception type."

Catch-all handlers must come last:

```cpp
try
{
    runTool();
}
catch (const std::runtime_error& error)
{
    std::cerr << error.what() << '\n';
}
catch (...)
{
    std::cerr << "unknown failure\n";
}
```

If `catch (...)` came first, more specific handlers would never get a chance.

### Wrapping `main`

A top-level catch can make program shutdown more controlled:

```cpp
int main()
{
    try
    {
        return runCommandLineTool();
    }
    catch (const std::exception& error)
    {
        std::cerr << "fatal: " << error.what() << '\n';
        return 1;
    }
    catch (...)
    {
        std::cerr << "fatal: unknown exception\n";
        return 1;
    }
}
```

This is often better than letting the runtime print an implementation-specific
uncaught-exception message.

### Do not keep running blindly after catch-all

If a catch-all handler catches something unexpected, assume program state may be
bad. Good catch-all behavior:

```text
log/print minimal diagnostic
release or flush top-level resources if safe
return failure / terminate
```

Bad catch-all behavior:

```text
ignore everything and continue normal execution
```

### Debugging tradeoff

A catch-all in `main` can hide the original unhandled exception from a debugger
because the stack is unwound before the catch-all runs.

In debug builds, some projects disable top-level catch-all handlers so the
debugger can break at the throw site. In release builds, the top-level handler
can produce a cleaner user-facing failure.

## 27.5 — Exceptions, classes, and inheritance

### Member functions and overloaded operators can throw

Operators often cannot return a separate error code because their signatures are
constrained.

```cpp
class Scores
{
private:
    int m_values[3] {};

public:
    int& operator[](int index)
    {
        if (index < 0 || index >= 3)
            throw std::out_of_range { "score index out of range" };

        return m_values[index];
    }
};
```

Without exceptions, `operator[]` would have few clean ways to report failure
while still returning `int&`.

### Constructors can fail by throwing

If a constructor throws:

```text
object construction is aborted
the object's destructor is not called
already-constructed members are destroyed
```

Example:

```cpp
#include <memory>
#include <stdexcept>

class TraceFile
{
private:
    std::unique_ptr<int> m_resource {};

public:
    explicit TraceFile(bool ok)
        : m_resource { std::make_unique<int>(42) }
    {
        if (!ok)
            throw std::runtime_error { "trace file setup failed" };
    }
};
```

If `TraceFile(false)` throws, `TraceFile::~TraceFile()` is not called because
there is no fully constructed `TraceFile` object. But `m_resource` was already
constructed, so `m_resource` is destroyed and frees its resource.

That is the RAII lesson:

```text
Put resource ownership inside members that clean themselves up.
Do not manually allocate in a constructor and hope the class destructor runs.
```

Good resource-owning members:

- `std::unique_ptr`
- `std::vector`
- `std::string`
- file stream objects
- project-specific RAII handles

### Exception classes

Throwing primitive values is vague:

```cpp
throw 2; // what does 2 mean?
```

Exception classes carry type and context:

```cpp
#include <stdexcept>
#include <string>

class ConfigError : public std::runtime_error
{
public:
    explicit ConfigError(const std::string& message)
        : std::runtime_error { message }
    {
    }
};
```

Use:

```cpp
void requireField(bool present)
{
    if (!present)
        throw ConfigError { "missing required field: seed_dir" };
}
```

Catch:

```cpp
try
{
    requireField(false);
}
catch (const ConfigError& error)
{
    std::cerr << "configuration error: " << error.what() << '\n';
}
catch (const std::exception& error)
{
    std::cerr << "other standard exception: " << error.what() << '\n';
}
```

### Catch derived exceptions before base exceptions

Handlers are checked in order.

```cpp
catch (const std::exception& error)
{
    // catches std::runtime_error and ConfigError too
}
catch (const ConfigError& error)
{
    // unreachable if ConfigError derives from std::exception
}
```

Correct order:

```cpp
catch (const ConfigError& error)
{
    // specific
}
catch (const std::exception& error)
{
    // general fallback for standard exception hierarchy
}
```

Rule:

```text
Most-derived handlers first, base handlers later.
```

### `std::exception` and `what()`

The standard library exception hierarchy derives from `std::exception`.

Common examples:

| Type | Header | Typical meaning |
| --- | --- | --- |
| `std::runtime_error` | `<stdexcept>` | generic runtime failure with message |
| `std::out_of_range` | `<stdexcept>` | index/key outside valid range |
| `std::bad_alloc` | `<new>` | allocation failed |
| `std::bad_cast` | `<typeinfo>` | failed reference `dynamic_cast` |

`std::exception` provides:

```cpp
virtual const char* what() const noexcept;
```

The returned message is for humans. Do not write logic that depends on exact
message text across compilers.

When overriding `what()`, keep `noexcept`:

```cpp
class MyError : public std::exception
{
public:
    const char* what() const noexcept override
    {
        return "my error";
    }
};
```

In many cases, deriving from `std::runtime_error` is simpler because it already
stores a message.

### Exception object lifetime

When you throw an object, the implementation stores an exception object outside
the ordinary stack frame so it can survive stack unwinding.

Implications:

- Exception objects should be copyable.
- Catch class exceptions by `const&`.
- Do not store pointers or references to stack locals inside an exception
  object if those locals may be destroyed during unwinding.

Bad shape:

```cpp
class BadError
{
private:
    const std::string& m_message; // might dangle
};
```

Better shape:

```cpp
class GoodError : public std::runtime_error
{
public:
    explicit GoodError(const std::string& message)
        : std::runtime_error { message }
    {
    }
};
```

## 27.6 — Rethrowing exceptions

### Catching does not always mean fully handling

Sometimes a function can add logging or cleanup but cannot decide the final
recovery strategy:

```cpp
void loadAllInputs()
{
    try
    {
        loadConfig();
        loadSeeds();
    }
    catch (const std::exception& error)
    {
        logFailure(error.what());
        throw; // caller still needs to handle failure
    }
}
```

This catches the exception, records context, and rethrows the same exception.

### Throwing a new exception

A catch block can throw a different exception:

```cpp
try
{
    parseLowLevelFormat();
}
catch (const std::runtime_error& error)
{
    throw ConfigError { std::string { "config parse failed: " } + error.what() };
}
```

This translates a low-level failure into a higher-level domain error.

The new exception is not caught by the same catch block. It propagates outward to
the next suitable handler.

### The wrong way to rethrow the same exception

This looks reasonable:

```cpp
catch (const std::exception& error)
{
    throw error; // bad if the original exception was derived
}
```

But `error` has static type `const std::exception&`. Throwing `error` creates a
new exception object from that static type. If the original exception was
`ConfigError`, the derived part can be sliced away.

Same problem with a simpler hierarchy:

```text
throw Derived
catch Base&
throw baseReference

result: throws a Base copy, not the original Derived object
```

### The right way

Use bare `throw;` inside a catch block:

```cpp
catch (const std::exception& error)
{
    logFailure(error.what());
    throw;
}
```

Bare `throw;` rethrows the current exception object exactly. It does not copy it
and does not slice it.

Rule:

```text
When rethrowing the same exception, write throw; by itself.
```

Bare `throw;` only makes sense while handling an active exception. Outside a
catch path, it is invalid behavior.

## 27.7 — Function try blocks

### The problem: constructor initializer lists run before the body

In a constructor:

```cpp
class Derived : public Base
{
public:
    Derived(int x)
        : Base { x } // runs before constructor body
    {
        // normal try block here is too late to catch Base{x}
    }
};
```

If `Base { x }` throws, the `Derived` constructor body has not started yet.
A normal `try` inside the body cannot wrap the base/member initializer list.

### Function try block syntax

```cpp
class Derived : public Base
{
public:
    Derived(int x)
    try
        : Base { x }
        , m_value { x }
    {
        if (x == 0)
            throw std::runtime_error { "bad value" };
    }
    catch (...)
    {
        logFailure("Derived construction failed");
        throw;
    }

private:
    int m_value {};
};
```

The `try` goes before the member initializer list. The `catch` blocks attach to
the whole function.

A constructor function try block can catch:

- Exceptions from base-class construction
- Exceptions from member initialization
- Exceptions from the constructor body

### Constructor function try handlers cannot recover

For constructors, the function-level catch block must throw a new exception or
rethrow the current one. If control reaches the end of the catch block, the
current exception is implicitly rethrown.

This is because the object failed to construct. There is no completed object to
return.

### Do not use function try blocks for member cleanup

When construction fails, the object is considered not alive. Accessing its
members from the constructor's function-level catch block is not a safe cleanup
strategy.

Wrong mental model:

```text
constructor catch block can manually clean up this object's members
```

Better model:

```text
members clean themselves through RAII as construction unwinds
function try block logs or translates the exception
```

### Function try blocks outside constructors

Function try blocks can be used on ordinary functions and destructors, but they
are rare there. Ordinary `try`/`catch` inside the function body is usually
clearer.

Practical rule:

```text
Use function try blocks mainly for constructors that need to log or translate
exceptions from base/member initialization.
```

## 27.8 — Exception dangers and downsides

### Manual cleanup is fragile

This leaks if `processPerson` throws:

```cpp
Person* person { new Person { "Ada" } };
processPerson(person);
delete person;
```

The exception skips directly past `delete`.

A better shape:

```cpp
#include <memory>

auto person { std::make_unique<Person>("Ada") };
processPerson(*person);
```

If `processPerson` throws, `person` is destroyed during stack unwinding and the
owned `Person` is deleted.

### Prefer RAII

RAII means resource ownership is tied to object lifetime:

```text
constructor acquires resource
destructor releases resource
```

Then cleanup works for:

- Normal returns
- Early returns
- Exceptions

RAII examples:

- `std::vector` manages dynamic array storage
- `std::string` manages character storage
- `std::unique_ptr` manages one heap object
- `std::fstream` manages a file handle

For exception-safe C++, prefer RAII objects over raw `new`/`delete` and manual
`open`/`close` pairs.

### Destructors should not throw

Throwing out of a destructor is dangerous. It is especially bad if another
exception is already being handled and stack unwinding is in progress. In that
case, a destructor throwing can cause immediate termination.

Rule:

```text
Destructors should not allow exceptions to escape.
```

If cleanup can fail:

- Log the failure
- Store status somewhere explicit
- Provide a separate `close()` / `commit()` operation that callers can invoke
  before destruction
- Catch and suppress inside the destructor if necessary

### Performance

Exceptions have costs:

- They can increase executable size.
- Some implementations require metadata to support unwinding.
- Throwing and catching is usually expensive because the runtime must find a
  handler and unwind the stack.

Many modern implementations make the non-throwing path cheap, but the thrown
path is still not something to use as ordinary loop control.

### When exceptions fit

Use exceptions when all or most of these are true:

- The failure is infrequent.
- The failure is serious enough that the current operation cannot continue.
- The local function cannot reasonably handle it.
- A return code would make the interface awkward or ambiguous.

Prefer non-exception mechanisms when:

- Failure is expected and frequent.
- The caller naturally branches on success/failure.
- The project convention avoids exceptions.
- The code is on a hot path and the error is part of normal control flow.

CS6340 warning: LLVM-style infrastructure often has strong conventions around
assertions, `Expected`/`Error`-style return objects, and build settings. Follow
the local project style before introducing exceptions.

## 27.9 — Exception specifications and noexcept

### Potentially throwing vs non-throwing

In C++, a function is either:

```text
potentially throwing
non-throwing
```

Most ordinary functions are potentially throwing unless declared otherwise.

`noexcept` says a function promises not to let exceptions escape to its caller:

```cpp
void reset() noexcept;
```

This is a contract. The compiler does not prevent every possible throw inside
the function body.

### What happens if a `noexcept` function lets an exception escape?

If an exception tries to leave a `noexcept` function:

```text
std::terminate() is called
stack unwinding may or may not happen
```

Example:

```cpp
void helper()
{
    throw std::runtime_error { "fail" };
}

void reset() noexcept
{
    helper(); // if helper throws and reset does not catch it, terminate
}
```

`noexcept` should be used only when the guarantee is real.

### `noexcept(true)` and `noexcept(false)`

These are equivalent:

```cpp
void f() noexcept;
void g() noexcept(true);
```

This says the function is potentially throwing:

```cpp
void h() noexcept(false);
```

The Boolean form is mostly useful in templates:

```cpp
template <typename T>
void relocate(T& value) noexcept(noexcept(T { std::move(value) }))
{
    T moved { std::move(value) };
    (void)moved;
}
```

The outer `noexcept(...)` is the function specifier. The inner `noexcept(...)`
is the operator described below.

### The `noexcept` operator

The `noexcept` operator checks, at compile time, whether an expression is known
not to throw:

```cpp
void mayThrow();
void willNotThrow() noexcept;

constexpr bool a { noexcept(mayThrow()) };     // false
constexpr bool b { noexcept(willNotThrow()) }; // true
constexpr bool c { noexcept(1 + 2) };          // true
```

It does not evaluate the expression. It asks about its exception specification.

### Exception safety guarantees

Exception safety is about what remains true if an exception interrupts an
operation.

| Guarantee | Meaning |
| --- | --- |
| No guarantee | Anything may be left messy or unusable |
| Basic guarantee | No leaks; objects remain usable, but state may change |
| Strong guarantee | Operation either succeeds completely or has no effect |
| No-throw / no-fail | No exception escapes, or the operation cannot fail |

Example of strong guarantee thinking:

```text
copy first, then commit the change
if copy fails, original object is untouched
```

Example of weaker behavior:

```text
modify original object, then fail halfway through
object is still valid but its state changed
```

### When to mark functions `noexcept`

Strong candidates:

- Move constructors
- Move assignment operators
- `swap` functions
- Cleanup/deallocation functions that must not throw
- Functions that truly provide a no-throw or no-fail guarantee

Possible candidates:

- Copy constructors and copy assignment operators that really cannot throw
- Destructors, though destructors are often implicitly `noexcept` when their
  members' destructors are `noexcept`

Do not add `noexcept` just because the current implementation happens not to
throw. It becomes part of the interface promise. Removing it later can break
callers or performance assumptions.

### A realistic `swap`

```cpp
#include <string>
#include <utility>

class Record
{
private:
    std::string m_name {};

public:
    void swap(Record& other) noexcept(noexcept(m_name.swap(other.m_name)))
    {
        m_name.swap(other.m_name);
    }
};
```

This says `Record::swap` is non-throwing exactly when `std::string::swap` is
known to be non-throwing.

For study purposes, the key point is not to memorize this expression. The key
point is:

```text
noexcept can be both a promise and a compile-time question.
```

## 27.10 — std::move_if_noexcept

### The move-constructor exception problem

Copying usually leaves the source object unchanged:

```text
copy source -> destination
if copy fails, source is still intact
```

Moving often modifies the source:

```text
move source -> destination
source gives up its resource
```

If a move operation throws after partially moving, the source object may have
already been changed. That can violate the strong exception guarantee.

This matters for containers. When a `std::vector<T>` grows, it may need to
relocate existing elements. It would prefer to move them for speed, but moving
is only clearly safe when the move operation is `noexcept` or copying is
unavailable.

### `std::move` always asks for move semantics

```cpp
T destination { std::move(source) };
```

`std::move` does not move by itself. It casts `source` to an rvalue so a move
constructor or move assignment operator can be selected.

It does not ask whether moving is exception-safe.

### `std::move_if_noexcept`

`std::move_if_noexcept` chooses between move and copy:

```cpp
#include <utility>

T destination { std::move_if_noexcept(source) };
```

Behavior:

```text
if T has a noexcept move constructor:
    return an rvalue -> move

else if T cannot be copied:
    return an rvalue -> move anyway

else:
    return an lvalue reference -> copy
```

The goal is to use move semantics only when doing so preserves the strong
exception guarantee, falling back to copy when copy is safer.

### Why `noexcept` on move constructors matters

```cpp
class Buffer
{
public:
    Buffer(Buffer&& other) noexcept;
    Buffer& operator=(Buffer&& other) noexcept;
};
```

Marking move operations `noexcept` is not just documentation. Standard
containers can use that fact to choose faster relocation paths.

If the move constructor is not `noexcept`, a container may copy elements instead
of moving them so it can preserve strong exception behavior.

### Copy-deleted types

If a type cannot be copied, `std::move_if_noexcept` may move even when the move
constructor is potentially throwing. There is no copy fallback.

That means the strong guarantee may be waived in those cases. This is common in
standard library container internals because move-only types must still be
usable.

### Practical rule

If you write a type that owns resources:

```text
write correct move operations
make them noexcept if they truly cannot throw
```

That makes the type work better with standard containers and generic code.

## 27.x — Chapter 27 summary and quiz

### Summary

- Exceptions decouple error detection from error handling.
- `throw` raises an exception.
- `try` marks a block whose exceptions should be watched.
- `catch` handles matching exception types.
- Exceptions are handled immediately; skipped statements do not execute.
- Catch matching is type-based and does not use ordinary numeric conversions.
- Exceptions can propagate from called functions to caller handlers.
- Stack unwinding destroys local objects in unwound stack frames.
- An uncaught exception calls `std::terminate`, and stack unwinding is not
  guaranteed in that case.
- `catch (...)` catches any exception and must be listed last.
- Constructors can report construction failure by throwing.
- If construction fails, the class destructor is not called, but constructed
  members are destroyed.
- RAII is the main technique that makes exception cleanup reliable.
- Catch class exceptions by `const&`.
- Catch derived exception classes before base exception classes.
- `std::exception` is the standard exception base; `what()` returns human
  diagnostic text.
- Use bare `throw;` to rethrow the current exception without slicing.
- Function try blocks are mainly for constructor initializer-list failures.
- Destructors should not let exceptions escape.
- `noexcept` is a contract that no exception will escape to the caller.
- If an exception escapes a `noexcept` function, `std::terminate` is called.
- The `noexcept` operator asks whether an expression is known not to throw.
- `std::move_if_noexcept` moves only when moving is known safe, otherwise it
  copies when possible.

### Quiz note

The LearnCpp chapter quiz practices throwing standard exceptions from class
construction and catching via the standard exception hierarchy. The prompt text
is intentionally omitted here; use the live LearnCpp page for the exact quiz.

## CS6340 patterns

### Error-handling choice table

| Situation | Often appropriate |
| --- | --- |
| Programmer bug / impossible invariant | `assert`, `static_assert`, or project assertion macro |
| Expected "not found" result | `std::optional`, status enum, iterator/end, or project result type |
| Recoverable parse/input failure | explicit result type or exception, depending on local style |
| Constructor cannot create a valid object | throw, or use a factory returning a result type |
| Cleanup/destructor failure | do not throw from destructor; log or expose explicit close/commit |
| Top-level command failure | catch, print diagnostic, return nonzero status |

### Exception-safe resource habit

Prefer:

```cpp
std::vector<std::string> seeds;
std::unique_ptr<Node> node;
std::ifstream input { path };
```

over:

```cpp
std::string* seeds = new std::string[count];
Node* node = new Node {};
FILE* input = std::fopen(path, "r");
```

The first group cleans itself up during stack unwinding. The second group needs
manual cleanup on every exit path.

### Reading `noexcept` in library code

When you see:

```cpp
Widget(Widget&&) noexcept;
void swap(Widget&) noexcept;
```

translate it as:

```text
This operation promises not to throw.
Generic code may rely on that promise for faster or safer behavior.
If the promise is broken, the program can terminate.
```

### Top-level tool pattern

For a small command-line tool that does use exceptions:

```cpp
int main(int argc, char** argv)
{
    try
    {
        return run(argc, argv);
    }
    catch (const std::exception& error)
    {
        std::cerr << "error: " << error.what() << '\n';
        return 1;
    }
    catch (...)
    {
        std::cerr << "error: unknown exception\n";
        return 1;
    }
}
```

Before using this in LLVM-based lab code, check whether the provided framework
expects exceptions, return codes, or LLVM-specific error types. Matching the
framework matters more than forcing one chapter's tool everywhere.
