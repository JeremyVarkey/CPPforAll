# Chapter 8 — Control Flow

> Source: <https://www.learncpp.com/> (Chapter 8)
> One file per chapter. Each lesson is a section below.

## Contents
- [8.1 — Control flow introduction](#81--control-flow-introduction)
- [8.2 — If statements and blocks](#82--if-statements-and-blocks)
- [8.3 — Common if statement problems](#83--common-if-statement-problems)
- [8.4 — Constexpr if statements](#84--constexpr-if-statements)
- [8.5 — Switch statement basics](#85--switch-statement-basics)
- [8.6 — Switch fallthrough and scoping](#86--switch-fallthrough-and-scoping)
- [8.7 — Goto statements](#87--goto-statements)
- [8.8 — Introduction to loops and while statements](#88--introduction-to-loops-and-while-statements)
- [8.9 — Do while statements](#89--do-while-statements)
- [8.10 — For statements](#810--for-statements)
- [8.11 — Break and continue](#811--break-and-continue)
- [8.12 — Halts (exiting your program early)](#812--halts-exiting-your-program-early)
- [8.13 — Introduction to random number generation](#813--introduction-to-random-number-generation)
- [8.14 — Generating random numbers using Mersenne Twister](#814--generating-random-numbers-using-mersenne-twister)
- [8.15 — Global random numbers (Random.h)](#815--global-random-numbers-randomh)
- [8.x — Chapter 8 summary and quiz](#8x--chapter-8-summary-and-quiz)

---

## 8.1 — Control flow introduction

### What "control flow" means

**Control flow** is the order in which statements execute.

The simplest program runs top-to-bottom:

```cpp
std::cout << "A\n";
std::cout << "B\n";
std::cout << "C\n";
```

Control flow:

```
A -> B -> C
```

But real programs need branches, loops, function calls, early exits, and error
paths. Those tools let execution take different paths.

### Categories of control flow statements

| Category | Purpose | Examples |
|---|---|---|
| Conditional branches | Choose whether/which code runs | `if`, `else`, `switch` |
| Jumps | Move execution somewhere else | `break`, `continue`, `return`, `goto` |
| Function calls | Transfer control to a function and come back | `doWork()` |
| Loops | Repeat code | `while`, `do while`, `for`, range-for |
| Halts | Stop the program | `std::exit`, `std::abort` |
| Exceptions | Nonlocal error handling | `throw`, `try`, `catch` |

CS6340 tie-in:

```
fuzzer main loop
  -> choose seed/campaign       (branch)
  -> mutate input               (function call)
  -> run target                 (function call / process)
  -> if crash: store failure    (branch)
  -> repeat until budget done   (loop)
```

Control flow is the skeleton of both your own C++ code and the programs you
analyze.

---

## 8.2 — If statements and blocks

### Basic `if`

An `if` statement conditionally executes a statement when its condition is true.

```cpp
if (score >= 90)
    std::cout << "A\n";
```

With an `else`:

```cpp
if (score >= 60)
    std::cout << "pass\n";
else
    std::cout << "fail\n";
```

### Blocks

A **block** groups multiple statements using braces:

```cpp
if (crashed)
{
    storeCrashingInput(input);
    ++failureCount;
}
```

Without braces, only the next single statement is controlled by the `if`.

Bad:

```cpp
if (crashed)
    storeCrashingInput(input);
    ++failureCount; // always runs; indentation lies
```

The compiler sees:

```cpp
if (crashed)
{
    storeCrashingInput(input);
}
++failureCount;
```

Best practice for this course: use braces, even for one-line bodies. It removes
ambiguity and makes future edits safer.

### Implicit blocks

The body of an `if` can be a single statement without braces, but it behaves like
a one-statement block. LearnCpp discusses the distinction, but the pragmatic
rule is:

```
write the braces now; future-you edits faster and breaks less
```

### `if-else` chain vs separate `if`s

Separate `if`s are independent:

```cpp
if (x > 0)  { std::cout << "positive\n"; }
if (x < 10) { std::cout << "small\n"; }
```

Both can run.

An `if` / `else if` / `else` chain picks at most one branch:

```cpp
if (x < 0)
{
    std::cout << "negative\n";
}
else if (x == 0)
{
    std::cout << "zero\n";
}
else
{
    std::cout << "positive\n";
}
```

ASCII:

```
separate ifs:
  test A -> maybe run A
  test B -> maybe run B
  test C -> maybe run C

if/else chain:
  test A -> if yes, stop chain
        -> else test B -> if yes, stop chain
        -> else run final fallback
```

Use a chain when the cases are mutually exclusive.

---

## 8.3 — Common if statement problems

### Dangling `else`

An `else` attaches to the nearest unmatched `if`.

```cpp
if (a)
    if (b)
        doThing();
else
    doOtherThing(); // attaches to if (b), not if (a)
```

The indentation suggests one thing; the grammar does another.

Use braces:

```cpp
if (a)
{
    if (b)
    {
        doThing();
    }
}
else
{
    doOtherThing();
}
```

### Flattening nested if-statements

Deep nesting makes logic hard to scan.

Nested:

```cpp
if (hasSeed)
{
    if (!crashed)
    {
        if (count < maxTests)
        {
            runNextTest();
        }
    }
}
```

Flattened with early returns/continues:

```cpp
if (!hasSeed)
    return;

if (crashed)
    return;

if (count >= maxTests)
    return;

runNextTest();
```

This style keeps the main path less buried. Use it carefully, but it is often
clearer in validation-heavy code.

### Null statements

A semicolon by itself is a **null statement**: it does nothing.

```cpp
if (ready); // null statement controlled by if
{
    launch(); // always runs
}
```

Compiler interpretation:

```cpp
if (ready)
{
    // do nothing
}

launch();
```

This tiny semicolon is a classic bug.

### `=` vs `==`

Assignment:

```cpp
if (x = 0) // assigns 0, then tests 0 -> false
```

Comparison:

```cpp
if (x == 0) // tests equality
```

Modern compilers warn about many suspicious assignments in conditions. Treat
warnings seriously; this is exactly the kind of mistake warnings catch.

---

## 8.4 — Constexpr if statements

### Runtime `if` vs `constexpr if`

A normal `if` is evaluated at runtime:

```cpp
if (userChoice == 1)
{
    runCampaignA();
}
```

A `constexpr if` is evaluated at compile time:

```cpp
if constexpr (sizeof(int) == 4)
{
    // compiled when the condition is true
}
else
{
    // discarded when the condition is true
}
```

The condition must be a constant expression.

### Why `if constexpr` exists

It lets compile-time code include or discard branches before runtime. This is
especially useful in template-heavy code where one branch may be invalid for
some types.

Mental model:

```
source:
  if constexpr (compile_time_condition)
      branch A
  else
      branch B

compiler keeps only the selected branch
```

### Difference from optimizer removing an ordinary `if`

A modern compiler may optimize away a normal `if` if its condition is provably
constant. But `if constexpr` is a language feature: the discarded branch is not
instantiated in the same way. That distinction becomes important with templates.

For CS6340 Lab 1, you probably will not need `if constexpr` directly. Know what
it means when you see it: compile-time branch, not a fuzzer runtime branch.

---

## 8.5 — Switch statement basics

### What `switch` does

A `switch` picks among discrete cases based on one controlling expression.

```cpp
switch (campaign)
{
case MutationA:
    runA();
    break;
case MutationB:
    runB();
    break;
case MutationC:
    runC();
    break;
default:
    reportBadCampaign();
    break;
}
```

This maps cleanly to enum-like choices.

### Case labels

`case` labels must be constant expressions:

```cpp
case 1:
case 'a':
case SomeEnumValue:
```

They are labels, not blocks.

### `default`

`default` handles the no-match case. Use it unless every impossible case is
already handled and you have a deliberate reason to omit it.

```cpp
default:
    std::cerr << "unknown option\n";
    break;
```

### `break`

Without `break`, execution continues into the next case. That is called
fallthrough and is usually a bug.

```cpp
switch (x)
{
case 1:
    std::cout << "one\n";
    break;
case 2:
    std::cout << "two\n";
    break;
}
```

### Switch vs if-else

Use `switch` when:

- you compare one expression against many discrete values,
- cases are simple and enum-like,
- the structure is easier to scan than a long `else if`.

Use `if` / `else if` when:

- conditions are ranges,
- conditions use different variables,
- conditions are complex boolean expressions.

```cpp
if (age < 18)              // range: use if
if (name == "Ada")         // string compare: use if
switch (campaign)          // enum/discrete: switch fits
```

---

## 8.6 — Switch fallthrough and scoping

### Fallthrough

Fallthrough happens when a `case` does not end with `break`, `return`, or another
control transfer.

```cpp
switch (n)
{
case 1:
    std::cout << "one\n";
case 2:
    std::cout << "two\n";
}
```

If `n == 1`, output is:

```
one
two
```

This is legal C++, but usually unintended.

### Intentional fallthrough

If fallthrough is intentional, mark it:

```cpp
switch (tier)
{
case 3:
    unlockAdvanced();
    [[fallthrough]];
case 2:
    unlockIntermediate();
    [[fallthrough]];
case 1:
    unlockBasic();
    break;
}
```

`[[fallthrough]]` documents intent and silences compiler warnings.

### Sequential case labels

Multiple labels can share the same body:

```cpp
switch (ch)
{
case 'y':
case 'Y':
    accept();
    break;

case 'n':
case 'N':
    reject();
    break;
}
```

### Labels do not create scope

`case` labels do not create a new block scope.

Potentially confusing:

```cpp
switch (x)
{
case 1:
    int value { 5 }; // can cause initialization/scope problems
    break;
case 2:
    // value's scope rules are not what a beginner expects
    break;
}
```

Use braces for case-local variables:

```cpp
switch (x)
{
case 1:
{
    int value { 5 };
    use(value);
    break;
}
case 2:
{
    int value { 10 };
    use(value);
    break;
}
}
```

Mental model:

```
switch body is one scope
case labels are signposts inside that scope
braces create actual local rooms
```

---

## 8.7 — Goto statements

### What `goto` does

`goto` jumps to a statement label inside the same function.

```cpp
goto cleanup;

cleanup:
    closeFiles();
```

Statement labels have function scope.

### Why avoid it

`goto` can make control flow hard to reason about because execution no longer
follows visible structure.

```
structured:
  if -> loop -> function return

goto-heavy:
  jump here -> jump there -> maybe skip initialization -> surprise
```

Best practice: avoid `goto` in normal C++.

Modern alternatives:

- functions,
- loops,
- `break`,
- `continue`,
- `return`,
- RAII/resource-owning objects,
- exceptions for exceptional paths.

You may still see `goto` in low-level C for cleanup paths, but it should not be
your first tool in CS6340 lab code.

---

## 8.8 — Introduction to loops and while statements

### Loop basics

A loop repeats code while a condition says to continue.

```cpp
while (count < 5)
{
    std::cout << count << '\n';
    ++count;
}
```

Execution:

```
test condition
  true  -> run body -> test again
  false -> exit loop
```

### `while`

A `while` loop checks the condition before each iteration. If the condition is
false at the start, the body never runs.

```cpp
while (hasWork())
{
    doOnePieceOfWork();
}
```

### Infinite loops

Intentional infinite loop:

```cpp
while (true)
{
    if (shouldStop())
        break;
}
```

Unintentional infinite loop:

```cpp
int i { 0 };
while (i < 10)
{
    std::cout << i << '\n';
    // forgot ++i
}
```

Every loop needs a believable exit story.

### Loop variables

A loop variable tracks progress:

```cpp
int count { 0 };
while (count < 10)
{
    ++count;
}
```

LearnCpp recommends signed integral loop variables. This avoids many
signed/unsigned comparison bugs that arise with `std::size_t`.

### Doing something every N iterations

Useful fuzzer pattern:

```cpp
if (count % 1000 == 0)
{
    std::cerr << "Count is " << count << '\n';
}
```

Modulo `%` gives the remainder. If `count % 1000 == 0`, then `count` is evenly
divisible by 1000.

### Nested loops

A nested loop is a loop inside a loop.

```cpp
for (int row { 0 }; row < 3; ++row)
{
    for (int col { 0 }; col < 4; ++col)
    {
        std::cout << row << ',' << col << ' ';
    }
    std::cout << '\n';
}
```

Grid mental model:

```
outer loop: rows
  row 0 -> inner loop visits col 0,1,2,3
  row 1 -> inner loop visits col 0,1,2,3
  row 2 -> inner loop visits col 0,1,2,3
```

Lab 1 Part 2 has this shape:

```
for each seed:
    for each test budget:
        mutate
        run target
        maybe stop after crash
```

---

## 8.9 — Do while statements

### `do while`

A `do while` loop runs the body first, then checks the condition.

```cpp
int choice {};

do
{
    std::cout << "Enter 1-3: ";
    std::cin >> choice;
}
while (choice < 1 || choice > 3);
```

Execution:

```
run body once
test condition
  true  -> run body again
  false -> exit
```

Use `do while` when the body must run at least once, such as prompting for input
before you can validate it.

For most other loops, `while` or `for` tends to read better.

---

## 8.10 — For statements

### Basic form

```cpp
for (init; condition; end-expression)
{
    body;
}
```

Example:

```cpp
for (int i { 0 }; i < 5; ++i)
{
    std::cout << i << '\n';
}
```

Evaluation order:

```
1. init once
2. test condition
3. if true, run body
4. run end-expression
5. go back to step 2
```

ASCII:

```
init -> condition? --false--> exit
          |
        true
          v
        body -> end-expression -> condition?
```

### Prefer `<` over `!=` for many numeric loops

This loop can run too long if `i` skips the target:

```cpp
for (int i { 0 }; i != 10; i += 2)
{
    // never hits 10 if step pattern changes badly
}
```

Safer:

```cpp
for (int i { 0 }; i < 10; i += 2)
{
}
```

### Off-by-one errors

Classic bug:

```cpp
for (int i { 0 }; i <= 10; ++i) // runs 11 times: 0 through 10
{
}
```

If you want 10 iterations starting from zero:

```cpp
for (int i { 0 }; i < 10; ++i) // 0 through 9
{
}
```

Range picture:

```
0 1 2 3 4 5 6 7 8 9 | 10
[ included values   ) excluded boundary
```

C++ loops commonly use half-open ranges: `[start, end)`.

### Omitted expressions

Any of the three for-loop expressions can be omitted:

```cpp
int i { 0 };
for (; i < 10; )
{
    ++i;
}
```

An infinite `for` loop:

```cpp
for (;;)
{
    // loop forever unless body exits
}
```

Use omitted expressions sparingly; clarity matters.

### Multiple counters

```cpp
for (int left { 0 }, right { 9 }; left < right; ++left, --right)
{
    std::cout << left << ' ' << right << '\n';
}
```

This is useful for symmetric scans, but it can become clever quickly.

### Define loop-only variables inside the loop

Prefer:

```cpp
for (int i { 0 }; i < maxTests; ++i)
{
    runTest(i);
}
```

Instead of:

```cpp
int i {};
for (i = 0; i < maxTests; ++i)
{
}
```

Keeping `i` inside the loop limits its scope and reduces accidental reuse.

---

## 8.11 — Break and continue

### `break`

`break` exits the nearest enclosing loop or switch.

```cpp
for (int i { 0 }; i < maxTests; ++i)
{
    if (failureCount >= maxCrashes)
        break;

    runOneTest();
}
```

In a `switch`, `break` prevents fallthrough.

### `break` vs `return`

`break` exits the loop/switch. `return` exits the whole function.

```cpp
for (...)
{
    if (done)
        break;  // continue after loop
}

return;         // leave function now
```

### `continue`

`continue` skips the rest of the current loop iteration and moves to the next
iteration.

```cpp
for (int i { 0 }; i < maxTests; ++i)
{
    if (!inputIsValid(i))
        continue;

    runTest(i);
}
```

### When to use break/continue

They are good when they make the exit/skip condition explicit and keep the main
path readable.

Reasonable:

```cpp
for (const auto& seed : seeds)
{
    if (seed.empty())
        continue;

    mutate(seed);
}
```

Less good: hiding many exits in a long loop body so the reader cannot tell when
the loop stops.

Best practice for your lab code: short loops with clear `break` / `continue`
conditions are fine. If the loop grows huge, extract helper functions.

---

## 8.12 — Halts (exiting your program early)

### Normal returns vs halts

Normally, functions return to their caller:

```cpp
return 0;
```

A **halt** stops the program without normal step-by-step return through the call
stack.

### `std::exit`

`std::exit()` terminates the program and returns a status code to the operating
system.

```cpp
#include <cstdlib>

std::exit(1);
```

Important: `std::exit()` does not clean up local automatic variables the same way
normal scope exit does. That means destructors for local objects may not run.

```
normal return:
  leave scope -> destroy locals -> caller resumes

std::exit:
  terminate process path -> local scope cleanup can be skipped
```

### `std::atexit`

You can register functions to run when `std::exit()` is called:

```cpp
std::atexit(cleanup);
```

This is niche. Prefer ordinary control flow unless you have a real process-level
termination need.

### `std::abort` and `std::terminate`

These end the program abnormally. They are for severe failure paths, not normal
validation.

CS6340 tie-in: sanitizers often abort a program when they detect a serious
runtime violation. Lab 1's runtime divide-by-zero check intentionally stops the
target program when the bug is detected.

### When to halt

Use a halt for unrecoverable process-level failure. In ordinary functions, prefer
return values, errors, or exceptions depending on the codebase.

---

## 8.13 — Introduction to random number generation

### Randomness in computers

Computers are deterministic. Most "random" numbers in ordinary programs are
generated by algorithms that produce sequences that look random.

A **pseudo-random number generator** (PRNG) is an algorithm that produces a
sequence of values from internal state.

```
seed -> PRNG state -> number -> new state -> number -> new state -> ...
```

Same seed usually means same sequence:

```
seed 123 -> 8, 42, 19, 7, ...
seed 123 -> 8, 42, 19, 7, ...
```

That can be good for debugging and bad for unpredictability.

### State

A PRNG has state. Each generated number updates that state. If you repeatedly
restart the generator with the same seed, you restart the same sequence.

Bad pattern:

```cpp
for (...)
{
    std::mt19937 rng { 12345 }; // re-created every iteration
    // produces same first value every time
}
```

Better:

```cpp
std::mt19937 rng { seed };

for (...)
{
    // reuse rng; state advances
}
```

### Seed quality

A **seed** initializes the generator's state. Better seeds produce less
predictable starting states.

Underseeding means giving a large-state PRNG too little meaningful entropy. The
sequence may still look random enough for games/tests, but it is not
cryptographic quality.

### PRNG quality

Good PRNGs tend to have:

- long period,
- good distribution,
- low correlation,
- acceptable speed,
- enough state space.

For CS6340 fuzzing, the goal is not cryptographic randomness. The goal is
effective exploration of input space.

### C++ randomization tools

Modern C++ uses `<random>` rather than old C `rand()`.

Core pieces:

| Piece | Role |
|---|---|
| Engine | Generates raw pseudo-random bits, e.g. `std::mt19937` |
| Distribution | Maps raw bits to useful range/shape, e.g. `std::uniform_int_distribution` |
| Seed source | Initializes engine, e.g. fixed seed, clock, `std::random_device` |

```
seed -> engine -> raw random bits -> distribution -> value in range
```

---

## 8.14 — Generating random numbers using Mersenne Twister

### Mersenne Twister

C++ provides Mersenne Twister engines:

```cpp
std::mt19937     // 32-bit Mersenne Twister
std::mt19937_64  // 64-bit Mersenne Twister
```

Use them via `<random>`.

### Basic dice roll

```cpp
#include <iostream>
#include <random>

int main()
{
    std::mt19937 rng { 12345 }; // fixed seed: reproducible
    std::uniform_int_distribution die { 1, 6 };

    std::cout << die(rng) << '\n';
}
```

Important: the distribution is called with the engine:

```cpp
die(rng)
```

### Fixed seed vs variable seed

Fixed seed:

```cpp
std::mt19937 rng { 12345 };
```

Pros:

- reproducible,
- excellent for debugging,
- same failing sequence can be rerun.

Cons:

- same sequence every run,
- weaker exploration if you always keep it fixed.

Variable seed:

```cpp
std::random_device rd;
std::mt19937 rng { rd() };
```

Pros:

- different sequence across runs,
- better exploration.

Cons:

- harder to reproduce bugs unless you log the seed.

### Seeding with the clock

Another common source:

```cpp
#include <chrono>
#include <random>

auto seed {
    static_cast<std::mt19937::result_type>(
        std::chrono::steady_clock::now().time_since_epoch().count()
    )
};

std::mt19937 rng { seed };
```

Clock seeding changes across runs, but it is not high-quality entropy.

### Only seed once

Do not reseed before each number.

Bad:

```cpp
for (int i { 0 }; i < 100; ++i)
{
    std::mt19937 rng { std::random_device{}() }; // repeated setup
    std::cout << dist(rng) << '\n';
}
```

Good:

```cpp
std::mt19937 rng { std::random_device{}() };

for (int i { 0 }; i < 100; ++i)
{
    std::cout << dist(rng) << '\n';
}
```

### Fuzzer debugging trick

For Lab 1, consider printing/logging the seed when a random strategy works:

```cpp
constexpr unsigned seed { 12345 }; // during debugging
std::mt19937 rng { seed };
```

When you find a crash, reproducibility is gold.

```
random crash found
  -> record seed + target + mutation campaign
  -> rerun same sequence
  -> debug with stable behavior
```

### Common random bugs

Same sequence every run:

- likely fixed seed,
- or deterministic default construction.

Same number repeatedly:

- generator recreated/reseeded inside the loop,
- distribution range too small,
- value not updated.

---

## 8.15 — Global random numbers (Random.h)

### Why a helper exists

Random generation is needed in many functions. Passing an engine everywhere can
be verbose; creating a new one everywhere is wrong. LearnCpp introduces a helper
header pattern (`Random.h`) to provide shared random utilities.

Conceptual shape:

```cpp
namespace Random
{
    inline std::mt19937 mt { std::random_device{}() };

    int get(int min, int max)
    {
        std::uniform_int_distribution dist { min, max };
        return dist(mt);
    }
}
```

This gives one shared engine whose state advances across calls.

Usage:

```cpp
int index { Random::get(0, 9) };
```

### Why not put everything in globals casually?

Global state has tradeoffs:

- easy to access,
- hard to reason about in large programs,
- can make tests less isolated,
- can hide dependencies.

For a small learning project or fuzzer helper, a controlled random namespace can
be fine. In larger production code, dependency injection or explicit engine
passing may be better.

### CS6340 mutation helper mental model

Good fuzzer random state:

```
one engine per fuzzer run
  -> mutateA uses it
  -> mutateB uses it
  -> mutateC uses it
  -> state advances naturally
```

Bad:

```
mutateA creates new engine every call
  -> weak randomness
  -> repeated values
  -> poor exploration
```

---

## 8.x — Chapter 8 summary and quiz

### Core takeaways

- Control flow is the order in which statements execute.
- Use braces for `if`, `else`, loops, and `switch` cases with local variables.
- `else` attaches to the nearest unmatched `if`.
- A stray semicolon after `if (...)` creates a null statement bug.
- Use `==` for comparison, `=` for assignment.
- `if constexpr` is a compile-time branch.
- `switch` is best for one expression compared against discrete values.
- `break` prevents switch fallthrough and exits loops.
- Mark intentional fallthrough with `[[fallthrough]]`.
- `case` labels do not create scope; braces do.
- Avoid `goto` in normal C++.
- `while` checks before running; `do while` runs at least once.
- `for` is ideal for counted iteration.
- Prefer half-open numeric loops: `i < count`, not `i <= count - 1`.
- `continue` skips to the next iteration.
- Halts like `std::exit` stop the process and bypass some normal cleanup.
- PRNGs produce deterministic sequences from state.
- Seed once, then reuse the engine.
- Use `<random>` tools like `std::mt19937` and distributions.

### Lab 1 control-flow map

```cpp
for (int seedIndex { 0 }; seedIndex < seedCount; ++seedIndex)
{
    std::string current { seeds[seedIndex] };

    for (int test { 0 }; test < maxTestsPerSeed; ++test)
    {
        std::string mutant { mutate(current) };

        if (crashesTarget(mutant))
        {
            saveFailure(mutant);
            break;
        }

        current = mutant;
    }
}
```

Concepts used:

- `for`: bounded mutation budget.
- `if`: crash branch.
- `break`: stop after success.
- `std::string`: owned mutable input.
- random generation: mutation choices.

### Mini drill

Write a small loop that mutates a string by replacing one random character:

```cpp
#include <iostream>
#include <random>
#include <string>

int main()
{
    std::mt19937 rng { 12345 };
    std::uniform_int_distribution charDist { 0, 25 };

    std::string input { "aaaaa" };

    for (int i { 0 }; i < 10; ++i)
    {
        std::uniform_int_distribution indexDist {
            0,
            static_cast<int>(input.length()) - 1
        };

        int index { indexDist(rng) };
        char replacement { static_cast<char>('a' + charDist(rng)) };

        input[static_cast<std::size_t>(index)] = replacement;
        std::cout << input << '\n';
    }
}
```

What this reinforces:

- one engine seeded once,
- `for` loop for bounded iteration,
- distribution for index selection,
- distribution for character selection,
- explicit cast where signed loop/index math meets string size types,
- mutating previous input rather than ignoring it.
