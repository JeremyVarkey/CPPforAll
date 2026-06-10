# Chapter 9 — Error Detection and Handling

> Source: <https://www.learncpp.com/> (Chapter 9)
> One file per chapter. Each lesson is a section below.

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

### Why "it compiled" is not enough

A compiling program has passed the compiler's syntax and type checks. It has not
proven that the program does what the programmer intended.

```cpp
int clampPercent(int value)
{
    if (value < 0)
        return 0;
    if (value > 100)
        return 100;

    return 10; // compiles, but should have returned value
}
```

The compiler can see that this function returns an `int`. It generally cannot
know the intended rule: "values already in range should be returned unchanged."

Testing is the process of running code under selected conditions and checking
whether actual behavior matches expected behavior. For CS6340, this chapter is a
direct bridge into software testing vocabulary: unit tests, coverage, sad paths,
assertions, and robust input handling.

### Testing gets hard because paths multiply

Every input, branch, loop, and state variable increases the number of possible
executions.

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

Testing every possible `int` is pointless. Testing representative categories is
useful:

| Input | Category being checked |
|---|---|
| `-1` | invalid negative input |
| `0` | lower valid boundary |
| `59` | failing boundary |
| `60` | passing boundary |
| `89` | passing upper boundary |
| `90` | excellent boundary |

Good testing is not "try random values until you feel better." It is a deliberate
attempt to cover meaningful paths and boundaries.

### Scope creep increases test responsibility

Scope creep happens when a program grows beyond the originally intended feature
set. A quick function may become reused in more places; a program that initially
accepted one simple input may later accept many.

```
one-off script
    -> reused helper
        -> accepts external input
            -> used by other code
                -> needs durable tests
```

The more other code depends on a function, the more valuable preserved tests
become.

### Test small units first

A **unit test** checks a small piece of code in isolation, typically one function
or one class behavior. Testing small units early narrows the search space when
something fails.

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

Temporary manual test:

```cpp
std::cout << isMutationOperator('A') << '\n'; // expected true
std::cout << isMutationOperator('x') << '\n'; // expected false
```

That is useful while writing, but it is easy to delete, forget, or misread.

### Preserve tests so they can be rerun

A simple preserved test function keeps expected behavior near the code.

```cpp
int testIsMutationOperator()
{
    if (!isMutationOperator('A')) return 1;
    if (!isMutationOperator('R')) return 2;
    if (!isMutationOperator('D')) return 3;
    if (isMutationOperator('x')) return 4;
    if (isMutationOperator('\0')) return 5;

    return 0;
}
```

Calling the test:

```cpp
int main()
{
    const int failedCase { testIsMutationOperator() };

    if (failedCase != 0)
        std::cerr << "testIsMutationOperator failed case "
                  << failedCase << '\n';
}
```

This is still not a full unit testing framework, but it captures the important
idea: the test stores both the operation and the expected result.

### Automate expected-answer checks

Manual output inspection does not scale. A test that prints "looks right" still
requires a human to compare output.

Better:

```cpp
bool testClampPercent()
{
    return clampPercent(-5) == 0
        && clampPercent(0) == 0
        && clampPercent(42) == 42
        && clampPercent(100) == 100
        && clampPercent(250) == 100;
}
```

The test returns a pass/fail result that can be wired into a script, CI job, or
larger test harness.

### Unit vs integration testing

| Test type | What it checks | Example |
|---|---|---|
| Unit test | A small piece in isolation | `parseOperator("+")` returns `Add` |
| Integration test | Pieces working together | read input, parse operator, compute result, print output |

Integration tests are valuable, but they are harder to diagnose. If an
integration test fails, the bug could be in any participating unit or in the way
the units connect.

CS6340 tie-in:

```
unit:        does this mutation operator transform one line correctly?
integration: does the fuzzer generate, run, and classify a mutant correctly?
```

---

## 9.2 — Code coverage

### Coverage answers "what ran?"

**Code coverage** measures how much source code was executed by tests. Coverage
does not prove correctness; it only tells you which code your tests reached.

```
coverage can say:      this branch ran
coverage cannot say:   this branch produced the right answer
```

High coverage with weak assertions is still weak testing.

### Statement coverage

**Statement coverage** asks whether each statement has executed.

```cpp
int maxOfTwo(int a, int b)
{
    int result { b };

    if (a > b)
        result = a;

    return result;
}
```

Calling `maxOfTwo(7, 3)` executes every statement:

```
int result { b };   yes
if (a > b)          yes
result = a;         yes
return result;      yes
```

But it does not test what happens when `a <= b`. Statement coverage can miss
untaken branches.

### Branch coverage

**Branch coverage** asks whether every branch outcome has executed.

For the same function:

```cpp
maxOfTwo(7, 3); // if condition true
maxOfTwo(3, 7); // if condition false
```

Now both paths through the `if` have been tested.

Diagram:

```
                 a > b ?
                /      \
            true        false
             |            |
        result = a     keep b
             \          /
              return result
```

Aim for branch coverage when testing decision-heavy code. It is usually more
meaningful than statement coverage.

### Branches without visible `else`

An `if` without an `else` still has two branch outcomes:

```cpp
if (enabled)
    runCheck();
```

Tests should cover:

- `enabled == true`: `runCheck()` executes.
- `enabled == false`: `runCheck()` does not execute.

The absence of an `else` body does not mean there is no false path.

### Loop coverage: the 0, 1, 2 rule

For loops, useful coverage usually means testing:

| Iterations | Why it matters |
|---|---|
| 0 | Does the loop skip cleanly? |
| 1 | Does the body work once? |
| 2 | Does state update correctly across iterations? |

```cpp
int sumFirstN(int n)
{
    int total { 0 };

    for (int value { 1 }; value <= n; ++value)
        total += value;

    return total;
}
```

Representative tests:

```cpp
sumFirstN(0) == 0; // zero iterations
sumFirstN(1) == 1; // one iteration
sumFirstN(2) == 3; // two iterations
```

If the two-iteration case works, the update step has at least demonstrated that
the loop can move from one iteration to the next.

### Category testing

Input domains are usually too large to test exhaustively. Group inputs into
categories that should behave similarly.

For integers:

- negative,
- zero,
- positive,
- boundary values,
- values that could overflow.

For floating-point values:

- exact-looking values such as `0.5`,
- precision-stress values such as `0.1`,
- negative values,
- values near comparison thresholds.

For strings:

- empty string,
- normal alphanumeric text,
- leading/trailing whitespace,
- embedded whitespace,
- all whitespace,
- very long text.

For pointers:

- valid pointer,
- `nullptr` if the function accepts a pointer and null is possible.

CS6340 tie-in:

```
fuzzer seed categories:
    empty input
    one-token input
    nested expression
    malformed expression
    very long input
    input with whitespace/control characters
```

Coverage tells you which code ran. Category thinking helps decide which tests
are worth writing.

---

## 9.3 — Common semantic errors in C++

### Syntax errors vs semantic errors

A **syntax error** violates C++ grammar and is normally caught by the compiler.

```cpp
int x { 3 // missing closing brace/semicolon
```

A **semantic error** is valid C++ that means something different from what the
programmer intended.

```cpp
int average(int total, int count)
{
    return total / count; // maybe wrong if caller expected fractional average
}
```

Semantic errors are more dangerous because the program can compile and run.

### Conditional logic errors

These happen when the condition does not match the intended rule.

```cpp
bool isAdult(int age)
{
    return age > 18; // probably should be age >= 18
}
```

Boundary tests catch many of these:

```cpp
isAdult(17) == false;
isAdult(18) == true;
isAdult(19) == true;
```

### Infinite loops

An infinite loop occurs when the loop condition never becomes false.

```cpp
int countdown { 3 };

while (countdown > 0)
{
    std::cout << countdown << '\n';
    // missing --countdown
}
```

Unsigned counters can also create accidental infinite loops:

```cpp
for (unsigned int i { 3 }; i >= 0; --i)
{
    std::cout << i << '\n';
}
```

`i >= 0` is always true for an unsigned value. After `0`, the value wraps to a
large unsigned number.

Prefer signed counters unless unsigned arithmetic is actually part of the model.

### Off-by-one errors

An **off-by-one error** means a loop or range includes one too many or one too
few elements.

```cpp
for (int i { 0 }; i <= length; ++i) // wrong for 0-based indexing
{
    process(i);
}
```

For `length == 5`, valid indices are:

```
0 1 2 3 4
```

The condition `i <= length` also permits `5`, which is outside the range.

Common patterns:

```cpp
for (int i { 0 }; i < length; ++i)     // 0-based count
for (int n { 1 }; n <= limit; ++n)     // 1-based sequence
```

Use whichever pattern matches the domain, then test boundaries.

### Operator precedence mistakes

C++ operators do not all bind in the order people casually read them.

```cpp
if (!ready == expected)
{
    // parsed as (!ready) == expected
}
```

If the intended meaning is "ready is not equal to expected", write it directly:

```cpp
if (ready != expected)
{
    // clearer
}
```

When mixing `&&` and `||`, use parentheses to make the grouping explicit:

```cpp
if ((isAdmin || isOwner) && accountEnabled)
{
    allowAccess();
}
```

### Floating-point precision

Floating-point numbers approximate many decimal values. Equality checks can fail
after arithmetic even when the math looks exact.

```cpp
double total { 0.1 + 0.2 };

if (total == 0.3) // fragile
    std::cout << "exact\n";
```

Use an approximate comparison when comparing computed floating-point values:

```cpp
#include <cmath>

bool nearlyEqual(double a, double b, double epsilon)
{
    return std::abs(a - b) <= epsilon;
}
```

### Integer division

If both operands are integers, `/` performs integer division.

```cpp
int passed { 7 };
int total { 10 };

double rate { passed / total }; // 0, then converted to 0.0
```

Convert before dividing:

```cpp
double rate { static_cast<double>(passed) / total }; // 0.7
```

The cast belongs at the arithmetic boundary, not after the integer result has
already been computed.

### Accidental null statements

A stray semicolon can make an `if` or loop body empty.

```cpp
if (shouldRun);
{
    runExpensiveCheck(); // always runs
}
```

The semicolon is a complete do-nothing statement. The block below it is not
controlled by the `if`.

### Missing braces around multi-statement bodies

Without braces, only the next statement belongs to the `if`, loop, or `else`.

```cpp
if (isValid)
    std::cout << "valid\n";
    saveResult(); // always runs
```

Prefer braces once a body is more than one statement, and consider using braces
consistently even for one-line bodies in code that changes often.

### Assignment instead of equality

```cpp
char answer { 'n' };

if (answer = 'y') // assigns, then tests assigned value
{
    launch();
}
```

This compiles because assignment expressions produce a value. Many compilers can
warn about this, but only if warnings are enabled and treated seriously.

Correct:

```cpp
if (answer == 'y')
{
    launch();
}
```

### Forgetting the function call operator

Using a function name without `()` refers to the function itself rather than
calling it.

```cpp
int getLimit()
{
    return 10;
}

if (getLimit) // wrong: did not call getLimit()
{
    std::cout << "truthy function pointer\n";
}
```

Correct:

```cpp
if (getLimit() > 0)
{
    std::cout << "positive limit\n";
}
```

### How this connects to testing

Common semantic errors usually sit at predictable places:

```
branch condition
loop condition/update
boundary index
operator grouping
numeric conversion
input validation
```

When reviewing or testing C++ code, spend extra attention there.

---

## 9.4 — Detecting and handling errors

### Bugs vs expected error cases

Some errors are programmer mistakes:

```cpp
// Programmer accidentally used the wrong comparison.
if (count <= maxIndex) { ... }
```

Other errors are expected realities of running software:

```cpp
// User supplied a filename that does not exist.
std::ifstream input { filename };
```

The first kind should be fixed. The second kind should be detected and handled.

### Happy paths and sad paths

The **happy path** is the ordinary no-error path.

The **sad path** is what happens when something goes wrong:

- input cannot be parsed,
- file cannot be opened,
- lookup misses,
- argument is outside the valid range,
- external service is unavailable.

Robust programs are designed for both.

```
parse input
   |
   +-- valid ----> compute result ----> print result
   |
   +-- invalid --> explain/retry/return failure
```

### Three common assumption points

Assumptions often fail when:

1. A function call is assumed to have succeeded.
2. Input is assumed to have the right format and meaning.
3. Function arguments are assumed to be semantically valid.

Example:

```cpp
int divideRoundedDown(int numerator, int denominator)
{
    return numerator / denominator;
}
```

This function assumes `denominator != 0`. If that is not guaranteed by the
caller, the function needs a strategy.

### Strategy 1: handle the error locally

If the function can fix or contain the problem, handle it there.

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

This is appropriate because input collection owns the retry policy.

### Strategy 2: return failure information to the caller

If the function cannot decide what to do, report the failure.

```cpp
bool tryDivide(int numerator, int denominator, int& result)
{
    if (denominator == 0)
        return false;

    result = numerator / denominator;
    return true;
}
```

Caller:

```cpp
int quotient {};

if (tryDivide(10, value, quotient))
    std::cout << quotient << '\n';
else
    std::cerr << "Cannot divide by zero\n";
```

The calculation code does not prompt the user or terminate the program. It gives
the caller enough information to choose a policy.

### Sentinel values

A **sentinel value** is a special value that means something outside the normal
result set.

```cpp
constexpr int notFound { -1 };

int findFirstSpace(std::string_view text)
{
    for (int i { 0 }; i < static_cast<int>(text.length()); ++i)
    {
        if (text[static_cast<std::size_t>(i)] == ' ')
            return i;
    }

    return notFound;
}
```

Sentinels are simple, but they are easy to misuse:

- the caller must remember to check,
- the sentinel must not be a valid normal result,
- the meaning may be unclear without a named constant.

Later notes prefer `std::optional<T>` when "maybe no value" is the real model.

### Strategy 3: halt on fatal errors

A **fatal** or **non-recoverable** error is severe enough that continuing would
make the program unreliable.

```cpp
int loadRequiredConfig()
{
    std::ifstream config { "required.cfg" };

    if (!config)
    {
        std::cerr << "required.cfg is missing\n";
        return 1; // from main, nonzero status means failure
    }

    return 0;
}
```

When possible, return from `main` with a nonzero exit code. Deep inside helper
functions, programs sometimes use halt functions such as `std::exit`, but doing
so couples that helper to whole-program policy.

### Strategy 4: throw an exception

Exceptions are C++'s structured mechanism for sending an error up the call stack
until some caller handles it. LearnCpp covers exceptions later. At this point,
know the role:

```
detect error in lower function
    -> throw
        -> caller can catch
            -> if nobody catches, program terminates
```

Do not use exceptions just because returning errors feels inconvenient. Use them
where the surrounding codebase treats exceptions as the normal error channel.

### `std::cout`, `std::cerr`, and logs

| Output | Use for |
|---|---|
| `std::cout` | normal user-facing output |
| `std::cerr` | diagnostics and error output |
| log file | durable event history for later inspection |

Interactive beginner programs often show user-facing input errors with
`std::cout` because the message is part of the normal interaction. Command-line
tools usually send error output to `std::cerr` so normal output and diagnostics
can be redirected separately.

CS6340 tie-in:

```cpp
std::cout << mutantResult << '\n';              // machine-readable result
std::cerr << "failed to parse seed file\n";     // diagnostic
```

Keeping these streams separate makes automated grading and scripting easier.

---

## 9.5 — std::cin and handling invalid input

### Why text input is hard

Users can type anything. A program asking for an `int` may receive:

```
42
42abc
abc
999999999999999999999999
<end-of-file>
```

A robust program decides which cases are acceptable and what happens otherwise.

### Simplified extraction model

For formatted extraction:

```cpp
std::cin >> value;
```

The extraction operator roughly does this:

1. Discards leading whitespace.
2. Waits for input if the buffer is empty.
3. Extracts characters that fit the target type.
4. Leaves unextracted characters in the input buffer.
5. Sets stream state to failure if nothing valid can be extracted.

ASCII view:

```
user types:     123abc\n
read int:       123
left in buffer: abc\n
```

That leftover input can affect the next extraction.

### Input validation

**Input validation** checks whether input matches what the program expects.

Three broad approaches:

| Approach | Idea | `std::cin` beginner use? |
|---|---|---|
| Inline validation | reject bad keystrokes immediately | usually not available |
| Read as string, parse yourself | validate full text before conversion | powerful, more work |
| Let `operator>>` extract, then handle stream state | use `std::cin` mechanics | common in these notes |

### Error case 1: extraction succeeds but value is meaningless

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

`std::cin` successfully reads a `char`, but the program still has to check
whether that character has domain meaning.

### Error case 2: extraction succeeds with extra input

If the program expects one number per prompt and the user types `12x`, extraction
to `int` can read `12` and leave `x\n` behind.

Use an ignore helper to discard the rest of the line:

```cpp
#include <limits>

void ignoreLine()
{
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}
```

Then:

```cpp
int readIntLoosely()
{
    int value {};
    std::cin >> value;
    ignoreLine();
    return value;
}
```

This accepts `12x` as `12`. Sometimes that is fine; sometimes extra characters
should be treated as invalid. Decide based on the program's input contract.

### Error case 3: extraction fails

If the user types `abc` when an `int` is expected, extraction fails.

Important stream facts:

- The invalid characters remain in the buffer.
- `std::cin` enters a failure state.
- Future extractions fail immediately until the stream is cleared.

Recovery pattern:

```cpp
if (!std::cin)
{
    std::cin.clear(); // reset stream state
    ignoreLine();     // discard bad input
}
```

Order matters. If the stream is still failed, `ignoreLine()` will not behave the
way you need.

### A reusable integer reader

```cpp
int readIntInRange(int min, int max)
{
    while (true)
    {
        std::cout << "Enter value [" << min << ", " << max << "]: ";

        int value {};
        std::cin >> value;

        const bool extracted { static_cast<bool>(std::cin) };
        std::cin.clear();
        ignoreLine();

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

This handles:

- text where a number was expected,
- leftover characters after extraction,
- semantically invalid but syntactically valid numbers.

### EOF

EOF means no more input is available. For files, reaching EOF can be normal. For
`std::cin`, a user can request EOF with an operating-system-specific key
combination.

If input is inside a `while (true)` retry loop, blindly clearing EOF can create
an infinite loop. If EOF means the user is done, exit the input routine or
terminate gracefully.

```cpp
if (std::cin.eof())
{
    std::cerr << "Input closed\n";
    std::exit(0);
}
```

For small programs, it is enough to remember that failed extraction and EOF are
not exactly the same problem.

---

## 9.6 — Assert and static_assert

### Preconditions, invariants, and postconditions

| Term | Meaning | Example |
|---|---|---|
| Precondition | Must be true before code runs | divisor is not zero |
| Invariant | Must remain true while code runs | index stays inside bounds |
| Postcondition | Must be true after code runs | returned value is sorted |

Preconditions are often checked at the top of a function.

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

This early-return style keeps error checks near the condition and avoids nesting
the main logic.

### Assertions document impossible states

An **assertion** is an expression that should be true unless there is a bug in
the program.

```cpp
#include <cassert>

int getBucket(int percent)
{
    assert(percent >= 0 && percent <= 100);
    return percent / 10;
}
```

If correct code only calls `getBucket()` with a percentage, then values outside
`0..100` indicate a programmer mistake. The assertion catches that mistake while
debugging.

### `assert` is a runtime debug check

`assert(condition)` is a preprocessor macro from `<cassert>`.

When assertions are enabled:

- the condition is evaluated at runtime,
- a false condition prints diagnostic information,
- the program aborts.

When `NDEBUG` is defined before including `<cassert>`, standard `assert` checks
are disabled.

```
debug build:    assert(percent <= 100) is checked
release build:  assert(percent <= 100) may be compiled out
```

Do not put side effects inside an assertion:

```cpp
assert(++index < size); // bad: increment may disappear in release builds
```

Prefer:

```cpp
++index;
assert(index < size);
```

### More descriptive assertions

Plain assertions can be cryptic:

```cpp
assert(found);
```

Use a string literal with `&&` when more context helps:

```cpp
assert(found && "expected target instruction to exist before instrumentation");
```

The string literal is true, so it does not change the logic. It does appear in
the assertion expression when the assertion fails.

### `static_assert`

`static_assert` checks a condition at compile time.

```cpp
static_assert(sizeof(int) >= 4, "this code expects at least 32-bit int");

int main()
{
}
```

Key properties:

- the condition must be a constant expression,
- no header is needed,
- failure is a compile error,
- release builds do not disable it,
- there is no runtime cost.

Use `static_assert` whenever the condition can be checked at compile time.

CS6340-style examples:

```cpp
enum class MutationKind
{
    ReplaceOp,
    DeleteStmt,
    InsertGuard,
    Count
};

static_assert(static_cast<int>(MutationKind::Count) == 3,
              "update mutation dispatch table");
```

If a new mutation kind is added, this assertion can force the dispatch table to
be updated.

### Assertion vs error handling

Use assertions for conditions that should never occur in correct code.

```cpp
void visitInstruction(Instruction *instruction)
{
    assert(instruction != nullptr);
    // ...
}
```

Use error handling for conditions that can happen during normal operation.

```cpp
std::ifstream input { path };

if (!input)
{
    std::cerr << "could not open " << path << '\n';
    return false;
}
```

Sometimes use both:

```cpp
double inverse(double x)
{
    assert(x != 0.0 && "caller should have filtered zero");

    if (x == 0.0)
        return 0.0; // release-build fallback policy

    return 1.0 / x;
}
```

The assertion catches a programmer error during development. The runtime check
keeps release behavior from becoming undefined or explosive if the impossible
case still happens.

---

## 9.x — Chapter 9 summary and quiz

### Core takeaways

- A compiling program can still be semantically wrong.
- Unit tests check small pieces in isolation.
- Integration tests check multiple pieces working together.
- Preserve tests when behavior matters long-term.
- Automate expected-answer checks instead of relying only on manual output.
- Statement coverage asks whether statements ran.
- Branch coverage asks whether each branch outcome ran.
- Loop coverage should include 0, 1, and 2 iterations.
- Category testing picks representative inputs from meaningful groups.
- Semantic errors often hide in conditions, loops, boundaries, conversions, and
  missing braces.
- Robust programs test both happy paths and sad paths.
- Error handling is for cases that can happen in normal operation.
- Assertions are for programmer mistakes and impossible states.
- `static_assert` is better than `assert` when the condition is compile-time
  knowable.
- `std::cin` failure recovery usually means detect failure, `clear()`, then
  discard bad buffered input.

### Testing checklist

For a function, ask:

- What are the normal inputs?
- What are the boundary inputs?
- What inputs are semantically invalid?
- Does every branch run in at least one test?
- Do loops have 0, 1, and 2 iteration tests?
- Does the function report failure clearly?
- Are impossible programmer mistakes documented with assertions?

### Decision table

| Situation | Prefer |
|---|---|
| Small behavior should stay correct | preserved unit test |
| Multiple functions must cooperate | integration test |
| Need to know which code ran | coverage measurement |
| User enters invalid input | validation and retry/error message |
| Function cannot produce result | return failure info / optional / exception policy |
| Programmer violated an assumption | `assert` |
| Compile-time assumption | `static_assert` |

### CS6340 bridge

This chapter is unusually relevant to software analysis and testing:

- Coverage vocabulary maps directly to test adequacy.
- Semantic errors are exactly the kind of faults mutation testing tries to
  simulate.
- Assertions express assumptions that static or dynamic analysis can reason
  about.
- Robust input handling matters for fuzzers because fuzzers intentionally
  generate malformed input.

```
test input -> program path -> observed behavior
     |             |                 |
 categories     coverage        oracle/assertion
```

The testing habit to build now: every branch and boundary is a place where a
future bug can hide.
