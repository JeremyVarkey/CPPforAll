# Chapter 6 — Operators

> Source: <https://www.learncpp.com/> (Chapter 6)

You already know what `+` and `*` do — you learned that in grade school. So why a whole chapter on operators? Because the moment you write more than one operator in a single line, a quiet question appears: *which one happens first?* And once you start asking a program to make decisions — is this number even, is this score high enough, are these two measurements close enough to call equal — you need operators that produce yes/no answers, and you need to know their failure modes. This chapter is about the small mechanics that decide whether an expression means what you think it means. They are easy to learn and easy to get subtly wrong, and a wrong operator rarely announces itself with a compiler error — it just quietly computes the wrong thing. By the end you'll be able to read and write any expression with confidence, and you'll know the handful of traps (integer division, the `^` that isn't exponentiation, floating-point equality) that catch nearly everyone the first time.

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

### Operators and operands

Two words first, because we'll use them constantly. An **operator** is the symbol that tells C++ what operation to perform — `+`, `-`, `*`, `/`, `=`, and the comparison and logical operators we'll meet later. An **operand** is the value the operator works on. In `4 + 2`, the `+` is the operator and `4` and `2` are its operands.

Operators are classified by how many operands they take. A **unary** operator takes one (`-x` negates a single value). A **binary** operator takes two — most of the ones you know, like `a + b`. There's exactly one common **ternary** operator (three operands), the conditional operator, which gets its own lesson later.

A single value like `2 + 3` is an easy case: there's only one operator, one obvious grouping. The interesting question shows up the moment you combine operators into a **compound expression**:

```cpp
int total { 4 + 2 * 3 };
//          ^   ^   ^
//      operands and operators inside one compound expression
```

Does that mean `(4 + 2) * 3`, which is `18`? Or `4 + (2 * 3)`, which is `10`? The two answers are different, so C++ needs an unambiguous rule. It has two: precedence and associativity.

### Precedence decides between *different* operators

**Operator precedence** answers the question: when operators of *different* kinds sit next to each other, which one grabs its operands first? Each operator in C++ has a precedence level. Higher-precedence operators bind to their operands before lower-precedence ones — exactly the "multiplication before addition" rule you already carry around in your head.

```cpp
int a { 4 + 2 * 3 };   // grouped as 4 + (2 * 3), result 10
int b { (4 + 2) * 3 }; // explicit grouping, result 18
```

Multiplication has higher precedence than addition, so `2 * 3` is grouped first and the result is `10`. Parentheses override the default: whatever you put inside them is grouped first, regardless of precedence. That's your escape hatch whenever the built-in rule isn't the one you want — or simply isn't obvious to a reader.

### Associativity decides between *equal*-precedence operators

Precedence settles fights between *different* operators. But what about a row of operators at the *same* precedence level?

```cpp
int result { 20 - 5 - 3 };
```

Subtraction has a single precedence level, so precedence alone can't tell us whether this is `(20 - 5) - 3` or `20 - (5 - 3)`. The first gives `12`; the second gives `18`. **Associativity** is the tie-breaker: it says whether equal-precedence operators group left-to-right or right-to-left.

Subtraction is **left-to-right** associative, so the row groups as `(20 - 5) - 3`, which is `12` — the answer you'd expect. Most operators work this way. A notable exception is assignment, which is **right-to-left**:

```cpp
int x {};
int y {};
int z {};
x = y = z = 7; // groups as x = (y = (z = 7))
```

Here `z = 7` happens first (and, as we'll see in a later lesson, the assignment itself produces a value — `7`), then `y` is assigned that value, then `x`. The whole chain ends with all three equal to `7`. That only works because assignment associates rightward.

> **Note:** Associativity is about *operator grouping* — which operands belong to which operator. It is **not** a promise about which operand the compiler reads first. That distinction is the heart of the next point, and it's the trap of this lesson.

### Grouping is not the same as evaluation order

Here's the subtlety that trips up even experienced programmers. Precedence and associativity tell the compiler how to *group* operators with their operands. They do **not**, in general, tell the compiler the order in which to actually *evaluate* those operands.

Consider three function calls used as arguments:

```cpp
int readNumber();
void record(int first, int second, int third);

record(readNumber(), readNumber(), readNumber());
```

Grouping is not in question here — each `readNumber()` call clearly belongs to one argument slot. But *which call runs first?* C++ does not require left-to-right here. The compiler is free to call them in an order you didn't expect, and different compilers (or the same compiler at different optimization levels) may differ. If `readNumber()` has a side effect — say it reads the next value from input — then the order genuinely changes the result, and your program's behavior becomes a coin toss across toolchains.

The fix is simple and worth internalizing now: when order matters, don't bury it inside one expression. Spell it out as separate statements, where order is guaranteed top-to-bottom.

```cpp
int first { readNumber() };
int second { readNumber() };
int third { readNumber() };

record(first, second, third);
```

Hold onto this mental separation:

- **Operator grouping** — "which operands belong to which operator?" — governed by precedence and associativity.
- **Operand evaluation order** — "which subexpression actually runs first?" — *not* generally governed by them.

They are related ideas but different rule sets, and conflating them is the source of some genuinely baffling bugs. We'll return to it in lesson 6.4 once we have side effects to make it concrete.

### Parenthesize for the reader, not just the compiler

There is a precedence table with dozens of rows, and you do *not* need to memorize it. The professional habit is the opposite of memorization: when an expression's grouping isn't immediately obvious, **add parentheses** so neither you nor the next reader has to consult a table.

```cpp
bool shouldInstrument {
    (functionHasBody && !isDeclaration) || forceInstrumentation
};
```

C++ would group this correctly without the parentheses (`&&` binds tighter than `||`), but the parentheses make the intent legible at a glance: "an eligible location, OR a forced override." For plain arithmetic, ordinary math grouping is usually readable enough on its own:

```cpp
int bytes { headerBytes + entryCount * bytesPerEntry };
```

But the moment you mix logical, relational, assignment, or conditional operators in one expression, prefer parentheses even where C++ technically doesn't need them. A representative example from the kind of guard conditions you'll write in the lab:

```cpp
bool shouldMutate {
    (remainingMutants > 0) && ((instructionIndex % stride) == 0)
};
```

That single expression uses `%`, `==`, `&&`, and parentheses. The grouping is correct without all of them — but with them, a future reader instantly sees that the modulo is a deliberate "every Nth" gate and the final answer is a boolean decision. You're writing for the person who has to audit this at 2 a.m., and that person might be you.

> **Best practice:** Use parentheses to make the grouping of any non-trivial compound expression obvious, rather than relying on a reader (or yourself) to recall the precedence table.

### Assignment is the common exception

You don't need to wrap the right-hand side of an ordinary assignment in parentheses — the `=` has very low precedence, so everything to its right groups first anyway:

```cpp
score = baseScore + bonusScore; // fine as written
```

But the instant an assignment is *nested inside* a larger expression, slow down and parenthesize it:

```cpp
while ((line = readLine()) != "")
{
    process(line);
}
```

The inner parentheses are doing real work. Without them, `line = readLine() != ""` would compare `readLine()` to `""` first (`!=` binds tighter than `=`) and assign the *boolean* result to `line` — almost certainly not what you meant. The parentheses also flag to the reader "yes, this is an assignment inside a condition, on purpose," which is the kind of thing that otherwise reads as a typo for `==`.

---

## 6.2 — Arithmetic operators

### Unary arithmetic operators

A **unary** arithmetic operator takes a single operand. There are two:

```cpp
int count { 5 };

int same { +count };    // unary plus: produces +5, rarely useful
int negated { -count }; // unary minus: produces -5
```

Unary plus exists mostly for symmetry; it leaves a value unchanged and you'll seldom write it. Unary minus negates. Place either one directly against its operand, with no space, so it reads as part of the value rather than as a stray binary operator:

```cpp
int debt { -balance }; // clearly "negative balance"
```

Be careful not to confuse a unary minus with binary subtraction when they appear together — they look alike but mean different things:

```cpp
int value { 10 - -3 }; // 10 minus negative-3, result 13
```

The first `-` is subtraction (binary); the second is negation (unary). The space between them is what keeps it readable.

### Binary arithmetic operators

The **binary** arithmetic operators take a left and a right operand and should be familiar:

| Operator | Meaning |
|---|---|
| `+` | addition |
| `-` | subtraction |
| `*` | multiplication |
| `/` | division |
| `%` | remainder (integers only) |

```cpp
int a { 9 };
int b { 4 };

int sum { a + b };        // 13
int difference { a - b }; // 5
int product { a * b };    // 36
int quotient { a / b };   // 2  — note: not 2.25
int remainder { a % b };  // 1
```

The `+`, `-`, and `*` operators hold no surprises. Division and remainder, however, both have important wrinkles — division here, remainder in the next lesson.

### Integer division versus floating-point division

The `/` operator behaves differently depending on the *types* of its operands, and this is one of the most common sources of "the math is wrong but I don't see why" bugs.

```cpp
int totalTests { 7 };
int failedTests { 2 };

int integerRatio { failedTests / totalTests }; // 0
double badRatio  { failedTests / totalTests };  // 0.0 — too late!
```

When **both** operands are integers, C++ performs **integer division**: it computes the quotient and throws away any fractional part. `2 / 7` is `0` with a remainder we're discarding, so `integerRatio` is `0`. The painful part is `badRatio`. You might expect `0.285…` because you stored it in a `double` — but the division ran *first*, as integer division, producing `0`, and only *then* was that `0` widened to `0.0`. Assigning to a `double` cannot recover a fraction that was already discarded.

To get floating-point division, at least one operand must be a floating-point type. The cleanest way to arrange that is to `static_cast` one operand before the division:

```cpp
double goodRatio {
    static_cast<double>(failedTests) / totalTests
}; // about 0.285714
```

Once one operand is a `double`, C++ converts the other to `double` too and performs floating-point division, keeping the fraction. (Casting *both* operands is also fine and some find it clearer — only one is strictly required.)

> **Warning:** With two integer operands, `/` discards the fraction *before* any assignment happens. If you want a fractional answer, cast an operand to a floating-point type **before** dividing — casting the result afterward is too late.

### Division by zero

Dividing an integer by zero is **undefined behavior** — the standard places no constraints on what happens, and on many systems it crashes the program. Floating-point division by zero is different: it follows IEEE rules and typically yields a special value like infinity or NaN rather than crashing. As a beginner, treat both as errors to guard against unless your design *explicitly* wants infinities or NaNs. Check the denominator before you divide:

```cpp
double ratioOrZero(int numerator, int denominator)
{
    if (denominator == 0)
        return 0.0;

    return static_cast<double>(numerator) / denominator;
}
```

### Arithmetic assignment operators

A very common pattern is updating a variable based on its own current value — add to a counter, scale an accumulator. The plain way works:

```cpp
int bytesProcessed { 0 };
bytesProcessed = bytesProcessed + 16;
```

…but C++ gives you a shorthand for it, the **arithmetic assignment operators**, which combine an operation with an assignment:

```cpp
bytesProcessed += 16; // same as bytesProcessed = bytesProcessed + 16
bytesProcessed -= 4;  // subtract-and-assign
bytesProcessed *= 2;  // multiply-and-assign
bytesProcessed /= 3;  // divide-and-assign
bytesProcessed %= 10; // remainder-and-assign
```

Each one reads the variable, applies the operation with the right-hand value, and stores the result back. Reach for them when they make an update read more directly — they're idiomatic in counters, accumulators, and index arithmetic:

```cpp
void recordCoveredBlock(int& coveredBlocks)
{
    coveredBlocks += 1;
}
```

### Arithmetic operators don't modify their operands

One mental model worth fixing now, because it matters once side effects enter the picture: a plain arithmetic operator *computes a new value*. It does **not** change the objects it read from.

```cpp
int x { 3 };
int y { x + 1 }; // y is 4; x is still 3

x = x + 1;       // NOW x changes — because we assigned back into it
x += 1;          // same update, shorter
```

`x + 1` produces `4`, but `x` itself is untouched until you assign something back into it. The arithmetic assignment operators (`+=` and friends) *do* modify their left operand — that's the whole point of them. Keep this distinction crisp; it's the foundation for understanding the side-effect traps in lesson 6.4.

---

## 6.3 — Remainder and Exponentiation

### Remainder works on integers

The remainder operator `%` gives you what's left over after integer division. If you split 17 blocks into pages of 8, you fill 2 full pages with 1 block left over:

```cpp
int blocks { 17 };
int blocksPerPage { 8 };

int fullPages { blocks / blocksPerPage }; // 2  (the quotient)
int leftover  { blocks % blocksPerPage };  // 1  (the remainder)
```

`/` gives the quotient; `%` gives the remainder. Together they account for the whole division. Note that `%` is defined for integer operands only — there is no remainder operator for floating-point types (the standard library function `std::fmod` fills that role if you ever need it).

The single most useful thing `%` does is test **divisibility**. A number is divisible by `n` exactly when dividing by `n` leaves no remainder — that is, when `x % n == 0`:

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

Look closely at `isOdd`. It tests `!= 0`, **not** `== 1`. That choice matters, and the reason is the next point.

### The sign of the result follows the left operand

Here is the rule that catches people: for `x % y`, the result takes the **sign of `x`** (the left operand). This is C++'s definition of remainder, and it has real consequences for negative numbers:

```cpp
int a { -13 % 5 };  // -3, not 2
int b {  13 % -5 }; // 3
```

`-13 % 5` is `-3` because the left operand is negative. This is exactly why `isOdd` above uses `!= 0`. Watch what happens to the tempting `== 1` version:

```cpp
// -5 % 2 is -1 in C++, not 1.
bool brokenOddCheck(int x)
{
    return (x % 2) == 1; // WRONG: returns false for -5, which is odd!
}
```

For `-5`, the remainder is `-1`, so `(-1) == 1` is `false` and the function claims `-5` is even. Using `(x % 2) != 0` sidesteps the whole issue: an odd number leaves a *nonzero* remainder regardless of sign. Make `!= 0` your default oddness test and you'll never hit this.

> **Warning:** In C++, `%` takes the sign of its *left* operand, so `-5 % 2` is `-1`, not `1`. Test oddness with `(x % 2) != 0`, never `== 1` — the `== 1` form is silently wrong for negative numbers.

Because LearnCpp's `%` can produce negative results, the term **remainder** is more accurate than **modulo** for it. (Mathematical modulo is usually defined to be non-negative, which is *not* how C++'s `%` behaves on negatives — a real difference, not just vocabulary.) When you genuinely need a non-negative wraparound — folding any index into the range `[0, size)`, the way a ring buffer or a circular array does — write that rule out explicitly:

```cpp
int wrapIndex(int index, int size)
{
    int wrapped { index % size };

    if (wrapped < 0)
        wrapped += size;

    return wrapped;
}
```

For `wrapIndex(-1, 3)`: `-1 % 3` is `-1`, which is negative, so we add `3` to get `2`. That's the "one step before position 0, wrapping around to the end" answer you want. You'll implement exactly this in the lab.

This divisibility-and-wrap toolkit also powers the common "do something every Nth time" gate:

```cpp
bool shouldSampleInstruction(int instructionIndex, int sampleEvery)
{
    return (sampleEvery > 0) && ((instructionIndex % sampleEvery) == 0);
}
```

Putting `sampleEvery > 0` first isn't just tidiness — it protects the `%` from ever dividing by zero, for reasons we'll make precise in lesson 6.8 on short-circuit evaluation.

### C++ has no exponent operator

If you're coming from Python or another language with `**`, unlearn it now: **C++ has no exponentiation operator.** And the symbol that *looks* like it might be one is a trap:

```cpp
int wrong { 2 ^ 8 }; // NOT 256 — the ^ is bitwise XOR
```

In C++, `^` is the **bitwise XOR** operator, an entirely different operation. `2 ^ 8` evaluates to `10`, not `256`. This is a classic beginner bug; the lab deliberately makes you confront it.

To raise a value to a power, use `std::pow` from `<cmath>`:

```cpp
#include <cmath>

double areaScale(double radius)
{
    return std::pow(radius, 2.0);
}
```

`std::pow` is a *floating-point* function — it takes and returns `double`. That's perfect for real-valued powers, but it has a subtlety when you want an integer answer: floating-point arithmetic can land a hair *below* the true value (a "perfect" `5^3` might compute as `124.9999…`), and casting straight to `int` truncates that down to `124`. The defensive idiom is to add `0.5` before casting, which rounds to the nearest integer:

```cpp
int p { static_cast<int>(std::pow(5, 3) + 0.5) }; // 125, reliably
```

You'll use exactly this pattern for the `powInt` task in the lab.

For exact integer powers, a small loop is often clearer and avoids the round-trip through floating point entirely:

```cpp
int powInt(int base, int exponent)
{
    int result { 1 };

    for (int i { 0 }; i < exponent; ++i)
        result *= base;

    return result;
}
```

(We haven't covered loops yet — that's Chapter 8 — so for the lab you'll use the `std::pow` approach. This loop version is here so you can see the idea, and it's listed as a stretch goal once you have loops.)

One last caution: integer exponentiation overflows *fast*. `10^10` does not fit in a 32-bit `int`, and once a result exceeds the type's range you get overflow, not a warning. If large powers matter, use a wider type and check ranges deliberately.

---

## 6.4 — Increment/decrement operators, and side effects

### Prefix versus postfix

Adding or subtracting one is so common that C++ gives it dedicated operators: `++` to increment and `--` to decrement. Each comes in two flavors — **prefix** (before the operand) and **postfix** (after) — and the difference between them is the source of a small but real gotcha.

| Form | What happens |
|---|---|
| `++x` | increment `x` first, then produce the **new** value |
| `--x` | decrement `x` first, then produce the **new** value |
| `x++` | copy the **old** `x`, increment `x`, then produce the **old copy** |
| `x--` | copy the **old** `x`, decrement `x`, then produce the **old copy** |

Both forms change `x` by one. What differs is the *value the expression yields*:

```cpp
int a { 5 };
int b { ++a }; // a becomes 6, and b gets the new value: b is 6

int c { 5 };
int d { c++ }; // c becomes 6, but d gets the OLD value: d is 5
```

The mental model:

- **`++x`** — change `x`, then hand back the updated `x`. The value you read *is* the new one.
- **`x++`** — stash a copy of the old `x`, change `x`, then hand back that old copy. The value you read is the *previous* one, even though `x` itself has already moved on.

When you only want the side effect — bump a counter, advance an index — and don't care about the value the expression produces, prefer **prefix**:

```cpp
++lineCount;
```

It's marginally more efficient for some types (no old-value copy to make) and it signals "I'm just incrementing." Reach for **postfix** specifically when the old value is the point of the expression:

```cpp
int nextId()
{
    static int s_next { 0 };
    return s_next++; // hand back the current id, THEN advance for next time
}
```

Here `s_next++` returns the current id (`0` the first time) and leaves `s_next` pointing at the next one (`1`). That "use it, then advance" semantics is exactly what postfix is for.

> **Best practice:** Default to prefix (`++x`) when you don't need the expression's value. Use postfix (`x++`) only when you specifically want the value *before* the increment.

### Side effects

We keep using the phrase "side effect," so let's pin it down. A **side effect** is any observable change a piece of code makes beyond computing and returning a value — modifying a variable, writing to output, changing a stream's state.

```cpp
x = 5;          // side effect: changes x
++x;            // side effect: changes x
std::cout << x; // side effect: changes the output stream
```

By contrast, `x + 1` has *no* side effect: it computes a value and changes nothing. If you write it as a statement on its own, the value is simply computed and thrown away:

```cpp
int x { 4 };
x + 1; // computes 5, discards it; x is still 4 — almost certainly a bug
```

Side effects are useful — a program with no side effects can't do anything observable. The danger is specifically *combining* a side effect with an unspecified evaluation order, which brings us back to the trap from lesson 6.1.

### Avoid order-dependent side effects

Recall that C++ doesn't promise an order for evaluating function arguments. So an expression that both **reads** and **modifies** the same variable across different operands is asking for trouble:

```cpp
int add(int left, int right)
{
    return left + right;
}

int x { 5 };
int bad { add(x, ++x) }; // DON'T write this
```

One argument reads `x` while the other modifies it, and nothing pins down which happens first. Does `add` receive `(5, 6)` or `(6, 6)`? It depends on the compiler and its settings — meaning the answer can change when you switch machines or turn on optimizations. That's not a bug you want to chase. Write the order out as statements, where it's guaranteed:

```cpp
int x { 5 };
int original { x };
++x;

int good { add(original, x) }; // unambiguous: add(5, 6)
```

The same principle applies to instrumentation-style code, where a reviewer often has to reconstruct exactly what happened and when:

```cpp
// Hard to audit — does chooseTarget see the old or new mutantId?
recordMutation(mutantId++, chooseTarget(mutantId));

// Clear — the order is on the page:
int currentMutant { mutantId };
++mutantId;
recordMutation(currentMutant, chooseTarget(mutantId));
```

> **Warning:** Never write an expression that reads a variable in one place and modifies it in another (like `add(x, ++x)`). The evaluation order is unspecified, so the result is unreliable across compilers. Split it into separate statements.

---

## 6.5 — The comma operator

### The comma operator is not the separator comma

The comma character does double duty in C++, and the two roles are unrelated. Most commas you write are plain **separators** — punctuation that divides a list:

```cpp
void logPair(int left, int right); // separates parameters
logPair(3, 4);                     // separates arguments
```

Those aren't operators at all; they're just syntax. The **comma operator** is a genuine operator, and it shows up only when a comma appears where C++ expects a single expression:

```cpp
int x { 1 };
int y { 2 };
int result { (++x, ++y) }; // result is 3
```

What the comma operator does: evaluate the **left** operand (for its side effects), throw that result away, then evaluate the **right** operand and produce *its* value as the value of the whole expression. So `(++x, ++y)` increments `x` to `2` (discarded), increments `y` to `3`, and the expression's value is `3`.

```
(left, right)
   |
   +-- left  is evaluated, then its result is discarded
   +-- right is evaluated and becomes the value of the expression
```

### Its very low precedence makes it easy to misread

The comma operator has the **lowest precedence of any operator** in C++ — lower even than assignment. That makes it dangerously easy to misjudge what binds to what:

```cpp
int a { 1 };
int b { 2 };
int z {};

z = (a, b); // the parens force the comma operator: z becomes b, so z == 2
z = a, b;   // groups as (z = a), b — z == 1, and b is computed then discarded
```

In the second line, because `=` binds tighter than the comma operator, `z = a` happens first (giving `z` the value `1`), *then* the comma operator evaluates `b` and throws it away. The line looks like it should assign `b` to `z`, but it doesn't. This kind of silent surprise is exactly why the comma operator is, in practice, almost always avoided.

### Prefer separate statements

Anywhere you might be tempted to use the comma operator, separate statements are clearer:

```cpp
// Avoid:
std::cout << (++x, ++y) << '\n';

// Prefer:
++x;
++y;
std::cout << y << '\n';
```

The one place the comma operator earns a legitimate place is the `for` loop, whose header has a single slot for the update expression but sometimes needs to advance two variables:

```cpp
for (int left { 0 }, right { 10 }; left < right; ++left, --right)
{
    // walk inward from both ends
}
```

Even here, keep both updates small and obvious. (We'll meet `for` loops properly in Chapter 8; this is just a preview of the one idiom where the comma operator is acceptable.)

> **Best practice:** Avoid the comma operator. It has the lowest precedence of any operator and reads like a separator, so it surprises people. The narrow exception is the update clause of a `for` loop.

---

## 6.6 — The conditional operator

### `?:` is an if-else that produces a value

So far the way to choose between two values has been an `if` statement. But `if` is a *statement* — it runs code; it doesn't itself evaluate to anything. Sometimes you want the *choosing* to be an expression, something that yields one value or the other and can sit inside a larger expression. That's the **conditional operator** — the only common C++ operator that takes three operands (it's the **ternary** operator we mentioned back in 6.1):

```cpp
condition ? expressionIfTrue : expressionIfFalse
```

It evaluates `condition`; if true, the whole thing becomes `expressionIfTrue`, otherwise it becomes `expressionIfFalse`. Read it as "if `condition`, then this, else that."

```cpp
int maxValue(int a, int b)
{
    return (a > b) ? a : b;
}
```

The equivalent `if` form does the same job in more lines:

```cpp
int maxValue(int a, int b)
{
    if (a > b)
        return a;

    return b;
}
```

Both are correct. The conditional version shines when the result is *naturally a single value* — a max, a label, a fallback. Use it for that, and keep using `if` when you're choosing between *actions* rather than values.

### It can go where statements can't

Because `?:` is an expression and not a statement, it fits in places an `if` simply cannot — most notably, initializing a `const` or `constexpr` in a single declaration:

```cpp
constexpr bool debugBuild { true };
constexpr int logLevel { debugBuild ? 3 : 1 };
```

You can't initialize one `constexpr` variable with an `if` statement in that position — an `if` runs *after* the variable would need to exist. The conditional operator does the selection inline, as part of the initializer. This is one of its genuinely useful niches.

### Parenthesize it in compound expressions

The conditional operator has very low precedence — it sits at the same level as assignment, and only the comma operator is lower. So whenever it shares an expression with other operators, wrap it in parentheses — both to get the grouping you intend and to keep it readable:

```cpp
int printedLimit { (isDebugMode ? debugLimit : releaseLimit) };

std::cout << (isDebugMode ? "debug" : "release") << '\n';
```

That second line *needs* the parentheses: without them, `<<` would bind more tightly than `?:` and the expression would be misgrouped. It's also good practice to parenthesize the *condition* when it contains operators of its own, so the three parts of the `?:` are visually distinct:

```cpp
int larger { (left > right) ? left : right };
```

### The two branches need compatible types

C++ has to give the whole conditional expression a single type, so the two result branches must be types it can reconcile into one. Often that's automatic — an `int` converts cleanly to `double`:

```cpp
double value { usePrecise ? 1.0 : 1 }; // the int 1 converts to 1.0
```

But mixing **signed and unsigned** in the two branches is a quiet hazard, because the usual conversions can turn a negative number into a huge positive one:

```cpp
int signedFallback { -1 };
unsigned count { 3 };

// Surprising: to reconcile the types, -1 may convert to a large unsigned value.
auto selected { useCount ? count : signedFallback };
```

When the two branches aren't naturally the same type, make the conversion explicit so you control it:

```cpp
int selected {
    useCount ? static_cast<int>(count) : signedFallback
};
```

> **Warning:** Avoid mixing signed and unsigned values in the two branches of `?:`. To unify the type, C++ may convert a negative signed value into a large unsigned one. Cast deliberately so you control the result type.

### Keep it short

The conditional operator is at its best for compact, value-producing choices:

```cpp
std::string_view statusName(bool passed)
{
    return passed ? "passed" : "failed";
}
```

It turns sour when either branch does real work, and *nested* conditionals — a `?:` inside another `?:` — can become genuinely hard to read. There's one well-behaved exception worth knowing: a top-to-bottom **ladder** of conditionals, formatted so each test sits on its own line, reads cleanly because the structure mirrors a list of bands:

```cpp
std::string_view ticketPriority(int score)
{
    return (score >= 90) ? "P0"
         : (score >= 70) ? "P1"
         : (score >= 40) ? "P2"
         :                 "P3";
}
```

The first test that holds wins, so order alone resolves the bands — `95` stops at `"P0"`, `75` falls through to `"P1"`, and so on. You'll build exactly this ladder in the lab. Outside of that tidy pattern, reach for `if` when the logic gets involved:

```cpp
std::string_view classifyCoverage(int hitCount)
{
    return (hitCount == 0) ? "uncovered" : "covered";
}
```

That last one is a good `?:` precisely because both outcomes are simple labels and the condition is one clean comparison.

---

## 6.7 — Relational operators and floating point comparisons

### Relational operators produce `bool`

To make decisions, a program needs to *compare* values, and comparisons are how we get the booleans that drive `if` statements and logical expressions. The **relational operators** each take two operands and produce `true` or `false`:

| Operator | Meaning |
|---|---|
| `<` | less than |
| `<=` | less than or equal |
| `>` | greater than |
| `>=` | greater than or equal |
| `==` | equal to |
| `!=` | not equal to |

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

For integers, booleans, and characters these are completely straightforward — the comparison means exactly what it says. The one operator pair to keep mentally separate is `=` (assignment, "store this") versus `==` (equality, "are these the same?"). Mixing them up is a classic bug; `if (x = 5)` *assigns* `5` to `x` and is almost never what you meant.

### Don't compare booleans to `true` or `false`

When you already have a boolean, comparing it to `true` or `false` is redundant — the value *is* the condition:

```cpp
if (isReady) // good: isReady is already true-or-false
{
    run();
}

if (!isReady) // good: "if not ready"
{
    wait();
}
```

Avoid the long-winded versions:

```cpp
if (isReady == true)  // redundant — just write if (isReady)
{
    run();
}
```

> **Best practice:** A boolean is already a condition. Write `if (ready)` and `if (!ready)`, not `if (ready == true)` or `if (ready == false)`.

### Floating-point equality is risky

Now the genuinely surprising part. The relational operators behave well on integers, but `==` and `!=` are **treacherous on floating-point values**, because most decimal fractions can't be represented exactly in binary. Tiny representation errors creep in, and two numbers that are "the same" on paper can differ in their last bits:

```cpp
double a { 0.1 + 0.2 };
double b { 0.3 };

bool exactlyEqual { a == b }; // often FALSE
```

`0.1`, `0.2`, and `0.3` each carry a small rounding error when stored as `double`, and the errors don't line up — so `0.1 + 0.2` lands a hair away from the stored `0.3`, and `==` reports them unequal. This isn't a bug in C++; it's how finite binary floating point works, and every language that uses IEEE doubles has the same behavior.

The lesson: **don't use `==` or `!=` to compare calculated floating-point values** unless you truly mean bit-for-bit identity. Instead, ask whether they're *close enough*.

> **Warning:** `0.1 + 0.2 == 0.3` is typically `false`. Never compare computed floating-point values with `==` or `!=`. Compare the magnitude of their difference against a small tolerance instead.

### Epsilon comparisons

The standard fix is to check whether the two values differ by less than some small tolerance, called an **epsilon**. The simplest version uses a fixed *absolute* tolerance:

```cpp
#include <cmath>

bool approxEqual(double a, double b, double epsilon)
{
    return std::abs(a - b) <= epsilon;
}
```

`std::abs` (from `<cmath>`) gives the magnitude of the difference, ignoring its sign; if that gap is within `epsilon`, we call them equal. Using `<=` rather than `<` is a deliberate nicety: it lets an `epsilon` of `0` still report truly identical values as equal. This `approxEqual` is exactly the helper you'll write in the lab — and `approxEqual(0.1 + 0.2, 0.3, 1e-9)` is `true`, the right answer.

A fixed absolute epsilon has a known weakness, though: a tolerance that's sensible near `1.0` is far too large near `0.000001` and far too small near `1'000'000.0`, because floating-point precision scales with magnitude. A sturdier helper combines an absolute tolerance (for values near zero) with a *relative* one (a fraction of the larger operand, for big values):

```cpp
#include <algorithm>
#include <cmath>

bool almostEqual(double a, double b, double relEpsilon, double absEpsilon)
{
    double diff { std::abs(a - b) };

    if (diff <= absEpsilon) // handles values very close to zero
        return true;

    double largest { std::max(std::abs(a), std::abs(b)) };
    return diff <= (largest * relEpsilon); // scales with magnitude
}
```

Most beginner programs don't need this much machinery, and the fixed-epsilon version is fine for the lab. But the *concept* is the keeper: equality for computed floating-point values means "close enough for this problem," not "the same stored bits." (The relative+absolute version is a stretch goal if you want to push further.)

When your data is integers — as most counts and indexes are — none of this applies; exact equality is exactly right:

```cpp
if (coveredBlocks == totalBlocks)
{
    // complete coverage — integers, so == is correct
}
```

But the moment you compute a ratio or percentage, switch mental models to the tolerance approach:

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

Relational operators give you individual `true`/`false` answers; **logical operators** let you combine them into compound conditions — "this *and* that," "this *or* that," "*not* this."

| Operator | Meaning |
|---|---|
| `!x` | logical NOT — `true` when `x` is `false` |
| `x && y` | logical AND — `true` only when **both** are true |
| `x \|\| y` | logical OR — `true` when **at least one** is true |

```cpp
bool canMutate {
    hasFunctionBody && !isDeclaration
};

bool shouldLog {
    verboseMode || hasError
};
```

`canMutate` is true only where there's a function body *and* it isn't merely a declaration; `shouldLog` is true if verbose mode is on *or* an error occurred. These read almost like English, which is the point.

### Parenthesize `!` around compound conditions

Logical NOT (`!`) has **high** precedence — higher than the relational operators — so it binds very tightly to whatever sits immediately to its right. That makes it easy to accidentally negate only part of what you intended:

```cpp
if (!(x > maxAllowed)) // correct: "x is NOT greater than the limit"
{
    // ...
}
```

Without the inner parentheses, the meaning silently changes:

```cpp
if (!x > maxAllowed) // groups as (!x) > maxAllowed — almost never what you want
{
    // ...
}
```

`!x` negates `x` first (turning a number into a `bool`, `true`/`false`), and *then* compares that to `maxAllowed` — nonsense for a numeric `x`. Parenthesize the thing you mean to negate.

### Short-circuit evaluation

Here is the most practically important behavior of `&&` and `||`: they **short-circuit**. They evaluate their left operand first, and if that alone settles the answer, they *skip* the right operand entirely.

For `&&`: if the left side is `false`, the whole expression is already `false` no matter what the right side is, so the right side is never evaluated.

```cpp
if ((ptr != nullptr) && ptr->isReady())
{
    use(*ptr);
}
```

This ordering is doing safety work. If `ptr` is null, the left side is `false`, so `ptr->isReady()` is *never called* — which is exactly right, because calling a member through a null pointer would be undefined behavior. The null check **must** come first; swap the two and the guard stops protecting you. This same pattern is why, back in lesson 6.3, the sampling gate put `sampleEvery > 0` before the `%`:

```cpp
return (sampleEvery > 0) && ((instructionIndex % sampleEvery) == 0);
```

If `sampleEvery` is `0`, the left side is `false` and the `%` on the right never runs — so you never compute `x % 0`, which would be undefined behavior. Short-circuiting turns operand order into a correctness tool. You'll rely on exactly this in the lab's sampling-gate task.

For `||`, the mirror image: if the left side is `true`, the whole expression is already `true`, so the right side is skipped.

```cpp
if (isCached || loadFromDisk())
{
    runAnalysis();
}
```

If the data is already cached, `loadFromDisk()` is never called. That can be a useful optimization — but a caution comes with it: don't hide a side effect you *need* on the right-hand side of `&&` or `||`, because short-circuiting may quietly skip it. Put effects you always want in their own statements.

> **Key insight:** `&&` and `||` evaluate left-to-right and stop as soon as the result is determined. Order your operands so a cheap or protective test (a null check, a `> 0` guard) comes first — short-circuiting then prevents the unsafe right-hand operand from ever running.

### De Morgan's laws

When you negate a compound condition, **De Morgan's laws** tell you how to push the `!` inward. They're worth memorizing because the rewritten form is often clearer:

```cpp
!(a && b)  ==  (!a || !b)   // not(both) = (not one) or (not the other)
!(a || b)  ==  (!a && !b)   // not(either) = (not this) and (not that)
```

The key move is that the operator *flips* when you distribute the negation: `&&` becomes `||`, and `||` becomes `&&`. An example:

```cpp
bool invalid {
    !(hasInput && hasOutput)
};

bool sameMeaning {
    !hasInput || !hasOutput
};
```

Both say "we're missing input or missing output (or both)." Use whichever reads more naturally for the situation; De Morgan's laws let you move between the two forms with confidence.

### Logical XOR via `!=`

C++ has no dedicated *logical* XOR operator — no "exactly one of these is true" symbol. But for two operands that are *already* `bool`, the `!=` operator does the job: two booleans are unequal precisely when exactly one of them is `true`.

```cpp
bool exactlyOneMode {
    useFileInput != useStdin
};
```

This is `true` when exactly one source is selected and `false` when both or neither are. The crucial caveat: this trick is valid **only** when both operands are genuinely `bool`. On integers or pointers, `!=` is ordinary inequality, not XOR, so don't reach for it there. (You'll use this `bool != bool` idiom directly in the lab's "exactly one source" task.)

### Alternative spellings

C++ also accepts keyword spellings for the logical operators — `and` for `&&`, `or` for `||`, `not` for `!`:

```cpp
if (ready and not failed)
{
    run();
}
```

These are real, standard C++ and compile identically. But the overwhelming majority of C++ code uses the symbolic forms, so that's what you should read fluently and write by default:

```cpp
if (ready && !failed)
{
    run();
}
```

### Reading real guard conditions

Putting it together, instrumentation and decision code is full of compound guards that have to be *both correct and auditable*:

```cpp
bool shouldInsertCounter {
    (functionHasBody && !isDeclaration) &&
    (instructionIsTerminator || instructionMayExecute)
};
```

The shape recurs constantly: an *eligible location* (`functionHasBody && !isDeclaration`) **and** *one of the desired target cases* (`... || ...`). The parentheses aren't decoration — they group the AND-block and the OR-block so a reviewer sees the structure at a glance. Writing these so they read without a precedence table in hand is the whole skill this chapter is building toward, and it's precisely what the lab exercises.

---

## 6.x — Chapter 6 summary and quiz

### Summary

You now have the full operator toolkit and, just as importantly, the failure modes that come with it.

- **Precedence** decides grouping between *different* operators; **associativity** decides it between *equal*-precedence operators. Neither one dictates *operand evaluation order* — that's a separate, largely unspecified thing.
- **Parenthesize** any non-trivial compound expression so the next reader doesn't need the precedence table.
- **Integer division** discards the fraction *before* assignment. Cast an operand to a floating-point type *before* dividing if you want a fractional answer.
- **`%`** is a remainder operator for integers, and its result takes the sign of the *left* operand — so test oddness with `(x % 2) != 0`, never `== 1`.
- C++ has **no exponentiation operator**; `^` is bitwise XOR. Use `std::pow` (and `+ 0.5` before casting to `int`).
- Prefer **prefix `++x` / `--x`** unless you specifically need the value before the change.
- Never write an expression that both reads and modifies the same variable in an unspecified order.
- **Avoid the comma operator** outside a `for` loop's update clause.
- Use **`?:`** for simple, value-producing choices (including the clean top-down ladder); use `if` for complex branching.
- **Don't compare computed floating-point values with `==`/`!=`** — use an epsilon tolerance.
- **`&&` and `||` short-circuit** — order operands so a protective test runs first.
- Use **De Morgan's laws** to rewrite negated compound conditions, flipping `&&` ↔ `||`.
- For two `bool`s, **`!=` is logical XOR** ("exactly one is true").

### Lab-facing takeaways

The chapter's project, the **Day-One Triage Console**, is nine small pure functions — each one a single operator-driven decision, with no loops anywhere (the grader supplies the repetition). It's deliberately built around the exact traps above: the negative-odd `% 2 == 1` mistake, the FizzBuzz "check *both* divisors first" ordering, the leap-year logical expression and where its parentheses go, the `0.1 + 0.2 != 0.3` epsilon case, folding a negative remainder in `wrapIndex`, the short-circuit that dodges `% 0` in the sampling gate, the priority ladder of `?:`, XOR-via-`!=`, and `powInt` standing in for the missing `**`.

The mechanics are small, but they decide whether a predicate fires in the right place. The habit to carry out of this chapter is to write these expressions so they can be reviewed *without guessing* — parenthesized, short-circuited deliberately, and using the safe idioms (`!= 0` for oddness, epsilon for floats) rather than the plausible-looking wrong ones:

```cpp
bool selected {
    (candidateCount > 0) &&
    ((instructionIndex % sampleEvery) == 0) &&
    !isIgnoredInstruction
};
```
