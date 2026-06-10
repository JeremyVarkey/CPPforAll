# Chapter 3 — Debugging C++ Programs

> Source: <https://www.learncpp.com/> (Chapter 3)
>
> Goal for CS6340: build a practical debugging mindset before Lab 1. You need
> to be able to separate syntax errors, compile-time semantic errors, runtime
> crashes, and logical bugs; then use prints, assertions, breakpoints, watches,
> and the call stack to locate the first point where reality diverges from your
> expectation.

## Contents

- [3.1 — Syntax and semantic errors](#31--syntax-and-semantic-errors)
- [3.2 — The debugging process](#32--the-debugging-process)
- [3.3 — A strategy for debugging](#33--a-strategy-for-debugging)
- [3.4 — Basic debugging tactics](#34--basic-debugging-tactics)
- [3.5 — More debugging tactics](#35--more-debugging-tactics)
- [3.6 — Using an integrated debugger: Stepping](#36--using-an-integrated-debugger-stepping)
- [3.7 — Using an integrated debugger: Running and breakpoints](#37--using-an-integrated-debugger-running-and-breakpoints)
- [3.8 — Using an integrated debugger: Watching variables](#38--using-an-integrated-debugger-watching-variables)
- [3.9 — Using an integrated debugger: The call stack](#39--using-an-integrated-debugger-the-call-stack)
- [3.10 — Finding issues before they become problems](#310--finding-issues-before-they-become-problems)
- [3.x — Chapter 3 summary](#3x--chapter-3-summary)

## 3.1 — Syntax and semantic errors

Programs can be wrong in different ways. The first debugging skill is naming the
kind of wrongness precisely.

### Syntax errors

A syntax error violates the grammar of C++.

The compiler cannot parse the program.

```cpp
#include <iostream>

int main()
{
    std::cout << "hello\n" // missing semicolon
    return 0;
}
```

The compiler sees something like:

```text
std::cout << "hello\n" return 0;
```

and cannot make a valid statement out of it.

Common syntax errors:

- missing semicolon
- missing closing brace
- missing closing parenthesis
- misspelled keyword
- invalid punctuation
- malformed string literal

Example:

```cpp
int main(
{
    return 0;
}
```

The `(` should be `()`. The grammar is broken.

### Compile-time semantic errors

A semantic error violates the meaning rules of C++, even if the syntax is
parseable.

```cpp
int main()
{
    int x { "hello" }; // cannot initialize int from string literal
    return 0;
}
```

The grammar is valid:

```text
type name { initializer };
```

But the meaning is invalid because `"hello"` is not an integer.

Other compile-time semantic errors:

```cpp
int x {};
x(); // x is not a function
```

```cpp
const int limit { 10 };
limit = 20; // cannot assign to const object
```

```cpp
int add(int a, int b)
{
    return a + b;
}

int main()
{
    add(1); // wrong number of arguments
}
```

### Logical errors

A logical error compiles and runs, but produces the wrong result.

```cpp
int area(int width, int height)
{
    return width + height; // compiles, but area should multiply
}
```

The compiler cannot know your intended formula. It only knows the expression is
valid C++.

Logical errors are usually the hardest because they require reasoning about
intent.

### Runtime errors

A runtime error happens while the program is executing.

```cpp
int divide(int a, int b)
{
    return a / b;
}

int main()
{
    return divide(10, 0); // division by zero at runtime
}
```

The code may compile, but a particular input triggers undefined behavior or a
crash.

Common runtime failures:

- divide by zero
- out-of-bounds access
- dereferencing a null pointer
- using a dangling pointer/reference
- stack overflow from runaway recursion
- file open failure that the program did not handle

### Error taxonomy

```text
program text
    |
    v
compiler parses grammar
    |
    +-- syntax error: not valid C++ structure
    |
    v
compiler checks meaning
    |
    +-- compile-time semantic error: valid shape, invalid meaning
    |
    v
program runs
    |
    +-- runtime error: crash/undefined behavior for actual execution
    |
    +-- logical error: program completes but answer is wrong
```

For CS6340, this distinction maps directly to testing:

- compilation catches syntax and many static semantic errors
- tests catch some runtime and logical errors
- sanitizers catch some runtime memory/undefined behavior bugs
- fuzzers search for inputs that trigger runtime failures or unusual behavior

## 3.2 — The debugging process

Debugging is the process of finding and fixing defects.

The key mental move is:

```text
symptom != root cause
```

A symptom is what you observe:

- program crashes
- output is wrong
- test fails
- variable has surprising value
- file is empty

The root cause is the earliest incorrect thing that explains the symptom.

### The debugging loop

```text
1. Reproduce the problem
2. Characterize what is wrong
3. Localize where behavior first diverges
4. Identify the root cause
5. Make the smallest correct fix
6. Re-test the original failure
7. Add or update tests if useful
```

Do not skip reproduction.

If you cannot reproduce the bug, you cannot reliably tell whether your fix
worked.

### Reproduce

Good reproduction:

```text
Run:
  ./fuzzer seed.txt 100

Observed:
  crashes after 17 iterations

Expected:
  either complete 100 iterations or report handled target crash
```

Bad reproduction:

```text
Sometimes it breaks.
```

Useful reproduction details:

- exact command
- exact input
- current working directory
- relevant environment variables
- build mode
- expected output
- actual output
- whether behavior is deterministic

### Characterize

Before changing code, state the bug in a falsifiable way.

Example:

```text
Expected: countBranches("a && b") returns 2.
Actual:   countBranches("a && b") returns 1.
```

This is much better than:

```text
Branch counting is broken.
```

The precise version tells you what to inspect and how to know the fix worked.

### Localize

Localizing means narrowing the bug to a smaller region.

Use questions like:

- Is input parsed correctly?
- Is the loop executing the expected number of times?
- Does the value become wrong before or after this function call?
- Is the object valid before this pointer is dereferenced?
- Did a file open successfully?

ASCII view:

```text
input
  |
  v
parse
  |
  v
mutate
  |
  v
run target
  |
  v
collect result
  |
  v
write report

Find the first arrow where expected state turns into bad state.
```

### Fix

The first fix should be the smallest change that addresses the root cause.

Avoid "debugging by demolition":

```text
Bug in one branch condition
    |
    v
rewrite entire module
```

That introduces new unknowns.

Prefer:

```text
Bug in one branch condition
    |
    v
correct condition
    |
    v
add test for that boundary
```

### Retest

Always retest the original failing case.

Also test nearby cases when possible:

- failing input
- smallest input
- empty input
- boundary input
- normal representative input
- one case on each branch

For CS6340, this is the bridge from debugging to systematic testing.

## 3.3 — A strategy for debugging

A good debugging strategy compares expected state to actual state.

The core question:

```text
At what exact point does the program first do something different
from what I thought it would do?
```

### Start with an expectation

Before stepping or printing, write the expectation.

```cpp
int doubleValue(int x)
{
    return x * x; // bug
}
```

For input `4`:

```text
expected return: 8
actual return:   16
```

Now inspect the function body:

```text
expected operation: x + x or x * 2
actual operation:   x * x
```

The bug is obvious once expectation and actual behavior are side by side.

### Trace from known-good to known-bad

If a program has many stages, identify a known-good checkpoint and a known-bad
checkpoint.

```text
input string is correct      known good
parsed tokens are correct    known good
mutation list is wrong       known bad
```

The root cause is between token parsing and mutation-list construction.

```text
input -> parse -> tokens -> build mutations -> run
                  good          bad
```

Then narrow further inside `build mutations`.

### Binary search the execution path

For long code paths, check the middle.

```text
start --------------------------- end
 good                            bad
                 |
                 v
             check middle
```

If middle is good, bug is after the middle.

If middle is bad, bug is before the middle.

Repeat until the bad region is small.

### Simplify the failing case

A smaller failing input is easier to reason about.

Instead of:

```text
program fails on a 10,000-character seed
```

try to reduce it:

```text
program fails on "%%"
```

This matters in fuzzing too: a minimized crashing input is more useful than a
giant one.

### Avoid guessing edits

Guessing edit loop:

```text
change something
compile
run
still broken
change something else
compile
run
new bug appears
```

Disciplined loop:

```text
observe
hypothesize
test hypothesis
make targeted change
verify
```

Debugging is investigation, not random mutation of your own source code. Let the
evidence tell you where to work.

## 3.4 — Basic debugging tactics

The simplest debugging tools are still useful:

- comments
- print statements
- temporary checks
- narrowing code regions
- comparing expected and actual values

### Commenting out code

Temporarily disabling code can isolate a problem.

```cpp
runParser(input);
// runMutation(input);
// runTarget(input);
```

If the crash disappears when `runMutation` and `runTarget` are disabled, the
bug is likely not in `runParser`.

Use this sparingly:

- do not leave random commented-out code in final work
- comment out one coherent region at a time
- prefer version control over keeping old code as comments

### Print debugging

Print statements expose runtime state.

```cpp
std::cerr << "seed.size() = " << seed.size() << '\n';
std::cerr << "index = " << index << '\n';
```

Use `std::cerr` for diagnostics so debug output is separated from normal program
output.

Good debug print:

```cpp
std::cerr << "[mutate] index=" << index
          << " size=" << seed.size()
          << " seed='" << seed << "'\n";
```

Bad debug print:

```cpp
std::cout << "here\n";
```

"Here" is better than nothing, but it often lacks enough context.

### Put prints around the suspected transition

```cpp
std::cerr << "before mutate: " << seed << '\n';
mutate(seed);
std::cerr << "after mutate:  " << seed << '\n';
```

This answers:

```text
Did mutate change the value the way I expected?
```

### Temporary checks

Add explicit checks where assumptions matter.

```cpp
if (index >= seed.size())
{
    std::cerr << "bad index: " << index
              << " size: " << seed.size() << '\n';
    return;
}
```

This can turn a mysterious crash into a precise report.

### Keep debug code easy to remove

Temporary diagnostics should be obvious and localized.

```cpp
// DEBUG
std::cerr << "value=" << value << '\n';
```

Before committing production/course-submission code, remove noisy temporary
prints unless they are intentionally part of error reporting.

## 3.5 — More debugging tactics

As programs grow, raw `std::cerr` statements can become noisy. More structured
debugging tactics help.

### Conditional debug output

Use a flag:

```cpp
bool debug { true };

if (debug)
{
    std::cerr << "round=" << round << " seed=" << seed << '\n';
}
```

This allows turning diagnostics on/off without deleting them.

### Preprocessor-controlled debug output

```cpp
#ifdef DEBUG_MUTATION
std::cerr << "mutating seed: " << seed << '\n';
#endif
```

Compile with a macro definition when you want the diagnostic:

```text
g++ -DDEBUG_MUTATION main.cpp
```

Mental model:

```text
DEBUG_MUTATION defined
    -> debug statement exists in compiled program

DEBUG_MUTATION not defined
    -> preprocessor removes that block before compilation
```

Use this carefully. Preprocessor conditionals can make code harder to read if
they spread everywhere.

### A small logging helper

Instead of repeating formatting:

```cpp
void debugLog(std::string_view tag, std::string_view message)
{
    std::cerr << '[' << tag << "] " << message << '\n';
}
```

Usage:

```cpp
debugLog("parser", "starting parse");
debugLog("mutator", "generated replacement");
```

A real logging library can add levels, timestamps, files, and sinks. For course
work, a tiny helper is often enough.

### Assertions as executable assumptions

Assertions are covered more later, but the debugging idea belongs here:

```cpp
#include <cassert>

void mutate(std::string& seed, std::size_t index)
{
    assert(index < seed.size());
    seed[index] = 'x';
}
```

An assertion says:

```text
If this condition is false, my program's assumptions are broken.
Stop immediately.
```

Assertions are for programmer errors and internal invariants, not friendly user
input handling.

### Use tools when print debugging gets expensive

Prints are fine, but debuggers let you:

- stop at a line without changing source
- inspect variables without adding output
- step one statement at a time
- inspect the call stack
- set conditional breakpoints

For subtle bugs, the debugger is often faster than scattering prints.

## 3.6 — Using an integrated debugger: Stepping

A debugger lets you execute a program under observation.

The main stepping commands are:

| Command | Meaning |
| --- | --- |
| Step over | execute the current line, but do not enter called functions |
| Step into | enter the called function |
| Step out | finish the current function and return to caller |
| Continue/run | keep running until breakpoint/program end |
| Run to cursor | continue until a selected line |

Names vary by IDE, but the concepts are stable.

### Current execution line

When stopped in a debugger, one line is the current line: the next statement to
execute.

```text
int main()
{
    int x { 3 };
=>  int y { doubleValue(x) };
    std::cout << y << '\n';
}
```

The arrow means:

```text
This statement has not executed yet.
```

### Step over

```cpp
int y { doubleValue(x) };
```

Step over runs the whole call to `doubleValue` and stops at the next line in the
current function.

Use step over when:

- you trust the called function
- the function is from the standard library
- you care about the result but not the internals

### Step into

Step into enters the called function.

```cpp
int doubleValue(int value)
{
=>  return value * 2;
}
```

Use step into when:

- you suspect the called function is wrong
- you need to inspect parameter values
- you want to see exactly which overload/function is called

### Step out

Step out finishes the current function and returns to the caller.

Use step out when:

- you stepped into a function accidentally
- you have seen enough inside the current function
- you want to return to the higher-level flow

### Stepping mental model

```text
main()
  |
  +-- step into add()
        |
        +-- inspect add internals
        |
        +-- step out
  |
  +-- back in main()
```

For CS6340 Lab 1, stepping through a small input can make LLVM/fuzzer control
flow less mysterious, especially when several helper functions call each other.

## 3.7 — Using an integrated debugger: Running and breakpoints

A breakpoint tells the debugger:

```text
Run normally until execution reaches this line, then pause.
```

Breakpoints are better than manually stepping from the beginning every time.

### Basic breakpoint

Suppose the suspicious function is:

```cpp
std::string mutate(std::string seed)
{
    if (!seed.empty())
        seed[0] = 'x';

    return seed;
}
```

Set a breakpoint at the first line of `mutate`.

Then run the program.

```text
program starts
  |
  v
runs normally
  |
  v
mutate called
  |
  v
debugger pauses
```

Now inspect `seed`.

### Continue

After stopping at a breakpoint, continue resumes execution until:

- another breakpoint
- program exit
- crash
- manual pause

### Conditional breakpoint

A conditional breakpoint stops only when a condition is true.

Example condition:

```text
seed.size() == 0
```

This is useful when a function runs thousands of times but only fails for one
case.

Conceptually:

```text
breakpoint reached
  |
  v
condition true?
  |       |
 yes     no
  |       |
pause   keep running
```

### Enable, disable, remove

You can usually:

- disable a breakpoint without deleting it
- re-enable it later
- delete it entirely

This matters when debugging multiple regions and trying not to stop at old
points constantly.

### Breakpoints vs debug prints

| Tool | Best for |
| --- | --- |
| Print | persistent trace, cheap text logs |
| Breakpoint | interactive inspection at a precise line |
| Conditional breakpoint | rare case inside many iterations |
| Watch | tracking a variable/expression while stepping |

The tools complement each other.

## 3.8 — Using an integrated debugger: Watching variables

Watching means observing variable values while the program is stopped or while
stepping.

Common debugger views:

- locals: variables in the current function
- watches: expressions you choose
- hover inspection: value under cursor
- memory view: raw memory, useful later

### Locals

For:

```cpp
int add(int a, int b)
{
    int result { a + b };
    return result;
}
```

When stopped inside `add`, locals may show:

```text
a       2
b       3
result  5
```

If `result` is `-858993460` or some other suspicious garbage-looking value,
that may indicate it has not been initialized yet or you are stopped before its
initialization line executed.

### Watches

A watch can be a variable:

```text
seed
```

or an expression:

```text
seed.size()
index < seed.size()
results.size()
```

Watch expressions are especially useful for invariants:

```text
index < seed.size()
```

If this flips from `true` to `false`, you have found the moment your assumption
breaks.

### Values change after execution, not before

When stopped on:

```cpp
int result { a + b };
```

`result` may not have the new value yet because the line has not executed.

After stepping over that line, it should.

This off-by-one mental model matters:

```text
debug arrow points at next statement to execute
```

### Optimized builds can confuse debugging

In optimized builds, the compiler may:

- remove variables
- inline functions
- reorder instructions
- keep values in registers
- merge equivalent code paths

That can make debugger state surprising.

Use debug builds for source-level debugging unless you are specifically chasing
an optimization-only bug.

## 3.9 — Using an integrated debugger: The call stack

The call stack shows the chain of active function calls.

Example program:

```cpp
void c()
{
    int* p {};
    *p = 1; // crash
}

void b()
{
    c();
}

void a()
{
    b();
}

int main()
{
    a();
}
```

At the crash, the call stack is:

```text
c()
b()
a()
main()
```

Usually displayed with the current function on top:

```text
#0 c()
#1 b()
#2 a()
#3 main()
```

ASCII:

```text
main calls a
  a calls b
    b calls c
      c crashes

stack top
  c
  b
  a
  main
stack bottom
```

### Stack frames

Each active function call has a stack frame containing its local context:

- function parameters
- local variables
- return address
- bookkeeping

When you select a stack frame in the debugger, you can inspect variables from
that function call.

### Why the call stack matters

The crash line is not always the root cause.

Example:

```cpp
void useIndex(const std::string& s, std::size_t index)
{
    std::cout << s[index] << '\n';
}
```

If `index` is out of range, `useIndex` may crash or behave badly. But the root
cause may be the caller that computed the bad index.

The call stack lets you move upward:

```text
useIndex()   symptom appears
mutate()     passed bad index
main()       selected bad configuration
```

### Recursion and stack overflow

The call stack also reveals runaway recursion:

```text
factorial()
factorial()
factorial()
factorial()
...
```

If the same function appears many times, check whether the recursive function
has a reachable base case.

## 3.10 — Finding issues before they become problems

The best bugs are the ones you prevent or catch early.

### Compile often

Do not write hundreds of lines before compiling.

Better rhythm:

```text
write small piece
compile
fix compiler errors
run small test
commit/checkpoint mentally
write next piece
```

Compiler errors are easier to fix when the recent change is small.

### Enable warnings

Warnings often identify real bugs.

Examples:

- variable may be uninitialized
- unused variable
- implicit conversion loses data
- signed/unsigned comparison
- function missing return
- unreachable code

Treat warnings as useful feedback, not noise.

For local compilation, common warning flags include:

```text
-Wall -Wextra -Wpedantic
```

Some teams also use:

```text
-Werror
```

which turns warnings into errors. This is strict, but it prevents warning piles
from accumulating.

### Use formatting consistently

Formatting does not make code correct by itself, but it makes incorrect
structure easier to see.

Bad indentation can hide logic errors:

```cpp
if (x > 0)
    std::cout << "positive\n";
    std::cout << "done\n"; // always runs
```

The indentation suggests both lines are controlled by the `if`, but only the
first line is.

Clear version:

```cpp
if (x > 0)
{
    std::cout << "positive\n";
}

std::cout << "done\n";
```

### Test incrementally

When building a function, test ordinary and boundary cases.

```cpp
int clampToNonnegative(int x)
{
    if (x < 0)
        return 0;

    return x;
}
```

Useful cases:

```text
x = -1 -> 0
x =  0 -> 0
x =  1 -> 1
```

The boundary is where many bugs live.

### Use static analysis and sanitizers when available

Static analysis inspects code without running it.

Sanitizers instrument the program to catch runtime errors.

Examples:

```text
AddressSanitizer: out-of-bounds, use-after-free
UndefinedBehaviorSanitizer: undefined behavior cases
ThreadSanitizer: data races
```

For CS6340, this ties directly into the course theme:

```text
static analysis: reason without executing
dynamic analysis: observe while executing
```

### Keep changes small

Small changes are easier to debug.

```text
one bug + one small change = easier cause/effect
many bugs + many changes = mystery soup
```

For labs, this means:

- get the baseline build working
- make one behavioral change
- compile
- run the smallest relevant test
- then continue

## 3.x — Chapter 3 summary

- Syntax errors violate the grammar of C++.
- Compile-time semantic errors violate meaning/type/rule constraints that the
  compiler can check.
- Runtime errors occur during execution.
- Logical errors compile and run but produce wrong behavior.
- Debugging should be evidence-driven: reproduce, characterize, localize, fix,
  and retest.
- The symptom is not necessarily the root cause.
- A good debugging strategy compares expected state to actual state and finds
  the first divergence.
- Print debugging is useful when prints are specific and contextual.
- `std::cerr` is usually better than `std::cout` for diagnostics.
- Conditional debug output and preprocessor-controlled diagnostics can reduce
  noise.
- Assertions document internal assumptions and stop quickly when they are false.
- A debugger lets you step, inspect values, use breakpoints, and read the call
  stack.
- Step over executes a call without entering it.
- Step into enters the called function.
- Step out finishes the current function and returns to the caller.
- Breakpoints pause execution at selected lines.
- Conditional breakpoints pause only when an expression is true.
- Watches track variables or expressions while debugging.
- The call stack shows the active chain of function calls and helps trace from
  symptom back toward caller-side causes.
- Warnings, formatting, incremental compilation, small tests, static analysis,
  and sanitizers catch issues earlier.

