# Chapter 8 — Control Flow

> Source: <https://www.learncpp.com/> (Chapter 8)

Up to now your programs have been straight roads: statements run top to bottom, once each, in the order you wrote them. That's fine for arithmetic on a fixed set of inputs, but it can't make a decision, can't repeat work, and can't bail out early when something goes wrong. This chapter hands you the steering wheel. You'll learn how to *branch* (choose which code runs), how to *loop* (run code many times), how to *jump* out of a loop the moment you're done, and how to stop the whole program when you must. We finish with randomness — how a deterministic machine produces numbers that look unpredictable, and how to do it correctly in modern C++. By the end you'll have every control-flow tool the rest of the course relies on, and you'll be ready to build this chapter's lab: a number-guessing engine whose decision logic you can test exactly.

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

Every program is a list of statements, but the order in which those statements actually execute is a separate thing from the order in which you wrote them. That execution order is called the program's **control flow**, and the statements that bend it — that let execution skip ahead, double back, or branch — are **control flow statements**.

When you've never used one, control flow is invisible because it's trivial. The program enters at the top of `main` and walks straight down:

```cpp
std::cout << "A\n";
std::cout << "B\n";
std::cout << "C\n";
```

The path of execution — the **straight-line** sequence the program follows — is simply A, then B, then C. There are no choices to make.

Real programs are not straight lines. They ask questions ("did the user type a valid number?"), repeat work ("keep dealing cards until the deck is empty"), and abandon a task partway through when something breaks. Each of those needs a tool that lets execution take a *different* path depending on what's true at that moment.

### A map of the tools

There are more control-flow tools than you might expect, and it helps to see them grouped by what they do before we meet them one at a time:

| Category | What it does | Examples |
|---|---|---|
| Conditional branches | Choose *whether*, or *which*, code runs | `if`, `else`, `switch` |
| Jumps | Move execution to another spot | `break`, `continue`, `return`, `goto` |
| Function calls | Hand control to a function, then resume | `doWork()` |
| Loops | Repeat a block of code | `while`, `do while`, `for` |
| Halts | Stop the program entirely | `std::exit`, `std::abort` |
| Exceptions | Handle errors out of the normal flow | `throw`, `try`, `catch` |

You've already met function calls. This chapter covers branches, loops, jumps, and halts. Exceptions get a chapter of their own much later — they're a way to handle errors without threading return codes through every function.

> **Key insight:** Control flow is the skeleton of a program. The data and the arithmetic are the muscle, but the branches and loops are what give a program its *shape* — the ability to respond differently to different situations instead of doing the exact same thing every time.

---

## 8.2 — If statements and blocks

### The basic `if`

The simplest branch is the **if statement**. It runs a statement only when a condition is true, and otherwise skips it:

```cpp
if (score >= 90)
    std::cout << "You earned an A\n";
```

The parentheses hold a **condition** — any expression that can be interpreted as a Boolean. If it's true, the controlled statement runs; if it's false, execution jumps straight past it.

Add an `else` to say "and if it *wasn't* true, do this instead":

```cpp
if (score >= 60)
    std::cout << "Pass\n";
else
    std::cout << "Fail\n";
```

Exactly one of those two lines runs, never both, never neither.

### Blocks: controlling more than one statement

On its own, an `if` controls a single statement — the next one, and only the next one. To make it control *several* statements as a unit, you wrap them in a **block**: a group of statements enclosed in braces `{ }`, which the language treats as one compound statement.

```cpp
if (passwordCorrect)
{
    grantAccess();
    logSuccessfulLogin();
}
```

Now both calls are governed by the condition. Without the braces, only `grantAccess()` would be conditional and `logSuccessfulLogin()` would run unconditionally — which brings us to a classic trap.

### The braceless trap

Indentation is for humans; the compiler ignores it entirely. So this code does *not* do what its layout suggests:

```cpp
if (accessGranted)
    openVault();
    soundAllClear();   // BUG: not controlled by the if — always runs
```

The compiler reads it exactly as if you had written:

```cpp
if (accessGranted)
{
    openVault();
}
soundAllClear();       // always runs, regardless of the condition
```

The fix is to always use a block, even when the body is a single statement:

```cpp
if (accessGranted)
{
    openVault();
    soundAllClear();
}
```

> **Best practice:** Put braces on every `if`, `else`, and loop body, even one-liners. It costs two characters and a line, and it permanently removes the "I added a second statement and forgot the braces" bug — the single most common beginner mistake in this whole chapter.

### Independent `if`s versus an `if`/`else if` chain

Two separate `if` statements are *independent*. Each is tested on its own, and any number of them can run:

```cpp
if (x > 0)  { std::cout << "positive\n"; }
if (x < 10) { std::cout << "small\n"; }
```

For `x == 5`, both fire — it's positive *and* small.

An `if` / `else if` / `else` **chain** is different: it picks *at most one* branch. The moment a condition is true, that branch runs and the rest of the chain is skipped:

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

The mental distinction:

```
separate ifs:                 if/else chain:
  test A -> maybe run A         test A -> if true, done
  test B -> maybe run B         else test B -> if true, done
  test C -> maybe run C         else run the final fallback
  (any number can fire)         (exactly one fires)
```

Reach for a chain when your cases are **mutually exclusive** — when a value is "negative or zero or positive," not "possibly several at once." Saying it as a chain documents that intent, and it's faster because the program stops testing once it finds the match. The lab's `judgeGuess` is exactly this shape: a guess is too low, *or* too high, *or* exactly right — never two of those — so it's one clean chain.

---

## 8.3 — Common if statement problems

`if` is simple, which is precisely why its bugs are sneaky: the code compiles, runs, and quietly does the wrong thing. Here are the four classics, all worth recognizing on sight.

### The dangling else

An `else` always binds to the *nearest* `if` that doesn't already have one. Indentation can make it look otherwise:

```cpp
if (a)
    if (b)
        doThing();
else                 // looks like it pairs with if (a)...
    doOtherThing();  // ...but it actually pairs with if (b)
```

Despite the indentation, that `else` belongs to `if (b)`. If you meant it to pair with `if (a)`, braces force the issue and make the grouping unambiguous:

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
    doOtherThing();   // now unambiguously paired with if (a)
}
```

This is the braces best practice earning its keep a second time.

### Flattening deep nesting

Nested `if`s pile up fast, and deeply indented logic is hard to follow because the "main path" gets buried under guard conditions:

```cpp
if (hasInput)
{
    if (!isCorrupt)
    {
        if (count < limit)
        {
            process();
        }
    }
}
```

A cleaner shape handles each failing condition first and leaves early, so the happy path sits at the bottom with no indentation at all:

```cpp
if (!hasInput)     return;
if (isCorrupt)     return;
if (count >= limit) return;

process();
```

These are called **early returns** (or **guard clauses**), and they trade nesting for a flat list of preconditions. They read like a checklist: "bail if no input; bail if corrupt; bail if over the limit; otherwise, process." Use them where they clarify; they shine in validation-heavy code.

### The null statement

A lone semicolon is a complete, do-nothing statement called a **null statement**. It's legal, occasionally useful, and a perfect disguise for a bug:

```cpp
if (ready);          // <-- this semicolon IS the if's entire body
{
    launch();        // a plain block; runs no matter what
}
```

The compiler reads that as "if `ready`, do nothing," followed by an unconditional block. So `launch()` fires whether or not `ready` is true. The stray semicolon is invisible at a glance, which is exactly what makes it dangerous.

### `=` versus `==`

`=` assigns; `==` compares. Type the wrong one in a condition and you get code that compiles but tests the wrong thing:

```cpp
if (x = 0)    // assigns 0 to x, then tests 0 -> always false
if (x == 0)   // tests whether x equals 0 -> what you meant
```

`if (x = 0)` assigns `0` to `x` and then uses that `0` as the condition, which is always false; `if (x = 5)` would assign `5` and be always true. Either way the test is meaningless and `x` got clobbered.

> **Warning:** Most compilers will warn when you assign inside a condition, because it's so often a typo. Compile with warnings on (`-Wall -Wextra`) and treat them as errors to fix, not noise to scroll past. This is the category of bug warnings were *invented* to catch.

---

## 8.4 — Constexpr if statements

### Runtime `if` versus compile-time `if`

An ordinary `if` is a *runtime* decision. The condition is evaluated while the program runs, every time execution reaches it, and the machine code for *both* branches is present in the program:

```cpp
if (userChoice == 1)   // decided when the program runs
{
    runOptionA();
}
```

A **constexpr if statement**, written `if constexpr`, moves the decision to *compile time*. The condition must be a constant expression — something the compiler can evaluate while building your program — and the compiler keeps only the branch that's selected, discarding the other entirely:

```cpp
if constexpr (sizeof(int) == 4)
{
    // compiled into the program when the condition is true
}
else
{
    // discarded — as if you'd never written it — when the condition is true
}
```

The mental model:

```
source you write:                 what the compiler keeps:
  if constexpr (compileTimeCond)     just the selected branch —
      branch A                       the other isn't compiled at all
  else
      branch B
```

### Why it exists

If the condition is known at compile time, why not let the optimizer drop the dead branch from an ordinary `if`? Sometimes it can. But `if constexpr` does something stronger: the discarded branch is *not compiled* — it doesn't even have to be valid code for the case that's thrown away. That distinction is invaluable in template code, where one branch might only make sense for some types and would fail to compile for others. `if constexpr` lets you write "use this approach for these types, that approach for those" and have the compiler quietly discard whichever doesn't apply.

> **Note:** You'll meet templates later in the course, and that's where `if constexpr` truly pays off. For now, the one thing to carry forward: when you see `if constexpr`, read it as "a branch the *compiler* chooses," not a decision made while the program runs.

---

## 8.5 — Switch statement basics

### What `switch` is for

When you find yourself comparing one expression against a list of specific values — `if (x == 1) ... else if (x == 2) ... else if (x == 3) ...` — there's a tool built for exactly that pattern. A **switch statement** takes a single expression and jumps directly to the matching case:

```cpp
switch (menuChoice)
{
case 1:
    newGame();
    break;
case 2:
    loadGame();
    break;
case 3:
    quit();
    break;
default:
    std::cout << "Not a valid option\n";
    break;
}
```

The expression in `switch (...)` is evaluated once, and control jumps to the `case` label whose value matches. It reads more clearly than a long `else if` ladder when every test is "is it *this* specific value?", and the compiler can often turn it into a fast jump rather than a sequence of comparisons.

### Case labels

A **case label** marks where to jump for a particular value. The value after `case` must be a **constant expression** — a literal, a `constexpr`, or (later) an enumerator — never a runtime variable:

```cpp
case 1:              // an integer literal
case 'a':            // a char literal (chars are integral)
case maxRetries:     // ok if maxRetries is constexpr
```

Note the colon: these are *labels*, signposts that mark a position, not blocks. The switch condition must be an integral type (or an enumeration); you can't `switch` on a `double` or a `std::string`.

### The `default` label

The **default label** is where control jumps when no `case` matches — the catch-all:

```cpp
default:
    std::cout << "unknown option\n";
    break;
```

It's optional, but you should almost always include one, if only to notice the cases you didn't expect. The lab leans on this directly: `hintText` maps the codes `-1`, `0`, and `+1` to text, and any *other* code — say `2` or `-2` — must land in `default` and return `"invalid"`. Forgetting `default` there is the difference between a robust function and one that returns garbage on bad input.

### `break`: ending a case

Here's the part that surprises everyone: a `case` label does not automatically stop at the next `case`. Once execution lands on a matching label, it keeps running *downward* through later cases until something stops it. The thing that stops it is **break**, which exits the switch:

```cpp
switch (x)
{
case 1:
    std::cout << "one\n";
    break;          // stop here — don't run into case 2
case 2:
    std::cout << "two\n";
    break;
}
```

Leave out a `break` and execution "falls through" into the next case — usually a bug, and the subject of the next lesson. For now: end every case with `break` (or a `return`).

### `switch` versus `if`/`else`

The two overlap, so here's the rule of thumb:

```cpp
if (age < 18)          // a range, not one value          -> if
if (name == "Ada")     // comparing a string              -> if
if (busy && !paused)   // a compound boolean condition     -> if
switch (menuChoice)    // one int/enum vs discrete values  -> switch
```

Use `switch` when you're matching **one integral expression against several specific constant values** — menu choices, enum states, single characters. Use `if`/`else if` when your tests involve ranges, strings, different variables, or compound boolean logic. The lab uses both deliberately: a `switch` for the fixed set of hint codes, and an `if`/`else` chain where the comparison is a range judgment.

---

## 8.6 — Switch fallthrough and scoping

### Fallthrough

We just saw that a `case` without a `break` lets execution continue into the *next* case. That behavior has a name: **fallthrough**. Watch what happens when the `break`s go missing:

```cpp
switch (n)
{
case 1:
    std::cout << "one\n";
case 2:
    std::cout << "two\n";
}
```

If `n == 1`, control lands on `case 1`, prints `one`, and — finding no `break` — falls straight through into `case 2` and prints `two` as well:

```
one
two
```

This is legal C++, and almost never what you meant. Fallthrough is the reason "always `break`" is a rule rather than a suggestion.

### Intentional fallthrough

Occasionally fallthrough is genuinely what you want — for instance, when several cases should accumulate behavior. When you do mean it, *say so* with the `[[fallthrough]]` attribute, which documents your intent and silences the compiler warning you'd otherwise get:

```cpp
switch (tier)
{
case 3:
    unlockAdvanced();
    [[fallthrough]];        // yes, I really mean to continue
case 2:
    unlockIntermediate();
    [[fallthrough]];
case 1:
    unlockBasic();
    break;
}
```

A platinum member (`tier == 3`) unlocks all three levels; gold unlocks two; silver, one. The attribute makes "I left out the `break` on purpose" visible to both the next reader and the compiler.

### Stacking case labels

A different and very common pattern: several labels sharing one body. This is *not* fallthrough — there's simply no code between the labels, so they all funnel into the same statements:

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

"Either `y` or `Y` means yes." Clean and idiomatic.

### Case labels don't create scope

One subtlety catches people: the body of a `switch` is a *single* scope. The `case` labels are signposts *inside* that one scope, not separate rooms. That means a variable declared in one case is technically in scope for the later cases too — which leads to confusing initialization rules:

```cpp
switch (x)
{
case 1:
    int value { 5 };   // initialization here is problematic...
    break;
case 2:
    // ...because `value` is in scope here too, but skipped over
    break;
}
```

The fix is to give a case its own block with braces, creating an actual nested scope:

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
    int value { 10 };  // a different, independent variable
    use(value);
    break;
}
}
```

The mental model:

```
the switch body is one room
case labels are signs posted on the wall
braces build actual walled-off rooms inside
```

> **Best practice:** If a `case` needs its own local variable, wrap that case in braces. It's the same "use a block" instinct from `if` statements, applied to switches.

---

## 8.7 — Goto statements

### What `goto` does

A **goto statement** is an unconditional jump to a labeled spot — a **statement label** — somewhere in the same function:

```cpp
goto cleanup;

// ... some code that gets skipped ...

cleanup:
    closeFiles();
```

Statement labels have *function scope*: you can jump forward or backward to any label in the current function, but not into a different one.

### Why we avoid it

`goto` is powerful and almost always the wrong tool. The problem is that it lets execution leap anywhere, so you can no longer understand the flow by reading the structure — you have to trace every jump by hand. It also makes it easy to jump *over* a variable's initialization and then use it, which is a genuine error the language tries to prevent.

```
structured flow:           goto-heavy flow:
  if -> loop -> return        jump here -> jump there ->
  (you can read the shape)    skip an init -> surprise bug
```

Everything `goto` can do, the structured tools do more clearly: `break` and `continue` to escape loops, `return` to leave a function, helper functions to factor out repetition, and (later) destructors and exceptions for cleanup on the way out.

> **Best practice:** Avoid `goto`. You will occasionally see it in old C code for cleanup-on-error paths, but in C++ you have better, clearer tools for every case where you'd be tempted to reach for it.

---

## 8.8 — Introduction to loops and while statements

### Why loops exist

Branches let a program choose; loops let it *repeat*. Without them, printing the numbers 1 through 100 would mean a hundred `cout` lines. With a loop, it's three. Any time you'd be tempted to copy-paste a statement with a small change each time, a loop is the answer.

### The `while` loop

A **while statement** is the simplest loop. It checks a condition, and *as long as that condition is true*, it runs its body and checks again:

```cpp
int count { 1 };
while (count <= 5)
{
    std::cout << count << ' ';
    ++count;
}
// prints: 1 2 3 4 5
```

The rhythm is **test, then run**:

```
check condition
  true  -> run body -> check again
  false -> exit the loop
```

Because the test comes *first*, a `while` whose condition starts out false runs its body zero times — it checks, finds false, and skips the whole thing. That "might run zero times" behavior is exactly what the lab's `countdownString(0)` relies on: a countdown starting from `0` should produce no numbers at all, just `"liftoff"`.

### Every loop needs an exit story

The single most important habit with loops: something inside the body has to make progress toward the condition becoming false. If it doesn't, the loop never ends.

```cpp
int i { 0 };
while (i < 10)
{
    std::cout << i << '\n';
    // BUG: forgot ++i, so i is always 0, condition is always true
}
```

That loop runs forever. The fix is the missing `++i`. Before you write the closing brace of any loop, ask: *what changes here that will eventually make the condition false?* That answer is the loop's **exit story**, and every loop must have a believable one.

> **Key insight:** A loop without a believable exit story is an infinite loop waiting to happen. Make the thing that ends the loop obvious — usually a counter that advances, or a condition that some body statement is working toward.

### Intentional infinite loops

Sometimes you *want* a loop with no built-in end, and you break out of it explicitly from inside:

```cpp
while (true)            // condition is never false on its own...
{
    if (shouldStop())
        break;          // ...so the exit story lives here
}
```

That's fine — the exit story is the `break`, and it's right there in the body. The danger isn't infinite loops on purpose; it's infinite loops by accident.

### Loop variables and counting

A variable that tracks a loop's progress is a **loop variable** (or **counter**). Conventionally these are integers, and a small but real piece of advice applies:

> **Best practice:** Prefer *signed* integers for loop counters. Unsigned types (like `std::size_t`) wrap around to a huge value when they go below zero, which turns `i >= 0` into an accidental infinite loop and makes signed/unsigned comparisons misbehave. A plain `int` sidesteps a whole category of subtle bugs.

### Doing something every N iterations

A handy idiom uses the modulo operator `%`, which gives the remainder of a division. A number is evenly divisible by `n` exactly when `x % n == 0`:

```cpp
if (count % 1000 == 0)
{
    std::cout << "Processed " << count << " items so far\n";
}
```

Drop that inside a loop and you get a progress report every thousandth iteration. The same `%` powers the lab's digit-summing task: `n % 10` peels off the last decimal digit of a number.

### Nested loops

A loop can live inside another loop. The inner loop runs to completion *for each* single pass of the outer one — the natural shape for anything grid-like:

```cpp
for (int row { 1 }; row <= 3; ++row)
{
    for (int col { 1 }; col <= 4; ++col)
    {
        std::cout << row << ',' << col << "  ";
    }
    std::cout << '\n';
}
```

```
outer loop walks the rows:
  row 1 -> inner loop visits col 1,2,3,4
  row 2 -> inner loop visits col 1,2,3,4
  row 3 -> inner loop visits col 1,2,3,4
```

(That example uses a `for` loop, which we formalize in 8.10 — but the nesting idea is identical for any loop kind.)

---

## 8.9 — Do while statements

### Run first, ask later

A `while` loop tests *before* running, so it can run zero times. Sometimes you need the opposite: a loop that runs its body *at least once* and then decides whether to repeat. That's a **do while statement**:

```cpp
int choice {};
do
{
    std::cout << "Enter a number from 1 to 3: ";
    std::cin >> choice;
}
while (choice < 1 || choice > 3);
```

The rhythm is **run, then test**:

```
run body once
check condition
  true  -> run body again
  false -> exit
```

The reason a `do while` fits here is that you can't validate the input until you've *gotten* some input — you have to run the prompt-and-read body once before there's anything to check. That "must happen at least once before the test makes sense" situation is the entire signature of `do while`.

The lab's `promptsUntilValid` is a deterministic stand-in for exactly this loop: it consumes values one at a time and checks each against a range, but it must consume the *first* value before it can test anything — so a `do while` is the honest choice, even though the input is handed in rather than typed.

> **Best practice:** Use `do while` only when the body genuinely must run before the first test. For nearly everything else, a `while` or `for` reads more clearly, because the exit condition is visible up top rather than buried at the bottom.

---

## 8.10 — For statements

### The shape

The **for statement** is the workhorse loop for counted iteration. It gathers the three things every counting loop needs — set up a counter, test it, advance it — into one compact header so they're impossible to lose track of:

```cpp
for (init-statement; condition; end-expression)
{
    // body
}
```

A concrete loop that prints 0 through 4:

```cpp
for (int i { 0 }; i < 5; ++i)
{
    std::cout << i << '\n';
}
```

Read it left to right: *start* `i` at 0, *keep going while* `i < 5`, and *after each pass* do `++i`.

### How it executes

The order is worth nailing down, because it's not quite top-to-bottom:

```
1. run the init-statement once (int i { 0 })
2. test the condition         (i < 5)
3. if false, exit the loop
4. if true, run the body
5. run the end-expression     (++i)
6. go back to step 2
```

The crucial subtlety: the end-expression (`++i`) runs *after* the body, not before it. So the body sees `i == 0` on the first pass, and `i` isn't bumped to `1` until that pass finishes.

### Prefer `<` over `!=` for numeric loops

These two conditions look interchangeable, but they aren't equally safe:

```cpp
for (int i { 0 }; i != 10; i += 2)   // fragile
for (int i { 0 }; i <  10; i += 2)   // robust
```

The first relies on `i` landing *exactly* on `10`. It does here, but change the step or the start and `i` might leap from `8` to `10`... or from `9` to `11`, sailing past `10` forever. The `<` form stops as soon as `i` reaches *or passes* the bound, so it can't overshoot into an infinite loop.

> **Best practice:** For numeric loops, prefer a relational test (`<`, `<=`) over an equality test (`!=`). It's robust against a counter that skips the exact stopping value.

### Off-by-one errors and half-open ranges

The most common loop bug in existence is being off by exactly one iteration, almost always from `<=` where you wanted `<`:

```cpp
for (int i { 0 }; i <= 10; ++i)   // runs 11 times: 0,1,...,10
for (int i { 0 }; i <  10; ++i)   // runs 10 times: 0,1,...,9
```

C++ programmers overwhelmingly count with **half-open ranges** — start *inclusive*, end *exclusive*, written `[start, end)`:

```
0 1 2 3 4 5 6 7 8 9 | 10
[  included values  ) excluded boundary
```

The payoff is that the count falls right out of the bounds: `i < count` runs exactly `count` times starting from 0. It also matches how the rest of C++ works — `begin`/`end` ranges, container sizes, string lengths — so half-open counting will feel native everywhere you go.

> **Best practice:** Write `i < count`, not `i <= count - 1`. The half-open form is the C++ idiom; it reads cleanly and the iteration count is obvious at a glance.

### Omitted expressions

Any of the three header pieces can be left blank. Omit them all and you get a deliberate infinite loop:

```cpp
for (;;)
{
    // loops forever unless the body breaks or returns
}
```

That's a legitimate alternative to `while (true)`. But omitting the *init* or *end* pieces while keeping the loop counted usually just hurts readability — keep the three parts together when they belong together.

### Multiple counters

A `for` header can manage more than one counter using the comma operator, which is handy for scanning from both ends toward the middle:

```cpp
for (int left { 0 }, right { 9 }; left < right; ++left, --right)
{
    std::cout << left << ' ' << right << '\n';
}
```

It's a neat tool, but it gets clever fast — reserve it for cases where two counters genuinely move in lockstep.

### Keep the counter inside the loop

Declare the loop variable in the `for` header, not before it. That confines its scope to the loop, so it can't be accidentally read or reused afterward:

```cpp
for (int i { 0 }; i < maxTests; ++i)   // i lives only inside the loop — good
{
    runTest(i);
}
```

The lab's capstone, `playRound`, is a `for` loop over the characters of a guess string — bounded iteration with the counter scoped to the loop, exactly this pattern.

---

## 8.11 — Break and continue

Loops have two more tools for steering from *inside* the body: one to leave the loop entirely, one to skip to the next pass.

### `break`

A **break statement** immediately exits the nearest enclosing loop (or switch). Execution resumes at the first statement *after* the loop:

```cpp
for (int i { 0 }; i < maxTests; ++i)
{
    if (failureCount >= maxFailures)
        break;          // we've seen enough failures — stop looping

    runOneTest();
}
// execution continues here after the break
```

In a `switch`, you already know `break` as the thing that prevents fallthrough — it's the same statement doing the same job: get me out of this construct.

### `break` versus `return`

These are easy to confuse, so be precise: `break` exits the *loop* and the function keeps going; `return` exits the *whole function* and the loop goes with it.

```cpp
for (...)
{
    if (done)
        break;      // leave the loop, then continue this function
}
cleanUp();          // <-- break lands here

// vs.

for (...)
{
    if (fatal)
        return;     // leave the function entirely; cleanUp() below is skipped
}
cleanUp();
```

The lab's `playRound` uses both flavors of exit deliberately: `break` to stop looping when the attempt budget runs out, and `return attempts` to leave the moment a guess is correct.

### `continue`

A **continue statement** skips the rest of the current iteration and jumps to the loop's next pass. It's the tool for "this one doesn't qualify — move on":

```cpp
for (int i { 0 }; i < count; ++i)
{
    if (!isValid(i))
        continue;       // skip the invalid ones

    process(i);         // only reached for valid i
}
```

One caution worth internalizing: in a `for` loop, `continue` still runs the end-expression before the next test, so your counter advances normally. (In a `while` loop, `continue` jumps straight back to the condition — so if your counter lives in the body *after* the `continue`, you can skip the increment and spin forever. Another reason `for` is the safer counting loop.)

The lab leans on `continue` for its sentinel: a guess of `0` is a "misclick" that should be *skipped without counting* — exactly what `continue` expresses, jumping to the next character without spending an attempt.

### Using them well

`break` and `continue` are at their best when they make an exit or skip condition *explicit* and keep the main path readable — a guard at the top of the body that handles the odd case and gets out of the way. They're at their worst when a long loop hides many scattered `break`s, so a reader can't tell when or why the loop stops.

> **Best practice:** Short loops with one or two clear `break`/`continue` conditions are great. If a loop grows large and tangled with exits, that's a signal to extract part of it into a helper function.

---

## 8.12 — Halts (exiting your program early)

### Returning versus halting

Normally a function ends by *returning* to whoever called it, unwinding the call stack one frame at a time, and `main` returning is how a healthy program ends:

```cpp
return 0;   // main returns -> program ends normally
```

A **halt** is different: it's a control-flow statement that terminates the program *directly*, without the orderly step-by-step return back up through every caller. C++ gives you a few, in the `<cstdlib>` header.

### `std::exit`

`std::exit()` ends the program immediately and hands a status code to the operating system (0 conventionally means success, nonzero means trouble):

```cpp
#include <cstdlib>

std::exit(1);   // stop now, tell the OS "something went wrong"
```

There's an important catch. Because `std::exit` doesn't unwind the stack the normal way, **destructors for your local objects may not run** — the careful cleanup that ordinarily happens as each scope exits can be skipped:

```
normal return:                std::exit:
  leave scope                   terminate the process directly
  -> destroy local objects      -> local-scope cleanup can be skipped
  -> caller resumes
```

That makes `std::exit` a tool for genuine "stop the whole program" moments, not a casual way to leave a function. To leave a function, `return`; to leave a loop, `break`.

### `std::atexit`

You can register a function to run *when* `std::exit` is called, using `std::atexit`:

```cpp
std::atexit(cleanup);   // cleanup() runs during std::exit
```

This is niche. Reach for ordinary control flow first; `std::atexit` is for the rare case where you truly need process-level cleanup wired to exit.

### `std::abort` and `std::terminate`

`std::abort()` and `std::terminate()` end the program *abnormally* — no cleanup, no status of your choosing, just an immediate stop. They're for severe, unrecoverable failures, not for ordinary input validation. You'll meet `std::abort` again next chapter: a failed `assert` calls it, which is precisely the point — an assertion that fails means a bug, and stopping hard is the safest response.

### When to halt

Use a halt for an unrecoverable, process-level failure where continuing would be worse than stopping. For ordinary errors inside a function — bad input, a missing file — prefer returning an error value or (later) throwing an exception, so the *caller* gets to decide what to do. A function that halts on bad input takes that choice away from everyone above it.

---

## 8.13 — Introduction to random number generation

### Randomness on a deterministic machine

Here's a puzzle: a computer follows its instructions exactly, the same way every time. So how does it produce a *random* number — a dice roll, a shuffled deck, an unpredictable enemy? The honest answer is that it usually doesn't, not truly. Instead it runs an algorithm that produces a sequence of numbers that *look* random, even though each one is completely determined by the last. That algorithm is a **pseudo-random number generator**, or **PRNG**.

A PRNG holds some internal **state**. Each time you ask for a number, it transforms its state into an output and updates the state for next time:

```
seed -> state -> number -> new state -> number -> new state -> ...
```

### The seed, and reproducibility

The **seed** is the value that sets the PRNG's *initial* state. And here's the consequence that defines everything about using a PRNG: **the same seed produces the same sequence**, every single time.

```
seed 123 -> 8, 42, 19, 7, ...
seed 123 -> 8, 42, 19, 7, ...   (identical run)
```

That determinism cuts both ways. It's wonderful for debugging — a fixed seed means a bug reproduces on demand, so you can chase it with a stable target instead of a moving one. It's terrible for unpredictability — a game that always deals the same hand isn't much of a game. Choosing your seed is choosing which of those you want.

### One generator, reused

Because each number advances the state, you must **create the generator once and reuse it**. The classic mistake is rebuilding it inside a loop, which restarts the same sequence on every pass:

```cpp
// WRONG: a fresh generator every iteration -> same first value every time
for (int i { 0 }; i < 10; ++i)
{
    std::mt19937 rng { 12345 };   // re-seeded to the same start, over and over
    std::cout << roll(rng) << '\n';
}
```

```cpp
// RIGHT: one generator, created once; its state advances across iterations
std::mt19937 rng { 12345 };
for (int i { 0 }; i < 10; ++i)
{
    std::cout << roll(rng) << '\n';
}
```

This is the most common randomness bug there is, and recognizing it — "why do I keep getting the same number?" — will save you real time.

### What makes a PRNG good

Not all PRNGs are equal. A good general-purpose one has a long **period** (it runs a very long time before the sequence repeats), a good **distribution** (values are spread evenly), and low correlation between successive outputs, all at acceptable speed. A weak generator betrays itself with visible patterns. (Note that "good for games and simulations" is a different, lower bar than "cryptographically secure" — for security-sensitive randomness you need dedicated tools, not a general PRNG.)

### The C++ toolkit: `<random>`

Modern C++ does randomness through the `<random>` header, and it's worth understanding its design, because it splits the job into three cooperating pieces:

| Piece | Role | Example |
|---|---|---|
| Engine | Produces raw pseudo-random bits | `std::mt19937` |
| Distribution | Shapes those bits into a useful range | `std::uniform_int_distribution` |
| Seed source | Sets the engine's starting state | a literal, the clock, `std::random_device` |

```
seed -> engine -> raw random bits -> distribution -> a value in your range
```

The separation is the point: the *engine* knows how to make random bits but not what range you want; the *distribution* knows how to map bits onto `1..6` (or any range) without bias. You combine them. C++'s older C-style `std::rand()` blurs these jobs together and has real flaws — prefer `<random>` for new code.

---

## 8.14 — Generating random numbers using Mersenne Twister

### The Mersenne Twister engine

The `<random>` engine you'll reach for most often is the **Mersenne Twister**, available in two widths:

```cpp
std::mt19937      // 32-bit Mersenne Twister
std::mt19937_64   // 64-bit Mersenne Twister
```

It has a long period and good statistical properties — an excellent default for games, simulations, and the lab's number-guessing driver.

### A dice roll, start to finish

Here's the whole pattern in one program: include `<random>`, make an engine, make a distribution, and call the distribution *with* the engine to get a value:

```cpp
#include <iostream>
#include <random>

int main()
{
    std::mt19937 rng { 12345 };                 // engine, fixed seed (reproducible)
    std::uniform_int_distribution die { 1, 6 }; // maps engine output to 1..6

    std::cout << die(rng) << '\n';              // a roll
    std::cout << die(rng) << '\n';              // another, state advances
}
```

The piece people get backwards is the call: it's `die(rng)`, not `rng()` alone. You're asking the *distribution* for a number, and handing it the engine to draw bits from. The `{ 1, 6 }` range is **inclusive on both ends** — `1` and `6` are both possible.

### Fixed seed versus variable seed

You have a real choice in how to seed, and it's the reproducibility trade-off from the last lesson made concrete.

A **fixed seed** gives the same sequence on every run:

```cpp
std::mt19937 rng { 12345 };
```

That's reproducible and ideal for debugging — the same "random" run, every time, so a bug stays put. The cost is that it's the same every time, which is no good for an actual game.

A **variable seed** draws from `std::random_device`, a source of (typically) genuine system entropy, so each run differs:

```cpp
std::random_device rd;
std::mt19937 rng { rd() };
```

That gives real unpredictability across runs. The cost is reproducibility: if a bug only appears for certain random sequences, you can't rerun it — *unless* you log the seed.

### Seeding from the clock

Another common variable seed is the current time, which differs run to run:

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

It changes each run, though it's lower-quality entropy than `std::random_device` and two runs started in the same instant could collide.

### Seed once

The same warning as before, stated as a rule because it matters that much: **seed the engine once**, then reuse it. Do not reconstruct or reseed it on each number:

```cpp
// WRONG: reseeding every iteration
for (int i { 0 }; i < 100; ++i)
{
    std::mt19937 rng { std::random_device{}() };  // expensive setup, repeated
    std::cout << dist(rng) << '\n';
}
```

```cpp
// RIGHT: seed once, reuse
std::mt19937 rng { std::random_device{}() };
for (int i { 0 }; i < 100; ++i)
{
    std::cout << dist(rng) << '\n';
}
```

> **Tip:** When you find a bug or an interesting result with a variable seed, *print the seed* before you use it. A logged seed turns an unreproducible one-in-a-thousand event into something you can replay exactly — the single best debugging habit there is for randomized code.

### A field guide to random bugs

Two symptoms cover most of what goes wrong:

- **Same sequence every run** — almost always a fixed seed (or a forgotten default construction). Switch to a variable seed if you wanted variety.
- **Same number over and over** — the generator is being recreated or reseeded inside the loop, or your distribution's range is tiny. Seed once, outside the loop.

---

## 8.15 — Global random numbers (Random.h)

### The problem a helper solves

Once several functions all need random numbers, the engine becomes awkward to manage. Passing it as a parameter through every function that needs it is verbose; creating a *new* engine in each function is the bug we just spent two lessons warning against. What you want is one shared engine that everyone can reach.

A common pattern is a small header — conventionally called `Random.h` — that wraps a single shared engine in a namespace and exposes a tidy helper:

```cpp
#include <random>

namespace Random
{
    inline std::mt19937 mt { std::random_device{}() };

    inline int get(int min, int max)
    {
        std::uniform_int_distribution dist { min, max };
        return dist(mt);
    }
}
```

Now any function can ask for a number without ever touching an engine directly, and they all share the *same* generator, so its state advances naturally across calls:

```cpp
int index { Random::get(0, 9) };
char letter { static_cast<char>('a' + Random::get(0, 25)) };
```

(The `inline` keyword here lets the shared engine live in a header that multiple `.cpp` files include without violating the one-definition rule — the linkage idea from the previous chapter, put to work.)

### The cost of global state

This is convenient, and convenience has a price worth naming. Global state — which is what `Random::mt` is — is easy to reach from anywhere, but that's exactly its weakness: a function that secretly depends on a global has a *hidden* input, which makes it harder to reason about and harder to test in isolation (two tests sharing one generator can interfere with each other).

For a small program, a game, or a learning project, a controlled random namespace like this is a perfectly reasonable trade. In larger systems you'd more often pass the engine explicitly or inject it as a dependency, so each function's inputs are visible in its signature. Knowing the trade-off is the point — there's no single right answer, only the right answer for the size of the program.

> **Note:** This same tension — "shared convenience versus visible, testable inputs" — is exactly why the lab keeps `<random>` out of its graded engine. The random rolls live in `main`; the decision logic is pure, takes its inputs as arguments, and returns a value. That split is what lets a grader pin the logic to exact answers, and it's the chapter's quiet thesis: keep the unpredictable part small and at the edges, so the part you need to trust stays deterministic.

---

## 8.x — Chapter 8 summary and quiz

### The big picture

Control flow is how a program stops being a straight line. You now have the whole toolkit:

- **Branches** decide *which* code runs. `if`/`else if`/`else` for ranges, strings, and compound conditions; `switch` for one integral value against a list of constants.
- **Loops** decide *how many times* code runs. `while` tests first (can run zero times); `do while` runs first then tests (at least once); `for` packages counted iteration into one tidy header.
- **Jumps** steer from inside. `break` leaves a loop or switch; `continue` skips to the next iteration; `return` leaves the whole function.
- **Halts** stop the program. `std::exit` for deliberate process termination (may skip cleanup); `std::abort` for abnormal failure.
- **Randomness** comes from `<random>`: an engine (`std::mt19937`) for raw bits, a distribution for the range, seeded *once* and reused.

### Habits worth keeping

- Brace every `if`, `else`, and loop body — even one-liners.
- Use `==` to compare, `=` to assign; let compiler warnings catch the slip.
- End every `switch` case with `break` (or `return`); mark deliberate fallthrough with `[[fallthrough]]`; brace any case that declares a variable.
- Give every loop a believable exit story; prefer signed loop counters.
- Count with half-open ranges: `i < count`, not `i <= count - 1`.
- Avoid `goto`; you have `break`, `continue`, `return`, and helper functions.
- Seed a PRNG once, reuse the engine, and log the seed when you need to reproduce a result.

### The lab ahead: a number-guessing engine

This chapter's project is a number-guessing game, built the way a real codebase would build it: the **decision logic** (judge a guess, label a hint, sum digits, build a countdown, validate input, play a round) lives in a *pure, deterministic engine* you can test exactly; the **randomness and keyboard I/O** live in a provided `main` that you don't grade. You'll write six small functions, each driven by a different control-flow tool from this chapter:

- `judgeGuess` — an `if`/`else if`/`else` **chain** returning a direction code (`-1`/`0`/`+1`).
- `hintText` — a `switch` mapping that code to text, with a `default` for bad input.
- `sumOfDigits` — a loop using `% 10` and `/= 10` to peel decimal digits.
- `countdownString` — a `while` that *builds up* a string, with separators only *between* counts.
- `promptsUntilValid` — a `do while`, because you must read a value before you can test it.
- `playRound` — the capstone `for` loop with a `continue` sentinel and a `break` budget.

The whole design is deliberate: by keeping `<random>` and `std::cin` in `main` and the logic pure, the engine's answers are fixed and checkable. That's not just a lab convenience — it's how you'd make any decision-heavy program (a fuzzer's mutation choices, a game's AI) testable: keep the unpredictable part at the edges, and the part you must trust deterministic at the core. Turn the grader's wall of red into one green line, and you'll have used every tool in this chapter at least once.

### Mini drill

Before the lab, here's a tiny program that exercises the core ideas together — one engine seeded once, a `for` loop for bounded iteration, two distributions, and the careful casts where signed loop math meets a string's `size_t`:

```cpp
#include <iostream>
#include <random>
#include <string>

int main()
{
    std::mt19937 rng { 12345 };                         // seed once
    std::uniform_int_distribution charDist { 0, 25 };   // a..z

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

It picks a random position and a random letter each pass and overwrites one character, mutating the *previous* result rather than starting fresh — a counted loop, a reused engine, and deliberate casts at the boundary between signed loop counters and unsigned string indices. That's the chapter in miniature.
