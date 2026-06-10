# Chapter 27 — Exceptions

> Source: <https://www.learncpp.com/> (Chapter 27)

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

---

## 27.1 — The need for exceptions

Every program you have written so far has, in some sense, assumed that things go right. You read a file and assume it opens. You divide and assume the denominator is not zero. You look up a key and assume it exists. Real programs do not get that luxury. The interesting question is not *whether* a function can fail — almost any function can — but *how it tells its caller that it failed*, and *who is responsible for doing something about it*.

This chapter is about one particular answer to that question: **exceptions**. Before we learn the mechanism, it is worth understanding the problem it solves, because exceptions are not the only tool and they are not always the right one. You will write better C++ if you reach for them deliberately rather than reflexively.

### Return codes are simple — until they spread everywhere

The oldest and simplest way to report failure is a **return code**: an ordinary value that doubles as a status flag.

```cpp
#include <string_view>
#include <cstddef>

int findFirst(std::string_view text, char target)
{
    for (std::size_t i { 0 }; i < text.size(); ++i)
    {
        if (text[i] == target)
            return static_cast<int>(i);
    }

    return -1; // sentinel: "not found"
}
```

This is easy to write and easy to read, and for an ordinary, expected outcome like "the character isn't in the string," it is perfectly reasonable. But notice the questions it quietly raises. Does `-1` mean "not found," or is it a real index that someone might one day want? Can the sentinel ever collide with a legitimate result? And — the question that haunts every return-code design — will *every* caller remember to check it?

Return codes are not bad. They are often the right answer. The trouble starts when the failure is serious, rare, or impossible to handle right where it happens.

### One return slot has to carry too much

A return value can only be one thing. When a function naturally returns a *useful* result, the return slot is already spoken for, and there is no room left to report failure.

```cpp
#include <vector>

double averageScore(const std::vector<int>& scores);
```

What should this return for an *empty* vector? Returning `0.0` is ambiguous — maybe the real average genuinely is zero. Returning some magic sentinel forces every caller to know the magic value. You could add an **out-parameter** that reports success separately, but that clutters the signature and the calling code:

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

    return static_cast<double>(sum) / static_cast<double>(scores.size());
}
```

Now every caller has to dance:

```cpp
bool ok {};
double average { averageScore(scores, ok) };

if (!ok)
{
    // handle failure before trusting `average`
}
```

For a genuinely *expected* "maybe there's no value" case, `std::optional` (which you met back in Chapter 12) is cleaner than any of these. The point here is narrower: when a single return slot has to carry both the answer *and* the verdict on whether there is an answer, something has to give.

### Error checks obscure the main path

Even when return codes work, they impose a tax on readability. Watch what happens when several fallible steps run in sequence:

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

The *story* of this function — open, read the header, read the body, validate — is buried under "if that failed, bail out." The error plumbing and the real work are braided together, and the more steps you add, the harder the braid is to follow.

Exceptions let you separate the two strands:

```text
normal path:     just do the work, step after step
exception path:  if something breaks, jump to a handler that knows what to do
```

The happy path reads like a straight line, and the failure handling lives off to the side.

### Constructors cannot return status codes at all

There is one situation where return codes are not merely awkward but *impossible*: constructors. A constructor has no return type, so there is no channel for `return Error::badConfig;`.

```cpp
#include <string_view>

class Config
{
public:
    explicit Config(std::string_view path);
};
```

If a `Config` cannot be built — the file is missing, the contents are malformed — what can the constructor do? The historical workarounds are all unpleasant: build a half-broken object and force callers to call `isValid()` before trusting it, or give up on constructors entirely and route creation through a factory function that returns an `optional` or a result type. Exceptions offer a fourth option that fits constructors *naturally*: throwing aborts construction outright, so a half-built object is never handed back.

> **Key insight:** A thrown exception aborts object construction. That makes exceptions the one error-reporting mechanism that works cleanly from inside a constructor, where there is no return value to hijack.

### What exceptions actually buy you

Strip away the syntax and the core idea is this: **exceptions let the code that *detects* a problem be different from the code that *handles* it.**

```text
low-level parser
  detects "required field is missing"
  throws ParseError                    -- it knows what went wrong

top-level tool
  catches ParseError
  prints a clear diagnostic            -- it knows what the program should do
  exits with a failure status
```

That separation matters because the function that notices the problem usually has no idea what the *program* should do about it. A deeply nested parser cannot reasonably decide whether to retry, prompt the user, or abort the whole run — but the top of the program can. Exceptions carry the failure across that gap automatically, through however many layers of function calls lie between the detector and the handler.

### Not every error should be an exception

Because exceptions are powerful, it is tempting to use them for everything. Resist that. They are designed for the *exceptional*, and using them as ordinary control flow makes code both slower and harder to reason about.

Good candidates for an exception:

- A serious error that prevents the requested operation from continuing.
- A rare failure, not a routine loop-control case.
- A failure that cannot sensibly be handled where it occurs.
- A situation where there is simply no clean return-value channel — like a constructor.

Weak candidates, where a return value or `std::optional` usually serves better:

- An ordinary "not found" result.
- User input that is expected to be wrong, often.
- Control flow on a hot inner loop.
- A codebase that has deliberately chosen *not* to use exceptions.

> **Note:** That last point is real and worth respecting. Large C++ projects — LLVM is a notable example — frequently avoid exceptions in core code, leaning instead on assertions and explicit status or result types (`Expected`, `Error`). Before introducing exceptions into someone else's codebase, learn the local convention. Matching the surrounding style matters more than any single chapter's preference.

With the *why* settled, let's learn the mechanism.

## 27.2 — Basic exception handling

Exception handling in C++ rests on three keywords. They are easiest to remember by their roles:

| Keyword | Role |
| --- | --- |
| `throw` | raises an exception — "something went wrong, here is what" |
| `try` | marks a region of code whose exceptions should be watched |
| `catch` | handles an exception of a matching type |

Here is the smallest complete shape that uses all three:

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

Read it as a sentence: *try* to run this block; if it *throws* an `int`, *catch* it here and deal with it. Let's take each keyword in turn.

### `throw` raises an exception

A `throw` statement raises an **exception object** — a value that travels outward looking for someone to handle it.

```cpp
throw 1;
throw "missing input";
throw std::runtime_error { "missing input" };
```

C++ will let you throw almost any kind of value: an `int`, a string literal, a full-blown object. *Letting* you, however, is not the same as recommending it. Throwing a bare `1` tells a handler nothing about what `1` means. In maintainable code, prefer a standard exception class or a custom exception class (both covered in 27.5) that carries a type *and* a human-readable message.

### `try` marks the watched region

A `try` block does not handle anything on its own. It simply draws a boundary around code whose failures you care about:

```cpp
try
{
    loadConfig(path);
    runAnalysis();
}
```

Any exception thrown *directly* inside that block — or thrown by a function *called* from inside it, no matter how deep — becomes eligible for the `catch` blocks attached to this `try`. That reach down the call chain is the whole point, and we will explore it in 27.3.

### `catch` handles one kind of exception

A `catch` block handles a single type of exception. It looks like a function parameter list with one parameter:

```cpp
catch (const std::runtime_error& error)
{
    std::cerr << error.what() << '\n';
}
```

How you declare that parameter matters:

- Catch **fundamental types by value** — they are cheap to copy: `catch (int code)`.
- Catch **class types by `const` reference**: `catch (const std::exception& error)`.

The `const` reference for class types is not a stylistic nicety. It avoids an unnecessary copy of the exception object, and — more importantly — it avoids **slicing**, the silent loss of a derived object's extra parts when it is copied into a base-class variable. (You met slicing with inheritance; it bites especially hard with exceptions, as we will see in 27.5 and 27.6.)

If you do not need the caught value — sometimes the *type* is the whole message — you may omit the name:

```cpp
catch (const std::runtime_error&)
{
    std::cerr << "runtime error\n";
}
```

> **Best practice:** Catch class-type exceptions by `const` reference. It avoids copying the exception object and prevents slicing of derived exception types.

### A complete example

Putting the three keywords together in a realistic shape:

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

Trace the control flow:

```text
main enters its try block
  calls reciprocal(0.0)
    reciprocal sees value == 0.0 and throws std::runtime_error
  the throw abandons reciprocal and jumps to the matching catch in main
catch block runs: prints the diagnostic
execution resumes on the line after the catch
```

Notice the final line: once the handler finishes, the program does *not* die — it carries on after the `catch`. A handled exception is a detour, not a crash.

### Matching is by type, and it is strict

When an exception is thrown, C++ looks for a `catch` whose type *matches*. That matching is by type, and it does **not** apply the ordinary numeric conversions you are used to from function calls:

```cpp
try
{
    throw 'x'; // a char
}
catch (int)
{
    // NOT reached -- char does not promote to int for catch matching
}
catch (char)
{
    // reached
}
```

A `char` would happily promote to `int` if you passed it to a function — but a `catch (int)` will not catch a thrown `char`. The matching rules are deliberately narrow. There is one important relaxation, which we will rely on heavily later: a handler for a **base class** *can* catch an exception of a **derived class**. Hold that thought until 27.5.

> **Warning:** Do not rely on numeric promotions or conversions in `catch` matching. Throw the type you intend to catch. If you `throw 'x'`, only a `catch (char)` (or a catch-all) will see it.

### A `throw` is immediate, not deferred

A return code sits in a variable, patiently waiting for someone to inspect it. A `throw` is the opposite — it transfers control *immediately*. The moment it executes, the rest of the current block is abandoned:

```cpp
try
{
    std::cout << "before\n";
    throw std::runtime_error { "stop" };
    std::cout << "after\n"; // never runs -- skipped by the throw
}
catch (const std::runtime_error&)
{
    std::cerr << "handled\n";
}
```

The `"after"` line is dead the instant the `throw` fires. Think of `throw` as a control-flow jump, not as setting a flag.

### What handlers typically do

A `catch` block is just code, so it can do anything — but in practice a handful of patterns cover almost every real handler:

- **Log or report** the error, then continue if the program can still proceed.
- **Translate** the failure into a return value at a boundary between subsystems.
- **Throw a new exception** of a different type or abstraction level (27.6).
- **Rethrow** the same exception after recording context (27.6).
- **Catch a fatal error at `main()`** and exit cleanly with a status code (27.4).

One pattern to use sparingly: the *empty* handler.

```cpp
catch (const std::exception&)
{
    // silently swallow -- almost always a mistake
}
```

This is legal, and it does fully handle the exception — but swallowing an error without a trace usually makes a future bug harder to diagnose, not easier. If you find yourself writing an empty catch, pause and ask whether you really mean to make the failure invisible.

> **Tip:** An empty `catch` block *does* handle the exception — control simply falls out the bottom and continues. But silence hides problems. At minimum, log something; reserve true silence for cases where you are certain the failure is harmless.

## 27.3 — Exceptions, functions, and stack unwinding

In 27.2 the `throw` and the `try` lived close together. The real power of exceptions only shows once they are *far apart* — when the function that detects a problem is several calls away from the function that handles it. That distance is where exceptions earn their keep, and understanding exactly what happens across it is the heart of this chapter.

### A thrown exception need not be inside the `try` block textually

The exception that a `try` block watches for does not have to be written inside the `try`. It only has to be thrown by something the `try` *reaches* — directly, or through any chain of function calls:

```cpp
#include <iostream>
#include <stdexcept>
#include <string_view>

int parseRequiredPort(std::string_view text)
{
    if (text.empty())
        throw std::runtime_error { "missing port" };

    return 8080; // pretend real parsing happened
}

void loadServer(std::string_view portText)
{
    int port { parseRequiredPort(portText) };
    (void)port; // suppress "unused" for this illustration
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

The `throw` is buried two levels down, inside `parseRequiredPort`. Yet `main`'s handler catches it, because the call that started this whole chain — `loadServer("")` — happened inside `main`'s `try` block. The exception rides back up the calls until it reaches a watcher.

### How an exception travels up: stack unwinding

To see *how* it gets back up, picture the **call stack** at the moment of the throw. Each active function call is a frame stacked on top of its caller:

```text
main                  <- bottom of the stack (has the try/catch)
  loadServer
    parseRequiredPort <- top of the stack (about to throw)
```

When `parseRequiredPort` throws and does not handle the exception itself, C++ walks back down the stack one frame at a time, looking for a matching handler:

```text
1. Look in parseRequiredPort for a matching handler.  None.
2. Pop parseRequiredPort off the stack.
3. Look in loadServer for a matching handler.         None.
4. Pop loadServer off the stack.
5. Look in main's try/catch.                          Match!
6. Run main's catch block.
```

This walk — popping frames while searching for a handler — is called **stack unwinding**. The exception is not "returned" through `loadServer` and `parseRequiredPort` in the ordinary sense; those functions are *abandoned mid-execution*, their frames torn down, until a handler is found.

> **Key insight:** Stack unwinding is the search for a handler. The runtime pops one stack frame at a time, checking each for a `catch` that matches the thrown type, until it finds one or runs out of stack.

### Unwinding destroys local objects — and that is the whole reason RAII works

Here is the part that makes exceptions *safe* rather than reckless. As each frame is popped during unwinding, its local objects are destroyed exactly as if the function had returned normally — their destructors run.

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

If `outer()` runs inside a matching `try`, the throw in `inner()` unwinds *both* frames — and both `Trace` destructors fire, printing `destroy inner local` then `destroy outer local`, before the handler ever runs.

This is not a detail; it is the foundation of exception-safe C++. Because destructors run during unwinding, any cleanup you put in a destructor happens *automatically* on the exception path, with no effort from you. That is **RAII** (Resource Acquisition Is Initialization), which you have been using since `std::vector` and `std::unique_ptr`: a resource owned by a local object is released when that object is destroyed, whether the function returns normally *or* is unwound by an exception.

The lesson, stated as advice: put cleanup in the destructors of member and local objects, never in hand-written "remember to close this at the end" code. Manual cleanup is skipped by a throw; destructor cleanup is not.

> **Key insight:** Stack unwinding runs the destructors of local objects in each abandoned frame. This is *why* RAII makes code exception-safe: cleanup tied to a destructor happens automatically on the exception path, but cleanup written as ordinary statements is jumped over by the `throw`.

### The `catch` must be attached to the right `try`

A `catch` only watches *its own* `try` block. Code that runs outside the `try` — even one line later — is not covered:

```cpp
try
{
    // empty -- nothing here throws
}
catch (const std::runtime_error&)
{
    // this handler guards only the (empty) block above
}

loadServer(""); // NOT inside the try; if this throws, the handler above won't catch it
```

If `loadServer("")` throws here, the exception sails right past the handler above, because that handler is bound to a `try` block that has already finished. Placement matters: a `try` block must actually *enclose* the call you want to protect.

### Once handled, an exception is finished

When some intermediate function catches an exception, that exception is *consumed*. It does not continue up to outer callers unless explicitly rethrown (27.6):

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

Once `middle` handles the exception, control resumes on the line after its `catch`, and `middle` returns normally. A caller of `middle` sees nothing unusual — no exception reaches it, because `middle` already dealt with it. Handling an exception ends its journey.

## 27.4 — Uncaught exceptions and catch-all handlers

We have assumed, so far, that a matching handler always exists somewhere up the stack. What if it does not? And how do you write a handler that catches *anything*, even an exception type you did not anticipate? Those two questions go together, because a program that throws a type nobody catches is exactly the failure a catch-all is meant to prevent.

### An uncaught exception terminates the program

If the unwinding search reaches the bottom of the stack without finding a matching `catch`, the exception is **uncaught**. At that point C++ gives up and calls `std::terminate()`, which by default aborts the program.

There is a subtle and important detail here:

> **Warning:** For an *uncaught* exception, the standard does not guarantee the stack is unwound before `std::terminate()` runs. That means your local destructors may or may not execute. Never rely on ordinary cleanup happening once an exception has reached the uncaught state — by then it is already too late to count on.

The practical takeaway: an uncaught exception is not a graceful shutdown. It is an abrupt one, and any "I'll clean up if something goes wrong" logic that assumed normal unwinding may simply not run.

### The catch-all handler: `catch (...)`

Sometimes you want a handler that catches *every* exception type, including ones you never anticipated. That is the **catch-all** handler, written with an ellipsis:

```cpp
try
{
    runTool();
}
catch (...) // the ellipsis means "any exception type at all"
{
    std::cerr << "unhandled exception\n";
}
```

The `...` here is not a typo and not a parameter pack — in a `catch` it is special syntax meaning "match anything." A catch-all has no parameter, so it cannot inspect *what* was thrown; it only knows that *something* was.

Because it matches everything, a catch-all must come **last** among the handlers on a `try`:

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

Handlers are tried top to bottom, and the first match wins. If `catch (...)` came first, it would swallow *every* exception before the specific handlers below it ever got a look — they would be dead code. Specific first, catch-all last.

> **Best practice:** Order your handlers from most specific to least specific, with `catch (...)` always last. Handlers are checked in source order, and a catch-all placed early makes every later handler unreachable.

### Wrapping `main` for a clean shutdown

A natural and valuable use of the catch-all is at the very top of the program. If you wrap the body of `main` in a `try`, any exception that would otherwise escape your program is caught and turned into a controlled exit:

```cpp
#include <exception>
#include <iostream>

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

This is almost always nicer than letting the runtime's default `std::terminate()` print some terse, implementation-specific message. You get to print a diagnostic *you* control and return a deliberate failure status. Catching `const std::exception&` first lets you extract a real message via `what()` (27.5); the catch-all mops up anything that did not derive from `std::exception`.

### Do not blindly continue after a catch-all

A catch-all is a blunt instrument. When it fires, you genuinely do not know what went wrong — which means you cannot assume your program's state is still sound. Reasonable catch-all behavior is conservative:

```text
log a minimal diagnostic
flush or release top-level resources if it is safe to do so
return a failure status (or terminate)
```

Unreasonable catch-all behavior is to shrug and carry on as if nothing happened:

```text
ignore the exception and resume normal execution  <- dangerous
```

If you caught something you did not expect, assume the worst about your program's invariants. Continuing normally risks compounding a small failure into corrupted data or a confusing crash much later.

### A debugging trade-off worth knowing

There is one cost to wrapping `main` in a catch-all: it can *hide* the original failure from your debugger. Because the stack is unwound on the way to the catch-all, by the time your handler runs, the frames where the exception was actually thrown are gone — so the debugger can no longer show you the throw site.

A common compromise: in **debug** builds, disable the top-level catch-all so the debugger breaks right at the throw, where the evidence is still intact. In **release** builds, keep it, so end users get a clean message instead of a raw termination. You get debuggability where you need it and polish where they need it.

## 27.5 — Exceptions, classes, and inheritance

So far our exceptions have been ad-hoc — a `runtime_error` here, a bare `int` there. This lesson is where exceptions grow up. Three ideas come together: member functions and operators can throw (giving them an escape hatch they otherwise lack), constructors can report failure by throwing (the problem we opened the chapter with), and — most importantly — exceptions are *objects in an inheritance hierarchy*, which is what makes catching them flexible and expressive. This is the lesson the chapter's SafeConfig lab leans on most heavily, so we will go slowly.

### Member functions and overloaded operators can throw

A regular function that fails has a choice: return a status code instead of its result. An **overloaded operator** often does not even have that option, because its signature is fixed by what the operator means.

```cpp
#include <stdexcept>

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

`operator[]` must return an `int&` so that `scores[0] = 90;` works. There is no room in that signature for a separate error code — and you cannot return a reference to "nothing." Throwing is the natural way for such an operator to refuse an invalid request. (This is exactly how `std::vector::at` reports an out-of-range index, while `operator[]` does not check at all — two deliberate designs.)

### Constructors report failure by throwing

Now we can close the loop opened in 27.1. A constructor with no valid object to build can simply throw. When it does, three things happen, in this order:

```text
1. Object construction is aborted.
2. The object's OWN destructor is NOT called (there is no completed object).
3. Any members that were ALREADY fully constructed are destroyed.
```

That third point is the one that turns RAII from a convenience into a guarantee. Consider:

```cpp
#include <memory>
#include <stdexcept>

class TraceFile
{
private:
    std::unique_ptr<int> m_resource {};

public:
    explicit TraceFile(bool ok)
        : m_resource { std::make_unique<int>(42) } // member built FIRST
    {
        if (!ok)
            throw std::runtime_error { "trace file setup failed" };
    }
};
```

If you call `TraceFile{ false }`, the constructor body throws. `TraceFile::~TraceFile()` does *not* run — there is no finished `TraceFile` to destroy. But `m_resource` was fully constructed *before* the body ran, so `m_resource` is destroyed, and its `unique_ptr` destructor frees the heap `int`. No leak, no manual cleanup, no special case.

Now imagine you had instead written `int* m_resource = new int{42};` by hand and planned to `delete` it in the destructor. The destructor never runs, so that raw pointer leaks. The difference is entirely whether the resource lives in a member that cleans *itself* up.

> **Key insight:** If a constructor throws, the object's own destructor never runs — but every fully-constructed member *is* destroyed. So put each resource in a self-cleaning member (`unique_ptr`, `vector`, `string`, a stream). Never `new` a raw resource in a constructor and hope the destructor will free it; a constructor throw makes that hope fail.

Good resource-owning members are the RAII types you already know: `std::unique_ptr`, `std::vector`, `std::string`, file streams, and any project-specific RAII handle.

### Exception classes carry meaning

Throwing a primitive is vague:

```cpp
throw 2; // what is 2? the caller has to guess
```

An **exception class** carries both a *type* (which determines which handler catches it) and *context* (a message a human can read). The easiest way to make one is to derive from a standard exception that already stores a message — `std::runtime_error`:

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

That is the entire class. By forwarding the message to the `std::runtime_error` base, you inherit a working `what()` for free. Now throwing carries intent:

```cpp
void requireField(bool present)
{
    if (!present)
        throw ConfigError { "missing required field: seed_dir" };
}
```

And catching can be as specific or as general as you like:

```cpp
#include <iostream>

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

The SafeConfig lab pushes this one level deeper: it derives `MissingKeyError` *from* `ConfigError`, which derives from `std::runtime_error`, which derives from `std::exception`. A four-level hierarchy means a single thrown `MissingKeyError` can be caught by a handler for *any* of those four types — whichever the catching code finds most convenient. That flexibility is the entire reason to build exceptions out of inheritance.

### Catch derived exceptions *before* base exceptions

Because a base-class handler catches derived exceptions too, handler *order* is not cosmetic — it is correctness. Handlers are tried top to bottom, first match wins. Put a base handler first and it will intercept everything, leaving the specific handler below it unreachable:

```cpp
// WRONG ORDER
catch (const std::exception& error)
{
    // catches std::exception AND everything derived from it,
    // including ConfigError -- so the handler below never runs
}
catch (const ConfigError& error)
{
    // unreachable: ConfigError derives from std::exception
}
```

Flip them so the *most-derived* type is checked first:

```cpp
// CORRECT ORDER
catch (const ConfigError& error)
{
    // the specific case
}
catch (const std::exception& error)
{
    // the general fallback for anything else in the hierarchy
}
```

The rule is the mirror image of the catch-all rule from 27.4, and for the same reason: most-derived handlers first, base handlers later, catch-all last.

> **Best practice:** Order exception handlers most-derived first, base class last. A base-class handler will catch its derived types too, so a base handler placed early makes the specific handlers beneath it dead code.

### `std::exception` and `what()`

Nearly all the exception types the standard library throws derive from a common root, `std::exception`. That is why `catch (const std::exception&)` is such a useful safety net — it catches the whole standard family with one handler. A few you will meet:

| Type | Header | Typical meaning |
| --- | --- | --- |
| `std::runtime_error` | `<stdexcept>` | a generic runtime failure that carries a message |
| `std::out_of_range` | `<stdexcept>` | an index or key outside the valid range |
| `std::invalid_argument` | `<stdexcept>` | an argument with an inappropriate value |
| `std::bad_alloc` | `<new>` | a memory allocation failed |

`std::exception` provides one member you will use constantly:

```cpp
virtual const char* what() const noexcept;
```

It returns a C-string describing the error. Two cautions about it. First, the message is meant for *humans* — a log line, a diagnostic — not for program logic. Different compilers and standard libraries word their built-in messages differently, so never branch on the exact text of `what()`. Second, notice that `what()` is `noexcept` (a promise it will not throw — see 27.9). If you ever override it yourself, you must keep that promise:

```cpp
#include <exception>

class MyError : public std::exception
{
public:
    const char* what() const noexcept override
    {
        return "my error";
    }
};
```

In practice you will rarely override `what()` directly, because deriving from `std::runtime_error` (which already stores and returns your message) is simpler and does the right thing automatically. Reach for raw `std::exception` only when you need a custom hierarchy that does *not* fit the `runtime_error` mold.

### Exception object lifetime: why `const&` and self-contained messages

One subtlety explains several of this chapter's rules at once. When you `throw` an object, the runtime makes a copy and stores it *outside* the normal stack — somewhere that survives unwinding — precisely so the exception can outlive the frames it tears down on its way to a handler.

That single fact has three consequences:

- **Exception objects should be copyable**, since the runtime copies the thrown value.
- **Catch class exceptions by `const&`** (the rule from 27.2), which binds straight to that stored object with no further copy and no slicing.
- **Never store a pointer or reference to a stack local inside an exception object.** Those locals are destroyed during unwinding, so the stored reference would dangle by the time a handler reads it.

The trap, concretely:

```cpp
#include <string>

// BAD: holds a reference that may dangle after unwinding
class BadError
{
private:
    const std::string& m_message;
public:
    explicit BadError(const std::string& message) : m_message { message } {}
};
```

The fix is to *own* the message, not borrow it — which is exactly what deriving from `std::runtime_error` does for you, because it copies the string into itself:

```cpp
#include <stdexcept>
#include <string>

// GOOD: the message is owned by the exception, copied in
class GoodError : public std::runtime_error
{
public:
    explicit GoodError(const std::string& message)
        : std::runtime_error { message }
    {
    }
};
```

> **Warning:** An exception object can outlive the function that threw it, so it must never hold a reference or pointer to a local of that function — the local is gone after unwinding, leaving a dangling reference. Store owned values (copy the data in). Deriving from `std::runtime_error` handles this for you, since it copies the message.

## 27.6 — Rethrowing exceptions

A `catch` block is often not the end of the story. Sometimes a function can do *part* of the job — add a log line, record some context, release a local resource — but cannot decide the final fate of the program. It needs to handle the exception just enough to do its bit, and then pass the problem along. C++ gives you two ways to "pass it along," and one of them has a sharp edge that trips up nearly everyone the first time.

### Catching does not have to mean *fully* handling

Here is the common shape: a mid-level function wants to record that something failed, but the decision about *what to do* belongs to its caller.

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
        logFailure(error.what()); // do our bit: record context
        throw;                    // ...then hand the problem upward
    }
}
```

It catches, logs, and then **rethrows** so the caller still gets to deal with the failure. The bare `throw;` is the key, and we will come back to exactly why it is written that way.

### Option one: throw a *new*, different exception

A handler may decide that the exception it caught is the *wrong abstraction* for its caller, and translate it into a higher-level one:

```cpp
#include <stdexcept>
#include <string>

try
{
    parseLowLevelFormat();
}
catch (const std::runtime_error& error)
{
    throw ConfigError { std::string { "config parse failed: " } + error.what() };
}
```

A low-level "byte 42 was unexpected" becomes a domain-level "config parse failed." This new exception is *not* caught by the same `catch` block that threw it — a handler does not catch its own throws. It begins a fresh trip up the stack to the next suitable handler. This translate-and-rethrow pattern is exactly Task 5 of the SafeConfig lab, where `rethrowOuter` catches a `ConfigError`, layers an `"outer: "` prefix onto its message, and throws a *new* `ConfigError`.

### Option two: rethrow the *same* exception — and the trap

When you want to rethrow the *same* exception rather than a new one, the obvious-looking code is wrong:

```cpp
// LOOKS reasonable, is SUBTLY BROKEN
catch (const std::exception& error)
{
    throw error; // re-throws a sliced COPY, not the original
}
```

Here is the problem. The variable `error` has static type `const std::exception&`. Writing `throw error;` tells the compiler to create a brand-new exception object *from that static type* — a `std::exception`. If the original exception was really a `ConfigError`, everything that made it a `ConfigError` is **sliced away**. A handler upstream that was waiting for `catch (const ConfigError&)` will not match the sliced copy, and your carefully-typed exception has silently degraded into a plain base.

The same trap in miniature:

```text
throw Derived            -- original is a Derived
catch (Base& b)          -- caught through a base reference
throw b;                 -- throws a Base COPY; the Derived part is lost
```

### The fix: bare `throw;`

The correct way to rethrow the current exception unchanged is `throw` with *no operand*:

```cpp
catch (const std::exception& error)
{
    logFailure(error.what());
    throw; // rethrows the ORIGINAL object, exactly, with no copy and no slicing
}
```

A bare `throw;` does not look at the static type of any variable. It re-raises the *actual* exception object the runtime is currently handling — the real `ConfigError`, derived parts intact — and sends it on up the stack untouched. No copy is made, and nothing is sliced.

> **Key insight:** Use bare `throw;` to rethrow the current exception. Writing `throw error;` throws a *copy* based on the variable's static type, slicing away any derived parts. `throw;` re-raises the original object exactly.

One constraint: a bare `throw;` is only meaningful while an exception is actively being handled — inside a `catch` block, or in something the `catch` block calls. If you execute `throw;` when there is no current exception, there is nothing to rethrow, and the program calls `std::terminate()`. Use it only on the exception path.

> **Best practice:** When you mean to rethrow the same exception, write `throw;` by itself. When you mean to raise a *different* exception (to translate or add context), construct and `throw NewType{...}` explicitly. Choosing between them is choosing whether the caller sees the original failure or your reinterpretation of it.

## 27.7 — Function try blocks

There is one corner of a constructor that an ordinary `try` block cannot reach, and this short lesson is about the special syntax that can. You will not need it often — the SafeConfig lab deliberately leaves it out of scope — but you should recognize it when you meet it, and understand precisely what it can and cannot do.

### The problem: initializer lists run before the body

Recall that a constructor builds its base class and its members *before* the constructor body's opening brace. By the time you are inside the body, all that initialization has already happened:

```cpp
class Derived : public Base
{
public:
    Derived(int x)
        : Base { x } // this runs BEFORE the body's {
    {
        // a try block here is already too late to wrap Base { x }
    }
};
```

If `Base { x }` throws, the `Derived` body never starts. A normal `try` written inside the body cannot wrap the initializer list, because the list runs first. So how do you catch an exception thrown while initializing a base or a member?

### The syntax: a `try` before the initializer list

A **function try block** moves the `try` *outside* the body, in front of the initializer list, so its `catch` covers the whole constructor — initializer list included:

```cpp
#include <stdexcept>

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
        throw; // see below -- this is mandatory here in spirit
    }

private:
    int m_value {};
};
```

The `try` sits before the `:`, and the `catch` attaches to the entire function. A constructor function try block can therefore catch exceptions from **base-class construction**, from **member initialization**, and from the **constructor body** — all three.

### A constructor's function try handler cannot recover

Here is the rule that makes function try blocks far less useful than they first appear. For a *constructor*, the handler is not allowed to swallow the exception and pretend construction succeeded. There is no completed object to hand back. If the handler does not throw something itself, the compiler implicitly rethrows the current exception for you when control reaches the end of the handler.

```text
constructor function try handler:
  it may log, it may translate to a different exception,
  but it CANNOT make the failure disappear --
  the current exception is rethrown automatically at the end.
```

So a constructor's function try handler is good for exactly one thing: observing the failure (to log or translate it) on its inevitable way out. It is not a recovery mechanism.

> **Note:** A constructor's function-try-block handler always ends by propagating an exception — either one you throw or the original, rethrown implicitly. It cannot return a successfully-constructed object, because there isn't one. Use it to log or translate construction failures, never to "fix them up."

### Do not use it to clean up members

A tempting but wrong mental model is "the handler can reach into the half-built object and tidy up its members." It cannot, safely. Once construction has failed, the object is considered *not alive*, and poking at its members from the handler is not a sound cleanup strategy.

The correct model is the RAII one you already trust from 27.5: members clean *themselves* up as construction unwinds, and the function try block merely logs or translates the exception as it passes through. Cleanup is automatic; the handler is only an observer.

### Outside constructors, you rarely want one

Function try blocks are legal on ordinary functions and destructors too, but there they buy you nothing an ordinary `try`/`catch` inside the body would not. Plain `try`/`catch` in the body is clearer, so prefer it.

> **Best practice:** Reach for a function try block essentially only when a *constructor* needs to log or translate an exception thrown while initializing a base class or a member — the one case an in-body `try` cannot reach. Everywhere else, use an ordinary `try`/`catch`.

## 27.8 — Exception dangers and downsides

Exceptions are not free, and they are not foolproof. Knowing where they bite — leaks from manual cleanup, throwing destructors, runtime cost — is what separates someone who *uses* exceptions from someone who uses them *safely*. Most of the dangers in this lesson have the same root cause and the same cure, and you have already met the cure: RAII.

### Manual cleanup is fragile, because `throw` skips it

The single most common exception bug is a leak caused by a `throw` jumping over a cleanup statement:

```cpp
Person* person { new Person { "Ada" } };
processPerson(person); // if this throws...
delete person;         // ...we never get here. Leak.
```

When `processPerson` throws, control transfers straight to a handler somewhere up the stack. The `delete person;` line is simply skipped, and the heap `Person` is leaked. This is not a contrived case — *any* raw resource freed by a statement after a fallible call is exposed to exactly this.

The fix is to let an object own the resource, so cleanup rides on a destructor instead of a statement:

```cpp
#include <memory>

auto person { std::make_unique<Person>("Ada") };
processPerson(*person); // if this throws...
// ...person's destructor runs during unwinding, deleting the Person. No leak.
```

Now there is no `delete` to skip. Whether `processPerson` returns normally or throws, `person` is destroyed as its frame unwinds, and the `unique_ptr` frees the `Person`.

### Prefer RAII for everything that needs releasing

The general principle, which by now should feel familiar:

```text
constructor acquires the resource
destructor releases the resource
```

Tie a resource's lifetime to an object, and cleanup becomes correct automatically on *every* exit path — normal return, early return, and exception alike. The standard library hands you RAII types for the common cases: `std::vector` owns array storage, `std::string` owns character storage, `std::unique_ptr` owns one heap object, `std::fstream` owns a file handle. Reach for these instead of raw `new`/`delete` or manual `open`/`close` pairs, and the leak above simply cannot happen.

> **Best practice:** In exception-safe code, never pair a raw `new` with a later `delete`, or an `open` with a later `close`, across code that might throw. Wrap the resource in an RAII type (`unique_ptr`, `vector`, `fstream`, …) so its destructor does the release no matter how the scope is left.

### Destructors must not let exceptions escape

This one is a hard rule, not a preference. Throwing *out of* a destructor is dangerous, and it becomes catastrophic in one specific situation: when the destructor runs *during stack unwinding* because another exception is already in flight. If a second exception tries to escape a destructor while the first is still being handled, C++ cannot reconcile two simultaneous exceptions and calls `std::terminate()` immediately — an instant, unrecoverable crash.

Since destructors run constantly during unwinding (that is the whole mechanism of 27.3), you must assume any destructor *could* be running mid-unwind. Therefore:

> **Warning:** Never let an exception escape a destructor. If a destructor throws while another exception is being handled during unwinding, the program calls `std::terminate()` and dies on the spot. This is why the SafeConfig lab's `Unwinder` destructor only increments a counter — a destructor must be a no-throw zone.

If cleanup in a destructor genuinely *can* fail, you have options that don't involve throwing: log the failure and move on; record a status flag the caller can inspect; or provide a separate `close()` / `commit()` method that callers invoke *before* destruction, where it is allowed to throw. The destructor itself catches and suppresses anything that would otherwise escape.

### Exceptions have a runtime cost

Exceptions are not magic; supporting them costs something:

- They can increase your executable's size, because the compiler emits metadata describing how to unwind each frame.
- The act of throwing and catching is comparatively *expensive* — the runtime has to search for a handler and unwind the stack, which is far slower than a simple return.

Modern implementations are heavily optimized so that the *non-throwing* path — the normal case where nothing goes wrong — is nearly free. But the *throwing* path remains costly. That is the concrete, performance-based reason behind the advice from 27.1: do not use exceptions for ordinary, frequent control flow. An exception on a hot inner loop can dominate your runtime.

### So: when do exceptions fit?

Pulling the whole chapter's judgement together, exceptions are a good fit when *most* of these hold:

- The failure is infrequent.
- The failure is serious enough that the current operation cannot continue.
- The local function cannot reasonably handle it itself.
- A return code would make the interface awkward or ambiguous.

Prefer a non-exception mechanism — a status code, an `optional`, an explicit result type — when:

- Failure is expected and frequent.
- The caller naturally branches on success vs. failure anyway.
- The code is on a hot path and the "error" is really normal control flow.
- The surrounding project convention avoids exceptions.

> **Note:** That last point deserves repeating one final time. Infrastructure projects like LLVM lean on assertions and `Expected`/`Error`-style return objects rather than C++ exceptions in their core. When you work inside such a codebase, follow its conventions first. The right error-handling tool is partly a property of the *project*, not just the situation.

## 27.9 — Exception specifications and noexcept

Up to now, exceptions have been something a function *might* do. This lesson is about a function declaring, as part of its public interface, that it *will not* — and why that promise is more than documentation. The keyword is `noexcept`, and it shows up everywhere in well-written library code. Learning to read it is as important as learning to write it.

### Every function is one of two things

In modern C++, every function is either **potentially throwing** or **non-throwing**. By default, ordinary functions are *potentially throwing* — the compiler assumes any of them might let an exception escape. You opt a function into the non-throwing category with `noexcept`:

```cpp
void reset() noexcept;
```

`noexcept` is a **contract**: this function promises that no exception will escape to its caller. It is a promise *you* make to everyone who calls `reset`. Crucially, the compiler does not verify the promise by inspecting the body — it does not stop you from writing a `throw` inside a `noexcept` function. It takes you at your word, and generic code will rely on that word.

### Breaking the promise calls `std::terminate`

What happens if an exception *does* try to leave a `noexcept` function? The answer is severe by design:

```text
an exception reaches the boundary of a noexcept function
  -> std::terminate() is called
  -> (stack unwinding may or may not have happened first)
```

```cpp
#include <stdexcept>

void helper()
{
    throw std::runtime_error { "fail" };
}

void reset() noexcept
{
    helper(); // if helper throws and reset doesn't catch it -> std::terminate
}
```

This is exactly the mechanism behind Task 3 of the SafeConfig lab: a function marked `noexcept` wraps its risky work in a `try` with a `catch (...)`, so that *nothing* can reach the boundary and trigger termination. The `noexcept` is the promise; the `catch (...)` is what makes the promise true.

> **Warning:** A `noexcept` function that lets an exception escape calls `std::terminate()` — a hard crash. Mark a function `noexcept` only when you can guarantee it. If a `noexcept` function calls anything that might throw, you must catch and contain that exception inside the function (often with `catch (...)`).

### `noexcept(true)`, `noexcept(false)`, and the operator

`noexcept` also has a Boolean form. These two declarations mean the same thing:

```cpp
void f() noexcept;       // non-throwing
void g() noexcept(true); // identical to f
```

And this one explicitly marks a function as *potentially throwing* — the default, made visible:

```cpp
void h() noexcept(false); // may throw
```

The Boolean form earns its keep in *templates*, where whether a function can throw may depend on the type it is instantiated with. To compute that, C++ also gives you the **`noexcept` operator** — a compile-time question: "is this expression known not to throw?"

```cpp
void mayThrow();
void willNotThrow() noexcept;

constexpr bool a { noexcept(mayThrow()) };     // false
constexpr bool b { noexcept(willNotThrow()) }; // true
constexpr bool c { noexcept(1 + 2) };          // true
```

The operator does *not* run the expression — it never calls `mayThrow()`. It only inspects the expression's exception specification at compile time and yields `true` or `false`. The same word, `noexcept`, thus plays two roles: a *specifier* on a function ("I promise not to throw") and an *operator* on an expression ("can this throw?"). You can even feed one into the other to write a function whose promise is conditional on a type's behavior:

```cpp
#include <string>

class Record
{
private:
    std::string m_name {};

public:
    // non-throwing exactly when std::string::swap is non-throwing
    void swap(Record& other) noexcept(noexcept(m_name.swap(other.m_name)))
    {
        m_name.swap(other.m_name);
    }
};
```

Do not memorize that expression. The point to carry away is simply that *`noexcept` is both a promise and a compile-time question*, and the two compose.

> **Key insight:** `noexcept` wears two hats. As a function *specifier* it is a promise ("I won't let an exception escape"). As an *operator* on an expression it is a compile-time test ("is this known not to throw?"). Generic code uses the operator to decide how strongly it can rely on the specifier.

### Exception safety guarantees

Closely related to `noexcept` is a vocabulary for describing *what stays true* if an exception interrupts an operation partway through. These are the standard **exception safety guarantees**, from weakest to strongest:

| Guarantee | What it promises |
| --- | --- |
| No guarantee | Anything goes — the object may be left broken or leaking |
| Basic guarantee | No leaks, and objects remain *usable*, but their state may have changed |
| Strong guarantee | All-or-nothing: the operation either fully succeeds or has *no effect* |
| No-throw / no-fail | No exception escapes at all, or the operation simply cannot fail |

The strong guarantee is the one worth picturing, because it shapes real designs. The classic recipe is **do the risky work on a copy, then commit**:

```text
strong guarantee:  copy first, then swap the copy in
                   if the copy step throws, the original is untouched
weaker behavior:   modify the original in place, then fail halfway
                   the object is still valid, but its state has changed
```

This "copy then commit" idea is exactly why the standard library cares so much about whether *moves* can throw — which is the entire subject of the next lesson.

### When to mark a function `noexcept`

A `noexcept` becomes part of your interface, so add it only where the guarantee is real and worth promising. Strong candidates — places where `noexcept` genuinely matters to callers and to the standard library:

- **Move constructors and move assignment operators.** (This unlocks container optimizations — see 27.10.)
- **`swap` functions.**
- **Cleanup and deallocation functions** that must not fail.
- Any function that truly offers a no-throw or no-fail guarantee.

Possible, but only if the guarantee is genuine:

- Copy constructors and copy assignment that really cannot throw.
- Destructors — though these are *already* implicitly `noexcept` in most cases, which is the standard's way of enforcing the "destructors must not throw" rule from 27.8.

> **Best practice:** Do not add `noexcept` just because the current implementation happens not to throw today. It becomes a promise callers and generic code depend on; removing it later can break them or silently disable optimizations. Add `noexcept` only where non-throwing is a guarantee you intend to *keep*.

## 27.10 — std::move_if_noexcept

This final lesson connects two threads you have been holding: move semantics (Chapter 22) and `noexcept` (just now). It answers a question that sounds academic but has very practical consequences for how fast your types are inside standard containers: *when is it safe to move an object instead of copying it?* The standard library's answer is a small utility, `std::move_if_noexcept`, and understanding it is the best possible reason to mark your move operations `noexcept`.

### Why moving can be riskier than copying

Start with the asymmetry. A **copy** leaves the source untouched — if it fails partway through, the original is still intact:

```text
copy source -> destination
if the copy throws, source is unchanged
```

A **move** typically *guts* the source — it hands the source's resources over to the destination, leaving the source empty:

```text
move source -> destination
source gives up its resources
```

Now suppose a move throws *after* it has already started transferring. The source may have been partly emptied, and the destination is not finished. You are stuck with two half-objects and no way back. The original is gone, the new one isn't built — the **strong guarantee** from 27.9 is shattered.

### Why a `vector` cares

This is not hypothetical; it is exactly the bind a `std::vector` is in when it grows. When a vector outgrows its storage, it allocates a bigger block and relocates every existing element into it. For speed it would love to *move* each element. But if moving an element could throw, and it threw halfway through relocating, the vector would be left with some elements moved, some not, and a half-built new buffer — a disaster it cannot undo.

So the vector faces a choice for each relocation: move (fast, but only safe if the move can't throw) or copy (slower, but the originals survive a failure so it can recover). It needs a way to pick the safe-yet-fast option automatically.

### `std::move` asks for a move unconditionally

Recall what `std::move` actually does: nothing at runtime except a cast. It casts its argument to an rvalue so that an overload resolution will *select* a move constructor or move assignment — if one exists.

```cpp
T destination { std::move(source) }; // requests move semantics
```

What `std::move` does *not* do is ask whether moving is safe. It will happily request a move even from a type whose move operation can throw. For the vector's relocation problem, that is too blunt.

### `std::move_if_noexcept` chooses for you

`std::move_if_noexcept` is the refinement. It looks at the type and decides whether to request a move or fall back to a copy:

```cpp
#include <utility>

T destination { std::move_if_noexcept(source) };
```

Its decision rule:

```text
if T has a noexcept move constructor:
    return an rvalue  -> a move is selected  (fast AND safe)

else if T cannot be copied at all:
    return an rvalue  -> move anyway  (no copy fallback exists)

else:
    return an lvalue  -> a copy is selected  (preserve the strong guarantee)
```

In plain terms: it moves when moving is *provably safe* (the move is `noexcept`), and otherwise prefers to copy so a failure can't corrupt the source. The only exception is a move-only type, where there is no copy to fall back to — then it moves regardless and accepts the weaker guarantee, because there is no alternative.

### This is why `noexcept` on your moves matters

Now the payoff. Marking your move operations `noexcept` is not idle decoration — it is the signal that flips `std::move_if_noexcept` (and therefore `std::vector` and friends) from the slow copy path onto the fast move path:

```cpp
class Buffer
{
public:
    Buffer(Buffer&& other) noexcept;            // <- noexcept unlocks fast relocation
    Buffer& operator=(Buffer&& other) noexcept; // <- same
};
```

If you *omit* `noexcept` from a move constructor that genuinely can't throw, the standard library has no way to know that, so it conservatively **copies** your objects during reallocation instead of moving them — quietly costing you performance for no reason. The `noexcept` is how you tell it the truth.

> **Key insight:** `std::move_if_noexcept` moves only when the move is `noexcept` (or when copying is impossible); otherwise it copies to preserve the strong guarantee. That is why marking a genuinely non-throwing move constructor `noexcept` is a real optimization — it lets containers relocate your objects by moving rather than copying.

### The rule to take with you

If you write a type that owns resources, the chapter's final piece of advice is short:

```text
write correct move operations
mark them noexcept when they truly cannot throw
```

Do those two things and your type slots cleanly into `std::vector` and every other standard container, getting the fast relocation path automatically. This is the quiet, everyday reason the C++ community cares about `noexcept` moves — and it is a fitting place to end a chapter about handling failure, because it is really about *not* failing in the first place.

> **Best practice:** Give resource-owning types correct move operations and mark them `noexcept` whenever the move cannot throw. Standard containers detect that promise via `std::move_if_noexcept` and relocate your objects by moving instead of copying — faster, with no loss of safety.

## 27.x — Chapter 27 summary and quiz

You have learned the full shape of C++ exceptions: what they are for, how they travel, and how to make code that uses them safe. The throughline of the chapter is a single idea — **exceptions decouple the detection of an error from its handling** — and almost everything else follows from making that decoupling reliable.

### The mechanism

- `throw` raises an exception object; `try` marks a watched region; `catch` handles a matching type.
- A `throw` is immediate control flow, not a deferred flag — statements after it are skipped.
- Catch matching is **by type and strict**: ordinary numeric conversions do not apply, but a base-class handler *does* catch derived exceptions.
- Exceptions propagate from called functions up to a caller's handler via **stack unwinding**, which pops frames while searching for a handler.
- Unwinding runs the **destructors of local objects** in each abandoned frame — this is exactly why RAII makes cleanup exception-safe.
- An **uncaught** exception calls `std::terminate`, and unwinding is *not* guaranteed in that case.
- `catch (...)` catches any exception and must be listed **last**.

### Exceptions, classes, and rethrowing

- Operators and constructors can report failure by **throwing** — the natural option when there is no usable return channel.
- If a constructor throws, the object's own destructor does *not* run, but every fully-constructed **member is destroyed** — so own resources in RAII members.
- Catch class exceptions **by `const&`** to avoid copies and slicing.
- Derive custom exceptions from the standard hierarchy (`std::runtime_error` is easiest); `std::exception::what()` returns human-readable text — never branch on its exact wording.
- Order handlers **most-derived first, base last** (and catch-all last of all).
- Use **bare `throw;`** to rethrow the current exception without slicing; construct and `throw NewType{...}` to translate or add context.

### Robustness and `noexcept`

- **Function try blocks** exist mainly to log or translate exceptions from a constructor's base/member initialization; their handlers cannot recover the object.
- **Destructors must not let exceptions escape** — a throwing destructor during unwinding calls `std::terminate`.
- Prefer RAII over manual `new`/`delete` and `open`/`close`; a `throw` skips manual cleanup but never skips a destructor.
- `noexcept` is a **contract** that no exception escapes; breaking it calls `std::terminate`. The `noexcept` **operator** is a compile-time test of whether an expression can throw.
- `std::move_if_noexcept` moves only when the move is known safe, otherwise copying when possible — which is why marking genuinely non-throwing **moves `noexcept`** lets containers relocate by moving.

### Putting it to work: the SafeConfig lab

The chapter's exercise, the **SafeConfig parser**, exercises this material end to end on a deliberately small surface. You build a custom exception hierarchy (`MissingKeyError : ConfigError : std::runtime_error`) and throw the *right type for each failure mode* — a missing key, a malformed value, an out-of-range integer are three structurally different failures, so they become three different exception types rather than three magic error codes. You will:

- throw a specific derived type and let it propagate (`getOrThrow`, `parsePort`);
- guard against the **vacuous-loop trap** — an empty string sails through a per-character digit check untouched, so you must reject `""` *before* it reaches `std::stoi("")`, which would leak a `std::invalid_argument` that is not part of your contract;
- build a **`noexcept` boundary** with `catch (...)` so failures are absorbed into a plain `bool` and nothing escapes to crash the program;
- make **stack unwinding physically observable** with an `Unwinder` probe whose destructor increments a counter *during* unwinding — before the catch block runs — proving with your own eyes that locals are destroyed as the exception travels;
- **catch, augment, and re-raise** with a new exception that layers `"outer: "` onto the original message.

Two idioms the lab insists on, both straight from this chapter: catch class types **by `const` reference**, and make any `noexcept` a *real* promise by backing it with a `catch (...)`. A `noexcept` you cannot keep is not a smaller bug than a leak — it is a guaranteed `std::terminate`.

> **Best practice:** When you reach for exceptions in your own code, let the chapter's central idea guide you: throw the most *specific* type that describes the failure, catch it by `const&` at the level that actually knows what to do, and lean on RAII so that whatever path the exception takes, your resources clean themselves up.

### Quiz note

The LearnCpp chapter quiz practices throwing standard exceptions from class construction and catching them through the standard exception hierarchy — the same muscles the SafeConfig lab builds. Work the lab to green, then use the live LearnCpp page for the exact quiz prompts.
