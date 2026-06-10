# Chapter 12 — Compound Types: References and Pointers

> Source: <https://www.learncpp.com/> (Chapter 12)

Up to now, every function you've written has lived in a kind of bubble. When you passed an argument, the function received a *copy*; whatever it did to that copy stayed inside the function and vanished when the function returned. That's safe and simple, but it has limits. Copies of large objects are expensive. A function that should hand back two results has nowhere to put the second one. And sometimes you genuinely *want* a function to reach back out and change the caller's variable — and copies make that impossible.

This chapter introduces the two C++ features that break the bubble: **references** and **pointers**. Both let a second name refer to the *same* object in memory, so a function can read and write the caller's data directly, with no copy involved. They sound similar, and they overlap, but they answer different questions. A reference is a permanent alias — once it names an object, it names that object forever, and it can never name "nothing." A pointer is a variable that *holds an address*; it can be reaimed, it can be deliberately empty (`nullptr`), and it forces you to think about whether the thing it points at still exists.

Getting these right is where C++ starts to feel like C++. It's also the gateway to reading real systems code. The CS6340 LLVM signatures you'll meet — `bool runOnFunction(Function &F)`, `Module *M { F.getParent() }` — are gibberish until references and pointers click, and then they read like plain English. By the end of this chapter you'll know exactly when to reach for each, how to pass and return them safely, how `const` interacts with both, and how `std::optional` lets a function say "maybe nothing" without resorting to a magic sentinel value.

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

### Fundamental types versus compound types

So far, the types you've used have all been **fundamental types** — the ones built directly into the language, with no assembly required:

```cpp
int
double
bool
char
```

Each names a single, self-contained kind of value. This chapter introduces **compound data types**: types that are *built out of* other types. You've actually met one already — `std::string` is a compound type — but now we'll look at the family systematically.

```cpp
int&        // reference to int
int*        // pointer to int
int[5]      // array of int
std::string // class type
```

Read each of those as "a type made from `int`" (or, for `std::string`, from `char`). Compound types are how C++ expresses *relationships* between values and *larger shapes* of data — not just "a number," but "another name for that number," or "the address where that number lives," or "five of those numbers in a row."

### Why this chapter matters

References and pointers are not syntax trivia to be memorized and forgotten. They're the vocabulary C++ uses to answer questions that come up constantly in real code:

- Is this function getting a *copy* of my object, or the *original*?
- Is this function allowed to *modify* my object?
- Can this value legitimately be *missing*?
- Who is responsible for keeping this object *alive*?
- Is the object this thing refers to *still alive*?

Those questions have life-or-death consequences in a large program, and a function's signature is where the answers are encoded. Consider these two LLVM signatures from CS6340:

```cpp
bool Instrument::runOnFunction(Function &F);
void instrumentCoverage(Module *M, Function &F, Instruction &I);
```

| Syntax | What it tells you |
|---|---|
| `Function &F` | `F` is another name for an existing `Function` — no copy, modifiable |
| `Instruction &I` | `I` refers to a real instruction, not a duplicate of one |
| `Module *M` | `M` holds the *address* of a `Module`; you may have to check it isn't null |

Right now those distinctions might look like noise. By the end of the chapter they'll be the first thing you read off a signature.

---

## 12.2 — Value categories (lvalues and rvalues)

Before we can talk about references, we need a small piece of theory that makes the rest of the chapter make sense. It explains why `int& ref { x };` is fine but `int& ref { 5 };` is an error — a rule that otherwise looks arbitrary.

### Every expression has two properties

You already know that every expression has a **type** — `x + 1` produces an `int`, `3.0 / 2.0` produces a `double`. What's new is that every expression *also* has a **value category**: a classification of *how the expression can be used*, separate from what type it produces.

```cpp
int x { 5 };
x + 1;
```

| Expression | Type | Value category, intuitively |
|---|---|---|
| `x` | `int` | an **lvalue** — names an object that has an identity |
| `5` | `int` | an **rvalue** — a temporary value, no lasting identity |
| `x + 1` | `int` | an **rvalue** — a computed result |

### Lvalues

An **lvalue** expression evaluates to an object (or function) that has an *identity* — a specific, addressable home in memory that persists. Because it has a home, you can put it on the left of an assignment, and you can take its address.

```cpp
int x { 5 };
x = 6;   // x is an lvalue: it can appear on the left of =
&x;      // and you can take its address
```

The name "lvalue" comes from "appears on the **l**eft of an assignment" — a useful mnemonic, even though the modern definition is about identity rather than position.

### Rvalues

An **rvalue** expression is a temporary value with no persistent identity. Literals and the results of computations are rvalues: they exist just long enough to be used, then they're gone.

```cpp
5;          // a literal: rvalue
x + 1;      // a computed result: rvalue
getValue(); // the value a function hands back: rvalue
```

Because an rvalue has no lasting home, you can't assign to one — there's nowhere to store the result:

```cpp
5 = x;       // nonsense: you can't assign into a literal
(x + 1) = 7; // nonsense: x + 1 isn't a place
```

### Lvalue-to-rvalue conversion

Here's the subtlety that ties it together. The *same* lvalue expression can be used in two ways: as the *object* it identifies, or as the *value* currently stored in that object. When an lvalue appears in a context that needs a value, C++ quietly reads the value out of the object — an **lvalue-to-rvalue conversion**.

```cpp
int x { 5 };
int y { x }; // the expression x identifies an object; its value 5 is read out
```

A mental model that helps:

```
x used as an object:  "the box labeled x"
x used as a value:    "the 5 currently inside the box"
```

> **Tip:** A rough-and-ready test: if you can take its address with `&expr`, it's probably an lvalue; if it's a literal or a freshly computed result, it's probably an rvalue. This isn't the full story — value categories get richer in later chapters — but it's exactly enough to understand the reference rules coming up.

---

## 12.3 — Lvalue references

### A reference is an alias

An **lvalue reference** is, quite literally, another name for an existing object. After you bind a reference to an object, the reference and the object are interchangeable: anything you do through the reference happens to the object.

```cpp
int x { 5 };
int& ref { x }; // ref is now another name for x

ref = 10; // this changes x — ref and x are the same object
```

Picture it like this. There is *one* box in memory, with *two* labels on it:

```
object:   x
value:    5
labels:   x, ref

after ref = 10:

object:   x
value:    10
labels:   x, ref
```

The `&` in a *type* is what makes a reference. Don't confuse it with the address-of operator (which we'll meet shortly):

```cpp
int& ref { x };  // & in a type:       "ref is a reference to int"
int* ptr { &x }; // & in an expression: "&x means address of x"
```

Same symbol, two completely different jobs, told apart by context.

### References must be initialized

A reference has nothing of its own to store — it's purely an alias — so it *must* be bound to an object the moment it's created. There's no such thing as a reference that refers to nothing.

```cpp
int x { 5 };
int& bad;      // error: a reference must be initialized
int& ok { x }; // fine
```

### References cannot be reseated

This is one of the most important — and most surprising — facts about references. Once a reference is bound to an object, it stays bound to *that* object for its entire life. There is no way to make it refer to a different object later. So what does assignment through a reference do? It assigns to the *referent*.

```cpp
int x { 1 };
int y { 2 };

int& ref { x };
ref = y; // does NOT make ref refer to y!
         // it copies y's value (2) into x
```

Trace it carefully:

```
before:
x = 1   <- ref
y = 2

ref = y;   // reads y's value, writes it into x

after:
x = 2   <- ref   (ref still aliases x)
y = 2
```

> **Key insight:** `ref = y` always means "write `y`'s value into whatever `ref` refers to." It never means "make `ref` refer to `y`." References are bound once and never move. Pointers, as you'll see, are the opposite.

### References bind to matching modifiable lvalues

A plain (non-const) lvalue reference can only bind to a *modifiable lvalue* of the same type. This is where the lvalue/rvalue theory pays off:

```cpp
int x { 5 };
int& ref { x }; // ok: x is a modifiable lvalue

int& bad { 5 }; // error: 5 is an rvalue, not an lvalue
```

The rule exists for a reason. If `int& bad { 5 }` were allowed, you'd have a *modifiable* alias to a temporary `5` — and writing through it would change a value with no real home. C++ forbids it. (A `const` reference relaxes this, for a reason we'll see in 12.4.)

### Modifying through a reference

Bound to a function parameter, a reference becomes a way for the function to edit the caller's variable:

```cpp
void addOne(int& value)
{
    ++value; // value aliases the caller's variable
}

int x { 5 };
addOne(x); // x is now 6
```

`value` isn't a copy of `x`; it *is* `x`, under another name. This is the foundation of pass-by-reference, which we'll explore properly in 12.5.

### References and lifetimes

A reference and the object it refers to have *related but separate* lifetimes. A reference can go out of scope while its object lives on:

```cpp
int x { 5 };
{
    int& ref { x };
} // ref is gone here; x is still perfectly alive
```

The dangerous case is the reverse: the object dies while a reference to it still exists.

### Dangling references

A **dangling reference** is a reference to an object that no longer exists. The classic way to create one is to return a reference to a local variable:

```cpp
int& bad()
{
    int local { 5 };
    return local; // BUG: local is destroyed when bad() returns
}
```

When the function returns, `local` ceases to exist, but the caller is handed a reference that still "points" at where it used to be:

```
inside bad():   local  [ alive ]  X destroyed at return
returned ref:                     [ still trying to refer ]  <- dangling
```

Using a dangling reference is **undefined behavior** — the program may print garbage, may appear to work, or may crash, all unpredictably. We'll return to this trap in 12.12; it's the single most common reference mistake.

> **Warning:** Never return a reference to a local variable. The local dies at the end of the function, and the reference you hand back dangles. The lab's `maxOf` task is built around exactly this trap.

### References are not objects

One last framing that LearnCpp emphasizes: a reference is *not an object* the way a variable is. It has no storage you manipulate directly — it's a name, an alias. You don't "change the reference"; every operation you write through a reference lands on the referent. Keep that in mind and references stop being mysterious.

---

## 12.4 — Lvalue references to const

### A read-only alias

A plain `int&` lets you both read *and* write the object. Often you want an alias that can only *read* — you want the no-copy convenience of a reference without granting permission to modify. That's an **lvalue reference to const**:

```cpp
int x { 5 };
const int& ref { x };

ref = 6; // error: can't modify through a const reference
x = 6;   // fine: x itself isn't const, so other names can still write it
```

Read the type as "reference to `const int`." Note the asymmetry: it's the *reference* that's read-only, not necessarily the object. `x` is still a modifiable variable; `ref` simply isn't allowed to be the one doing the modifying.

### Const references are more flexible about what they'll bind to

Here's the payoff that makes const references so common. A non-const reference is picky — it only binds to modifiable lvalues. A const reference will happily bind to:

- modifiable lvalues,
- const lvalues,
- and **rvalues / temporaries**.

```cpp
int x { 5 };
const int& a { x }; // binds to an lvalue
const int& b { 5 }; // binds to an rvalue — and this is allowed!
```

Why is `const int& b { 5 }` legal when `int& b { 5 }` was an error? Because the danger in the non-const case was that you might *write* through the alias to a temporary. A const reference can't write, so that danger is gone, and the binding is safe.

### Lifetime extension

There's a neat rule that makes the temporary case actually useful. When a const reference is bound *directly* to a temporary, the temporary's lifetime is **extended** to match the reference's lifetime. Normally a temporary `5` would be destroyed at the end of the expression that created it — but the binding keeps it alive as long as `ref` lives:

```cpp
const int& ref { 5 };
std::cout << ref << '\n'; // ok: the temporary 5 is still alive
```

```
without binding:  temporary 5 dies at end of expression
with const ref:   ref lifetime       [---------------]
                  temporary lifetime [---------------]  (extended to match)
```

> **Warning:** Lifetime extension only works for a temporary bound *directly* to a const reference in the same scope. It does **not** rescue a reference returned from a function — that temporary is already gone by the time the caller sees it. We'll hit this limit again in 12.12.

### Binding through a conversion

One subtlety to file away: if binding a const reference requires a *type conversion*, the reference binds to a freshly created temporary holding the converted value — not to the original object.

```cpp
double d { 3.14 };
const int& ref { static_cast<int>(d) }; // binds to a temporary int (value 3)
```

```
d:    double 3.14
conversion creates temporary int 3
ref binds to that temporary int 3   (not to d)
```

So `ref` is `3`, and changing `d` later would not change `ref`. This can surprise you if you expected the reference to track the original — it's tracking a converted copy instead.

---

## 12.5 — Pass by lvalue reference

### The cost of copying

When you pass an argument **by value**, the function gets a copy:

```cpp
void print(std::string s); // s is a copy of the caller's string
```

For an `int` that's nothing. For a `std::string`, a `std::vector`, or a big struct, that copy means allocating and duplicating potentially a lot of data on *every single call*. That's wasteful when the function only needs to look at the data.

### Passing by reference avoids the copy

Make the parameter a reference and no copy happens — the parameter simply aliases the caller's object:

```cpp
void printLength(std::string& s)
{
    std::cout << s.length() << '\n'; // reads the caller's string directly
}
```

The flip side of "no copy" is "real access": a non-const reference parameter can also *modify* the caller's object.

```cpp
void clear(std::string& s)
{
    s.clear(); // empties the CALLER's string, not a copy
}
```

That's a feature when you want it (this is pass-by-reference's whole point) and a footgun when you don't — which is why the const version in 12.6 exists.

### A non-const reference only accepts modifiable lvalues

Because the function might write through it, a non-const reference parameter can only be called with a modifiable lvalue argument — the same binding rule as 12.3, now at a call site:

```cpp
std::string name { "Ada" };
clear(name);     // ok: name is a modifiable lvalue

clear("Ada");    // error: a string literal is not a modifiable std::string lvalue
```

If your function isn't supposed to change the argument, don't use a non-const reference — reach for a const reference (12.6) or `std::string_view`.

This is also why so many LLVM passes take non-const references:

```cpp
bool Instrument::runOnFunction(Function &F);
```

`F` is passed by non-const reference precisely because the pass needs to both *inspect* and *modify* the function's IR in place. The signature is telling you: this function will change the thing you hand it.

---

## 12.6 — Pass by const lvalue reference

### The best of both worlds for read-only parameters

Combine the two ideas from the last sections — pass by reference (no copy) and const (no modification) — and you get the single most common parameter idiom in C++ for non-trivial types:

```cpp
void printName(const std::string& name)
{
    std::cout << name << '\n'; // can read, cannot modify, did not copy
}
```

This avoids the copy *and* guarantees to the caller that their string won't be touched. When in doubt about how to take a class-type parameter you only need to read, this is the default.

### Different argument types bind through temporaries

Just like in 12.4, a const reference parameter can bind to an argument that needs converting — by binding to a temporary:

```cpp
void printInt(const int& x);

short s { 5 };
printInt(s); // s is converted to a temporary int; x binds to that temporary
```

Worth remembering when the types don't match exactly: the reference may be aliasing a converted temporary, not your original variable.

### Pass by value or by const reference?

A practical decision guide:

| Kind of type | Prefer |
|---|---|
| Cheap fundamental types (`int`, `double`, `bool`, `char`) | pass by value |
| Expensive class types (`std::string`, vectors, large structs) | pass by const reference (or a view) |
| Read-only, string-like parameter | `std::string_view` in most cases |

```cpp
void setRetries(int retries);                 // small + cheap: by value
void printVector(const std::vector<int>& xs); // large: by const reference
void log(std::string_view message);           // read-only string: by view
```

For a cheap type, a reference would just add a layer of indirection for no benefit — copying an `int` is as cheap as referring to one. Save references for types where the copy actually costs something.

### Why `std::string_view` often beats `const std::string&`

You met `std::string_view` back in Chapter 5; here's where it shines. `const std::string&` is fine when the caller already has a `std::string` — but if they pass a string *literal*, the compiler must first build a temporary `std::string` (an allocation!) just to bind the reference.

`std::string_view` sidesteps that. It can cheaply view, with no allocation:

- a C-style string literal,
- a `std::string`,
- another `std::string_view`.

```cpp
void log(std::string_view message); // accepts all three, copies none of them
```

> **Best practice:** For a read-only string parameter, prefer `std::string_view`. It accepts more kinds of arguments than `const std::string&` and avoids surprise allocations from string literals. The lab uses exactly this for its `findFirst` search input.

---

## 12.7 — Introduction to pointers

References are an alias *baked in* at compile time — convenient, but rigid: no null, no reseating. Pointers trade that rigidity for power. A pointer is a *variable that stores an address*, and because it's a real object with its own storage, you can change which address it holds, ask whether it holds a valid one, and pass it around. Let's build it up from the address-of operator.

### The address-of operator

Every object lives somewhere in memory, at some address. The `&` operator, applied to an object, gives you that address:

```cpp
int x { 5 };
std::cout << &x << '\n'; // prints something like 0x7ffd...
```

### A pointer stores an address

A **pointer** is an object whose value is a memory address. You declare one with `*` in the type, and you typically initialize it with the address of something:

```cpp
int x { 5 };
int* ptr { &x }; // ptr holds the address of x
```

```
x:
  address 0x1000
  value   5

ptr:
  value   0x1000  ──┐
                    ▼
                    x
```

`ptr` doesn't contain `5` — it contains *where to find* `5`.

### The dereference operator

To go from the address back to the object it points at, you **dereference** with `*`:

```cpp
std::cout << *ptr << '\n'; // follows ptr to x, prints 5
*ptr = 10;                 // writes through ptr into x; x is now 10
```

The `*` symbol, like `&`, does double duty — tell them apart by where they appear:

```cpp
int* ptr; // * in a declaration: "ptr is a pointer to int"
*ptr;     // * in an expression:  "dereference ptr to reach its object"
```

### Always initialize your pointers

An uninitialized pointer holds a *garbage* address — some leftover bit pattern. Dereferencing it is undefined behavior, and a nasty kind, because it may not crash predictably. So always initialize:

```cpp
int* ptr {};       // value-initialized to NULL (points at nothing) — safe
int* ptr2 { &x };  // points at x
```

> **Best practice:** Always initialize a pointer. If you don't have a real address for it yet, initialize it to null (`{}` or `nullptr`) so it's clearly "points at nothing" rather than "points at garbage."

### Pointers can be reseated

Here is the headline difference from references. A pointer is a variable, so you can *assign a new address to it* — make it point somewhere else entirely:

```cpp
int x { 1 };
int y { 2 };

int* ptr { &x }; // points at x
ptr = &y;        // now points at y
```

```
reference: bound once, can never be reseated
pointer:   can be pointed somewhere else any time
```

Contrast this with `ref = y` from 12.3, which wrote *into* the referent. With a pointer, `ptr = &y` changes *the pointer itself*; the old object `x` is untouched. That difference — reseatable vs. fixed — is the heart of choosing between them.

### How big is a pointer?

A pointer stores an *address*, and an address is the same size regardless of what type it points at. So `sizeof` a pointer reflects the platform's address size, not the pointed-to type:

```cpp
sizeof(int*);    // typically 8 on a 64-bit system
sizeof(double*); // also typically 8 — same address size
```

### Dangling pointers

Pointers have the same lifetime hazard references do. A **dangling pointer** holds the address of an object that has since been destroyed:

```cpp
int* ptr {};
{
    int x { 5 };
    ptr = &x; // ptr points at x
} // x is destroyed here; ptr now holds a stale address

// *ptr is undefined behavior — x no longer exists
```

The address is still *there* in `ptr`, but what lived at that address is gone. Dereferencing a dangling pointer is undefined behavior, exactly like a dangling reference. The next section's `nullptr` gives us one tool to make this kind of mistake easier to catch.

---

## 12.8 — Null pointers

### A pointer that points at nothing

Unlike a reference, a pointer is allowed to point at *nothing*. Such a pointer is a **null pointer**. Value-initializing a pointer makes it null:

```cpp
int* ptr {}; // null pointer
```

Better still, say it explicitly with the keyword `nullptr`:

```cpp
int* ptr { nullptr };
```

`nullptr` is the modern, type-safe null pointer literal.

> **Best practice:** Use `nullptr` to represent a null pointer, not the integer `0` or the old `NULL` macro. `nullptr` is unambiguous — it's *only* ever a null pointer — and it interacts correctly with overload resolution and templates.

### Never dereference a null pointer

A null pointer points at nothing, so there's nothing to dereference. Doing it anyway is undefined behavior:

```cpp
int* ptr { nullptr };
std::cout << *ptr; // undefined behavior — there's no object there
```

The fix is to *check before you dereference*:

```cpp
if (ptr != nullptr)
{
    std::cout << *ptr << '\n';
}
```

A pointer converts to `bool` — null is `false`, non-null is `true` — so the idiomatic short form is:

```cpp
if (ptr)
{
    std::cout << *ptr << '\n'; // only runs when ptr is non-null
}
```

> **Best practice:** Always null-check a pointer before dereferencing it, unless you can *prove* from the surrounding code that it can't be null. The lab's `swapByPtr` and `describePointer` tasks both hinge on a `if (!ptr) ...` guard placed *before* any dereference.

### Null out a pointer instead of leaving it dangling

A dangling pointer and a null pointer are both "invalid to dereference," but they differ in one crucial way: you can *test* for null, and you can't reliably test for dangling. So when a pointer's target goes away but the pointer itself stays in scope, setting it to null turns a silent landmine into something a guard can catch:

```cpp
ptr = nullptr; // now `if (ptr)` will correctly skip it
```

This doesn't solve *ownership* — it won't free anything or fix who's responsible for the object — but it makes accidental use detectable rather than catastrophic.

### Reference when it must exist; pointer when it might not

This is the design rule that decides between the two for the rest of your C++ life:

```cpp
void process(Instruction& I); // I MUST refer to a real instruction
void process(Instruction* I); // I MIGHT be nullptr — "no instruction" is allowed
```

A reference *cannot* be null, so a reference parameter is a promise: "you must give me a real object." A pointer parameter says: "an object, or nothing — I'll handle both."

```
must exist            -> reference
may be absent / reseatable -> pointer
```

---

## 12.9 — Pointers and const

`const` and pointers combine in a way that trips up nearly everyone at first, because there are *two independent things* a `const` could be protecting. Whenever you see `const` near a pointer, ask two separate questions:

1. Can I modify the **pointed-to value** through this pointer?
2. Can I change **the pointer itself** to point somewhere else?

These are independent, so there are four combinations. Let's take them one at a time.

### Pointer to const value

```cpp
const int value { 5 };
const int* ptr { &value };
```

Read it as "`ptr` is a pointer to `const int`." You can't modify the value through it, but you *can* reaim the pointer:

```cpp
*ptr = 6;        // error: can't modify a const int through the pointer

const int other { 7 };
ptr = &other;    // ok: the pointer itself isn't const, so it can be reseated
```

Note that a pointer-to-const can point at a *non*-const object too — it just promises not to modify it *through this pointer*. It's a read-only window onto whatever it's aimed at.

### Const pointer

Swap which side of `*` the `const` is on, and you swap which thing is protected:

```cpp
int value { 5 };
int* const ptr { &value };
```

Now `ptr` is a **const pointer**: it's locked onto one address forever (like a reference, in that respect), but the value it points at is fair game:

```cpp
*ptr = 6;     // ok: the pointed-to int is not const
ptr = &other; // error: ptr itself is const — it can't be reseated
```

### Const pointer to const value

Put `const` on both sides and you lock everything down:

```cpp
const int value { 5 };
const int* const ptr { &value };
```

You can neither modify the value through `ptr` nor reaim `ptr`.

### Reading the declarations

```cpp
int* p;             // pointer to int
const int* p;       // pointer to const int        (value is read-only here)
int* const p;       // const pointer to int        (pointer is locked)
const int* const p; // const pointer to const int  (both locked)
```

> **Tip:** The mnemonic is about *which side of the `*` the `const` sits on*. `const` **before** the `*` protects the pointed-to value; `const` **after** the `*` protects the pointer itself. "Pointer to const" vs. "const pointer" — the word order in English matches the C++.

A `const int*` is exactly what you want for a read-only pointer parameter — the lab's `describePointer(const int* ptr)` uses it to promise it won't modify the int it's handed.

---

## 12.10 — Pass by address

### Passing a pointer to a function

**Pass by address** just means passing a pointer as an argument. The function receives the address, and can dereference it to reach (and optionally modify) the caller's object:

```cpp
void addOne(int* ptr)
{
    if (ptr)        // guard: only proceed if non-null
    {
        ++(*ptr);   // dereference, then increment the caller's int
    }
}

int x { 5 };
addOne(&x); // pass the ADDRESS of x; x becomes 6
```

Like pass-by-reference, this avoids copying the object and lets the function modify the caller's data. Unlike pass-by-reference, the caller must explicitly write `&x`, and the function must cope with the possibility of `nullptr`.

### Null-checking pointer parameters

A pointer parameter can be null unless the function's contract clearly forbids it. So when null is possible, guard against it before dereferencing:

```cpp
void print(const std::string* s)
{
    if (s == nullptr)
        return; // nothing to print

    std::cout << *s << '\n'; // safe: we know s is non-null here
}
```

### Prefer references for required arguments

If a function *requires* its argument to exist, a reference says so more clearly and removes the null case entirely:

```cpp
void print(const std::string& s); // no null possible, no check needed
```

Reach for a pointer parameter when one of these is genuinely true:

- "no object" (`nullptr`) is a meaningful state,
- the function needs to reseat or store the pointer,
- a C-style API requires it,
- ownership/allocation conventions are expressed with pointers.

This is why LLVM hands you pointers in places like:

```cpp
Module *M = F.getParent();
```

The relationship "the module that contains this function" is modeled as an address, and depending on the API the answer could be absent — so a pointer, with its null option, is the natural fit. Always read the API's contract to learn whether a returned pointer can be null.

---

## 12.11 — Pass by address (part 2)

### Modeling an optional argument

A pointer parameter can express "you may give me this, or not," using `nullptr` for "not":

```cpp
void printMaybe(const std::string* message)
{
    if (message)
        std::cout << *message << '\n';
}

printMaybe(nullptr); // perfectly fine: prints nothing
```

In modern C++, `std::optional` (12.15) is often the clearer choice for an optional *value*, while a pointer remains a good fit for optional *access to an existing object* (where you don't want to copy it).

### Changing what a pointer parameter points at

Here's a subtlety that catches people. A pointer passed by value is itself *copied* — the function gets its own copy of the address. So reaiming that copy doesn't touch the caller's pointer:

```cpp
void reseat(int* ptr, int* other)
{
    ptr = other; // changes only the function's local copy of the pointer
}
```

The caller's pointer is exactly where it was. If you actually need to change the *caller's pointer*, you must pass a **reference to a pointer**:

```cpp
void reseat(int*& ptr, int* other)
{
    ptr = other; // ptr now aliases the caller's pointer, so this sticks
}
```

```
pass int* (by value):
  caller ptr ──> x
  local  ptr ──> x   (a copy)
  reseating the local changes nothing for the caller

pass int*& (reference to pointer):
  the parameter is an alias for the caller's pointer
  reseating it changes the caller's pointer
```

Read `int*&` right to left: "a reference (`&`) to a pointer (`*`) to int." It looks exotic, but it's just the by-reference rule applied to a pointer instead of an `int`.

### "Everything is pass by value," really

LearnCpp points out a deeper truth worth knowing: at the machine level, C++ only ever really passes *values* — pass-by-reference and pass-by-address are mechanisms layered on top, both of which boil down to handing the function something address-like. You don't need this to use the features, but it demystifies them. The practical, API-level distinction is what matters day to day:

- **pass by value** — the function gets a copy of the object;
- **pass by reference** — the parameter is an alias for the caller's object;
- **pass by address** — the function gets a pointer holding the object's address.

---

## 12.12 — Return by reference and return by address

We've passed references and pointers *into* functions. You can also return them *out* — but now lifetime becomes a sharp edge, because the function is ending right as you hand something back.

### Return by reference

A function can return a reference, which gives the caller direct access to an existing object rather than a copy of it:

```cpp
int& getElement(std::vector<int>& values, int index)
{
    return values[static_cast<std::size_t>(index)];
}
```

Because this returns a reference into the caller's own vector, the caller can even *write* through the returned reference and modify the element in place.

### The returned object must outlive the function

The iron rule: whatever you return a reference to must still exist *after* the function returns. Returning a reference to a local is the cardinal sin:

```cpp
int& bad()
{
    int x { 5 };
    return x; // BUG: x is destroyed when bad() returns — dangling reference
}
```

The cases that are *safe* all share one property — the referent outlives the call:

- returning a reference to an object that was passed *in* by reference,
- returning a reference to a member of an object that outlives the call,
- returning a reference to a `static` object (only with care).

This is precisely the lab's `maxOf` trap. Returning a reference to one of the *parameters* is safe — the parameters alias the caller's variables, which outlive the call. Returning a reference to a fresh local would dangle.

### Lifetime extension does not save returns

You might hope the lifetime-extension rule from 12.4 rescues this. It does not — extension only applies to a temporary bound directly to a const reference in the same scope, not to one handed back across a `return`:

```cpp
const int& bad()
{
    return 5; // BUG: the temporary is gone before the caller can use it
}
```

> **Warning:** Returning a reference (or pointer) to a local variable or to a temporary is undefined behavior. The temporary's lifetime is *not* extended across the return. If you need to return a brand-new value, return *by value*.

### The receiver decides: alias or copy

When you call a reference-returning function, what you assign the result *to* decides whether you keep an alias or take a copy:

```cpp
int& ref { getElement(values, 0) };  // ref aliases the element — write-through works
int copy { getElement(values, 0) };  // copy is an independent copy of the value
```

Both are valid; they just mean different things. Initialize a reference and you stay connected to the original; initialize a plain `int` and you snapshot its value.

### Return by address

Returning a *pointer* works the same way, with one extra ability: a pointer can be `nullptr`, so it can express "no result found":

```cpp
int* findValue(std::vector<int>& values, int target)
{
    for (auto& value : values)
    {
        if (value == target)
            return &value; // found: return its address
    }

    return nullptr; // not found: a clear "nothing here"
}
```

Use return-by-address when "not found" / "nothing" is a natural outcome that `nullptr` can carry. Use return-by-reference when a valid object is *guaranteed* to exist.

---

## 12.13 — In and out parameters

Now that you can pass and return references and pointers, it's worth naming the *roles* parameters play. Thinking in these terms makes APIs easier to design and to read.

### In parameters

An **in parameter** carries information *into* the function for it to read. Pass-by-value and pass-by-const-reference are both in parameters:

```cpp
void print(std::string_view message); // function reads message, doesn't change it
```

### Out parameters

An **out parameter** is one the function *writes to* in order to deliver a result back to the caller — typically a non-const reference (or pointer):

```cpp
void getLineCol(int& line, int& col)
{
    line = 10;
    col = 5;
}
```

The caller declares the variables, passes them in, and reads the results afterward:

```cpp
int line {};
int col {};
getLineCol(line, col); // line is now 10, col is now 5
```

There's a real readability cost here, though: nothing at the *call site* visually signals that `line` and `col` are about to be modified. They look like ordinary arguments. Keep that drawback in mind — it's the main reason out parameters are discouraged when a return value would do.

### In/out parameters

An **in/out parameter** is one the function both reads *and* writes — typically a non-const reference to an object it modifies in place:

```cpp
void normalize(std::string& text)
{
    // reads the current text, then rewrites it in place
}
```

The lab's `addBonusInPlace(int& score, int bonus)` is the textbook in/out: it reads `score`'s current value and writes the increased value back, all through the one reference.

### Prefer return values when you can

When a function produces a small result, returning it is almost always clearer than an out parameter, because the data flow is obvious from the signature. Instead of:

```cpp
void parsePoint(std::string_view text, int& line, int& col); // out params
```

prefer:

```cpp
struct Point
{
    int line {};
    int col {};
};

Point parsePoint(std::string_view text); // result is right there in the return type
```

(You'll meet `struct` properly in Chapter 13; the point here is the shape, not the syntax.)

> **Best practice:** Prefer returning a value over writing through an out parameter when it's practical. Reserve non-const reference parameters for cases where *modifying the caller's object is the whole point* of the function — like `void instrumentInstruction(Instruction& I)` — not as a trick to avoid a return.

---

## 12.14 — Type deduction with pointers, references, and const

Back in Chapter 10 you met `auto`, which deduces a variable's type from its initializer. References, pointers, and `const` interact with `auto` in ways that are perfectly logical once you know the rules — and surprising if you don't. The key fact: `auto` deduces the *value type* by default, dropping references and top-level const.

### `auto` drops references

```cpp
int x { 5 };
int& ref { x };

auto y { ref }; // y is int — a COPY of x, not a reference to it
```

`auto` looked at `ref`, saw "an int," and gave you an int. If you want a reference, ask for one explicitly with `auto&`:

```cpp
auto& y { ref }; // y is int& — aliases x
```

### Top-level versus low-level const

To predict what `auto` does with `const`, distinguish two kinds:

- **Top-level const** applies to the object *itself*: `const int x` — the variable `x` is const.
- **Low-level const** applies to what's being *pointed or referred to*: `const int* ptr` — the *pointee* is const, the pointer isn't.

`auto` drops *top-level* const (you're making a copy, and a copy is free to be non-const), but keeps *low-level* const (it's part of the pointed-to type):

```cpp
const int x { 5 };
auto a { x };       // int — top-level const dropped
const auto b { x }; // const int — you asked for it back
```

For references, the same logic, made explicit:

```cpp
const int& ref { x };
auto a { ref };        // int — a plain copy
const auto& b { ref }; // const int& — a const reference, no copy
```

### Type deduction with pointers

With pointers, `auto` keeps the pointer-ness automatically — but `auto*` lets you state it explicitly, which both documents intent and requires the initializer to actually be a pointer:

```cpp
int x { 5 };
int* ptr { &x };

auto p1 { ptr };  // int*
auto* p2 { ptr }; // int* — same result, but pointer-ness is spelled out
```

This is the meaning of LLVM's common idiom:

```cpp
auto *M { F.getParent() };
```

"Deduce the pointed-to type (`Module`) from `getParent()`, but make it visually clear that `M` is a *pointer*."

### Pointers and const, deduced

The top-level / low-level distinction applies to pointers too. A *const pointer* has top-level const (the pointer itself is const), which `auto` drops:

```cpp
int x { 5 };
int* const constPtr { &x };

auto p { constPtr }; // int* — the pointer's own const is dropped
```

If you want the deduced pointer to *stay* const, ask:

```cpp
auto* const p { constPtr }; // int* const
```

But a pointer to *const value* has low-level const, which `auto` keeps, because it's part of the pointed-to type:

```cpp
const int y { 5 };
auto p { &y }; // const int* — low-level const preserved
```

### Summary table

| Declaration | Deduces / means |
|---|---|
| `auto x { ref };` | a copy of the referenced value |
| `auto& x { ref };` | a reference (no copy) |
| `const auto& x { expr };` | a const reference, no copy |
| `auto p { ptr };` | the pointer type, deduced |
| `auto* p { ptr };` | a pointer, with pointer-ness required and explicit |
| `const auto* p { ptr };` | a pointer to a const value |
| `auto* const p { ptr };` | a const pointer |

---

## 12.15 — std::optional

### The problem: a result that might not exist

Some functions can't always produce an answer. "Find the index of this character" — but what if the character isn't there? "Parse this line" — but what if it's malformed? The old approach is a **sentinel**: pick some normal-looking return value to secretly mean "nothing."

```cpp
int findIndex(/* ... */); // returns -1 when not found
```

The trouble is that `-1` is a perfectly good `int`. Nothing in the type stops a caller from forgetting the special case and using `-1` as if it were a real index. The "maybe nothing" lives only in your memory and a comment.

### `std::optional<T>` makes "maybe nothing" part of the type

`std::optional<T>` (from `<optional>`) holds *either* a value of type `T` *or* nothing at all. Now "no result" is encoded in the type itself, and the caller is forced to deal with it:

```cpp
#include <optional>
#include <string_view>

std::optional<int> findIndex(std::string_view text, char target)
{
    for (int i { 0 }; i < static_cast<int>(text.length()); ++i)
    {
        if (text[static_cast<std::size_t>(i)] == target)
            return i; // implicitly wraps i into an optional<int>
    }

    return std::nullopt; // the explicit "no value"
}
```

Returning an `int` constructs an optional that *has* a value; returning `std::nullopt` constructs the empty one.

Using the result, you check first, then read the value with `*` (just like a pointer, but with `.has_value()` available too):

```cpp
auto index { findIndex("abc", 'b') };

if (index.has_value())
{
    std::cout << *index << '\n'; // safe: we confirmed there's a value
}
```

Since an optional converts to `bool` (true when it holds a value), the short form reads nicely:

```cpp
if (index)
{
    std::cout << *index << '\n';
}
```

> **Warning:** Dereferencing an empty optional with `*` is undefined behavior, exactly like dereferencing a null pointer. Always confirm with `if (opt)` or `.has_value()` before reading `*opt`.

### Trade-offs

What you gain:

- no magic sentinel a caller can misread;
- absence is visible *in the type*, so the compiler and the reader both see it;
- the caller is pushed to handle the empty case.

What it costs:

- a little more syntax at the call site;
- it isn't the right shape for every result (sometimes you want exceptions, or a status enum);
- there's no `std::optional` of a *reference* in the common standard usage — for "an existing object or nothing," a pointer is still the tool.

### Optional parameters

`std::optional` can also model an optional *parameter* ("a value may or may not be supplied"). Often, though, an overload or a default argument expresses that more cleanly. Reach for `std::optional` when "maybe a value" is genuinely part of the model, not just to make one parameter skippable.

This is exactly the CS6340 payoff:

```cpp
std::optional<CoveragePoint> parseCoverageLine(std::string_view line);
```

That signature *says* parsing can fail — far clearer than returning `{0, 0}` and hoping every caller knows it secretly means "couldn't parse."

---

## 12.x — Chapter 12 summary and quiz

### Core takeaways

- **Compound types** are built from other types — references, pointers, arrays, classes.
- Every expression has a **value category**: **lvalues** identify objects with a home; **rvalues** are temporaries.
- A **reference** is an alias. It must be initialized, and it can never be reseated.
- A non-const lvalue reference binds only to **modifiable lvalues**.
- A **const lvalue reference** can also bind to const objects and temporaries, and extends a directly-bound temporary's lifetime.
- **Pass by reference** avoids copies; **pass by const reference** avoids copies *and* forbids modification.
- Prefer **`std::string_view`** for read-only string parameters.
- A **pointer** stores an address; it can be null and can be reseated.
- Dereferencing a **null, dangling, or uninitialized** pointer is undefined behavior.
- Use **`nullptr`**, never `0` or `NULL`.
- **Reference when the object must exist; pointer when null or reseating is meaningful.**
- `const int*`, `int* const`, and `const int* const` mean three different things.
- **Never return a reference or pointer to a local variable.**
- Prefer **returning a value** over an out parameter when practical.
- `auto` drops references and top-level const; ask with `auto&`, `const`, `auto*` when you want them.
- **`std::optional<T>`** models "maybe a `T`" explicitly, with no magic sentinel.

### Reference vs. pointer at a glance

| Question | Reference | Pointer |
|---|---|---|
| Must be initialized? | Yes | Not required, but always should be |
| Can be null? | No | Yes (`nullptr`) |
| Can be reseated? | No | Yes |
| Member access | `ref.member` | `ptr->member` or `(*ptr).member` |
| Best for | a required object | an optional or reseatable object |

### Decoding the CS6340 signatures

```cpp
bool Instrument::runOnFunction(Function &F);
```

`F` is an existing LLVM function, passed by non-const reference — the pass may inspect *and* modify it, no copy made.

```cpp
Module *M = F.getParent();
```

`M` holds the *address* of the parent module. You'd reach its members with `M->member`. It's a pointer because the relationship is address-modeled and absence may be possible — check the API.

```cpp
Instruction &I
```

`I` is an alias for an existing instruction. No instruction is copied; the function works on the real one.

### Mini drill

A small program tying together the chapter's tools — `std::string_view` for read-only input, `std::optional<int>` for a maybe-found index, `std::string&` for deliberate mutation, and the `static_cast<std::size_t>` you need at the indexing boundary:

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

    std::cout << mutant << '\n'; // abXde
}
```

This is the heart of the chapter's **Alias Workshop** lab, where you'll build six small functions — `swapByRef`, `swapByPtr`, `maxOf`, `describePointer`, `addBonusInPlace`, and `findFirst` — that together exercise every reference and pointer pattern above. The grader doesn't just check return values; it checks that the *caller's* variables actually changed, and even compares raw addresses to prove the aliasing is physical. Watch for the chapter's number-one trap in `maxOf`: return a reference to a *parameter* (which outlives the call), never to a local (which dangles). Once these six click, those LLVM signatures will read like plain English.
