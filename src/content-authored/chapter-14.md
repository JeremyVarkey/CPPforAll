# Chapter 14 — Introduction to Classes

> Source: <https://www.learncpp.com/> (Chapter 14)

Up to this point, your programs have organized data and behavior as two separate things. You had structs to hold data, and free functions that acted on that data — a `Fraction` struct sitting in one place, a `reduce(Fraction&)` function sitting in another. That arrangement works, and for genuinely passive bundles of data it's the right call. But it has a weakness that grows more painful as programs get larger: nothing stops a caller from reaching in and corrupting the data. If a `Fraction`'s denominator must never be zero, a plain struct has no way to *enforce* that. Anyone can write `f.denominator = 0` and the language won't object.

This chapter introduces the C++ tool built to close that gap: the **class**. A class lets you bundle data together with the functions that operate on it, *and* draw a line between the parts callers are allowed to touch and the parts they aren't. That single capability — hiding data behind a controlled interface — is the foundation of object-oriented programming, and it changes how you design programs. You'll learn how to write member functions that act on an object, how `const` lets the compiler enforce read-only access, how access specifiers wall off your internals, and how **constructors** guarantee that every object is born in a valid state. By the end you'll be able to build a `Fraction` class that simply *cannot* hold an invalid value — the central idea this chapter is organized around.

If you're coming from Java, the concepts here will feel familiar: classes, methods, constructors, private fields. The work is in the mechanics, which are genuinely different in C++ — value semantics instead of references, deterministic lifetimes instead of garbage collection, and a `const` system Java has no real equivalent for. We'll flag those differences as we go.

## Contents
- [14.1 — Introduction to object-oriented programming](#141--introduction-to-object-oriented-programming)
- [14.2 — Introduction to classes](#142--introduction-to-classes)
- [14.3 — Member functions](#143--member-functions)
- [14.4 — Const class objects and const member functions](#144--const-class-objects-and-const-member-functions)
- [14.5 — Public and private members and access specifiers](#145--public-and-private-members-and-access-specifiers)
- [14.6 — Access functions](#146--access-functions)
- [14.7 — Member functions returning references to data members](#147--member-functions-returning-references-to-data-members)
- [14.8 — The benefits of data hiding (encapsulation)](#148--the-benefits-of-data-hiding-encapsulation)
- [14.9 — Introduction to constructors](#149--introduction-to-constructors)
- [14.10 — Constructor member initializer lists](#1410--constructor-member-initializer-lists)
- [14.11 — Default constructors and default arguments](#1411--default-constructors-and-default-arguments)
- [14.12 — Delegating constructors](#1412--delegating-constructors)
- [14.13 — Temporary class objects](#1413--temporary-class-objects)
- [14.14 — Introduction to the copy constructor](#1414--introduction-to-the-copy-constructor)
- [14.15 — Class initialization and copy elision](#1415--class-initialization-and-copy-elision)
- [14.16 — Converting constructors and the explicit keyword](#1416--converting-constructors-and-the-explicit-keyword)
- [14.17 — Constexpr aggregates and classes](#1417--constexpr-aggregates-and-classes)
- [14.x — Chapter 14 summary and quiz](#14x--chapter-14-summary-and-quiz)

---

## 14.1 — Introduction to object-oriented programming

### Two ways to organize a program

There are two broad philosophies for structuring code, and it helps to name them before we commit to one.

**Procedural programming** organizes a program around *functions* — the steps the program performs — and treats data as something those functions are handed. The data is over here; the operations that act on it are over there. A point you can move looks like this:

```cpp
struct Point
{
    int x {};
    int y {};
};

void movePoint(Point& p, int dx, int dy)
{
    p.x += dx;
    p.y += dy;
}
```

The `Point` holds the data. `movePoint` knows how to change it. They are connected only by the fact that the function happens to take a `Point`. Nothing binds them together; nothing stops you from writing some *other* function that mangles a `Point` in a way `Point` never anticipated.

I want to be clear that this is not a bad design. For simple data — coordinates, a pair of dates, a color — keeping data and operations separate is often the *clearest* choice. Procedural code is not something to apologize for.

### Object-oriented programming

**Object-oriented programming (OOP)** organizes a program around *objects* — bundles that own both their data and the behavior that acts on that data. Instead of a function that takes a `Point`, the `Point` itself knows how to move:

```cpp
class Point
{
public:
    void move(int dx, int dy)
    {
        m_x += dx;
        m_y += dy;
    }

private:
    int m_x {};
    int m_y {};
};
```

Now the data (`m_x`, `m_y`) and the behavior (`move`) live in the same place. You ask a `Point` to move *itself*: `p.move(3, 4)`. The object is the unit of organization.

Picture the difference like this. In the procedural world, data and behavior are two separate boxes connected by a "used by" arrow. In the OOP world, they're inside one box:

```text
procedural:                    object-oriented:
  Point data                     Point object
     |                           +---------------------+
   used by                       | data: x, y          |
     |                           | behavior: move()    |
  movePoint()                    +---------------------+
```

### Why this is worth the trouble

Bundling data with behavior and controlling access to the data buys you several things that matter enormously as programs grow:

- **Related things stay together.** The operations on a `Point` are *in* `Point`, where you'd look for them.
- **Implementation details can hide.** Callers use `move()`; they never see `m_x` and `m_y`, so you're free to change how a point is stored later.
- **Invariants can be enforced.** If an object should never reach an invalid state, the object itself can refuse to.
- **Misuse gets harder.** When the only way to change an object is through functions you wrote, there are fewer ways for a caller to break it.
- **Large programs become tractable.** You reason about one object's guarantees at a time instead of tracking every place its raw data might be touched.

> **Key insight:** OOP isn't about ceremony or new syntax for its own sake. Its real payoff is *control* — over who can change an object's data, and how. That control is what lets you make promises about an object that stay true no matter who uses it.

If you've written Java, you already own the conceptual half of this. The C++ half — value semantics, references, `const`, constructors, destructors, copying, access control — is what the rest of this chapter teaches.

### What "object" means in C++

One terminology note, because it trips up people coming from other languages. In C++, an **object** is simply *a region of storage that can hold a value*. That is broader than the Java meaning. A plain `int` variable is an object in C++. So is a `Point`.

```cpp
int x { 5 }; // x is an object
Point p {};  // p is also an object
```

When we talk about "objects" in this chapter we usually mean class-type objects, since those are the interesting ones here. But keep the general definition in mind — it explains why the language treats your custom types and the built-in types so consistently.

---

## 14.2 — Introduction to classes

### The problem classes solve: invariants

To understand *why* classes exist, you need one idea first: the **invariant**. An invariant is a condition that must always hold for an object to be considered valid — a rule the object promises to obey for its entire life.

Consider a fraction. A fraction is a numerator over a denominator, and there's a rule baked into the very concept: **the denominator must never be zero.** Dividing by zero is undefined; a fraction with a zero denominator isn't a fraction, it's a bug. "Denominator is not zero" is the invariant.

Now try to enforce that invariant with a plain struct:

```cpp
struct Fraction
{
    int numerator {};
    int denominator {};
};
```

You can't. The data is public, which means anyone can write whatever they like:

```cpp
Fraction f { 1, 2 };
f.denominator = 0; // nothing stops this — the invariant is now broken
```

The struct has no opinion about its own validity. It's a passive bag of two ints. The moment your program relies on "this fraction is always valid," a struct can't carry that weight.

### How a class protects an invariant

A **class** can make its data private and force every change to go through functions you control. Those functions become the only doorways to the data — and a doorway can have a guard.

```cpp
class Fraction
{
public:
    void setDenominator(int denominator)
    {
        if (denominator != 0)        // the guard
            m_denominator = denominator;
    }

private:
    int m_numerator {};
    int m_denominator { 1 };
};
```

Now `m_denominator` is private. A caller *cannot* write `f.m_denominator = 0` — the compiler forbids it. The only way to change the denominator is `setDenominator`, and that function simply refuses a zero. The invariant is no longer a hope; it's enforced by the structure of the code.

> **Key insight:** Making data private isn't about secrecy. It's about funneling every change through code that can check it. A private data member plus a guarding member function is how a class makes a promise it can actually keep.

### Defining a class

A class definition introduces a brand-new type. Here's the general shape, using a `Date`:

```cpp
class Date
{
public:
    void print() const;

private:
    int m_year {};
    int m_month {};
    int m_day {};
};
```

The `class` keyword names the type. Inside, `public:` and `private:` label sections (we'll cover those properly in 14.5). Member functions and data members live between the braces. Note the semicolon after the closing brace — a class definition is a statement, and forgetting that semicolon is one of the most common beginner errors in C++.

Once a class is defined, you create objects of that type just like any other type:

```cpp
Date today {};
```

`today` is an object — a `Date` — with its own copies of `m_year`, `m_month`, and `m_day`.

### You already use classes constantly

This is not exotic machinery reserved for advanced code. A huge fraction of the standard library is classes:

```cpp
std::string         // a class that manages a sequence of characters
std::vector<int>    // a class that manages a growable array
std::optional<int>  // a class that holds "a value, or nothing"
```

Every time you've called `.size()` on a string or `.push_back()` on a vector, you were calling a member function on a class object, using exactly the machinery this chapter unpacks. The goal here is to take you from *using* classes to *writing* them.

> **Note:** The same is true of the large C++ APIs you'll meet in later coursework. Libraries like LLVM are built almost entirely from classes — `Module`, `Function`, `Instruction` — each with private data, member functions, and invariants. Learning to read a class is learning to read those APIs.

---

## 14.3 — Member functions

### State and behavior

A class has two kinds of members, and the distinction is worth saying plainly:

- **Data members** store the object's *state* — what it currently is.
- **Member functions** define the object's *behavior* — what it can do.

Here's a tiny counter that has both:

```cpp
class Counter
{
public:
    void increment()
    {
        ++m_value;
    }

    int value() const
    {
        return m_value;
    }

private:
    int m_value {};
};
```

`m_value` is the state. `increment()` and `value()` are the behavior — one changes the state, one reports it.

### Calling a member function

You call a member function on a specific object, using the dot operator:

```cpp
Counter counter {};
counter.increment();
std::cout << counter.value() << '\n'; // prints 1
```

Here's the part that's genuinely new. When you write `counter.increment()`, the object before the dot — `counter` — is the **implicit object** that the function operates on. Inside `increment()`, when you write `m_value`, you're referring to *this object's* `m_value`. You never have to pass the object in; it's the thing you called the function on.

```cpp
void increment()
{
    ++m_value; // means: increment THIS object's m_value
}
```

So if you have two counters, `a.increment()` touches `a`'s `m_value` and `b.increment()` touches `b`'s — same function, different implicit object each time.

```text
counter.increment()
   |
   v
inside increment(), the implicit object is `counter`,
so m_value means counter.m_value
```

> **Key insight:** A member function always acts on the object it was called on. Unqualified member names inside the function — like `m_value` — silently mean "the implicit object's `m_value`." That implicit object is the whole reason you don't pass the object as an argument.

### Defining a member function: inside or outside

You can write a member function's body in two places. The first is **inside the class**, right where it's declared:

```cpp
class Counter
{
public:
    void increment()
    {
        ++m_value;
    }

private:
    int m_value {};
};
```

The second is to **declare it inside and define it later**, outside the class body:

```cpp
class Counter
{
public:
    void increment(); // declaration only

private:
    int m_value {};
};

void Counter::increment() // definition, elsewhere
{
    ++m_value;
}
```

The `Counter::` prefix is the key. It says "this `increment` is the one belonging to `Counter`" — without it, the compiler would think you're defining a free function named `increment`. The `::` is the *scope resolution operator*; you're reaching into `Counter`'s scope to define one of its members.

For this chapter, we'll keep bodies **inside** the class. It's the simplest form, it's all the chapter's lab needs, and the `.h`/`.cpp` split that makes the out-of-class form pay off is a Chapter 15 topic.

### Members can refer to members declared later

One pleasant surprise: a member function can use data members that are declared *below* it in the class. The compiler reads the whole class definition before it compiles any function body, so order within the class doesn't restrict what a function can name.

That freedom lets you order a class for *human* readers rather than the compiler. The convention you'll see throughout this course — and most C++ codebases — is:

1. the **public interface** first (what users care about),
2. the **private implementation and data** last (what they don't).

### Member functions can be overloaded

Just like free functions, member functions can be overloaded — same name, different parameter lists:

```cpp
class Printer
{
public:
    void print(int value);
    void print(std::string_view value);
};
```

The compiler picks the right `print` based on the argument type, exactly as it does for overloaded free functions (Chapter 11).

### Structs can have member functions too

Here's something that surprises people: in C++, **structs can also have member functions.** Technically, the *only* difference between `struct` and `class` is the default access level:

| Type keyword | Default member access |
|---|---|
| `struct` | public |
| `class` | private |

That's it. Everything else they can do, both can do. So the choice between them is a matter of *intent*, not capability:

- Use **`struct`** for passive aggregates — bundles of data with no invariant to protect, where everything is meant to be public.
- Use **`class`** for objects that have invariants and behavior, where data should be private behind an interface.

> **Best practice:** Reach for `struct` when you're just grouping public data, and `class` when the type has private state and a meaningful interface. Following this convention tells your reader what kind of type they're looking at before they read a single member.

---

## 14.4 — Const class objects and const member functions

### The problem: const objects need read-only functions

You already know `const` from earlier chapters — a `const` object can't be modified. That rule applies to class objects too:

```cpp
const Counter c {};
// c.increment(); // must be forbidden: increment() modifies the object
```

That's clearly correct — `increment()` changes `m_value`, and `c` is `const`, so the call must be rejected. But now consider the *reading* function:

```cpp
const Counter c {};
std::cout << c.value(); // value() only reads — surely this should be allowed?
```

Logically `value()` is harmless on a const object; it just reads `m_value`. But the compiler doesn't read your function bodies to decide what's safe — it can't, in general. It needs you to *declare your intent*. It needs a way to know, from the function's signature alone, which member functions promise not to modify the object.

### Const member functions

You make that promise by writing `const` after the parameter list:

```cpp
class Counter
{
public:
    int value() const // the trailing const is the promise
    {
        return m_value;
    }

private:
    int m_value {};
};
```

That trailing `const` declares: *"calling this function will not modify the object."* The compiler holds you to it — if you tried to write `++m_value` inside a `const` member function, compilation would fail. The general syntax:

```cpp
return_type function_name(parameters) const
```

Once `value()` is `const`, the earlier call is fine:

```cpp
const Counter c {};
std::cout << c.value(); // ok: value() is a const member function
```

> **Key insight:** A const object may only call const member functions. Marking a function `const` is a promise to the compiler — and the compiler enforces it both ways: it checks your function keeps the promise, and it lets const objects rely on it.

A non-const object, by the way, can call *either* kind — const or non-const member functions. The restriction only flows one direction: const objects are limited to const members.

### Why this matters far more than it first appears

You might think const objects are rare, so this is a minor rule. It isn't — and the reason is **const references**. The standard, efficient way to pass a class object into a function is by const reference (you saw this in Chapters 5 and 12): it avoids a copy and promises the function won't change the argument.

```cpp
void printCounter(const Counter& c)
{
    std::cout << c.value() << '\n'; // value() MUST be const for this to compile
}
```

Inside this function, `c` is effectively const. So it can only call const member functions. If you forgot to mark `value()` as `const`, this perfectly reasonable function would *fail to compile* — even though it obviously only reads.

> **Best practice:** Mark every member function that doesn't modify the object as `const`. Skip it and you'll hit a wall the first time someone passes your object by const reference — which, for class types, is most of the time.

### Const and non-const overloads

A function can actually be overloaded on `const`. A class can offer both a const and a non-const version of the same operation:

```cpp
class Buffer
{
public:
    std::string& text() { return m_text; }             // chosen for non-const objects
    const std::string& text() const { return m_text; } // chosen for const objects

private:
    std::string m_text {};
};
```

When you call `text()` on a modifiable `Buffer`, you get the non-const version (which lets you change the text). On a const `Buffer`, you get the const version (read-only). This is an advanced convenience you'll see in library types; for now, just recognize it when you meet it.

---

## 14.5 — Public and private members and access specifiers

### Access control, in one sentence

**Access specifiers** decide who is allowed to use a member. They're the mechanism behind everything we said about hiding data. Here are the three levels:

| Access | Who can access the member |
|---|---|
| `public` | anyone — any code, anywhere |
| `private` | only member functions (and *friends*) of the same class |
| `protected` | the class and its derived classes — covered with inheritance, later |

For this chapter, the two that matter are `public` and `private`. Public members form the **interface** — the controls you offer the outside world. Private members form the **implementation** — the inner workings only the class itself may touch.

### struct default vs class default, made concrete

We saw the difference in 14.3; here it is in code. Members with no specifier above them take the type's default:

```cpp
struct S
{
    int x {}; // public by default (struct)
};

class C
{
    int x {}; // private by default (class)
};
```

In practice, almost every real class spells its access out explicitly rather than relying on the default — it's clearer for the reader:

```cpp
class Fuzzer
{
public:
    void run();

private:
    int m_count {};
};
```

A specifier applies to every member below it until the next specifier appears, so you typically write one `public:` block and one `private:` block.

### The `m_` naming convention

You've seen it in every example: private data members get an `m_` prefix — `m_count`, `m_name`, `m_denominator`. This is the convention LearnCpp uses and the one this course follows.

```cpp
private:
    int m_count {};
    std::string m_name {};
```

The prefix isn't required by the language; it's a readability aid. When you see `m_count` in a function body, you instantly know it's a data member of the object, not a local variable or a parameter. That distinction matters constantly inside member functions, where locals and members mingle.

### Access is per-class, not per-object

Here's a subtlety that catches people, and it'll matter for the lab: a member function can access the private members of *any* object of its own class — not just the implicit object it was called on.

```cpp
class Counter
{
public:
    bool sameValue(const Counter& other) const
    {
        return m_value == other.m_value; // legal: other is also a Counter
    }

private:
    int m_value {};
};
```

Inside `sameValue`, `m_value` is this object's value and `other.m_value` is the *other* object's value — and reaching into `other.m_value` is perfectly legal even though it's private. Why? Because access control in C++ is **per-class, not per-object.** You are inside a `Counter` member function, and `other` is a `Counter`, so its privates are open to you.

> **Key insight:** Private means "private to the class," not "private to this one object." Any member function of `Fraction` can read any `Fraction`'s private data. The chapter's lab leans on exactly this when one fraction compares itself to another.

### When to use struct, when to use class — the practical rule

Pulling the guidance together:

Use a **`struct`** when:

- all members are meant to be public,
- the type mostly just groups data,
- there's no real invariant that needs protecting.

Use a **`class`** when:

- some data should be private,
- behavior needs to guard an invariant,
- there's a meaningful public interface distinct from the internals.

A `Fraction` with its non-zero-denominator rule is squarely a `class`. A plain 2D point with two freely-editable coordinates is comfortable as a `struct`.

---

## 14.6 — Access functions

### Getters and setters

Once data is private, callers can't read or write it directly — so how does the outside world interact with it at all? Through **access functions**: small public member functions that expose *controlled* access to private data.

```cpp
class Person
{
public:
    std::string_view name() const // getter / accessor: reads
    {
        return m_name;
    }

    void setName(std::string name) // setter / mutator: writes
    {
        m_name = name;
    }

private:
    std::string m_name {};
};
```

The vocabulary:

- A **getter** (or **accessor**) reads a value and hands it back.
- A **setter** (or **mutator**) takes a value and changes the object.

Getters are almost always `const` — they only read. Setters can't be, since they modify the object.

### Naming accessors

Two naming styles are common, and both are fine:

```cpp
name()      // the "noun" style
getName()   // the "get" style
```

Neither is more correct; what matters is consistency. Pick the style your codebase already uses and stick to it. This course generally uses the `name()` form.

### Return by value or by const reference?

How a getter returns its value depends on how expensive that value is to copy. For cheap types like `int`, return by value — copying an `int` is free:

```cpp
int count() const { return m_count; }
```

For expensive owned members like a long string, you *can* return a const reference to avoid the copy:

```cpp
const std::string& name() const { return m_name; }
```

But returning a reference into your object hands the caller a window into your internals, which raises lifetime and encapsulation questions. We'll dig into those in the very next lesson — they're subtle enough to deserve their own section. (For read-only string access where the object clearly outlives the use, `std::string_view` is often the cleanest middle ground, as the `Person` example above shows.)

### If getters exist, why bother making data private?

This is the question every newcomer asks, and it's a fair one. If you're going to write a getter *and* a setter for a member, haven't you just made it public with extra steps?

No — and the difference is everything that an access function can do *besides* pass the value through. A getter or setter can:

- **validate** a value before storing it (reject a zero denominator),
- **compute** a value rather than store it (a `Rectangle` might compute `area()` from width and height),
- **log or debug** access while you're hunting a bug,
- **preserve an invariant** that raw public data couldn't,
- **change the implementation later** without breaking callers — swap the storage, keep the interface.

A public data member offers *none* of these. The accessor is a seam where you can insert logic, now or in the future.

> **Best practice:** Don't reflexively generate a getter and setter for every member. A setter is part of your public interface, and every setter you add is a new way for a caller to change the object. Add an accessor when callers genuinely need that access — and make sure each one keeps the object valid.

---

## 14.7 — Member functions returning references to data members

This lesson is a careful look at one decision a getter has to make: should it return a **copy** of a member, or a **reference** to it? Each choice has a real cost, and getting it wrong leads to either wasted performance or dangling references.

### Returning by value copies

The straightforward getter returns by value:

```cpp
std::string name() const
{
    return m_name; // makes a copy of m_name
}
```

This is safe — the caller gets their own independent copy. But for a large member, that copy can be expensive, and if the caller only wanted to *look* at the value, the copy was pure waste.

### Returning by const reference avoids the copy

To skip the copy, return a const reference:

```cpp
const std::string& name() const
{
    return m_name; // no copy: hands back a reference to the member
}
```

No copy is made. But there's a string attached, literally: the caller now holds a reference that points *into your object*. That reference is only valid for as long as the object is alive.

```text
Person object: [---------------- alive ----------------]
returned ref:        [ valid only while the Person lives ]
```

If the object is destroyed while the caller is still holding that reference, the reference **dangles** — it points at storage that no longer holds a valid object, and using it is undefined behavior. Returning by value never has this problem, because the caller's copy is independent.

> **Warning:** A const reference returned from a getter is a borrowed view into the object, not an independent value. It stays valid only as long as the object does. Returning by value trades a copy for safety; returning by reference trades safety for speed. Choose deliberately.

### Match the return type to the member

When you do return a reference to a member, the return type should match the member's type:

```cpp
const std::string& name() const; // returns a reference to the std::string m_name
```

Returning a reference to something *other* than the actual member — or to a temporary computed inside the function — is a recipe for a dangling reference.

### Never return a *non-const* reference to private data

There's a sharper version of this rule. Returning a *const* reference at least keeps the data read-only. Returning a **non-const** reference hands the caller the keys:

```cpp
std::string& name() // dangerous: returns a modifiable reference to private data
{
    return m_name;
}
```

This quietly destroys your encapsulation. The caller can now reach in and mutate the member with zero validation:

```cpp
person.name().clear(); // wipes the private member, bypassing every guard
```

You spent all of 14.5 making `m_name` private so changes would go through controlled functions — and a non-const reference getter throws that away by giving callers a direct handle. Only return a non-const reference when you *deliberately* intend to allow that mutation and accept the consequences for your invariants.

### The temporary-object trap

One last hazard. Be careful calling a reference-returning getter on a **temporary** object:

```cpp
const std::string& ref { Person{}.name() }; // dangerous
```

`Person{}` is a temporary — it's destroyed at the end of this statement. So `ref` is left pointing into an object that no longer exists. We'll meet temporaries properly in 14.13; for now, just register the danger: a reference into a temporary dangles almost immediately.

---

## 14.8 — The benefits of data hiding (encapsulation)

We've been circling this idea for several lessons. Now let's name it and make the case directly. **Encapsulation** — also called **data hiding** — is the practice of keeping an object's data private and exposing only a controlled set of public functions to work with it. It is the central design principle this whole chapter has been building toward.

### Interface versus implementation

The cleanest way to think about encapsulation is to separate two things in your mind:

- The **interface** is *what* callers can do — the public functions, the operations you promise to support.
- The **implementation** is *how* the class does it — the private data, the helper functions, the internal machinery.

```cpp
class Timer
{
public:
    void start();                  // interface: what you can do
    double elapsedSeconds() const;

private:
    // implementation: how it works — hidden, and free to change
};
```

A caller of `Timer` writes `start()` and `elapsedSeconds()`. They neither know nor care whether you store a start timestamp, a tick count, or anything else. As long as the interface keeps its promises, you can rewrite the implementation entirely and *no caller breaks.*

> **Key insight:** Code against an interface, not an implementation. When callers depend only on your public functions, you keep the freedom to change everything behind them. That freedom is the single most valuable thing encapsulation buys you.

### The concrete payoffs

Keeping data private and exposing controlled operations gives you, in practice:

- a **smaller public surface** — fewer things to understand and fewer to misuse,
- **fewer ways to break the object**, because there are fewer paths to its data,
- **enforceable invariants**, since every change runs through your code,
- **early rejection of invalid values**, before they corrupt anything,
- the **freedom to change internals** without touching callers,
- **easier debugging** — when something's wrong with the data, there are only a handful of places that could have changed it.

That last point is worth dwelling on. If a `Fraction`'s denominator goes bad and the only code that can write `m_denominator` is inside `Fraction`, your search for the bug is *tiny*. With a public field, the culprit could be anywhere in the program.

### Centralizing an invariant

Here's encapsulation enforcing the fraction invariant in one place — the constructor (which we formally meet in 14.9):

```cpp
class Fraction
{
public:
    Fraction(int numerator, int denominator)
        : m_numerator { numerator }
        , m_denominator { denominator == 0 ? 1 : denominator } // guard, in one spot
    {
    }

private:
    int m_numerator {};
    int m_denominator { 1 };
};
```

The "denominator is never zero" rule is checked in exactly one location. Every `Fraction` that exists came through here, so every `Fraction` is valid. That's the power of routing all construction through a guarded entry point.

### Prefer non-member functions when internals aren't needed

A useful corollary: if a function can do its job using only the class's *public* interface, consider making it a **non-member** (free) function rather than a member.

```cpp
bool isEmpty(const Buffer& b)
{
    return b.size() == 0; // uses only the public interface
}
```

`isEmpty` doesn't need `Buffer`'s private data — `size()` is enough. Leaving it outside the class keeps the class's interface smaller and the function's dependencies honest. A leaner interface is easier to learn, easier to maintain, and easier to keep correct.

### Order your class for the reader

Finally, a small habit that pays off constantly. Put the **public interface first** and the **private details last**:

```cpp
class Thing
{
public:
    // public types and functions — what users came to see

private:
    // private helpers and data — what they don't need to
};
```

Someone reading your class wants to know what it *does* before how it works. Leading with the public interface answers their first question immediately.

---

## 14.9 — Introduction to constructors

### The front door for object creation

We've kept gesturing at constructors; now they take center stage. A **constructor** is a special member function whose job is to *initialize* a new object. It runs automatically when an object is created, and it's where you guarantee the object starts life in a valid state.

```cpp
class Fraction
{
public:
    Fraction(int numerator, int denominator)
        : m_numerator { numerator }
        , m_denominator { denominator }
    {
    }

private:
    int m_numerator {};
    int m_denominator { 1 };
};
```

A constructor has two distinctive features that set it apart from ordinary member functions:

- its **name is exactly the class name** — `Fraction`,
- it has **no return type** — not even `void`.

You invoke it implicitly by creating an object and supplying initializers:

```cpp
Fraction f { 1, 2 }; // calls Fraction(int, int) with 1 and 2
```

The part after the colon — `: m_numerator { numerator }, m_denominator { denominator }` — is the **member initializer list**, and it's important enough to get its own lesson next. For now, read it as "initialize each member from the matching argument."

### A constructor's job is a valid object

Think of the constructor as the *only legitimate front door* into an object. Raw, uninitialized storage goes in; a fully valid object comes out.

```text
raw storage  ->  constructor  ->  valid object
```

This is the cleanest place to enforce an invariant, because *every* object is born here. If the constructor refuses to produce an invalid fraction, then no invalid fraction can exist. That's a far stronger guarantee than checking validity scattered throughout your program.

> **Key insight:** A constructor is your one chance to establish an object's invariant at birth. Enforce it here and the rest of your code gets to *assume* every object is valid — because there was no other way to make one.

### Constructors are never const

You'll never mark a constructor `const`. The reason follows from what it does: a constructor's whole purpose is to *modify* the object — to write its initial values. A const member function promises not to modify the object, which is the opposite of a constructor's job. (There's also a deeper reason: during the constructor, the object isn't fully formed yet, so "const object" wouldn't even be meaningful.)

### Constructors versus setters

Both constructors and setters change an object's data, so when do you use which?

- A **constructor** establishes a valid *initial* state, once, at creation.
- A **setter** modifies an *already-existing* object, later.

The design guidance that falls out of this: if a value is *required* for the object to be valid, demand it in the constructor — don't make it an optional setter the caller might forget. A `Fraction` can't sensibly exist without a denominator, so the denominator belongs in the constructor, not in a `setDenominator` the caller has to remember to call.

---

## 14.10 — Constructor member initializer lists

### Initialize, don't assign

The **member initializer list** is the preferred way to give a constructor's members their starting values. It's the part between the parameter list and the constructor body, introduced by a colon:

```cpp
class Fraction
{
public:
    Fraction(int numerator, int denominator)
        : m_numerator { numerator }     // initializer list
        , m_denominator { denominator }
    {
        // body (empty here)
    }

private:
    int m_numerator {};
    int m_denominator { 1 };
};
```

Each `m_member { value }` entry *initializes* that member, and the whole list runs **before** the constructor body executes. By the time the body's opening brace is reached, every member listed already holds its value.

### Why the list beats assignment in the body

You could, instead, leave the members default-initialized and assign to them in the body:

```cpp
Example(int x)
{
    m_x = x; // assignment — happens AFTER m_x was already default-initialized
}
```

This works for simple types but it's doing more than necessary. The member `m_x` first gets default-initialized, *then* immediately overwritten by the assignment — two steps where one would do. The initializer-list version does it in a single step:

```cpp
Example(int x)
    : m_x { x } // direct initialization — one step, no wasted default
{
}
```

For an `int` the difference is negligible, but for class-type members the wasted default-construction can cost real work. And there's a harder reason that makes the list non-negotiable in some cases:

> **Key insight:** Some members *can only* be initialized in the member initializer list — `const` members and reference members among them. They can't be assigned to in the body, because by the time the body runs they're already established and can't be changed. Get comfortable with the initializer list now; later, it's the only option.

> **Best practice:** Prefer the member initializer list over assignment in the constructor body. It initializes directly instead of default-then-assign, and it's the only way to initialize `const` and reference members.

### Members initialize in declaration order — a real gotcha

Here's a rule that surprises everyone exactly once. Members are initialized in the order they are **declared in the class** — *not* the order you list them in the initializer list.

```cpp
class Example
{
private:
    int m_a {};
    int m_b {};

public:
    Example()
        : m_b { 2 }       // listed first...
        , m_a { m_b }     // ...but m_a is declared first, so m_a initializes FIRST
    {
    }
};
```

Reading the list top to bottom, you'd expect `m_b` to be set to `2` and then `m_a` to be set to `2` from it. Wrong. Because `m_a` is *declared* before `m_b`, `m_a` initializes first — and at that moment `m_b` is still uninitialized. So `m_a` gets a garbage value. The order in the list is irrelevant; declaration order wins.

> **Warning:** Initialization follows declaration order, not the order of the initializer list. To avoid being bitten, always **list your initializers in the same order the members are declared.** Then the two orders agree and there's no surprise.

### The constructor body is for what the list can't do

The list handles direct member initialization. The **body** is for everything else — validation, adjustment, or setup that an initializer can't express cleanly:

```cpp
Fraction(int n, int d)
    : m_numerator { n }
    , m_denominator { d }
{
    if (m_denominator == 0) // a check the init list can't easily do
        m_denominator = 1;
}
```

When you *can* fold the logic into the initializer list (as the `denominator == 0 ? 1 : denominator` trick in 14.8 did), that's often cleaner. But a body is the right home for multi-step validation or anything that doesn't reduce to a single expression. The chapter's lab uses exactly this shape: initialize the raw values in the list, then guard and normalize in the body.

---

## 14.11 — Default constructors and default arguments

### The default constructor

A **default constructor** is one that can be called with *no arguments*. It's what runs when you create an object with empty braces:

```cpp
class Counter
{
public:
    Counter() = default;

private:
    int m_value {};
};

Counter c {}; // calls the default constructor
```

The `= default` syntax (more on it below) asks the compiler to generate a standard default constructor for you. Here it produces a `Counter` whose `m_value` is `0`, courtesy of the `{}` member initializer on `m_value`.

### Value initialization: prefer the braces

When you create an object that needs a default constructor, prefer empty braces:

```cpp
Counter c {}; // value initialization — preferred
```

over the bare form:

```cpp
Counter c;    // default initialization — allowed, but use {} instead
```

The braced form is **value initialization**, and it's both clearer and safer — it's visually obvious that you're initializing, and for some types it guarantees zeroing that the bare form doesn't. The braces also dodge a notorious C++ parsing quirk: writing `Counter c()` doesn't create an object at all — the compiler reads it as a *function declaration* for a function named `c` returning a `Counter`. The empty-brace form `Counter c {}` has no such ambiguity.

> **Best practice:** Use `Type name {}` (value initialization) to create a default-constructed object. It's unambiguous, and it sidesteps the `Type name()` "most vexing parse," which the compiler reads as a function declaration rather than an object.

### Constructors with default arguments

A constructor can use default arguments (Chapter 11), and this is often the simplest way to make a single constructor serve several call shapes:

```cpp
class Fraction
{
public:
    Fraction(int numerator = 0, int denominator = 1)
        : m_numerator { numerator }
        , m_denominator { denominator }
    {
    }

private:
    int m_numerator {};
    int m_denominator { 1 };
};
```

With those defaults, one constructor handles every case:

```cpp
Fraction a {};      // 0/1  — both defaulted
Fraction b { 3 };   // 3/1  — denominator defaulted
Fraction c { 3, 4 }; // 3/4  — nothing defaulted
```

Notice that this constructor is *also* a default constructor — it can be called with no arguments because both parameters have defaults. You don't need a separate `Fraction()` as well.

### The implicit default constructor — and how you lose it

If you write *no constructors at all*, the compiler quietly generates a default constructor for you — the **implicit default constructor**. That's why simple structs and classes can be created with `{}` even when you never wrote a constructor.

But there's a catch that surprises people: **the moment you declare any constructor of your own, the compiler stops generating the implicit one.** So if you write a `Fraction(int, int)` and nothing else, then `Fraction f {};` no longer compiles — there's no default constructor anymore.

When you want a default constructor *and* you've declared other constructors, ask for it explicitly with `= default`:

```cpp
Counter() = default; // "compiler, generate the standard default constructor"
```

> **Note:** Only provide a default constructor when a default-constructed object actually *makes sense*. A `Fraction` defaulting to `0/1` is reasonable. But some types have no sensible "empty" state — for those, omit the default constructor and force callers to supply the values that make the object valid.

---

## 14.12 — Delegating constructors

### A trap: calling a constructor from the body

Suppose two constructors share setup work, and you try to reuse one from inside the other by calling it in the body. It compiles — but it does *not* do what you'd hope:

```cpp
class Foo
{
public:
    Foo()
    {
        Foo(1); // does NOT initialize this object!
    }

    Foo(int x)
        : m_x { x }
    {
    }

private:
    int m_x {};
};
```

That `Foo(1)` inside the body doesn't initialize the object being constructed. It creates a *brand-new temporary* `Foo`, initializes *that* to `1`, and then throws it away at the end of the statement. The object you were actually constructing — `*this` — never gets its `m_x` set this way. This is a classic and confusing bug.

### Delegating constructors do it correctly

The right way to have one constructor reuse another is a **delegating constructor**: instead of initializing members, the constructor's initializer list calls *another constructor of the same class*.

```cpp
class Foo
{
public:
    Foo()
        : Foo { 1 } // delegate to Foo(int) — it initializes the object
    {
    }

    Foo(int x)
        : m_x { x }
    {
    }

private:
    int m_x {};
};
```

Now `Foo()` says "construct this object by running `Foo(int)` with the argument `1`." The target constructor fully initializes the object, then control returns to the delegating constructor's body (here, empty). One construction, one object, shared logic.

One rule to remember: when a constructor delegates, the initializer list contains *only* the delegation — you can't also initialize members directly in the same list, because the delegated-to constructor is responsible for that.

### Often a default argument is simpler

Delegation is the right tool when two construction paths genuinely share *logic*. But when the only difference between them is a *value*, a default argument (14.11) is simpler and clearer:

```cpp
Foo(int x = 1)
    : m_x { x }
{
}
```

This single constructor handles both `Foo {}` and `Foo { 5 }` with no delegation at all. Reach for delegating constructors when the shared work is substantial enough that expressing it as a separate constructor genuinely reads better; otherwise, a default argument usually wins.

---

## 14.13 — Temporary class objects

### What a temporary is

A **temporary object** (or **anonymous object**) is an object with no name — created, used, and destroyed within a single expression. You've made them with built-in types for ages without naming them; class types work the same way.

```cpp
std::string { "hello" }; // a temporary std::string, gone at the end of this line
```

A temporary class object lives until the end of the **full expression** it appears in, then is automatically destroyed. That short lifetime is the whole point and the whole hazard.

Temporaries show up most often as function arguments, where naming an object just to pass it would be noise:

```cpp
printName(std::string { "Ada" }); // construct a temporary string, pass it, discard it
```

You don't need a named `std::string` variable here — you need a string *for the duration of this call*, and a temporary is exactly that.

### Return by value creates temporary-like objects

Returning an object by value also produces something temporary-shaped — a value that exists to be handed back to the caller:

```cpp
Fraction makeHalf()
{
    return Fraction { 1, 2 }; // construct the return value
}
```

You might imagine this builds a `Fraction` inside `makeHalf`, then copies it out to the caller. In modern C++, it usually doesn't — the compiler constructs the result *directly* in the caller's storage. We'll see exactly why in the next lesson on copy elision.

### `static_cast` versus constructing a temporary

Two syntaxes both produce a converted value, and it's worth knowing which to reach for:

```cpp
auto x { static_cast<int>(3.5) }; // emphasizes: a conversion is happening
auto y { int(3.5) };              // function-style construction
```

For *converting* a value — especially a built-in type, and especially a narrowing one — prefer `static_cast`. It announces "I'm deliberately converting here," which is exactly the signal a reader wants. Reserve normal construction syntax (`Type { ... }`) for *building class objects*, where construction is the natural reading.

### The lifetime warning, once more

Because temporaries die at the end of their full expression, the dangling-reference danger from 14.7 applies with full force: **never keep a reference or a `std::string_view` that points into a temporary.** The temporary will be gone before you use the reference.

```cpp
std::string_view sv { std::string { "oops" } }; // sv dangles immediately
```

The temporary `std::string` is destroyed at the semicolon; `sv` is left viewing freed storage. This is the same trap as 14.7's `Person{}.name()` — a temporary plus a borrowed view is a bug waiting to fire.

---

## 14.14 — Introduction to the copy constructor

### Copying an object from another

A **copy constructor** is a constructor that initializes a new object as a *copy* of an existing object of the same type. It's what runs when you write one of these:

```cpp
Fraction a { 1, 2 };
Fraction b { a };  // copy-construct b from a
Fraction c = a;    // also copy-construct (from existing object a)
```

Written out by hand, a copy constructor for `Fraction` looks like this:

```cpp
class Fraction
{
public:
    Fraction(const Fraction& other)            // the copy constructor
        : m_numerator { other.m_numerator }
        , m_denominator { other.m_denominator }
    {
    }

private:
    int m_numerator {};
    int m_denominator { 1 };
};
```

The parameter is the object being copied *from*. Notice it accesses `other`'s private members directly — legal, because (14.5) access is per-class and we're inside a `Fraction` member.

### Why the parameter must be a reference

The parameter of a copy constructor must be a reference — conventionally `const ClassName&`. This isn't a style preference; it's a logical necessity. Imagine it took the parameter *by value* instead:

```cpp
Fraction(Fraction other); // imagine this — a disaster
```

To pass an argument by value, the compiler must *copy* it into the parameter. But copying a `Fraction` is what the copy constructor *is*. So passing by value would require calling the copy constructor to set up the call to the copy constructor — forever:

```text
copy ctor takes Fraction by value
   -> needs a copy to create the parameter
   -> calls the copy ctor
   -> needs a copy to create THAT parameter
   -> calls the copy ctor ... (infinite)
```

Taking the parameter by reference breaks the loop: a reference binds to the existing object without copying it. And `const` reference, specifically, because copying shouldn't modify the source.

> **Key insight:** The copy constructor takes its argument by `const` reference because taking it by value would require a copy to make the copy — infinite recursion. A reference binds without copying, which is the only way out.

### The implicit copy constructor

In the vast majority of cases you don't write a copy constructor at all. If you don't, the compiler generates an **implicit copy constructor** that copies each member, one by one. For a `Fraction` — two `int`s — that member-wise copy is exactly what you want.

```cpp
Fraction a { 1, 2 };
Fraction b { a }; // uses the compiler-generated copy constructor — copies both ints
```

> **Best practice:** Prefer the implicit (compiler-generated) copy constructor. Only write your own when member-wise copying isn't the right behavior — which, for types that own a raw resource, it sometimes isn't. For ordinary value types, the implicit one is correct and you should leave it alone.

### Requesting or forbidding copies explicitly

Two tools let you state your intent about copying directly. Ask for the default copy behavior with `= default`:

```cpp
Fraction(const Fraction&) = default; // "generate the standard member-wise copy"
```

Or forbid copying entirely with `= delete`:

```cpp
Fraction(const Fraction&) = delete; // "this type may not be copied"
```

A deleted copy constructor makes any attempt to copy the object a *compile error*. That's exactly what you want for types representing a unique resource — a thing there should only ever be one of, like a sole owner of a file or a piece of hardware. You'll meet such types properly in later chapters; for now, recognize `= delete` as "copying this is forbidden, on purpose."

---

## 14.15 — Class initialization and copy elision

### The copies you imagine aren't always there

Look at this code through the lens of everything we've said about copying:

```cpp
Fraction make()
{
    return Fraction { 1, 2 };
}

Fraction f { make() };
```

A naïve mental model says this performs *two* copies:

```text
construct a temporary inside make()
   -> copy it into make()'s return value
   -> copy that into f
```

If that were really happening, returning objects by value would be wasteful, and you might be tempted to contort your code to avoid it. Here's the good news: in modern C++, those copies usually don't happen at all.

### Copy elision

**Copy elision** is an optimization in which the compiler *omits* an unnecessary copy and instead constructs the object directly in its final destination.

```text
instead of:  temp  ->  copy  ->  f
the compiler does:  construct f directly
```

Rather than build a temporary and copy it into `f`, the compiler builds the result *as* `f` from the start. No temporary, no copy. And under C++17, some forms of this elision aren't merely *allowed* — they're **mandatory**: the standard requires the compiler to elide the copy in cases like returning a temporary by value. It's not a maybe; it's guaranteed.

> **Key insight:** C++17 guarantees that certain copies are elided entirely. Returning a freshly-constructed object by value doesn't pay for a copy — the compiler builds the result directly where it belongs. This is *why* returning by value is both clean and efficient in modern C++.

### What this means for how you write code

The practical consequence is liberating: **return objects by value and don't worry about phantom copies.**

```cpp
CoveragePoint makePoint(int line, int column)
{
    return CoveragePoint { line, column }; // clean, and free thanks to elision
}
```

You'll sometimes see older code twist itself into knots — passing an empty object in by reference for the function to fill, just to "avoid a copy" that elision was going to eliminate anyway. Don't imitate that. A function that *creates* a value should *return* it. The chapter's lab uses this directly: a `multipliedBy` method computes a new `Fraction` and returns it by value, and copy elision makes that return cost nothing.

> **Best practice:** Write functions that produce a new object as returning it by value. Don't contort the design into out-parameters to dodge copies — C++17 copy elision has already handled it.

---

## 14.16 — Converting constructors and the explicit keyword

### A single-argument constructor can convert implicitly

A constructor that can be called with a single argument has a side effect you might not expect: the compiler will use it to *implicitly convert* that argument type into your class type, wherever such a conversion would make a call work.

```cpp
class Token
{
public:
    Token(int id)
        : m_id { id }
    {
    }

private:
    int m_id {};
};

void process(Token token);

process(5); // compiles! int 5 is implicitly converted to Token { 5 }
```

`process` wants a `Token`, you passed an `int`, and the compiler noticed that `Token` has a constructor taking an `int` — so it silently built a `Token` from your `5`. A constructor used this way is called a **converting constructor**.

Sometimes that's convenient. Often it's a trap: you might pass a number meaning one thing and have it silently turn into a `Token`, with no error to warn you that you fed the wrong kind of value to the function.

### `explicit` blocks the implicit path

The `explicit` keyword on a constructor tells the compiler: *don't use this for implicit conversions.* The object can still be constructed — just not behind the caller's back.

```cpp
class Token
{
public:
    explicit Token(int id)
        : m_id { id }
    {
    }

private:
    int m_id {};
};

void process(Token token);

// process(5);          // ERROR now — no implicit int -> Token
process(Token { 5 });   // OK — the conversion is spelled out, intent is clear
```

With `explicit`, the surprising `process(5)` no longer compiles. The caller must write `Token { 5 }`, which makes the construction visible and deliberate. You've traded a little brevity for a lot of safety.

> **Best practice:** Make single-argument constructors `explicit` by default. Drop `explicit` only when you have a specific, deliberate reason to want the implicit conversion. The chapter's lab marks its whole-number `Fraction(int)` constructor `explicit` for exactly this reason — so a stray `int` can't silently masquerade as a `Fraction`.

### Only one user-defined conversion in a chain

A guard rail worth knowing: C++ permits at most *one* user-defined conversion in an implicit conversion sequence. The compiler won't chain your `int -> Token` through a second custom conversion to reach some third type. This prevents the wildest implicit-conversion surprises — but it's not a reason to *rely* on implicit conversions. The cleaner design is to not depend on surprising conversions in the first place, which is what `explicit` helps you achieve.

---

## 14.17 — Constexpr aggregates and classes

### Constructing objects at compile time

You met `constexpr` for values and functions in earlier chapters. It extends to classes too: with the right ingredients, you can construct and use class objects entirely at **compile time**, with zero runtime cost.

The key is marking the relevant members `constexpr`:

```cpp
class Point
{
public:
    constexpr Point(int x, int y) // constexpr constructor
        : m_x { x }
        , m_y { y }
    {
    }

    constexpr int x() const { return m_x; } // constexpr accessor

private:
    int m_x {};
    int m_y {};
};

constexpr Point p { 1, 2 };       // constructed at compile time
static_assert(p.x() == 1);        // checked at compile time
```

A member function may be `constexpr` if it's capable of being evaluated at compile time when called in a constant-expression context. The `constexpr` constructor lets `Point p` itself be `constexpr`; the `constexpr` accessor lets `p.x()` be used inside `static_assert`, which is evaluated by the compiler before your program ever runs.

### What it takes for an object to be constexpr

For a class object to be `constexpr`, its construction and any operations you perform on it in the constant-expression context must all be valid at compile time — which in turn requires the constructor (and those operations) to be `constexpr`. This is most useful for small, value-like types and for compile-time configuration: lookup tables, fixed coordinates, configuration constants baked in before the program starts.

### `constexpr` and `const` are not the same thing

A common point of confusion: `constexpr` does not automatically mean `const` in the member-function sense. Whether a `constexpr` member function is also `const` depends on the rules and the standard version. For your purposes while learning, the safe and conventional habit is to mark accessor-style `constexpr` member functions `const` as well — they only read, so the `const` belongs there anyway:

```cpp
constexpr int value() const; // both: evaluable at compile time AND read-only
```

That combination — `constexpr` for compile-time use, `const` for read-only intent — is the form you'll write most often.

---

## 14.x — Chapter 14 summary and quiz

### The big picture

This chapter took you from *using* classes to *writing* them, and the through-line was a single idea: a class lets you wrap data behind a controlled interface so the object can guarantee its own validity. Everything else served that goal.

- **OOP** bundles data and behavior into objects. In C++, even fundamental variables are technically objects, but *class-type* objects are the ones that add member data and member functions.
- **Classes exist to protect invariants.** Private data plus guarding member functions is how an object refuses to enter an invalid state.
- **Member functions** operate on an **implicit object** — the thing before the dot — so unqualified member names mean "this object's member."
- **Mark read-only member functions `const`.** Const objects and const references (the normal way to pass class objects) can only call const members.
- **`struct` defaults to public, `class` to private** — and that default is their only technical difference. Use `struct` for passive data, `class` for objects with invariants.
- **Access functions** offer controlled reads and writes — but don't generate getters and setters mindlessly; each is part of your public surface.
- **Returning references to data members** avoids copies but creates lifetime and encapsulation risks; never return a non-const reference to private data casually.
- **Constructors** initialize objects and should establish a valid initial state. They're named for the class, have no return type, and are never `const`.
- **Prefer the member initializer list** over assignment in the body. Members initialize in **declaration order**, so list them in that order.
- Use **`= default`** to request a generated default (or copy) constructor; **`= delete`** to forbid an operation.
- **Delegating constructors** call another constructor from the initializer list to share setup logic.
- **Temporary objects** have short lifetimes — until the end of the full expression — so never hold a reference into one.
- **Prefer the implicit copy constructor** unless member-wise copying is wrong for your type.
- **Copy elision** (mandatory in some C++17 cases) makes returning by value clean and free.
- **Make single-argument constructors `explicit`** by default to block surprising implicit conversions.
- **`constexpr`** classes and member functions can participate in compile-time evaluation.

### The Java-to-C++ bridge

If Java is your background, here are the mental adjustments this chapter demands:

| Java intuition | C++ correction |
|---|---|
| A class variable is a reference to a heap object | A C++ class object can live directly as a value, on the stack |
| The constructor initializes the object | Same idea — but the member initializer list matters, and runs in declaration order |
| Methods can be called on any (non-null) reference | C++ has `const` objects and `const` member functions; constness restricts which members you may call |
| Assigning one object variable to another shares a reference | Assigning C++ class objects copies them by value |
| The garbage collector reclaims objects eventually | C++ object lifetime is deterministic and scope-based — destroyed at the end of scope, predictably |

### Reading real class APIs

The large C++ libraries you'll meet later are built from exactly these pieces. A few lines of LLVM-style code, decoded with this chapter's vocabulary:

```cpp
DebugLoc loc = inst.getDebugLoc(); // getDebugLoc() is a member function called on inst
```

```cpp
auto* mod = func.getParent(); // a member function that returns a pointer
```

```cpp
mod->getOrInsertFunction(/* ... */); // -> calls a member function through a pointer
```

Nothing there is new anymore. A member function call on an object, a function returning a pointer, the `->` form of member access through a pointer — you can read all of it now.

### A worked example

Here's a small class that exercises most of the chapter at once — private data, a member initializer list, an `explicit` constructor, a mutating member function, and `const` accessors:

```cpp
#include <iostream>
#include <string>
#include <string_view>

class CampaignStats
{
public:
    explicit CampaignStats(std::string name) // explicit single-arg constructor
        : m_name { name }
    {
    }

    void recordTest(bool crashed) // mutates: not const
    {
        ++m_tests;
        if (crashed)
            ++m_crashes;
    }

    std::string_view name() const { return m_name; } // const accessors below
    int tests() const { return m_tests; }
    int crashes() const { return m_crashes; }

private:
    std::string m_name {};
    int m_tests {};
    int m_crashes {};
};

int main()
{
    CampaignStats stats { "MutationA" };
    stats.recordTest(false);
    stats.recordTest(true);

    std::cout << stats.name() << ": "
              << stats.crashes() << '/'
              << stats.tests() << " crashes\n";
}
```

Trace what each piece demonstrates: the `explicit` constructor blocks an accidental `std::string -> CampaignStats` conversion; the member initializer list sets `m_name` from the argument; `recordTest` mutates state and so is *not* `const`; the three accessors only read and so *are* `const`; `m_name` is an owned `std::string` while `name()` returns a lightweight read-only `std::string_view`.

### Where this is headed: the lab

The chapter's lab puts all of this into a single class you build yourself: a **`Fraction`** — a rational number stored as `numerator / denominator` with the invariant that the denominator is never zero and is always kept positive (the numerator carries any negative sign). You'll write:

- a **default constructor** (`0/1`) using a member initializer list,
- a **two-argument constructor** that stores the values, guards against a zero denominator in its body, and calls a provided `reduce()` helper to normalize the sign and divide out the GCD,
- an **`explicit` whole-number constructor** so a stray `int` can't silently become a `Fraction`,
- **`const` accessors** for the numerator and denominator,
- an **`equals`** method that reads another `Fraction`'s private fields directly — legal because access is per-class,
- a **`multipliedBy`** method that computes a new `Fraction` and returns it by value, leaning on copy elision to make that free.

Every one of those tasks is a concept from this chapter made concrete. By the time the grader turns green, you won't just have read about classes — you'll have built one that *cannot* hold an invalid value, which is the entire point.
