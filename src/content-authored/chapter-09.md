# Chapter 9 — Error Detection and Handling

> Source: <https://www.learncpp.com/> (Chapter 9)

Up to now we've mostly cared whether a program *compiles* — whether the grammar is right and the types line up. That's a low bar. A program that compiles has cleared the compiler's spell-check; it has not promised to be *correct*, and it has certainly not promised to survive contact with a real, careless, occasionally hostile user. This chapter is about the gap between "it builds" and "it works, and keeps working." We'll learn how to *test* code so we catch our own mistakes early, how to think about which tests are worth writing, the handful of semantic bugs that bite C++ programmers again and again, and finally the two distinct disciplines of dealing with things that go wrong: **handling errors** that are part of normal life, and **asserting** that things which should be impossible really are. By the end you'll be ready to build the chapter's lab — a "front desk" of small, testable functions that refuses to crash no matter what gets typed at it.

## Contents
- [9.1 — Introduction to testing your code](#91--introduction-to-testing-your-code)
- [9.2 — Code coverage](#92--code-coverage)
- [9.3 — Common semantic errors in C++](#93--common-semantic-errors-in-c)
- [9.4 — Detecting and handling errors](#94--detecting-and-handling-errors)
- [9.5 — std::cin and handling invalid input](#95--stdcin-and-handling-invalid-input)
- [9.6 — Assert and static_assert](#96--assert-and-static_assert)
- [9.x — Chapter 9 summary and quiz](#9x--chapter-9-summary-and-quiz)

---

## 9.1 — Introduction to testing your code

### "It compiled" is not "it works"

When your program builds without errors, the compiler is telling you something narrow: the syntax is legal and the types are consistent. It is *not* telling you that the program does what you meant. The compiler simply has no way to know what you meant.

```cpp
int clampPercent(int value)
{
    if (value < 0)
        return 0;
    if (value > 100)
        return 100;

    return 10; // compiles fine — but should have returned value
}
```

The compiler can see that this function returns an `int`, and that `10` is an `int`, so it is perfectly happy. What it cannot see is your *intent*: "a value already between 0 and 100 should come back unchanged." That intent lives in your head, not in the type system. The only way to find out whether the code matches the intent is to **run it under chosen conditions and check the result** — and that is exactly what testing is.

> **Key insight:** The compiler verifies that your code is *well-formed*. Testing verifies that your code is *correct*. Those are different questions, and only you can ask the second one.

### Why testing gets hard: the paths multiply

If a program were a single straight line of statements, you could test it by running it once. But every `if`, every loop, every input, and every piece of state opens up a new branch of possible behavior, and the number of distinct ways a program can execute grows quickly.

```cpp
std::string categoryForScore(int score)
{
    if (score < 0)
        return "invalid";
    if (score < 60)
        return "failing";
    if (score < 90)
        return "passing";
    return "excellent";
}
```

You could not possibly test every `int` value here — there are billions — and you wouldn't want to, because most of them tell you nothing new. The skill is choosing a small set of inputs that each exercises a *different decision*. For this function, the interesting inputs cluster around the boundaries where the answer changes:

| Input | What it checks |
|---|---|
| `-1` | the invalid (negative) case |
| `0` | the lowest valid score |
| `59` | last "failing" value |
| `60` | first "passing" value |
| `89` | last "passing" value |
| `90` | first "excellent" value |

Good testing is not "mash in random numbers until it feels okay." It is a deliberate hunt for the inputs that pin down each branch and each boundary. We'll formalize that instinct in the next lesson under the name *coverage*.

### Scope creep raises the stakes

A function you wrote as a five-minute throwaway has a way of growing up. The quick helper gets reused somewhere else; the program that read one tidy input starts accepting many messy ones. This drift — a program quietly outgrowing its original purpose — is called **scope creep**.

```
one-off script
  └─ becomes a reusable helper
       └─ starts accepting external input
            └─ gets called by other code
                 └─ now really needs durable tests
```

The lesson: the more *other* code leans on a function, the more a preserved test for that function pays off. A bug in a leaf function that everyone calls is a bug everywhere.

### Test small units first

A **unit test** checks one small piece of code — typically a single function or a single behavior of a class — in isolation. The reason to start small is diagnostic: when a tiny, isolated test fails, you already know roughly where the bug lives. When a giant end-to-end test fails, the bug could be anywhere.

```cpp
bool isMutationOperator(char c)
{
    switch (c)
    {
    case 'A':
    case 'R':
    case 'D':
        return true;
    default:
        return false;
    }
}
```

While writing this you might check it by hand:

```cpp
std::cout << isMutationOperator('A') << '\n'; // expect 1 (true)
std::cout << isMutationOperator('x') << '\n'; // expect 0 (false)
```

That's genuinely useful in the moment — but it's fragile. You have to read the output yourself, you have to *remember* what each line was supposed to print, and the moment the program grows you'll delete those lines to get them out of the way. The expected answers live only in your memory.

### Preserve your tests so they can run again

A small step up is to put the checks into a function of their own, so the test — and the expected answers — stay next to the code permanently.

```cpp
int testIsMutationOperator()
{
    if (!isMutationOperator('A')) return 1;
    if (!isMutationOperator('R')) return 2;
    if (!isMutationOperator('D')) return 3;
    if (isMutationOperator('x'))  return 4;
    if (isMutationOperator('\0')) return 5;

    return 0; // all checks passed
}
```

Now `main` can run it and report which case broke:

```cpp
int main()
{
    const int failedCase { testIsMutationOperator() };

    if (failedCase != 0)
        std::cerr << "testIsMutationOperator failed case " << failedCase << '\n';
}
```

This isn't a real testing *framework* — those exist, and you'll meet them later — but it already captures the essential idea: a preserved test stores both **the operation to run** and **the answer you expect**, so re-running it requires no memory and no human eyeballing.

### Automate the answer-checking

The reason to encode the expected answer (rather than just printing and looking) is that *manual inspection does not scale*. A test that prints "looks right to me" still needs a human to compare. A test that *returns* pass or fail can be wired into a script, a build step, or a continuous-integration job that runs on every change.

```cpp
bool testClampPercent()
{
    return clampPercent(-5)  == 0
        && clampPercent(0)   == 0
        && clampPercent(42)  == 42
        && clampPercent(100) == 100
        && clampPercent(250) == 100;
}
```

> **Best practice:** Write tests that *decide* pass or fail themselves, rather than printing values for you to eyeball. A test only protects you if it can run unattended.

### Unit vs. integration testing

There are two altitudes you can test at, and you want both.

| Kind | Checks | Example |
|---|---|---|
| **Unit test** | one small piece in isolation | `parseOperator("+")` returns `Add` |
| **Integration test** | several pieces working *together* | read input → parse it → compute → print the result |

Integration tests are valuable because they catch problems in the *seams* between functions — the places where each unit is individually fine but they don't quite agree on the handoff. Their downside is diagnostic: when an integration test fails, the fault could be in any participating unit or in how they're wired together. That's exactly why you build a foundation of unit tests *underneath* your integration tests — so that when the big test goes red, you can lean on the green unit tests to narrow down where it isn't.

The chapter lab leans on this split directly: each validator function (quantity check, range clamp, keystroke parser, stock lookup, digits check) is unit-tested in isolation, and the interactive input loop is closer to an integration test driven by a hostile script.

---

## 9.2 — Code coverage

### Coverage answers one question: "what ran?"

**Code coverage** measures how much of your source code was actually executed by your tests. It's a useful number, but it's important to be clear about what it does and does not tell you.

```
coverage CAN say:     this line / this branch executed
coverage CANNOT say:  this line / this branch produced the right answer
```

A test suite can drive every line of a program and still prove nothing if its checks are weak. High coverage with no real assertions is a green light bolted to a broken machine. Treat coverage as a way to find code your tests *never touch* — a floor, not a finish line.

### Statement coverage

**Statement coverage** asks the simplest version of the question: has each statement run at least once?

```cpp
int maxOfTwo(int a, int b)
{
    int result { b };

    if (a > b)
        result = a;

    return result;
}
```

Call `maxOfTwo(7, 3)` and trace it: `result { b }` runs, the `if` condition runs, `result = a` runs (because `7 > 3`), and `return result` runs. Every statement executed — 100% statement coverage from a single call.

And yet that single call never tested what happens when `a <= b`. We never observed the case where the `if` body is *skipped*. Statement coverage felt complete, but it quietly missed an entire path.

### Branch coverage

**Branch coverage** is the stronger question: has every *outcome* of every decision executed — both the true path and the false path?

```cpp
maxOfTwo(7, 3); // exercises the if-condition being true
maxOfTwo(3, 7); // exercises the if-condition being false
```

Now both ways through the `if` have been taken.

```
              a > b ?
             /        \
         true          false
          |              |
      result = a      keep b
           \           /
            return result
```

> **Best practice:** When you're testing code with real decision-making in it, aim for branch coverage, not just statement coverage. It's the difference between "every line ran" and "every *decision* was made both ways."

### A missing `else` is still a branch

A common trap: people see an `if` with no `else` and assume there's only one path to test.

```cpp
if (enabled)
    runCheck();
```

There are still two outcomes here. To cover both you need:

- `enabled == true` — `runCheck()` runs.
- `enabled == false` — `runCheck()` is skipped.

The *absence of an `else` body* does not mean the false path doesn't exist. It just means nothing happens on it — and "nothing happens" is itself a behavior you should confirm.

### Loops: the 0, 1, 2 rule

Loops have their own coverage trap, because the bugs in a loop tend to hide at its edges. A reliable habit is to test a loop with **zero, one, and two** iterations.

| Iterations | What it shakes out |
|---|---|
| 0 | Does the loop skip cleanly when there's nothing to do? |
| 1 | Does the body work correctly a single time? |
| 2 | Does state carry over correctly *between* iterations? |

```cpp
int sumFirstN(int n)
{
    int total { 0 };

    for (int value { 1 }; value <= n; ++value)
        total += value;

    return total;
}
```

Three small tests cover the interesting cases:

```cpp
sumFirstN(0) == 0; // zero iterations
sumFirstN(1) == 1; // one iteration
sumFirstN(2) == 3; // two iterations (1 + 2)
```

The two-iteration case is the important one: it's the smallest test that proves the loop can carry state *forward* from one pass to the next. Many loop bugs — an update in the wrong place, an accumulator reset each time — survive a one-iteration test and die on a two-iteration test.

### Category testing

Even with branch and loop coverage in mind, the input space is usually far too big to test exhaustively. The trick is to sort inputs into **categories** — groups that should all behave the same way — and test a representative from each.

For an integer parameter, the usual categories are:

- negative values,
- zero,
- positive values,
- the boundary values (just inside and just outside the valid range),
- values large enough to risk overflow.

For floating-point values:

- clean-looking values like `0.5`,
- precision-stress values like `0.1`,
- negatives,
- values sitting right at a comparison threshold.

For strings:

- the empty string,
- ordinary alphanumeric text,
- leading and trailing whitespace,
- embedded whitespace,
- a string that's *all* whitespace,
- something very long.

For pointers:

- a valid pointer,
- `nullptr`, if the function accepts a pointer and null is a real possibility.

> **Tip:** Coverage tells you which code *ran*; category thinking tells you which *inputs are worth feeding it*. Use them together — categories pick the tests, coverage confirms they reached everything.

The chapter lab's grader is built straight out of this category list: it hits each function with the empty string, with letters where digits go, with values just outside the valid band, with trailing junk, and with end-of-input. If you write your functions with those categories in mind, the grader has nothing left to surprise you with.

---

## 9.3 — Common semantic errors in C++

### Syntax errors vs. semantic errors

A **syntax error** breaks the grammar of C++. The compiler catches these for you, points at the line, and refuses to build.

```cpp
int x { 3 // missing the closing brace and semicolon
```

A **semantic error** is different and more dangerous: the code is perfectly legal C++, it compiles, it runs — and it does the *wrong thing*, because it means something other than what you intended.

```cpp
int average(int total, int count)
{
    return total / count; // integer division — loses the fraction
}
```

Nothing here is illegal. The bug is that the meaning doesn't match the intent. Because the compiler can't help you with semantic errors, the rest of this lesson is a tour of the ones that catch C++ programmers most often — so you know where to *look*.

### Conditional logic errors

These happen when a condition doesn't quite encode the rule you had in mind — usually an off-by-a-boundary slip.

```cpp
bool isAdult(int age)
{
    return age > 18; // almost certainly should be age >= 18
}
```

Boundary tests are exactly what flush these out:

```cpp
isAdult(17) == false;
isAdult(18) == true;   // this one fails with the buggy '>'
isAdult(19) == true;
```

### Infinite loops

An infinite loop is one whose condition never becomes false. The classic cause is forgetting to advance the loop toward its exit.

```cpp
int countdown { 3 };

while (countdown > 0)
{
    std::cout << countdown << '\n';
    // forgot --countdown — the condition can never become false
}
```

There's a subtler version that involves *unsigned* arithmetic:

```cpp
for (unsigned int i { 3 }; i >= 0; --i)
{
    std::cout << i << '\n';
}
```

This loop never ends. An `unsigned int` can never be negative, so `i >= 0` is *always* true. Worse, when `i` is `0` and you decrement it, it doesn't go to `-1` — it **wraps around** to a huge positive value, and the loop sails on.

> **Best practice:** Use a *signed* loop counter unless unsigned arithmetic is genuinely part of what you're modeling. It sidesteps a whole family of wraparound surprises.

### Off-by-one errors

An **off-by-one error** is a loop or range that runs one time too many or one time too few. The most common form indexes one past the end:

```cpp
for (int i { 0 }; i <= length; ++i) // wrong for 0-based indexing
{
    process(i);
}
```

If `length == 5`, the valid indices are:

```
0 1 2 3 4
```

But `i <= length` also lets `i` reach `5`, one past the last valid index. The fix is to match the comparison to how you're counting:

```cpp
for (int i { 0 }; i < length; ++i)   // 0-based: i goes 0..length-1
for (int n { 1 }; n <= limit; ++n)   // 1-based: n goes 1..limit
```

Pick the pattern that matches your domain — then test the boundaries to confirm you picked right.

### Operator precedence mistakes

C++ does not always group operators the way casual reading suggests.

```cpp
if (!ready == expected)
{
    // C++ reads this as: (!ready) == expected
}
```

If what you meant was "ready is not equal to expected," say so directly — it's both correct and clearer:

```cpp
if (ready != expected)
{
    // unambiguous
}
```

And whenever you mix `&&` and `||`, add parentheses to make the grouping explicit rather than trusting yourself (and the next reader) to recall the precedence table:

```cpp
if ((isAdmin || isOwner) && accountEnabled)
{
    allowAccess();
}
```

### Floating-point precision

Floating-point types can only *approximate* most decimal values, so two computations that look like they should land on the same number often don't. Testing them for exact equality is fragile.

```cpp
double total { 0.1 + 0.2 };

if (total == 0.3) // fragile — this can be false!
    std::cout << "exact\n";
```

When you must compare computed floating-point values, check whether they're *close enough* rather than exactly equal:

```cpp
#include <cmath>

bool nearlyEqual(double a, double b, double epsilon)
{
    return std::abs(a - b) <= epsilon;
}
```

### Integer division

When *both* operands of `/` are integers, C++ performs integer division and throws away the remainder — even if you're about to store the result in a `double`.

```cpp
int passed { 7 };
int total { 10 };

double rate { passed / total }; // 7 / 10 is 0 (int), then converted to 0.0
```

The division already happened in integer arithmetic before anything was converted. To get the real answer, convert *before* dividing so the division itself is floating-point:

```cpp
double rate { static_cast<double>(passed) / total }; // 0.7
```

> **Key insight:** The cast has to go at the arithmetic boundary — *before* the division — not after. Once the integer division has produced `0`, no later conversion can recover the lost fraction.

### Accidental null statements

A stray semicolon can quietly detach a body from the `if` or loop that was supposed to control it.

```cpp
if (shouldRun);          // this semicolon is a complete, empty statement
{
    runExpensiveCheck(); // not controlled by the if — runs unconditionally
}
```

That semicolon *is* the entire `if` body — a do-nothing statement. The block beneath it is just an ordinary block that always runs.

### Missing braces around multi-statement bodies

Without braces, only the *single next statement* belongs to an `if`, loop, or `else`.

```cpp
if (isValid)
    std::cout << "valid\n";
    saveResult(); // NOT part of the if — runs every time
```

The indentation lies; only the `std::cout` line is guarded. The fix is braces:

```cpp
if (isValid)
{
    std::cout << "valid\n";
    saveResult();
}
```

> **Best practice:** Once a body is more than one statement, always brace it. In code that changes often, consider bracing even one-line bodies, so that *adding* a second line later can't reintroduce this bug.

### Assignment instead of equality

A single `=` where you meant `==` compiles, because an assignment is itself an expression that produces a value — and that value gets tested.

```cpp
char answer { 'n' };

if (answer = 'y') // assigns 'y' to answer, THEN tests 'y' (nonzero → true)
{
    launch();     // always runs, and answer is now 'y'
}
```

Many compilers will warn about this — but only if warnings are turned on and you actually read them. The correct version compares:

```cpp
if (answer == 'y')
{
    launch();
}
```

### Forgetting the function-call operator

Writing a function's name *without* `()` doesn't call it — it refers to the function itself, which in a boolean context is just a non-null thing and therefore "true."

```cpp
int getLimit()
{
    return 10;
}

if (getLimit) // bug: never calls getLimit(); always truthy
{
    std::cout << "this prints no matter what\n";
}
```

What you meant was to call it and test the result:

```cpp
if (getLimit() > 0)
{
    std::cout << "positive limit\n";
}
```

### Where these bugs cluster

Notice that almost every error above lives in one of a small number of places:

```
branch conditions      loop conditions / updates
boundary indices       operator grouping
numeric conversions    input validation
```

> **Tip:** When you review or test C++ code, spend your attention budget on exactly these spots. They're where the compiler can't help you and where the bugs concentrate.

---

## 9.4 — Detecting and handling errors

### Two very different kinds of "error"

The word *error* covers two situations that call for completely different responses, and confusing them is itself a common bug.

Some errors are **programmer mistakes** — the code is simply wrong:

```cpp
// Wrong comparison; this is a bug in the code itself.
if (count <= maxIndex) { /* ... */ }
```

Other errors are **expected facts of life** — the code is correct, but the world didn't cooperate:

```cpp
// The user gave a filename that doesn't exist. Not a code bug.
std::ifstream input { filename };
```

The first kind you *fix*. The second kind you *handle* — detect it and respond gracefully — because it will keep happening no matter how good your code is. This distinction comes back at the end of the chapter when we contrast `assert` (for the first kind) with error handling (for the second kind). For now, this lesson is about the second kind.

### Happy paths and sad paths

The **happy path** is the ordinary, everything-went-fine route through your code. The **sad path** is what happens when something goes wrong along the way:

- input can't be parsed,
- a file won't open,
- a lookup finds nothing,
- an argument is out of range,
- an external service is down.

A robust program is *designed* for both paths from the start, not patched for the sad path after a crash report comes in.

```
parse input
   ├── valid   → compute result → print result      (happy path)
   └── invalid → explain / retry / report failure    (sad path)
```

### Three places assumptions tend to break

When you go looking for missing sad-path handling, three assumptions account for most of it:

1. **A called function is assumed to have succeeded** (but it might have failed).
2. **Input is assumed to be well-formed** (but the user typed garbage).
3. **A function's arguments are assumed to be sensible** (but the caller passed something invalid).

Consider:

```cpp
int divideRoundedDown(int numerator, int denominator)
{
    return numerator / denominator; // assumes denominator != 0
}
```

This function silently assumes `denominator` is nonzero. If the caller can't guarantee that, the function needs a *strategy* for the zero case. There are four broad strategies, and the rest of this lesson walks through them.

### Strategy 1 — handle the error locally

If the function itself knows what to do about the problem, the simplest answer is to deal with it right there.

```cpp
int readMenuChoice()
{
    while (true)
    {
        std::cout << "Choice (1-3): ";

        int choice {};
        std::cin >> choice;

        if (choice >= 1 && choice <= 3)
            return choice;

        std::cout << "Invalid choice. Try again.\n";
    }
}
```

This is the right place to handle the error because input collection *owns* the retry policy — re-prompting is precisely this function's job. Handle locally when the function has both the knowledge and the authority to recover.

### Strategy 2 — return failure information to the caller

Often the function *detects* a problem but isn't the right place to *decide* what to do about it. A calculation function, for instance, has no business prompting the user or shutting down the program. In that case, report the failure upward and let the caller choose the policy.

```cpp
bool tryDivide(int numerator, int denominator, int& result)
{
    if (denominator == 0)
        return false; // couldn't compute; tell the caller

    result = numerator / denominator;
    return true;      // succeeded; result is now valid
}
```

The caller decides what failure means in *their* context:

```cpp
int quotient {};

if (tryDivide(10, value, quotient))
    std::cout << quotient << '\n';
else
    std::cerr << "Cannot divide by zero\n";
```

Notice the division of labor: `tryDivide` knows *that* something failed; only the caller knows *how to react*. This keeps low-level code reusable, because it isn't hard-wired to one program's idea of error handling.

### Sentinel values

A **sentinel value** is a special return value that means "no normal result" — a value carved out of the result space to carry an out-of-band signal.

```cpp
constexpr int notFound { -1 };

int findFirstSpace(std::string_view text)
{
    for (int i { 0 }; i < static_cast<int>(text.length()); ++i)
    {
        if (text[static_cast<std::size_t>(i)] == ' ')
            return i;
    }

    return notFound; // no space exists — signal it
}
```

Sentinels are simple and they're everywhere, but they come with hazards you have to respect:

- the caller has to *remember* to check for the sentinel,
- the sentinel must be a value that can never be a legitimate result (a `-1` index works because real indices are non-negative),
- without a *named* constant like `notFound`, a bare `-1` is a mystery to the next reader.

> **Note:** Later in the course you'll meet `std::optional<T>`, which models "maybe there's a value, maybe not" more safely than a sentinel — the "missing" case can't be mistaken for a real result. The chapter lab deliberately uses a sentinel (`kInvalidChoice`) because `std::optional` is out of scope here, but the stretch goals point you at the upgrade.

### Strategy 3 — halt on a fatal error

Some errors are **non-recoverable**: continuing would leave the program in a state where nothing it does can be trusted. A required configuration file that's missing is a good example — there's no sensible "carry on anyway."

```cpp
int loadRequiredConfig()
{
    std::ifstream config { "required.cfg" };

    if (!config)
    {
        std::cerr << "required.cfg is missing\n";
        return 1; // nonzero status from main signals failure
    }

    return 0;
}
```

> **Best practice:** When you can, signal a fatal error by *returning a nonzero exit code from `main`*. Deep inside a helper, programs sometimes call a halt function like `std::exit`, but doing so couples that one helper to whole-program policy — it can no longer be reused anywhere that *doesn't* want to terminate. Prefer reporting failure upward and letting `main` make the final call.

### Strategy 4 — throw an exception

C++ also has a structured mechanism, **exceptions**, for sending an error up the call stack until some caller is willing to handle it. Exceptions get a full chapter of their own later; right now you only need the shape of the idea:

```
detect error in a lower function
   └─ throw
        └─ some caller can catch and handle it
             └─ if nobody catches it, the program terminates
```

> **Warning:** Don't reach for exceptions just because returning an error code feels tedious. Use the error channel — return values, or exceptions — that the surrounding codebase already treats as normal. In the chapter lab, exceptions are explicitly *out of scope*: you'll signal failure with sentinels and validation loops instead.

### `std::cout`, `std::cerr`, and logs

Where you send error output matters, because different destinations serve different readers.

| Stream | Use for |
|---|---|
| `std::cout` | normal, user-facing output |
| `std::cerr` | diagnostics and error messages |
| a log file | a durable history of events to inspect later |

Interactive beginner programs often print input-error messages with `std::cout`, because in a conversation with the user those messages *are* part of the normal interaction. Command-line tools, by contrast, usually send errors to `std::cerr` so that normal output and diagnostics can be redirected to different places — letting you capture machine-readable results while error chatter goes elsewhere.

```cpp
std::cout << result << '\n';                  // machine-readable result
std::cerr << "failed to parse input file\n";  // diagnostic
```

Keeping the two streams separate is what makes a program scriptable and gradeable — and it's a habit worth building now.

---

## 9.5 — std::cin and handling invalid input

### Why text input is genuinely hard

`std::cin` is your program's front door to a human, and humans type *anything*. Ask for an `int` and you might receive:

```
42
42abc
abc
999999999999999999999999
<end-of-file>
```

A robust program decides, in advance, which of these it will accept and what it does with the rest. The first step is understanding exactly how extraction behaves, so let's open the hood.

### A simplified model of extraction

When you write:

```cpp
std::cin >> value;
```

the extraction operator roughly goes through these steps:

1. It **skips leading whitespace** (spaces, tabs, newlines).
2. If the input buffer is empty, it **waits** for the user to type and press Enter.
3. It **extracts as many characters as fit** the target type, and stops at the first character that doesn't fit.
4. It **leaves the unextracted characters in the buffer** for next time.
5. If it couldn't extract anything valid at all, it puts the stream into a **failure state**.

A picture of step 3 and 4:

```
user types:      123abc⏎
extracted (int): 123
left in buffer:  abc⏎
```

That leftover `abc⏎` is the crucial detail: it doesn't vanish. It sits in the buffer and will ambush your *next* extraction unless you clear it out. Most `std::cin` bugs trace back to forgetting about leftover input.

### Input validation

**Input validation** is the act of checking whether what you received matches what you expected. There are three broad approaches:

| Approach | Idea | Beginner use with `std::cin`? |
|---|---|---|
| Inline validation | reject bad keystrokes as they're typed | usually not available to us |
| Read-as-string, then parse | grab the whole token as text, validate it yourself | powerful, but more work |
| Let `operator>>` extract, then inspect the stream state | use `std::cin`'s own mechanics | the workhorse for this chapter |

The chapter lab uses both of the last two: a strict digits-only string check (read-then-validate) *and* a stream-state recovery loop. Let's walk through the three error cases the stream-state approach has to survive.

### Error case 1 — extraction succeeds, but the value is meaningless

Sometimes `std::cin` reads a perfectly valid value of the right *type* that's still wrong for your program.

```cpp
char readCommand()
{
    while (true)
    {
        std::cout << "Command [a/r/q]: ";

        char command {};
        std::cin >> command;

        switch (command)
        {
        case 'a':
        case 'r':
        case 'q':
            return command;
        default:
            std::cout << "Unknown command.\n";
        }
    }
}
```

The extraction never *fails* here — a `char` is a `char`. The stream is happy. It's your program that has to ask the further question: does this character actually *mean* anything in my menu? That's a domain check, separate from the stream's success.

### Error case 2 — extraction succeeds, but there's extra input left over

Suppose you expect one number per prompt and the user types `12x`. Extraction to `int` reads `12` and leaves `x⏎` in the buffer. The extraction *succeeded*, but there's debris waiting to trip the next read.

A small helper discards the rest of the current line:

```cpp
#include <limits>

void ignoreLine()
{
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}
```

`ignore` throws away characters until it has consumed a newline (or the absurdly large maximum, whichever comes first). With that in hand:

```cpp
int readIntLoosely()
{
    int value {};
    std::cin >> value;
    ignoreLine();    // toss any trailing junk on this line
    return value;
}
```

This version *accepts* `12x` as `12`. Whether that's correct depends on your program's contract: sometimes trailing characters are harmless, sometimes they should make the whole input invalid. The point is that it's a *decision* — make it deliberately.

### Error case 3 — extraction fails outright

If the user types `abc` where an `int` is expected, extraction *fails*, and three things are now true:

- the invalid characters are **still in the buffer**,
- `std::cin` is in a **failure state**,
- every future extraction **fails immediately** until you clear that state.

That last point is the killer: once the stream has failed, it stays failed, and every subsequent `>>` is a no-op. The recovery is a fixed two-step:

```cpp
if (!std::cin)
{
    std::cin.clear(); // reset the stream's failure state
    ignoreLine();     // discard the bad characters still in the buffer
}
```

> **Warning:** The order matters. You must `clear()` *before* `ignoreLine()`. While the stream is still in a failed state, `ignore` won't do its job — so reset the state first, *then* drain the buffer.

### A reusable, robust integer reader

Putting the pieces together yields the pattern at the heart of the chapter lab: read an integer that must fall within `[min, max]`, re-prompting until you get one.

```cpp
int readIntInRange(int min, int max)
{
    while (true)
    {
        std::cout << "Enter value [" << min << ", " << max << "]: ";

        int value {};
        std::cin >> value;

        const bool extracted { static_cast<bool>(std::cin) }; // snapshot success first
        std::cin.clear();                                     // then reset any failure
        ignoreLine();                                         // then drain the line

        if (!extracted)
        {
            std::cout << "That was not an integer.\n";
            continue;
        }

        if (value < min || value > max)
        {
            std::cout << "That value is out of range.\n";
            continue;
        }

        return value;
    }
}
```

The ordering is the whole trick, so it's worth saying out loud: **snapshot → clear → ignore → decide.** You record whether the extraction worked *before* `clear()` wipes that information; you always drain the line so trailing junk from a `12x` can't poison the next prompt; and only then do you decide whether to accept, reject as out-of-range, or reject as non-numeric. This single function survives letters, leftover characters, and valid-but-out-of-range numbers.

### EOF — the end of input

**EOF** ("end of file") means there is no more input to be had. For a file, hitting EOF is perfectly normal — it's just the end. For `std::cin`, a user can *signal* EOF deliberately with an operating-system-specific key combination (commonly Ctrl-D or Ctrl-Z), and a redirected input file ends with EOF too.

Here's the danger: if your reader sits inside a `while (true)` retry loop and you blindly `clear()` an EOF condition, you create an **infinite loop** — there's no more input, so every retry instantly re-fails, forever. When EOF means "the user is done," you must *leave* the input routine rather than retry.

```cpp
if (std::cin.eof())
{
    std::cerr << "Input closed\n";
    std::exit(0);
}
```

> **Key insight:** A *failed extraction* and *EOF* are not the same problem. A failed extraction means "that input was bad — try again"; EOF means "there will never be any more input — stop asking." Treating EOF like an ordinary failure is exactly how a retry loop becomes an infinite loop. The chapter lab makes you check for EOF explicitly so a closed stream can't spin forever.

---

## 9.6 — Assert and static_assert

### Preconditions, invariants, and postconditions

Before we reach for any tool, it helps to name the *kinds of conditions* we want to guarantee.

| Term | Meaning | Example |
|---|---|---|
| **Precondition** | must be true *before* a piece of code runs | the divisor is not zero |
| **Invariant** | must stay true *while* the code runs | an index stays within bounds |
| **Postcondition** | must be true *after* the code runs | the returned list is sorted |

Preconditions are commonly checked right at the top of a function, before the real work begins:

```cpp
double ratio(int numerator, int denominator)
{
    if (denominator == 0)
    {
        std::cerr << "denominator must not be zero\n";
        return 0.0;
    }

    return static_cast<double>(numerator) / denominator;
}
```

This "check-and-return-early" style keeps the error handling next to the condition it guards and saves the main logic from being buried inside nested `if`s.

### Assertions document the impossible

An **assertion** is an expression you claim is *always* true — and if it isn't, that doesn't mean the world misbehaved, it means *your code has a bug*. That's the key mental shift: assertions aren't for user errors, they're for catching states that *cannot happen unless you made a mistake*.

```cpp
#include <cassert>

int getBucket(int percent)
{
    assert(percent >= 0 && percent <= 100);
    return percent / 10;
}
```

If correct code only ever calls `getBucket` with a real percentage, then a value outside `0..100` is proof that *some caller* is broken. The assertion catches that broken caller during development, loudly and immediately, instead of letting a nonsense value slip downstream.

### `assert` is a runtime debug check

`assert(condition)` is a preprocessor macro from `<cassert>`. When assertions are enabled, it:

- evaluates the condition at runtime,
- and if the condition is false, prints diagnostic information (the failed expression, the file, the line) and **aborts the program**.

Crucially, assertions can be switched *off* for release builds. When the macro `NDEBUG` is defined before `<cassert>` is included, `assert` expands to nothing and its checks vanish:

```
debug build:    assert(percent <= 100) is checked at runtime
release build:  assert(percent <= 100) may be compiled out entirely
```

This compile-it-out behavior leads directly to the most important rule about assertions:

> **Warning:** Never put a *side effect* inside an `assert`. Because the assertion can disappear in release builds, anything with a side effect would disappear with it — and now your debug and release builds do different things.

```cpp
assert(++index < size); // BAD: the increment vanishes in release builds
```

Do the work outside the assertion, and assert on the result:

```cpp
++index;
assert(index < size);   // GOOD: the increment always happens
```

### Make assertions explain themselves

A bare assertion gives a cryptic failure message — just the expression that tripped.

```cpp
assert(found);
```

A common trick adds an explanatory string with `&&`:

```cpp
assert(found && "expected target to exist before processing");
```

This works because a non-empty string literal is always "true," so `found && "..."` has the same truth value as `found` alone — the logic is unchanged. But when the assertion *fails*, the whole expression (including your message) is what gets printed, so the failure now explains itself.

> **Best practice:** Attach a descriptive `&& "message"` to assertions whose failure wouldn't otherwise be self-explanatory. The chapter lab requires this — every `assert` you write there should say *why* it should hold.

### `static_assert` — checking at compile time

`static_assert` is the assertion's compile-time cousin. It checks a condition *while the program is being compiled*, and a failure is a compile error rather than a runtime abort.

```cpp
static_assert(sizeof(int) >= 4, "this code expects at least a 32-bit int");

int main()
{
}
```

Its properties make it strictly better than `assert` *when it applies*:

- the condition must be a **constant expression** (knowable at compile time),
- no header is required,
- a failure stops the build with your message,
- it is **never** compiled out — release builds check it too,
- it costs **nothing** at runtime, because the check happened before the program ran.

> **Best practice:** If a condition can be checked at compile time, prefer `static_assert` over `assert`. You catch the problem earlier (at build time), the check can never be accidentally disabled, and it adds zero runtime cost.

A particularly nice use is guarding a table against drifting out of sync with an enum:

```cpp
enum class MutationKind
{
    ReplaceOp,
    DeleteStmt,
    InsertGuard,
    Count
};

static_assert(static_cast<int>(MutationKind::Count) == 3,
              "update the mutation dispatch table");
```

Add a fourth kind to the enum and this assertion fails the build, forcing you to update whatever depends on the count. The chapter lab uses exactly this technique to prove its stock table has one entry per menu item — a compile-time guarantee that the table and the menu can't fall out of step.

### Assertions vs. error handling: when to use which

This is the distinction the whole chapter has been building toward. Use an **assertion** for a condition that should *never* be false in correct code — a programmer's broken assumption:

```cpp
void visitInstruction(Instruction* instruction)
{
    assert(instruction != nullptr); // a null here means the caller is buggy
    // ...
}
```

Use **error handling** for a condition that *can* legitimately happen while the program runs — a fact of the world:

```cpp
std::ifstream input { path };

if (!input)
{
    std::cerr << "could not open " << path << '\n'; // a missing file is normal
    return false;
}
```

The dividing line is simple: *could a correct program, given a cooperative world, ever hit this?* If yes — bad input, a missing file — handle it. If no — it would only happen because of a bug — assert it. And occasionally you want both: an assertion to catch the bug in development, plus a defensive runtime fallback so that *if* the impossible somehow happens in a release build, the program degrades gracefully instead of detonating.

```cpp
double inverse(double x)
{
    assert(x != 0.0 && "caller should have filtered out zero");

    if (x == 0.0)
        return 0.0; // release-build safety net if the impossible happens

    return 1.0 / x;
}
```

> **Key insight:** Don't `assert` on user input. A user typing letters into a number prompt is *not* a bug in your program — it's Tuesday. Validate and re-prompt for user mistakes; reserve assertions for *your own* mistakes. The chapter lab enforces this split precisely: `assert`/`static_assert` guard the function contracts (a caller passing `lo > hi`), while the `std::cin` loop handles everything a real person might type.

---

## 9.x — Chapter 9 summary and quiz

### The big ideas

You learned to tell the difference between *building* and *working*, and picked up the tools that close the gap:

- A program that compiles can still be **semantically wrong** — the compiler checks form, you check meaning.
- **Unit tests** check small pieces in isolation; **integration tests** check pieces working together. Build the units first so failures are easy to localize.
- **Preserve** your tests when the behavior matters long-term, and write them to **decide pass/fail themselves** instead of relying on manual inspection.
- **Statement coverage** asks whether statements ran; **branch coverage** asks whether each decision went both ways. Aim for branch coverage on decision-heavy code.
- Test loops with **0, 1, and 2 iterations**; the two-iteration case is what proves state carries forward.
- **Category testing** picks representative inputs from meaningful groups — empty, boundary, negative, junk.
- Semantic bugs cluster in **conditions, loop updates, boundary indices, numeric conversions, and missing braces**.
- Robust programs handle both the **happy path** and the **sad path**.
- **Error handling** is for things that happen in normal operation; **assertions** are for programmer mistakes and impossible states.
- Prefer **`static_assert`** over `assert` whenever the condition is knowable at compile time.
- `std::cin` recovery is a fixed dance: **snapshot success → `clear()` → `ignore()` the line → decide** — and treat **EOF** as "stop," not "retry."

### A testing checklist

Before you trust a function, ask:

- What are its normal inputs?
- What are its boundary inputs?
- What inputs are *valid in type but meaningless* to this function?
- Does every branch run in at least one test?
- Do its loops have 0-, 1-, and 2-iteration tests?
- Does it report failure clearly?
- Are the impossible cases documented with assertions?

### A decision table

| Situation | Reach for |
|---|---|
| Small behavior must stay correct over time | a preserved unit test |
| Several functions must cooperate | an integration test |
| You need to know which code ran | a coverage measurement |
| The user enters invalid input | validation + a clear message or retry |
| A function can't produce a result | return failure info (sentinel / optional / exception, per the codebase) |
| A caller violated an assumption | `assert` |
| The assumption is knowable at compile time | `static_assert` |

### On to the lab

Everything in this chapter converges on one exercise: **the Robust Input Desk**. You'll write a small library of pure, individually testable functions — a quantity validator with *inclusive* boundaries, a range clamp guarded by a precondition `assert`, a keystroke parser that hands back a *sentinel* on junk instead of crashing, a stock lookup protected by a `static_assert` on its table size, and a strict digits-only check that reads the whole token before judging it. Then the capstone ties the chapter together: a `readIntInRange` that runs the full `std::cin` recovery loop — snapshot, clear, ignore, decide, and handle EOF — and refuses to crash no matter what the grader throws at it.

The habit to carry forward is the one this whole chapter has been arguing for: **every branch and every boundary is a place a future bug can hide.** Test there first.
