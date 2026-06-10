# Chapter 24 - Inheritance

> Source: <https://www.learncpp.com/> (Chapter 24)
> One file per chapter. Each lesson is a section below.

## Contents
- [24.1 - Introduction to inheritance](#241---introduction-to-inheritance)
- [24.2 - Basic inheritance in C++](#242---basic-inheritance-in-c)
- [24.3 - Order of construction of derived classes](#243---order-of-construction-of-derived-classes)
- [24.4 - Constructors and initialization of derived classes](#244---constructors-and-initialization-of-derived-classes)
- [24.5 - Inheritance and access specifiers](#245---inheritance-and-access-specifiers)
- [24.6 - Adding new functionality to a derived class](#246---adding-new-functionality-to-a-derived-class)
- [24.7 - Calling inherited functions and overriding behavior](#247---calling-inherited-functions-and-overriding-behavior)
- [24.8 - Hiding inherited functionality](#248---hiding-inherited-functionality)
- [24.9 - Multiple inheritance](#249---multiple-inheritance)
- [24.x - Chapter 24 summary and quiz](#24x---chapter-24-summary-and-quiz)

---

## 24.1 - Introduction to inheritance

In the previous chapter you learned how to build complex types by *combining* simpler ones — a `Car` that has an `Engine`, a `Department` that holds a list of `Employee`s. That technique, **composition**, is the workhorse of class design, and you will keep using it. But it answers only one of the two questions a type system needs to answer. Composition tells you what an object *contains*. This chapter is about a different relationship: what an object *is*.

Why do we need a second tool at all? Because real designs are full of families. A coverage pass and a mutation pass are both *kinds of* analysis pass; a square and a triangle are both *kinds of* shape; an employee is a *kind of* person. Each family member shares a common core of state and behavior, and then specializes it. You could copy that common core into every member by hand, but copies drift apart and bugs multiply. **Inheritance** lets one class acquire the properties and behavior of another, so the shared core lives in exactly one place and the specializations build on top of it.

### Composition models "has-a"; inheritance models "is-a"

The cleanest way to decide which tool you want is to finish a sentence about your two types.

Composition builds a class by giving it parts. It models a **has-a** or **part-of** relationship — the whole is made out of its members:

```cpp
class FunctionReport
{
private:
    std::string m_name;
    std::vector<int> m_coveredBlocks;
};
```

A `FunctionReport` *has* a name and *has* a list of covered block IDs. Neither member *is* a report; together they make one.

Inheritance builds a class by acquiring everything a more general class already has, then adding to it. It models an **is-a** relationship:

```cpp
class InstrumentationPass
{
public:
    void recordStart();
    void recordFinish();
};

class CoveragePass : public InstrumentationPass
{
public:
    void recordBranch(int branchId);
};
```

A `CoveragePass` *is an* `InstrumentationPass` — every coverage pass can record a start and a finish, because it inherited those abilities — and it adds one more, `recordBranch`. We have not retyped `recordStart` or `recordFinish`; `CoveragePass` simply has them.

> **Key insight:** Composition answers "what is this object made of?" (has-a). Inheritance answers "what kind of thing is this object?" (is-a). When you can truthfully say "a Derived *is a* Base," inheritance is on the table.

### Hierarchies move from general to specific

Inheritance naturally grows hierarchies. The base class captures the common idea; each derived class is a more specific version of it:

```
AnalysisPass
  |
  +-- CoverageAnalysisPass
  |
  +-- MutationAnalysisPass
```

Here, anything every analysis pass needs lives in `AnalysisPass`. `CoverageAnalysisPass` and `MutationAnalysisPass` each inherit that shared core and then add what makes them distinct.

The same shape shows up everywhere once you look for it:

```
Shape
  |
  +-- Rectangle
  |     |
  |     +-- Square
  |
  +-- Triangle
```

If every rectangle has a width and a height, a square inherits that general "rectangle" idea and adds a stricter promise: its width and height are equal. The further down the tree you go, the more specific — and more constrained — the type becomes.

### Inheritance is powerful, but it is not automatically good design

A warning right at the start, because this is the single most common way to misuse inheritance. The test for inheritance is not "do I want to reuse this code?" The test is "is the is-a relationship actually true in my program's model?"

Use inheritance when the is-a sentence is honest:

```cpp
class Employee : public Person {};     // an employee is a person -- good
```

Do *not* reach for inheritance merely because one class happens to want functionality another class already provides:

```cpp
class CsvWriter : public std::vector<std::string> {}; // usually poor design
```

A CSV writer is not a vector. It might *use* a vector internally, or *hold* one — that is composition. By inheriting from `std::vector`, `CsvWriter` would publicly expose dozens of vector operations (`pop_back`, `resize`, `clear`) that make no sense for a writer and that callers could now call to corrupt its state. When you are tempted to inherit for code reuse alone, prefer composition; we will return to this judgement throughout the chapter.

> **Best practice:** Reach for inheritance only when a derived object genuinely *is a* kind of the base object. If your only motivation is reusing the base's code, use composition (a member) instead.

> **Note (CS6340):** LLVM is built on class hierarchies, so reading its code is largely reading inheritance. A function receives an `llvm::Instruction*`, and a *more specific* kind of instruction may be hiding behind it:
>
> ```cpp
> llvm::Instruction* instruction = ...;
>
> if (auto* branch = llvm::dyn_cast<llvm::BranchInst>(instruction))
> {
>     // branch is a more specific kind of instruction
> }
> ```
>
> Conceptually the family looks like:
>
> ```
> Instruction
>   |
>   +-- BranchInst
>   |
>   +-- BinaryOperator
>   |
>   +-- CallInst
> ```
>
> A `BranchInst` still *is an* `Instruction` and supports everything an instruction supports — it just adds branch-specific operations on top. Every assignment in the course leans on exactly this reading.

---

## 24.2 - Basic inheritance in C++

Now the mechanics. We will introduce the vocabulary, the syntax, and — most importantly — the mental model of what a derived object actually *is* in memory. Get that model right and the rest of the chapter follows from it.

### The vocabulary of base and derived

An inheritance relationship has two roles, and unfortunately each role goes by several names. You will meet all of them in books and code, so it is worth a quick table:

| Term | Meaning |
|---|---|
| Base class | the class being inherited from |
| Parent class | another name for the base class |
| Superclass | another name for the base class |
| Derived class | the class doing the inheriting |
| Child class | another name for the derived class |
| Subclass | another name for the derived class |

This text mostly says **base** and **derived**, but treat parent/child and super/sub as exact synonyms.

The syntax is one line:

```cpp
class Base
{
};

class Derived : public Base
{
};
```

The colon introduces the base list; `public Base` means "derive publicly from `Base`." **Public inheritance** is the normal choice for is-a relationships, and it is the only kind you need for almost everything you will write. (Lesson 24.5 covers the other kinds and why they are rare.)

### A derived object contains a base subobject

Here is the idea that everything else rests on. When you derive `Derived` from `Base`, the base's members are *not* textually pasted into the derived class. Instead, a derived object physically *contains* a complete base-class object inside it, called the **base subobject**, plus its own additional members.

Let us make it concrete:

```cpp
class Pass
{
private:
    std::string m_name;

public:
    explicit Pass(std::string name)
        : m_name { std::move(name) }
    {
    }

    const std::string& name() const { return m_name; }
};

class CoveragePass : public Pass
{
private:
    int m_numCounters {};

public:
    CoveragePass(std::string name, int numCounters)
        : Pass { std::move(name) },
          m_numCounters { numCounters }
    {
    }

    int numCounters() const { return m_numCounters; }
};
```

A `CoveragePass` object is laid out like this:

```
CoveragePass object
  |
  +-- Pass portion        (the base subobject)
  |     +-- m_name
  |
  +-- CoveragePass portion
        +-- m_numCounters
```

Think of it like a set of nested boxes: the `CoveragePass` box has a fully-formed `Pass` box inside it. That inner box is a real `Pass`, with its own `m_name` and its own ability to run `name()`. This is *why* the construction rules in the next lessons exist — there is a genuine `Pass` in there that has to be built before the `CoveragePass` part around it.

### A derived class can use the behavior it inherited

Because every `CoveragePass` *has* a real `Pass` inside it, it can use everything the `Pass` interface offers:

```cpp
CoveragePass pass { "branch coverage", 12 };

std::cout << pass.name() << '\n';        // inherited from Pass
std::cout << pass.numCounters() << '\n'; // defined in CoveragePass
```

`name()` was never declared in `CoveragePass`, yet `pass.name()` works — the call reaches into the base subobject and runs `Pass::name()` on it. You write the common behavior once, in the base, and every derived class gets it for free.

### Sibling classes are not related to each other

Two classes that share a base are **siblings**, but sharing a parent does *not* make them interchangeable:

```cpp
class CoveragePass : public Pass {};
class MutationPass : public Pass {};
```

```
Pass
  |
  +-- CoveragePass
  |
  +-- MutationPass
```

Both are kinds of `Pass`, but a `CoveragePass` is *not* a kind of `MutationPass`, and vice versa. The relationship only points up and down the tree, never sideways. What a shared base *does* give you is that each sibling can be used wherever a `Pass` is expected — subject to the rules we develop in later lessons.

### Inheritance chains

A derived class can itself be a base for something more specific, forming a chain:

```cpp
class Pass {};
class FunctionPass : public Pass {};
class CoverageFunctionPass : public FunctionPass {};
```

```
Pass
  |
  +-- FunctionPass
        |
        +-- CoverageFunctionPass
```

A `CoverageFunctionPass` contains a `FunctionPass` subobject, which in turn contains a `Pass` subobject — boxes within boxes within boxes. This is powerful when each level adds a real specialization. It turns *harmful* when the levels exist only to share a few helper functions: a deep chain that does not correspond to genuine is-a relationships is hard to read and harder to change. Depth should buy you meaning, not just convenience.

> **Best practice:** Let an inheritance chain grow only when each new level is a true specialization of the one above it. If a level exists purely to borrow a helper, you probably want composition there instead.

---

## 24.3 - Order of construction of derived classes

We just established that a derived object contains a base subobject. That immediately raises a question of *order*: when you build a `CoveragePass`, what gets built first — the `Pass` part inside it, or the `CoveragePass` part around it? The answer is not arbitrary, and understanding *why* it is what it is will save you from a whole category of initialization bugs.

### Base classes are constructed first

When a derived object is created, C++ constructs it from the **most-base class down to the most-derived class**. The base subobject is fully built before the derived portion is touched.

You can watch it happen by putting a print in each constructor:

```cpp
#include <iostream>

class Pass
{
public:
    Pass()
    {
        std::cout << "Pass\n";
    }
};

class FunctionPass : public Pass
{
public:
    FunctionPass()
    {
        std::cout << "FunctionPass\n";
    }
};

int main()
{
    FunctionPass pass {};
}
```

Output:

```text
Pass
FunctionPass
```

`Pass` runs first, `FunctionPass` second — even though you only asked for a `FunctionPass`. The compiler quietly arranges for the base to be built before the derived constructor body runs.

### The same order extends down a chain

The rule applies at every level of a chain: each base is fully constructed before the class that derives from it.

```cpp
class A
{
public:
    A() { std::cout << "A\n"; }
};

class B : public A
{
public:
    B() { std::cout << "B\n"; }
};

class C : public B
{
public:
    C() { std::cout << "C\n"; }
};

C object {};
```

Output:

```text
A
B
C
```

Read it as a top-down build:

```
requested: construct C

1. construct A portion   (most base)
2. construct B portion
3. construct C portion   (most derived)
```

### Why this order, and not the other way around?

This is the part worth internalizing, because it explains every construction rule in this chapter. The derived constructor is allowed to *use* inherited members — it might call a base function, or read a base value, while initializing its own part. For that to be safe, the base subobject must already be alive and valid by the time the derived constructor runs. So the base is built first.

The reverse dependency does not exist: a well-designed base class knows nothing about its derived classes, so the base constructor has no reason to need the derived portion. And that is good, because during base construction the derived portion does not exist yet. This is why reaching "downward" from a base constructor into derived state is always a mistake — there is nothing there to reach.

> **Key insight:** Bases construct before deriveds so that a derived constructor can rely on its inherited members already being initialized. A base constructor must never depend on derived state, because that state does not exist yet when the base runs.

### Destruction runs in reverse

Destruction unwinds construction. The most-derived part is destroyed first, then each base in turn, ending at the most-base class:

```cpp
class A
{
public:
    A()  { std::cout << "A ctor\n"; }
    ~A() { std::cout << "A dtor\n"; }
};

class B : public A
{
public:
    B()  { std::cout << "B ctor\n"; }
    ~B() { std::cout << "B dtor\n"; }
};
```

For `B b {};` the full life cycle prints:

```text
A ctor
B ctor
B dtor
A dtor
```

The symmetry is deliberate: you tear down the derived part — which may still want to use base members during its own cleanup — *before* you tear down the base it depends on. Construction goes base-first; destruction goes derived-first; the base is always the first thing alive and the last thing gone.

> **Note (CS6340):** When you derive a pass from an LLVM framework base class, this order still holds — the framework's base portion is constructed before your derived constructor body runs:
>
> ```cpp
> class MyPass : public SomeFrameworkPass
> {
> public:
>     MyPass()
>     {
>         // SomeFrameworkPass portion is already constructed here -- safe to use.
>     }
> };
> ```
>
> Never try to use derived-class state from a base-class constructor. In large frameworks that is a classic source of mystifying "the field was null during construction" bugs.

---

## 24.4 - Constructors and initialization of derived classes

The previous lesson showed *that* the base is constructed first. This lesson shows *how* you control that — how a derived constructor chooses which base constructor runs, what data it may and may not initialize, and what happens when you forget to specify a base constructor. This is the most hands-on lesson in the chapter, and it is exactly the machinery the chapter lab asks you to write.

### A derived constructor initializes its own part and chooses a base constructor

Division of labor is the theme. A derived constructor is responsible for initializing the *derived* portion. The *base* portion is initialized by a base constructor — and the derived constructor gets to say *which* base constructor that is, by naming it in the member-initializer list:

```cpp
class Pass
{
private:
    std::string m_name;

public:
    explicit Pass(std::string name)
        : m_name { std::move(name) }
    {
    }

    const std::string& name() const { return m_name; }
};

class FunctionPass : public Pass
{
private:
    int m_priority {};

public:
    FunctionPass(std::string name, int priority)
        : Pass { std::move(name) },   // initialize the base subobject
          m_priority { priority }     // initialize the derived member
    {
    }
};
```

Read the initializer list as two jobs. `Pass { std::move(name) }` constructs the `Pass` subobject by handing `name` to the `Pass` constructor. `m_priority { priority }` initializes the `FunctionPass`'s own member. The base call comes first in the list because, as you now know, the base is built first.

### A derived constructor cannot initialize base members directly

It might be tempting to skip the middleman and initialize the base's member yourself. You cannot:

```cpp
class FunctionPass : public Pass
{
public:
    FunctionPass(std::string name)
        : m_name { std::move(name) }  // error: m_name belongs to Pass, not FunctionPass
    {
    }
};
```

This fails for a structural reason, not just an access one. Initializing the base subobject is the *base constructor's* job — it is the code that knows how `Pass` wants to be built. A derived class only names a base member in its own initializer list if that member is one it declared. To set up the base, you call a base constructor and pass it what it needs.

> **Warning:** A derived class cannot list a base-class member in its own member-initializer list. Initialize the base by calling a base constructor (e.g. `Pass { ... }`), and let that constructor initialize the base's members.

### The full construction sequence

Putting it together, here is everything that happens for one line:

```cpp
FunctionPass pass { "coverage", 10 };
```

```
1. allocate enough memory for the whole FunctionPass object
2. enter the FunctionPass constructor
3. construct the Pass subobject using Pass{"coverage"}
4. initialize FunctionPass's own members, such as m_priority
5. run the FunctionPass constructor body
6. return to the caller
```

The subtlety to notice: even though you *wrote* the base call `Pass { ... }` inside the `FunctionPass` initializer list, it *executes* before any of the `FunctionPass` members are initialized and before the constructor body runs. The initializer list is not executed top to bottom as written — the base subobject always goes first.

### A constructor may call only its immediate base constructor

In a chain, each constructor may name only the constructor of its **direct** parent — never a grandparent. Information flows one link at a time:

```cpp
class A
{
public:
    explicit A(int) {}
};

class B : public A
{
public:
    B(int a, double)
        : A { a }       // B initializes its immediate base, A
    {
    }
};

class C : public B
{
public:
    C(int a, double b, char)
        : B { a, b }    // C initializes B -- it cannot name A directly
    {
    }
};
```

`C` cannot reach past `B` to construct `A`. Instead `C` hands `B` what `B` needs, and `B` decides how to construct `A`. Each class is responsible only for its immediate parent, which keeps the chain's responsibilities local: `C` does not need to know how `A` is built, only how `B` is.

### Private base members stay private

Inheritance does not unlock the base's private members. A derived class still cannot touch them directly:

```cpp
class FunctionPass : public Pass
{
public:
    FunctionPass()
        : Pass { "function" }
    {
        // m_name = "other"; // error: m_name is private in Pass
    }
};
```

This is exactly the encapsulation you would want. `Pass` chose to keep `m_name` private, and that choice is honored even for classes that inherit from it. A derived class reaches base state only through the base's `public` and `protected` interface — a base constructor here, an accessor like `name()` elsewhere. (The next lesson introduces `protected`, the access level that exists specifically to give derived classes a controlled door in.)

### Forgetting the base constructor: the missing-default-constructor error

If a derived constructor does *not* name a base constructor, the compiler silently tries to call the base's **default constructor**. If the base has none, you get a compile error that can be puzzling until you know to look for it:

```cpp
class Pass
{
public:
    explicit Pass(std::string name) {}   // no default constructor
};

class BadPass : public Pass
{
public:
    BadPass()
    {
        // no base constructor named -> compiler tries Pass{} -> none exists -> error
    }
};
```

`BadPass` will not compile, because there is no `Pass()` for it to fall back on. The fix is to name the base constructor that *does* exist:

```cpp
class GoodPass : public Pass
{
public:
    GoodPass()
        : Pass { "good" }   // explicitly call the available base constructor
    {
    }
};
```

> **Tip:** If a derived class fails to compile complaining about a missing or deleted base constructor, you almost certainly forgot to name a base constructor in the initializer list. Add `: Base { ... }` with arguments the base actually accepts.

### A look ahead: base destructors

Destruction runs derived-first, then base, as you saw in 24.3:

```
~FunctionPass()
~Pass()
```

One caveat is worth planting now even though its solution belongs to the next chapter. If you ever destroy a derived object *through a base-class pointer or reference*, this clean derived-then-base sequence can silently break unless the base declares a **virtual destructor**. That is a Chapter 25 topic (virtual functions), but the warning starts here: the moment you manage derived objects through base pointers, virtual destructors stop being optional.

> **Warning:** Deleting a derived object through a base pointer without a virtual destructor in the base is undefined behavior. You do not need virtual destructors for the value-based hierarchies in this chapter, but you will the moment polymorphic base pointers appear in Chapter 25.

---

## 24.5 - Inheritance and access specifiers

So far we have used `public` and `private` exactly as you learned them in the classes chapters. Inheritance adds a wrinkle and a third option. There are now *two* audiences who might want to reach a member — outside code, and derived classes — and `public`/`private` treat them identically. The new specifier, `protected`, exists to tell those two audiences apart. This lesson also covers a separate dial: the access level on the inheritance *itself*.

### Three member access levels

Here is the complete picture of who can reach a member, by access specifier:

| Access specifier | Same class? | Derived class? | Outside (public) code? |
|---|---:|---:|---:|
| `public` | Yes | Yes | Yes |
| `protected` | Yes | Yes | No |
| `private` | Yes | No | No |

Read the `protected` row carefully, because it is the only new thing here. A `protected` member is accessible to the class itself *and to its derived classes*, but **not** to outside code. It carves out a middle ground that did not exist before inheritance: "family only."

```cpp
class Pass
{
protected:
    int m_runCount {};
};

class FunctionPass : public Pass
{
public:
    void markRun()
    {
        ++m_runCount;       // ok: m_runCount is protected, FunctionPass is family
    }
};
```

If `m_runCount` had been `private`, `markRun` would not compile — private means "this class only," and derived classes are outsiders to it. If it had been `public`, then any caller anywhere could bump the run count, which is probably not what you want. `protected` is the Goldilocks setting: derived classes in, the public out.

### Prefer private data with protected functions

`protected` data is convenient, but convenience here comes at a cost, and it is worth understanding before you sprinkle `protected:` everywhere. When a derived class reads or writes a `protected` data member directly, it becomes coupled to the *exact representation* of that member. If the base class later changes what `m_runCount` means, or replaces it with a different representation, every derived class that touched it directly can silently break.

The looser, sturdier design is to keep data `private` and expose a `protected` *function* that derived classes call instead:

```cpp
class Pass
{
private:
    int m_runCount {};

protected:
    void incrementRunCount()
    {
        ++m_runCount;       // the base controls how the count changes
    }
};
```

This is better than exposing the raw member:

```cpp
protected:
    int m_runCount {};      // tighter coupling -- derived classes see the representation
```

With the function version, the base keeps control of *how* the count is maintained, and is free to change the underlying representation without breaking its derived classes — they only ever call `incrementRunCount()`. The base's interface to its family is behavior, not raw storage.

> **Best practice:** Prefer `private` data members and a small `protected` interface of functions over `protected` data members. Protected functions let the base change its internals without breaking derived classes.

### Three kinds of inheritance

Separately from member access, the *inheritance itself* has an access level. You have always written `public`; you can also write `protected` or `private`:

```cpp
class PublicDerived : public Base {};
class ProtectedDerived : protected Base {};
class PrivateDerived : private Base {};
```

If you omit the specifier for a `class`, it defaults to `private` — which is rarely what you want, so write `public` explicitly:

```cpp
class Derived : Base {};   // private inheritance by default -- usually a mistake to rely on
```

For ordinary is-a relationships — which is almost everything — you want `public`.

### How the inheritance kind transforms inherited access

The inheritance access acts as a *ceiling* on how visible the base's members are through the derived class. Here is the full transformation table:

| Base member access | Public inheritance | Protected inheritance | Private inheritance |
|---|---|---|---|
| `public` | public in derived | protected in derived | private in derived |
| `protected` | protected in derived | protected in derived | private in derived |
| `private` | inaccessible | inaccessible | inaccessible |

Two patterns to take away. First, the bottom row never changes: a base's `private` members are inaccessible to derived classes no matter which inheritance kind you choose — privacy is absolute. Second, the inheritance kind can only *lower* visibility, never raise it: public inheritance leaves access as-is, protected caps it at protected, and private caps it at private.

### Public inheritance preserves the is-a interface

Public inheritance is the one that models is-a, and the table tells you why: a `public` member of the base stays `public` on the derived object.

```cpp
class Pass {};
class FunctionPass : public Pass {};
```

That means external code may use a `FunctionPass` as a `Pass` wherever a `Pass` is expected — the base's public interface is still public on the derived object. This is the contract behind every "pass the derived where a base is wanted" pattern you will write.

### Private inheritance is implementation reuse, not is-a

Private inheritance is the odd one out. It makes the *entire* base interface private inside the derived class, so the relationship is invisible to outside code. It does **not** model a public is-a relationship — it is a way to reuse a base's implementation privately:

```cpp
class Timer
{
public:
    void reset();
};

class Benchmark : private Timer
{
public:
    void start()
    {
        reset();   // Benchmark may use Timer's machinery internally
    }
};
```

Inside `Benchmark`, `reset()` is callable. But because the inheritance is private, public users of `Benchmark` cannot call `reset()` through a `Benchmark` object — the `Timer` interface is sealed off from the outside. A `Benchmark` is *not* publicly a `Timer`; it merely uses one's implementation.

In practice, composition almost always expresses this more clearly:

```cpp
class Benchmark
{
private:
    Timer m_timer;   // "Benchmark has a Timer" -- usually the better design
};
```

Composition makes the relationship obvious ("Benchmark *has a* Timer") and exposes only what `Benchmark` chooses to. Reach for private inheritance only in the narrow cases where you need something composition cannot give you; otherwise, prefer the member.

> **Best practice:** Use `public` inheritance for is-a relationships. Prefer composition over `private`/`protected` inheritance — they model "implemented in terms of," which a member usually expresses more clearly.

> **Note (CS6340):** When you read inherited code, keep two questions apart: (1) Can the derived class's *own code* access this member? (2) Can *outside code* access it through the derived object? They are not the same. A derived class routinely uses `protected` members internally that public callers can never see — so "the derived class uses it" does not imply "it is part of the public interface."

---

## 24.6 - Adding new functionality to a derived class

Inheritance would be dull if all a derived class could do was repeat its base. The point of specialization is to *add* — new data, new functions, new abilities that the base does not have. This short lesson is about that additive half of inheritance, and about the design judgement of *where* a given piece of functionality belongs.

### Derived classes can add members

The simplest extension is to declare data or functions that exist only on the derived type. The base never sees them; they are pure additions:

```cpp
class Pass
{
private:
    std::string m_name;

public:
    explicit Pass(std::string name)
        : m_name { std::move(name) }
    {
    }

    const std::string& name() const { return m_name; }
};

class CoveragePass : public Pass
{
private:
    int m_branchCounters {};

public:
    CoveragePass(std::string name, int branchCounters)
        : Pass { std::move(name) },
          m_branchCounters { branchCounters }
    {
    }

    int branchCounters() const { return m_branchCounters; }
};
```

`CoveragePass` *inherits* `name()` from `Pass` and *adds* `branchCounters()` of its own. A `CoveragePass` object now offers both; a plain `Pass` offers only `name()`. The derived class is a strict superset of the base's interface.

### Put functionality at the right level

The interesting decision is not *whether* you can add a function — it is *where* it should live. The guideline is about scope of applicability.

If a function makes sense for *every* class in the family, it belongs in the base, so all of them inherit it:

```cpp
class Pass
{
public:
    void printName() const;   // every pass has a name worth printing
};
```

If a function makes sense only for *one* specialization, keep it in that derived class:

```cpp
class CoveragePass : public Pass
{
public:
    void printCoverageTable() const;   // only coverage passes have a coverage table
};
```

Pushing derived-specific functions up into the base is a common mistake with real costs. It bloats the base interface, and it forces *unrelated* derived classes to carry an operation that is meaningless for them — a `MutationPass` would inherit a `printCoverageTable()` it can never sensibly implement. Keep the base honest: it should hold only what is genuinely common to the whole family.

> **Best practice:** Place a function in the base only if it applies to all derived classes. Functionality specific to one specialization belongs in that derived class, not pushed up into a base that other classes also inherit.

### Derived code reuses inherited behavior

When a derived function needs something the base already does, it just calls it — no duplication required:

```cpp
class CoveragePass : public Pass
{
public:
    void describe() const
    {
        std::cout << "coverage pass: " << name() << '\n';   // name() inherited from Pass
    }
};
```

`describe()` is new behavior on `CoveragePass`, but it leans on the inherited `name()` rather than re-implementing how a name is stored or retrieved. This is the everyday payoff of inheritance: the derived class adds a thin layer of new behavior on top of a base it can freely call.

> **Note (CS6340):** When you build small helper hierarchies for course work, apply the same level test. A `name()` that every operator shares belongs in the base; a `canMutate(const llvm::Instruction&)` that only mutation operators implement belongs in the derived `ArithmeticMutationOperator`, not on a broad analysis base that other operators also inherit.

---

## 24.7 - Calling inherited functions and overriding behavior

Adding *new* functions is the easy case. The richer — and trickier — case is when a derived class wants to provide its *own* version of a function the base already has. This lesson is about how the compiler decides which function a call lands on, how to redefine base behavior in a derived class, how to still reach the base version when you need it, and a surprising trap involving overloads. It also sets up the central lesson the chapter lab dramatizes, so read the last subsection with care.

### Function lookup starts in the most-derived class

When you call a member function on an object, the compiler does not search the whole hierarchy at once. It starts at the **most-derived class** and looks for a function with that *name*. If it finds one there, it stops searching. If it does not, it walks one step up the chain and looks again, repeating until it finds a match or runs out of classes.

```cpp
class Pass
{
public:
    void identify() const
    {
        std::cout << "Pass\n";
    }
};

class CoveragePass : public Pass
{
};

CoveragePass pass {};
pass.identify();        // not found in CoveragePass -> found in Pass -> Pass::identify()
```

`CoveragePass` declares no `identify`, so lookup climbs to `Pass` and finds it there.

### Redefining a function in the derived class

If the derived class *does* declare a function with the same name, lookup finds that one first and stops — so calls through a derived object use the derived version:

```cpp
class CoveragePass : public Pass
{
public:
    void identify() const
    {
        std::cout << "CoveragePass\n";
    }
};

CoveragePass pass {};
pass.identify();        // found in CoveragePass -> CoveragePass::identify()
```

This is **redefinition** (also called **hiding**): the derived `identify` hides the base `identify` for lookups that start in `CoveragePass`. It is important to be precise about what this is *not*. This is ordinary, compile-time name lookup based on the type you are calling through. It is **not** the runtime polymorphism you get from `virtual` functions — that is the entire subject of Chapter 25. Here, the function chosen depends on the *statically known type* used at the call site, decided when the program is compiled.

### Calling the base version explicitly

A derived function often wants to *extend* the base behavior, not replace it wholesale — do something extra, then defer to what the base already does. You reach the hidden base version by qualifying its name with the base class and `::`:

```cpp
class CoveragePass : public Pass
{
public:
    void identify() const
    {
        std::cout << "CoveragePass\n";
        Pass::identify();            // explicitly call the base version
    }
};
```

`Pass::identify()` says "look up `identify` starting in `Pass`, not in the current class." That qualification is mandatory here, and forgetting it is a memorable bug:

```cpp
void identify() const
{
    identify();         // DANGER: unqualified -> finds THIS function -> infinite recursion
}
```

Without the `Pass::` qualifier, `identify()` resolves to the function you are currently inside — `CoveragePass::identify` calls itself forever. Whenever you mean "the inherited version," name the base scope explicitly.

> **Warning:** Inside a derived function that redefines a base function, an *unqualified* call to the same name calls the derived function recursively. Use `Base::function()` to reach the inherited version.

This pattern — redefine, add your bit, then call `Base::function()` — is exactly what the chapter lab's `TimestampLogger::log()` does. It prepends a tag to the message, then calls `Logger::log(decorated)` to do the actual formatting and bookkeeping. The explicit `Logger::` is required; plain `log(...)` would recurse.

### Calling a base friend operator like operator<<

There is one wrinkle when the "base function" you want is a friend operator such as `operator<<`. A friend is *not* a member of the base class, so there is no `Base::operator<<` to qualify. The way to select the base overload is to **cast the object to a base reference** so that normal overload resolution picks the base version:

```cpp
class Pass
{
public:
    friend std::ostream& operator<<(std::ostream& out, const Pass&)
    {
        return out << "Pass";
    }
};

class CoveragePass : public Pass
{
public:
    friend std::ostream& operator<<(std::ostream& out, const CoveragePass& pass)
    {
        out << "CoveragePass built on ";
        out << static_cast<const Pass&>(pass);   // select the base's operator<<
        return out;
    }
};
```

The cast `static_cast<const Pass&>(pass)` is to a *reference*, deliberately. Casting to a base *value* would copy out just the base subobject — a copy you do not want and, in polymorphic code, an outright bug (this is "slicing," another Chapter 25 topic). A reference cast simply *views* the existing object as its base part, with no copy.

> **Best practice:** To reuse a base friend operator from a derived one, `static_cast` the object to a *base reference* (`static_cast<const Base&>(obj)`), never a base value. A reference cast avoids copying or slicing the object.

### Overloads and the hiding surprise

Here is the trap that catches everyone once. When a derived class declares a function with a given name, it hides the *entire set* of base functions with that name — every overload — from unqualified lookup, not just the one with a matching signature:

```cpp
class Base
{
public:
    void print(int)    { std::cout << "Base int\n"; }
    void print(double) { std::cout << "Base double\n"; }
};

class Derived : public Base
{
public:
    void print(double) { std::cout << "Derived double\n"; }
};

Derived d {};
d.print(5);     // calls Derived::print(double) -- NOT Base::print(int)
```

You might expect `d.print(5)` to call `Base::print(int)`, since `5` is an `int` and `Base` has a perfect match. It does not. Lookup finds the name `print` in `Derived`, stops climbing, and considers *only* `Derived`'s overload set — which contains just `print(double)`. The `int` argument is then converted to `double`. The base's `print(int)` was never in the running, because lookup never reached `Base`.

> **Key insight:** Declaring *any* function named `f` in a derived class hides *all* base overloads named `f` from unqualified lookup. Name lookup finds the name and stops; it does not merge base and derived overload sets.

### Bringing base overloads back with using

If you want the base overloads to participate alongside your derived one, pull them into the derived scope with a `using` declaration:

```cpp
class Derived : public Base
{
public:
    using Base::print;           // make Base's print overloads visible here

    void print(double)
    {
        std::cout << "Derived double\n";
    }
};
```

Now `Derived`'s overload set includes both the inherited `print(int)`, `print(double)` from `Base` *and* the new `print(double)` — so `d.print(5)` can resolve to `Base::print(int)` as you first expected. `using Base::print;` says "consider the base's `print` overloads as candidates here too."

> **Note (CS6340):** Everything in this lesson resolves at compile time, by *static type* and name-lookup rules — not by the runtime type of the object. When LLVM code achieves runtime dispatch, it uses `virtual` functions and type inquiry (Chapter 25). The compile-time lookup you learned here is the foundation those mechanisms build on, and it is exactly what makes the lab's `processLog` cliffhanger behave the way it does.

---

## 24.8 - Hiding inherited functionality

The last lesson let a derived class *replace* inherited behavior. This one is about *restricting* it: changing the access level of an inherited member, or blocking a call entirely. These are useful tools for tidying up an interface — but, as you will see, they are partial. They change how a member looks *through the derived type*; they do not erase it from the base. Knowing the limits matters as much as knowing the technique.

### Changing inherited access with a using declaration

A derived class can change the access level of an inherited member it can *already access*, by re-declaring the member's name with a `using` declaration under a new access specifier. A common use is to take a `protected` base function and make it `public` on the derived type:

```cpp
class BaseCounter
{
protected:
    void printCount() const
    {
        std::cout << "count\n";
    }
};

class PublicCounter : public BaseCounter
{
public:
    using BaseCounter::printCount;   // inherited protected function is now public here
};
```

Now outside code can call it through a `PublicCounter`:

```cpp
PublicCounter counter {};
counter.printCount();             // ok -- public via the using declaration
```

Note the syntax: a `using` declaration names the function *without* parentheses or arguments — `using BaseCounter::printCount;`, never `printCount()`.

### You cannot expose what you cannot access

There is a hard limit: a derived class can only re-expose members it can *already* reach. Since `private` base members are inaccessible to derived classes (24.5), you cannot use this trick to crack them open:

```cpp
class Base
{
private:
    void secret();
};

class Derived : public Base
{
public:
    // using Base::secret;   // error: secret is private in Base -- inaccessible to Derived
};
```

This is the encapsulation guarantee holding firm. If a base genuinely wants to give controlled access to derived classes, *it* must offer a `protected` (or `public`) function — a derived class cannot grant itself access the base withheld.

### Hiding a public inherited member

The trick runs the other way too: a derived class can take a `public` inherited member and make it `private`, narrowing the derived interface:

```cpp
class Base
{
public:
    int value {};
};

class Derived : public Base
{
private:
    using Base::value;   // value is no longer publicly accessible through Derived
};
```

After this, `derived.value` from outside code is rejected. This is sometimes used to hide a piece of poor base design that you cannot change. But be honest with yourself about what it achieves — it is *not* real encapsulation, because the member is still public on the `Base` interface, and anyone can reach it by viewing the object as a `Base`:

```cpp
Derived d {};
Base& b { d };
b.value = 5;            // still allowed -- the Base interface is unchanged
```

The hiding only affects lookups *through the derived type*. The base subobject is unchanged, and a base reference sees right past your restriction.

> **Warning:** Making an inherited member `private` in a derived class hides it only through the derived type. The member remains accessible through a base pointer or reference, so this is not true encapsulation — merely a derived-interface convenience.

### Deleting inherited-looking functionality

A blunter option is to `= delete` a function in the derived class, which blocks calls to it *through the derived type*:

```cpp
class Base
{
public:
    int value() const { return 7; }
};

class Derived : public Base
{
public:
    int value() const = delete;
};
```

Now a direct call is rejected, though the base version is still reachable with explicit qualification:

```cpp
Derived d {};
// d.value();                 // compile error: deleted

std::cout << d.Base::value(); // explicit base call still works -> prints 7
```

As with the access trick, deleting a derived function blocks one *call syntax*; it does not remove the member from the base class. The base member still exists and is still reachable through the base, by qualification or through a base reference.

### Access and deletion apply to the whole name

One precision point that mirrors the overload-hiding rule from 24.7: a `using` declaration changes access for *every* overload of a name, not one of them. If the base has overloads:

```cpp
class Base
{
public:
    void configure(int);
    void configure(std::string_view);
};
```

then:

```cpp
class Derived : public Base
{
private:
    using Base::configure;   // makes ALL inherited configure overloads private
};
```

makes *every* inherited `configure` private through `Derived`. You cannot use a single `using` declaration to change access for just one overload of an overloaded name — it operates on the name as a whole.

### A design warning

Step back when you find yourself doing a lot of this. If a derived class needs to hide or delete a *substantial* part of its base's interface, that is strong evidence the is-a relationship is not real — the derived type does not actually want to be the base. The cleaner design is composition: hold the base as a member and expose only the operations you want.

```cpp
class Adapter
{
private:
    Base m_base;        // expose only the operations Adapter chooses to forward
};
```

Composition lets the wrapper define its public interface deliberately, instead of inheriting a large interface and then fighting to suppress the parts that do not fit. Hiding a member here and there is fine; hiding most of the base is a design smell.

> **Best practice:** A few access tweaks are fine, but if a derived class must hide much of its base, prefer composition. Wrapping the base as a member lets you choose the public interface instead of inheriting one and suppressing it piece by piece.

---

## 24.9 - Multiple inheritance

Every inheritance you have written so far has had a single base. C++ also permits a class to inherit from *more than one* base at once — **multiple inheritance**. It is a genuine feature with legitimate uses, but it also introduces ambiguities and a famous structural hazard, the diamond problem. The honest summary, which we will earn over this lesson, is: useful in narrow circumstances, easy to overuse, and best approached with caution.

### The syntax

List several bases, separated by commas, each with its own access specifier:

```cpp
class Named
{
public:
    std::string name() const;
};

class Timed
{
public:
    double elapsedSeconds() const;
};

class TimedPass : public Named, public Timed
{
};
```

A `TimedPass` now contains one subobject for *each* base, side by side, plus its own part:

```
TimedPass
  |
  +-- Named portion
  |
  +-- Timed portion
  |
  +-- TimedPass portion
```

It inherits `name()` from `Named` and `elapsedSeconds()` from `Timed`, combining two independent capabilities into one type.

### Mixins: the well-behaved use

The cleanest application of multiple inheritance is the **mixin**: a small, sharply-scoped base class that contributes one focused piece of behavior, designed to be combined with others. Each mixin does one thing:

```cpp
class HasEnabledFlag
{
private:
    bool m_enabled { true };

public:
    bool enabled() const { return m_enabled; }
    void setEnabled(bool enabled) { m_enabled = enabled; }
};

class HasLabel
{
private:
    std::string m_label;

public:
    void setLabel(std::string label) { m_label = std::move(label); }
    const std::string& label() const { return m_label; }
};

class ToolButton : public HasEnabledFlag, public HasLabel
{
};
```

A `ToolButton` composes two small, orthogonal abilities. The mixins are deliberately tiny — they are not meant to stand alone as domain objects, only to be mixed in. When the bases are this independent, multiple inheritance is clean because nothing overlaps.

### Ambiguous names

Trouble starts when two bases provide a member with the *same name*. An unqualified call then has no single right answer, and the compiler refuses to guess:

```cpp
class UsbDevice
{
public:
    int id() const { return 10; }
};

class NetworkDevice
{
public:
    int id() const { return 20; }
};

class UsbNetworkAdapter : public UsbDevice, public NetworkDevice
{
};

UsbNetworkAdapter adapter {};
// adapter.id();              // error: ambiguous -- which id()?

std::cout << adapter.UsbDevice::id();      // 10
std::cout << adapter.NetworkDevice::id();  // 20
```

Scope qualification resolves each call, but having to disambiguate *every* shared-name call is friction that accumulates. Two bases with overlapping interfaces is a sign that multiple inheritance may be the wrong tool.

### The diamond problem

The structural hazard is the **diamond problem**, which arises when a class inherits from two classes that *both* inherit from the same base:

```cpp
class Device {};

class Scanner : public Device {};
class Printer : public Device {};

class Copier : public Scanner, public Printer {};
```

Drawn out, it forms a diamond:

```
      Device
      /    \
 Scanner  Printer
      \    /
      Copier
```

By default, a `Copier` ends up with *two* separate `Device` subobjects — one reached through `Scanner`, one through `Printer`. That single fact unleashes a cascade of awkward questions:

- Is there one device identity here, or two?
- When code accesses a `Device` member, which of the two paths does it mean?
- How should construction initialize the shared conceptual base — once, or twice?

C++ offers **virtual base classes** to collapse the duplicated base into one shared subobject, which addresses some diamond cases — but that is a later topic, and it adds its own complexity. The lesson to carry now is simpler: multiple inheritance can complicate object layout and name lookup quickly, and diamonds are where it bites hardest.

### When multiple inheritance is reasonable — and when it is not

A short judgement guide. Multiple inheritance can be appropriate when the bases are small mixins, the relationships are independent and genuinely useful, ambiguity is low, and the alternatives would be *more* complex. Avoid it when single inheritance would do the job, when composition expresses the design better, when the bases have overlapping public interfaces, when ownership and lifetime become murky, or when the hierarchy forms diamonds without a deliberate plan.

> **Best practice:** Reserve multiple inheritance for combining small, independent mixins. When bases share names or form diamonds, prefer single inheritance or composition — both keep object layout and name lookup simple.

> **Note (CS6340):** LLVM and other large C++ frameworks do use multiple inheritance and mixin-like patterns internally, so you will *read* it. But for your own course work you rarely need to *design* a multiple-inheritance hierarchy. Default to plain helper functions, small structs, and composition; reach for multiple inheritance only when a framework actually requires it.

---

## 24.x - Chapter 24 summary and quiz

You arrived in this chapter knowing how to assemble objects out of parts. You leave it knowing how to arrange whole *families* of types — a base that captures what is common, derived classes that specialize it, and a precise set of rules for how those pieces are built, accessed, and called. That same shape underlies real C++ codebases, the LLVM `Instruction` family included. The one thing this chapter deliberately left unfinished — making a call through a base reference run the *derived* version — is the doorway into Chapter 25.

### Summary

- **Inheritance models an is-a relationship.** Use it when a derived object genuinely is a kind of the base; otherwise prefer composition.
- The class inherited from is the **base** (parent, superclass); the class doing the inheriting is the **derived** (child, subclass).
- **Public inheritance** is the normal form, and it preserves the base's public interface on the derived object.
- A derived object **contains a base subobject** plus its own portion.
- **Construction runs most-base to most-derived**; **destruction runs the reverse**. A base must be alive before the derived part that depends on it.
- A derived constructor **chooses which immediate base constructor** runs, by naming it in the member-initializer list. It may call only its *direct* base.
- A derived constructor **cannot initialize base members directly** — it calls a base constructor to do that. Private base members remain inaccessible.
- If no base constructor is named, the compiler calls the base **default constructor**; a missing one is a compile error.
- **`protected`** members are accessible to derived classes but not to outside code. Prefer `private` data with a small `protected` *function* interface over `protected` data.
- Public, protected, and private inheritance each transform inherited access differently; only **public** inheritance preserves the base's public interface.
- Derived classes can **add new members** and **redefine** inherited behavior (compile-time hiding, not yet runtime polymorphism).
- Use **`Base::function()`** to call a hidden base version explicitly; an unqualified call would recurse. Cast to a **base reference** to reuse a base friend operator.
- Declaring a name in a derived class **hides the entire base overload set** for that name; **`using Base::name;`** brings the base overloads back into scope.
- **`using Base::member`** can change the access of an inherited member the derived class can already reach; it applies to the whole name, all overloads.
- A **deleted** derived function blocks a call through the derived type, but the base member still exists and is reachable through the base.
- **Multiple inheritance** is available but should be used sparingly, owing to name ambiguity and diamond-shaped hierarchies.

### An inheritance design checklist

Before reaching for inheritance, ask yourself:

1. Is the derived object *truly* a kind of the base object?
2. Should public users be able to use the derived object *as* a base object? (If yes, public inheritance.)
3. Is shared code my *only* motivation? If so, prefer composition.
4. Does the base expose a stable interface that derived classes can rely on?
5. Will I manage derived objects through base pointers or references? If so, virtual behavior and virtual destructors matter — continue into Chapter 25 before designing around base pointers.

### The cliffhanger, and where Chapter 25 begins

The chapter lab — the **Report-Logger Family** — makes one limitation physical, and it is worth stating plainly here because it motivates everything next. You will write a base `Logger` and a derived `TimestampLogger` that redefines `log()` to prepend a tag. Then a free function takes a base reference and calls through it:

```cpp
std::string processLog(Logger& l, const std::string& msg)
{
    return l.log(msg);
}
```

When you pass a `TimestampLogger` into that `Logger&` and run it, the tag *does not appear*. The base `Logger::log()` runs, even though the object truly is a `TimestampLogger`. This is **static binding**: without `virtual`, the compiler resolves `l.log(msg)` using the *declared type* of the reference (`Logger`), not the runtime type of the object behind it. It is correct, expected, and exactly the behavior this chapter's rules predict — and it is also disappointing, because you clearly wanted the derived version.

That gap — "the object is a `TimestampLogger`, so why did the base run?" — is not a bug to work around. It is the precise problem that **virtual functions** were invented to solve, and closing it is the first order of business in Chapter 25.

> **Note (CS6340):** Translate `class BranchInst : public Instruction` as four plain statements: a `BranchInst` *is an* `Instruction`; it *contains* an `Instruction` base portion; `Instruction` behavior is callable on it; and it *adds* branch-specific operations. When a helper takes `void inspect(const llvm::Instruction& I)`, it accepts any object that *is an* `Instruction`, derived types included. *Which* implementation runs when that helper calls a function — base or derived — is decided by the virtual-function machinery of Chapter 25. This chapter built the static foundation; the next one makes the calls dispatch at runtime.
