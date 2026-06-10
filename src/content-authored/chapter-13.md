# Chapter 13 — Compound Types: Enums and Structs

> Source: <https://www.learncpp.com/> (Chapter 13)

Up to now, the types you've reached for have been ones C++ handed you: `int`, `double`, `bool`, `char`, and the standard-library types built on top of them like `std::string`. They're the universal building blocks, and you can express a great deal with them. But notice what happens when you try to model something from a real problem. A point on a screen is *two* integers that belong together. A traffic light is *one* of exactly three states. A catalogue item is a name, a tier, a quantity, and a price — four values that are meaningless apart and meaningful together. The fundamental types can hold those values, but they can't say that the values belong together, or that a quantity is one of a fixed set of choices. The meaning lives only in your head and in your variable names.

This chapter is where you start building your own vocabulary. A **program-defined type** is a type you define in your own code, shaped to your problem instead of to the machine. We'll cover the two workhorses. **Enumerations** name a small, closed set of choices — `Common`, `Rare`, `Epic` — so that a "rarity" is a *rarity* in the type system and not a nameless integer. **Structs** bundle related values under one name, so that a point is a *point* and not two loose `int`s you have to remember to keep together. Along the way you'll learn how to print these types, pass them around efficiently, give them safe defaults, and even template them so one definition serves many element types. By the end you'll be designing types the way a working C++ programmer does: letting the type system carry the meaning, so the compiler can catch the mistakes you'd otherwise have to catch by hand.

## Contents
- [13.1 — Introduction to program-defined (user-defined) types](#131--introduction-to-program-defined-user-defined-types)
- [13.2 — Unscoped enumerations](#132--unscoped-enumerations)
- [13.3 — Unscoped enumerator integral conversions](#133--unscoped-enumerator-integral-conversions)
- [13.4 — Converting an enumeration to and from a string](#134--converting-an-enumeration-to-and-from-a-string)
- [13.5 — Introduction to overloading the I/O operators](#135--introduction-to-overloading-the-io-operators)
- [13.6 — Scoped enumerations (enum classes)](#136--scoped-enumerations-enum-classes)
- [13.7 — Introduction to structs, members, and member selection](#137--introduction-to-structs-members-and-member-selection)
- [13.8 — Struct aggregate initialization](#138--struct-aggregate-initialization)
- [13.9 — Default member initialization](#139--default-member-initialization)
- [13.10 — Passing and returning structs](#1310--passing-and-returning-structs)
- [13.11 — Struct miscellany](#1311--struct-miscellany)
- [13.12 — Member selection with pointers and references](#1312--member-selection-with-pointers-and-references)
- [13.13 — Class templates](#1313--class-templates)
- [13.14 — Class template argument deduction (CTAD) and deduction guides](#1314--class-template-argument-deduction-ctad-and-deduction-guides)
- [13.15 — Alias templates](#1315--alias-templates)
- [13.x — Chapter 13 summary and quiz](#13x--chapter-13-summary-and-quiz)
- [13.y — Using a language reference](#13y--using-a-language-reference)

---

## 13.1 — Introduction to program-defined (user-defined) types

### Why the built-in types aren't enough

C++ ships with a respectable set of fundamental types — `int`, `double`, `bool`, `char`, and so on — plus standard-library types like `std::string`. For a surprising amount of code, that's all you need. But the moment your program starts modeling a *domain* — a game shop, a compiler, a graphics scene — you run into concepts that no built-in type names directly.

Consider a couple of them:

```cpp
int line { 27 };
int column { 13 };
```

Those two `int`s describe a position in a source file, but nothing in the code *says* so. To the compiler they're just two numbers. You could accidentally pass them in the wrong order, mix one of them up with an unrelated count, or store them in separate containers that drift out of sync — and the compiler would never object, because as far as it knows, an `int` is an `int`.

A **program-defined type** (also called a *user-defined type*) is a type you define yourself, tailored to your problem. C++ gives you two main tools for this in this chapter:

```cpp
enum class Rarity { Common, Rare, Epic };

struct CoveragePoint
{
    int line {};
    int column {};
};
```

`Rarity` says, in the type system, "this value is one of exactly three tiers." `CoveragePoint` says "these two integers are a line/column pair, and they travel together." The payoff is that meaning you used to carry in your head now lives in the type — and the compiler can check it for you.

```text
int, int        ->  two generic numbers, meaning lives in your head
CoveragePoint   ->  a line/column pair, meaning lives in the type
Rarity          ->  one of a known set of tiers, meaning lives in the type
```

### Defining a type

A type definition introduces a new name and describes what objects of that type look like. It usually appears before any code that uses it:

```cpp
struct Student
{
    std::string name {};
    int id {};
};
```

The definition tells the compiler three things: that the type `Student` exists, what members it has (`name` and `id`), and how an object of that type is laid out and interpreted in memory. Once the compiler has seen this, you can declare `Student` objects, pass them to functions, and return them — just like any built-in type.

### Where type definitions live in multi-file programs

A type used in only one `.cpp` file can be defined right there. But most useful types get used across several files, and there the right home is a **header file**:

```cpp
// CoveragePoint.h
#ifndef COVERAGE_POINT_H
#define COVERAGE_POINT_H

struct CoveragePoint
{
    int line {};
    int column {};
};

#endif
```

Every `.cpp` file that needs `CoveragePoint` includes this header, and they all get the identical definition. You might worry this violates the One Definition Rule — after all, the definition now appears in many translation units. It doesn't: type definitions are specifically *exempt* from the part of the ODR that forbids multiple definitions, **provided every copy is identical** (which including the same header guarantees). This exemption is exactly why putting types in headers is the standard practice.

> **Key insight:** A type definition describes a *shape*, not an object — no storage is set aside, no code is generated. That's why it's safe to repeat the identical definition in every file that needs it, and why headers are the natural home for your types.

### Naming convention

In the style we follow throughout this course, program-defined types get **capitalized** names. This visually separates your types from variables and from the lowercase fundamental types:

```cpp
struct CoveragePoint {};   // a type
enum class Campaign {};    // a type

CoveragePoint point {};    // a variable of that type
```

You'll see the same convention in large C++ codebases: type names like `Function`, `Module`, `Instruction`, and `DebugLoc` are all program-defined or library-defined types, capitalized for exactly this reason. The mechanism behind them is the same one you're learning now — your own `struct`s and `enum`s use the identical machinery.

---

## 13.2 — Unscoped enumerations

### A type whose values are a fixed list of names

Some quantities aren't really numbers at all — they're *choices* from a short, fixed menu. A color that's red, green, or blue. A traffic light that's red, yellow, or green. A campaign that's one of a handful of named strategies. An **enumeration** is a compound type built for exactly this: its set of possible values is a list of named alternatives that *you* spell out.

The classic form is the **unscoped enumeration**, introduced with the `enum` keyword:

```cpp
enum Color
{
    red,
    green,
    blue,
};
```

`Color` is the enumeration type. `red`, `green`, and `blue` are its **enumerators** — the named values an object of type `Color` is allowed to take. (The trailing comma after `blue` is optional and harmless; many people include it so adding a new enumerator later is a one-line change.)

### Why enums beat bare integers

Here's the problem an enum solves. Suppose a function takes a "campaign" selector encoded as an integer:

```cpp
int campaign { 1 };   // ...1 meaning what, exactly?
```

A reader has no idea what `1` means without hunting down documentation or a comment, and nothing stops you from passing `7` — a value that corresponds to no real campaign at all. Now the same idea with an enum:

```cpp
enum Campaign
{
    mutationA,
    mutationB,
    mutationC,
};

Campaign campaign { mutationA };
```

The code now *says what it means*. `mutationA` is self-documenting, and the type `Campaign` advertises to every reader and every function that this value is a campaign and nothing else. You've replaced a comment-and-hope convention with something the compiler understands.

### "Unscoped" means the names leak

The word *unscoped* is a warning label. The enumerators of an unscoped enum are placed into the **surrounding scope** — the same scope as the enum type itself. So after defining `Color`, the names `red`, `green`, and `blue` are floating around in that scope directly, not tucked inside `Color`. That leads to collisions:

```cpp
enum Color
{
    red,
    green,
    blue,
};

enum TrafficLight
{
    // red,   // error: 'red' is already a name in this scope (from Color)
    yellow,
    cautionGreen,
};
```

`TrafficLight` would love to have its own `red`, but `Color` already claimed that name in the shared scope. The historical workaround is to prefix every enumerator by hand:

```cpp
enum Color
{
    color_red,
    color_green,
    color_blue,
};
```

It works, but it's noisy, and it's a symptom that the tool is leaking names it shouldn't. The real fix arrives in lesson 13.6 with **scoped enumerations** (`enum class`), which keep their enumerators to themselves. For now, just recognize the leakage as the defining quirk — and limitation — of unscoped enums.

### An enumerated type is a distinct type

Internally, the compiler stores enumerators as integers (more on that next lesson). It's tempting to think of `Color` as "really just an int," but resist that — `Color` is its own **distinct type**, and treating it as one is the whole point:

```cpp
Color c { red };   // good: a Color holding a named Color value
```

Use the named enumerators, not raw integers, when you assign and compare. That's what keeps the meaning visible and lets the type system help you.

> **Best practice:** Reach for an enumeration whenever a value is one of a small, fixed set of named options. It documents intent and constrains the value far better than a bare `int` plus a comment.

---

## 13.3 — Unscoped enumerator integral conversions

### Every enumerator has an integer value

An unscoped enum is backed by integers, and each enumerator gets one. Unless you say otherwise, counting starts at `0` and increases by one down the list:

```cpp
enum Color
{
    red,    // 0
    green,  // 1
    blue,   // 2
};

std::cout << green << '\n';   // prints 1
```

That last line works because an unscoped enumerator **implicitly converts to its integer value**. Sometimes that's convenient — you can use an enumerator as an array index, say. But it's also a hole in the type safety: `std::cout << green` happily prints `1` instead of complaining that you never taught it how to print a `Color`. The value silently decays to a number, and the meaning you worked to capture evaporates. (Scoped enums in 13.6 close this hole.)

### Choosing the values yourself

You don't have to accept the automatic `0, 1, 2, ...`. You can assign specific integer values to enumerators, which is handy when the numbers mean something outside your program — error codes, protocol constants, hardware registers:

```cpp
enum ErrorCode
{
    ok = 0,
    fileNotFound = 10,
    permissionDenied = 20,
};
```

Any enumerator you *don't* assign simply continues incrementing from the previous one. So if you wrote `a = 5, b, c`, then `b` is `6` and `c` is `7`.

### Value initialization gives you the zero value

If you value-initialize an unscoped enum with empty braces, you get the enumerator whose value is `0` (if one exists):

```cpp
Color c {};   // underlying value 0, which is 'red' in this enum
```

This has a practical design consequence: the enumerator you list **first** (or explicitly set to `0`) becomes the default. So it pays to make the zero value a sensible default state rather than an arbitrary one:

```cpp
enum Status
{
    unknown,   // 0 — a good default: "we don't know yet"
    ready,
    failed,
};
```

> **Best practice:** When it's reasonable, arrange your enum so that the enumerator valued `0` is a meaningful default — often something like `unknown`, `none`, or `default`. A value-initialized enum will land on it.

### The underlying type

Behind every enum is an integral **underlying type** that determines how much storage it uses. The compiler picks a sensible default, but you can specify one explicitly with a `: type` suffix:

```cpp
#include <cstdint>

enum Color : std::uint8_t
{
    red,
    green,
    blue,
};
```

Here every `Color` occupies a single byte. Do this only when you have a concrete reason — controlling memory layout, matching a binary file format, or interoperating with other code. For everyday enums, let the compiler choose.

### Converting an integer *to* an enum

The reverse conversion — integer to enum — is the dangerous direction, because an arbitrary integer might not correspond to any named enumerator. C++ won't do it implicitly for you; you have to ask with a `static_cast`:

```cpp
Color c { static_cast<Color>(99) };   // compiles, but 99 names no real color
```

This is legal, and occasionally necessary (for example, when turning a number you parsed from external input back into an enum). But the result can be an enum object holding a value that matches none of your enumerators. So whenever you cast *into* an enum from data you don't control, treat the result as suspect and validate it — a habit we'll lean on in the next lesson.

> **Warning:** `static_cast<SomeEnum>(n)` does no checking. If `n` doesn't correspond to a named enumerator, you get an enum object in an out-of-range state. Always validate enum values that originate from external input.

---

## 13.4 — Converting an enumeration to and from a string

### The compiler doesn't know your enumerator names

You named your enumerators `MutationA`, `MutationB`, `MutationC` — but those names are a convenience for *you*, the programmer. By the time the program runs, they've been compiled down to integers; the spelling is gone. So when you try to print an enum, C++ has no idea you wanted the word "MutationA" rather than the number `0`. If you want that mapping, you write it yourself.

The standard tool is a small helper function with a `switch`:

```cpp
#include <string_view>

enum class Campaign
{
    MutationA,
    MutationB,
    MutationC,
};

std::string_view toString(Campaign campaign)
{
    switch (campaign)
    {
    case Campaign::MutationA: return "MutationA";
    case Campaign::MutationB: return "MutationB";
    case Campaign::MutationC: return "MutationC";
    }

    return "Unknown";
}
```

A few details worth noticing. The return type is `std::string_view`, not `std::string`: every value we return is a string *literal*, and literals live for the entire run of the program, so a non-owning view onto one is perfectly safe and copies no characters. The `switch` lists every enumerator, and the trailing `return "Unknown";` catches anything that slips through — including out-of-range values forged with a `static_cast`, which we just saw are possible. (This pattern is the heart of Task 1 in this chapter's lab, where you'll write a `rarityLabel` that maps `Rarity` values to their names.)

### Going the other way: string to enum

The inverse — turning text back into an enum — has an extra wrinkle: the text might not match any enumerator. You can't return a "real" `Campaign` in that case without lying, so the honest return type is `std::optional<Campaign>`, which can hold a campaign *or* signal "no match":

```cpp
#include <optional>
#include <string_view>

std::optional<Campaign> campaignFromString(std::string_view text)
{
    if (text == "MutationA") return Campaign::MutationA;
    if (text == "MutationB") return Campaign::MutationB;
    if (text == "MutationC") return Campaign::MutationC;

    return std::nullopt;   // no enumerator matched
}
```

Returning `std::nullopt` on failure is far better than inventing a default like `MutationA` and hoping the caller notices something went wrong. With `std::optional`, parse failure is *explicit* — the caller has to look inside the optional before using it, so a bad string can't silently masquerade as a valid campaign.

### Why this matters at program boundaries

Conversions like these live at the **edges** of your program, where structured data meets the messy outside world. A command-line argument arrives as a string:

```text
./fuzzer ./target fuzz_input fuzz_output MutationA
```

but your internal code wants a proper `Campaign`, not a string it has to re-check at every turn:

```text
"MutationA"  ->  Campaign::MutationA
```

Do the conversion once, right at the boundary, and from then on the rest of your program enjoys a clean, type-checked enum. That's the whole reason `toString`/`fromString` helpers earn their keep: they're the translators between the outside world's strings and your inside world's types.

---

## 13.5 — Introduction to overloading the I/O operators

### Teaching operators to work with your types

You've used `<<` and `>>` with streams since your very first program: `std::cout << x` and `std::cin >> y`. What you may not have realized is that `<<` and `>>` are just *functions* in disguise — and C++ lets you write new versions of them that work with your own types. This is **operator overloading**, and we'll cover it in full in a later chapter; here we take a first look, because it's the natural finish to the enum-printing story from 13.4.

The two operators that matter for input and output are:

- `operator<<` — the output (insertion) operator,
- `operator>>` — the input (extraction) operator.

### Making an enum printable

Right now, printing a `Campaign` means calling your helper explicitly every time:

```cpp
std::cout << toString(campaign);
```

That works, but it's a small papercut — you have to remember the helper exists and wrap every print in it. By overloading `operator<<` for `Campaign`, you fold the helper *into* the stream syntax:

```cpp
#include <ostream>

std::ostream& operator<<(std::ostream& out, Campaign campaign)
{
    return out << toString(campaign);
}
```

Now a `Campaign` prints like any built-in type:

```cpp
std::cout << Campaign::MutationA << '\n';   // prints: MutationA
```

Look closely at the signature. It takes the stream **by reference** (`std::ostream& out`) and **returns that same stream by reference**. That return is what makes chaining work — and chaining is something you rely on constantly without thinking about it.

### Reading an enum, and why input is harder

Input is the trickier direction, because reading from a stream can *fail*: the user might type something that's not a valid enumerator. A well-behaved `operator>>` reads the raw text, tries to convert it, and — if conversion fails — puts the stream into a failed state so the caller can detect the problem:

```cpp
#include <istream>
#include <string>

std::istream& operator>>(std::istream& in, Campaign& campaign)
{
    std::string text {};
    in >> text;

    if (auto parsed { campaignFromString(text) })   // did it match an enumerator?
        campaign = *parsed;
    else
        in.setstate(std::ios_base::failbit);        // mark the stream as failed

    return in;
}
```

Two things to note. The `Campaign&` parameter is a *non-const* reference because the operator's whole job is to fill it in. And on failure we call `in.setstate(std::ios_base::failbit)` — the same mechanism that makes `std::cin >> someInt` fail when the user types letters. The caller checks the stream as usual and reacts.

### Why the stream is returned by reference

That return-the-stream detail is what powers the chaining you do every day:

```cpp
std::cout << a << b << c;
```

This is really `((std::cout << a) << b) << c`. Each `<<` returns the stream, so the next `<<` has a stream to operate on. If `operator<<` returned `void`, the chain would break after the first step. Returning the stream by reference is the contract that keeps the dominoes falling.

> **Note:** You don't need to overload these operators for this chapter's lab — the exercise keeps `Item` data-only and handles printing with plain functions. Reading these patterns now pays off when starter code or libraries hand you stream-heavy utilities, and it primes you for the operator-overloading chapter later on.

---

## 13.6 — Scoped enumerations (enum classes)

### A stricter, safer enum

Lessons 13.2 and 13.3 left us with two complaints about unscoped enums: their enumerator names *leak* into the surrounding scope (causing collisions), and they *implicitly convert to int* (eroding type safety). **Scoped enumerations** fix both. You write them with the two-word keyword `enum class`:

```cpp
enum class Campaign
{
    MutationA,
    MutationB,
    MutationC,
};
```

The single most important consequence: the enumerators no longer leak. They live *inside* the enum's scope, so you reach them through the type name:

```cpp
Campaign campaign { Campaign::MutationA };   // note the Campaign:: prefix
```

That `Campaign::` prefix is the price of admission, and it buys you a lot.

### What you gain

Here are the two improvements side by side:

| Feature | Unscoped `enum` | Scoped `enum class` |
|---|---|---|
| Enumerator names leak into surrounding scope | Yes | No |
| Implicitly converts to `int` | Yes | No |
| Name-collision risk | Higher | Lower |
| Type safety | Weaker | Stronger |

The no-leak property means two enums can share enumerator spellings without a fight:

```cpp
enum class Color        { red, green };
enum class TrafficLight { red, yellow, green };

Color c { Color::red };
TrafficLight t { TrafficLight::red };
```

Both have a `red`, and there's no collision — each `red` is qualified by its own type. The unscoped version of this would have been a compile error, as we saw in 13.2.

### No accidental conversion to int

A scoped enum will *not* silently turn into an integer:

```cpp
// std::cout << Campaign::MutationA;        // error: no implicit conversion to int
```

At first this feels like a restriction, but it's a feature: it stops the meaning from leaking out as a nameless number, and it forces you to be deliberate. When you genuinely need the underlying value, you ask for it with a `static_cast`:

```cpp
int value { static_cast<int>(Campaign::MutationA) };   // explicitly: 0
```

The cast is a small bit of friction by design — it marks the exact spot where you're stepping outside the type's safety, which is precisely where a reader should pay attention. The flip side is that comparing scoped enums needs no cast at all: `a.rarity == b.rarity` just works, because `==` is defined on the enum type itself. (You'll use exactly that in the lab's `isSameItem`.)

### A note on `using enum` (C++20)

The `Campaign::` prefix is usually a virtue, but in a block that's saturated with one enum's values — a big `switch`, say — it can get repetitive. C++20 added `using enum` to pull a scoped enum's enumerators into the current scope temporarily:

```cpp
using enum Campaign;               // C++20
Campaign c { MutationA };          // now the prefix is optional here
```

Use it sparingly and locally. Most of the time the explicit `Campaign::MutationA` spelling is clearer, because it tells the reader at a glance which enum a name belongs to.

> **Best practice:** Default to `enum class` (scoped enumerations). You give up only the implicit int conversion — which you rarely want anyway — and you gain no name leakage, stronger type safety, and the freedom to reuse enumerator spellings across different enums. Reach for plain `enum` only when you have a specific reason, such as deliberately wanting the int conversion.

---

## 13.7 — Introduction to structs, members, and member selection

### Grouping values that belong together

Enums solved "one of a fixed set of choices." The other recurring need is the opposite: *several values that belong together as a unit*. A point is a line and a column. A student is a name and an ID. A catalogue item is a name, a tier, a quantity, and a price. A **struct** defines a type whose objects bundle a set of named **members**:

```cpp
struct CoveragePoint
{
    int line {};
    int column {};
};
```

This says: a `CoveragePoint` is one object that contains an `int line` and an `int column`. With the type defined, you create objects of it and initialize their members with a brace list:

```cpp
CoveragePoint point { 27, 13 };   // line = 27, column = 13
```

The values fill the members **in declaration order** — `27` goes to `line` because `line` is declared first, `13` goes to `column`. (We'll dig into this initialization in 13.8.)

### Reaching into a struct with the `.` operator

To read or write an individual member, you use the **member selection operator**, a dot:

```cpp
std::cout << point.line << ',' << point.column << '\n';   // prints: 27,13
point.line = 28;                                          // now line == 28
```

`point.line` means "the `line` member of the object `point`." You can read it, assign to it, pass it, anything you'd do with a plain variable of that member's type. Mentally, picture the object as a labeled box:

```text
CoveragePoint point
+--------+--------+
| line   | column |
|   27   |   13   |
+--------+--------+
```

### Why a struct beats loose variables

You could, of course, just keep two separate variables:

```cpp
int line { 27 };
int column { 13 };
```

versus the grouped form:

```cpp
CoveragePoint location { 27, 13 };
```

The grouped version wins on every axis that matters as a program grows. One name (`location`) travels as a unit, so you can't pass the line without the column or let them drift apart. You can pass the pair to a function in one argument, return it in one value, and store many of them without juggling two parallel containers. And the type announces what the pair *means*. Loose variables put all that bookkeeping on you; a struct hands it to the compiler.

### A preview of member defaults

You may have noticed the empty braces in the struct definition: `int line {};`. Those are **default member initializers**, and they matter enough that 13.9 is devoted to them — but here's the idea in brief. A member can carry its own default value, used whenever an object is created without an explicit value for that member:

```cpp
struct CoveragePoint
{
    int line { 0 };
    int column { 0 };
};

CoveragePoint p {};   // p.line == 0, p.column == 0 — a known, safe state
```

The rule is simple: a member's default kicks in exactly when initialization doesn't supply a value for it. Why bother? Because without defaults, certain ways of creating a struct leave its fundamental members holding **garbage** — whatever bits happened to be in that memory. Giving every member a default guarantees an object always starts life in a defined state, and that single habit eliminates a whole category of nasty, hard-to-reproduce bugs.

> **Best practice:** Give every data member of a simple struct a default member initializer (often just `{}`). It guarantees objects start in a known state and prevents accidental use of uninitialized values.

---

## 13.8 — Struct aggregate initialization

### Not every form of initialization is safe

Suppose you define a struct with no defaults on its members:

```cpp
struct Point
{
    int x;   // no default initializer
    int y;   // no default initializer
};
```

Now some ways of creating a `Point` will leave `x` and `y` **uninitialized** — holding indeterminate values — and reading them is undefined behavior. This is the danger 13.7 warned about, and it's the reason this lesson and the next push you toward initializers that fill members reliably. Keep that risk in mind as we walk through the initialization forms.

### What "aggregate" means

A struct like `Point` — public data members, no user-written constructors, nothing fancy — is called an **aggregate**. The defining feature of an aggregate is that you can initialize it with a brace list, and the values get handed to the members in order. This is **aggregate initialization**:

```cpp
Point p { 1, 2 };   // x = 1, y = 2
```

The mapping is positional — first value to first member, second to second:

```text
Point { 1, 2 }
        |  |
        v  v
        x  y
```

### Leaving some members out

You don't have to supply a value for every member. Any member you omit is **value-initialized** — which for fundamental types like `int` means it's set to `0`:

```cpp
Point p { 1 };   // x = 1, y = 0  (y value-initialized)
```

And if the struct *does* have default member initializers, an omitted member falls back to its default instead. The combination of "supply what you want, defaults fill the rest" is what makes aggregate initialization both safe and concise.

### Const structs

A struct object can be `const`, which locks all of its members against modification through that object:

```cpp
const Point origin { 0, 0 };
// origin.x = 1;   // error: can't modify a member of a const object
```

This is the same `const` you've used on plain variables, applied to the whole bundle: a `const` struct is a snapshot you can read but not change.

### Designated initializers (C++20)

Positional initialization is fine for a two-member `Point`, but for a struct with several members it gets hard to read — was that third number the quantity or the price? C++20's **designated initializers** let you name the member each value targets:

```cpp
Point p { .x = 1, .y = 2 };   // C++20
```

This is far clearer at the call site, especially when you skip some members. One rule to remember: in C++, the designators must appear in **declaration order** — you can't write `{ .y = 2, .x = 1 }`. (C is more lenient here; C++ is not.)

### Assigning with an initializer list

You can also *assign* to an existing struct using brace syntax, which replaces all its members at once:

```cpp
Point p {};       // p is {0, 0}
p = { 3, 4 };     // now p is {3, 4}
```

This reads as "give `p` these member values," and it follows the same positional rules as aggregate initialization.

> **Tip:** For a struct with more than two or three members, prefer designated initializers (`{ .name = ..., .quantity = ... }`) when your compiler supports C++20. Naming each field makes the call site self-documenting and protects you from silently transposing two values of the same type.

---

## 13.9 — Default member initialization

### Make "no value given" mean something safe

The previous lesson showed that leaving members without initializers is a hazard — some creation paths leave them holding garbage. **Default member initialization** is the fix, and it's so cheap and so effective that you should treat it as the default habit for every simple struct. You give each member a default value right in the struct definition:

```cpp
struct FuzzStats
{
    int testsRun {};
    int crashesFound {};
    double secondsElapsed {};
};
```

Those `{}` initializers value-initialize each member — `0` for the `int`s, `0.0` for the `double`. Now no matter how a `FuzzStats` gets created, it starts in a clean zero state. There is no path that produces garbage.

### Explicit values still win

Adding defaults doesn't take away your ability to supply values; an explicit initializer simply overrides the default for that member:

```cpp
struct Point
{
    int x { 0 };
    int y { 0 };
};

Point p { 5, 6 };   // x = 5, y = 6 — the explicit values override the defaults
```

### Omitted members fall back to their defaults

And when you supply *some* values but not all, each member you skip uses its default:

```cpp
Point p { 5 };   // x = 5 (explicit), y = 0 (default)
```

This is the behavior that makes defaults so pleasant: you specify only what's interesting and trust the rest to be sensible.

### Default initialization versus value initialization

Two ways of creating an object look almost identical but differ in an important detail:

```cpp
Point p1;     // default initialization (no braces)
Point p2 {};  // value initialization (empty braces)
```

The history is that, *without* default member initializers, `p1` could leave members uninitialized in some contexts while `p2 {}` would zero them. Once your struct gives every member a default initializer — as you should — both forms produce a fully initialized object, because the member defaults take over. Even so, prefer the braces:

```cpp
Point p {};   // clear, unambiguous, and always fully initialized
```

The empty-brace form states your intent to start from a clean slate, and it sidesteps the subtle distinction entirely. Combined with default member initializers, it's the form you'll reach for almost every time.

> **Best practice:** Two habits, applied together, eliminate uninitialized-member bugs: give every member a default initializer in the struct definition, and create objects with brace initialization (`Type x {};` or `Type x { ... };`). The lab's `ledger.h` already provides the defaults — your job is to keep using brace initialization throughout.

---

## 13.10 — Passing and returning structs

### Passing a struct into a function

Once a struct can travel as a unit, the question becomes *how* it should travel across a function call. There are three idioms, and choosing among them is mostly about size and intent.

A **small** struct — a couple of `int`s, say — is cheap to copy, so passing it **by value** is perfectly fine:

```cpp
void printPoint(CoveragePoint p);   // copies two ints — trivial
```

A **larger** struct — one holding a `std::string`, a few other members, anything whose copy isn't free — should be passed **by const reference**. This hands the function the caller's actual object (no copy) while promising not to modify it:

```cpp
void printStats(const FuzzStats& stats);   // no copy, read-only
```

And when the function's *purpose* is to change the caller's struct, you pass **by non-const reference** — the same `&`, but without `const`:

```cpp
void resetStats(FuzzStats& stats);   // mutates the caller's object
```

That single keyword is the whole story: `const FuzzStats&` means "I'll look but not touch," and `FuzzStats&` means "I'm here to change it." This chapter's lab leans on exactly this distinction — `itemWorth(const Item&)` reads, `restock(Item&)` mutates.

> **Best practice:** Pass small, cheap-to-copy structs by value. Pass larger structs you only read by `const` reference. Use a non-`const` reference only when modifying the caller's object is the function's actual job.

### Passing a temporary

You don't always have a named struct lying around to pass — you can construct one on the spot and hand it straight in:

```cpp
printPoint(CoveragePoint { 27, 13 });   // build a temporary and pass it
```

The temporary lives just long enough for the call and then disappears. This is handy when a struct exists only to bundle arguments for a single call.

### Returning a struct

Returning a struct by value is completely ordinary and is the clean way to hand back a composite result:

```cpp
CoveragePoint makePoint(int line, int column)
{
    return CoveragePoint { line, column };
}
```

You might worry about the cost of copying the result out of the function. In practice, modern C++ all but erases it: through **copy elision** and move semantics, the compiler typically constructs the result directly in the caller's variable, with no real copy at all. Return by value freely — it's both the clearest and, in practice, an efficient choice.

### Structs let a function return more than one thing

Here's where returning structs really earns its place. A function can naturally produce only one value — but sometimes you have two logical results, like "did it succeed?" *and* "what's the parsed point?" The clumsy way is out-parameters: extra reference parameters the function writes into. The clean way is to bundle the results into a struct and return that:

```cpp
struct ParseResult
{
    bool ok {};
    CoveragePoint point {};
};

ParseResult parseCoverageLine(std::string_view line);
```

Now the caller gets one tidy value carrying everything, the success flag and the data are obviously paired, and there are no mysterious reference parameters to track. A struct return reads better than a fistful of out-parameters almost every time.

---

## 13.11 — Struct miscellany

This lesson gathers three practical facts about structs that don't each need a full section but matter once you start designing real types.

### Structs can contain other program-defined types

Members aren't limited to fundamental types — a struct member can be *another* struct, or an enum, or any type you've defined. This is how you assemble a domain model out of small pieces:

```cpp
enum class Campaign { MutationA, MutationB, MutationC };

struct CoveragePoint
{
    int line {};
    int column {};
};

struct FuzzResult
{
    Campaign campaign {};      // a program-defined enum member
    CoveragePoint location {}; // a program-defined struct member
    bool crashed {};
};
```

Types **compose**: a `FuzzResult` is built from a `Campaign`, a `CoveragePoint`, and a `bool`, each of which carries its own meaning. This nesting is the foundation of how programs model their domains — small types combining into bigger ones, meaning accumulating at every layer.

### A struct that owns data should hold owning members

When a struct is meant to *own* its data — to be responsible for keeping it alive — its members should be **owning types**. An owning type, like `std::string` or `std::vector`, manages the storage for its contents and keeps them valid for as long as the object lives:

```cpp
struct SeedCorpus
{
    std::vector<std::string> seeds {};   // owns its strings (preview: std::vector is Chapter 16)
};
```

The trap to avoid is storing a **non-owning** member — a raw pointer or a `std::string_view` — and expecting it to keep data alive. It won't. A view or pointer only *refers* to data owned elsewhere; if that data dies first, the view is left **dangling**, pointing at freed memory:

```cpp
struct BadSeed
{
    std::string_view text {};   // refers to data it doesn't own — may dangle later
};
```

If the struct needs the data to outlive whatever produced it, store an owning member instead:

```cpp
struct Seed
{
    std::string text {};   // owns its characters; safe to keep around
};
```

Non-owning members aren't forbidden — they're a fine optimization when the lifetimes are obvious and the view clearly doesn't outlive what it points at. But the default for a type that stores data should be ownership. (This is precisely why the lab's `makeItem` converts its `std::string_view` parameter into an owning `std::string` member.)

> **Warning:** A `std::string_view` or raw pointer member does *not* own what it refers to. Store one only when the referenced data is guaranteed to outlive the struct. When in doubt, use an owning member like `std::string`.

### Size, alignment, and padding

A struct's size in memory isn't always the simple sum of its members' sizes. To let the CPU access each member efficiently, the compiler aligns members to certain address boundaries, and to do that it may insert unused **padding** bytes between them:

```cpp
struct Example
{
    char c;   // 1 byte
    int i;    // 4 bytes, but typically wants a 4-byte-aligned address
};
```

A plausible layout puts three padding bytes after `c` so that `i` lands on an aligned boundary:

```text
byte:  0    1    2    3    4 5 6 7
       c   pad  pad  pad   i i i i
```

So this struct may occupy 8 bytes even though its members total only 5. A consequence: **member order can affect a struct's size**, because reordering changes where padding is needed. You can occasionally shrink a struct by ordering members from largest to smallest. But don't reorder for size by reflex — readability usually matters more. Optimize layout only when measurements show the size or cache behavior actually matters.

> **Note:** Padding is why `sizeof(SomeStruct)` can exceed the sum of its members' sizes. It's a normal, automatic consequence of alignment, not a bug — and rarely something to worry about until you're tuning memory or performance.

---

## 13.12 — Member selection with pointers and references

### The same dot, for objects and references

You already know that you reach a member of a struct object with the dot operator. The good news is that a **reference** to a struct uses the exact same syntax — a reference is just another name for the object it binds to, so member access looks identical:

```cpp
CoveragePoint p { 27, 13 };
CoveragePoint& ref { p };   // ref is another name for p

std::cout << p.line << '\n';     // 27
std::cout << ref.line << '\n';   // 27 — same object, same dot
```

There's no special syntax to learn for references. If you have an object or a reference, you use `.`.

### Through a pointer, use the arrow

A **pointer** is different. A pointer doesn't *hold* a struct — it holds the *address* of one. To get at a member, you first have to follow the pointer to the object it points at (dereference it), and *then* select the member. You could write that out the long way:

```cpp
CoveragePoint* ptr { &p };       // ptr holds the address of p
std::cout << (*ptr).line << '\n';   // dereference, then select — clunky
```

The `(*ptr).line` spelling works but is awkward, and the parentheses are mandatory (without them, `*ptr.line` would mean the wrong thing). Because following-a-pointer-then-selecting is so common, C++ gives it a dedicated operator: the **arrow**, `->`:

```cpp
std::cout << ptr->line << '\n';   // exactly the same as (*ptr).line, but clean
```

`ptr->line` is by definition equivalent to `(*ptr).line`. It's the idiomatic way to reach a member through a pointer — use it.

> **Key insight:** Use `.` when you have an object or a reference; use `->` when you have a pointer. `ptr->member` is shorthand for `(*ptr).member` — it dereferences and selects in one clean step.

### Chaining arrows through a data structure

When pointers link objects together — as in a linked list or a tree — the arrows chain naturally, each one stepping through one pointer:

```cpp
node->next->value
```

Read left to right, that's: start at the pointer `node`, follow it to find the `next` member (itself a pointer), follow *that* to the object it points at, and select its `value`:

```text
node       pointer to a node
  ->next   follow it; next is also a pointer
     ->value   follow that; select the value member
```

Each `->` is one hop across one pointer. This is the everyday vocabulary of walking pointer-linked structures.

### A real-world decoder

You'll meet this `.`-versus-`->` choice constantly in production C++, where APIs hand you a mix of references and pointers. The rule that decides which operator to use is simply *what type is in your hand*:

```cpp
Module* M = F.getParent();   // getParent returns a pointer
M->getOrInsertFunction(/* ... */);   // M is a pointer  -> use ->
```

```cpp
Instruction& I = /* ... */;   // I is a reference
I.getDebugLoc();              // I is a reference  -> use .
```

There's nothing to memorize beyond the one rule: pointer gets `->`, object-or-reference gets `.`. Once that's automatic, code that mixes the two reads without friction.

---

## 13.13 — Class templates

### The duplication problem

Imagine you need a type that holds a pair of values — two of the same kind:

```cpp
struct IntPair
{
    int first {};
    int second {};
};
```

That serves `int`s. But soon you want a pair of `double`s, so you write `DoublePair`. Then a pair of `std::string`s — `StringPair`. Every one of these is the *same shape*; only the element type changes. Copying that shape over and over is tedious and, worse, fragile: fix a bug in one and you have to remember to fix it in all the others.

### Templating the shape

The function templates from Chapter 11 solved this same duplication for *functions* — write the algorithm once, leave the type as a blank. A **class template** does the same for *types*. You write the struct once with the element type left as a parameter, and the compiler stamps out a concrete type for each element type you actually use:

```cpp
template <typename T>
struct Pair
{
    T first {};
    T second {};
};
```

`T` is a placeholder for "some type, to be chosen later." When you use `Pair`, you fill in `T` in angle brackets, and the compiler generates the matching type on the spot:

```cpp
Pair<int> p { 1, 2 };        // T = int
Pair<double> d { 1.5, 2.5 }; // T = double
```

The mental model is a stencil: one `Pair<T>` pattern, many concrete types pressed out of it.

```text
Pair<T>  (the pattern)
   T = int     ->  Pair<int>
   T = double  ->  Pair<double>
```

### More than one type parameter

A template isn't limited to a single placeholder. If your pair should hold two *different* types — a key and a value, say — you give it two type parameters:

```cpp
template <typename T, typename U>
struct Pair
{
    T first {};
    U second {};
};

Pair<int, std::string> item { 1, "one" };   // T = int, U = std::string
```

### The standard library already has this

Pairs are so universally useful that the standard library ships one: `std::pair<T, U>`, in the `<utility>` header.

```cpp
#include <utility>

std::pair<int, int> location { 27, 13 };
```

So when *should* you write your own struct versus reaching for `std::pair`? The deciding factor is *meaning*. For a value with real domain significance, a named struct with named members reads far better:

```cpp
CoveragePoint location { 27, 13 };   // location.line, location.column — self-explanatory
```

Compare that to `std::pair`, whose members are the anonymous `.first` and `.second` — fine for throwaway, generic plumbing, but a poor name for "a line and a column." Use `std::pair` for generic utility code; use a named struct when the pair *means* something.

### Templates live in headers

One practical consequence sets templates apart from ordinary functions. To generate `Pair<int>`, the compiler needs the *full definition* of `Pair` visible at the point where you use it — not just a declaration. So class templates are defined entirely in **header files**, where every file that uses them can see the whole definition. This differs from ordinary non-template functions, where a declaration can live in a header and the definition in a `.cpp` file. With templates, the definition has to come along.

---

## 13.14 — Class template argument deduction (CTAD) and deduction guides

### Letting the compiler figure out the type arguments

Spelling out template arguments can feel redundant when the compiler could obviously work them out from the values you're initializing with. **Class template argument deduction**, or **CTAD**, lets you drop the angle-bracket arguments and have the compiler deduce them from the initializer:

```cpp
std::pair p { 1, 2.5 };   // CTAD deduces std::pair<int, double>
```

The compiler sees an `int` and a `double` and concludes `std::pair<int, double>`. Without CTAD you'd write the arguments yourself:

```cpp
std::pair<int, double> p { 1, 2.5 };   // explicit — same type, more typing
```

Both produce the identical type; CTAD just removes the boilerplate when the answer is unambiguous.

### Deduction guides

Most of the time CTAD "just works" because the standard library types come with the help it needs. Occasionally a class template can't tell the compiler how to map an initializer to its type parameters on its own; in those cases the library provides **deduction guides** — small hints that spell out the deduction. You're unlikely to *write* deduction guides any time soon, but it's worth knowing the term, because the reason your favorite standard types deduce so smoothly is often a deduction guide working quietly behind the scenes.

### Where CTAD doesn't reach

CTAD is a convenience, not a universal rule, and it has limits worth knowing:

- it doesn't apply to **function parameters** — a parameter type must be fully specified;
- it doesn't reliably cover every non-static member initialization pattern;
- it can't deduce anything when the initializer doesn't carry enough type information.

When clarity is on the line, prefer to write the arguments explicitly anyway:

```cpp
std::vector<int> values { 1, 2, 3 };   // explicit element type — unmistakable
```

> **Tip:** Let CTAD remove boilerplate when the deduced type is obvious from the initializer. Write the template arguments out explicitly when doing so makes the code clearer, or when CTAD simply can't deduce what you mean.

---

## 13.15 — Alias templates

### Naming a family of types

Chapter 10 introduced type aliases — `using Score = int;` — to give a shorter or more meaningful name to an existing type. An **alias template** extends that idea to *templated* types: it lets you name a whole family of types parameterized by a type:

```cpp
template <typename T>
using Vec = std::vector<T>;

Vec<int> values { 1, 2, 3 };   // exactly std::vector<int>
```

`Vec<int>` is simply another name for `std::vector<int>` — not a new type, just a more convenient spelling. The payoff grows with the complexity of the type you're aliasing. A long, repeated template type can be collapsed into a short, meaningful name:

```cpp
template <typename T>
using OptionalRef = std::optional<std::reference_wrapper<T>>;
```

Writing `OptionalRef<Widget>` everywhere is far kinder to readers than spelling out `std::optional<std::reference_wrapper<Widget>>` each time.

### A note of caution

Alias templates are a readability tool, and like any abstraction they can be overused. If you hide the *real* data structure behind an alias so thoroughly that readers can no longer tell what's actually being used — a map? a vector? something custom? — you've traded clarity for brevity, which is a bad deal. Use alias templates to tame genuinely complex, repeated types, not to obscure simple ones.

```cpp
// A reasonable alias: names a recurring, nontrivial shape.
template <typename T>
using SeedMap = std::map<Campaign, std::vector<T>>;
```

> **Best practice:** Use an alias template to give a clear name to a complex, frequently repeated templated type. Don't alias a type so aggressively that the reader can no longer see which data structure they're actually dealing with.

---

## 13.x — Chapter 13 summary and quiz

### Core takeaways

You started this chapter with only the types C++ gave you; you're ending it able to design your own. The threads to carry forward:

- **Program-defined types** let the type system model your domain, so meaning lives in the code instead of in your head.
- Put **reusable type definitions in headers** — type definitions are exempt from the ODR's one-definition rule as long as every copy is identical.
- **Unscoped enums** leak their enumerator names into the surrounding scope and implicitly convert to `int`.
- **Scoped enums (`enum class`)** fix both problems — no name leakage, no implicit int conversion — and are the sensible default.
- Write **helper functions** to convert enums to and from strings; return `std::optional` on a parse that can fail.
- **Operator overloads** can make your types print and read through streams, but they're optional for learning code.
- **Structs** group related data under one named type, accessed with the `.` operator.
- Use **`.`** for objects and references, **`->`** for pointers (`ptr->m` is `(*ptr).m`).
- Give every struct member a **default initializer** and create objects with **brace initialization** — together they banish uninitialized members.
- **Aggregate initialization** fills members in declaration order; omitted members value-initialize or fall back to their defaults.
- Pass small structs **by value**, larger read-only structs **by `const` reference**, and use a **non-`const` reference** only to mutate.
- Prefer **returning a struct** over a cluster of out-parameters.
- A struct that **owns data** should hold owning members (like `std::string`), not dangling-prone views or pointers.
- **Padding and alignment** can make a struct larger than the sum of its members.
- **Class templates** define type patterns like `Pair<T>`; **CTAD** can deduce their arguments; **alias templates** name complex templated types.

### Designing types for a real program

Here's the whole chapter distilled into one design. Instead of passing around a loose pile of values whose relationships live only in your memory:

```cpp
int campaign;
int line;
int column;
std::string input;
bool crashed;
```

you model the domain with purpose-built types:

```cpp
enum class Campaign
{
    MutationA,
    MutationB,
    MutationC,
};

struct CoveragePoint
{
    int line {};
    int column {};
};

struct FuzzResult
{
    Campaign campaign {};
    std::string input {};
    bool crashed {};
};
```

The second version *says what the values mean*, keeps related data together, and lets the compiler catch mistakes the loose version would wave through. That's the entire argument for program-defined types, in one before-and-after.

### Member-access cheat sheet

```cpp
CoveragePoint point { 27, 13 };
CoveragePoint& ref { point };
CoveragePoint* ptr { &point };

point.line;   // object    -> dot
ref.line;     // reference -> dot
ptr->line;    // pointer   -> arrow
```

### A complete worked example

This small program pulls the chapter's central ideas together — a scoped enum for a closed set of choices, a struct for grouped data, an enum-to-string helper, and a read-only struct passed by const reference:

```cpp
#include <iostream>
#include <string_view>

enum class Campaign
{
    MutationA,
    MutationB,
    MutationC,
};

struct CoveragePoint
{
    int line {};
    int column {};
};

std::string_view toString(Campaign campaign)
{
    switch (campaign)
    {
    case Campaign::MutationA: return "MutationA";
    case Campaign::MutationB: return "MutationB";
    case Campaign::MutationC: return "MutationC";
    }

    return "Unknown";
}

void printCoverage(Campaign campaign, const CoveragePoint& point)
{
    std::cout << toString(campaign)
              << " reached "
              << point.line << ','
              << point.column << '\n';
}

int main()
{
    CoveragePoint point { 27, 13 };
    printCoverage(Campaign::MutationA, point);
}
```

Every line earns its place: `enum class` for a small closed set of choices, `struct` for grouped domain data, `std::string_view` for returning string literals that already live forever, `const CoveragePoint&` for read-only no-copy passing, and a `switch` over a scoped enum with a fallback `return`.

### The lab: Inventory Ledger

This chapter's exercise puts every idea above to work in one small library. You'll build an **Inventory Ledger** that tracks game-shop items by rarity tier. A `Rarity` is a scoped `enum class` (`Common`, `Rare`, `Epic`), and an `Item` is a data-only `struct` holding a name, a rarity, a quantity, and a unit value. All the behavior lives in **free functions** outside the struct — deliberately, because keeping `Item` a pure "bag of data" sets up the leap to member functions in the next chapter.

The six tasks walk straight through the chapter:

- **`rarityLabel`** — `switch` on a scoped enum, with a `default` arm that catches out-of-range values (13.4, 13.6).
- **`rarityMultiplier`** — the same switch pattern, returning the tier's value multiplier, kept in one place so every caller stays in sync.
- **`itemWorth`** — take the `Item` by `const Item&`, access members with `.`, and delegate to `rarityMultiplier` rather than inlining magic numbers (13.7, 13.10).
- **`makeItem`** — aggregate-initialize and return an `Item` by value, converting a `std::string_view` parameter into an owning `std::string` member (13.8, 13.10, 13.11).
- **`restock`** — mutate through a non-`const` `Item&`, clamping a negative amount to zero (13.10, 13.12).
- **`isSameItem`** — compare struct fields, using `==` directly on the scoped enum (13.6).

The starter compiles but every function returns a placeholder, so `make test` starts **red**; filling in the six blocks turns it **green**. That red-to-green transition is the whole exercise — and by the end you'll have written, in miniature, exactly the enum-plus-struct-plus-free-functions pattern that pervades real C++ codebases.

---

## 13.y — Using a language reference

LearnCpp closes the chapter with a skill that pays dividends for the rest of your programming life: how to read a **language reference**. A guided tutorial like this course takes you through concepts in a deliberate order. A reference is a different kind of tool — a lookup manual for the exact rules, signatures, and behavior of a feature you already know you need. Learning to use one is how you stop depending on tutorials for every answer.

### Tutorial versus reference

The two serve different moments, and it helps to keep them straight:

```text
tutorial:
  teaches a concept in a planned sequence
  explains why a feature exists
  gives curated, build-up examples

reference:
  documents exact syntax, overloads, and edge cases
  lists constraints and guarantees precisely
  helps when you already know what to look up
```

You reach for a tutorial to *learn* something new, and a reference to *confirm a detail* about something you already half-know. Both are essential; they just answer different questions.

### What to look for in a reference entry

When you look up a C++ feature or a standard-library component, a good reference entry will tell you, among other things:

- the required **header** and **namespace**,
- the **declaration / signature**,
- any **template parameters** and what they mean,
- the **parameters** and **return type**,
- **preconditions** and **postconditions**,
- **complexity** guarantees,
- **invalidation** rules (for example, which container operations invalidate iterators),
- worked **examples**, and the **language-standard version** the feature requires.

For a standard-library type, a quick mental sketch might read:

```text
std::vector
  header:    <vector>
  namespace: std
  key ops:   push_back, size, operator[], at, begin/end
  invalidation: some operations invalidate iterators and references
```

### Reading a declaration

References speak in declarations, not prose, so it's worth being able to read one fluently. A class-template declaration:

```cpp
template <class T>
class vector;
```

reads as: `vector` is a class template, `T` is its element-type parameter, and `std::vector<int>` is the instantiation with `T = int`. A member-function declaration:

```cpp
std::size_t size() const noexcept;
```

reads as: it returns a `std::size_t`, takes no parameters, can be called on `const` objects (the trailing `const`), and promises not to throw (`noexcept`). Once you can decode a signature line like this, the densest reference page becomes navigable.

### Why this habit matters

Real codebases — the standard library, and the LLVM-style code you'll meet in later study — constantly require lookups. When you hit an unfamiliar name, the move is always the same: find its declaration, check its header and namespace, read its return type and parameters, and note any lifetime, ownership, or error behavior. Suppose you see

```cpp
auto it { seeds.begin() };
```

and you're not sure what `it` is. A reference tells you `std::vector<T>::begin()` returns an *iterator* pointing at the first element — and now you know how to use `it`. The same loop applies to any project-specific API you encounter.

### The practical lookup loop

Boiled down to a routine you can run on autopilot:

```text
1. Identify the exact name.
2. Decide what it is — language syntax, standard library, or project API.
3. Find the declaration / signature.
4. Check the required header and namespace.
5. Read the return type and parameters.
6. Note lifetime, ownership, invalidation, and error behavior.
7. If still unsure, write a tiny local example and run it.
```

That loop is the difference between believing you must memorize the whole language and knowing you can *interrogate* the toolchain whenever you need an answer. The second mindset is the one that scales — and it's the note this chapter, and your move from guided learning to self-sufficient programming, ends on.
