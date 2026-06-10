# Chapter 1 — C++ Basics

> Source: <https://www.learncpp.com/> (Chapter 1)
> One file per chapter. Each lesson is a section below.

## Contents
- [1.1 — Statements and the Structure of a Program](#11--statements-and-the-structure-of-a-program)
- [1.2 — Comments](#12--comments)
- [1.3 — Introduction to Objects and Variables](#13--introduction-to-objects-and-variables)
- [1.4 — Variable Assignment and Initialization](#14--variable-assignment-and-initialization)
- [1.5 — Introduction to iostream: cout, cin, and endl](#15--introduction-to-iostream-cout-cin-and-endl)
- [1.6 — Uninitialized Variables and Undefined Behavior](#16--uninitialized-variables-and-undefined-behavior)
- [1.7 — Keywords and Naming Identifiers](#17--keywords-and-naming-identifiers)
- [1.8 — Whitespace and Basic Formatting](#18--whitespace-and-basic-formatting)
- [1.9 — Introduction to Literals and Operators](#19--introduction-to-literals-and-operators)
- [1.10 — Introduction to Expressions](#110--introduction-to-expressions)
- [1.11 — Developing Your First Program](#111--developing-your-first-program)
- [1.x — Chapter 1 summary and quiz](#1x--chapter-1-summary-and-quiz)

---

## 1.1 — Statements and the Structure of a Program

- **Statement** = *"the smallest independent unit of computation in the C++
  language."* Like a sentence in English: it instructs the program to *do*
  something. **Most statements end in a semicolon `;`** (the semicolon marks the
  end of the statement).
- **Function** = a group of statements that execute **sequentially, top to
  bottom**. Functions are the main way programs are organized.
- **`main()`** = every C++ program **must** have a function named `main`
  (lowercase). Execution **starts** at `main`; when `main`'s last statement
  finishes, the program normally ends.
- **Identifier** = the name of a function, variable, or other entity.

### Hello World, dissected

```cpp
#include <iostream>      // 1: preprocessor directive — pulls in the I/O library

                         // 2: blank line — ignored by compiler (readability)
int main()               // 3: define the main function (returns an int)
{                        // 4: opening brace — start of main's body
   std::cout << "Hello world!";  // 5: statement — print text to console
   return 0;             // 6: return 0 to the OS — "success"
}                        // 7: closing brace — end of main's body
```

| Line | What it does |
|---|---|
| `#include <iostream>` | Preprocessor directive; needed for `std::cout`. Without it, compilation fails. |
| `int main()` | Declares `main`, returning an `int`. |
| `{` … `}` | The braces delimit the function **body** — everything inside belongs to `main`. |
| `std::cout << "Hello world!";` | The actual work: send text to the console. |
| `return 0;` | Hand `0` back to the OS = ran successfully. |

### Syntax & syntax errors

- **Syntax** = the grammar rules for valid C++.
- **Syntax error** = code that breaks those rules; compilation **halts**.
- The compiler reports a line number, but **sometimes the real problem is the
  line *before*** the reported one (e.g. a missing `;` on line 5 may be reported
  on line 6).

> **Key insight:** if a reported line looks fine, check the line *above* it.
> **Nomenclature:** writing `main()` with parentheses is shorthand to signal
> "this is a function," distinguishing it from a variable named `main`.
> **Author's note:** confusion at this stage is normal; later lessons expand
> every piece.

---

## 1.2 — Comments

A **comment** is a note in the source that the compiler **ignores** — it's for
humans.

### Two kinds

```cpp
std::cout << "Hello"; // single-line: from // to end of line is ignored

/* multi-line (C-style) comment:
   this line is ignored
   and so is this one */
```

> **Multi-line comments cannot be nested.** The first `*/` ends the comment:
> ```cpp
> /* outer /* inner */ this text is NOT commented — error */
> ```
> A single-line `//` *inside* a multi-line comment is fine.

### What to comment, at three levels

| Level | Describe the… | Example |
|---|---|---|
| Library / program / function | **WHAT** (the goal) | `// This program calculates the student's final grade` |
| Inside a function | **HOW** (the approach/algorithm) | `// sum weighted scores, divide by count, assign letter` |
| Single statement | **WHY** (the reasoning) — *not* what | see below |

**Statement-level: WHY, not WHAT.**

```cpp
// BAD — just restates the code
// Set sight range to 0
sight = 0;

// GOOD — explains the reason
// Player drank a potion of blindness and cannot see
sight = 0;
```

```cpp
// BAD
// Calculate cost of items
cost = quantity * 2 * storePrice;

// GOOD
// Items are bought in pairs, so multiply quantity by 2
cost = quantity * 2 * storePrice;
```

Great comments capture **design decisions**: *"Used a linked list instead of an
array for faster insertion."*

> **Best practice:** *"Comment your code liberally, as if speaking to someone who
> has no idea what it does. Don't assume you'll remember why you made the
> choices you made."*

### Commenting out code (4 reasons)

1. New code that isn't finished — keep it from breaking the build.
2. Broken code you're not ready to fix.
3. Debugging — disable sections to isolate a problem.
4. Trying a replacement — keep the old version while testing the new.

**Shortcuts:** VS → Edit ▸ Advanced ▸ Comment Selection; Code::Blocks → Edit ▸
Comment/Uncomment; **VS Code → `Ctrl + /`**.

> **Tip:** use `//` for documentation and `/* */` for commenting-out (so they
> don't collide). For blocks that themselves contain `/* */`, use the
> `#if 0 ... #endif` preprocessor trick.
> **As an aside:** tools like **Doxygen** generate documentation from specially
> formatted comments.
> **Author's note:** this tutorial's own comments are deliberately *over*-verbose
> for teaching — not a model of production commenting.

---

## 1.3 — Introduction to Objects and Variables

- **Data** = *"any information that can be moved, processed, or stored by a
  computer."*
- **Value** = a single piece of data:
  - numbers: `5`, `-6.7`
  - characters (single quotes): `'H'`, `'$'`
  - text (double quotes): `"Hello"`
  > **Key insight:** single quotes → a character; double quotes → text;
  > no quotes → a number.
- **Literal** = a value written directly in source code. Literals are
  **read-only** — you can't change them, which is *why* we need variables.

```cpp
std::cout << 5;       // numeric literal
std::cout << 'H';     // character literal
std::cout << "Hello"; // text literal
```

### Memory, objects, variables

- **RAM** = think of it as *"a series of numbered boxes"* the program uses to
  store data while running.
- **Object** = *"a region of storage (typically RAM or a CPU register) that can
  hold a value."* C++ discourages touching memory addresses directly; you use
  objects and let the compiler manage location.
- **Variable** = *"an object that has a name (identifier)."*
  > **Key insight:** *An object stores a value in memory; a variable is an object
  > with a name.*
  > **Nomenclature:** in general programming "object" is broad; in C++ it
  > specifically **excludes functions**.

### Variable definition

```cpp
int x; // definition: "I want a variable named x of type int"
```

- At **compile time** the compiler records the name (`x`) and type (`int`).
- At **runtime** the object gets actual storage — this is **allocation**.
  > **Key insight:** an object is *"created"* once storage is reserved for it.
- Define variables inside a function:
  ```cpp
  int main()
  {
      int x; // definition of x
      return 0;
  }
  ```

### Data types

- **Type** *"determines what kind of value the object will store."*
- `int` = **integer** (whole numbers: `4`, `0`, `-12`). `double` = numbers with
  fractional parts.
  > **Key insight:** the type **must be known at compile time** so the compiler
  > knows how much memory to reserve. A variable's type can't change at runtime.

### Multiple variables in one statement

```cpp
int a, b;        // OK — same as: int a; int b;
int a, int b;    // ERROR — don't repeat the type
int a, double b; // ERROR — can't mix types in one statement
int a; double b; // OK but discouraged
```

> **Best practice:** define each variable in its **own statement on its own
> line** — clearer and avoids the pitfalls above.

---

## 1.4 — Variable Assignment and Initialization

The distinction: **initialization** gives a value *at the moment of creation*;
**assignment** changes the value *later*.

### Assignment (`=`)

```cpp
int width;   // define
width = 5;   // copy-assignment: copy 5 into width
width = 7;   // reassignment: now 7 (old value overwritten)
```

- Called **copy-assignment**: copies the right-hand value into the left-hand
  variable. A variable holds **one value at a time**.
  > **Warning:** a top beginner mistake is confusing `=` (assignment) with `==`
  > (equality).

### The forms of initialization

```cpp
int a;        // 1. default-initialization   → indeterminate (garbage) value
int b = 5;    // 2. copy-initialization      (C-style; copies 5 into b)
int c ( 6 );  // 3. direct-initialization    (parentheses)
int d { 7 };  // 4. direct-list-initialization  (braces)  ← PREFERRED
int e {};     // 5. value-initialization     (empty braces → zero)
int f = { 8 };// 6. copy-list-initialization (rarely used)
```

### Why braces `{}` win: they forbid **narrowing conversions**

```cpp
int w1 { 4.5 }; // ERROR — would lose the .5; compiler stops you
int w2 = 4.5;   // compiles silently → 4 (data lost quietly!)
int w3 ( 4.5 ); // compiles silently → 4 (data lost quietly!)
```

> The narrowing restriction applies to the **list-initialization itself**, not
> to later assignments.

List-initialization is preferred because it's the most **consistent** (works
nearly everywhere), **disallows narrowing**, and supports lists of values.

> **Best practice:** *"Prefer direct-list-initialization (`int x { 0 };`) or
> value-initialization (`int x {};`) to initialize your variables."* (Stroustrup
> and Herb Sutter agree.)
> - Use `int x { 0 };` when you actually use the initial value.
> - Use `int x {};` when the value is temporary and will be overwritten.
> **Universal rule:** *initialize your variables upon creation.*

### Initializing multiple variables

```cpp
int e { 9 }, f { 10 }; // both initialized — fine

int a, b = 5;     // PITFALL: only b is initialized; a is left uninitialized
int a = 5, b = 5; // correct: each gets its own initializer
```
> Each variable needs its **own** initializer.

### `[[maybe_unused]]` (C++17)

```cpp
[[maybe_unused]] double gravity { 9.8 }; // suppresses "unused variable" warning
```
Use sparingly — normally just delete unused variables.

---

## 1.5 — Introduction to iostream: cout, cin, and endl

`#include <iostream>` gives you the input/output stream library.

### Output: `std::cout` and `<<`

```cpp
std::cout << "Hello world!"; // text
std::cout << 5;              // number
int x{ 5 };
std::cout << x;              // variable's value
std::cout << "x is: " << x << " units"; // chain multiple with <<
```
Think of `<<` (the **insertion operator**) as a conveyor belt sending data
*toward* `std::cout`.

### Newlines: `'\n'` vs `std::endl`

Without a newline, outputs run together: `Hi!My name is Alex.`

```cpp
std::cout << "Hi!" << '\n';        // newline only (fast)
std::cout << "Name: Alex" << '\n';
std::cout << "Hi!" << std::endl;   // newline + FLUSH the buffer (slower)
```

- `std::endl` outputs a newline **and flushes** the output buffer.
- `'\n'` outputs **just** a newline; the system flushes on its own periodically.
  > **Best practice:** *"Prefer `\n` over `std::endl`."* Repeated `endl` = wasteful flushes.
- Quoting: standalone newline → **single quotes** `'\n'`; inside text → just put
  it in the string: `"Hello\n"`. `\n` is one character (linefeed, ASCII 10)
  despite being two symbols in source.
  > **Warning:** it's a **backslash** `\n`. A forward slash `'/n'` is wrong and
  > prints garbage (e.g. `12142`).

### Input: `std::cin` and `>>`

```cpp
std::cout << "Enter a number: ";
int x{};
std::cin >> x;                      // extraction operator reads keyboard input
std::cout << "You entered " << x << '\n';

int a{}, b{};
std::cin >> a >> b;                 // read two values (whitespace-separated)
```

**How extraction (`>>`) behaves:**
- Leading whitespace is skipped; reads valid characters until whitespace/invalid.
- `3.2` into an `int` → extracts `3`, leaves `.2` for next read.
- `5a` into an `int` → extracts `5`, leaves `a`.
- `abc` into an `int` → **fails**, variable set to `0`, stream stays in a failed
  state until cleared.

> **Memory aid:** `std::cout`/`std::cin` are always on the **left**; the operator
> points in the direction data flows — `cout << x` pushes `x` out; `cin >> x`
> pulls input into `x`.
> **Best practice:** still **initialize** a variable before `std::cin >> x`
> (consistency), and output a newline when a line of output is complete.
> **Aside (buffering):** output is queued in a buffer and flushed in batches for
> performance; if the program crashes before a flush, buffered output is lost.

---

## 1.6 — Uninitialized Variables and Undefined Behavior

### Uninitialized variables

Unlike many languages, **C++ does not auto-initialize most variables.** An
uninitialized variable holds whatever **garbage** was already in that memory.

```cpp
#include <iostream>
int main()
{
    int x;                    // uninitialized
    std::cout << x << '\n';   // prints "who knows!" — e.g. 7177728, then 5277592
    return 0;
}
```

Nomenclature: **initialized** = value given *at definition*; **assigned** =
value given *later*; **uninitialized** = no value given yet. (Assigning to an
uninitialized variable makes it no longer uninitialized.)

Why C++ does this: a **performance optimization inherited from C** — decades ago,
auto-zeroing thousands of variables you'd overwrite anyway wasted cycles. The
tradeoff makes less sense today, hence the "always initialize" rule.

### Undefined Behavior (UB)

**UB** = *"the result of executing code whose behavior is not well-defined by the
C++ language."* The standard says nothing about what happens. Symptoms:

- Different results every run / consistently wrong / sometimes right
- Right at first, then wrong later
- Crashes (immediately or later)
- Works on one compiler, not another
- Breaks after an *unrelated* change
- Or *appears* to work correctly (the most dangerous case)

> *"Undefined behavior is like a box of chocolates — you never know what you're
> going to get."* This is why "it worked for me" proves nothing in C++.

> **Warning (debug vs release):** Visual Studio **debug** builds pre-fill memory,
> which can *hide* uninitialized-variable bugs that then appear in **release**.

### Two cousins of UB

| Term | Meaning |
|---|---|
| **Implementation-defined behavior** | Behavior the implementation chooses but **must document** and keep consistent. E.g. `sizeof(int)` is usually `4`, but could be `2`. |
| **Unspecified behavior** | Like implementation-defined, but the implementation **need not document** it. |

Avoid relying on either — code may break across compilers/settings.

> **Best practice / overarching rule:** **always initialize your variables.**

---

## 1.7 — Keywords and Naming Identifiers

### Keywords

C++ reserves **~92 keywords** (as of C++23) with special meaning — you can't use
them as names. Samples: `int`, `return`, `if`, `else`, `for`, `while`, `class`,
`struct`, `void`, `bool`, `const`, `static`, `template`, `namespace`, `public`,
`private`. C++20 added `concept`, `consteval`, `constinit`, `co_await`,
`co_return`, `co_yield`, `requires`, `char8_t`.

### Identifier rules (mandatory)

1. **Not a keyword.**
2. **Only** letters, digits, and underscore `_` (no spaces/symbols).
3. **Cannot start with a digit** (letter or underscore only).
4. **Case-sensitive:** `nvalue`, `nValue`, `NVALUE` are three different names.

```
3some            // INVALID — starts with digit
my variable      // INVALID — spaces
void             // INVALID — keyword
_apples          // valid but discouraged (leading underscore)
sum, numFruit    // conventional
```

### Naming conventions

- **Variables/functions:** start **lowercase**.
- **User-defined types** (struct/class/enum): start **Capitalized**.
- **Multi-word:** `snake_case` (used by the std library) **or** `camelCase`
  (this tutorial). Pick one and be consistent.

> **Best practice:** in existing code, **match the existing convention** even if
> it's not "modern."

**Five guidelines:**
1. **Descriptive names** — `customerCount`, `numApples`, `monstersKilled` beat
   `data`, `ccount`. (Loop counter `i` is fine for short scopes.)
2. **Avoid leading underscores** — reserved for OS/library/compiler.
3. **Length ∝ scope** — short-lived → short name; widely visible/specific →
   longer. `minutesElapsed` > `time`.
4. **Avoid abbreviations** except common ones (`num`, `cm`, `idx`).
   > **Key insight:** *"Code is read more often than it is written"* — time saved
   > typing is time every reader later wastes.
5. **Use a comment** instead of an absurdly long name.

---

## 1.8 — Whitespace and Basic Formatting

C++ is **whitespace-independent**: the compiler generally ignores spaces, tabs,
and newlines. *Amount* doesn't matter; one space separates tokens as well as ten.

### Where whitespace DOES matter

- **Token separation:** `int x;` needs a space — `intx` is one unknown name.
- **Inside string literals** whitespace is preserved literally:
  ```cpp
  std::cout << "Hello          world!"; // all spaces printed
  ```
- **No newline inside a normal string literal:**
  ```cpp
  std::cout << "Hello
       world!"; // ERROR
  ```
- **Adjacent string literals are concatenated:**
  ```cpp
  std::cout << "Hello "
               "world!"; // prints "Hello world!"
  ```
- A `//` comment ends at the newline. Each `#include` must be on its own line.

A statement may span multiple lines:
```cpp
std::cout
    << "Hello world"; // valid
```

### Formatting recommendations

- **Indentation:** 4 spaces per level (3 also fine). Spaces give consistent
  alignment across editors.
- **Braces:** this tutorial puts the opening brace on its **own line**:
  ```cpp
  int main()
  {
      // statements
  }
  ```
- **Line length:** *"keep your lines to 80 chars or less."*
- **Split lines with the operator at the START of the next line:**
  ```cpp
  std::cout << 3 + 4
      + 5 + 6
      * 7 * 8;
  ```
- **Align** related code, and separate logical blocks with blank lines:
  ```cpp
  cost          = 57;
  pricePerItem  = 24;
  numberOfItems = 17;
  ```

> **Best practice:** match an existing project's style; consistency beats
> preference. Use **automatic formatting** (VS: Edit ▸ Advanced ▸ Format
> Document; `clang-format` exists too). Reference style guides: **C++ Core
> Guidelines** (Stroustrup & Sutter), plus Google / **LLVM** / GCC guides.

---

## 1.9 — Introduction to Literals and Operators

### Literals

A **literal** = *"a fixed value inserted directly into the source code."* In
`int x { 5 };` and `std::cout << "Hello world!";`, `5` and `"Hello world!"` are
literals. Their values are **compiled into the executable** and can't change.

- **Literal vs variable:** a literal's value is baked into the program; a
  variable names a memory location whose value is fetched (and can change) at
  runtime.

### Operators, operands, operations

- **Operation** = takes some input values (**operands**) and produces an output
  (**return value**).
- **Operator** = the symbol for the operation. `2 + 3`: operands `2`,`3`;
  operator `+`; returns `5`.

### Arity (number of operands)

| Arity | Operands | Example |
|---|---|---|
| **Unary** | 1 | `-5` (negation flips sign) |
| **Binary** | 2 | `3 + 4`; `std::cout << x` |
| **Ternary** | 3 | conditional `?:` (the only one) |
| **Nullary** | 0 | `throw` (the only one) |

Some symbols do double duty: `-` is unary (`-5`) **and** binary (`4 - 3`).

### Chaining & return values

Operators chain — one's output feeds another. `2 * 3 + 4` → `6 + 4` → `10`
(PEMDAS order). Most operators **return a value** (`2 + 3` returns `5`).

### Side effects

A **side effect** = *"an observable effect beyond producing a return value."*
- `x = 5` has the side effect of changing `x`.
- `std::cout << 5` has the side effect of printing.

> **Advanced:** both `=` and `<<` **return their left operand**, which is what
> makes chaining work:
> - `x = y = 5` → `x = (y = 5)`: assign 5 to `y`, return `y`, assign to `x`.
> - `std::cout << "a" << "b"` → `(std::cout << "a") << "b"`: print "a", return
>   `std::cout`, print "b".
> We usually use `=` and `<<` for their **side effects**, not their return value.

---

## 1.10 — Introduction to Expressions

- **Expression** = *"a non-empty sequence of literals, variables, operators, and
  function calls that calculates a value."*
- **Evaluation** = the act of computing an expression; it yields a **result**.

Each piece evaluates to a value:
- literal `2` → `2`
- variable `x` → its stored value
- `2 + 3` → `5`
- function call `five()` → its return value

> **Key principle:** *"Wherever a single value is expected, you can use a
> value-producing expression instead."*

### Expressions vs statements

- An **expression** computes a value but **can't stand alone**.
- A **statement** is a complete instruction ending in `;`.

```cpp
2 + 3            // expression — won't compile by itself
int x{ 2 + 3 };  // statement that contains an expression
```

### Expression statements

An **expression statement** = an expression + `;`:

```cpp
x = 5;     // expression statement (useful: side effect assigns x)
2 * 3;     // legal but useless — result discarded
std::cout << x; // expression statement, useful for its side effect
```

### Terminology

| Term | Meaning | In `x = 4 + 5` |
|---|---|---|
| **Subexpression** | an expression used as an operand of another | `x`, and `4 + 5` |
| **Full expression** | an expression that's not a subexpression of anything | the whole `x = 4 + 5` |
| **Compound expression** | uses two or more operators | `x = 4 + 5` (uses `=` and `+`) |

> Expressions can also evaluate to objects, functions, or nothing (a void
> function call) — but "produces a value" is the core mental model for now.

---

## 1.11 — Developing Your First Program

**Goal:** ask the user for an integer, double it, print the result.

### Building it up (and a deliberate bug)

The lesson adds one piece at a time. It intentionally writes the **wrong
operator** first:
```cpp
std::cin << num; // WRONG — cin uses >> (extraction), not <<
```
…to show that the compiler error teaches you the fix:
```cpp
std::cin >> num; // correct
```

### Three ways to compute & print (worst → best)

```cpp
// Not good: destroys the original input
num = num * 2;
std::cout << "Double that number is: " << num << '\n';

// Mostly good: extra variable for a single use
int doublenum{ num * 2 };
std::cout << "Double that number is: " << doublenum << '\n';

// Preferred: compute inline, keep num intact for reuse
std::cout << "Double that number is: " << num * 2 << '\n';
```

### Final program

```cpp
#include <iostream>

int main()
{
    std::cout << "Enter an integer: ";

    int num{ };
    std::cin >> num;

    std::cout << "Double that number is: " << num * 2 << '\n';

    return 0;
}
```

### Best practices

- **Incremental development:** *"add one piece at a time, make sure it compiles,
  and test it."* Don't write the whole thing then drown in errors.
- **Test each step** before moving on.
- **Simplicity first** — make it work, then refine.

> **Author's note:** programming is **iterative** — *"You have to write a program
> once to know how you should have written it the first time."* First solutions
> are rarely optimal; you refine over multiple passes (remove debug code, add
> comments, handle errors, simplify). And you **don't need to memorize**
> everything — understand what's possible, then look up specifics when needed.

## 1.x — Chapter 1 summary and quiz

### Concept checkpoint: double and triple

```cpp
#include <iostream>

int main()
{
    std::cout << "Enter an integer: ";

    int num{ };
    std::cin >> num;

    std::cout << "Double " << num << " is: " << num * 2 << '\n';
    std::cout << "Triple " << num << " is: " << num * 3 << '\n';

    return 0;
}
```
Keeping `num` intact is exactly what lets you use it for both lines.
