# Chapter 15 — More on Classes

> Source: <https://www.learncpp.com/> (Chapter 15)

In Chapter 14 you learned to *build* a class: data members, access specifiers, constructors, member-initialization lists, `const` member functions. You can now define a type and hand callers a clean public interface. This chapter is about everything that makes classes *practical at scale* — the machinery that turns a textbook class into one you'd actually ship.

We'll start with a small mystery: when you write `account.deposit(50)`, how does `deposit` know *which* account to modify? The answer is a hidden pointer named `this`, and once you understand it, two useful patterns fall out for free — method chaining and explicit member access. From there we'll cover the questions a real codebase forces on you. *Where* does a class live when several files need it (header and source split)? How do you scope a helper type so its name travels with the class that owns it (nested types)? What runs when an object dies, and how does C++ guarantee cleanup happens (destructors and RAII)? How do you give a class a type parameter so one definition serves many element types (class templates)? How do you attach data and functions to the *class itself* rather than to each object (static members)? And when does it make sense to let an outside function reach into a class's private internals (friendship)? We close with ref qualifiers, an advanced tool you'll mostly *read* rather than write. By the end you'll have the full vocabulary that production C++ — and the LLVM code you'll meet in your studies — assumes you already know.

## Contents
- [15.1 — The hidden this pointer and member function chaining](#151--the-hidden-this-pointer-and-member-function-chaining)
- [15.2 — Classes and header files](#152--classes-and-header-files)
- [15.3 — Nested types (member types)](#153--nested-types-member-types)
- [15.4 — Introduction to destructors](#154--introduction-to-destructors)
- [15.5 — Class templates with member functions](#155--class-templates-with-member-functions)
- [15.6 — Static member variables](#156--static-member-variables)
- [15.7 — Static member functions](#157--static-member-functions)
- [15.8 — Friend non-member functions](#158--friend-non-member-functions)
- [15.9 — Friend classes and friend member functions](#159--friend-classes-and-friend-member-functions)
- [15.10 — Ref qualifiers](#1510--ref-qualifiers)
- [15.x — Chapter 15 summary and quiz](#15x--chapter-15-summary-and-quiz)

---

## 15.1 — The hidden this pointer and member function chaining

### One function, many objects

Here is a puzzle worth pausing on. Suppose you have a tiny counter class:

```cpp
class Counter
{
public:
    void increment() { ++m_value; }

private:
    int m_value {};
};
```

There is exactly *one* `increment` function in your whole program — the compiler doesn't make a fresh copy per object. Yet you can have many counters:

```cpp
Counter a {};
Counter b {};

a.increment();  // bumps a's m_value
b.increment();  // bumps b's m_value
```

The same single function modifies `a`'s data on the first call and `b`'s data on the second. So when `increment` writes `++m_value`, *which* `m_value` does it mean? There's nothing in the function body that names `a` or `b`. How does it know?

### The hidden `this` pointer

The answer is that member functions secretly take an extra argument: a pointer to the object you called them on. Inside the function, that pointer has a fixed name — **`this`**. When you write:

```cpp
a.increment();
```

the compiler effectively rewrites it as if `increment` were a free function receiving the object's address:

```
a.increment()

roughly becomes:
Counter::increment(&a)
                    ^
                    this
```

So `this` holds the address of the object the member function is currently working on. On the first call above, `this == &a`; on the second, `this == &b`. And every unqualified member name inside the function is silently understood as "the member of the object `this` points to." `++m_value` is shorthand for `++this->m_value`:

```cpp
void increment()
{
    ++m_value;         // what you write
    // ++this->m_value; // what it means
}
```

That's the whole trick. There is one function, and `this` is how it finds the right object each time it runs.

> **Key insight:** Every non-static member function receives a hidden pointer, `this`, holding the address of the object it was called on. Unqualified member accesses inside the function are implicitly `this->member`. That single pointer is what lets one function body operate on any object.

### Explicit `this`

Because `this->member` is the long form of `member`, you can always write it out — and occasionally you should. The most common reason is a name clash. If a constructor or setter parameter has the *same* name as the member it sets, the bare name refers to the parameter, and the member gets shadowed:

```cpp
class Person
{
public:
    void setName(std::string name)
    {
        this->m_name = name; // left side is the member, right side the parameter
    }

private:
    std::string m_name {};
};
```

Here the members are `m_`-prefixed, so `name` and `m_name` are already distinct and `this->` is just a readability choice. But if you followed a style where the parameter were literally named `m_name` too — or in template code where the compiler needs a nudge to know a name is a member — `this->` becomes *required* to disambiguate. Outside those cases, leave it off; cluttering every access with `this->` adds noise without meaning.

> **Best practice:** Omit `this->` by default. Reach for it only when it resolves a real name conflict or is required in a template context. Prefixing members with `m_` sidesteps most clashes before they happen.

### Returning `*this` enables chaining

`this` is a pointer; dereference it and `*this` is the object itself. That opens a small but powerful pattern. If a member function returns `*this` *by reference*, the caller gets the same object back, ready for another call:

```cpp
class Counter
{
public:
    Counter& increment()
    {
        ++m_value;
        return *this;
    }

    Counter& reset()
    {
        m_value = 0;
        return *this;
    }

private:
    int m_value {};
};
```

Because each call hands back the object, you can stack calls left to right:

```cpp
Counter c {};
c.increment().increment().reset();
```

Read it as a pipeline. `c.increment()` returns a reference to `c`; the next `.increment()` runs on that same `c`; `.reset()` runs on it again:

```
c.increment()  -> returns c
   .increment() -> runs on c again, returns c
      .reset()  -> runs on c
```

The detail that makes this work is the `&` in the return type. `Counter& increment()` returns a *reference* to the live object. Drop the `&` and you'd return a `Counter` — a *copy* — so every chained call would operate on a throwaway temporary, and your original `c` would never change past the first call. You'll meet this pattern again in the IdCard lab, where the grader literally compares the address of the returned reference against the address of the original card to prove no copy was made.

### `this` inside a const member function

Recall from Chapter 14 that a member function marked `const` promises not to modify the object. The mechanism behind that promise is `this`. In an ordinary member function, `this` is a `Counter*`. In a `const` member function, it's a `const Counter*` — a pointer through which the object cannot be modified:

```cpp
int value() const   // this is a const Counter* in here
{
    return m_value; // reading is fine
    // ++m_value;   // error: can't modify through a pointer-to-const
}
```

So "const member function" really means "a member function whose `this` points to const." That is exactly why such a function may read members but not assign to them — the type system forbids it. (There is a narrow escape hatch, the `mutable` keyword, which lets a specific member stay writable even through a const `this`; it exists for caches and bookkeeping, and is not something to reach for casually.)

### Why `this` is a pointer, not a reference

You may wonder why `this` is a pointer when references would read more cleanly. The honest answer is historical: `this` predates references in C++'s evolution, and changing it now would break a staggering amount of code. The practical consequence is the arrow syntax — you write `this->member`, not `this.member`, because `this` is a pointer and `->` is how you reach through a pointer to a member.

---

## 15.2 — Classes and header files

### Where should a class definition live?

The classes you've written so far have all sat in a single `.cpp` file alongside `main`. That's fine for a one-file program, but real projects span many files, and often several of them need the *same* class type. A logging module, the code that creates objects, the tests — all of them need to know what a `CampaignStats` looks like. You can't copy the class definition into each file by hand; they'd drift out of sync, and you'd be maintaining three definitions of one type.

The solution is the same one you used for functions in Chapter 2: put the shared thing in a **header file** and `#include` it wherever it's needed. For a class, the header holds the class *definition* — the full `class { ... };` block listing members and declaring member functions. Each file that includes the header gets the complete picture.

```cpp
// CampaignStats.h
#ifndef CAMPAIGN_STATS_H
#define CAMPAIGN_STATS_H

#include <string>
#include <string_view>

class CampaignStats
{
public:
    explicit CampaignStats(std::string name);
    void recordTest(bool crashed);
    std::string_view name() const;

private:
    std::string m_name {};
    int m_tests {};
    int m_crashes {};
};

#endif
```

Notice the header *declares* the member functions but doesn't define their bodies (except the trivial cases we'll discuss below). The bodies go in a matching `.cpp` file.

### Defining members out of class

When a member function's body lives outside the class definition, you must tell the compiler which class it belongs to, using the scope-resolution operator `ClassName::`:

```cpp
// CampaignStats.cpp
#include "CampaignStats.h"

CampaignStats::CampaignStats(std::string name)
    : m_name { name }
{
}

std::string_view CampaignStats::name() const
{
    return m_name;
}
```

`CampaignStats::name` reads as "the `name` function that belongs to `CampaignStats`." Without that qualifier the compiler would think you're defining a free function and complain that it can't find the members. The convention is simple and universal:

```
CampaignStats.h    // the class definition + member declarations
CampaignStats.cpp  // the out-of-class member definitions
```

This header/source split is exactly how the IdCard lab is structured, and exactly how industrial code is laid out — `llvm/IR/Value.h` declares the `Value` class; `Value.cpp` defines its members.

### Does a class in a header violate the one-definition rule?

A fair worry: if five `.cpp` files all `#include "CampaignStats.h"`, doesn't the program end up with five definitions of `CampaignStats`? The one-definition rule (ODR) forbids defining the same entity twice, after all.

C++ carves out an explicit exception for class types: a class may be defined in multiple translation units **provided every definition is identical** — which is automatically true when they all come from the same header. The include guard (`#ifndef ... #define ... #endif`) handles a different problem: it stops the header from being pasted in *twice within a single* translation unit. The two mechanisms work together — guards prevent duplication inside one file; the ODR exception permits the identical definition across files.

> **Key insight:** Putting a class definition in a header does not break the ODR, because the standard explicitly allows identical class definitions across translation units. Include guards prevent double-inclusion within one file; the ODR exception covers the rest.

### Member functions defined in-class are implicitly inline

You've already seen tiny member functions defined right inside the class:

```cpp
class Counter
{
public:
    int value() const { return m_value; } // defined in-class

private:
    int m_value {};
};
```

A function defined inside the class definition is automatically **inline**. In modern C++, "inline" is mostly about *linkage*, not speed: it tells the linker "this definition may legitimately appear in several translation units; treat them as one and don't complain." That's precisely what lets a header-defined member function be included everywhere without triggering a duplicate-definition error. It does *not* promise the compiler will physically paste the function body into each call site — that optimization is a separate decision the compiler makes on its own.

### Why not put everything in the header?

If headers are so convenient, why split anything into a `.cpp` at all? Because cramming implementation into headers has real costs:

- **Compile time.** Every file that includes the header recompiles all that code. Change one function body and every dependent file rebuilds.
- **Leaked detail.** Implementation in the header is implementation exposed to every client, inviting them to depend on things they shouldn't.
- **Ripple effects.** A change to a header forces recompilation of everything downstream; a change confined to a `.cpp` rebuilds only that one file.

The working rule: keep the class *definition* and small, stable accessors in the header; put substantial member bodies in the `.cpp`. The one firm exception is templates, which — as you'll see in 15.5 — generally *must* be fully visible in the header.

> **Best practice:** Header gets the class definition and trivial inline accessors. The `.cpp` gets the heavier member bodies. This keeps compile times down and implementation details out of clients' sight — except for templates, which need their definitions in the header.

### Default arguments belong in the declaration

If a member function has a default argument, write the default *once*, in the header declaration — never repeat it in the `.cpp` definition:

```cpp
// header
void resize(int width, int height = 100);
```

```cpp
// cpp — no default here
void Window::resize(int width, int height)
{
}
```

Putting the default in both places is an error: the compiler would see the same default specified twice. The IdCard lab makes a point of this — its constructor has default arguments declared in `idcard.h`, and your job is to define the body in `idcard.cpp` *without* restating them.

---

## 15.3 — Nested types (member types)

### A type that belongs to a class

Sometimes a helper type only makes sense *in the context of* a particular class. A parser has parser states. A card has card roles. A coverage tracker has coverage points. These types have no independent life of their own — naming them at file scope would scatter them away from the class they serve and clutter the surrounding namespace.

C++ lets you define a type *inside* a class. Such a **nested type** (or **member type**) belongs to the class the same way a member variable or member function does:

```cpp
class Fuzzer
{
public:
    enum class Campaign
    {
        MutationA,
        MutationB,
        MutationC,
    };
};
```

To name the type from outside, qualify it with the enclosing class:

```cpp
Fuzzer::Campaign campaign { Fuzzer::Campaign::MutationA };
```

The payoff is scoping and intent. `Fuzzer::Campaign` reads as "the kind of campaign a fuzzer runs," and the name `Campaign` doesn't collide with any other `Campaign` elsewhere in your program. You'll do exactly this in the IdCard lab, where `Role` is a nested `enum class` — `IdCard::Role::Engineer` — because a role only means something attached to a card.

### Nested type aliases

Nested types aren't limited to enums. A `using` alias works just as well, and it's a tidy way to keep a class's vocabulary near the class itself:

```cpp
#include <set>
#include <utility>

class CoverageTracker
{
public:
    using Point = std::pair<int, int>;
    using PointSet = std::set<Point>;
};
```

Now callers (and the class's own members) can write `CoverageTracker::Point` instead of spelling out `std::pair<int, int>` everywhere. The standard library leans on this heavily — `std::vector<T>::size_type` and `std::string::iterator` are nested aliases of exactly this kind.

### Access control applies to nested types

A nested type obeys the same `public`/`private` rules as any other member. If callers need to name the type, make it public. If it's a private implementation detail — a state machine's internal states, say — keep it private and the outside world can't even refer to it:

```cpp
class Parser
{
private:
    enum class State
    {
        Start,
        InNumber,
        Done,
    };

    State m_state { State::Start };
};
```

The default rule is the familiar one: expose only what callers genuinely need.

### A nested class is just a class

You can nest a full class, not only an enum or alias — but be careful about expectations carried over from other languages. A nested class in C++ is an ordinary, independent class that happens to be *scoped* inside another. It does **not** automatically hold a reference to an enclosing object the way an inner class does in some languages. If a nested class needs to talk to an instance of the outer class, you must hand it that instance explicitly (typically as a constructor argument or function parameter). Nesting in C++ is about *name scoping*, not about an implicit parent-child link.

> **Warning:** A nested class does not get an implicit pointer to an enclosing object. If you've used inner classes in Java, drop that intuition here — in C++, nesting only controls where the type's name lives. To act on an outer object, the nested class must be given one explicitly.

---

## 15.4 — Introduction to destructors

### The cleanup problem

A constructor's job is to get an object *ready* — to acquire whatever it needs to do its work. But many objects acquire things that must later be *given back*:

- heap memory,
- an open file handle,
- a network socket,
- a lock,
- a temporary file on disk,
- a database connection.

If you acquire one of these and never release it, you leak the resource — the file stays open, the lock stays held, the memory is never returned. The acquisition has an obvious home (the constructor). What C++ needs is an equally reliable home for the *release*. That home is the destructor.

### What a destructor is

A **destructor** is a special member function that runs automatically when an object is destroyed. Its name is the class name with a leading tilde:

```cpp
#include <iostream>

class Trace
{
public:
    ~Trace()
    {
        std::cout << "destroyed\n";
    }
};
```

A destructor takes **no parameters** and has **no return type** — there's nothing to pass it and nothing for it to return; it exists purely to clean up. A class has exactly one destructor (unlike constructors, which you can overload).

### Destruction is tied to scope

The reason destructors are so powerful in C++ is *when* they run: for a local object, the destructor fires automatically the moment the object goes out of scope, with no call from you.

```cpp
{
    Trace t {}; // constructed here
} // t goes out of scope here -> destructor runs automatically
```

The lifecycle is completely deterministic:

```
enter scope -> construct t -> use t -> leave scope -> destructor runs
```

This is the foundation of one of C++'s defining ideas: **RAII**, *Resource Acquisition Is Initialization*. The pattern is to acquire a resource in the constructor and release it in the destructor. Then, because the destructor is guaranteed to run when the object leaves scope, the resource is guaranteed to be released — even if an exception fires or the function returns early. You bind the resource's lifetime to an object's lifetime, and scope takes care of the rest. The IdCard lab makes this visible without any heap memory at all: the destructor decrements a live-card counter, and because the test creates cards in nested `{ }` blocks, you can watch the count fall by exactly the right amount the instant each block closes.

> **Key insight:** A local object's destructor runs automatically when it leaves scope. Bind a resource's lifetime to an object (acquire in the constructor, release in the destructor) and cleanup becomes automatic and exception-safe. This is RAII — the single most important idiom for resource management in C++.

### The implicit destructor

If you don't write a destructor, the compiler generates one for you. For most classes that's all you need — when an object is destroyed, each of its members is destroyed in turn, and well-behaved members clean up after themselves:

```cpp
#include <string>
#include <vector>

class SeedCorpus
{
private:
    std::vector<std::string> m_seeds {};
}; // implicit destructor destroys m_seeds, which frees its strings
```

`SeedCorpus` holds a `std::vector` of `std::string`s, both of which manage their own memory. When a `SeedCorpus` dies, the compiler-generated destructor destroys the vector, which destroys each string, which frees its buffer — a whole tree of cleanup, automatic and correct, without you writing a single line. You only need to *write* a destructor when your class manages a raw resource the members don't clean up on their own (you'll hit that in Chapter 19 with `new`/`delete`).

### A caution about `std::exit`

There's one important way to *bypass* destructors: calling `std::exit()`. It terminates the program immediately and does **not** unwind the stack, so destructors of local automatic objects never run — and any RAII cleanup they would have performed is skipped. Prefer returning normally from `main` (or letting an exception propagate) so that scopes exit in the usual way and every destructor gets its turn. Reach for `std::exit` only when an abrupt halt is genuinely what you intend.

> **Warning:** `std::exit()` does not destroy local objects, so RAII cleanup is skipped. Favor normal returns over `std::exit` unless you specifically want the process to stop without unwinding.

---

## 15.5 — Class templates with member functions

### A class with a type left blank

In Chapter 11 you wrote function templates — one function body with the type left as a parameter, so the compiler could stamp out a version per type. Classes can be parameterized the same way. A **class template** is a class with one or more type parameters; the compiler generates a concrete class for each set of type arguments you use.

The motivation is identical to function templates. A "box that holds one value" works the same whether the value is an `int`, a `std::string`, or anything else. Writing `IntBox`, `StringBox`, and so on by hand would be the same copy-paste tax overloading saved you from. Instead, parameterize:

```cpp
template <typename T>
class Box
{
public:
    explicit Box(T value)
        : m_value { value }
    {
    }

    const T& value() const
    {
        return m_value;
    }

private:
    T m_value {};
};
```

You supply the type argument in angle brackets when you create an object:

```cpp
Box<int> i { 5 };
Box<std::string> s { "hello" };
```

`Box<int>` and `Box<std::string>` are genuinely different types, each generated from the one template. You've been using class templates all along — `std::vector<int>`, `std::optional<double>`, and `std::pair<int, int>` are all instantiations of class templates.

### Defining members outside a class template

In-class member definitions (like the two above) need nothing special. But when you define a class template's members *outside* the class, each definition must be reintroduced as a template, and the class name carries its parameter list:

```cpp
template <typename T>
class Box
{
public:
    explicit Box(T value);
    const T& value() const;

private:
    T m_value {};
};

template <typename T>
Box<T>::Box(T value)
    : m_value { value }
{
}

template <typename T>
const T& Box<T>::value() const
{
    return m_value;
}
```

Two things to read carefully. First, every out-of-class definition is preceded by its own `template <typename T>` — that's not boilerplate decoration, it tells the compiler "this is a template definition," and omitting it is an error. Second, the class name in the qualifier is `Box<T>`, not plain `Box`, because outside the class you must say *which* `Box` — the one parameterized on `T`.

### The injected class name

There's a small convenience that softens that second rule. *Inside* the body of `Box<T>` — including inside its out-of-class member definitions, after the `Box<T>::` part — you can usually refer to the class as just `Box`, and the compiler understands you mean `Box<T>`. This is called the **injected class name**: the template's own name is automatically available, unqualified, within its scope. Outside the class's scope, though, be explicit and write `Box<T>` (or a concrete instantiation like `Box<int>`).

### Class templates live in headers

Recall from Chapter 11 *why* function templates must live in headers: the compiler can only generate a concrete version when it can see the template's full definition at the point of use. Class templates inherit the same rule. The compiler can't instantiate `Box<std::string>` from a mere declaration — it needs the complete template, member bodies and all, visible wherever you use it. So a class template's definition *and* its member definitions go in the header together.

> **Best practice:** Define class templates and their members entirely in a header. Unlike ordinary classes — where heavy member bodies belong in a `.cpp` — a template needs its full definition visible at every point of instantiation, so the header is the correct home.

---

## 15.6 — Static member variables

### Per-object data versus per-class data

Every member variable you've written so far is a *per-object* member: each object gets its own copy. Two `Counter`s have two independent `m_value`s. That's usually what you want — an object's state is its own.

But occasionally a piece of data belongs to the *type itself*, not to any one object. How many `Counter`s have ever been created? That's not a property of any single counter — it's a property of the whole class, shared by all of them. For data like this, C++ provides the **static member variable**: a single variable that lives with the class, not duplicated per object.

```cpp
class Counter
{
public:
    Counter()
    {
        ++s_created; // every constructor bumps the one shared counter
    }

    static int s_created;
};

int Counter::s_created { 0 }; // definition lives outside the class
```

The picture to hold in mind:

```
Counter a:  has its own m_value
Counter b:  has its own m_value

class Counter:  s_created — one variable, shared by a, b, and every other Counter
```

Every `Counter` constructor increments the *same* `s_created`, so after creating three counters it reads `3`. The `s_` prefix is a common convention marking a member as static, just as `m_` marks an ordinary member.

### Accessing a static member

Because a static member belongs to the class rather than an object, you reach it through the class name with the scope-resolution operator — no object required:

```cpp
std::cout << Counter::s_created << '\n';
```

You *can* also access it through an object (`a.s_created`), and it'll compile, but that reads as though it were per-object state, which is exactly the wrong impression. Prefer `Counter::s_created` to make the class-level nature obvious.

> **Best practice:** Access static members through the class name (`Counter::s_created`), not through an object. The class-name form signals that the data belongs to the type, not to any instance.

### Initializing static members

A non-inline static data member is *declared* inside the class but must be *defined* once, outside it — which is what `int Counter::s_created { 0 };` above does. That definition belongs in a `.cpp` file (not the header), or you'll get multiple definitions across translation units.

That separate-definition step is a classic source of "undefined reference" link errors, so modern C++ offers a cleaner option: mark the member `inline` and initialize it right in the class, no out-of-class definition needed:

```cpp
class Counter
{
public:
    inline static int s_created { 0 }; // declared and defined in one place
};
```

This is usually the friendlier choice for new code, and it's exactly what the IdCard lab uses for its `s_liveCount` and `s_nextSerial`.

### Static constants

A very common use of static members is class-level constants. Marking such a constant `static constexpr` gives you a single, compile-time-known value attached to the class:

```cpp
class Config
{
public:
    static constexpr int maxTests { 10000 };
};
```

`Config::maxTests` is a named constant scoped to `Config`. (A `static constexpr` member is implicitly inline, so it needs no separate out-of-class definition — the value is right there in the class.) This is the idiomatic way to give a class named, type-associated constants.

---

## 15.7 — Static member functions

### A function that belongs to the class

Just as data can belong to the class rather than an object, so can a function. A **static member function** is called on the class, not on an instance:

```cpp
class Random
{
public:
    static int get(int min, int max);
};

int value { Random::get(1, 6) }; // called through the class, no object
```

You invoke it with `Random::get(...)` — there's no `Random` object in sight, and none is needed. Static member functions are the natural partner to static member variables: a static function to read a static count, for instance.

### No `this` pointer

Here is the defining property, and the reason everything else about static member functions follows. A static member function has **no `this` pointer** — because it isn't called on any object, there's no "current object" for `this` to point at.

That single fact tells you exactly what a static member function can and can't touch. It can use:

- static members (they belong to the class, which is always available),
- objects passed in as arguments,
- globals.

But it *cannot* directly access the class's non-static members, because those live in some object — and a static function has no object:

```cpp
class Counter
{
public:
    static int created()
    {
        return s_created; // ok: static, belongs to the class
        // return m_value; // error: which object's m_value? there isn't one
    }

private:
    int m_value {};
    inline static int s_created {};
};
```

`s_created` is fine — it's the class's, and the class is always there. `m_value` is an error — there's no object whose `m_value` we could mean. The IdCard lab's `liveCount()` and `nextSerial()` are static for exactly this reason: they report class-wide bookkeeping, so they need no object and touch only static members.

> **Key insight:** A static member function has no `this`, so it can read static members, its arguments, and globals — but it cannot touch the class's non-static members, because there is no object for them to belong to.

### Pure-static class, or a namespace?

If you find yourself writing a class that has *only* static members and no per-object state at all, pause. A class is for bundling object state with the functions that operate on it; a class with no state isn't really doing that job. Often a plain **namespace** expresses the same grouping more honestly:

```cpp
namespace Random
{
    int get(int min, int max);
}
```

A namespace groups related free functions without pretending there's an object to instantiate. Reach for a class when there's genuine per-object state; reach for a namespace when you just want a named home for related functions and constants.

One last note for readers coming from other languages: C++ has no "static constructor" that runs once to initialize class-level state. Static initialization follows its own rules (and has its own well-known pitfalls around initialization order across translation units), so don't expect a class-level setup hook to fire automatically.

---

## 15.8 — Friend non-member functions

### Granting access from the inside

Encapsulation is the whole point of `private`: outside code can't reach a class's internals. But every so often a function that is *logically* part of a class's interface has to live *outside* the class — and it still needs the private data to do its job. The classic example is a function that prints an object: by convention it's a non-member, yet it has to read the very fields you've kept private.

C++'s answer is **friendship**. A class can name a specific outside function as a *friend*, and that function — though not a member — gets full access to the class's private and protected members:

```cpp
#include <iostream>

class Point
{
public:
    Point(int x, int y)
        : m_x { x }, m_y { y }
    {
    }

    friend void print(const Point& p); // grants print access to privates

private:
    int m_x {};
    int m_y {};
};

void print(const Point& p)
{
    std::cout << p.m_x << ',' << p.m_y << '\n'; // reaches private members
}
```

The crucial direction here: **friendship is granted by the class, from the inside.** An outside function can't *declare itself* a friend and force its way in — that would defeat encapsulation entirely. The class chooses, deliberately, whom to trust. A `friend` declaration is the class saying "I extend my private access to this specific function."

### The common case: stream operators

The single most common reason to use a friend function is the stream-output operator. To let `std::cout << myPoint` work, you write an `operator<<` — which, for reasons you'll learn in Chapter 21, must be a *non-member* function — yet it needs to read the object's private fields to print them. Friendship bridges the gap:

```cpp
#include <iostream>

class Point
{
public:
    friend std::ostream& operator<<(std::ostream& out, const Point& p)
    {
        return out << p.m_x << ',' << p.m_y; // private access via friendship
    }

private:
    int m_x {};
    int m_y {};
};
```

(Operator overloading is formally Chapter 21 — here it's just a preview of *why* friendship exists. The IdCard lab lists a friend `operator<<` only as an optional stretch goal for the same reason.) This pattern is so standard that any streamable class you meet in the wild is likely to use it.

### Does friendship break encapsulation?

It's a reasonable concern, and the answer is "only if you abuse it." A friend function *is* given private access, so a sprawling web of friends would indeed erode your class's boundaries. But a small, focused friend — a stream operator, a closely-related helper — is better understood as *part of the class's interface that happens to be written as a non-member*. The class explicitly opted in; the access isn't a leak, it's a deliberate extension. The guidance is about restraint:

> **Best practice:** Prefer a non-friend function whenever the public interface is enough. Use `friend` when an operation is naturally a non-member but genuinely needs the internals — and keep the set of friends small and intentional. A short friend list is a sign of healthy encapsulation; a long one is a smell.

---

## 15.9 — Friend classes and friend member functions

### Befriending an entire class

Friendship isn't limited to single functions. A class can declare another *class* as a friend, granting *all* of that class's member functions access to its privates:

```cpp
class Storage
{
    friend class StorageInspector; // every member of StorageInspector gets in

private:
    int m_secret {};
};
```

Now any member function of `StorageInspector` can read `Storage::m_secret`. This is occasionally just what you want — two classes designed to work hand in glove, where one legitimately needs deep access to the other. But it's a blunt instrument: you've opened *all* of `Storage`'s internals to *all* of `StorageInspector`'s code, creating tight coupling between the two. Use it sparingly, and only when the two classes are genuinely part of one design.

### Befriending a single member function

A friend *class* is often more access than you need. If only one function of `StorageInspector` actually requires the access, you can befriend just that one member function instead — a far more surgical grant:

```cpp
class Storage; // forward declaration: Inspector mentions Storage before it's defined

class Inspector
{
public:
    void inspect(const Storage& s); // declared, defined later
};

class Storage
{
    friend void Inspector::inspect(const Storage& s); // only this one function

private:
    int m_secret {};
};
```

This narrows the trust to exactly the function that needs it — everything else in `Inspector` stays locked out. The cost is fiddly ordering: `Inspector::inspect` must be *declared* before `Storage` can befriend it, yet `inspect` takes a `Storage&`, so `Storage` must be at least forward-declared first. Getting the forward declarations and definition order right is the price of the precision.

> **Note:** Friend member functions give you the tightest possible grant — one function, not a whole class — but they demand careful ordering of declarations. If the forward-declaration dance starts to feel contorted, treat that as a design signal: heavy friendship requirements often mean two classes are too entangled and the boundary between them should be reconsidered.

---

## 15.10 — Ref qualifiers

### The problem: lvalue objects and rvalue temporaries

A member function can be called on two quite different kinds of object. One is a named, lasting object — an **lvalue**. The other is a nameless **rvalue** temporary that exists only for the duration of the expression:

```cpp
#include <string>

std::string s {};
s.size();                    // called on s, an lvalue (lasting) object
std::string{ "hi" }.size();  // called on a temporary, an rvalue object
```

For a harmless function like `size()` the distinction doesn't matter. But it matters a great deal when a member function returns a *reference into the object's own data*. Returning such a reference is fine for a lasting object — the data outlives the call. From a temporary, it's a trap: the temporary is destroyed at the end of the full expression, and the reference is left dangling, pointing at freed storage. (This is the same lifetime hazard you met in Chapter 14 around returning references.)

### Qualifying a member function by value category

**Ref qualifiers** let a member function specify *which* kind of object it may be called on. A trailing `&` means "only on lvalue objects"; a trailing `&&` means "only on rvalue objects." This lets you offer two behaviors — a safe one for lasting objects, a different one for temporaries:

```cpp
#include <string>
#include <utility>

class Buffer
{
public:
    const std::string& data() const& // lvalue objects: hand back a reference
    {
        return m_data;
    }

    std::string data() && // rvalue objects: the temporary is dying, so move out
    {
        return std::move(m_data);
    }

private:
    std::string m_data {};
};
```

On a lasting `Buffer`, `data()` returns a cheap reference into `m_data` — safe, because the buffer outlives the call. On a temporary `Buffer`, returning a reference would dangle, so the `&&` overload instead *moves* the data out into a returned value (you'll study moves properly in Chapter 22). The two qualifiers route each call to the appropriate version:

| Qualifier | Callable on |
|---|---|
| `&` | lvalue implicit object (a named, lasting object) |
| `&&` | rvalue implicit object (a temporary) |

### When to reach for this

Ref qualifiers are a precision tool, and an advanced one. They genuinely matter when you're writing a type that hands out references to its internals and you want to make misuse on temporaries impossible — but for the everyday classes you'll write as you learn, they're usually unnecessary overhead. The right goal at this stage is *recognition*: when you see `const&` or `&&` after a member function's parameter list in library code, you'll know it's qualifying the function by the value category of the object it's called on. Reach for the feature yourself only when you're deliberately managing value categories — and connect it back to Chapter 14: ref qualifiers exist precisely to prevent dangerous patterns like returning a reference into a temporary.

> **Note:** Ref qualifiers (`&` and `&&` after a member function) are advanced. Learn to *recognize* them in library code now; adopt them in your own classes only when you are deliberately controlling whether a function may run on temporaries versus lasting objects.

---

## 15.x — Chapter 15 summary and quiz

### Core takeaways

You've moved from defining a class to wielding the full machinery a real codebase expects. The throughline:

- **`this`** is a hidden pointer to the object a member function was called on; unqualified member accesses mean `this->member`. It's the mechanism by which one function body serves every object.
- Writing `this->` explicitly is for resolving name clashes or satisfying template rules — otherwise omit it.
- Returning **`*this` by reference** lets calls **chain**; the `&` in the return type is what keeps each call on the original object instead of a copy.
- Inside a `const` member function, `this` points to const — which is *why* such functions can't modify members.
- A class **definition lives in a header**; out-of-class member bodies use `ClassName::member` and live in a `.cpp`. This doesn't violate the ODR (identical class definitions across translation units are allowed).
- Member functions defined **in-class are implicitly inline**, which is what permits header definitions; heavy bodies otherwise belong in a `.cpp` — except for **templates**, which need full definitions in the header.
- **Nested types** scope a helper type to the class that owns it; access control applies, and a nested class is independent (no implicit link to an enclosing object).
- **Destructors** (`~ClassName()`, no params, no return) run automatically at end of scope, powering **RAII** — bind a resource's lifetime to an object and cleanup is guaranteed.
- **Static member variables** belong to the class, not each object; prefer `inline static` for easy in-class initialization, and access them through the class name.
- **Static member functions** have no `this`, so they reach only static members, arguments, and globals. A pure-static class is often better as a namespace.
- **Friend** functions and classes can read private members, but friendship is granted by the class and should stay small and intentional.
- **Ref qualifiers** control whether a member function runs on lvalue or rvalue objects — advanced; recognize them, reach for them rarely.

### The class mechanics map

A single class touching most of the chapter's machinery:

```cpp
class Example
{
public:
    Example();              // constructor (Ch 14)
    ~Example();             // destructor — runs at end of scope

    Example& reset();       // returns *this for chaining
    int value() const;      // const member function: this points to const

    static int created();   // static: no this pointer

private:
    int m_value {};
    inline static int s_created {}; // class-wide, not per object
};
```

### How this connects to the IdCard lab

The chapter's lab, the **IdCard Badge Printer**, exercises nearly all of this at once and the *right* way for a real project — the class declaration in `idcard.h`, your member definitions in `idcard.cpp` using `ClassName::method()` syntax:

- The **constructor** assigns each card a serial from the static `s_nextSerial`, advances it, and bumps the static `s_liveCount` — static bookkeeping in action.
- The **destructor** decrements `s_liveCount`; because tests create cards in nested `{ }` scopes, you *watch* the count fall the instant each scope closes — RAII made physical, with no `new` anywhere.
- The **setters** use the explicit-`this->` idiom and return `IdCard&`, so calls chain — and the grader checks the returned reference's address equals the card's, proving no copy.
- **Const accessors** return private members and are callable on a `const IdCard&`.
- **Static member functions** `liveCount()` and `nextSerial()` read the static state with no `this` — and `static` appears only in the header declaration, never in the `.cpp` definition.
- **`Role`** is a nested `enum class` (`IdCard::Role`), and the constructor's default arguments live only in the header.

Fill in the five marked blocks, run `make test`, and turn it green. Everything you need is in the sections above.

### A look ahead: where these patterns lead

This machinery is the shared vocabulary of essentially all serious C++. The LLVM code you'll study later is built from exactly these pieces: member calls (`I.getDebugLoc()`) and pointer member calls (`M->getOrInsertFunction(...)`), nested types and aliases, static helpers and constants, class templates (`std::vector<T>`, `std::optional<T>`), destructors and RAII inside library objects, and friend/operator patterns in streamable types. With Chapter 15 behind you, those won't be mysteries — they'll be patterns you recognize.
