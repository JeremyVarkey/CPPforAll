# Chapter 1 — C++ Basics

> Source: <https://www.learncpp.com/> (Chapter 1)

Welcome to C++. By the end of this chapter you will be able to read a complete
program, understand what every line does, store and change data in variables,
read input from a person and print results back to them, and reason about the
single most important hazard a C++ beginner faces — *undefined behavior*. None of
it is hard in isolation. The skill you are really building is the habit of
looking at a few lines of code and knowing, precisely, what the machine will do
with them.

We will take it one piece at a time, and we will always ask *why* a rule exists
before memorizing *what* it is.

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

A computer program is just a list of instructions. The smallest such instruction
in C++ is called a **statement**. Think of a statement as a sentence: just as a
sentence is the smallest unit of a paragraph that expresses a complete thought, a
statement is the smallest independent unit of computation in C++. It tells the
program to *do* something — print a message, store a value, return a result.

Most statements end in a **semicolon** (`;`). The semicolon is how you mark the
end of one statement and the start of the next, much as a period ends a sentence.
Forget it, and the compiler keeps reading your next line as a continuation of the
current statement — which is the source of a great many beginner errors.

Statements rarely live alone. They are grouped into **functions**, and a function
is simply a named sequence of statements that run **top to bottom, in order**.
Functions are the primary way C++ programs are organized; you will write many of
your own starting in the next chapter.

One function is special. Every C++ program **must** contain a function named
`main` (lowercase — capitalization matters in C++). Execution always *starts* at
the first line of `main`, and when `main`'s final statement finishes, the program
normally ends. You can think of `main` as the front door: the operating system
walks in there, and only there.

One more piece of vocabulary: an **identifier** is the name of something you
define — a function, a variable, and so on. `main` is an identifier. So are the
names you will invent for your variables later in this chapter.

### Hello World, dissected

Here is the traditional first program, annotated line by line.

```cpp
#include <iostream>      // 1: preprocessor directive — pulls in the I/O library

                         // 2: blank line — ignored by the compiler (readability)
int main()               // 3: define the main function (returns an int)
{                        // 4: opening brace — start of main's body
    std::cout << "Hello world!";  // 5: statement — print text to the console
    return 0;            // 6: return 0 to the OS — "success"
}                        // 7: closing brace — end of main's body
```

Walking through it:

| Line | What it does |
|---|---|
| `#include <iostream>` | A *preprocessor directive*. It pulls in the input/output library so you can use `std::cout`. Leave it out and compilation fails. |
| `int main()` | Declares the `main` function, which produces an `int` (an integer) as its result. |
| `{` … `}` | The braces delimit the function **body** — everything between them belongs to `main`. |
| `std::cout << "Hello world!";` | The actual work: send the text to the console. |
| `return 0;` | Hand the value `0` back to the operating system. By convention, `0` means "the program ran successfully." |

Don't worry about understanding every mechanism here — what `#include` *really*
does, why `std::` precedes `cout`. Each of those gets its own treatment soon.
Right now, the goal is just to recognize the shape of a program: an include at
the top, an `int main()` with a braced body, statements inside, a `return 0` to
finish.

> **Note:** When we write `main()` with parentheses, the parentheses are a hint
> to *you*, the reader, that we're talking about a function rather than, say, a
> variable that happens to be named `main`. The parentheses are also where a
> function's inputs would go — more on that next chapter.

### Syntax and syntax errors

Every human language has grammar — rules about how words may be arranged.
Programming languages have grammar too, and in C++ it's called **syntax**. Code
that obeys the rules is *syntactically valid*; code that breaks them contains a
**syntax error**, and when the compiler hits one, compilation **halts**. You
cannot run a program that won't compile.

The good news: the compiler tells you the line number where it got confused. The
catch: the line it names is where it *noticed* the problem, which is not always
where the problem *is*. A classic example is a missing semicolon — leave one off
line 5, and because line 6 now reads as a continuation of line 5, the compiler
may flag the error on line 6.

> **Key insight:** If the line the compiler complains about looks perfectly fine,
> check the line *directly above* it. A surprising fraction of syntax errors are
> really one line earlier than reported.

Confusion at this stage is completely normal. You are looking at machinery whose
inner workings have not been explained yet. That is by design — we name the parts
now and explain each one fully as the chapter unfolds.

---

## 1.2 — Comments

Code is read far more often than it is written — by teammates, by reviewers, and
above all by *future you*, who will remember none of today's reasoning. A
**comment** is a note you leave in the source code for those human readers. The
compiler ignores comments entirely; they exist purely to explain.

### Two kinds of comment

C++ gives you two ways to write a comment.

```cpp
std::cout << "Hello"; // single-line: from // to the end of the line is ignored

/* multi-line (C-style) comment:
   this line is ignored
   and so is this one */
```

A **single-line comment** starts with `//` and runs to the end of that line — use
it for short notes. A **multi-line comment** starts with `/*` and continues, even
across line breaks, until the first `*/` — use it for longer explanations.

That phrase "until the *first* `*/`" carries a trap.

> **Warning:** Multi-line comments cannot be nested. The first `*/` closes the
> comment, no matter how many `/*` you opened:
> ```cpp
> /* outer /* inner */ this text is NOT commented — error */
> ```
> Here the comment ends at the first `*/`, so the compiler tries to compile the
> trailing text and fails. (A single-line `//` *inside* a multi-line comment is
> harmless, however.)

### What to write in a comment

The hard part of commenting is not the syntax — it's deciding *what* to say. A
useful rule of thumb is that the right thing to comment depends on the scope you
are commenting at:

| Level | Describe the… | Example |
|---|---|---|
| Library / program / function | **WHAT** — the goal | `// This program calculates the student's final grade` |
| Inside a function | **HOW** — the approach | `// sum weighted scores, divide by count, assign a letter` |
| A single statement | **WHY** — the reasoning | see below |

The most common mistake is writing statement-level comments that just *restate the
code in English*. The code already says what it does; a good comment says *why*.

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

The very best comments capture *design decisions* — the reasoning a future reader
could never reconstruct from the code alone. For example: *"Used a linked list
instead of an array here for faster insertion."*

> **Best practice:** Comment your code liberally, as if speaking to someone who
> has no idea what it does. Don't assume you'll remember why you made the choices
> you made — you won't.

### Commenting out code

Comments have a second use: temporarily disabling code without deleting it. Wrap
a line or block in comment markers and the compiler skips it. Four common reasons
to do this:

1. You're writing new code that isn't finished and don't want it to break the build yet.
2. You have broken code you're not ready to fix.
3. You're debugging, and you disable sections to isolate which part causes a problem.
4. You're trying a replacement, and you keep the old version around while testing the new one.

Most editors have a one-keystroke shortcut to comment or uncomment the selected
lines — in VS Code it's `Ctrl + /`. Look for "Comment Selection" or similar in
your editor's Edit menu.

> **Tip:** A handy convention is to use `//` for documentation and `/* */` for
> commenting-out, so the two never collide. If you need to comment out a block
> that itself contains `/* */` comments, reach for the preprocessor trick
> `#if 0 ... #endif`, which disables everything between it regardless of inner
> comments.

> **Note:** Throughout this course you'll see code commented far more heavily than
> you'd ever write in production — that's deliberate, to teach. Real code is
> commented where it adds insight, not on every line.

---

## 1.3 — Introduction to Objects and Variables

Programs exist to manipulate **data** — any information that can be moved,
processed, or stored by a computer. A single piece of data is a **value**. Values
come in a few flavors you'll meet constantly:

- **numbers**, written plainly: `5`, `-6.7`
- **characters** — a single symbol in single quotes: `'H'`, `'$'`
- **text** — a run of characters in double quotes: `"Hello"`

The quoting is not decoration; it changes the *meaning*.

> **Key insight:** Single quotes mean a single character (`'H'`), double quotes
> mean text (`"Hello"`), and no quotes at all means a number (`5`). Mixing these
> up is a common early slip.

A value written directly in your source code is called a **literal**:

```cpp
std::cout << 5;       // numeric literal
std::cout << 'H';     // character literal
std::cout << "Hello"; // text literal
```

Literals are **read-only**: `5` is forever `5`, baked into the program. That
permanence is exactly *why* we need variables — we need somewhere to keep values
that can *change* as the program runs.

### Memory, objects, and variables

While your program runs, its data lives in **RAM** (random-access memory). A
useful mental picture: RAM is a long row of numbered boxes, each able to hold a
little data, and the program puts values into boxes and reads them back as needed.

C++ deliberately keeps you from juggling those box numbers (memory addresses) by
hand. Instead it gives you a higher-level abstraction:

- An **object** is a region of storage — typically in RAM — that can hold a value.
  You ask for an object; the compiler decides which "box" it lives in.
- A **variable** is *an object that has a name*. The name is an identifier you
  choose, and you use it to refer to the object throughout your code.

> **Key insight:** An object stores a value in memory; a variable is simply an
> object with a name. From here on, "variable" is the word you'll use almost
> always — but knowing the object underneath it will matter later.

> **Note:** In everyday programming the word "object" is used loosely. In C++ it
> has this specific meaning — a region of storage holding a value — and notably it
> *excludes* functions.

### Defining a variable

To create a variable, you write a **definition** that states its name and its
type:

```cpp
int x; // definition: "I want a variable named x of type int"
```

Two distinct moments are involved here:

- At **compile time**, the compiler reads this line and records that there will be
  a variable named `x` of type `int`.
- At **runtime**, when execution reaches this point, the object is actually given
  storage. Reserving that storage is called **allocation**, and the object is said
  to be **created** once it has storage.

Variable definitions normally live inside a function:

```cpp
int main()
{
    int x; // definition of x
    return 0;
}
```

### Data types

Every variable has a **type**, and the type determines what kind of value the
object can store. You've already seen two:

- `int` holds an **integer** — a whole number like `4`, `0`, or `-12`.
- `double` holds a number that may have a **fractional part**, like `3.14`.

> **Key insight:** A variable's type must be known at **compile time**, because
> that's how the compiler figures out how much memory to reserve and how to
> interpret the bits. A variable's type is fixed when you define it and cannot
> change while the program runs.

### Defining several variables at once

You *may* define multiple variables of the same type in one statement:

```cpp
int a, b;        // OK — same as: int a; int b;
int a, int b;    // ERROR — don't repeat the type
int a, double b; // ERROR — can't mix types in one statement
int a; double b; // OK but discouraged
```

The pitfalls above — repeating the type, mixing types — are easy to trip over,
and even the "legal" multi-variable form tends to read worse than separate lines.

> **Best practice:** Define each variable in its own statement on its own line.
> It's clearer, it sidesteps the errors above, and it leaves room for an
> initializer — which is exactly what the next lesson is about.

---

## 1.4 — Variable Assignment and Initialization

Defining a variable gives you a named box. The next question is: how does a value
get *into* that box? There are two distinct moments at which that can happen, and
C++ gives them two different names:

- **Initialization** puts a value in *at the moment the variable is created*.
- **Assignment** puts a value in *later*, after the variable already exists.

The distinction matters more than it first appears, and we'll see why by the end
of this lesson.

### Assignment with `=`

After a variable exists, you change its value with the assignment operator `=`:

```cpp
int width;   // define width
width = 5;   // copy-assignment: copy 5 into width
width = 7;   // reassignment: width is now 7 (the old 5 is gone)
```

This is called **copy-assignment** because it copies the value on the right into
the variable on the left. A variable holds exactly **one value at a time**;
assigning a new value overwrites the old one completely.

> **Warning:** A classic beginner error is confusing `=` (assignment) with `==`
> (the equality test, which you'll meet in a later chapter). They look almost
> alike and mean entirely different things. `=` *changes* a value; `==` *checks*
> whether two values are equal.

### The forms of initialization

Initialization — giving a value at creation — has several spellings in C++. You
don't need to memorize all of them today, but you should recognize them, because
you'll encounter all of them in real code:

```cpp
int a;        // 1. default-initialization      → indeterminate ("garbage") value
int b = 5;    // 2. copy-initialization         (C-style; copies 5 into b)
int c ( 6 );  // 3. direct-initialization       (parentheses)
int d { 7 };  // 4. direct-list-initialization  (braces)  ← PREFERRED
int e {};     // 5. value-initialization        (empty braces → zero)
int f = { 8 };// 6. copy-list-initialization    (rarely used)
```

Form 1 deserves a flag right away: writing `int a;` with no initializer leaves `a`
holding whatever bits were already in that memory — a garbage value. That's a real
hazard, and lesson 1.6 is devoted to it.

### Why braces `{}` win: they forbid narrowing

Among all those forms, the brace forms (4 and 5) are the ones to reach for. The
reason is that **list-initialization disallows narrowing conversions** — it stops
you from quietly throwing data away.

```cpp
int w1 { 4.5 }; // ERROR — would lose the .5; the compiler stops you
int w2 = 4.5;   // compiles silently → 4  (the .5 is lost, quietly!)
int w3 ( 4.5 ); // compiles silently → 4  (the .5 is lost, quietly!)
```

A `double` value like `4.5` doesn't fit in an `int` without losing the fractional
part. The older forms let it happen silently and leave you with `4`, hiding a
potential bug. The brace form makes the compiler refuse — it turns a silent data
loss into an error you can see and fix.

> **Note:** This narrowing restriction applies to the list-initialization itself.
> It does not prevent you from later *assigning* a narrowing value; it's about the
> moment of creation.

Braces are preferred for three reasons: they're the most **consistent** form
(they work nearly everywhere in the language), they **disallow narrowing**, and
they extend naturally to initializing things with lists of values.

> **Best practice:** Prefer direct-list-initialization (`int x { 0 };`) or
> value-initialization (`int x {};`). Use `int x { 0 };` when you have a meaningful
> starting value, and `int x {};` when the value is just a placeholder you'll
> overwrite shortly. This is the advice given by C++'s own creator (Bjarne
> Stroustrup) and other leading authorities.

> **Key insight:** Initialize your variables when you create them. This single
> habit prevents an entire category of bugs, as the next two lessons make
> painfully clear.

### Initializing several variables

If you do define multiple variables in one statement, each one needs *its own*
initializer:

```cpp
int e { 9 }, f { 10 }; // both initialized — fine

int a, b = 5;     // PITFALL: only b is initialized; a is left uninitialized
int a = 5, b = 5; // correct: each variable gets its own initializer
```

The middle line is a trap worth burning into memory: `int a, b = 5;` does *not*
set both to 5. The `= 5` attaches only to `b`. `a` is left uninitialized.

### Suppressing "unused variable" warnings

Sometimes you define a variable you don't (yet) use, and the compiler warns you
about it. In C++17 you can mark such a variable with `[[maybe_unused]]` to silence
that specific warning:

```cpp
[[maybe_unused]] double gravity { 9.8 }; // suppresses the "unused variable" warning
```

Use this sparingly. Most of the time, an unused variable is just clutter you
should delete rather than annotate.

---

## 1.5 — Introduction to iostream: cout, cin, and endl

A program that can't communicate isn't much use. The standard library's
input/output stream facilities let your program write to the console and read what
the user types. You unlock them with one line at the top of your file:

```cpp
#include <iostream>
```

`iostream` stands for "input/output stream," and including it makes `std::cout`,
`std::cin`, and friends available.

### Output with `std::cout` and `<<`

`std::cout` represents the console's output. You send data to it with the
**insertion operator** `<<`:

```cpp
std::cout << "Hello world!"; // text
std::cout << 5;              // a number
int x{ 5 };
std::cout << x;              // a variable's value
std::cout << "x is: " << x << " units"; // chain several pieces with <<
```

A helpful image: `<<` is a conveyor belt carrying data *toward* `std::cout` and out
to the screen. The arrows literally point in the direction the data flows. And as
the last line shows, you can chain many `<<` together in one statement, mixing
text, numbers, and variables freely.

### Newlines: `'\n'` versus `std::endl`

By default, output does not include line breaks, so consecutive outputs run
together on one line:

```text
Hi!My name is Alex.
```

To break to a new line you have two tools:

```cpp
std::cout << "Hi!" << '\n';        // newline only (fast)
std::cout << "Name: Alex" << '\n';
std::cout << "Hi!" << std::endl;   // newline AND flush the buffer (slower)
```

Both end the line, but they differ in a subtle way. Output is not written to the
screen one character at a time; it's collected in a **buffer** and written out in
batches, which is far more efficient. `std::endl` prints a newline *and then*
forces the buffer to be written out immediately (a **flush**). `'\n'` prints just
the newline and lets the system flush whenever it normally would.

> **Best practice:** Prefer `'\n'` over `std::endl`. Flushing on every line is
> wasted work; the system flushes often enough on its own. Reach for `std::endl`
> only in the rare case you genuinely need output forced out *right now*.

A note on quoting the newline. When the newline stands alone, write it in single
quotes as `'\n'` (it's a single character). When it's part of a larger string,
just put it inside the quotes: `"Hello\n"`. Although you type `\n` as two symbols,
it represents *one* character — the linefeed (ASCII value 10).

> **Warning:** It's a **backslash**: `\n`. A forward slash — `'/n'` — is wrong and
> will print garbage rather than a newline. This is an easy typo to make and a
> confusing one to debug.

### Input with `std::cin` and `>>`

`std::cin` represents keyboard input. You pull data *out* of it into a variable
with the **extraction operator** `>>`:

```cpp
std::cout << "Enter a number: ";
int x{};
std::cin >> x;                      // read what the user types into x
std::cout << "You entered " << x << '\n';

int a{}, b{};
std::cin >> a >> b;                 // read two values (separated by whitespace)
```

Notice the operators point the *opposite* way from output. That's the mnemonic:

> **Tip:** `std::cout` and `std::cin` always sit on the **left**, and the operator
> points the way the data moves. `cout << x` pushes `x` *out* to the screen;
> `cin >> x` pulls input *in* to `x`.

### How extraction actually behaves

The extraction operator follows a few rules worth knowing, because they explain
some surprising results:

- It first **skips any leading whitespace**, then reads valid characters until it
  hits whitespace or a character that doesn't fit the target type.
- Typing `3.2` into an `int` extracts `3` and leaves `.2` waiting in the stream for
  the next read.
- Typing `5a` into an `int` extracts `5` and leaves `a` behind.
- Typing `abc` into an `int` **fails**: the variable is set to `0`, and the stream
  enters a failed state and stays there (refusing further reads) until it's
  cleared. You'll learn to recover from this in a later chapter.

> **Best practice:** Even though `std::cin >> x` is about to overwrite `x`,
> still initialize `x` when you define it (e.g. `int x{};`) — consistency with the
> "always initialize" rule keeps you safe by habit. And print a newline once a
> complete line of output is finished.

---

## 1.6 — Uninitialized Variables and Undefined Behavior

This lesson is short, but it's arguably the most important in the chapter. It
explains a hazard that makes C++ genuinely different from most languages you may
have seen — and a category of bug that can waste hours if you don't understand it.

### Uninitialized variables

In many languages, a freshly created variable starts at a sensible default like
zero. **C++ does not do this for most variables.** A variable you define without
an initializer holds whatever bits already happened to be sitting in that piece of
memory — a meaningless **garbage** value.

```cpp
#include <iostream>

int main()
{
    int x;                    // uninitialized — holds garbage
    std::cout << x << '\n';   // prints "who knows!" — maybe 7177728, maybe 5277592
    return 0;
}
```

Run that program twice and you might see two different numbers, or the same one,
or zero by luck. The point is that the value is *unpredictable*.

A bit of vocabulary to keep these straight:

- **Initialized** — given a value *at the point of definition*.
- **Assigned** — given a value *later*, with `=`.
- **Uninitialized** — given no value yet at all.

(Assigning a value to an uninitialized variable, of course, makes it no longer
uninitialized.)

Why does C++ behave this way? It's a **performance choice inherited from C**.
Decades ago, automatically zeroing out thousands of variables — many of which the
program would immediately overwrite anyway — wasted measurable CPU time. The
tradeoff made more sense then than it does now, which is exactly why the modern
guidance is the blunt rule: *always initialize.*

### Undefined behavior

Reading an uninitialized variable is one example of a broader and more dangerous
phenomenon: **undefined behavior**, usually abbreviated **UB**. Undefined behavior
is the result of executing code whose behavior the C++ language does not define.
The standard simply says nothing about what should happen — so *anything* may.

The symptoms are maddeningly varied. Code with undefined behavior might:

- produce different results every run, or be consistently wrong, or be sometimes right;
- work correctly at first and then go wrong later;
- crash — immediately, or much later;
- work on one compiler and fail on another;
- break after an *unrelated* change elsewhere in the program;
- or — the most dangerous outcome — *appear* to work perfectly.

> **Key insight:** Undefined behavior is like a box of chocolates — you never know
> what you're going to get. This is why "but it worked on my machine" proves
> nothing in C++. A program with UB can run flawlessly for you and crash for
> someone else, with no code change in between.

> **Warning:** Some compilers' *debug* builds pre-fill memory with known patterns,
> which can accidentally *hide* an uninitialized-variable bug — until you switch to
> a *release* build, where the bug suddenly appears. Don't trust "it works in debug"
> as evidence the code is correct.

### Two milder cousins of UB

Not every gap in the standard is full-blown undefined behavior. Two related terms
describe behavior that's unpredictable across systems but not chaotic:

| Term | Meaning |
|---|---|
| **Implementation-defined behavior** | The implementation gets to choose the behavior, but it **must document** its choice and stay consistent. For example, the size of an `int` is usually 4 bytes, but the standard permits 2. |
| **Unspecified behavior** | Like implementation-defined, except the implementation **need not document** what it chose. |

Both are worth avoiding when you can: code that leans on them may behave
differently when you change compilers or settings.

> **Best practice:** Always initialize your variables. If you take one rule from
> this entire chapter, take this one — it eliminates the most common and most
> confusing source of undefined behavior a beginner will hit.

---

## 1.7 — Keywords and Naming Identifiers

You name things constantly in C++ — variables, functions, types. Good names are
one of the quiet differences between code that's a pleasure to read and code
that's a slog. This lesson covers the rules you *must* follow and the conventions
you *should*.

### Keywords

C++ reserves a set of words — around 92 of them as of C++23 — that have built-in
meaning. These are **keywords**, and you cannot use them as names of your own. You
already know several:

```text
int    return  if      else    for     while   class   struct
void   bool    const   static  template namespace public private
```

The language keeps growing: C++20, for instance, added keywords such as `concept`,
`consteval`, `constinit`, `requires`, `char8_t`, and the coroutine words
`co_await`, `co_return`, and `co_yield`. You don't need to memorize the list — you'll
absorb the important ones as you go, and the compiler will object if you ever try
to use one as a name.

### The rules for identifiers

An **identifier** — any name you invent — must obey four hard rules:

1. It **cannot be a keyword**.
2. It may contain **only** letters, digits, and the underscore `_` — no spaces, no
   other symbols.
3. It **cannot start with a digit** (it must begin with a letter or an underscore).
4. It is **case-sensitive**: `nvalue`, `nValue`, and `NVALUE` are three *different*
   names.

Some quick examples of valid and invalid identifiers:

```text
3some            // INVALID — starts with a digit
my variable      // INVALID — contains a space
void             // INVALID — it's a keyword
_apples          // valid, but discouraged (leading underscore — see below)
sum, numFruit    // good, conventional names
```

### Naming conventions

Beyond the hard rules, the C++ community follows conventions. These aren't enforced
by the compiler, but following them makes your code instantly readable to other C++
programmers:

- **Variables and functions** start with a **lowercase** letter.
- **User-defined types** (structs, classes, enums — all coming in later chapters)
  start with a **Capital** letter.
- For **multi-word** names, pick a style and stick to it: `snake_case` (words
  joined by underscores, the style the standard library uses) or `camelCase` (each
  later word capitalized, the style this course uses).

> **Best practice:** When you join an existing project, match the conventions
> *already in use there* — even if they aren't the ones you'd choose. Consistency
> within a codebase matters more than any individual preference.

Five guidelines for choosing *good* names:

1. **Be descriptive.** `customerCount`, `numApples`, and `monstersKilled` tell the
   reader what they hold; `data` and `ccount` make them guess. (A loop counter named
   `i` is the accepted exception, fine for a tiny scope.)
2. **Avoid leading underscores.** Names beginning with `_` are reserved for the
   compiler, standard library, and operating system. Steer clear.
3. **Let length scale with scope.** A variable used for two lines can have a short
   name; one that's visible across a large region, or whose meaning is specific,
   deserves a longer, clearer one. `minutesElapsed` beats a bare `time`.
4. **Avoid abbreviations** unless they're universally understood (`num`, `cm`,
   `idx`). The few keystrokes you save get paid back, with interest, by every reader
   who has to decode them.
5. **Use a comment** rather than an absurdly long name when you need to explain
   more than a name reasonably can.

> **Key insight:** Code is read far more often than it is written. The seconds you
> save with a cryptic name are spent many times over by everyone who later has to
> figure out what it meant — including you.

---

## 1.8 — Whitespace and Basic Formatting

**Whitespace** is the catch-all term for spaces, tabs, and newlines. C++ is
**whitespace-independent**: the compiler generally doesn't care how much of it you
use or where. One space separates two tokens exactly as well as ten do, and you can
spread a statement across several lines. Because the compiler ignores it,
whitespace exists for *one* audience — human readers. Good formatting is how you
make code legible.

### Where whitespace *does* matter

There are a handful of exceptions where whitespace carries meaning:

- **Separating tokens.** `int x;` needs the space; written `intx`, the compiler
  sees a single unknown name.
- **Inside string literals**, whitespace is preserved exactly as typed:
  ```cpp
  std::cout << "Hello          world!"; // every one of those spaces is printed
  ```
- **A normal string literal cannot span a line break:**
  ```cpp
  std::cout << "Hello
       world!"; // ERROR
  ```
- **Adjacent string literals are joined together** by the compiler, which is the
  proper way to split a long string across lines:
  ```cpp
  std::cout << "Hello "
               "world!"; // prints "Hello world!"
  ```
- A `//` comment always ends at the newline, and each `#include` directive must sit
  on its own line.

A single statement is free to span multiple lines, which is useful for long ones:

```cpp
std::cout
    << "Hello world"; // valid — one statement across two lines
```

### Formatting recommendations

Since the compiler won't impose a style, you should adopt a consistent one. The
conventions this course follows:

- **Indentation:** indent one level (4 spaces is the convention used here) for each
  level of nesting. Spaces, rather than tabs, give the same alignment in every
  editor.
- **Braces on their own line.** This course places a function's opening brace on its
  own line, lined up under the function:
  ```cpp
  int main()
  {
      // statements
  }
  ```
- **Line length:** keep lines to about 80 characters so they fit comfortably and
  don't require horizontal scrolling.
- **When you must break a long line, put the operator at the *start* of the next
  line**, where it's easy to see:
  ```cpp
  std::cout << 3 + 4
      + 5 + 6
      * 7 * 8;
  ```
- **Align related code and group it.** Vertically aligning related statements, and
  separating logical blocks with blank lines, makes structure visible at a glance:
  ```cpp
  cost          = 57;
  pricePerItem  = 24;
  numberOfItems = 17;
  ```

> **Best practice:** Match an existing project's style rather than imposing your
> own — consistency beats personal preference. Most editors can reformat code for
> you automatically (look for "Format Document"), and the `clang-format` tool does
> this from the command line. For deeper guidance, the **C++ Core Guidelines**
> (maintained by Stroustrup and Sutter) are an excellent reference.

---

## 1.9 — Introduction to Literals and Operators

We've used literals and operators informally already. This lesson gives them
proper names and a clearer mental model, because nearly everything you compute in
C++ is built from these two ingredients.

### Literals

A **literal** is a fixed value written directly into the source code. In
`int x { 5 };` the `5` is a literal, and in `std::cout << "Hello world!";` the
`"Hello world!"` is a literal too. A literal's value is compiled straight into the
executable and cannot change while the program runs.

Contrast that with a **variable**: a variable names a location in memory whose
value is looked up at runtime and *can* change. A literal *is* its value; a variable
*refers to* a value. That difference is the whole reason both exist — literals give
you constant values to work with, variables give you changeable storage to put
them in.

### Operators, operands, and operations

An **operation** takes one or more input values, called **operands**, and produces
an output, called the **return value**. The symbol that specifies which operation
to perform is the **operator**.

Take `2 + 3`. The operands are `2` and `3`, the operator is `+`, and the operation
returns `5`. Simple — but this vocabulary lets us talk precisely about more
involved expressions.

### Arity: how many operands

Operators are classified by **arity** — the number of operands they take:

| Arity | Operands | Example |
|---|---|---|
| **Unary** | 1 | `-5` (the `-` negates, flipping the sign) |
| **Binary** | 2 | `3 + 4`; also `std::cout << x` |
| **Ternary** | 3 | the conditional operator `?:` (the only one) |
| **Nullary** | 0 | `throw` (the only one) |

Notice that one symbol can have different arities depending on context: `-` is
unary in `-5` (negation) but binary in `4 - 3` (subtraction). The compiler tells
them apart by how many operands it finds.

### Chaining and return values

Because most operators *return* a value, the output of one operation can become an
operand of the next — operators **chain**. Evaluating `2 * 3 + 4` proceeds as
`6 + 4`, then `10`, following the usual order of operations you learned in math
class (multiplication before addition).

### Side effects

Some operators do more than compute a value — they have a **side effect**, an
observable effect beyond producing a return value:

- `x = 5` returns a value *and* has the side effect of changing `x`.
- `std::cout << 5` returns a value *and* has the side effect of printing to the
  screen.

When you write these, you almost always care about the side effect, not the return
value. But the return value isn't wasted — it's the secret behind chaining.

> **Note:** Both `=` and `<<` return their *left* operand, which is precisely what
> makes chains work. `x = y = 5` groups as `x = (y = 5)`: assign 5 to `y`, then the
> result (`y`) is assigned to `x`. Likewise `std::cout << "a" << "b"` groups as
> `(std::cout << "a") << "b"`: print `"a"`, get `std::cout` back, then print `"b"`.

---

## 1.10 — Introduction to Expressions

We've reached a concept that ties the chapter together. An **expression** is a
non-empty sequence of literals, variables, operators, and function calls that
together compute a value. The act of computing an expression is called
**evaluation**, and what it produces is called the **result**.

Every small piece you've seen evaluates to a value:

- the literal `2` evaluates to `2`
- the variable `x` evaluates to its stored value
- `2 + 3` evaluates to `5`
- a function call like `five()` evaluates to whatever the function returns

> **Key insight:** Wherever C++ expects a single value, you may supply a
> value-producing expression instead. This is a deep and freeing idea: anywhere a
> `5` could go, so could `2 + 3`, or `x`, or `someFunction()`. You're not limited
> to plugging in literals.

### Expressions versus statements

These two terms are easy to confuse, so let's pin them down:

- An **expression** computes a value but **cannot stand on its own** as a complete
  instruction.
- A **statement** is a complete instruction, ending in a semicolon.

```cpp
2 + 3            // an expression — this alone will NOT compile
int x{ 2 + 3 };  // a statement that contains the expression 2 + 3
```

### Expression statements

You *can* turn an expression into a statement by adding a semicolon. The result is
called an **expression statement**:

```cpp
x = 5;          // expression statement — useful, for its side effect of assigning x
2 * 3;          // legal, but useless — the result 6 is computed then discarded
std::cout << x; // expression statement — useful, for its side effect of printing
```

The middle line is a reminder that *legal* and *useful* aren't the same thing.
`2 * 3;` compiles fine, computes `6`, and immediately throws it away. The other two
earn their keep through their side effects.

### A bit more terminology

When expressions nest inside one another, these terms help describe the structure.
Using `x = 4 + 5` as the example:

| Term | Meaning | In `x = 4 + 5` |
|---|---|---|
| **Subexpression** | an expression used as an operand of another | `x`, and `4 + 5` |
| **Full expression** | an expression that is not a subexpression of anything else | the whole `x = 4 + 5` |
| **Compound expression** | one that uses two or more operators | `x = 4 + 5` (uses both `=` and `+`) |

> **Note:** For now, "an expression produces a value" is the right mental model.
> Strictly, expressions can also evaluate to objects or functions, or to nothing at
> all (a call to a function that returns nothing) — but that nuance can wait.

---

## 1.11 — Developing Your First Program

Now we put the whole chapter to work. The goal is a small but complete program:
**ask the user for an integer, double it, and print the result.** Modest as it is,
writing it well teaches a habit that scales to programs of any size.

### Build it up one piece at a time

The wrong way to write a program is to type the entire thing, hit compile, and
then drown in a wall of errors with no idea which line caused what. The right way
is **incremental**: add one small piece, compile, make sure it works, then add the
next. When something breaks, you know it's in the piece you just added.

Let's do exactly that, and even make a deliberate mistake to see how the compiler
helps. Suppose we first reach for the wrong operator on input:

```cpp
std::cin << num; // WRONG — cin uses >> (extraction), not <<
```

This won't compile, and that's a *feature*. The compiler's error message points
you straight at the fix — `std::cin` takes `>>`, the operator that pulls data in:

```cpp
std::cin >> num; // correct
```

Catching the slip the moment you introduce it is the entire payoff of working
incrementally.

### Three ways to double and print — worst to best

Once we have the number, we need to double it and print it. There are several ways,
and comparing them is instructive:

```cpp
// Not good: this destroys the original input by overwriting num
num = num * 2;
std::cout << "Double that number is: " << num << '\n';

// Mostly good, but creates an extra variable used only once
int doublenum{ num * 2 };
std::cout << "Double that number is: " << doublenum << '\n';

// Preferred: compute the expression inline; num is left untouched for reuse
std::cout << "Double that number is: " << num * 2 << '\n';
```

The first version is the trap: by reassigning `num`, you lose the original value,
which you might still need. The second works but spends a whole variable on a
single use. The third is cleanest — recall from lesson 1.10 that *anywhere a value
is expected, an expression will do*, so `num * 2` can sit right inside the output
statement. And keeping `num` intact will matter in the very next section.

### The finished program

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

Read it against everything you've learned: the `#include` brings in I/O, `main`
returns an `int`, `num` is brace-initialized (so it's never uninitialized),
`std::cin >> num` reads input, an inline expression doubles it on the way to
`std::cout`, and `return 0` reports success.

### How real programs get written

A closing word on process, because it applies to everything you'll build from here.

> **Best practice:** Develop incrementally — add one piece at a time, make sure it
> compiles, and test it before moving on. Make it work first; refine it afterward.

> **Note:** Programming is *iterative*. You essentially have to write a program once
> to learn how you should have written it the first time — first attempts are rarely
> the cleanest. You'll go back and remove debugging code, add comments, handle edge
> cases, and simplify. And you do **not** need to memorize the language: understand
> what's *possible*, then look up the specifics when you need them.

This incremental, expression-aware mindset is exactly what the chapter's lab asks
of you next.

## 1.x — Chapter 1 summary and quiz

You've covered the foundation the rest of the course builds on. To recap the big
ideas: programs are **statements** grouped into **functions**, and every program
starts at `main`. **Variables** are named **objects** that store values of a given
**type**, and you should **always initialize** them — preferably with braces
(`int x { 0 };` or `int x {};`), which catch narrowing and protect you from
**undefined behavior**. You print with `std::cout <<`, read with `std::cin >>`, and
end lines with `'\n'`. Values come from **literals** and from **expressions**, and
*wherever a value is expected, an expression can take its place*.

### Concept checkpoint: double and triple

Here's a small program that exercises the whole chapter at once. It reads a number,
then prints both its double and its triple:

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

Notice *why* this works: because we compute `num * 2` and `num * 3` as inline
expressions rather than overwriting `num`, the original value survives to be used
on both lines — and even printed alongside each result. That single habit, leaving
your input intact and reaching for expressions, is what the upcoming lab leans on.

### Bringing it together: the Cash Register lab

The chapter's exercise asks you to write the brain of a supermarket cash register.
It reads three whole numbers — a **unit price**, a **quantity**, and the **cash
paid** — then prints a receipt with the line item, the subtotal, and the change
due. There are no functions, no `if`, and no loops yet; the entire point is to get
fluent with the four skills this chapter is about:

- **defining and initializing** variables with braces (1.3, 1.4),
- **reading input** with a chained `std::cin >> a >> b >> c;` (1.5),
- **computing** with arithmetic operators and expressions — `unitPrice * quantity`
  for the subtotal, `amountPaid - subtotal` for the change (1.9, 1.10), and
- **printing** the result with `std::cout` and `'\n'` (1.5).

The starter compiles but prints all zeros; your job is to fill in the three marked
blocks and turn the grader from red to green. Every tool you need is in the lessons
above — go build it.
