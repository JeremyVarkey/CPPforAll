# Chapter 28 — Input and Output

> Source: <https://www.learncpp.com/> (Chapter 28)
>
> Goal for CS6340: read and write C++ stream code confidently enough to load
> seeds, print diagnostics, validate input, use string streams, and work with
> files while building Lab 1-style tooling.

## Contents

- [28.1 — Input and output streams](#281--input-and-output-streams)
- [28.2 — Input with `istream`](#282--input-with-istream)
- [28.3 — Output with `ostream` and `ios`](#283--output-with-ostream-and-ios)
- [28.4 — Stream classes for strings](#284--stream-classes-for-strings)
- [28.5 — Stream states and input validation](#285--stream-states-and-input-validation)
- [28.6 — Basic file I/O](#286--basic-file-io)
- [28.7 — Random file I/O](#287--random-file-io)
- [Chapter 28 summary](#chapter-28-summary)
- [CS6340 patterns](#cs6340-patterns)
- [Mini drill](#mini-drill)

---

## 28.1 — Input and output streams

You have been using input and output since your very first program. `std::cout << "Hello\n";` printed a greeting; `std::cin >> x;` read a number. What you have *not* yet done is stop and ask what those names actually are, or why the same two operators — `<<` and `>>` — keep showing up no matter where the data is going. This chapter answers that, and the answer turns out to be one of the most elegant ideas in C++.

The unifying concept is the **stream**.

### What a stream is

A stream is an abstraction for a *sequence of characters flowing from a source or to a destination*. That is the whole idea. The word "stream" is well chosen: think of a river of characters moving in one direction. An **input stream** carries characters *toward* your program; an **output stream** carries characters *away* from it.

```text
input stream:

keyboard / file / string / network-ish source
        |
        v
     program

output stream:

     program
        |
        v
terminal / file / string / log-ish destination
```

The power of the abstraction is that your program does not need to know or care what is on the other end of the pipe. A stream hides the source or destination behind a uniform interface. The same `<<` that prints to the terminal can write to a file or build up a string in memory — because all three are just streams, and a stream is a stream.

### The standard streams

You already know the most common streams. They come from `<iostream>` and are available to every program:

```cpp
#include <iostream>

std::cout // standard output  — normal program output
std::cin  // standard input   — keyboard / piped input
std::cerr // standard error   — diagnostics, unbuffered
std::clog // standard logging — diagnostics, buffered
```

The split between `std::cout` and `std::cerr` is worth internalizing early. Both usually appear on your terminal, but they are *different* streams. `std::cout` is for the program's real output — the answer. `std::cerr` is for errors and diagnostics — the commentary. Keeping them separate means a user can redirect the answer to a file while still seeing error messages on screen, or vice versa. You will lean on this distinction constantly when you write tools.

> **Best practice:** Send normal program output to `std::cout` and diagnostics, warnings, and errors to `std::cerr`. They look the same on screen but can be redirected independently.

### Output: the insertion operator `<<`

`operator<<` **inserts** data into an output stream.

```cpp
#include <iostream>

int main()
{
    std::cout << "Hello\n";
    std::cerr << "Error message\n";

    return 0;
}
```

Read this line:

```cpp
std::cout << value;
```

literally as *"send `value` into `std::cout`."* The angle brackets even point in the direction the data flows — out of your code and into the stream.

### Input: the extraction operator `>>`

`operator>>` **extracts** data from an input stream.

```cpp
#include <iostream>

int main()
{
    int x {};

    std::cin >> x;

    std::cout << "You entered " << x << '\n';

    return 0;
}
```

Read this line:

```cpp
std::cin >> x;
```

as *"read from `std::cin` into `x`."* Again the brackets point the way: data flows out of the stream and into your variable.

> **Tip:** The arrows always point in the direction the data moves. `cout << x` pushes `x` out to the stream; `cin >> x` pulls from the stream into `x`. If you ever blank on which operator to use, picture the flow.

### The stream class family

`std::cin` and `std::cout` are not magic globals — they are *objects* of specific stream classes. Here are the ones you will meet in this chapter:

| Kind | Header | Use |
| --- | --- | --- |
| `std::istream` | `<istream>` / `<iostream>` | input |
| `std::ostream` | `<ostream>` / `<iostream>` | output |
| `std::iostream` | `<iostream>` | both input and output |
| `std::ifstream` | `<fstream>` | input file stream |
| `std::ofstream` | `<fstream>` | output file stream |
| `std::fstream` | `<fstream>` | input/output file stream |
| `std::istringstream` | `<sstream>` | input from a string |
| `std::ostringstream` | `<sstream>` | output to a string |
| `std::stringstream` | `<sstream>` | input/output string stream |

These classes are related by inheritance. You do not need to memorize the tree, but seeing it once explains *why* the same operators work everywhere:

```text
ios_base
   |
 basic_ios
   |
   +-- basic_istream  -> istream, ifstream, istringstream
   |
   +-- basic_ostream  -> ostream, ofstream, ostringstream
```

The practical takeaway is the only thing you must remember: **the file streams and string streams are kinds of `istream`/`ostream`.** A `std::ifstream` *is an* `std::istream`; a `std::ofstream` *is an* `std::ostream`. So everything you know about `>>` on `std::cin` works on a file. Everything you know about `<<` on `std::cout` works on a string stream. You learn the operators once and reuse them forever.

### Why this matters: stream polymorphism

Here is the payoff. Because file streams and string streams inherit from `ostream`, you can write *one* function that accepts a `std::ostream&` and it will write to the console, a file, or a string without changing a line:

```cpp
#include <ostream>
#include <string>

void writeResult(std::ostream& out, const std::string& seed, bool crashed)
{
    out << seed << ',' << crashed << '\n';
}
```

The function says "give me somewhere to write" without caring where that is. Now the same code serves three destinations:

```cpp
writeResult(std::cout, "abc", false);            // to the terminal
```

```cpp
std::ofstream file { "results.csv" };
writeResult(file, "abc", false);                 // to a file
```

```cpp
std::ostringstream text {};
writeResult(text, "abc", false);                 // into a string
std::string line { text.str() };
```

This is a clean, reusable design: **depend on `std::ostream&` (or `std::istream&`) when you only need to write somewhere (or read from somewhere).** Your function becomes testable — you can feed it a string stream in a unit test — and flexible, all for free. The chapter's lab leans on exactly this idea, and so does CS6340 Lab 1, where the same writer serves both the console and a results file.

> **Key insight:** Accepting `std::ostream&`/`std::istream&` by reference makes a function *stream-polymorphic*: one implementation, many sources and destinations. This single trick is the reason streams are worth understanding deeply rather than memorizing.

---

## 28.2 — Input with `istream`

`std::istream` is the base input stream type that sits behind `std::cin`, `std::ifstream`, and `std::istringstream`. Learn its behavior once and it applies to all three. In this lesson we focus on the workhorse — the extraction operator `>>` — and the gotchas that trip up nearly everyone the first time.

### Formatted extraction skips leading whitespace

The extraction operator `>>` performs **formatted input**: it interprets the characters according to the type of the variable you are reading into. A key part of that behavior is that it **skips leading whitespace** (spaces, tabs, newlines) before it starts reading.

```cpp
int x {};
std::cin >> x;
```

If the user types this, with spaces in front:

```text
   42
```

the leading spaces are discarded and `x` receives `42`. That convenience is exactly what you want when reading numbers — you do not have to worry about stray spaces.

For strings, formatted extraction also *stops* at the first whitespace after it has read something:

```cpp
std::string word {};
std::cin >> word;
```

Given the input:

```text
hello world
```

`word` receives only:

```text
hello
```

The space ends the extraction, leaving `world` in the buffer. So `>>` reads one whitespace-delimited token at a time. This is perfect for "read three numbers separated by spaces," and useless for "read a sentence." For the latter, you need a different tool.

### Reading a whole line with `std::getline`

When spaces are part of the data — a full name, a line of a file, a sentence — use `std::getline`, which reads everything up to (and consuming, but not storing) the next newline:

```cpp
#include <iostream>
#include <string>

int main()
{
    std::string line {};

    std::getline(std::cin, line);

    std::cout << "line: " << line << '\n';

    return 0;
}
```

Input:

```text
hello world
```

stores the whole thing:

```text
hello world
```

So the rule of thumb is simple: `>>` for tokens, `std::getline` for lines.

### The classic trap: mixing `>>` and `getline`

This is the bug that every C++ programmer writes exactly once before learning the lesson. Suppose you read a number, then a line:

```cpp
int count {};
std::string name {};

std::cin >> count;
std::getline(std::cin, name);
```

With this input:

```text
3
Jeremy
```

you would expect `count == 3` and `name == "Jeremy"`. Instead, `name` comes back **empty**. Why?

The answer is in the buffer. `std::cin >> count` reads the `3` and stops — but it leaves the newline you pressed after the `3` sitting in the stream. It does not consume that newline, because `>>` only skips *leading* whitespace, not trailing. Then `std::getline` runs, sees that leftover newline *immediately*, and concludes the line is over before it reads any characters. You get an empty string.

```text
input buffer before extraction:

[3][\n][J][e][r][e][m][y][\n]

after std::cin >> count:

[\n][J][e][r][e][m][y][\n]
 ^
 getline stops here at once, returning ""
```

The fix is to consume that stray whitespace first. The cleanest way is the `std::ws` manipulator, which tells the stream to skip leading whitespace before `getline` reads:

```cpp
std::cin >> count;
std::getline(std::cin >> std::ws, name); // skip the leftover newline first
```

`std::cin >> std::ws` discards leading whitespace (including the dangling newline) and returns the stream, which `getline` then reads from. Now `name` receives `"Jeremy"`.

> **Warning:** After reading with `>>`, a newline is left in the input buffer. If a `std::getline` follows, it will read that empty leftover and return `""`. Use `std::getline(std::cin >> std::ws, str)` — or an explicit `ignore` — to clear it first.

### Character-level tools: `get`, `peek`, and `ignore`

Sometimes you need to work below the level of formatted extraction, one character at a time. Three member functions give you that control.

`get()` reads a single character, *including* whitespace (it does not skip):

```cpp
char ch {};
std::cin.get(ch);
```

`peek()` looks at the next character *without* removing it from the stream — useful when you want to decide what to do before committing to a read:

```cpp
char next { static_cast<char>(std::cin.peek()) };
```

`ignore()` discards characters. With one argument it discards that many; with two it discards up to (and including) a delimiter, whichever comes first:

```cpp
std::cin.ignore(1000, '\n'); // discard up to 1000 chars, or up to a newline
```

The magic number `1000` is a guess at "a lot." A more honest "discard the rest of this line no matter how long" uses the largest possible stream size:

```cpp
#include <limits>

std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
```

This means "ignore characters until you hit a newline, however many that takes." You will see this exact incantation constantly, especially paired with `clear()` after bad input:

```cpp
std::cin.clear();                                                   // reset error flags
std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // throw away the bad line
```

We will return to that pair in 28.5 — it is the standard recovery move after a failed read. For now, just register the shape of it.

### Putting it together: a reusable input function

Recall the stream-polymorphism idea from 28.1. It works for input too. A function that takes a `std::istream&` can read a seed from the keyboard, a file, or a string with no changes:

```cpp
#include <istream>
#include <string>

bool readSeed(std::istream& in, std::string& seed)
{
    return static_cast<bool>(std::getline(in, seed));
}
```

The same function serves all three sources:

```cpp
readSeed(std::cin, seed);                          // from the keyboard
```

```cpp
std::ifstream file { "seeds.txt" };
readSeed(file, seed);                              // from a file
```

```cpp
std::istringstream text { "abc\nxyz\n" };
readSeed(text, seed);                              // from a string (great for tests)
```

The cast to `bool` deserves a note: `std::getline` returns the stream, and a stream converts to `true` while it is still readable and `false` once a read has failed (end of input or error). So `readSeed` returns `true` while there is more to read and `false` when the source is exhausted — the perfect condition for a `while` loop. We make that conversion explicit here, but it works implicitly inside `if` and `while`, which is exactly where we are headed in 28.5.

---

## 28.3 — Output with `ostream` and `ios`

`std::ostream` is the output stream type behind `std::cout`, `std::ofstream`, and `std::ostringstream`. Where 28.2 was about getting data *in* cleanly, this lesson is about getting data *out* in the shape you want — aligned columns, fixed decimals, named booleans. The tool for raw insertion is the operator you already know; the tools for *formatting* are the manipulators in `<iomanip>`.

### Insertion and chaining

You insert values with `operator<<`:

```cpp
std::cout << "seed=" << seed << " crashed=" << crashed << '\n';
```

That one statement does several insertions in a row, and it works because of a small but important detail: **each insertion returns the stream itself.** So the expression chains left to right:

```text
((std::cout << "seed=") << seed) << ...
```

`std::cout << "seed="` evaluates to `std::cout`, which is then the left operand of the next `<<`, and so on. This is the same reason the input chain `in >> a >> b >> c` works. The returned-stream trick is what makes the fluent `<< ... << ... << ...` style possible at all.

### `std::endl` versus `'\n'`

There are two common ways to end a line, and they are not equivalent.

```cpp
std::cout << "done\n";          // newline only
std::cout << "done" << std::endl; // newline AND flush
```

`'\n'` writes a newline character. `std::endl` writes a newline *and then flushes the stream* — it forces any buffered output to be physically written out immediately. Flushing has a cost: streams buffer output precisely because writing in big batches is far faster than writing one character at a time. Flushing on every line throws that optimization away.

So the guidance is: prefer `'\n'` for ordinary line breaks. Reach for `std::endl` only when you have a specific reason to flush *now* — for example, to make sure a message appears on screen before the program does something risky that might crash before the buffer would naturally flush.

```cpp
for (const std::string& seed : seeds)
{
    std::cout << seed << '\n'; // '\n' in a hot loop; std::endl would flush every iteration
}
```

> **Best practice:** Use `'\n'` for normal output. Use `std::endl` only when you genuinely need an immediate flush. In loops, prefer `'\n'` — flushing every iteration can dramatically slow a program.

### Formatting numbers with `<iomanip>`

To control how values look, include the manipulators header:

```cpp
#include <iomanip>
```

**Field width** with `std::setw` pads the next item to a minimum width:

```cpp
std::cout << std::setw(8) << 42 << '\n'; // "      42" (right-aligned in 8 cols)
```

**Precision** with `std::setprecision` controls how many digits a floating-point value shows:

```cpp
double ratio { 1.0 / 3.0 };
std::cout << std::setprecision(3) << ratio << '\n'; // 0.333
```

**Fixed notation** with `std::fixed` makes `setprecision` mean "digits after the decimal point," giving you clean money-style output:

```cpp
std::cout << std::fixed << std::setprecision(2) << 3.14159 << '\n'; // 3.14
```

**Boolean names** with `std::boolalpha` print `true`/`false` instead of `1`/`0`:

```cpp
std::cout << std::boolalpha   << true << '\n'; // true
std::cout << std::noboolalpha << true << '\n'; // 1
```

**Number base** with `std::dec`, `std::hex`, and `std::oct` controls how integers are printed:

```cpp
std::cout << std::dec << 31 << '\n'; // 31
std::cout << std::hex << 31 << '\n'; // 1f
std::cout << std::oct << 31 << '\n'; // 37
```

### The one rule that bites everyone: persistence

Here is the subtlety that the lab's formatting task hinges on. **Some manipulators stick to the stream until you change them back; others apply only to the very next item.**

Most format *state* manipulators — `std::hex`, `std::fixed`, `std::setprecision`, `std::boolalpha` — are **sticky**. Once set, they affect every subsequent insertion of the relevant kind:

```cpp
std::cout << std::hex << 31 << '\n'; // 1f
std::cout << 31 << '\n';             // STILL 1f — hex is still in effect!
std::cout << std::dec;               // you must switch back yourself
```

`std::setw`, by contrast, is a **one-shot**: it applies only to the immediately following insertion and then resets to zero on its own.

```cpp
std::cout << std::setw(6) << 42 << 7 << '\n'; // "    42" then "7" with no padding
```

This asymmetry is not arbitrary. Width is naturally a per-field thing (you set it fresh for each column), whereas precision or base is naturally a mode you stay in. But it does mean you must reset sticky manipulators yourself when you are done with them:

```cpp
std::cout << std::dec; // good habit: restore decimal after any hex diagnostics
```

> **Warning:** `std::setw` resets after one insertion, but `std::fixed`, `std::setprecision`, `std::hex`, and `std::boolalpha` persist on the stream. If a later, unrelated value comes out in the wrong base or with the wrong decimals, a leftover sticky manipulator is almost always the cause. Reset it explicitly.

### Designing output functions

Bring back the stream-polymorphism principle one more time. Write your output routines against `std::ostream&` so they work for console, file, and string alike:

```cpp
#include <ostream>
#include <string>

struct Result
{
    std::string seed;
    bool crashed {};
    int coverage {};
};

void writeResult(std::ostream& out, const Result& result)
{
    out << result.seed << ','
        << std::boolalpha << result.crashed << ','
        << result.coverage << '\n';
}
```

Because it takes any `ostream`, you can test it against a string stream before you ever touch a file — capture the output as a string and inspect it:

```cpp
std::ostringstream out {};
writeResult(out, Result { "abc", false, 12 });

std::cout << out.str(); // abc,false,12
```

That last move — write to a string stream, then read back `.str()` and check it — is the foundation of the chapter's lab. String streams deserve their own lesson, which is next.

---

## 28.4 — Stream classes for strings

So far our streams have talked to the outside world — the keyboard, the terminal, files. **String streams** turn the same machinery inward: they let you treat an ordinary `std::string` as if it were a stream. This sounds modest, but it is one of the most useful tools in everyday C++, because it lets you *parse* and *build* structured text using the exact same `>>` and `<<` you already know.

Include:

```cpp
#include <sstream>
```

There are three types, mirroring the input/output split you have already seen:

```cpp
std::istringstream  // read FROM a string  (an input stream backed by a string)
std::ostringstream  // write TO a string   (an output stream backed by a string)
std::stringstream   // read and write a string buffer
```

### `std::istringstream` — parse text apart

Construct an `istringstream` from a string and it behaves *exactly* like `std::cin`, except it draws characters from that string instead of the keyboard. This makes parsing a line of structured fields trivial:

```cpp
#include <iostream>
#include <sstream>
#include <string>

int main()
{
    std::string line { "abc 12 1" };
    std::istringstream in { line };

    std::string seed {};
    int coverage {};
    bool crashed {};

    in >> seed >> coverage >> crashed;

    std::cout << seed << ' ' << coverage << ' ' << crashed << '\n'; // abc 12 1

    return 0;
}
```

Each `>>` skips whitespace and extracts one token, converting it to the target type along the way — `"abc"` to a string, `12` to an `int`, `1` to a `bool`. You get all of formatted extraction's type conversion and whitespace handling, applied to a string you already have in hand.

```text
string line:

"abc 12 1"
   |
   v
istringstream
   |
   +--> seed     = "abc"
   +--> coverage = 12
   +--> crashed  = true
```

This is the standard way to split a line into typed fields, and it is precisely what Task 1 of the lab asks you to do: wrap a `"name score"` line in an `istringstream` and extract a string and a `double`.

### `std::ostringstream` — build text up

An `ostringstream` is the mirror image: insert into it with `<<`, then retrieve the accumulated text with `.str()`. It is the clean alternative to gluing strings together by hand:

```cpp
#include <sstream>
#include <string>

std::string makeReportLine(const std::string& seed, bool crashed, int coverage)
{
    std::ostringstream out {};

    out << seed << ',' << std::boolalpha << crashed << ',' << coverage;

    return out.str();
}
```

Without a string stream, you would have to convert `crashed` and `coverage` to strings yourself and concatenate everything, which is verbose and error-prone. With one, you reuse all of `<<`'s formatting — including the `<iomanip>` manipulators from 28.3 — to assemble a string. This is the heart of Task 2 of the lab, where you build an aligned table row with `std::setw` and friends and return `out.str()`.

### `std::stringstream` — both directions

When you need to write *and then* read the same buffer, use `std::stringstream`:

```cpp
std::stringstream stream {};

stream << "42";   // write into the buffer

int value {};
stream >> value;  // read it back out, converted — value == 42
```

A common use is converting between text and numbers in both directions. But for most code, you should prefer the *specific* type:

- `std::istringstream` when you only read,
- `std::ostringstream` when you only write.

The specific type documents your intent at a glance, and the lab actually *requires* this: using `stringstream` where `istringstream` or `ostringstream` would do is forbidden, precisely because the specific name communicates more.

> **Best practice:** Prefer `std::istringstream` for parsing and `std::ostringstream` for building. Reach for `std::stringstream` only when you genuinely read and write the same buffer. The narrower type makes the code's intent obvious.

### Reusing a string stream

If you reuse a single stream for multiple parses, you must reset *two* separate things: the error **state** and the stored **contents**. Forgetting either is a classic bug.

```cpp
std::stringstream stream {};

stream << "bad";

int value {};
stream >> value; // fails — "bad" is not an int, so failbit is set

stream.clear();  // reset the error flags (failbit, etc.)
stream.str("");  // reset the stored string to empty

stream << "123";
stream >> value; // now succeeds — value == 123
```

`clear()` fixes the *state*; `str("")` fixes the *contents*. They are independent, and a stream that has failed will keep failing until you `clear()` it (you will see exactly this in 28.5).

For beginner-friendly code, though, the simplest move is usually to just *make a fresh stream* for each line. A local stream inside a loop is constructed clean every iteration, so there is nothing to reset:

```cpp
for (const std::string& line : lines)
{
    std::istringstream in { line }; // fresh, clean stream each pass
    // parse this line...
}
```

> **Tip:** When parsing many lines, prefer constructing a new `std::istringstream` per line over reusing one and resetting it. A fresh stream cannot carry stale state or leftover contents, which eliminates a whole category of bugs.

---

## 28.5 — Stream states and input validation

Up to now we have quietly assumed that reads succeed. They do not always. A user types `abc` when you asked for a number; a file ends in the middle of a record; a parse hits garbage. A robust program has to *notice* these failures and respond. The mechanism for noticing is the **stream state**, and the discipline of responding well is **input validation** — arguably the most practically important topic in this chapter.

### The state flags

Every stream carries a small set of internal flags that record how it is doing:

| State | Meaning |
| --- | --- |
| `goodbit` | no error; everything is fine |
| `eofbit` | end of input has been reached |
| `failbit` | the last formatted operation failed (e.g. expected a number, got letters) |
| `badbit` | a serious, usually unrecoverable stream error occurred |

You rarely query these flags by name. Instead you use the stream's most important feature: **a stream converts to a boolean.** A stream is `true` while it is in a good state and `false` once `failbit` or `badbit` is set. That single conversion powers nearly every input idiom you will write.

You can test the stream directly:

```cpp
if (std::cin)
{
    // the stream is in a good state
}
```

But far more useful is testing the *result of an extraction*, because `>>` returns the stream:

```cpp
int value {};

if (std::cin >> value)
{
    // read succeeded — value holds the number
}
else
{
    // read failed — the input was not a valid integer
}
```

This is the single most important input pattern in C++. **The extraction and the success check happen in one expression.** And because `>>` chains, you can check several reads at once — if *any* of them fails, the whole condition is false:

```cpp
if (in >> name >> score) // true only if BOTH extractions succeeded
{
    // both name and score are valid
}
```

That exact idiom is what Task 1 of the lab is built around, and what Task 5 turns into a loop.

> **Key insight:** `if (in >> x)` and `while (in >> x)` are the canonical input idioms. The extraction returns the stream, and the stream is `true` only if the read succeeded. Read input and check success in a single expression — do not split them.

### What happens when extraction fails

It is worth being precise about the aftermath of a failed read, because the consequences surprise people. Suppose you ask for an integer:

```cpp
int value {};
std::cin >> value;
```

and the user types:

```text
abc
```

Then, in order:

- the extraction **fails** — `abc` is not an integer;
- `failbit` is **set**, so the stream now evaluates to `false`;
- `value` is **not** assigned the input (in C++11 and later it is set to `0`, but the point is it does not hold what the user typed);
- the offending characters are **left in the buffer** — `>>` could not consume them;
- and crucially, **every future extraction also fails immediately**, because the stream stays in its failed state until you clear it.

That last point is the trap. A failed stream does not recover on its own. If you loop trying to read again without fixing the state, you get an infinite loop of instant failures.

### The recovery pattern

To recover, you do two things: reset the state, then discard the bad input that is still sitting in the buffer.

```cpp
#include <iostream>
#include <limits>

std::cin.clear();                                                   // 1. reset the error flags
std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // 2. throw away the bad line
```

`clear()` returns the stream to `goodbit` so reads can succeed again. `ignore(...)` removes the leftover characters up to and including the next newline, so the *next* read starts fresh. Skip either step and you are stuck: `clear()` without `ignore()` re-reads the same bad characters; `ignore()` without `clear()` does nothing useful because the stream is still failed.

> **Warning:** A failed stream stays failed. Until you call `clear()`, every further extraction returns immediately without reading. After clearing, `ignore()` the bad input still in the buffer, or your next read will choke on the same characters.

### A complete validation loop

Combine the pattern with a loop and you get the standard "keep asking until the input is valid" routine:

```cpp
#include <iostream>
#include <limits>

int readInt()
{
    while (true)
    {
        std::cout << "Enter an integer: ";

        int value {};
        if (std::cin >> value)
        {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return value; // success: clear the rest of the line, hand back the value
        }

        std::cin.clear();                                                   // failure: reset state
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // discard bad input

        std::cout << "Invalid input\n";
    }
}
```

The logic reads cleanly:

```text
loop forever:
    try to extract an integer
    if it succeeds:
        discard the rest of the line  (e.g. trailing junk after the number)
        return the value
    if it fails:
        clear the stream state
        discard the bad input
        announce the error and retry
```

Notice that even on *success* we `ignore` to the end of the line. That discards any stray characters the user typed after a valid number, so they do not contaminate the next read.

### Two kinds of validation are not the same

Parsing successfully and being *acceptable* are different questions. `42` parses as an `int` whether or not your program wants a value in `[1, 100]`. So validation has two independent layers:

```text
type validation:   did the input parse as the expected type?
range validation:  is the parsed value acceptable to this program?
```

Range validation builds on type validation:

```cpp
int readIntBetween(int min, int max)
{
    while (true)
    {
        int value { readInt() };          // type validation (parses as int)

        if (value >= min && value <= max) // range validation (is it allowed?)
            return value;

        std::cout << "Enter a value from " << min << " to " << max << '\n';
    }
}
```

Task 1 of the lab does exactly this two-step: it checks that the line parsed (`if (in >> name >> score)`) *and* that the score falls in `[0.0, 100.0]`. Both must hold for the record to be `ok`.

### Validating strings and lines

For strings, extraction almost always *succeeds* as long as there is input at all — a string can hold practically anything. So the meaningful validation is on the *contents*, which you check yourself:

```cpp
bool isValidSeed(const std::string& seed)
{
    return !seed.empty() && seed.size() <= 4096;
}
```

Then read lines and screen each one:

```cpp
std::string seed {};

while (std::getline(std::cin, seed)) // loop ends when input is exhausted
{
    if (!isValidSeed(seed))
    {
        std::cerr << "invalid seed\n";  // diagnostic goes to cerr, not cout
        continue;
    }

    runTarget(seed);
}
```

The `while (std::getline(...))` condition is the same boolean-stream trick: `getline` returns the stream, which is `true` while there is more to read and `false` at end of input. This loop is the bread and butter of reading a file or a corpus line by line.

### When `>>` is too lenient: stricter numeric parsing

One sharp edge of formatted extraction is that it is happy to read a *prefix*. Given the token:

```text
123abc
```

`>> value` will read `123` and stop, leaving `abc` behind — it considers that a success. Sometimes you want stricter: the *whole* token must be a number or you reject it. The modern tool for that is `std::from_chars`, which tells you both whether parsing succeeded and exactly where it stopped:

```cpp
#include <charconv>
#include <string>

bool parseIntStrict(const std::string& text, int& value)
{
    const char* begin { text.data() };
    const char* end   { text.data() + text.size() };

    auto result { std::from_chars(begin, end, value) };

    return result.ec == std::errc{}   // parsing succeeded, AND
        && result.ptr == end;         // it consumed the ENTIRE string
}
```

The two conditions say it all: parsing must succeed *and* must consume the whole string, so a trailing `abc` causes rejection. You will not strictly need `std::from_chars` for the lab — but the broader lesson is worth keeping: when correctness matters, reading a whole line and then validating it explicitly is often cleaner and safer than chaining interactive `std::cin >> value` reads.

---

## 28.6 — Basic file I/O

Everything you have learned about `std::cin` and `std::cout` now transfers, almost unchanged, to files. That is the dividend of the inheritance hierarchy from 28.1: a file stream *is a* stream, so the same `<<`, `>>`, and `std::getline` work on it. The only genuinely new ideas are opening a file, checking that the open succeeded, and a couple of options for *how* to open it.

File streams live in:

```cpp
#include <fstream>
```

and come in three flavors:

```cpp
std::ifstream // input file stream  — read from a file
std::ofstream // output file stream — write to a file
std::fstream  // input/output file stream — both
```

You open a file simply by naming it when you construct the stream. The stream's *destructor* closes the file automatically, so you almost never call `.close()` yourself — when the stream goes out of scope, it flushes and closes. This is RAII at work: the file's lifetime is tied to the object's lifetime.

### Reading a file line by line

```cpp
#include <fstream>
#include <iostream>
#include <string>

int main()
{
    std::ifstream file { "seeds.txt" };

    if (!file)
    {
        std::cerr << "could not open seeds.txt\n";
        return 1;
    }

    std::string line {};
    while (std::getline(file, line))
    {
        std::cout << "seed: " << line << '\n';
    }

    return 0;
}
```

Two lines carry the weight here. First:

```cpp
if (!file)
```

This is the **open-success check**, and you must write it every single time you open a file. Opening can fail for many reasons — the file does not exist, you lack permission, the path is wrong. `if (!file)` asks "did the open fail?" using the same boolean-stream conversion you already know. Skip this check and your program will silently read nothing (or, worse, write nowhere) and you will spend an afternoon confused.

> **Best practice:** After opening any file stream, immediately check `if (!file)` (or `if (!file.is_open())`) and handle the failure. An unchecked open that quietly failed is one of the most common file-I/O bugs.

Second, the loop:

```cpp
while (std::getline(file, line))
```

is exactly the line-reading idiom from 28.2 and 28.5 — `getline` returns the stream, which becomes `false` at end of file, ending the loop. Reading every line of a file is this one loop, and you will write it dozens of times.

### Writing a file

```cpp
#include <fstream>
#include <iostream>

int main()
{
    std::ofstream file { "results.txt" };

    if (!file)
    {
        std::cerr << "could not open results.txt\n";
        return 1;
    }

    file << "abc,false,12\n"; // same << as std::cout — it just lands on disk
    file << "xyz,true,4\n";

    return 0;
}
```

Writing is identical to printing, except the characters go to disk instead of the screen. There is one behavior to internalize: **by default, opening an `ofstream` truncates the file** — it erases any existing contents and starts from empty. Open `results.txt` for writing and whatever was there before is gone. Usually that is what you want (you are producing fresh output), but if it is not, you need a different mode.

### Open modes

You control how a file is opened by passing a second argument. The modes you will actually reach for:

| Mode | Meaning |
| --- | --- |
| `std::ios::in` | open for reading |
| `std::ios::out` | open for writing |
| `std::ios::app` | append — every write goes to the end of the file |
| `std::ios::ate` | open and seek to the end initially |
| `std::ios::trunc` | truncate (erase) an existing file |
| `std::ios::binary` | binary mode — no text translation |

The one you will want most often is **append**, for adding to a file without erasing it:

```cpp
std::ofstream file { "results.txt", std::ios::app }; // writes go to the end; nothing is erased
```

Modes combine with the bitwise-OR operator `|` when you need more than one:

```cpp
std::ofstream file { "out.bin", std::ios::binary | std::ios::trunc };
```

### A trap worth its own section: working directory

A relative path like `"seeds.txt"` is resolved relative to the program's **current working directory** — the directory the program is *run from* — which is **not** necessarily where your source file lives.

```text
source file:        project/src/main.cpp
program run from:    project/build/
relative open:       "seeds.txt"
actual path tried:   project/build/seeds.txt
```

So a file you can plainly see sitting next to your `.cpp` may appear "missing" because the program is running from a build directory and looking there instead. When a file "does not exist" even though you are staring right at it, **check the working directory** before anything else.

> **Warning:** Relative paths are resolved from the program's working directory, not the source file's location. This bites hard inside build directories, Docker containers, and autograder harnesses — for CS6340, the grader runs from a specific directory, so pass paths exactly as the harness expects and never hard-code an absolute path.

### Reading a whole file into a vector

A very common need is to slurp every line of a file into a `std::vector<std::string>`. Here it is, with the open check baked in:

```cpp
#include <fstream>
#include <string>
#include <vector>

std::vector<std::string> readLines(const std::string& path)
{
    std::ifstream file { path };

    if (!file)
        return {}; // open failed — return an empty vector

    std::vector<std::string> lines {};
    std::string line {};

    while (std::getline(file, line))
        lines.push_back(line);

    return lines;
}
```

Returning `{}` on failure is the quiet option — the caller gets an empty vector and may not realize the file was missing rather than empty. Depending on the program, you might prefer to print an error, throw an exception (Chapter 27), or return a status object. Choose loudness to match how much the failure matters.

### Writing structured results

Pulling the chapter together, here is a routine that writes a vector of structs to a file as CSV. It combines output streams, file streams, range-based loops, formatting, and structs — and it is almost exactly what CS6340 Lab 1 asks for:

```cpp
struct Result
{
    std::string seed;
    bool crashed {};
    int coverage {};
};

void writeResults(const std::string& path, const std::vector<Result>& results)
{
    std::ofstream file { path };

    if (!file)
    {
        std::cerr << "could not open " << path << '\n';
        return;
    }

    for (const Result& result : results)
    {
        file << result.seed << ','
             << std::boolalpha << result.crashed << ','
             << result.coverage << '\n';
    }
}
```

This is the territory of Task 4 in the lab: open an `ofstream`, check it, write a report string, and later open an `ifstream` to read the first line back with `getline`. Same operators, same idioms — just pointed at disk.

---

## 28.7 — Random file I/O

So far every file read or write has been **sequential**: you start at the beginning and move steadily toward the end, byte after byte. That covers most text processing. But sometimes you need to *jump* — read a header, leap to byte 1000, write four bytes in the middle, come back. That is **random file I/O**: moving to an arbitrary position in a file and reading or writing there. It is more powerful and considerably more fragile than line-by-line text I/O, so we treat it as advanced background.

```text
sequential:           random access:

read byte 0           jump to byte 1000
read byte 1           read 20 bytes
read byte 2           jump to byte 12
...                   write 4 bytes
```

### Seek and tell

Two pairs of operations move and report the position. The naming convention is a small piece of history worth learning, because it explains the otherwise cryptic function names:

```text
g = get = reading      (input position)
p = put = writing      (output position)
```

For the **read** position (input streams):

```cpp
file.seekg(position);      // seek-get: move the read position
auto pos { file.tellg() }; // tell-get: report the read position
```

For the **write** position (output streams):

```cpp
file.seekp(position);      // seek-put: move the write position
auto pos { file.tellp() }; // tell-put: report the write position
```

`seek` moves you; `tell` tells you where you are.

### Finding a file's size

A classic use of seek/tell is measuring a file: jump to the end, ask the position (which equals the size in bytes), jump back to the start.

```cpp
#include <fstream>
#include <iostream>

int main()
{
    std::ifstream file { "input.txt", std::ios::binary };

    if (!file)
    {
        std::cerr << "open failed\n";
        return 1;
    }

    file.seekg(0, std::ios::end);            // jump to the end
    std::streampos size { file.tellg() };    // position == size in bytes
    file.seekg(0, std::ios::beg);            // jump back to the start

    std::cout << "size: " << size << " bytes\n";

    return 0;
}
```

The two-argument `seekg` form takes an offset and an *origin* to measure it from:

| Origin | Meaning |
| --- | --- |
| `std::ios::beg` | the beginning of the file |
| `std::ios::cur` | the current position |
| `std::ios::end` | the end of the file |

So `seekg(0, std::ios::end)` means "0 bytes from the end" — i.e. the very end — and `seekg(0, std::ios::beg)` returns you to the start.

### Binary read and write

Formatted `>>` and `<<` are for *text* — they interpret and convert characters. For raw bytes, you want the `read` and `write` members, which move bytes verbatim, and you want **binary mode** so the platform does not silently translate line endings.

Reading every byte of a file into a buffer:

```cpp
#include <fstream>
#include <vector>

std::vector<char> readBytes(const std::string& path)
{
    std::ifstream file { path, std::ios::binary };

    if (!file)
        return {};

    file.seekg(0, std::ios::end);
    std::streamsize size { file.tellg() };
    file.seekg(0, std::ios::beg);

    std::vector<char> bytes(static_cast<std::size_t>(size)); // size the buffer

    if (size > 0)
        file.read(bytes.data(), size); // read the raw bytes into contiguous storage

    return bytes;
}
```

A few things to register here:

- **binary mode** (`std::ios::binary`) avoids platform-specific text translation, so the bytes you read are exactly the bytes on disk;
- **`read`** moves raw characters into a buffer rather than parsing them;
- **`bytes.data()`** points at the vector's contiguous storage — `read` needs a plain `char*`;
- the **casts** between `std::streamsize` and `std::size_t` are deliberate: stream sizes and container sizes are different (and differently signed) types, so you convert carefully.

Writing the buffer back out is the mirror image:

```cpp
#include <fstream>
#include <vector>

void writeBytes(const std::string& path, const std::vector<char>& bytes)
{
    std::ofstream file { path, std::ios::binary };

    if (!file)
        return;

    file.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
}
```

### A worked example: byte mutation

Here is where this connects to fuzzing, the CS6340 backdrop. A simple mutation reads a seed's bytes, flips one bit somewhere in the middle, and writes the result as a new file:

```cpp
std::vector<char> bytes { readBytes("seed.bin") };

if (!bytes.empty())
{
    std::size_t index { bytes.size() / 2 };
    bytes[index] ^= 0x01; // flip the lowest bit at the middle byte
}

writeBytes("mutant.bin", bytes);
```

```text
seed.bin bytes:

index: 0    1    2    3
byte:  0x41 0x42 0x43 0x44

flip the low bit at index 2:

index: 0    1    2    3
byte:  0x41 0x42 0x42 0x44
```

The `^= 0x01` is an exclusive-OR with a single set bit, which toggles that bit — `0x43` (binary `...011`) becomes `0x42` (binary `...010`). A fuzzer makes thousands of such tiny perturbations, feeding each mutant to a target program to hunt for crashes.

### Handle with care

Random file I/O is genuinely more error-prone than line-based text I/O, so keep these hazards in mind:

- seeking past valid positions in the file;
- assuming a text file's bytes map cleanly to characters on every platform (they may not — newlines especially);
- mixing formatted extraction (`>>`) with raw `read` on the same stream without thinking it through;
- ignoring failed reads or writes;
- converting a *failed* `tellg()` result to an unsigned size — `tellg` returns `-1` on failure, and casting `-1` to an unsigned type yields a gigantic number.

That last one deserves a defensive habit. Check for the failure sentinel before trusting a position:

```cpp
auto pos { file.tellg() };

if (pos == std::streampos { -1 })
{
    // tellg failed — do NOT cast this to a size
}
```

> **Warning:** `tellg()` returns `-1` on failure. Cast that to `std::size_t` and you get an enormous positive number, which will then be used to size a buffer or seek — a recipe for crashes. Always check for the `-1` sentinel before converting a stream position to a size.

For the lab, and for most of Lab 1, ordinary line-based text I/O is all you need. Binary and random access are here so that when you meet byte-level mutation — or any code that pokes at a file's interior — the seek/tell vocabulary is already familiar rather than mysterious.

---

## Chapter 28 summary

This chapter rested on a single unifying idea — the **stream** — and showed how one interface serves the terminal, files, and strings alike. The essentials to carry forward:

- Streams model input and output as flows of characters; the same operators work no matter the source or destination.
- `std::cin`, `std::cout`, `std::cerr`, and `std::clog` are the standard streams. Send real output to `std::cout`, diagnostics to `std::cerr`.
- `operator<<` inserts into output streams; `operator>>` extracts from input streams. Both chain because each returns the stream.
- Formatted extraction (`>>`) skips leading whitespace and reads one token; `std::getline` reads a whole line.
- Mixing `>>` and `getline` leaves a newline behind. Clear it with `std::getline(in >> std::ws, str)` or `ignore`.
- `get`, `peek`, and `ignore` work at the character-buffer level.
- Prefer `'\n'` over `std::endl` unless you specifically need a flush.
- `<iomanip>` provides `setw`, `setprecision`, `fixed`, and `boolalpha`. Remember: `setw` resets after one item; most others persist until changed back.
- String streams let a `std::string` behave like a stream — `istringstream` parses text apart, `ostringstream` builds it up, retrieved with `.str()`.
- Streams track state (`eofbit`, `failbit`, `badbit`) and convert to `bool`. `if (in >> x)` and `while (in >> x)` are the canonical idioms.
- A failed stream stays failed. After bad input, `clear()` the state and `ignore()` the leftover characters before retrying.
- Validate both that input *parsed* and that it is *acceptable* — type validation and range/content validation are separate.
- File streams live in `<fstream>`: `ifstream` reads, `ofstream` writes, `fstream` does both. Always check `if (!file)` after opening.
- Opening an `ofstream` truncates by default; use `std::ios::app` to append.
- Relative paths resolve from the program's working directory, not the source file's location.
- Random file I/O uses seek/tell (`g` = read, `p` = write); use binary mode and raw `read`/`write` for byte-oriented work, and guard against a `-1` from `tellg()`.

## CS6340 patterns

The patterns below are the ones you will reach for again and again when building fuzzing and analysis tooling. They are nothing more than the chapter's idioms, assembled.

### Load seed lines

Read a corpus file into a vector, checking the open and skipping blank lines:

```cpp
std::vector<std::string> loadSeeds(const std::string& path)
{
    std::ifstream file { path };

    if (!file)
    {
        std::cerr << "could not open seed file: " << path << '\n';
        return {};
    }

    std::vector<std::string> seeds {};
    std::string line {};

    while (std::getline(file, line))
    {
        if (!line.empty())
            seeds.push_back(line);
    }

    return seeds;
}
```

### Write results to any stream

Make the writers stream-polymorphic so the same code serves the console and a file:

```cpp
void writeCsvHeader(std::ostream& out)
{
    out << "seed,crashed,coverage\n";
}

void writeCsvRow(std::ostream& out, const Result& result)
{
    out << result.seed << ','
        << std::boolalpha << result.crashed << ','
        << result.coverage << '\n';
}
```

Then point them at a file (or `std::cout`, unchanged):

```cpp
std::ofstream file { "results.csv" };

writeCsvHeader(file);

for (const Result& result : results)
    writeCsvRow(file, result);
```

### Parse a config line

Use an `istringstream` to pull typed fields out of a line, returning the stream's success as a `bool`:

```cpp
struct Config
{
    int trials {};
    int timeoutMs {};
};

bool parseConfigLine(const std::string& line, Config& config)
{
    std::istringstream in { line };

    return static_cast<bool>(in >> config.trials >> config.timeoutMs);
}
```

For stricter parsing, additionally confirm that no extra junk remains after the expected fields.

### Debug with `std::cerr`

Route status and diagnostic messages to `std::cerr` so they stay separate from the program's real output on `std::cout`:

```cpp
std::cerr << "running seed: " << seed << '\n';
```

That separation is what lets you redirect the results to a file while still watching progress on screen.

## Mini drill

Write a function that reads a file containing one seed per line and returns only the seeds whose length is at most `maxLength`.

Think before you peek: which stream type opens the file? Which check guards the open? Which loop reads every line? What comparison filters by length?

One possible solution:

```cpp
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

std::vector<std::string> loadBoundedSeeds(const std::string& path,
                                          std::size_t maxLength)
{
    std::ifstream file { path };

    if (!file)
    {
        std::cerr << "could not open " << path << '\n';
        return {};
    }

    std::vector<std::string> seeds {};
    std::string line {};

    while (std::getline(file, line))
    {
        if (line.size() <= maxLength)
            seeds.push_back(line);
    }

    return seeds;
}
```

Every piece is a chapter idiom doing its job:

- `std::ifstream` for file input;
- `if (!file)` for the open-success check;
- `std::getline` for full-line reading in a `while` loop driven by the stream's boolean state;
- `std::vector<std::string>` for dynamic storage;
- `std::size_t` for the length comparison, matching the type `std::string::size()` returns.

With this much under your belt, you are ready for the chapter's lab — the **Report Engine** — where you parse score lines with `istringstream`, format aligned rows with `ostringstream` and `<iomanip>`, round-trip a report through a real file, and count valid integers with the `while (in >> val)` idiom. Every task is one of the patterns above, made physical.
