# Chapter 6 — Operators

> Source: <https://www.learncpp.com/> (Chapter 6)
> One file per chapter. Each lesson is a section below.

## Contents
- [6.1 — Operator precedence and associativity](#61--operator-precedence-and-associativity)
- [6.2 — Arithmetic operators](#62--arithmetic-operators)
- [6.3 — Remainder and Exponentiation](#63--remainder-and-exponentiation)
- [6.4 — Increment/decrement operators, and side effects](#64--incrementdecrement-operators-and-side-effects)
- [6.5 — The comma operator](#65--the-comma-operator)
- [6.6 — The conditional operator](#66--the-conditional-operator)
- [6.7 — Relational operators and floating point comparisons](#67--relational-operators-and-floating-point-comparisons)
- [6.8 — Logical operators](#68--logical-operators)
- [6.x — Chapter 6 summary and quiz](#6x--chapter-6-summary-and-quiz)

---

## 6.1 — Operator precedence and associativity

### Operators, operands, and compound expressions

An **operator** tells C++ what operation to perform. An **operand** is the value
or expression the operator works on.

```cpp
int total { 4 + 2 * 3 };
//          ^   ^   ^
//       operands and operators inside one compound expression
```

A simple expression like `2 + 3` has only one obvious grouping. A compound
expression like `4 + 2 * 3` needs rules so the compiler knows whether this means
`(4 + 2) * 3` or `4 + (2 * 3)`.

### Precedence controls grouping between different operators

**Operator precedence** answers: when different operators appear near each
other, which operator binds to its operands first?

```cpp
int a { 4 + 2 * 3 };   // grouped as 4 + (2 * 3), result 10
int b { (4 + 2) * 3 }; // explicit grouping, result 18
```

Multiplication has higher precedence than addition, so `2 * 3` is grouped
before the addition. Parentheses override the default grouping and should be
used when the intended grouping is not obvious.

### Associativity controls grouping between equal-precedence operators

**Operator associativity** answers: when adjacent operators have the same
precedence, should they group from left to right or right to left?

```cpp
int leftToRight { 20 - 5 - 3 }; // grouped as (20 - 5) - 3, result 12

int x {};
int y {};
int z {};
x = y = z = 7; // assignment groups right-to-left: x = (y = (z = 7))
```

Associativity is about operator grouping, not necessarily about which operand is
read first.

### Grouping is not the same as evaluation order

This is the trap in this lesson.

Precedence and associativity tell the compiler how to group operators with
operands. They do **not** generally tell the compiler which operand or function
argument must be evaluated first.

```cpp
int readNumber();
void record(int first, int second, int third);

record(readNumber(), readNumber(), readNumber());
```

The compiler can evaluate those three arguments in an order you did not expect.
If the order matters, split the work into separate statements.

```cpp
int first { readNumber() };
int second { readNumber() };
int third { readNumber() };

record(first, second, third);
```

Mental model:

```
operator grouping:
    "which operands belong to which operator?"

operand evaluation order:
    "which subexpression is actually run first?"

These are related but not the same rule set.
```

### Parenthesize for readers, not just for the compiler

You do not need to memorize the whole precedence table to write good code.
Parenthesize non-trivial expressions so a reader can see the intended grouping.

```cpp
bool shouldInstrument {
    (functionHasBody && !isDeclaration) || forceInstrumentation
};

bool clearer {
    (functionHasBody && (!isDeclaration)) || forceInstrumentation
};
```

For arithmetic-only expressions, normal math grouping is often readable:

```cpp
int bytes { headerBytes + entryCount * bytesPerEntry };
```

For mixed logical, relational, assignment, and conditional expressions, prefer
parentheses even when C++ would technically do the right thing without them.

CS6340 tie-in:

```cpp
bool shouldMutate {
    (remainingMutants > 0) && ((instructionIndex % stride) == 0)
};
```

That expression uses `%`, `==`, `&&`, and parentheses. The parentheses tell a
future reader that the modulo calculation is intentional and that the final
answer is a boolean decision.

### Assignment is a common exception

For a single assignment, the right-hand side normally does not need extra
parentheses:

```cpp
score = baseScore + bonusScore;
```

But once an assignment is nested inside another expression, slow down:

```cpp
while ((line = readLine()) != "") // common pattern in some styles
{
    process(line);
}
```

The assignment is parenthesized because otherwise it is easy to mistake the
expression for a comparison.

---

## 6.2 — Arithmetic operators

### Unary arithmetic operators

Unary arithmetic operators take one operand.

```cpp
int count { 5 };

int same { +count };  // unary plus; rarely useful
int negated { -count }; // unary minus; result -5
```

Place unary `+` or `-` directly against the operand:

```cpp
int debt { -balance }; // readable
```

Do not confuse unary minus with binary subtraction:

```cpp
int value { 10 - -3 }; // 10 minus negative 3, result 13
```

### Binary arithmetic operators

Binary arithmetic operators take left and right operands.

| Operator | Meaning |
|---|---|
| `+` | addition |
| `-` | subtraction |
| `*` | multiplication |
| `/` | division |
| `%` | integer remainder |

```cpp
int a { 9 };
int b { 4 };

int sum { a + b };        // 13
int difference { a - b }; // 5
int product { a * b };    // 36
int quotient { a / b };   // 2, because both operands are int
int remainder { a % b };  // 1
```

### Integer division vs floating-point division

The `/` operator behaves differently depending on operand types.

```cpp
int totalTests { 7 };
int failedTests { 2 };

int integerRatio { failedTests / totalTests }; // 0
double badRatio { failedTests / totalTests };  // 0.0, division already happened as int

double goodRatio {
    static_cast<double>(failedTests) / totalTests
}; // about 0.285714
```

If both operands are integers, C++ performs integer division and discards the
fractional part. Assigning the result to `double` afterward cannot recover the
discarded fraction.

To force floating-point division, make at least one operand floating point:

```cpp
double failureRate {
    static_cast<double>(failedTests) / static_cast<double>(totalTests)
};
```

### Division by zero

Integer division by zero is undefined behavior. Floating-point division by zero
has special IEEE behavior on many platforms, but beginner code should still
treat it as an error unless the design explicitly expects infinities or NaNs.

```cpp
double ratioOrZero(int numerator, int denominator)
{
    if (denominator == 0)
        return 0.0;

    return static_cast<double>(numerator) / denominator;
}
```

### Arithmetic assignment operators

C++ has shorthand operators for updating a variable based on its current value.

```cpp
int bytesProcessed { 0 };

bytesProcessed = bytesProcessed + 16;
bytesProcessed += 16; // equivalent intent, shorter

bytesProcessed -= 4;
bytesProcessed *= 2;
bytesProcessed /= 3;
bytesProcessed %= 10;
```

Use these when they express an update clearly. They are common in counters,
accumulators, indexing code, and instrumentation counters.

```cpp
void recordCoveredBlock(int& coveredBlocks)
{
    coveredBlocks += 1;
}
```

### Arithmetic operators do not modify operands by themselves

Most arithmetic operators compute a new value. They do not change the original
objects unless you assign the result somewhere.

```cpp
int x { 3 };
int y { x + 1 }; // y is 4, x remains 3

x = x + 1;       // now x changes
x += 1;          // same update intent
```

This distinction matters when reading code with side effects in later lessons.

---

## 6.3 — Remainder and Exponentiation

### Remainder is integer-only

The remainder operator `%` returns the remainder left after integer division.

```cpp
int blocks { 17 };
int blocksPerPage { 8 };

int fullPages { blocks / blocksPerPage }; // 2
int leftover { blocks % blocksPerPage };  // 1
```

Useful patterns:

```cpp
bool isMultipleOfFour(int x)
{
    return (x % 4) == 0;
}

bool isEven(int x)
{
    return (x % 2) == 0;
}

bool isOdd(int x)
{
    return (x % 2) != 0;
}
```

Use `!= 0` for oddness rather than `== 1`, because negative odd numbers produce
a negative remainder in C++.

```cpp
// -5 % 2 is -1 in C++, not 1.
bool brokenOddCheck(int x)
{
    return (x % 2) == 1; // false for -5
}
```

### The sign follows the left operand

For `x % y`, the result has the sign of `x`.

```cpp
int a { -13 % 5 }; // -3
int b {  13 % -5 }; // 3
```

That is why LearnCpp prefers the word **remainder** over **modulo** for C++'s
`%` operator. Mathematical modulo is often defined differently for negative
values.

When you need a non-negative wraparound index, write that rule directly:

```cpp
int wrapIndex(int index, int size)
{
    int wrapped { index % size };

    if (wrapped < 0)
        wrapped += size;

    return wrapped;
}
```

CS6340 tie-in:

```cpp
bool shouldSampleInstruction(int instructionIndex, int sampleEvery)
{
    return (sampleEvery > 0) && ((instructionIndex % sampleEvery) == 0);
}
```

This kind of modulo gate is common in fuzzing, sampling, and lightweight
instrumentation logic.

### C++ has no exponent operator

There is no built-in `**` operator in C++. The `^` operator is not exponentiation
in C++; it is bitwise XOR.

```cpp
int wrong { 2 ^ 8 }; // not 256
```

For floating-point powers, use `std::pow` from `<cmath>`:

```cpp
#include <cmath>

double areaScale(double radius)
{
    return std::pow(radius, 2.0);
}
```

`std::pow` is a floating-point function. For exact integer powers, a small loop
is often clearer and avoids converting through floating point:

```cpp
int powInt(int base, int exponent)
{
    int result { 1 };

    for (int i { 0 }; i < exponent; ++i)
        result *= base;

    return result;
}
```

Be aware that integer exponentiation overflows quickly:

```cpp
// 10^10 does not fit in a 32-bit int.
```

If overflow matters, use a wider type and explicit range checks.

---

## 6.4 — Increment/decrement operators, and side effects

### Prefix vs postfix

Increment and decrement are compact ways to add or subtract one.

| Form | What happens |
|---|---|
| `++x` | increment `x`, then produce the updated value |
| `--x` | decrement `x`, then produce the updated value |
| `x++` | copy old `x`, increment `x`, then produce the old value |
| `x--` | copy old `x`, decrement `x`, then produce the old value |

```cpp
int a { 5 };
int b { ++a }; // a is 6, b is 6

int c { 5 };
int d { c++ }; // c is 6, d is 5
```

Diagram:

```
++x:
    x changes first
    expression value is the new x

x++:
    old x is copied
    x changes
    expression value is the old copy
```

If you only need to increment a variable as a statement, prefer prefix:

```cpp
++lineCount;
```

Postfix is useful when the old value is the point:

```cpp
int nextId()
{
    static int s_next { 0 };
    return s_next++; // return current ID, then prepare the next one
}
```

### Side effects

A **side effect** is an observable change beyond producing a value.

Examples:

```cpp
x = 5;          // changes x
++x;           // changes x
std::cout << x; // changes output stream state
```

By contrast, `x + 1` computes a value but does not modify `x`.

```cpp
int x { 4 };
x + 1; // value 5 is computed and discarded; x is still 4
```

### Avoid side-effect order traps

Do not write expressions that depend on the order in which operands or function
arguments are evaluated.

```cpp
int add(int left, int right)
{
    return left + right;
}

int x { 5 };
int bad { add(x, ++x) }; // do not write this
```

The code is unclear because one argument reads `x` while another argument
modifies `x`. Different compilers and optimization settings can expose this kind
of mistake differently.

Write the order as statements:

```cpp
int x { 5 };
int original { x };
++x;

int good { add(original, x) };
```

CS6340 tie-in:

```cpp
// Hard to audit:
recordMutation(mutantId++, chooseTarget(mutantId));

// Clear:
int currentMutant { mutantId };
++mutantId;
recordMutation(currentMutant, chooseTarget(mutantId));
```

Instrumentation code is often read under time pressure. Avoid clever side-effect
expressions where a future bug hunt would have to reconstruct evaluation order.

---

## 6.5 — The comma operator

### The comma operator is not the same as a separator comma

C++ uses the comma character in two different ways.

Separator commas are ordinary syntax:

```cpp
void logPair(int left, int right); // separates parameters
logPair(3, 4);                     // separates arguments
```

The **comma operator** is an actual operator:

```cpp
int x { 1 };
int y { 2 };
int result { (++x, ++y) }; // result is 3
```

The comma operator evaluates its left operand, discards that result, evaluates
its right operand, and produces the right operand's value.

```
(left, right)
   |
   +-- left is evaluated for side effects
   +-- right is evaluated and becomes the expression value
```

### Lowest precedence makes it easy to misread

The comma operator has very low precedence.

```cpp
int a { 1 };
int b { 2 };
int z {};

z = (a, b); // z becomes b, so z == 2
z = a, b;  // grouped as (z = a), b; z == 1 and b is discarded
```

This is one reason the comma operator is usually avoided.

### Prefer separate statements

```cpp
// Avoid:
std::cout << (++x, ++y) << '\n';

// Prefer:
++x;
++y;
std::cout << y << '\n';
```

The main common exception is in `for` loops, where the loop syntax has one slot
for the update expression:

```cpp
for (int left { 0 }, right { 10 }; left < right; ++left, --right)
{
    // process a pair of positions
}
```

Even there, use it only when both updates are small and obvious.

---

## 6.6 — The conditional operator

### `?:` is an expression-level if-else

The conditional operator is the only common C++ operator with three operands:

```cpp
condition ? expressionIfTrue : expressionIfFalse
```

It chooses between two expressions based on a condition.

```cpp
int maxValue(int a, int b)
{
    return (a > b) ? a : b;
}
```

Equivalent statement form:

```cpp
int maxValue(int a, int b)
{
    if (a > b)
        return a;

    return b;
}
```

The conditional operator is most useful when the result is naturally a value.

### It can initialize constants

Because `?:` is an expression, it can be used where statements cannot.

```cpp
constexpr bool debugBuild { true };
constexpr int logLevel { debugBuild ? 3 : 1 };
```

An `if` statement cannot directly initialize one `constexpr` variable in the
same expression position.

### Parenthesize conditionals in compound expressions

The conditional operator has low precedence. Parenthesize it when mixed with
other operators.

```cpp
int printedLimit { (isDebugMode ? debugLimit : releaseLimit) };

std::cout << (isDebugMode ? "debug" : "release") << '\n';
```

Also consider parenthesizing the condition when the condition itself uses
operators:

```cpp
int larger { (left > right) ? left : right };
```

### The two result expressions need compatible types

C++ must be able to determine a single result type.

```cpp
double value { usePrecise ? 1.0 : 1 }; // int can convert to double
```

Avoid mixing signed and unsigned values in conditional operands:

```cpp
int signedFallback { -1 };
unsigned count { 3 };

// Surprising: -1 may be converted to a large unsigned value.
auto selected { useCount ? count : signedFallback };
```

Make the conversion explicit when the types are not naturally the same:

```cpp
int selected {
    useCount ? static_cast<int>(count) : signedFallback
};
```

### Keep it short

Conditional expressions are good for compact choices:

```cpp
std::string_view statusName(bool passed)
{
    return passed ? "passed" : "failed";
}
```

They become hard to read when nested or when either branch does significant
work. Use `if` statements for complex logic.

CS6340 tie-in:

```cpp
std::string_view classifyCoverage(int hitCount)
{
    return (hitCount == 0) ? "uncovered" : "covered";
}
```

That is a good conditional expression because both branches are simple labels.

---

## 6.7 — Relational operators and floating point comparisons

### Relational operators produce bool values

Relational operators compare values and produce `true` or `false`.

| Operator | Meaning |
|---|---|
| `<` | less than |
| `<=` | less than or equal |
| `>` | greater than |
| `>=` | greater than or equal |
| `==` | equal |
| `!=` | not equal |

```cpp
bool isPositive(int x)
{
    return x > 0;
}

bool hasExpectedCount(int actual, int expected)
{
    return actual == expected;
}
```

These are straightforward for integers and booleans.

### Do not compare booleans to `true` or `false`

```cpp
if (isReady)   // good
{
    run();
}

if (!isReady)  // good
{
    wait();
}
```

Avoid:

```cpp
if (isReady == true)  // redundant
{
    run();
}
```

The condition is already boolean.

### Floating-point equality is risky

Floating-point values often contain small representation errors.

```cpp
double a { 0.1 + 0.2 };
double b { 0.3 };

bool exactlyEqual { a == b }; // often false
```

Avoid `==` and `!=` for calculated floating-point values unless exact bit-level
equality is truly what you mean.

### Epsilon comparisons

For many beginner cases, compare the difference against a small absolute
tolerance:

```cpp
#include <cmath>

bool almostEqualAbs(double a, double b, double epsilon)
{
    return std::abs(a - b) <= epsilon;
}
```

But one fixed epsilon is not suitable for all magnitudes. A tolerance that is
reasonable near `1.0` may be too tiny for values near `1'000'000.0` and too large
for values near zero.

A more flexible helper combines absolute and relative tolerance:

```cpp
#include <algorithm>
#include <cmath>

bool almostEqual(double a, double b, double relEpsilon, double absEpsilon)
{
    double diff { std::abs(a - b) };

    if (diff <= absEpsilon)
        return true;

    double largest { std::max(std::abs(a), std::abs(b)) };
    return diff <= (largest * relEpsilon);
}
```

This is more than most simple programs need, but the concept matters: equality
for floating-point calculations is usually "close enough for this problem," not
"same exact stored value."

CS6340 tie-in:

Most LLVM IR counters and indexes are integers, so exact equality is normal:

```cpp
if (coveredBlocks == totalBlocks)
{
    // complete coverage
}
```

If you compute a percentage or probability, switch mental models:

```cpp
double coverage { static_cast<double>(coveredBlocks) / totalBlocks };

if (almostEqual(coverage, 1.0, 1e-12, 1e-12))
{
    // effectively 100 percent
}
```

---

## 6.8 — Logical operators

### Logical operators combine boolean conditions

| Operator | Meaning |
|---|---|
| `!x` | logical NOT: true when `x` is false |
| `x && y` | logical AND: true when both are true |
| `x || y` | logical OR: true when at least one is true |

```cpp
bool canMutate {
    hasFunctionBody && !isDeclaration
};

bool shouldLog {
    verboseMode || hasError
};
```

### Parenthesize `!` around compound conditions

Logical NOT has high precedence.

```cpp
if (!(x > maxAllowed))
{
    // x is not greater than the limit
}
```

Avoid:

```cpp
if (!x > maxAllowed) // grouped as (!x) > maxAllowed
{
}
```

### Short-circuit evaluation

`&&` and `||` are short-circuit operators.

For `&&`, if the left side is false, the whole expression is false and the right
side is not evaluated.

```cpp
if ((ptr != nullptr) && ptr->isReady())
{
    use(*ptr);
}
```

The null check must come first. If `ptr` is null, `ptr->isReady()` is never
called.

For `||`, if the left side is true, the whole expression is true and the right
side is not evaluated.

```cpp
if (isCached || loadFromDisk())
{
    runAnalysis();
}
```

This can be useful, but do not hide important side effects in the right-hand
side unless the short-circuit behavior is central to the logic.

### De Morgan's laws

De Morgan's laws help rewrite negated compound conditions:

```cpp
!(a && b) == (!a || !b)
!(a || b) == (!a && !b)
```

Example:

```cpp
bool invalid {
    !(hasInput && hasOutput)
};

bool sameMeaning {
    !hasInput || !hasOutput
};
```

These rewrites are useful when a condition reads awkwardly.

### Logical XOR

C++ does not have a dedicated logical XOR operator. For boolean operands,
`!=` works as "exactly one is true."

```cpp
bool exactlyOneMode {
    useFileInput != useStdin
};
```

Only use this when both operands are already `bool`. If they are integers or
pointers, convert intentionally.

### Alternative operator spellings

C++ supports keyword alternatives such as `and`, `or`, and `not`:

```cpp
if (ready and not failed)
{
    run();
}
```

Most C++ codebases use the symbolic forms:

```cpp
if (ready && !failed)
{
    run();
}
```

For CS6340 and LLVM code, expect `&&`, `||`, and `!`.

### Logical conditions in instrumentation code

Instrumentation and mutation code often has guard conditions that must be both
correct and readable.

```cpp
bool shouldInsertCounter {
    (functionHasBody && !isDeclaration) &&
    (instructionIsTerminator || instructionMayExecute)
};
```

The exact LLVM predicates will vary, but the shape is common:

```
eligible program location
AND
one of the desired target cases
```

Parentheses make the intent easy to audit.

---

## 6.x — Chapter 6 summary and quiz

### Summary

- Precedence and associativity determine expression grouping, not general
  operand evaluation order.
- Parenthesize non-trivial expressions so the next reader does not need a
  precedence table.
- Integer division discards fractional results. Cast before division when a
  fractional answer is needed.
- `%` is a remainder operator for integers. With negative operands, the result
  has the sign of the left operand.
- C++ has no exponentiation operator. `^` is not exponentiation.
- Prefer prefix `++x` / `--x` unless you specifically need the previous value.
- Avoid expressions that both read and modify the same variable in unclear
  orders.
- Avoid the comma operator except in narrow `for` loop update cases.
- Use `?:` for simple expression-level choices, not complex branching.
- Do not compare calculated floating-point values with exact equality unless
  exact representation equality is truly intended.
- Use logical operators with short-circuit behavior intentionally.
- Use De Morgan's laws to simplify negated compound boolean expressions.

### Lab-facing takeaways

For CS6340 work, this chapter mainly affects correctness in predicates:

```cpp
bool selected {
    (candidateCount > 0) &&
    ((instructionIndex % sampleEvery) == 0) &&
    !isIgnoredInstruction
};
```

The mechanics are small, but they decide whether instrumentation fires in the
right places. Write these expressions so they can be reviewed without guessing.
