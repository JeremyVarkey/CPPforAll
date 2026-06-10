# Chapter 10 — Type Conversion, Type Aliases, and Type Deduction

> Source: <https://www.learncpp.com/> (Chapter 10)

C++ is a statically typed language: every object, every expression, every value you write down has a type, fixed at compile time. But real programs are mongrels. You add an `int` to a `double`. You hand a `float` to a function that wanted a `double`. You compare a loop counter against the unsigned length of a string. In every one of these moments, the two types don't match — and yet the code compiles and runs. Something is quietly stepping in to bridge the gap.

That something is **type conversion**, and this chapter is about taking control of it. We'll see which conversions are safe and which silently throw away data, why `7 / 2` famously gives you `3` instead of `3.5`, how to *ask* for a conversion out loud with `static_cast`, and how brace initialization slams the door on the dangerous ones. Then we'll turn to two tools for taming the type system itself: **type aliases** (`using`), which give clumsy types readable names, and **type deduction** (`auto`), which lets the compiler figure out a type so you don't have to spell it. By the end you'll be able to look at almost any expression and answer the question that quietly governs correctness in C++: *what type does this actually become?*

## Contents
- [10.1 — Implicit type conversion](#101--implicit-type-conversion)
- [10.2 — Floating-point and integral promotion](#102--floating-point-and-integral-promotion)
- [10.3 — Numeric conversions](#103--numeric-conversions)
- [10.4 — Narrowing conversions, list initialization, and constexpr initializers](#104--narrowing-conversions-list-initialization-and-constexpr-initializers)
- [10.5 — Arithmetic conversions](#105--arithmetic-conversions)
- [10.6 — Explicit type conversion (casting) and static_cast](#106--explicit-type-conversion-casting-and-static_cast)
- [10.7 — Typedefs and type aliases](#107--typedefs-and-type-aliases)
- [10.8 — Type deduction for objects using the auto keyword](#108--type-deduction-for-objects-using-the-auto-keyword)
- [10.9 — Type deduction for functions](#109--type-deduction-for-functions)
- [10.x — Chapter 10 summary and quiz](#10x--chapter-10-summary-and-quiz)

---

## 10.1 — Implicit type conversion

### Why conversions have to exist

Start from the constraint. C++ is statically typed, so the type of every value is decided before the program ever runs. An `int` holds whole numbers in a particular bit pattern; a `double` holds real numbers in an entirely different one. They are not interchangeable at the level of the machine — the bits that spell `3` as an `int` are nothing like the bits that spell `3.0` as a `double`.

So what happens here?

```cpp
double d { 3 };       // we wrote an int literal, but d is a double
int x { 2 };
double y { x + 0.5 }; // x is an int, but it's added to a double
```

In each line the compiler is handed a value of one type where a value of *another* type is wanted. Rather than reject the code, it produces a new value of the needed type from the one you gave it. That act — **producing a value of one type from a value of another type** — is a *type conversion*.

```
source value:  int 3
conversion:    int  ->  double
result value:  double 3.0
```

Notice the framing carefully: a conversion does not *change* the original object. The `int` `3` is not mutated into a `double`. A brand-new `double` value, `3.0`, is created from it, and *that* is what gets used. The original is untouched.

### Implicit conversion

A conversion is **implicit** when you didn't ask for it — the compiler performs it automatically because the surrounding context demands a particular type and you supplied a different one. You write no special syntax; the language just does the work.

```cpp
void printDouble(double value);

int n { 5 };
printDouble(n); // n is an int, but printDouble wants a double
                // -> the int 5 is implicitly converted to the double 5.0
```

These automatic conversions are happening constantly, in a handful of predictable places:

- **initialization** — `double d { 3 };`
- **assignment** — `d = someInt;`
- **function arguments** — binding `n` to a `double` parameter, above
- **return statements** — `return someInt;` from a function returning `double`
- **mixed arithmetic** — `someInt + someDouble`
- **boolean conditions** — `if (someInt)` converts the `int` to `bool`

You will not write `convert(...)` at any of these spots. The conversion is woven into the language itself.

### The standard conversions, in buckets

C++ defines a large catalogue of built-in conversion rules — collectively, the *standard conversions*. You don't need to memorize the catalogue, but it helps to know roughly what lives in it:

| Conversion kind | Example |
|---|---|
| Numeric promotion | `short` → `int`, `float` → `double` |
| Numeric conversion | `double` → `int`, `int` → `double`, signed → unsigned |
| Boolean conversion | `0` → `false`, nonzero → `true` |
| Qualification conversion | adding `const` |
| Pointer conversion | derived pointer → base pointer, null pointer conversions |

This chapter zooms in on the numeric rows — promotions and numeric conversions — because those are where the everyday surprises live. The pointer and qualification rows belong to later chapters on references, pointers, and inheritance.

### A conversion can compile and still be wrong

Here is the uncomfortable truth that motivates the rest of the chapter. The compiler will happily perform a conversion that destroys information:

```cpp
int x { static_cast<int>(3.9) }; // x becomes 3 — the .9 is simply gone
```

The conversion *succeeds*. The code runs. But `3` is almost certainly not what a human expecting `3.9` had in mind. The compiler can transform the value; it cannot read your intent. That gap — between "the conversion is legal" and "the conversion is what I meant" — is exactly why C++ gives you warnings, why brace initialization rejects the dangerous cases, and why `static_cast` exists so you can say "yes, I really do mean to lose that data."

> **Key insight:** A conversion compiling successfully tells you the language *permits* it, not that it *preserves your value*. Always ask the second question yourself.

> **Note (CS6340 tie-in):** Real APIs are picky about types. LLVM functions traffic in `unsigned`, `size_t`, `Value*`, `Instruction*`, and references, and they convert between them constantly. The single most useful habit you can build now is to look at any expression and ask, "what type does this actually become?" — because the answer drives both correctness and the warnings your compiler hands you.

---

## 10.2 — Floating-point and integral promotion

### The safe subset: promotion

Of all the conversions C++ performs, one family is special because it is essentially *free of risk*: **numeric promotion**. A promotion converts a smaller or less-preferred arithmetic type into a larger, "preferred" type that the language likes to compute with — and it does so without ever losing value, because the bigger type can represent everything the smaller one could.

```cpp
short s { 3 };
int i { s };      // short widened to int — every short value fits in an int

float f { 1.5f };
double d { f };   // float widened to double — value preserved
```

Because every source value maps cleanly onto a destination value, promotions are value-preserving. That's the whole reason they get their own name and their own special status in the conversion rules.

### Why the language bothers: keeping the operator set small

Promotions aren't just a courtesy — they solve a real design problem. Imagine if arithmetic operators had to be defined separately for every combination of tiny types:

```
char  + char
char  + short
short + short
bool  + char
unsigned char + short
... and on, and on
```

That's a combinatorial explosion. Instead, C++ *promotes* the small types up to `int` (or `unsigned int`) first, then performs the operation using the ordinary `int` machinery. One set of integer operations covers all the little types. The promotion rules are the glue that makes that economy work.

### Integral promotions

The integral promotions take the small integer-ish types — `bool`, `char`, `signed char`, `unsigned char`, `short`, and certain small enums — and convert them to `int`, provided `int` can represent all their values. (If it can't, they promote to `unsigned int` instead.)

This has a consequence that ambushes nearly every beginner:

```cpp
char a { 10 };
char b { 20 };

auto c { a + b }; // c is an int, NOT a char!
```

Both operands are `char`, yet the *result* is `int`. The two `char`s were promoted to `int` before the `+` happened, so the addition produced an `int`. Arithmetic on small integral types routinely hands you back an `int` — a fact worth tattooing somewhere, because it explains a lot of otherwise baffling type behavior.

> **Tip:** When you do arithmetic on `char` or `short` values and want the result to stay that small type, you'll need to convert it back explicitly — for example with `static_cast<char>(...)`. The lab leans on exactly this when it asks you to keep a `char` a `char`.

### Floating-point promotion

There is one floating-point promotion: `float` widens to `double`.

```cpp
float f { 0.1f };
double d { f };  // float promoted to double
```

A small but important point of vocabulary: *not every widening is a promotion.* Going from `float` to `double` is a promotion; going from `int` to `double`, or `float` to `long double`, is a numeric *conversion* (next section), even though both widen. Promotion is a specific, privileged subset — the conversions the standard singles out as the safe, preferred-for-computation ones. Keep the word "promotion" reserved for that subset; everything else is a "conversion."

---

## 10.3 — Numeric conversions

### What counts as a numeric conversion

A **numeric conversion** is the conversion between arithmetic types that *isn't* a promotion. If a promotion is the safe, value-preserving widening, numeric conversions are everything else in the arithmetic-to-arithmetic space — including all the conversions that can lose data.

```cpp
double d { 5.0 };
int i { static_cast<int>(d) }; // double -> int

int n { 5 };
double x { n };                // int -> double
```

### Safe enough, versus genuinely lossy

Some numeric conversions happen to preserve the value because the particular number involved fits cleanly on both sides:

```cpp
int n { 5 };
double d { n }; // 5 is exactly representable as a double — no loss
```

Others can quietly mangle the value:

```cpp
int a { static_cast<int>(3.7) };                  // 3 — fractional part discarded
unsigned int b { static_cast<unsigned int>(-1) }; // wraps to a huge value
short c { static_cast<short>(100000) };           // may overflow: 100000 doesn't fit a 16-bit short
```

The danger isn't the conversion category as such — it's whether *this particular value* survives the trip. `int`→`double` is usually fine for ordinary integers but can lose precision for enormous ones; `double`→`int` always discards the fraction.

### The data-loss categories

It pays to internalize where the losses come from:

| Conversion | What can go wrong |
|---|---|
| Floating → integral | the fractional part is discarded (truncation toward zero) |
| Wider integer → narrower integer | the value may not fit, and gets wrapped/truncated |
| Signed → unsigned | negative values wrap around to large positives |
| Floating → narrower floating | precision and/or range is lost |
| Integer → floating | very large integers may lose precision |

```
double 3.9
   |
   |  convert to int  (truncates toward zero)
   v
int 3        the fractional .9 is gone
```

### The signed-to-unsigned trap

This one deserves special attention because it bites people in real code, not just in textbooks.

```cpp
int signedValue { -1 };
unsigned int unsignedValue { static_cast<unsigned int>(signedValue) };
```

`unsignedValue` is **not** `-1` — an `unsigned int` can't represent negatives at all. Instead the bit pattern is reinterpreted under unsigned rules, and `-1` becomes the *largest* value the type can hold (commonly `4294967295` for a 32-bit `unsigned int`). The minus sign didn't survive; it turned into an enormous positive number.

Why care? Because the standard library hands you unsigned types constantly:

```cpp
std::string s { "abc" };
auto len { s.length() }; // an unsigned size type, not int
```

The moment you mix a signed loop counter or a signed subtraction with one of these unsigned lengths, you risk a value flipping into the billions. The lab's `safeLength` task is built around exactly this hazard: `.length()` returns unsigned, and subtracting two of them — `safeLength("ab") - safeLength("abcd")` — should give `-2`, but would *wrap* to a giant positive number if you left it unsigned. The cure is to convert deliberately, on purpose, where you can see it.

> **Warning:** Mixing signed and unsigned values is one of the most reliable sources of subtle bugs in C++. When a `size_t` length meets signed arithmetic, decide the types on purpose — usually by converting the length to a signed `int` with a `static_cast` you can point at — rather than letting the implicit rules decide for you.

---

## 10.4 — Narrowing conversions, list initialization, and constexpr initializers

### Naming the danger: narrowing

A **narrowing conversion** is a numeric conversion that *may* lose data or change the value — the lossy ones from the table you just saw, gathered under a single label. "Narrowing" is exactly the right image: you're pouring a value into a container that may be too small or the wrong shape to hold it faithfully.

```cpp
int a = 3.5;     // double -> int: the .5 is lost (narrowing)
int b = 100000L; // long -> int: may not fit, depending on platform (narrowing)
```

Both of these compile with ordinary `=` (copy) initialization. The compiler might warn, but it won't stop you. And that's the problem: an accidental narrowing looks identical to an intentional one.

### Brace initialization slams the door

Here is one of the strongest practical arguments for the brace-initialization habit this course has pushed since the start. **List (brace) initialization refuses to perform a narrowing conversion** — it's a hard compile error, not a warning you might miss:

```cpp
int x { 3.5 }; // COMPILE ERROR: narrowing conversion from double to int
```

The braces turn a silent, lossy accident into a loud failure you cannot ignore. That is a feature. With braces, the only way data gets lost is if you *say* so.

```cpp
int x { 3 };     // fine — no narrowing
double d { 3 };  // fine — int -> double is a promotion-like widening, value preserved
```

> **Best practice:** Prefer brace initialization. Beyond the consistency reasons from earlier chapters, it catches narrowing conversions at compile time — turning a whole category of silent data-loss bugs into errors you can't ship by accident.

### Intentional narrowing: say it out loud

Sometimes you genuinely *do* want to narrow — you have a `double` and you truly want the integer part. The right move is not to fall back to `=` and hope; it's to state your intent with `static_cast`:

```cpp
double raw { 3.8 };
int truncated { static_cast<int>(raw) }; // 3 — and the cast says "I meant to do this"
```

That `static_cast` communicates two things at once to anyone reading the code:

```
I know this loses the fractional data.
This is deliberate, not an accident.
```

It also satisfies the brace initializer — because once you've cast `raw` to `int`, the value going into the braces is *already* an `int`, so there's no narrowing left to reject.

### The constexpr exception

There's a sensible loophole. If the value being converted is a **constant expression** and the compiler can *prove* it fits in the destination type, brace initialization allows the conversion — there's no risk, because the compiler has already checked the specific value:

```cpp
constexpr int small { 5 };
char c { small }; // OK: the compiler knows 5 fits in a char on this platform
```

But the proof has to be possible at compile time. If the value isn't known until runtime, the compiler can't guarantee anything, and the brace initializer goes back to rejecting the narrowing:

```cpp
int runtime { 5 };
// char c { runtime }; // ERROR: narrowing — the compiler can't prove 'runtime' fits
```

The distinction is the same one from Chapter 5: a `constexpr` value is knowable to the compiler, so it can vouch for it; a runtime value is not.

> **Note (CS6340 tie-in):** When you convert indexes, sizes, opcodes, line numbers, and columns between types — which you'll do constantly against LLVM APIs — run through four questions: (1) Could the destination type be *smaller* than the source? (2) Could the *signedness* change? (3) Is this conversion intentional? (4) Should a `static_cast` document it? If the answer to the last one is yes, write the cast.

---

## 10.5 — Arithmetic conversions

### Operators want matching operands

Most binary arithmetic operators — `+`, `-`, `*`, `/`, `%`, the comparisons — can only operate on two values of the *same* type. The CPU adds two `double`s, or two `int`s; it has no single instruction for "add an `int` to a `double`." So before such an operator runs on mismatched operands, C++ first converts one of them so both share a common type. *Then* the operation proceeds.

```cpp
int i { 2 };
double d { 3.5 };

auto result { i + d }; // i is converted to double; result is a double (5.5)
```

The mental model is a two-step dance:

```
int + double
  -> convert the int (2) to double (2.0)
  -> double + double  =>  2.0 + 3.5
  -> double result (5.5)
```

This is the rule that the lab's `mean` and `percentage` tasks hinge on. If *both* operands are `int`, the common type is already `int`, the division happens in integer arithmetic, and `7 / 2` truncates to `3` before anything else gets a chance. Convert *one* operand to `double` first, and the arithmetic conversions promote the other to `double` too — so the whole division is real-valued, and you get `3.5`. The cast has to come *before* the divide; casting the result afterward is too late, because the truncation already happened.

### The usual arithmetic conversions

C++ chooses the common type using a ranked set of rules called the *usual arithmetic conversions*. The full rule list is detailed, but the working intuition covers almost everything you'll meet:

- If any operand is a **floating-point** type, the operands convert to the highest-ranked floating type present (so an `int` with a `double` gives `double`; a `float` with a `double` gives `double`).
- Otherwise, the **integral promotions** apply first, lifting small types to `int`.
- Then **signedness and rank** rules pick the common integer type among what's left.

The first bullet is precisely why the lab's `weightedMean` needs *no cast at all*: each `int * double` product converts the `int` to `double` automatically, so the numerator and denominator are already `double` by the time you divide. Adding a cast there would be a lie — it would imply a narrowing that simply isn't happening.

### When signedness flips your intuition

The integer half of these rules hides the same signed/unsigned trap from before, now triggered *automatically* by an operator:

```cpp
int x { -1 };
unsigned int y { 1 };

if (x < y)
{
    std::cout << "is -1 less than 1?\n"; // ...you'd think so
}
```

Read it innocently and `-1 < 1` is obviously true. But the usual arithmetic conversions, faced with one signed and one unsigned operand of the same rank, convert the *signed* one to unsigned. So `x` becomes a huge positive value before the comparison, and `-1 < 1` may evaluate to **false**.

```
signed -1
  -> converted to unsigned (rank rules)
  -> a huge positive value
  -> comparison "x < y" flips against intuition
```

Your defenses:

- avoid mixing signed and unsigned operands in the first place;
- take compiler warnings about signed/unsigned comparisons seriously — don't silence them blindly;
- use an explicit `static_cast` only when you *know* the value's range is safe;
- prefer signed counters for ordinary counting, unless an API forces unsigned on you.

> **Warning:** A comparison between a signed and an unsigned value does not mean what it reads like. The signed operand is converted to unsigned first, so negatives become enormous positives. Don't reach for a cast to quiet the warning — fix which types you're comparing.

### A glimpse of `std::common_type`

If you ever want to *ask* the type system which common type two types would produce, the standard library can tell you:

```cpp
#include <type_traits>

using Common = std::common_type_t<int, double>; // Common is double
```

You won't need this for the lab, but it's useful vocabulary — and a reminder that "what type does this become?" is a question the language itself can answer.

---

## 10.6 — Explicit type conversion (casting) and static_cast

### Asking for a conversion on purpose

An **explicit conversion** is one you request directly in the source code, rather than letting it happen implicitly. The everyday tool for this is `static_cast`:

```cpp
double d { 3.7 };
int i { static_cast<int>(d) }; // 3 — explicitly requested
```

The whole point of writing the conversion out is that it's *visible*. A reader (including future-you) can see exactly where a value changes type, and the brace initializer accepts it because you've already done the narrowing yourself.

### Avoid C-style casts

C inherited a terser cast syntax that C++ still accepts:

```cpp
int i { (int)d }; // C-style cast — avoid this in C++
```

Steer clear of it. A C-style cast is a kind of chameleon: depending on the types involved it can silently behave as any of several different C++ casts, some of them far more dangerous than a `static_cast`. That ambiguity makes it harder to read, harder to audit, and easier to misuse. The lab forbids C-style casts for exactly this reason.

> **Best practice:** For ordinary value conversions, use `static_cast<T>(expr)`. Avoid C-style casts like `(int)x` — they're imprecise about *which* kind of conversion they perform, which makes them harder to reason about and review.

### Why the syntax is deliberately loud

`static_cast` is verbose on purpose. A conversion that loses data should be *conspicuous*, not tucked away:

```
static_cast<int>(d)
            ^^^      target type — what you're converting TO
                ^    source expression — what you're converting FROM
```

The clunkiness is a feature. It makes casts stand out when you skim the code, so each one invites the question "is this loss intended?"

### Using `static_cast` to document a narrowing

A `static_cast` isn't only a mechanism — it's documentation. It records a decision:

```cpp
std::string s { "abc" };
int length { static_cast<int>(s.length()) };
```

This says, in effect: "I know `.length()` returns an unsigned size type. I am intentionally treating it as an `int` here, and I'm confident the value is small enough to fit." That's a reasonable promise for short strings — and it's exactly the move the lab's `safeLength` task asks for: convert the unsigned length to a signed `int` so that later subtraction doesn't wrap.

### A cast is not the same as constructing a temporary

These two lines look alike but do different jobs:

```cpp
int a { static_cast<int>(3.7) }; // a is 3 — a conversion that narrows
int b { int { 3.7 } };           // COMPILE ERROR: list-init blocks the narrowing
```

`static_cast` performs the conversion. The `int{ 3.7 }` form tries to *construct* an `int` using list initialization — which refuses the narrowing and won't compile. Use `static_cast` when conversion is the point; use direct/list initialization when you mean to construct an object with safe, narrowing-checked initialization.

### A cast is a promise — make it true

Two CS6340-flavored examples of the judgment in action:

```cpp
auto line { debugLoc.getLine() };          // some unsigned type from the API
int lineForRuntime { static_cast<int>(line) }; // promise: this line number fits in int
```

```cpp
char replacement {
    static_cast<char>('a' + randomOffset)  // arithmetic promoted to int; cast back to char
};
```

The temptation, when a warning appears, is to bury it under a cast and move on. Resist it.

> **Key insight:** A cast is a promise to the compiler — "trust me, this conversion is fine." Don't scatter casts just to silence warnings; only write one when you understand *why* the value is safe to convert. A cast you can't justify is a bug waiting in disguise.

---

## 10.7 — Typedefs and type aliases

### Giving a type a second name

A **type alias** introduces a new *name* for an existing type. It doesn't create a new type — it creates a synonym. The modern syntax uses the `using` keyword:

```cpp
using StudentId = int;
using ErrorCode = int;
```

Now you can declare variables with the alias, and they behave exactly as the underlying type would:

```cpp
StudentId id { 42 }; // id is just an int, wearing a more descriptive label
```

Read the declaration left to right: *using* `StudentId` *to mean* `int`. That readability is the entire selling point.

### An alias is not a distinct type

This is the crucial caveat, and the one the lab's stretch goal pokes at. An alias and its underlying type are *the same type* — so the compiler treats them as freely interchangeable:

```cpp
using StudentId = int;
using CourseId  = int;

StudentId s { 1 };
CourseId  c { s }; // perfectly legal: both are just int, so this is int -> int
```

You might *wish* this assignment were an error — mixing a student id and a course id sounds like a bug. But aliases buy you **readability, not type safety**. `StudentId` and `CourseId` are two names for one type, and the compiler sees through both to the `int` underneath. (To get genuine, enforced distinctness, you'd wrap the value in a `struct` or `enum class` — tools from later chapters.)

> **Key insight:** A type alias renames a type; it does not create a new one. The compiler enforces nothing extra. Aliases make code *read* better — they don't make it *safer*.

### Aliases obey scope

An alias is a declaration like any other, so it lives within the scope where you declare it:

```cpp
void f()
{
    using Count = int;
    Count x { 0 };
}

// Count is not visible out here
```

Declare an alias at namespace scope for project-wide use, or inside a function when it's a local convenience.

### `typedef`: the older spelling

Before `using`, C and C++ spelled aliases with `typedef`:

```cpp
typedef int StudentId; // older syntax — note the reversed order
```

This still works, but prefer `using` in modern C++:

```cpp
using StudentId = int; // reads left-to-right, and plays better with templates
```

The `using` form reads in the natural order (*name = type*, like an assignment) and, unlike `typedef`, extends cleanly to template aliases. There's no reason to reach for `typedef` in new code.

> **Best practice:** Use `using` for type aliases, not `typedef`. It reads more naturally and is strictly more capable.

### Where aliases earn their keep

A few situations where an alias genuinely improves the code:

**1. Naming a type by its role:**

```cpp
using Distance = double; // "Distance" tells the reader what the number means
```

**2. Taming a long, ugly type:**

```cpp
using SeedPool      = std::vector<std::string>;
using CampaignSeeds = std::map<Campaign, SeedPool>;
```

**3. Documentation through naming:**

```cpp
using LineNumber   = int;
using ColumnNumber = int;
```

**4. Single-point maintenance:** if a quantity's type might change, an alias localizes the edit:

```cpp
using Score = double; // change to float in one place, everywhere follows
```

The lab uses this idea directly: `Score`, `Percent`, and `Average` are all aliases for `double`. They make the function signatures read like the domain ("this returns an `Average`") while leaving the underlying arithmetic unchanged.

### The cost of overdoing it

Aliases have a downside when overused: a wall of aliases can *hide* the real type and force readers to keep a translation table in their heads. Reach for an alias when it clarifies intent or collapses an unwieldy type — not to rename every `int` you ever declare.

> **Note (CS6340 tie-in):** Coverage bookkeeping in the project gets far more readable with a couple of aliases:
> ```cpp
> using CoveragePoint = std::pair<int, int>; // (line, column)
> using CoverageSet   = std::set<CoveragePoint>;
> ```
> A function returning a `CoverageSet` says what it *means*; one returning `std::set<std::pair<int, int>>` makes you decode it.

---

## 10.8 — Type deduction for objects using the auto keyword

### Letting the compiler name the type

When you initialize a variable, the type of the initializer already tells the compiler what type the variable should be. The **`auto`** keyword lets you take advantage of that: instead of writing the type, you write `auto`, and the compiler *deduces* it from the initializer.

```cpp
auto x { 5 };        // int      — deduced from the int literal 5
auto y { 3.14 };     // double   — deduced from the double literal
auto name { "Ada" }; // const char*  — NOT std::string (more on this below)
```

Because deduction needs *something to deduce from*, an `auto` variable must be initialized:

```cpp
auto x; // ERROR: no initializer, nothing to deduce the type from
```

### `auto` drops top-level const

A subtlety that matters: when `auto` deduces a type, it strips off any top-level `const` from the initializer. The deduced variable is a fresh copy, so the source's constness doesn't carry over by default:

```cpp
const int a { 5 };
auto b { a };       // b is int, not const int — the const was dropped
```

If you want the deduced variable to be `const`, add it back yourself:

```cpp
const auto b { a }; // now b is const int
```

The same goes for combining `auto` with `constexpr`:

```cpp
constexpr auto maxTests { 10000 }; // deduces int; the constexpr is yours to add
```

### The string-literal gotcha

This is the `auto` surprise that catches everyone, so let's be explicit about it:

```cpp
auto s { "hello" };
```

`s` is **not** a `std::string`. A string literal like `"hello"` is a C-style string, and `auto` deduces it to a pointer-to-`const`-`char` (`const char*`). If you wanted the rich, owning `std::string` type — with `.length()`, concatenation, and all the rest — you have to ask for it by name:

```cpp
std::string s { "hello" };      // an owning std::string
std::string_view sv { "hello" }; // a lightweight read-only view
```

> **Warning:** `auto` on a string literal gives you a `const char*`, not a `std::string`. When you want a real string object, write the type explicitly — don't let `auto` quietly hand you a raw pointer.

### When `auto` helps

`auto` is at its best when the type is either obvious or annoyingly verbose:

- the type is plain from the initializer (`auto count { 0 };`);
- the real type is long or noisy (iterators, template-heavy library types);
- the exact type could change later without affecting the surrounding logic.

```cpp
auto it { seeds.begin() }; // the iterator type is a mouthful; auto spares you spelling it
```

This is why the lab uses `auto` for the scale-factor local in `roundTo`: its initializer is a `double` from a lookup table, so the type is unmistakable, and `auto` keeps the line clean.

LLVM-style code often pairs `auto` with `*` to keep the pointer-ness visible even while deducing what's pointed to:

```cpp
auto* M { F.getParent() }; // deduce the pointee type, but make clear M is a pointer
```

That `auto*` reads as: *deduce the pointed-to type, but I want it obvious this is a pointer.*

### When `auto` hurts

The flip side: `auto` can *hide* a type that the reader actually needs to see.

```cpp
auto x { getValue() }; // ...what on earth is x?
```

If the type is part of what makes the code understandable, spell it out. This is doubly true while you're *learning* a concept — when the type is the lesson, naming it reinforces the lesson.

> **Best practice:** Use an explicit type when the type is part of the point you're making (or learning). Reach for `auto` when the initializer makes the type obvious, or when the real type is long and noisy enough that spelling it adds clutter rather than clarity.

---

## 10.9 — Type deduction for functions

### Deducing a function's return type

Just as `auto` can deduce the type of a variable from its initializer, it can deduce a function's **return type** from its `return` statements:

```cpp
auto add(int a, int b)
{
    return a + b; // both int -> the function's return type is deduced as int
}
```

The compiler looks at what you return and infers the return type. (If a function has several `return` statements, they must all yield the *same* type, or the deduction is ambiguous and won't compile.)

### Where it helps

Return-type deduction shines when the return type is long and would just be noise to repeat:

```cpp
auto makeSeeds()
{
    return std::vector<std::string>{ "a", "b", "c" }; // return type deduced
}
```

The type is right there in the `return`; writing it twice adds nothing.

### Where it hurts

But a function *declaration* is a contract — it's the first thing a caller reads, often the *only* thing, when the definition lives in another file. Hiding the return type behind `auto` robs the reader of that information:

```cpp
auto readCoverage(); // ...returns what, exactly?
```

Far clearer to state the contract:

```cpp
std::set<std::pair<int, int>> readCoverage();
```

Or, combining this lesson with the last, lean on an alias to get *both* an explicit return type and readability:

```cpp
using CoveragePoint = std::pair<int, int>;
using CoverageSet   = std::set<CoveragePoint>;

CoverageSet readCoverage(); // explicit AND readable
```

> **Best practice:** For functions whose definition is visible right there (and especially short helpers), return-type `auto` can reduce clutter. For functions that form a public API — declared in a header, called from elsewhere — prefer an explicit return type so callers can see the contract without hunting for the definition.

### Trailing return type

C++ offers an alternative spelling, the *trailing return type*, where `auto` sits in the usual spot and the real type follows an arrow:

```cpp
auto add(int a, int b) -> int
{
    return a + b;
}
```

For a plain function this is just a stylistic variant. Its real value comes in certain template situations, where the return type *depends on* the parameter types and can only be expressed after the parameters are named. You'll see this form in modern and library code; recognize it now so it doesn't surprise you later.

### A note on parameters

Type deduction does **not** extend to ordinary function parameters in the simple way it does to variables and return types. Plain `auto` parameters are actually a shorthand for *function templates* — a topic for the next chapter. For now, give parameters explicit types:

```cpp
void mutate(std::string input); // clear and unambiguous — write the type
```

---

## 10.x — Chapter 10 summary and quiz

### Core takeaways

- A **type conversion** produces a value of one type from a value of another; it doesn't mutate the original.
- **Implicit conversions** happen automatically — at initialization, assignment, argument passing, return, mixed arithmetic, and boolean contexts.
- **Promotions** are the safe, value-preserving widenings to preferred computation types (`short`/`char`→`int`, `float`→`double`).
- **Numeric conversions** are everything else between arithmetic types, and many of them lose data.
- **Narrowing conversions** are the lossy ones; **brace initialization rejects them at compile time**, which is a major reason to prefer braces.
- Use **`static_cast`** for intentional, visible conversions; avoid C-style casts.
- In **mixed arithmetic**, operands are converted to a common type first — which is why `int / int` truncates and converting one operand to `double` fixes it.
- **Signed/unsigned** mixing inverts your intuition: the signed operand becomes a huge positive value.
- **Type aliases** (`using`) improve readability but create no new distinct type and add no type safety.
- **`auto`** deduces object types from initializers — but drops top-level `const` and turns string literals into `const char*`.
- **Return-type deduction** is handy for noisy types, but public APIs benefit from explicit return types.

### Lab-oriented decision table

| Situation | Good move |
|---|---|
| Make a real average out of `total / count` | `static_cast<double>(total) / count` — cast *before* the divide |
| Convert a `size_t` length to `int` for a short, controlled string | `static_cast<int>(s.length())`, after checking the range is small |
| Round a `double` to the nearest `int`, half away from zero | nudge by `0.5` toward the value's sign, then `static_cast<int>` |
| Readable coverage storage type | `using CoverageSet = std::set<std::pair<int, int>>;` |
| API returns a pointer with a noisy type | `auto* M { F.getParent() };` |
| `auto` on a string literal | be careful — it's `const char*`, not `std::string` |
| `int * double` already gives `double` | add **no** cast — one would falsely imply a narrowing |
| Warning about signed/unsigned comparison | don't silence it; align the types deliberately |

### Setting up the lab

The chapter's lab is **`statkit`**, the number-crunching core of a grade book: a handful of pure functions — `mean`, `percentage`, `roundToInt`, `roundTo`, `clampScore`, `letterGrade`, `safeLength`, `weightedMean`. Tiny as they are, almost every one is a trap for the lesson this chapter exists to teach: *a conversion the compiler performs silently is not always the conversion you meant.*

The headline bug is **integer-division truncation**. You know the mean of a 7-point total over 2 students is `3.5` — so when `mean(7, 2)` returns `3`, the error isn't abstract, it's arithmetic you can see is wrong. The fix is never "add a cast somewhere"; it's placing a `static_cast` at *exactly the right spot* so the division happens in `double`, not `int`. The other functions drill the rest of the chapter: documenting an intentional `float`→`int` narrowing (`roundToInt`), taming the unsigned `size_t` from `.length()` (`safeLength`), leaning on the usual arithmetic conversions when an `int` meets a `double` so that *no cast is needed* (`weightedMean`), and using `using` aliases (`Score`, `Percent`, `Average`) and `auto` to keep the code readable.

The starter compiles immediately but every function returns a placeholder, and several deliberately reproduce the classic bug — so `make test` starts **RED** and the failures point straight at the conversion you must fix. A few idioms the lab insists on, all straight from this chapter:

- **Cast *before* the divide** in `mean` and `percentage` — casting the result is too late.
- **`static_cast` to document** the `float`→`int` narrowing in `roundToInt` and the unsigned→signed conversion in `safeLength`.
- **`auto`** for the scale-factor local in `roundTo`, where the type is obvious from its initializer.
- **No cast** in `weightedMean` — adding one would lie about a narrowing that isn't there.

Doubles are compared with an epsilon helper, never `==`, exactly as the earlier chapters insisted. Turning that wall of red into one green line *is* the exercise — and every green check is a conversion you got to mean on purpose.

### Mini drill

A small program that exercises several of the chapter's ideas at once:

```cpp
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

using MutantList = std::vector<std::string>; // a readability alias

int main()
{
    MutantList mutants { "abc", "abcd", "abcde" };

    for (const auto& mutant : mutants) // const auto& avoids copying each string
    {
        auto length { mutant.length() }; // unsigned size type, deduced

        if (length > 4)
        {
            int lengthForLog { static_cast<int>(length) }; // documented conversion
            std::string_view label { "long mutant" };       // read-only label

            std::cout << label << ": " << lengthForLog << '\n';
        }
    }
}
```

What it reinforces:

- `using` creates a readability alias (`MutantList`).
- `const auto&` deduces the element type and binds by reference, so no string is copied.
- `.length()` does **not** return an `int` — it's an unsigned size type.
- `static_cast<int>` documents the intentional conversion to a signed `int`.
- `std::string_view` is the right choice for a read-only label.
