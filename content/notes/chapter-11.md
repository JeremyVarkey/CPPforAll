# Chapter 11 — Function Overloading and Function Templates

> Source: <https://www.learncpp.com/> (Chapter 11)
> One file per chapter. Each lesson is a section below.

## Contents
- [11.1 — Introduction to function overloading](#111--introduction-to-function-overloading)
- [11.2 — Function overload differentiation](#112--function-overload-differentiation)
- [11.3 — Function overload resolution and ambiguous matches](#113--function-overload-resolution-and-ambiguous-matches)
- [11.4 — Deleting functions](#114--deleting-functions)
- [11.5 — Default arguments](#115--default-arguments)
- [11.6 — Function templates](#116--function-templates)
- [11.7 — Function template instantiation](#117--function-template-instantiation)
- [11.8 — Function templates with multiple template types](#118--function-templates-with-multiple-template-types)
- [11.9 — Non-type template parameters](#119--non-type-template-parameters)
- [11.10 — Using function templates in multiple files](#1110--using-function-templates-in-multiple-files)
- [11.x — Chapter 11 summary and quiz](#11x--chapter-11-summary-and-quiz)

---

## 11.1 — Introduction to function overloading

### One operation, multiple type shapes

Function overloading lets multiple functions share the same name when their
parameter lists are different enough for the compiler to tell them apart.

Without overloading:

```cpp
int scoreInt(int x)
{
    return x + 10;
}

double scoreDouble(double x)
{
    return x + 10.0;
}
```

The names encode type details that callers should not have to remember.

With overloading:

```cpp
int score(int x)
{
    return x + 10;
}

double score(double x)
{
    return x + 10.0;
}
```

The caller asks for the operation. The compiler chooses the overload based on
the argument types.

```cpp
score(4);    // calls score(int)
score(4.5);  // calls score(double)
```

### Overloaded functions

Each function with the same name is an **overload**.

```cpp
void logValue(int value);
void logValue(double value);
void logValue(std::string_view value);
```

These are all named `logValue`, but each has a different parameter type.

### Why overloading is useful

Overloading is useful when functions have the same conceptual job:

- print a value,
- compare values,
- construct an object from different inputs,
- parse data from different sources,
- operate on one item vs several items.

It is not a license to reuse one name for unrelated behavior.

Good:

```cpp
void printToken(int tokenId);
void printToken(std::string_view spelling);
```

Questionable:

```cpp
void process(int count);          // run count iterations
void process(std::string path);   // delete a file at path
```

The overload set should still feel like one operation.

### Overloading and CS6340-style code

LLVM-style code often has functions that accept related program objects:

```cpp
void dumpName(const Function& function);
void dumpName(const BasicBlock& block);
void dumpName(const Instruction& instruction);
```

Those overloads are readable because the operation is the same: print the name
or identifying label for a program element.

---

## 11.2 — Function overload differentiation

### Differentiation means "can these declarations coexist?"

Before overload resolution can choose a function call, the declarations
themselves must be distinguishable.

```cpp
void print(int);
void print(double); // ok: parameter type differs
```

```cpp
void print(int);
void print(int);    // not a new overload: same parameter list
```

### What differentiates overloads?

| Function property | Differentiates overloads? | Notes |
|---|---|---|
| Number of parameters | Yes | `f(int)` vs `f(int, int)` |
| Parameter types | Yes | `f(int)` vs `f(double)` |
| Parameter order | Yes | `f(int, double)` vs `f(double, int)` |
| Return type | No | caller may ignore the return value |
| Type aliases | No | alias is the same underlying type |
| Top-level `const` on by-value parameter | No | parameter is still copied |

### Number of parameters

```cpp
int combine(int a, int b)
{
    return a + b;
}

int combine(int a, int b, int c)
{
    return a + b + c;
}
```

These are differentiated because one takes two parameters and the other takes
three.

### Parameter type and order

```cpp
void record(int line, std::string_view file);
void record(std::string_view file, int line);
```

The types appear in a different order, so these are different overloads.

This can be readable when both call shapes are natural, but too many overloads
with swapped argument order can make calls harder to audit.

### Type aliases do not create new types

```cpp
using UserId = int;
using GroupId = int;

void printId(int id);
void printId(UserId id);  // same as printId(int)
void printId(GroupId id); // same as printId(int)
```

`UserId` and `GroupId` improve readability, but they are aliases for `int`, not
distinct types.

If the types need overload-level separation, use actual program-defined types:

```cpp
struct UserId
{
    int value {};
};

struct GroupId
{
    int value {};
};

void printId(UserId id);
void printId(GroupId id);
```

### Return type is not enough

```cpp
int nextValue();
double nextValue(); // error: return type alone cannot differentiate
```

A call like this gives the compiler no context:

```cpp
nextValue(); // return value ignored
```

Even when context exists, C++ does not use return type to differentiate overloads
or choose between otherwise identical functions.

### `const` by value does not differentiate

```cpp
void consume(int value);
void consume(const int value); // not a separate overload
```

Both functions receive a copied `int`. The `const` only affects whether the
function body can modify its local copy.

Different story for references and pointers, which Chapter 12 covers:

```cpp
void inspect(int& value);
void inspect(const int& value); // different parameter type
```

### Name mangling

Compiled object files need unique names for functions. Compilers typically
mangle function names by encoding parameter information into the compiled symbol.

Conceptually:

```
source name:     visit(int)
symbol idea:     visit__int

source name:     visit(double)
symbol idea:     visit__double
```

The exact mangled names are compiler-specific. The important point is that
overloads are not magic at link time; the compiler emits distinguishable symbols
for distinguishable signatures.

---

## 11.3 — Function overload resolution and ambiguous matches

### Resolution means "which overload does this call use?"

Differentiation asks whether declarations can coexist. **Overload resolution**
asks which overload is the best match for a particular call.

```cpp
void print(int);
void print(double);

print(5);   // exact int match
print(5.5); // exact double match
```

Exact matches are simple. Non-exact matches are where the rules matter.

### High-level matching order

The compiler tries increasingly flexible matches:

1. Exact match, including trivial adjustments.
2. Numeric promotion.
3. Numeric conversion.
4. User-defined conversion.
5. Ellipsis match.
6. No match.

If exactly one match is found at a step, that overload wins. If more than one
match is found at the same step and no single one is better, the call is
ambiguous.

```
call expression
    |
    v
exact? -------- yes, one ---> use it
    |
promotion? ---- yes, one ---> use it
    |
conversion? --- yes, one ---> use it
    |
user-defined? - yes, one ---> use it
    |
ellipsis? ----- yes, one ---> use it
    |
no match or ambiguous -> compile error
```

### Exact matches

```cpp
void handle(int);
void handle(double);

handle(3);   // handle(int)
handle(3.0); // handle(double)
```

Some minor adjustments can still count in the exact-match family, such as
discarding top-level `const` from a by-value argument.

### Promotions beat conversions

Numeric promotions are preferred over broader numeric conversions.

```cpp
void show(int);
void show(double);

char c { 'A' };
show(c); // char promotes to int, so show(int)
```

`char` to `int` is a promotion. `char` to `double` is a conversion. Promotion
wins.

### Ambiguous conversions

```cpp
void draw(long);
void draw(double);

draw(10); // int can convert to long or double
```

If both candidates require conversions at the same rank and neither is clearly
better, the compiler reports an ambiguous call.

You can resolve ambiguity by:

```cpp
draw(10L);                         // make the literal long
draw(static_cast<double>(10));      // ask for double explicitly
```

### Multiple parameters

For calls with multiple arguments, one overload must be at least as good for all
parameters and better for at least one.

```cpp
void mix(int, double);
void mix(double, int);

mix(1, 2); // ambiguous: first is better for arg1, second is better for arg2
```

Neither overload dominates the other.

### Non-template functions vs templates

When a non-template function and a function template both match, the non-template
function often wins if it is an equally good match.

```cpp
void print(bool value)
{
    std::cout << std::boolalpha << value << '\n';
}

template <typename T>
void print(T value)
{
    std::cout << value << '\n';
}

print(true); // calls print(bool), the non-template exact match
```

That is usually what you want: the non-template overload can provide behavior
specialized for a particular type.

### Practical overload hygiene

To keep overload sets predictable:

- avoid overloads that differ only by easily converted arithmetic types,
- avoid mixing default arguments with many overloads unless calls stay obvious,
- use `static_cast` at call sites when the intended overload matters,
- use deleted overloads to block dangerous implicit conversions.

---

## 11.4 — Deleting functions

### Deleted functions are intentionally uncallable

The `= delete` specifier marks a function as unusable. If overload resolution
selects a deleted function, compilation fails.

```cpp
void printCode(int code)
{
    std::cout << code << '\n';
}

void printCode(char) = delete;
void printCode(bool) = delete;
```

Now these are rejected:

```cpp
printCode('A');  // char overload is deleted
printCode(true); // bool overload is deleted
```

This is better than silently promoting `'A'` to `65` or `true` to `1` when those
calls do not make semantic sense.

### Deleted overloads participate in overload resolution

Deleted functions are not invisible. They can still be chosen as the best match.
The point is to make that chosen call a compile-time error.

```cpp
void setRetryCount(int count);
void setRetryCount(double) = delete;

setRetryCount(3);   // ok
setRetryCount(3.5); // compile error, not silent truncation
```

This is a useful way to protect APIs from unwanted implicit conversions.

### Delete broad matches when narrowing is risky

```cpp
void reserveSlots(std::size_t count);

void reserveSlots(int) = delete;    // avoid negative-to-huge surprises
void reserveSlots(double) = delete; // avoid truncation
```

Whether this is too strict depends on the codebase. For beginner programs, it is
often enough to understand that `= delete` can make invalid calls fail early.

### CS6340 tie-in

For mutation or instrumentation code, deleted overloads can preserve API intent.

```cpp
void instrument(Function& function);
void instrument(Function*) = delete; // caller must prove object exists
```

This forces callers to pass a real reference instead of a pointer that might be
`nullptr`.

---

## 11.5 — Default arguments

### Defaults are inserted at the call site

A **default argument** supplies a value when the caller omits an argument.

```cpp
void repeat(std::string_view text, int times = 1)
{
    for (int count { 0 }; count < times; ++count)
        std::cout << text;
}
```

Calls:

```cpp
repeat("x", 3); // uses explicit 3
repeat("x");    // compiler treats as repeat("x", 1)
```

The compiler inserts the default value where the function is called.

### Use defaults for common policy values

Good defaults are values that callers frequently want but may occasionally
override.

```cpp
void writeLog(std::string_view message, bool includeTimestamp = true);
int rollDie(int sides = 6);
```

Defaults are also useful when adding a new parameter to an existing function.
Existing call sites keep compiling, while newer call sites can opt into the new
parameter.

### Defaults must be right-aligned

Once a parameter has a default, all parameters to its right must also have
defaults.

```cpp
void connect(std::string_view host,
             int port = 443,
             bool useTls = true); // ok
```

Not allowed:

```cpp
void connect(std::string_view host = "localhost",
             int port); // error: non-default after default
```

C++ does not let a call skip a middle argument:

```cpp
connect(, 8080); // no such syntax
```

Arguments supplied by the caller are matched from the left.

### Put defaults in declarations, not repeated definitions

If a function has a forward declaration, put the default argument there.

Header:

```cpp
// logger.h
void writeLog(std::string_view message, bool includeTimestamp = true);
```

Source:

```cpp
// logger.cpp
void writeLog(std::string_view message, bool includeTimestamp)
{
    // implementation
}
```

Do not repeat the default in both places within the same translation unit.

Reason:

```
call site must see the default
    -> header declaration is visible
    -> source definition may not be visible
```

### Defaults can make overloads ambiguous

```cpp
void draw(int width);
void draw(int width, int height = 10);

draw(5); // ambiguous: could mean draw(int) or draw(int, int)
```

Default arguments are not part of the function signature for differentiation,
but they do affect which calls are viable. Keep overload sets with defaults
small and obvious.

### Default arguments vs overloads

Use a default argument when one implementation handles both cases:

```cpp
void printLine(std::string_view text, char ending = '\n');
```

Use overloads when the omitted/alternate form has meaningfully different logic:

```cpp
void loadConfig();
void loadConfig(std::string_view path);
```

Both are tools for nicer call sites; do not combine them in ways that make the
reader guess.

---

## 11.6 — Function templates

### The duplication problem

Overloads can reduce naming clutter, but they can still duplicate implementation.

```cpp
int smaller(int a, int b)
{
    return (a < b) ? a : b;
}

double smaller(double a, double b)
{
    return (a < b) ? a : b;
}
```

The algorithm is the same. Only the type changes.

### Function template basics

A **function template** is a pattern the compiler uses to generate functions for
specific types.

```cpp
template <typename T>
T smaller(T a, T b)
{
    return (a < b) ? a : b;
}
```

`T` is a **type template parameter**. It is a placeholder for a real type that
will be supplied later.

Conceptual stencil:

```
template:
    T smaller(T, T)

instantiations:
    int smaller(int, int)
    double smaller(double, double)
    std::string smaller(std::string, std::string)
```

The template itself is not the function that runs. The generated function for a
specific type is what runs.

### Template parameter declaration

```cpp
template <typename T>
```

This line:

- says the next declaration is a template,
- declares `T` as a type placeholder,
- scopes `T` to that template.

`typename` and `class` both work for type template parameters:

```cpp
template <class T>
T identity(T value)
{
    return value;
}
```

Prefer `typename` in these notes because the placeholder can be any type, not
only a class type.

### Template parameter naming

Common convention:

| Name | Typical use |
|---|---|
| `T` | first simple type parameter |
| `U` | second simple type parameter |
| `N` | integer non-type parameter |
| descriptive capitalized name | when requirements are non-obvious |

Example:

```cpp
template <typename Iterator>
void advanceOne(Iterator& iterator)
{
    ++iterator;
}
```

The name `Iterator` documents that the type must support iterator-like
increment.

### Template requirements are real

The template compiles for a type only if the operations used in the template body
make sense for that type.

```cpp
template <typename T>
T addOne(T value)
{
    return value + 1;
}
```

Works for:

```cpp
addOne(3);   // int + int
addOne(2.5); // double + int
```

May not work for:

```cpp
addOne("abc"); // pointer arithmetic on string literal, not "abc1"
```

Templates are flexible, but they are not magic. The function body still has to
be meaningful after `T` is replaced with the actual type.

---

## 11.7 — Function template instantiation

### Instantiation creates real functions

**Function template instantiation** is the process of generating a function from
a function template for specific template arguments.

```cpp
template <typename T>
T larger(T a, T b)
{
    return (a < b) ? b : a;
}

int x { larger<int>(1, 2) };
```

Conceptually, the compiler creates something like:

```cpp
int larger(int a, int b)
{
    return (a < b) ? b : a;
}
```

That generated function is an instantiated function. LearnCpp also uses the term
function instance informally.

### Explicit template arguments

You can name the template argument directly:

```cpp
larger<int>(1, 2);
larger<double>(1, 2); // arguments converted to double parameters
```

`larger<double>(1, 2)` asks for `T == double`, so the generated function takes
`double` parameters. The integer arguments are converted to `double`.

### Template argument deduction

Usually, let the compiler deduce `T` from the function arguments:

```cpp
larger(1, 2);     // T deduced as int
larger(1.5, 2.0); // T deduced as double
```

Normal call syntax is preferred when it expresses the intended call.

### Deduction does not do type conversion

```cpp
larger(1, 2.5); // cannot deduce one T from int and double
```

For a single-`T` template, both parameters must deduce the same type. Options:

```cpp
larger<double>(1, 2.5);                    // explicit T
larger(static_cast<double>(1), 2.5);        // make arguments match
```

Or write a template with multiple template parameters, covered in 11.8.

### `<>` forces template-only consideration

If a non-template overload also exists:

```cpp
int larger(int a, int b)
{
    return (a < b) ? b : a;
}

template <typename T>
T larger(T a, T b)
{
    return (a < b) ? b : a;
}
```

Calls:

```cpp
larger(1, 2);      // can prefer non-template larger(int, int)
larger<>(1, 2);    // use template argument deduction, templates only
larger<int>(1, 2); // explicitly instantiate/use template
```

Use the normal form unless you specifically need the template version.

### Templates instantiate only when used

A function template definition can exist without generating code for every
possible type.

```cpp
template <typename T>
T doubleValue(T value)
{
    return value + value;
}
```

If the program only calls `doubleValue(3)`, the compiler only needs an `int`
version in that translation unit. It does not generate a version for every type
imaginable.

### Static locals are per instantiation

Each generated function has its own static local variables.

```cpp
template <typename T>
int nextId(T)
{
    static int id { 0 };
    return ++id;
}
```

Conceptually:

```
nextId<int>       has its own static id
nextId<double>    has its own static id
nextId<char>      has its own static id
```

So:

```cpp
nextId(1);    // int id becomes 1
nextId(2);    // int id becomes 2
nextId(3.5);  // double id becomes 1
```

This is useful occasionally, but it can surprise you if you expected one shared
counter.

### Generic programming

Templates enable **generic programming**: writing algorithms in terms of the
operations they need rather than in terms of one concrete type.

```cpp
template <typename T>
bool isInsideClosedRange(T value, T low, T high)
{
    return !(value < low) && !(high < value);
}
```

This works for any `T` where `<` has the expected meaning.

---

## 11.8 — Function templates with multiple template types

### Single-`T` templates require matching argument types

```cpp
template <typename T>
T add(T a, T b)
{
    return a + b;
}

add(1, 2);     // ok: T is int
add(1.0, 2.0); // ok: T is double
add(1, 2.0);   // deduction problem
```

The last call gives the compiler conflicting deductions for `T`.

### Multiple type parameters

Use separate template parameters when arguments can naturally differ.

```cpp
template <typename T, typename U>
auto add(T a, U b)
{
    return a + b;
}
```

Now:

```cpp
add(1, 2.0); // T is int, U is double
```

### Use `auto` return type when result type differs

If the return type is `T`, then `add<int, double>` would return `int`, possibly
discarding the fractional part.

```cpp
template <typename T, typename U>
T badAdd(T a, U b)
{
    return a + b; // result may be narrowed into T
}
```

`auto` lets the compiler deduce the return type from the return expression:

```cpp
template <typename T, typename U>
auto add(T a, U b)
{
    return a + b;
}
```

Important limitation: a function with deduced return type must be defined before
callers use it, because the compiler needs the function body to determine the
return type.

### Abbreviated function templates in C++20

C++20 allows `auto` in normal function parameter lists to create an abbreviated
function template:

```cpp
auto add(auto a, auto b)
{
    return a + b;
}
```

Each `auto` parameter is an independent type parameter.

Use this for small, obvious templates. Use explicit `template <typename T>` style
when the relationship between types matters or the constraints need names.

### Overloading function templates

Function templates can also be overloaded.

```cpp
template <typename T>
T combine(T a, T b)
{
    return a + b;
}

template <typename T, typename U>
auto combine(T a, U b)
{
    return a + b;
}

template <typename T, typename U, typename V>
auto combine(T a, U b, V c)
{
    return a + b + c;
}
```

The compiler chooses among viable function templates. If one template is more
specialized/restrictive, it can be preferred. If no best template exists, the
call is ambiguous.

### Design guideline

Use one type parameter when the arguments should be the same type:

```cpp
template <typename T>
void swapValues(T& a, T& b);
```

Use multiple type parameters when different types are expected:

```cpp
template <typename T, typename U>
auto scaled(T value, U factor);
```

That distinction documents the type relationship.

---

## 11.9 — Non-type template parameters

### Template parameters can be values too

So far, template parameters have represented types:

```cpp
template <typename T>
T identity(T value);
```

A **non-type template parameter** represents a compile-time value.

```cpp
template <int N>
void printN()
{
    std::cout << N << '\n';
}

printN<5>();
```

Here `N` is not a type. It is an `int` value known at compile time.

### Why use non-type template parameters?

Use them when a value must be available as a constant expression inside the
template.

```cpp
template <int Max>
bool isValidIndex(int index)
{
    static_assert(Max > 0, "Max must be positive");
    return index >= 0 && index < Max;
}
```

`Max` can be used in `static_assert` because it is a compile-time value.

### Common non-type parameter kinds

Modern C++ allows several categories, including:

- integral values,
- enumeration values,
- `std::nullptr_t`,
- pointers/references with suitable linkage,
- floating-point values in C++20,
- some literal class values in C++20.

For this course's C++ refresher, the important beginner cases are integral
values and enumeration values.

### `std::array` and `std::bitset`

You already see non-type template parameters in standard library types:

```cpp
#include <array>
#include <bitset>

std::array<int, 5> values {};
std::bitset<8> flags {};
```

In `std::array<int, 5>`:

| Template argument | Kind | Meaning |
|---|---|---|
| `int` | type | element type |
| `5` | non-type value | array length |

### Non-type arguments must be compile-time values

```cpp
int length { 5 };
// std::array<int, length> values {}; // error: length is runtime value

constexpr int fixedLength { 5 };
std::array<int, fixedLength> values {}; // ok
```

If the size is only known at runtime, use `std::vector`.

### `auto` non-type template parameters

C++17 allows the compiler to deduce the non-type parameter type:

```cpp
template <auto Value>
void printConstant()
{
    std::cout << Value << '\n';
}

printConstant<42>();  // Value is int 42
printConstant<'x'>(); // Value is char 'x'
```

This is concise, but can hide the expected kind of value. For beginner code,
`template <int N>` is often clearer when the value is specifically an integer.

### CS6340-style use

Compile-time sizes can make table-shaped code self-checking:

```cpp
template <std::size_t N>
void printCounters(const std::array<int, N>& counters)
{
    for (std::size_t i { 0 }; i < N; ++i)
        std::cout << i << ": " << counters[i] << '\n';
}
```

`N` lets the function know the array length at compile time without hard-coding
it.

---

## 11.10 — Using function templates in multiple files

### Templates need their definitions visible

For an ordinary function, a forward declaration is enough for the compiler to
compile a call. The linker can find the separate definition later.

```cpp
// main.cpp
int addOne(int);

int main()
{
    return addOne(3);
}
```

```cpp
// add.cpp
int addOne(int value)
{
    return value + 1;
}
```

Templates are different because the compiler must see the template definition to
instantiate the needed function.

### Why a template forward declaration is not enough

Problem shape:

```cpp
// main.cpp
template <typename T>
T addOne(T value); // declaration only

int main()
{
    return addOne(3); // needs addOne<int>
}
```

```cpp
// add.cpp
template <typename T>
T addOne(T value)
{
    return value + 1;
}
```

When compiling `main.cpp`, the compiler sees the call `addOne(3)` and needs to
instantiate `addOne<int>`. But it only has the declaration, not the body. It
cannot generate the function.

```
main.cpp compile:
    sees call addOne<int>
    needs template body
    body not visible
    cannot instantiate
```

The separate `add.cpp` may also not instantiate `addOne<int>` if no call in that
translation unit requires it. The linker then has no generated function to link.

### Put template definitions in headers

Typical solution:

```cpp
// add_one.h
#ifndef ADD_ONE_H
#define ADD_ONE_H

template <typename T>
T addOne(T value)
{
    return value + 1;
}

#endif
```

Then include the header wherever the template is used:

```cpp
#include "add_one.h"

int main()
{
    return addOne(3);
}
```

Each translation unit that uses the template sees the full definition and can
instantiate the needed function.

### Why this does not violate ordinary multiple-definition rules

Functions generated from function templates are implicitly inline. Identical
definitions can appear in multiple translation units, and the linker can merge
or select the appropriate instantiations.

Important distinction:

```
template definition in header:
    pattern available to each translation unit

instantiated function:
    generated only for types actually used there
```

### Practical header guideline

For templates:

- define the whole template in the header,
- keep the template body small enough to be readable,
- use include guards or `#pragma once`,
- avoid putting unrelated implementation detail into headers unless templates
  require it.

CS6340 tie-in:

```cpp
// Small generic helpers used by multiple lab files belong in headers.
template <typename Container>
bool emptyAfterTrim(const Container& tokens)
{
    return tokens.empty();
}
```

If a helper is not a template, prefer ordinary `.h` declaration plus `.cpp`
definition.

---

## 11.x — Chapter 11 summary and quiz

### Core takeaways

- Function overloading lets related functions share a name.
- Overloads must differ by parameter count, parameter type, parameter order, or
  certain member-function qualifiers.
- Return type alone does not differentiate overloads.
- Type aliases do not create distinct overloadable types.
- Top-level `const` on by-value parameters does not create a new overload.
- Overload resolution chooses the best function for a call.
- Exact matches beat promotions; promotions beat conversions.
- Ambiguity happens when multiple candidates are equally good.
- `= delete` makes unwanted calls fail at compile time.
- Default arguments are inserted at the call site.
- Defaults must be supplied from right to left; callers cannot skip a middle
  argument.
- Put default arguments in visible declarations, usually headers.
- Function templates are patterns for generating functions.
- Template argument deduction does not perform normal type conversions.
- Multiple template parameters allow mixed argument types.
- `auto` return type is useful when the result type depends on an expression.
- Non-type template parameters pass compile-time values.
- Template definitions generally belong in headers because callers need the body
  to instantiate them.

### Overload-resolution quick table

| Call situation | Likely outcome |
|---|---|
| exact parameter type exists | exact overload wins |
| no exact match, promotion exists | promotion match wins |
| only conversions exist | one conversion wins or ambiguity |
| deleted overload is best | compile error |
| non-template and template both exact | non-template often preferred |
| default arguments make several calls viable | possible ambiguity |

### Template mental model

```
template source code
    |
    | used with int
    v
generated int function

template source code
    |
    | used with double
    v
generated double function
```

Write the template once, but remember that every used type combination creates a
real function with real type-checking.

### CS6340 bridge

This chapter helps decode C++ library and LLVM-style APIs:

- overloads explain why one function name can accept different program objects,
- deleted overloads show how APIs prevent invalid conversions,
- templates explain why `std::vector<T>`, `std::array<T, N>`, and generic helper
  functions work,
- header placement explains why templated helpers usually cannot hide their
  definitions in `.cpp` files.

The main debugging habit: when a function call surprises you, ask two separate
questions:

1. Which overload or template instance did the compiler choose?
2. What conversions or deductions made that choice possible?
