# Chapter 3 — Debugging C++ Programs

> Source: <https://www.learncpp.com/> (Chapter 3)

Sooner or later — usually sooner — you will write a program that does not do what you meant. It might refuse to compile. It might compile, run, and then crash. Or, most insidiously, it might run quietly to completion and hand you a confidently wrong answer. Every working programmer spends a large fraction of their time not *writing* code but figuring out *why code misbehaves*. That skill has a name: **debugging**.

This chapter is about building a debugging mindset before you need it under pressure. The central idea is simple to state and surprisingly hard to practice: a bug is just a place where **reality diverges from your expectation**, and your job is to find the *first* point where that divergence happens. Everything else — print statements, assertions, breakpoints, watches, the call stack — is a tool in service of that one question.

We will start by naming the different ways a program can be wrong, because the kind of wrongness tells you which tool to reach for. Then we will turn debugging from a panicky guessing game into a disciplined, repeatable process.

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

Before you can fix a bug, you have to know what *kind* of bug it is. Programs fail in categorically different ways, and each category is caught at a different moment in the program's life — some by the compiler before the program ever runs, others only while it executes. Learning to name the kind of wrongness precisely is the first debugging skill, because it tells you where to look and which tool will help.

There are four families worth knowing: **syntax errors**, **compile-time semantic errors**, **runtime errors**, and **logical errors**. The first two are caught by the compiler. The last two are not — and that gap is exactly why this chapter exists.

### Syntax errors

A **syntax error** violates the *grammar* of C++. Just as English grammar dictates that a sentence needs certain pieces in a certain order, C++ has rules about how statements must be structured. When you break one, the compiler cannot even parse what you wrote — it cannot figure out what statement you intended — and it refuses to continue.

```cpp
#include <iostream>

int main()
{
    std::cout << "hello\n" // missing semicolon
    return 0;
}
```

A statement in C++ ends with a semicolon. Without it, the compiler runs the two lines together and effectively sees:

```text
std::cout << "hello\n" return 0;
```

That is not a valid statement, so compilation fails. The good news about syntax errors is that the compiler catches *all* of them, and it points (roughly) at where the grammar broke. Common offenders:

- a missing semicolon
- a missing closing brace `}` or parenthesis `)`
- a misspelled keyword (`itn` instead of `int`)
- invalid punctuation
- a malformed string literal (a missing closing quote)

Here is another:

```cpp
int main(
{
    return 0;
}
```

The function header should read `int main()`. The lone `(` is never closed, so the grammar is broken and the program will not compile.

> **Key insight:** Syntax errors are the friendliest bugs you will meet. The compiler finds every one of them for you and refuses to produce a program until they are gone. The errors that survive compilation are the ones you have to hunt yourself.

### Compile-time semantic errors

A **semantic error** is subtler. The *grammar* is fine — the compiler can parse your statement — but its *meaning* is invalid. Think of a sentence like "The number seven tastes purple." It is grammatically well-formed, yet it means nothing sensible. C++ has analogous meaning-rules, and the compiler checks many of them.

```cpp
int main()
{
    int x { "hello" }; // cannot initialize an int from a string
    return 0;
}
```

The shape is a perfectly legal variable definition:

```text
type name { initializer };
```

But the *meaning* is wrong: `"hello"` is a string literal, not an integer, and there is no sensible way to store it in an `int`. The compiler rejects it. A few more meaning-violations the compiler will catch:

```cpp
int x {};
x(); // x is an int, not a function — you can't call it
```

```cpp
const int limit { 10 };
limit = 20; // limit is const; you promised never to change it
```

```cpp
int add(int a, int b)
{
    return a + b;
}

int main()
{
    add(1); // add needs two arguments; you supplied one
}
```

Each of these is grammatically valid but semantically nonsensical, and because the compiler can detect the contradiction *before the program runs*, we call them **compile-time** semantic errors. Like syntax errors, they cost you a failed compile but no mystery — the compiler tells you what it objected to.

### Logical errors

Now the difficulty rises. A **logical error** is code that compiles cleanly *and* runs without crashing — and still produces the wrong result. The grammar is fine, the meaning is well-formed C++, but the program does not do what *you* intended.

```cpp
int area(int width, int height)
{
    return width + height; // compiles fine — but area should multiply!
}
```

This is valid C++ in every technical sense. `width + height` is a legal expression that yields an `int`. The compiler has no way to know you meant `width * height` — it cannot read your mind, only your code. So it compiles the function, runs it, and cheerfully returns the perimeter-ish sum when you asked for an area.

Logical errors are the hardest of the four families precisely because *no tool flags them for you*. They require reasoning about **intent**: you have to compare what the program *does* against what you *meant* it to do. That comparison is the heart of debugging, and the rest of this chapter is largely about doing it efficiently.

> **Warning:** A clean compile proves only that your program is grammatically and semantically well-formed C++. It says *nothing* about whether the program is correct. "It compiles" and "it works" are entirely different claims.

### Runtime errors

A **runtime error** occurs while the program is executing. The code compiled fine, but some particular input or situation drives it into trouble — often **undefined behavior** or an outright crash.

```cpp
int divide(int a, int b)
{
    return a / b;
}

int main()
{
    return divide(10, 0); // division by zero — boom, at runtime
}
```

Dividing by zero is not something the compiler can reliably forbid (the divisor is often computed at runtime), so this program builds without complaint and then misbehaves only when `b` happens to be `0`. Other common runtime failures:

- dividing by zero
- reading or writing past the end of an array (an **out-of-bounds** access)
- dereferencing a null pointer
- using a **dangling** pointer or reference (one whose target no longer exists)
- a stack overflow from recursion that never reaches its base case
- a file that fails to open, when the program assumed it would succeed

The defining trait is that the trigger lives in the *data*, not the *text*: the same code is fine on most inputs and fails on a specific one.

### Putting the taxonomy together

It helps to see the four families as a pipeline. Your program text flows through the compiler and then into execution, and each stage can reject it for a different reason:

```text
program text
    |
    v
compiler parses grammar
    |
    +-- syntax error: not valid C++ structure          (compile fails)
    |
    v
compiler checks meaning
    |
    +-- compile-time semantic error: valid shape,       (compile fails)
    |   invalid meaning
    |
    v
program runs
    |
    +-- runtime error: crash / undefined behavior       (runs, then breaks)
    |   on the actual execution
    |
    +-- logical error: program completes but the         (runs, wrong answer)
        answer is wrong
```

The two errors above the "program runs" line are caught for you by the compiler. The two below it are yours to find. Notice that as you move down the list, the feedback gets quieter — from "won't compile, here's the line" all the way down to "ran perfectly, gave you a wrong number with a straight face."

This distinction is not academic; it maps directly onto how software is tested:

- **compilation** catches syntax errors and many static semantic errors;
- **tests** catch some runtime and logical errors by checking outputs against expectations;
- **sanitizers** (which we will meet in 3.10) catch certain runtime memory and undefined-behavior bugs;
- **fuzzers** search for the specific inputs that trigger runtime failures.

The chapter lab — the **Coin-Tray Auditor** — lives entirely in the bottom-right box. Every one of its six planted bugs is a *logical* error: the program compiles cleanly, runs without crashing, and prints the wrong totals. That is deliberate. It forces you to debug the way the compiler can't help with — by comparing expected output against actual, function by function.

## 3.2 — The debugging process

Debugging is the process of finding and fixing **defects** — the actual flaws in your code — as opposed to merely making **symptoms** go away. That distinction is the single most important idea in this chapter, so let us put it front and center:

```text
symptom != root cause
```

A **symptom** is what you *observe*: the program crashed, the output is wrong, a test failed, a variable held a surprising value, an output file came out empty. A symptom is real and useful, but it is rarely where the bug lives. The **root cause** is the *earliest incorrect thing* that explains the symptom — the first domino. Fix a symptom and the bug often reappears somewhere else; fix the root cause and it is gone for good.

> **Key insight:** The line that crashes is usually a *victim*, not the culprit. A null-pointer dereference is the symptom; the real bug is wherever that pointer was supposed to be set and wasn't. Chase the cause upstream, not the symptom where it happens to surface.

### The debugging loop

Effective debugging follows a repeatable loop. When you are stuck, it is almost always because you skipped one of these steps:

```text
1. Reproduce the problem
2. Characterize what is wrong
3. Localize where behavior first diverges
4. Identify the root cause
5. Make the smallest correct fix
6. Re-test the original failure
7. Add or update tests if useful
```

We will walk through each step. The discipline matters more than it looks: skipping reproduction, in particular, is how people spend an afternoon "fixing" a bug they never actually triggered.

### Reproduce

You cannot reliably fix what you cannot reliably trigger. If a bug appears only sometimes — and you do not know *when* — then after you "fix" it you have no way to tell whether the fix worked or the bug simply chose not to show up this time. So the first move is always to find a recipe that makes the failure happen on demand.

A good reproduction is concrete and complete:

```text
Run:
  ./fuzzer seed.txt 100

Observed:
  crashes after 17 iterations

Expected:
  either complete 100 iterations, or report a handled target crash
```

A bad reproduction is a shrug:

```text
Sometimes it breaks.
```

The difference is everything. To make a reproduction solid, write down the details that affect the outcome: the exact command, the exact input, the current working directory, any relevant environment variables, the build mode (debug vs. optimized), the expected output, the actual output, and whether the behavior is deterministic or varies between runs.

> **Best practice:** Never start changing code before you can make the bug happen on command. A reproduction you can run in one line is your test for whether the fix actually worked.

### Characterize

Before touching any code, state the bug in a **falsifiable** way — a claim precise enough that you could prove it true or false by running one test. The form to aim for is "expected X, got Y":

```text
Expected: countBranches("a && b") returns 2.
Actual:   countBranches("a && b") returns 1.
```

Compare that to the vague version:

```text
Branch counting is broken.
```

The precise statement does two jobs at once. It tells you *exactly* what to inspect (the path that turns the input `"a && b"` into a count), and it gives you an unambiguous test for success: when `countBranches("a && b")` returns `2`, you are done. The vague version tells you neither.

This is exactly the shape the lab's bug report uses — each ticket reads `coinWorth(3, kQuarter)` expected `75`, got `28`. That is not a coincidence; a good bug report is already half a debugging session, because someone has done the characterize step for you.

### Localize

**Localizing** means narrowing the bug down to a smaller and smaller region of code until it has nowhere left to hide. You do this by asking pointed yes/no questions about the program's state at various points and checking which side of each checkpoint still looks correct:

- Is the input parsed correctly?
- Is this loop executing the number of times I expect?
- Does this value go wrong *before* or *after* that function call?
- Is the object still valid right before this pointer is dereferenced?
- Did the file actually open?

Picture the program as a pipeline of stages, each transforming the state from the one before:

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

Find the first arrow where good state turns into bad state.
```

That "first arrow" is your bug. Everything upstream of it is innocent; everything downstream is just inheriting the damage.

### Identify the root cause and fix

Once you have localized the bug to a small region, find the precise root cause — and then make the **smallest change that corrects it**. Resist the urge to rewrite.

The trap here has a name: **debugging by demolition**. You see a bug somewhere in a module, lose patience, and rewrite the whole module from scratch:

```text
Bug in one branch condition
    |
    v
rewrite the entire module
```

This feels productive but it is usually a mistake. The rewrite throws away all the parts that were *working*, and it introduces a fresh batch of unknowns — now you have to re-verify everything, and any new bug you introduce is tangled up with the old one. Prefer the surgical alternative:

```text
Bug in one branch condition
    |
    v
correct just that condition
    |
    v
add a test for that boundary
```

> **Best practice:** The first fix should be the smallest change that addresses the root cause — ideally a one-token or one-line edit. The lab is built around this rule: each of its six bugs is a single wrong operator, a single swapped operand, a single bad constant. Big rewrites are forbidden, not as a stylistic preference but because they destroy your ability to reason about cause and effect.

### Retest

After fixing, run the *original failing case* again and confirm it now passes. This is the payoff for having a solid reproduction back in step one — you have an exact test for "is it fixed?"

But do not stop at the one case. A good fix is one you have probed from a few angles, so check the cases most likely to reveal a sloppy patch:

- the failing input (it should now pass)
- the smallest possible input
- the empty input
- inputs right at a boundary
- a normal, representative input
- one input that exercises each branch

This habit — testing the boundaries and the neighbors, not just the one case you fixed — is the bridge from ad-hoc debugging to systematic testing, which is where this course is headed.

## 3.3 — A strategy for debugging

The previous section gave you a *process*; this section gives you the *strategy* that powers its core step. Underneath all the tooling, debugging reduces to answering one question:

```text
At what exact point does the program first do something
different from what I thought it would do?
```

Every technique below — expectations, checkpoints, binary search, simplification — is a different way of pinning down that **first divergence**. Find it, and the bug is usually obvious. Skip it, and you are reduced to guessing.

### Start with an expectation

You cannot detect a divergence between expected and actual behavior if you never wrote down what you *expected*. So before you step through code or sprinkle in print statements, commit to a prediction.

```cpp
int doubleValue(int x)
{
    return x * x; // bug: this squares instead of doubling
}
```

For the input `4`, write down the expectation *first*:

```text
expected return: 8
actual return:   16
```

The mismatch tells you the function is wrong. Now look at the body with your expectation in hand:

```text
expected operation: x + x, or x * 2
actual operation:   x * x
```

Side by side, the bug is unmissable: `*` should be `+` (or the second operand should be `2`). Writing the expectation down is what makes the bug *jump out* — without it, `x * x` looks like a perfectly reasonable line of code.

### Trace from known-good to known-bad

When a program has several stages, you rarely need to inspect all of them. Instead, find one checkpoint you can confirm is **correct** and one you can confirm is **wrong** — the bug is trapped between them.

```text
input string is correct      known good
parsed tokens are correct    known good
mutation list is wrong       known bad
```

The root cause lies somewhere between "tokens are correct" and "mutation list is wrong":

```text
input -> parse -> tokens -> build mutations -> run
                  good          bad
```

Now you can ignore parsing entirely and focus inside `build mutations`. This is exactly how the lab is structured: each function (`coinWorth`, `trayValue`, `changeOwed`, …) is a stage you can confirm in isolation. Once you have verified that `coinWorth` is correct, you get to *trust* it and move on — the bug must be in a function you have not yet cleared.

### Binary search the execution path

When the suspect region is still large, do not scan it linearly from the top. Check the **middle**:

```text
start --------------------------- end
 good                            bad
                 |
                 v
             check the middle
```

If the state is still good at the midpoint, the bug is in the second half. If it is already bad at the midpoint, the bug is in the first half. Either way, one check has eliminated half of the remaining code. Repeat, and a region of a thousand lines collapses to a handful in about ten checks. This **binary search** is the most powerful localization technique you have, and it works whether you are bisecting lines of execution, commits in version control, or elements of an input.

### Simplify the failing case

A smaller failing input is dramatically easier to reason about than a large one. If your program crashes on a 10,000-character input, your first job is often not to debug the crash but to *shrink the input* until it is as small as possible while still failing:

```text
program fails on a 10,000-character seed
        |
        v   (cut the input down, keep it failing)
program fails on "%%"
```

A two-character failing case fits in your head; a ten-thousand-character one does not. This idea — **minimization** — recurs throughout testing and fuzzing: a minimal crashing input is far more valuable than a giant one, because it points almost directly at the cause.

### Avoid guessing edits

Finally, the anti-pattern to unlearn. Under pressure, it is tempting to fall into the guess-and-check loop:

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

This rarely converges, and it often makes things *worse* — each blind edit can introduce a new bug, so now you are debugging a tangle of your own making. Replace it with the disciplined loop:

```text
observe
hypothesize
test the hypothesis
make one targeted change
verify
```

> **Key insight:** Debugging is *investigation*, not random mutation of your own source code. Let the evidence tell you where to work. Every edit you make should be answering a specific question you have already asked — not a hope that *this* time it compiles into something that works.

## 3.4 — Basic debugging tactics

You now have a process and a strategy. This section covers the simplest *tools* for carrying them out — the ones that need nothing but a text editor and a recompile. They are humble, but they are also the tools you will reach for most often, because they work everywhere: on any platform, in any build, even on a remote machine where you cannot run a debugger. The core set is comments, print statements, temporary checks, narrowing code regions, and comparing expected against actual values.

### Commenting out code

One of the fastest ways to localize a bug is to *temporarily disable* part of the program and see whether the symptom goes away. If it does, the bug was in the disabled part; if it does not, the bug is elsewhere — and you have just cleared a whole region.

```cpp
runParser(input);
// runMutation(input);
// runTarget(input);
```

If the crash vanishes when `runMutation` and `runTarget` are commented out, then `runParser` is probably innocent, and your attention belongs downstream. This is the known-good / known-bad split from 3.3, done with the bluntest possible instrument.

A few cautions, because commenting-out is easy to abuse:

- Comment out one *coherent* region at a time, so you can interpret the result.
- Do not leave random commented-out code sitting in finished work — it confuses the next reader (often future-you).
- For keeping a record of old code, prefer **version control** over commented-out blocks. Git remembers; comments just clutter.

### Print debugging

The workhorse. A **print statement** dumps a piece of runtime state to the screen so you can see what the program is actually doing, rather than what you assume it is doing.

```cpp
std::cerr << "seed.size() = " << seed.size() << '\n';
std::cerr << "index = " << index << '\n';
```

Notice the stream: `std::cerr`, not `std::cout`. This matters. `std::cout` is *standard output* — the program's real results, the thing a user or a grader reads. `std::cerr` is *standard error* — a separate channel meant for diagnostics. Sending debug noise to `std::cerr` keeps it from contaminating the program's actual output, and it lets you redirect or discard the two streams independently.

> **Best practice:** Send debugging output to `std::cerr`, never `std::cout`. Your diagnostics then stay separate from the program's real results, so a script (or a grader) reading the output is not tripped up by your debug noise.

The *quality* of a print matters as much as its placement. A good debug print carries enough context to interpret on its own:

```cpp
std::cerr << "[mutate] index=" << index
          << " size=" << seed.size()
          << " seed='" << seed << "'\n";
```

A poor one does not:

```cpp
std::cout << "here\n";
```

"here" tells you the line executed and nothing else — not which iteration, not what the values were, not what the program thought was true at that moment. A tag and a few labeled values turn a print from a faint heartbeat into an actual report. (And note the second example uses `std::cout`, compounding the problem.)

### Put prints around the suspected transition

The most informative place for a print is *bracketing* the operation you suspect — one before, one after — so you can see exactly what the operation did to the state:

```cpp
std::cerr << "before mutate: " << seed << '\n';
mutate(seed);
std::cerr << "after mutate:  " << seed << '\n';
```

This directly answers the question "did `mutate` change the value the way I expected?" If `before` is right and `after` is wrong, the bug is inside `mutate`. If `before` is *already* wrong, the bug is upstream and `mutate` is innocent. Either way, you have located which side of one arrow the divergence lives on — the whole game from 3.3, in two lines.

### Temporary checks

Sometimes you do not just want to *see* a value; you want to *assert* something about it and shout if the assumption is violated. A temporary check turns a silent assumption into a loud, precise report:

```cpp
if (index >= seed.size())
{
    std::cerr << "bad index: " << index
              << " size: " << seed.size() << '\n';
    return;
}
```

Without this, an out-of-bounds index might cause a mysterious crash several steps later, or — worse — silently corrupt data and produce a wrong answer with no crash at all. With it, the moment the assumption breaks, you get a clear message naming exactly what went wrong and where.

### Keep debug code easy to remove

Temporary diagnostics are temporary. Make them obvious so you can find and delete them later:

```cpp
// DEBUG
std::cerr << "value=" << value << '\n';
```

> **Tip:** Mark throwaway diagnostics with an unmistakable comment like `// DEBUG` and keep them localized. Before you submit or commit, strip the noisy ones out — unless a particular message is genuinely part of your program's error reporting, in which case it has earned its place.

## 3.5 — More debugging tactics

Print debugging works, but it does not scale gracefully. As a program grows, a forest of raw `std::cerr` statements becomes its own problem: the output is overwhelming, and every time you want to debug a different area you are editing source, recompiling, and re-deleting prints. This section covers tactics that keep diagnostics under control — ways to turn them on and off, to centralize their formatting, and to encode your assumptions directly into the code.

### Conditional debug output

The simplest improvement is a flag that gates your diagnostics, so you can silence them without deleting them:

```cpp
bool debug { true };

if (debug)
{
    std::cerr << "round=" << round << " seed=" << seed << '\n';
}
```

Flip `debug` to `false` and every guarded print goes quiet — no editing, no recompiling away your hard-won instrumentation. When you need the traces back, flip it to `true`. This is a small thing, but it is the difference between diagnostics you maintain and diagnostics you keep rewriting.

### Preprocessor-controlled debug output

You can go one step further and have the **preprocessor** remove debug code from the compiled program entirely, so it imposes zero cost in a normal build:

```cpp
#ifdef DEBUG_MUTATION
std::cerr << "mutating seed: " << seed << '\n';
#endif
```

The block between `#ifdef` and `#endif` is compiled *only* if the macro `DEBUG_MUTATION` is defined. You define it from the command line when you want the diagnostic:

```text
g++ -DDEBUG_MUTATION main.cpp
```

The mental model:

```text
DEBUG_MUTATION defined
    -> the debug statement is compiled into the program

DEBUG_MUTATION not defined
    -> the preprocessor deletes that block before the compiler ever sees it
```

The payoff is that your release build carries none of the debug weight, yet the instrumentation stays in the source, ready to switch back on. The cost is readability: if `#ifdef` blocks sprout everywhere, the code becomes hard to follow. Use the technique deliberately, not reflexively.

### A small logging helper

If you find yourself repeating the same `'[' << tag << "] " << ...` formatting over and over, factor it into a tiny helper. This is just the "don't repeat yourself" instinct from Chapter 2 applied to diagnostics:

```cpp
#include <iostream>
#include <string_view>

void debugLog(std::string_view tag, std::string_view message)
{
    std::cerr << '[' << tag << "] " << message << '\n';
}
```

Now every diagnostic is one consistent line:

```cpp
debugLog("parser", "starting parse");
debugLog("mutator", "generated replacement");
```

A full logging library would add severity levels, timestamps, and the ability to route messages to files. For coursework, a five-line helper like this is usually all the structure you need — and it keeps your output uniform enough to grep through later.

### Assertions as executable assumptions

We cover assertions in depth in a later chapter, but the *debugging idea* belongs here, because an assertion is the natural evolution of the "temporary check" from 3.4 — promoted from a throwaway `if` into a permanent, self-documenting guard.

```cpp
#include <cassert>

void mutate(std::string& seed, std::size_t index)
{
    assert(index < seed.size());
    seed[index] = 'x';
}
```

An `assert` states a condition you believe must *always* be true at that point. If it holds, the program continues silently. If it is ever false, the program halts immediately and tells you which assertion failed and where. In effect, the assertion says:

```text
If this condition is false, my program's assumptions are broken.
Stop now, before the damage spreads.
```

That last part is the value: an assertion catches the bug at the *moment the invariant breaks*, not three function calls later when a corrupted value finally causes a visible crash. It collapses the distance between cause and symptom.

> **Note:** Assertions are for *programmer errors* and internal invariants — things that should be impossible if your code is correct. They are **not** for validating user input. A user typing a bad value is an expected situation you should handle gracefully; an index running past the end of a string you control is a bug, and an assertion is exactly how you want to be told about it.

### When to put down the prints and pick up a debugger

Print debugging has real limits. Every print means editing source, recompiling, running, reading, and eventually deleting. For a stubborn or subtle bug, that loop is slow. A **debugger** lets you inspect a running program without touching its source at all:

- stop at any line without adding a single statement;
- inspect *any* variable without printing it;
- step through execution one statement at a time;
- read the call stack to see how you got here;
- set breakpoints that fire only when a condition is true.

For elusive bugs, a debugger is often far faster than scattering prints and guessing where to put them. The next four sections are a tour of exactly these capabilities.

## 3.6 — Using an integrated debugger: Stepping

A **debugger** is a program that runs *your* program under observation, letting you pause it, inspect it, and advance it one piece at a time. Most modern IDEs bundle a debugger right into the editor — an **integrated debugger** — so you can set breakpoints by clicking in the margin and watch variable values appear next to your code. The specific buttons and menu names differ from one IDE to the next, but the underlying *concepts* are stable across all of them, and those concepts are what you should learn. Treat any tool-specific clicking as a thin skin over the ideas below.

The most fundamental capability is **stepping**: advancing the program a controlled amount and stopping again to look. There are a handful of stepping commands:

| Command          | Meaning                                                          |
| ---------------- | --------------------------------------------------------------- |
| **Step over**    | execute the current line, but do not descend into called functions |
| **Step into**    | execute the current line, descending into the called function   |
| **Step out**     | run until the current function returns, then stop in the caller |
| **Continue/run** | keep running until a breakpoint or the program ends             |
| **Run to cursor**| continue until execution reaches a line you selected            |

### The current execution line

When a debugger pauses, it highlights one line as the **current line** — the next statement *about to* execute. Most debuggers draw an arrow at it:

```text
int main()
{
    int x { 3 };
=>  int y { doubleValue(x) };
    std::cout << y << '\n';
}
```

The arrow's meaning is precise and easy to get wrong:

```text
This statement has NOT executed yet.
```

So at this moment `y` does not yet hold its value — the line that initializes it is queued up, not done. Keep this "the arrow points at what is *about* to happen" rule firmly in mind; it explains a lot of apparent surprises when you start watching variables in 3.8.

### Step over

Suppose the current line is a function call:

```cpp
int y { doubleValue(x) };
```

**Step over** runs the entire call to `doubleValue` as a single unit and stops on the *next* line of the current function. You do not see the inside of `doubleValue`; you just get its result and move on. Reach for step over when:

- you trust the called function to be correct;
- the function is library code you have no interest in tracing;
- you care about the *result* of the call, not its internals.

### Step into

**Step into** is the opposite choice: instead of treating the call as one unit, you descend into it and stop on its first line.

```cpp
int doubleValue(int value)
{
=>  return value * 2;
}
```

Now you are inside `doubleValue`, where you can inspect `value` and watch what the function actually computes. Reach for step into when:

- you *suspect* the called function is where the bug lives;
- you want to confirm the parameter values it received;
- you need to see exactly which function (or which overload) got called.

### Step out

**Step out** finishes the function you are currently inside and drops you back in its caller, right after the call. It is the "I've seen enough in here, get me back up" command. Use it when:

- you stepped *into* a function you actually meant to step over;
- you have learned what you needed from the current function;
- you want to return to the higher-level flow without manually stepping through every remaining line.

### A mental model for stepping

Stepping is really just navigating up and down the chain of function calls:

```text
main()
  |
  +-- step into add()
  |     |
  |     +-- inspect add's internals
  |     |
  |     +-- step out
  |
  +-- back in main(), right after the call to add()
```

Step into goes *down* a level, step out comes *back up*, and step over stays at the current level. With those three moves you can walk through any program at exactly the granularity you want — fast over the parts you trust, slow and careful through the parts you suspect.

## 3.7 — Using an integrated debugger: Running and breakpoints

Stepping from the very first line of `main` every time you debug is tedious, especially when the bug is buried deep in a program that does a lot of work first. A **breakpoint** fixes this. It is a marker you place on a line that tells the debugger:

```text
Run normally — full speed — until execution reaches this line,
then pause and hand control to me.
```

In other words, a breakpoint lets you *skip past* all the code you do not care about and stop exactly where you do.

### Setting a basic breakpoint

Say the suspicious function is `mutate`:

```cpp
std::string mutate(std::string seed)
{
    if (!seed.empty())
        seed[0] = 'x';

    return seed;
}
```

You set a breakpoint on the first line of `mutate` and run the program. Execution proceeds normally until the moment `mutate` is called, then stops:

```text
program starts
  |
  v
runs normally (no stopping)
  |
  v
mutate is called
  |
  v
debugger pauses on the breakpoint
```

Now you can inspect `seed` and see exactly what `mutate` was handed — without having stepped through a single line of the code that ran before it.

### Continue

Once you are paused at a breakpoint, **continue** resumes full-speed execution until the program hits another breakpoint, exits, crashes, or you pause it again. Together, breakpoints and continue let you hop from one point of interest to the next, ignoring everything in between.

### Conditional breakpoints

Here is where breakpoints become genuinely powerful. A function might be called thousands of times and misbehave on exactly *one* of them. Stopping on every call would be unbearable. A **conditional breakpoint** stops *only* when an expression you supply is true:

```text
condition:  seed.size() == 0
```

With that condition attached, the debugger races past every call where `seed` is non-empty and pauses only on the one empty-string case you actually care about:

```text
breakpoint reached
  |
  v
is the condition true?
  |          |
 yes         no
  |          |
pause     keep running
```

This is the interactive cousin of the conditional `if`-check from 3.4 — it isolates a rare case inside a sea of normal ones, without you having to edit and recompile the program.

### Managing breakpoints

Most debuggers let you **disable** a breakpoint without deleting it, **re-enable** it later, or **delete** it outright. This sounds minor but it is not: when you are juggling several suspected regions, being able to silence an old breakpoint temporarily — rather than losing it — saves you from constantly stopping at points you have already cleared.

### Choosing between breakpoints and prints

Breakpoints and print debugging are complementary, not competitors. Each is the right tool for a different situation:

| Tool                     | Best for                                              |
| ------------------------ | ----------------------------------------------------- |
| Print                    | a persistent, scrollable trace you can save and grep  |
| Breakpoint               | interactive inspection at one precise line            |
| Conditional breakpoint   | catching a rare case among many iterations            |
| Watch                    | tracking a variable or expression as you step (next section) |

A print leaves a written record you can study after the fact; a breakpoint lets you poke around live. Use whichever answers the question in front of you.

## 3.8 — Using an integrated debugger: Watching variables

When the debugger is paused, you want to see the program's *state* — the current values of its variables. **Watching** is the umbrella term for inspecting those values, either while the program sits stopped or as you step it forward. Debuggers offer several windows for this:

- **locals** — automatically shows every variable in the current function;
- **watches** — shows specific expressions you choose to track;
- **hover inspection** — hover the cursor over a variable to see its value;
- **memory view** — raw bytes at an address, which becomes useful once you reach pointers.

### Locals

The **locals** window lists the variables in the function you are currently stopped in, with their live values. For:

```cpp
int add(int a, int b)
{
    int result { a + b };
    return result;
}
```

paused just after `result` is computed, the locals view might read:

```text
a       2
b       3
result  5
```

That is a clear, immediate picture of the function's state — no print statements required. One thing to watch for: if a variable shows a wild value like `-858993460` before its initialization line has run, that is not a bug in your data — it is **uninitialized memory**. The variable exists but has not been given a value yet, so it holds whatever garbage was already in that memory. (This is also a vivid argument for always initializing your variables, as Chapter 1 urged.)

### Watches

A **watch** is an expression *you* tell the debugger to evaluate and keep showing. It can be a plain variable:

```text
seed
```

or a computed expression:

```text
seed.size()
index < seed.size()
results.size()
```

Watches earn their keep on **invariants** — conditions you believe should always hold. Suppose you expect `index` to stay within bounds. Watch the expression:

```text
index < seed.size()
```

As you step, that watch reads `true`, `true`, `true` … and the instant it flips to `false`, you have caught the *exact moment* your assumption broke. That is the first divergence from 3.3, located precisely, without a single print.

### Values update *after* a line runs, not before

Recall the rule from 3.6: the arrow points at the line *about to* execute. So when you are stopped on

```cpp
int result { a + b };
```

the variable `result` does **not** yet hold `a + b` — that line has not run. Only after you *step over* it does `result` take on its value. This off-by-one is the most common source of "why is my variable wrong?!" confusion in a debugger. The fix is simply to remember:

```text
the debug arrow points at the NEXT statement to execute,
so a line's effects appear only after you step past it.
```

### Optimized builds can lie to the debugger

One last caution. When you compile with optimizations on, the compiler is free to rearrange your program for speed in ways that scramble the debugger's view. It may:

- eliminate a variable it decided was unnecessary;
- inline a function so it has no separate frame to step into;
- reorder instructions so the "current line" jumps around;
- keep a value in a CPU register where the debugger cannot easily show it;
- merge code paths that looked distinct in the source.

The result is a debugging session where variables read `<optimized out>` and the arrow leaps backward unexpectedly.

> **Best practice:** Use an unoptimized **debug build** (for example `-g` without `-O2`) when you are debugging at the source level. Save optimized builds for the rare case where the bug *only* appears under optimization — chasing that is a different and more advanced task.

## 3.9 — Using an integrated debugger: The call stack

When a program crashes, the line it crashed on is only half the story. The other half is *how it got there* — the chain of function calls that led to that line. The **call stack** records exactly this chain, and reading it is one of the most valuable debugging skills you can develop.

Consider this deliberately broken program:

```cpp
void c()
{
    int* p {};
    *p = 1; // crash: writing through a null pointer
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

`main` calls `a`, which calls `b`, which calls `c`, which dereferences a null pointer and crashes. At the moment of the crash, the call stack captures that whole lineage:

```text
c()
b()
a()
main()
```

Debuggers conventionally show the *current* function on top and number the frames:

```text
#0 c()      <- where the crash happened (current function)
#1 b()      <- called c
#2 a()      <- called b
#3 main()   <- called a
```

Picture it as a literal stack of plates, each call placed on top of its caller:

```text
main calls a
  a calls b
    b calls c
      c crashes

stack top
  c        <- you are here
  b
  a
  main
stack bottom
```

### Stack frames

Each active call owns a **stack frame** — a private slab of memory holding everything that call needs:

- the function's parameters;
- its local variables;
- the **return address** (where to resume in the caller once this function finishes);
- some bookkeeping.

In the debugger you can *select* any frame in the stack and inspect that call's locals and parameters. Selecting frame `#3 main()` lets you see `main`'s variables; selecting `#0 c()` shows `c`'s. This is how you reconstruct the full state of the program at the moment things went wrong — not just in the function that crashed, but all the way up the chain.

### Why the call stack matters: the crash line is rarely the culprit

This connects straight back to the symptom-versus-root-cause idea from 3.2. The line that crashes is the *symptom*; the bug that put it there often lives in a *caller*.

```cpp
void useIndex(const std::string& s, std::size_t index)
{
    std::cout << s[index] << '\n';
}
```

If `index` is out of range, `useIndex` is where the trouble surfaces — but `useIndex` did nothing wrong; it faithfully used the index it was handed. The real bug is in whoever computed that bad `index` and passed it in. The call stack is what lets you walk *upward* from the symptom to the cause:

```text
useIndex()   the symptom appears here
mutate()     ...but this caller passed the bad index
main()       ...because it selected a bad configuration
```

You start at frame `#0`, see that the index is bad, then climb to frame `#1` to ask "where did this index come from?" — and keep climbing until you reach the function that first got it wrong. That is the root cause.

> **Key insight:** When something crashes, read the call stack from the top *and* look upward. The frame that crashed shows you *what* broke; the frames above it usually show you *why*.

### Recursion and stack overflow

The call stack also makes runaway recursion visible at a glance. A correct recursive function reaches a **base case** and stops calling itself. A broken one does not — and the stack fills up with copies of the same function until it overflows:

```text
factorial()
factorial()
factorial()
factorial()
... (thousands more) ...
```

If you open the call stack and see the same function repeated hundreds or thousands of times, that is your signal: check whether the recursion has a base case it can actually *reach*. A base case that exists but is never satisfied is just as broken as no base case at all.

## 3.10 — Finding issues before they become problems

Everything so far has been about *finding* bugs after they appear. The best debugging, though, is the debugging you never have to do — bugs caught at the moment of writing, or prevented from existing at all. This final section collects habits and tools that catch issues early, when they are cheap, instead of late, when they are expensive. The theme is **shortening the distance between writing a mistake and discovering it.**

### Compile often

Do not write hundreds of lines and *then* compile. When a fresh compile reports ten errors across code you wrote an hour ago, you have to reconstruct the context for each one. When it reports an error in the five lines you just typed, the fix is obvious. So adopt a tighter rhythm:

```text
write a small piece
compile
fix any compiler errors
run a small test
mentally checkpoint: this works
write the next piece
```

Each compile is a checkpoint. The smaller the change since the last one, the easier any new error is to pin on its cause.

### Enable compiler warnings

Your compiler can warn you about code that is *legal* but *suspicious* — and those warnings frequently point at genuine bugs before you ever run the program:

- a variable that may be used uninitialized;
- a variable you declared but never used (often a sign you wired something up wrong);
- an implicit conversion that silently loses data;
- a comparison between signed and unsigned values;
- a function that can reach its end without returning;
- code that can never be reached.

Each of these has saved real programmers from real bugs. Turn warnings on and *read* them — they are free advice, not noise. Common flags to enable:

```text
-Wall -Wextra -Wpedantic
```

Some teams go further with:

```text
-Werror
```

which promotes every warning to a hard error, so the code simply will not build until the warnings are addressed. It is strict, but it prevents warnings from quietly piling up and getting ignored.

> **Best practice:** Compile with `-Wall -Wextra` and treat the warnings as a to-do list, not background noise. The lab requires exactly this — your fixes must be warning-clean under `-Wall -Wextra`. A warning is the compiler doing free bug-hunting on your behalf.

### Use consistent formatting

Formatting does not make code correct, but bad formatting can *hide* incorrectness. Misleading indentation is a classic trap:

```cpp
if (x > 0)
    std::cout << "positive\n";
    std::cout << "done\n"; // looks guarded by the if — it is NOT
```

The indentation suggests both prints belong to the `if`. They do not: without braces, an `if` controls only the *single* statement that follows it, so `"done\n"` prints unconditionally. The bug is invisible at a glance precisely *because* the formatting lies. Written clearly — with braces and honest indentation — the structure cannot deceive you:

```cpp
if (x > 0)
{
    std::cout << "positive\n";
}

std::cout << "done\n"; // now obviously unconditional
```

Consistent formatting makes wrong *structure* look wrong, which is exactly when you want to notice it.

### Test incrementally, especially at the boundaries

As you build a function, test it on ordinary inputs *and* on the edges where bugs love to hide:

```cpp
int clampToNonnegative(int x)
{
    if (x < 0)
        return 0;

    return x;
}
```

The interesting inputs are not the obvious middle of the range but the values right around the decision point:

```text
x = -1  ->  0
x =  0  ->  0
x =  1  ->  1
```

The transition between `0` and `1` is exactly where an off-by-one or a flipped comparison would show up. Testing `x = 50` would never catch such a bug; testing the boundary will. This is the same instinct the lab's `rollsValue` bug rewards — an off-by-one constant that only a carefully chosen input exposes.

### Use static analysis and sanitizers when available

Two families of tools automate bug-finding beyond what the compiler does. They split neatly along the course's central theme of *static* versus *dynamic* analysis:

**Static analysis** inspects your code *without running it*, reasoning about all possible executions to flag suspect patterns. **Sanitizers** do the opposite: they instrument your program so that, *while it runs*, certain bugs are caught the instant they occur instead of corrupting state silently. Common sanitizers:

```text
AddressSanitizer (ASan):           out-of-bounds access, use-after-free
UndefinedBehaviorSanitizer (UBSan): undefined-behavior cases
ThreadSanitizer (TSan):            data races between threads
```

The two approaches complement each other, and together they capture the contrast at the heart of this course:

```text
static analysis:  reason about the program WITHOUT executing it
dynamic analysis: observe the program WHILE it executes
```

### Keep changes small

A final habit that ties the whole chapter together: change one thing at a time. When a single small change introduces a bug, the cause is obvious — it was the one thing you changed. When ten changes introduce a bug, you are left bisecting your own work to find which one did it.

```text
one bug  + one small change  =  clear cause and effect
many bugs + many changes     =  mystery soup
```

For lab work, this means: get the baseline building first, make one behavioral change, compile, run the smallest relevant test, and only then move on. It is the disciplined loop from 3.3, applied not just to fixing bugs but to *avoiding* them.

> **Tip:** Small, frequent, verified steps are not slower than big leaps — they are faster, because you almost never have to debug a mystery. You trade a little bookkeeping for a lot of certainty.

This is precisely how to attack the chapter lab. Run the grader, read the *first* failure, open that one function, fix the single bug, rebuild, and repeat. Six bugs, one localized fix at a time — clean cause and effect at every step. The lab is not asking you to write a program; it is asking you to *practice the loop* this chapter is built around.

## 3.x — Chapter 3 summary

Debugging is a learnable, repeatable discipline, not a knack. Its foundation is a single question — *where does the program first diverge from what I expected?* — and a small kit of tools for answering it.

**The four kinds of error.** A clean compile only rules out the first two:

- **Syntax errors** violate the grammar of C++; the compiler catches every one.
- **Compile-time semantic errors** have valid grammar but invalid meaning (wrong types, assigning to `const`, wrong argument counts); the compiler catches these too.
- **Runtime errors** occur during execution — divide by zero, out-of-bounds access, null dereference, runaway recursion.
- **Logical errors** compile and run but produce the wrong answer. No tool flags them; only comparing behavior against intent does.

**The debugging process.** Reproduce the failure reliably; *characterize* it as "expected X, got Y"; *localize* the first point of divergence; identify the *root cause* (which is rarely the symptom's location); make the *smallest correct fix*; then *retest* the original case and its neighbors. Avoid **debugging by demolition** — a rewrite trades one known bug for many unknown ones.

**A strategy for localizing.** Always start from a written expectation. Trace from a known-good checkpoint to a known-bad one; **binary search** the region between them; **simplify** the failing input until it fits in your head. Investigate from evidence — never edit at random.

**Basic and structured tactics.** Comment out regions to isolate a fault. Use specific, well-labeled print statements sent to `std::cerr`, not `std::cout`. Gate diagnostics behind a flag or a preprocessor macro so you can switch them on and off. Use **assertions** to encode internal invariants that should never be false (not for user-input validation).

**The integrated debugger.** *Step over* runs a call without entering it; *step into* enters it; *step out* returns to the caller. The debug arrow points at the line *about to* execute, so a variable's new value appears only *after* you step past its line. **Breakpoints** pause at chosen lines; **conditional breakpoints** pause only when an expression is true — ideal for a rare case among many iterations. **Watches** track variables and invariants as you step. The **call stack** shows the chain of active calls, letting you climb from a crash *upward* to the caller that actually caused it. Debug with an unoptimized build for a faithful view.

**Catching issues early.** Compile often, in small steps. Enable `-Wall -Wextra` and treat warnings as real feedback. Format honestly so wrong structure looks wrong. Test the boundaries, where bugs live. Use static analysis (reason without running) and sanitizers (observe while running). Keep every change small, so cause and effect stay clear.

Carry one sentence out of this chapter: **the symptom is not the root cause, and your job is to find the first place reality diverges from your expectation.** The Coin-Tray Auditor lab is your first chance to practice it — six logical bugs in code that compiles cleanly, fixed one localized change at a time.
