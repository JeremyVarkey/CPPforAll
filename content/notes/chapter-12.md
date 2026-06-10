# Chapter 12 — Compound Types: References and Pointers

> Source: <https://www.learncpp.com/> (Chapter 12)
> One file per chapter. Each lesson is a section below.

## Contents
- [12.1 — Introduction to compound data types](#121--introduction-to-compound-data-types)
- [12.2 — Value categories (lvalues and rvalues)](#122--value-categories-lvalues-and-rvalues)
- [12.3 — Lvalue references](#123--lvalue-references)
- [12.4 — Lvalue references to const](#124--lvalue-references-to-const)
- [12.5 — Pass by lvalue reference](#125--pass-by-lvalue-reference)
- [12.6 — Pass by const lvalue reference](#126--pass-by-const-lvalue-reference)
- [12.7 — Introduction to pointers](#127--introduction-to-pointers)
- [12.8 — Null pointers](#128--null-pointers)
- [12.9 — Pointers and const](#129--pointers-and-const)
- [12.10 — Pass by address](#1210--pass-by-address)
- [12.11 — Pass by address (part 2)](#1211--pass-by-address-part-2)
- [12.12 — Return by reference and return by address](#1212--return-by-reference-and-return-by-address)
- [12.13 — In and out parameters](#1213--in-and-out-parameters)
- [12.14 — Type deduction with pointers, references, and const](#1214--type-deduction-with-pointers-references-and-const)
- [12.15 — std::optional](#1215--stdoptional)
- [12.x — Chapter 12 summary and quiz](#12x--chapter-12-summary-and-quiz)

---

## 12.1 — Introduction to compound data types

### Fundamental vs compound types

A **fundamental type** is built directly into the language:

```cpp
int
double
bool
char
```

A **compound data type** is built from other types.

```cpp
int&        // reference to int
int*        // pointer to int
int[5]      // array of int
std::string // class type
```

Compound types are how C++ models "relationships to values" and "larger shapes
of data."

### Why this chapter matters

References and pointers are not just syntax trivia. They answer:

- Is this function receiving a copy or the original object?
- Can the function modify the caller's object?
- Can this value be missing/null?
- Who controls the lifetime?
- Is this object still alive?

CS6340 tie-in:

```cpp
bool Instrument::runOnFunction(Function &F)
void instrumentCoverage(Module *M, Function &F, Instruction &I)
```

These signatures are impossible to read fluently until references and pointers
click:

| Syntax | Meaning |
|---|---|
| `Function &F` | `F` is another name for an existing `Function` object |
| `Instruction &I` | `I` refers to an existing instruction, not a copy |
| `Module *M` | `M` stores the address of a `Module`; it may need null awareness |

---

## 12.2 — Value categories (lvalues and rvalues)

### Expressions have two important properties

Every expression has:

1. A **type**: what kind of value it produces.
2. A **value category**: how that expression can be used.

```cpp
int x { 5 };
x + 1
```

| Expression | Type | Value category intuition |
|---|---|---|
| `x` | `int` | lvalue: names an object with identity |
| `5` | `int` | rvalue: temporary value |
| `x + 1` | `int` | rvalue: computed result |

### Lvalues

An **lvalue** expression evaluates to an identifiable object or function. It has
an address you can meaningfully take.

```cpp
int x { 5 };
x = 6;       // x is an lvalue; it can appear on left side of assignment
&x;          // can take address
```

### Rvalues

An **rvalue** expression is a temporary value that does not have persistent
identity in the same way.

```cpp
5
x + 1
getValue()
```

You cannot assign to a normal rvalue:

```cpp
5 = x;       // nonsense
(x + 1) = 7; // nonsense
```

### Lvalue-to-rvalue conversion

When an lvalue appears where a value is needed, C++ reads the value from the
object.

```cpp
int x { 5 };
int y { x }; // x expression identifies object; value 5 is read
```

Mental model:

```
x as object identity:  "the box named x"
x as value:           "the contents of x"
```

### Quick test

Usually:

- If you can take its address with `&expr`, it is probably an lvalue.
- If it is a literal or temporary computation, it is probably an rvalue.

```cpp
&x;       // ok
// &(x+1); // not ok for ordinary rvalue
```

This is simplified, but good enough for the reference rules in this chapter.

---

## 12.3 — Lvalue references

### Reference basics

An **lvalue reference** acts as another name for an existing object.

```cpp
int x { 5 };
int& ref { x };

ref = 10; // changes x
```

Diagram:

```
object:        x
memory value:  5
names:         x, ref

after ref = 10:
object:        x
memory value:  10
names:         x, ref
```

The `&` in a type means reference:

```cpp
int& ref { x };
```

Do not confuse it with address-of:

```cpp
int* ptr { &x }; // here &x means "address of x"
```

### References must be initialized

A reference has to bind to something immediately:

```cpp
int& bad;      // error
int& ok { x }; // ok
```

### References cannot be reseated

Once a reference is bound to an object, it stays bound to that object.

```cpp
int x { 1 };
int y { 2 };

int& ref { x };
ref = y;       // assigns y's value into x; does not make ref refer to y
```

Diagram:

```
before:
x = 1   <- ref
y = 2

ref = y;

after:
x = 2   <- ref
y = 2
```

This is one of the most important reference facts.

### References usually bind to matching lvalues

```cpp
int x { 5 };
int& ref { x }; // ok

// int& bad { 5 }; // error: non-const lvalue reference cannot bind rvalue
```

Non-const lvalue references bind to modifiable lvalues.

### Modifying through a reference

```cpp
void addOne(int& value)
{
    ++value;
}

int x { 5 };
addOne(x); // x becomes 6
```

`value` is not a copy. It is a reference to `x`.

### References and lifetimes

The reference and the object it refers to have related but separate lifetimes.
A reference can go out of scope while the original object remains alive:

```cpp
int x { 5 };

{
    int& ref { x };
} // ref gone, x still exists
```

But if the object dies while a reference still exists, the reference dangles.

### Dangling references

A **dangling reference** refers to an object that no longer exists.

```cpp
int& bad()
{
    int local { 5 };
    return local; // bad: local dies when function returns
}
```

Lifetime diagram:

```
function call:
local object: [ alive inside function ] X destroyed
returned ref:                         [ tries to continue ]
                                      ^ dangling
```

Using a dangling reference is undefined behavior.

### References are not objects

LearnCpp emphasizes that references are not objects in the same way variables
are. A reference is an alias. You generally do not manipulate the reference
itself; operations on the reference affect the referent.

---

## 12.4 — Lvalue references to const

### Const reference

An lvalue reference to const cannot be used to modify the object.

```cpp
int x { 5 };
const int& ref { x };

// ref = 6; // error
x = 6;      // ok: x itself is not const
```

The reference is read-only, not necessarily the original object.

### Why const references are useful

They can bind to:

- modifiable lvalues,
- const lvalues,
- rvalues/temporaries,
- sometimes values of different but convertible types.

```cpp
const int& a { x };  // x is lvalue
const int& b { 5 };  // 5 is rvalue; ok
```

A non-const lvalue reference cannot bind to `5`, but a const lvalue reference
can.

### Lifetime extension

When a const reference is directly bound to a temporary, the temporary's lifetime
is extended to match the reference's lifetime.

```cpp
const int& ref { 5 };
std::cout << ref << '\n'; // ok
```

Diagram:

```
temporary 5 would normally die at end of expression
const ref binding extends it:

ref lifetime:       [-------------]
temporary lifetime: [-------------]
```

This rule does not magically solve all lifetime issues, especially across
function boundaries.

### Binding different types

```cpp
double d { 3.14 };
const int& ref { static_cast<int>(d) }; // binds to temporary int
```

If conversion is needed, the reference binds to a temporary converted value, not
the original object.

```
d: double 3.14
conversion creates temporary int 3
ref binds to temporary int 3
```

This can surprise you if you expected the reference to track the original.

---

## 12.5 — Pass by lvalue reference

### Copy cost

Pass by value copies the argument:

```cpp
void print(std::string s); // copies string
```

For large objects, copies can be expensive.

### Pass by reference

```cpp
void printLength(std::string& s)
{
    std::cout << s.length() << '\n';
}
```

This avoids a copy, because `s` refers to the caller's string.

But non-const reference also allows modification:

```cpp
void clear(std::string& s)
{
    s.clear(); // modifies caller's string
}
```

### Can only accept modifiable lvalues

```cpp
std::string name { "Ada" };
clear(name);      // ok

// clear("Ada");  // error: literal is not modifiable std::string lvalue
```

If the function should not modify the argument, use pass by const reference or
`std::string_view`.

CS6340:

```cpp
bool Instrument::runOnFunction(Function &F)
```

`F` is passed by non-const reference because the LLVM pass may inspect and modify
the function's IR.

---

## 12.6 — Pass by const lvalue reference

### Read-only, no-copy parameter

```cpp
void printName(const std::string& name)
{
    std::cout << name << '\n';
}
```

This avoids copying and prevents the function from modifying `name`.

### Different argument types can bind via temporaries

```cpp
void printInt(const int& x);

short s { 5 };
printInt(s); // may bind to temporary int converted from short
```

Be aware: the reference may refer to a temporary conversion, not the original.

### Pass by value vs const reference

General rule:

| Type kind | Prefer |
|---|---|
| Cheap fundamental types (`int`, `double`, `bool`) | pass by value |
| Expensive class types (`std::string`, vectors, large structs) | pass by const reference or view |
| Read-only string-like parameter | `std::string_view` in most cases |

```cpp
void setRetries(int retries);                  // value
void printVector(const std::vector<int>& xs);   // const ref
void log(std::string_view message);             // view
```

### Why `std::string_view` often beats `const std::string&`

`const std::string&` accepts `std::string` cheaply, but a string literal may need
to become a temporary `std::string`.

`std::string_view` can view:

- C-style string literals,
- `std::string`,
- `std::string_view`.

So for read-only string parameters:

```cpp
void log(std::string_view message); // flexible and cheap
```

---

## 12.7 — Introduction to pointers

### Address-of operator

`&object` produces the object's memory address.

```cpp
int x { 5 };
std::cout << &x << '\n';
```

### Pointers

A pointer is an object that stores a memory address.

```cpp
int x { 5 };
int* ptr { &x };
```

Diagram:

```
x:
  address 0x1000
  value   5

ptr:
  value   0x1000
          |
          v
          x
```

### Dereference operator

`*ptr` accesses the object being pointed at.

```cpp
std::cout << *ptr << '\n'; // prints x's value
*ptr = 10;                 // changes x
```

Do not confuse:

```cpp
int* ptr; // * in declaration: ptr is pointer to int
*ptr;     // * in expression: dereference pointer
```

### Initialize pointers

Best practice: initialize pointers.

```cpp
int* ptr {};      // null pointer
int* ptr2 { &x }; // points at x
```

Uninitialized pointers contain garbage addresses. Dereferencing one is undefined
behavior.

### Pointer assignment

Pointers can be reseated:

```cpp
int x { 1 };
int y { 2 };

int* ptr { &x };
ptr = &y; // now points to y
```

This is a key difference from references.

```
reference: cannot reseat
pointer:   can point somewhere else
```

### Pointer size

A pointer's size is the size needed to store an address on the platform, not the
size of the pointed-to type.

```cpp
sizeof(int*)    // likely 8 on 64-bit system
sizeof(double*) // also likely 8
```

### Dangling pointers

A dangling pointer stores the address of an object that no longer exists.

```cpp
int* ptr {};

{
    int x { 5 };
    ptr = &x;
} // x destroyed

// *ptr is undefined behavior
```

After the object dies, the address is stale.

---

## 12.8 — Null pointers

### Null pointer

A null pointer points to nothing.

```cpp
int* ptr {};
```

Use `nullptr` explicitly:

```cpp
int* ptr { nullptr };
```

`nullptr` is the modern null pointer literal. Prefer it over `0` or `NULL`.

### Never dereference null

```cpp
int* ptr { nullptr };
// std::cout << *ptr; // undefined behavior
```

Check before dereferencing:

```cpp
if (ptr != nullptr)
{
    std::cout << *ptr << '\n';
}
```

Or simply:

```cpp
if (ptr)
{
    std::cout << *ptr << '\n';
}
```

### Use nullptr to avoid dangling pointers

If a pointer no longer points to a valid object, set it to null if it remains in
scope.

```cpp
ptr = nullptr;
```

This does not fix ownership by itself, but it makes accidental use easier to
detect than a stale address.

### Prefer references when null is not meaningful

If a parameter must refer to a valid object, prefer reference:

```cpp
void process(Instruction& I); // must exist
```

If "no object" is a meaningful state, pointer may be appropriate:

```cpp
void process(Instruction* I); // caller may pass nullptr
```

Rule:

```
must exist -> reference
may be absent/reseated -> pointer
```

---

## 12.9 — Pointers and const

Const with pointers has two independent questions:

1. Can I modify the pointed-to value through this pointer?
2. Can I change the pointer to point somewhere else?

### Pointer to const value

```cpp
const int value { 5 };
const int* ptr { &value };
```

Read right-ish:

```
ptr is a pointer to const int
```

You cannot modify the int through `ptr`:

```cpp
// *ptr = 6; // error
```

But `ptr` itself can point elsewhere:

```cpp
const int other { 7 };
ptr = &other; // ok
```

### Const pointer

```cpp
int value { 5 };
int* const ptr { &value };
```

`ptr` must always point to the same address, but the pointed-to value can be
modified:

```cpp
*ptr = 6; // ok
// ptr = &other; // error
```

### Const pointer to const value

```cpp
const int value { 5 };
const int* const ptr { &value };
```

Neither the pointee through this pointer nor the pointer address can be changed.

### ASCII const map

```cpp
int* p;              // pointer to int
const int* p;        // pointer to const int
int* const p;        // const pointer to int
const int* const p;  // const pointer to const int
```

Mnemonic:

```
const before *  -> pointed-to value is const through pointer
const after *   -> pointer variable itself is const
```

---

## 12.10 — Pass by address

### Pass by address

Passing by address means passing a pointer:

```cpp
void addOne(int* ptr)
{
    if (ptr)
    {
        ++(*ptr);
    }
}

int x { 5 };
addOne(&x);
```

This avoids copying and can modify the caller's object.

### Null checking

Pointer parameters may be null unless the API clearly forbids it. Check when
null is possible:

```cpp
void print(const std::string* s)
{
    if (s == nullptr)
        return;

    std::cout << *s << '\n';
}
```

### Prefer references for required objects

If a function requires an argument, use reference:

```cpp
void print(const std::string& s); // no null case
```

Use pointer when:

- null has meaning,
- reseating is needed,
- C-style API requires it,
- dynamic allocation/ownership conventions require pointer syntax.

CS6340:

```cpp
Module *M = F.getParent();
```

LLVM APIs often return pointers because object relationships may be represented
by addresses and sometimes absence is possible. Check the API contract.

---

## 12.11 — Pass by address (part 2)

### Optional arguments

Pointer parameters can model optional input:

```cpp
void printMaybe(const std::string* message)
{
    if (message)
        std::cout << *message << '\n';
}

printMaybe(nullptr);
```

In modern C++, `std::optional` may be better for optional values, while pointers
can be better for optional access to an existing object.

### Changing what a pointer parameter points at

Passing a pointer by value copies the pointer:

```cpp
void reseat(int* ptr, int* other)
{
    ptr = other; // changes local copy only
}
```

The caller's pointer is unchanged.

If you need to change the caller's pointer, you need a reference to pointer:

```cpp
void reseat(int*& ptr, int* other)
{
    ptr = other;
}
```

Diagram:

```
pass int*:
  caller ptr ----> x
  function ptr --> x   (copy)
  reseat function ptr only

pass int*&:
  function ref is alias for caller ptr
  reseat changes caller ptr
```

This syntax is advanced-looking but reads as: "reference to a pointer to int."

### There is only pass by value, technically

LearnCpp notes a deeper model: references and pointers are mechanisms that let
functions access caller objects, but at the machine level something address-like
is still passed. The practical API distinction remains:

- pass by value: function gets a copy of the object,
- pass by reference: function parameter aliases the object,
- pass by address: function gets a pointer value storing the object's address.

---

## 12.12 — Return by reference and return by address

### Return by reference

A function can return a reference:

```cpp
int& getElement(std::vector<int>& values, int index)
{
    return values[static_cast<std::size_t>(index)];
}
```

This returns access to an existing object, not a copy.

### The object must outlive the function

Never return a reference to a local variable:

```cpp
int& bad()
{
    int x { 5 };
    return x; // dangling
}
```

Safe cases:

- returning a reference to an object passed in by reference,
- returning a reference to a data member of an object that outlives the call,
- returning a reference to a static object only with care.

### Lifetime extension does not save function returns

This is unsafe:

```cpp
const int& bad()
{
    return 5; // temporary does not get safely extended across return
}
```

### Assigning from returned reference may copy

```cpp
int& ref { getElement(values, 0) }; // ref aliases element
int copy { getElement(values, 0) }; // copy gets element value
```

The receiving variable controls whether you keep a reference or copy the value.

### Return by address

```cpp
int* findValue(std::vector<int>& values, int target)
{
    for (auto& value : values)
    {
        if (value == target)
            return &value;
    }

    return nullptr;
}
```

Use return by address when "not found" is a natural result via `nullptr`.

Use return by reference when a valid object is guaranteed.

---

## 12.13 — In and out parameters

### In parameters

An **in parameter** provides information to a function.

```cpp
void print(std::string_view message);
```

The function reads it.

### Out parameters

An **out parameter** is modified by the function to produce output.

```cpp
void getLineCol(int& line, int& col)
{
    line = 10;
    col = 5;
}
```

The caller sees:

```cpp
int line {};
int col {};
getLineCol(line, col);
```

Problem: the call site does not visually reveal that `line` and `col` will be
modified.

### In/out parameters

An **in/out parameter** is read and modified.

```cpp
void normalize(std::string& text)
{
    // read current text, then modify it
}
```

### Prefer return values when practical

Instead of:

```cpp
void parsePoint(std::string_view text, int& line, int& col);
```

Consider:

```cpp
struct Point
{
    int line {};
    int col {};
};

Point parsePoint(std::string_view text);
```

Return values often make data flow clearer.

### When non-const references are appropriate

Use non-const references when mutation is central to the operation:

```cpp
void mutateInput(std::string& input);
void instrumentInstruction(Instruction& I);
```

Avoid using non-const reference output parameters just to dodge returning a
small value.

---

## 12.14 — Type deduction with pointers, references, and const

### `auto` drops references by default

```cpp
int x { 5 };
int& ref { x };

auto y { ref }; // y is int, a copy
```

If you want a reference:

```cpp
auto& y { ref }; // y aliases x
```

### Top-level vs low-level const

Top-level const applies to the object itself:

```cpp
const int x { 5 };
```

Low-level const applies to something being pointed/referred to:

```cpp
const int* ptr { &x }; // pointer to const int
```

`auto` drops top-level const unless you ask for it:

```cpp
const int x { 5 };
auto a { x };       // int
const auto b { x }; // const int
```

For references:

```cpp
const int& ref { x };
auto a { ref };        // int copy
const auto& b { ref }; // const int& reference
```

### Type deduction and pointers

```cpp
int x { 5 };
int* ptr { &x };

auto p1 { ptr };  // int*
auto* p2 { ptr }; // int*, pointer-ness explicit
```

`auto*` requires the initializer to be a pointer and makes the declaration
visually clearer.

LLVM-style:

```cpp
auto *M { F.getParent() };
```

This means: deduce the `Module` part, but make clear `M` is a pointer.

### Type deduction and const pointers

```cpp
int x { 5 };
int* const constPtr { &x };

auto p { constPtr }; // int*; top-level const on pointer dropped
```

If you need the pointer itself const:

```cpp
auto* const p { constPtr };
```

If the pointed-to value is const:

```cpp
const int y { 5 };
auto p { &y }; // const int*
```

Low-level const is preserved because it is part of the pointed-to type.

### Summary table

| Declaration | Deduces / means |
|---|---|
| `auto x { ref };` | copy of referenced value |
| `auto& x { ref };` | reference |
| `const auto& x { expr };` | const reference, no copy |
| `auto p { ptr };` | pointer type deduced |
| `auto* p { ptr };` | pointer required and explicit |
| `const auto* p { ptr };` | pointer to const value |
| `auto* const p { ptr };` | const pointer |

---

## 12.15 — std::optional

### The problem: maybe no value

Sometimes a function may or may not produce a value.

Bad sentinel approach:

```cpp
int findIndex(...); // returns -1 if not found
```

The caller must remember that `-1` is special.

### `std::optional`

`std::optional<T>` represents either:

- a `T` value,
- or no value.

```cpp
#include <optional>

std::optional<int> findIndex(std::string_view text, char target)
{
    for (int i { 0 }; i < static_cast<int>(text.length()); ++i)
    {
        if (text[static_cast<std::size_t>(i)] == target)
            return i;
    }

    return std::nullopt;
}
```

Using it:

```cpp
auto index { findIndex("abc", 'b') };

if (index.has_value())
{
    std::cout << *index << '\n';
}
```

Shortcut:

```cpp
if (index)
{
    std::cout << *index << '\n';
}
```

### Pros and cons

Pros:

- no magic sentinel,
- absence is explicit in the type,
- caller must handle maybe-value.

Cons:

- a little more syntax,
- not ideal for every output shape,
- optional references are not part of standard `std::optional` in current common
  C++ usage.

### Optional parameters

LearnCpp discusses optional as a possible optional function parameter. In many
cases, overloads or default arguments are cleaner. Use optional when "a value may
or may not be supplied" is truly part of the model.

CS6340 tie-in:

```cpp
std::optional<CoveragePoint> parseCoverageLine(std::string_view line);
```

This is clearer than returning `{0, 0}` and hoping callers know it means parse
failure.

---

## 12.x — Chapter 12 summary and quiz

### Core takeaways

- Compound types are built from other types.
- Lvalues identify objects; rvalues are temporary/computed values.
- References are aliases and must be initialized.
- Non-const lvalue references bind to modifiable lvalues.
- References cannot be reseated.
- Const lvalue references can bind to temporaries and extend their lifetime in
  direct binding contexts.
- Pass by reference avoids copies and can allow modification.
- Pass by const reference avoids copies and prevents modification.
- Prefer `std::string_view` for read-only string parameters.
- Pointers store addresses and can be reseated.
- Dereferencing invalid, dangling, or null pointers is undefined behavior.
- Use `nullptr`, not `0` or `NULL`.
- Prefer references when an object must exist; use pointers when null/reseating
  is meaningful.
- `const int*`, `int* const`, and `const int* const` mean different things.
- Do not return references/pointers to local variables.
- Prefer return values over out parameters when practical.
- `auto` drops references and top-level const unless you preserve them.
- `std::optional<T>` models "maybe a T" explicitly.

### Reference vs pointer quick table

| Question | Reference | Pointer |
|---|---|---|
| Must be initialized? | Yes | No, but should be |
| Can be null? | No ordinary null reference | Yes, `nullptr` |
| Can be reseated? | No | Yes |
| Access syntax | `ref.member` | `ptr->member` or `(*ptr).member` |
| Best for | required object | optional/reseatable object |

### CS6340 decoder

```cpp
bool Instrument::runOnFunction(Function &F)
```

`F` is an existing LLVM function. The pass can inspect or modify it.

```cpp
Module *M = F.getParent();
```

`M` stores the address of the parent module. Use `M->member` to access members.

```cpp
Instruction &I
```

`I` is an alias for an existing instruction. No instruction copy is being made.

### Mini drill

```cpp
#include <iostream>
#include <optional>
#include <string>
#include <string_view>

std::optional<int> findChar(std::string_view input, char target)
{
    for (int i { 0 }; i < static_cast<int>(input.length()); ++i)
    {
        if (input[static_cast<std::size_t>(i)] == target)
            return i;
    }

    return std::nullopt;
}

void replaceAt(std::string& input, int index, char replacement)
{
    input[static_cast<std::size_t>(index)] = replacement;
}

int main()
{
    std::string mutant { "abcde" };

    if (auto index { findChar(mutant, 'c') })
    {
        replaceAt(mutant, *index, 'X');
    }

    std::cout << mutant << '\n';
}
```

What this reinforces:

- `std::string_view` for read-only search input.
- `std::optional<int>` for maybe-found index.
- `std::string&` for intentional mutation.
- `static_cast<std::size_t>` at the indexing boundary.
