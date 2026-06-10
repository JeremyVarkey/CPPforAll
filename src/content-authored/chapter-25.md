# Chapter 25 — Virtual Functions

> Source: <https://www.learncpp.com/> (Chapter 25)
>
> Goal for CS6340: understand enough inheritance, virtual dispatch, abstract
> interfaces, object slicing, and casts to read C++ that manipulates LLVM-style
> class hierarchies without panic.

## Contents

- [25.1 — Pointers and references to the base class of derived objects](#251--pointers-and-references-to-the-base-class-of-derived-objects)
- [25.2 — Virtual functions and polymorphism](#252--virtual-functions-and-polymorphism)
- [25.3 — The `override` and `final` specifiers, and covariant return types](#253--the-override-and-final-specifiers-and-covariant-return-types)
- [25.4 — Virtual destructors, virtual assignment, and overriding virtualization](#254--virtual-destructors-virtual-assignment-and-overriding-virtualization)
- [25.5 — Early binding and late binding](#255--early-binding-and-late-binding)
- [25.6 — The virtual table](#256--the-virtual-table)
- [25.7 — Pure virtual functions, abstract base classes, and interface classes](#257--pure-virtual-functions-abstract-base-classes-and-interface-classes)
- [25.8 — Virtual base classes](#258--virtual-base-classes)
- [25.9 — Object slicing](#259--object-slicing)
- [25.10 — Dynamic casting](#2510--dynamic-casting)
- [25.11 — Printing inherited classes using `operator<<`](#2511--printing-inherited-classes-using-operator)
- [25.x — Chapter 25 summary and quiz](#25x--chapter-25-summary-and-quiz)
- [CS6340 patterns](#cs6340-patterns)
- [Mini drill](#mini-drill)

---

## 25.1 — Pointers and references to the base class of derived objects

In Chapter 24 you built inheritance hierarchies, and you ended it on a cliffhanger: when you called a function through a base-class reference, you always got the *base* version, even when the object underneath was really a derived type. That was deliberate. This chapter is the resolution. By the end of it, one line of code — `base.run()` — will be able to do different things depending on what object is actually sitting at the other end, and you will understand exactly how and why.

But first we have to set the stage, because the whole mechanism rests on one quiet fact: **a base pointer or reference can point at a derived object.**

Inheritance encodes an "is-a" relationship:

```text
Derived is-a Base
```

If `Derived` publicly inherits from `Base`, then every `Derived` object literally *contains* a `Base` subobject inside it.

```cpp
class Base
{
public:
    int m_base {};
};

class Derived : public Base
{
public:
    int m_derived {};
};
```

You can picture a `Derived` object as two stacked layers, the base part on top and the derived additions below:

```text
Derived object

+--------------------+
| Base subobject     |
|   m_base           |
+--------------------+
| Derived part       |
|   m_derived        |
+--------------------+
```

Because that `Base` subobject is genuinely *there*, C++ lets you grab a handle to just the base portion. A base reference or a base pointer can bind to a derived object:

```cpp
Derived d {};

Base& ref { d };
Base* ptr { &d };
```

Nothing is copied here. `ref` and `ptr` are simply other ways of looking at the *same* `d` — but looking at it through a narrower window. That window is the doorway into polymorphism.

### A base reference sees only the base interface

Here is the catch. Even though `ref` ultimately refers to a `Derived` object, the *static type* of the expression `ref` is `Base&`. The compiler decides which members you may name based on that static type, not on what the object really is.

```cpp
Derived d {};
Base& ref { d };

ref.m_base = 1;      // ok: m_base is part of Base's interface
// ref.m_derived = 2; // error: Base's interface does not expose m_derived
```

The object is a full `Derived`, with `m_derived` and all — but through a `Base&` you simply cannot name `m_derived`, because the type of the expression is `Base&` and `Base` has no such member.

```text
actual object:      Derived
expression type:    Base&
visible interface:  Base's public/protected members only
```

This is not a limitation to fight against; it is the feature. A base reference lets you handle a derived object *without committing to its exact type*. You see the common interface and nothing more.

### Why this matters

Suppose your program produces several kinds of run result, all of which share a name:

```cpp
#include <string>

class Result
{
public:
    std::string name {};
};

class CrashResult : public Result
{
public:
    int signal {};
};

class CoverageResult : public Result
{
public:
    int newEdges {};
};
```

A function that only needs the shared `name` can ask for a `const Result&`:

```cpp
#include <iostream>

void printName(const Result& result)
{
    std::cout << result.name << '\n';
}
```

and then accept *either* derived type without any overloads:

```cpp
CrashResult crash {};
CoverageResult coverage {};

printName(crash);
printName(coverage);
```

`printName` is written once. It works on "anything that is a `Result`" instead of needing one copy per derived type. That economy — one function, many concrete types — is what we are building toward.

### A base handle does not (yet) give you derived behavior

There is one more thing to settle before we add the magic word, and it is the exact trap from Chapter 24. Consider this *intentionally non-virtual* example:

```cpp
#include <iostream>

class Base
{
public:
    void identify() const
    {
        std::cout << "Base\n";
    }
};

class Derived : public Base
{
public:
    void identify() const
    {
        std::cout << "Derived\n";
    }
};

int main()
{
    Derived d {};
    Base& ref { d };

    ref.identify(); // prints "Base", not "Derived"

    return 0;
}
```

Read that output again: `Base`. The object is a `Derived`, but the call picked `Base::identify()`. Because `identify()` is an ordinary, non-virtual function, the compiler resolves it using the *static type* of `ref`, which is `Base&`. It never looks at the real object.

> **Key insight:** Binding a base reference to a derived object does not, by itself, give you polymorphic behavior. For non-virtual functions, the static type of the expression decides which function runs.

That is the gap. The next lesson closes it with a single keyword.

## 25.2 — Virtual functions and polymorphism

A **virtual function** is a member function declared so that a call through a base pointer or base reference dispatches to the *most-derived override* — the version belonging to the object's real type, not the version named by the expression's static type.

The change is one word: `virtual` on the base declaration.

```cpp
#include <iostream>

class Base
{
public:
    virtual void identify() const
    {
        std::cout << "Base\n";
    }
};

class Derived : public Base
{
public:
    void identify() const override
    {
        std::cout << "Derived\n";
    }
};
```

Now run the same code that disappointed us a moment ago:

```cpp
Derived d {};
Base& ref { d };

ref.identify(); // prints "Derived"
```

The call now follows the *actual object*, not the expression type. That is the entire payoff of the chapter — and it cost exactly one keyword.

### Static type versus dynamic type

To talk about this precisely, name the two types every expression carries:

```text
static type:
  the type known from the expression at compile time

dynamic type:
  the actual, most-derived type of the object at runtime
```

For the code above:

```cpp
Derived d {};
Base& ref { d };
```

| Expression | Static type | Dynamic type |
|------------|-------------|--------------|
| `d`        | `Derived`   | `Derived`    |
| `ref`      | `Base&`     | `Derived`    |

The whole rule fits in two lines:

- **Non-virtual** member calls use the **static type**.
- **Virtual** member calls through a pointer or reference use the **dynamic type**.

That is why `ref.identify()` flipped from `Base` to `Derived` the instant we made `identify()` virtual: we switched it from the static-type rule to the dynamic-type rule.

### How to picture virtual dispatch

```text
Base& ref
   |
   v
+--------------------+
| Derived object     |
|   Base subobject   |
|   Derived override |
+--------------------+
   |
   v
ref.identify()
   |
   v
Derived::identify()   <- chosen at runtime, from the real object
```

The reference is a `Base&`, but at the moment of the call, control is routed to the override that the *object* actually carries. This routing is called **dynamic dispatch** (or **late binding** — more on that in 25.5).

### Polymorphism

"Polymorphism" means "many forms." In this chapter, **runtime polymorphism** means: one base interface can refer to many concrete derived types, and a virtual call selects the right behavior at runtime.

```cpp
void printIdentity(const Base& base)
{
    base.identify();
}

Base b {};
Derived d {};

printIdentity(b); // Base
printIdentity(d); // Derived
```

`printIdentity` is compiled once. It does not know — and does not need to know — which derived type any caller will hand it. Each call dispatches to the correct override on its own.

### A more concrete example

This is the shape you will meet again in the chapter's lab. A `TestCase` base declares the common interface; each concrete test fills it in:

```cpp
#include <iostream>
#include <string>

class TestCase
{
public:
    virtual ~TestCase() = default;   // virtual destructor — explained in 25.4

    virtual std::string name() const
    {
        return "generic test";
    }

    virtual bool run() const
    {
        return true;
    }
};

class CrashTest : public TestCase
{
public:
    std::string name() const override
    {
        return "crash test";
    }

    bool run() const override
    {
        return false;
    }
};

class CoverageTest : public TestCase
{
public:
    std::string name() const override
    {
        return "coverage test";
    }

    bool run() const override
    {
        return true;
    }
};

void report(const TestCase& test)
{
    std::cout << test.name() << ": " << test.run() << '\n';
}
```

Calling it:

```cpp
CrashTest crash {};
CoverageTest coverage {};

report(crash);    // crash test: 0
report(coverage); // coverage test: 1
```

`report` takes a `const TestCase&`, yet both `name()` and `run()` dispatch to the derived implementations. One function body, many behaviors. That is exactly the engine a real testing or compiler framework runs on when it iterates over a list of cases and calls `run()` on each.

### An override must match the signature

For a derived function to override a base virtual, its signature must match closely: same name, same parameter types, same const-qualification. A near-miss does **not** override — it quietly creates a brand-new function.

```cpp
class Base
{
public:
    virtual void f(int) const {}
};

class Derived : public Base
{
public:
    void f(int) const override {} // overrides
};
```

These do **not** override:

```cpp
class BadDerived : public Base
{
public:
    void f(double) const {} // different parameter type -> new function
};

class BadDerived2 : public Base
{
public:
    void f(int) {} // missing const -> new function
};
```

These mismatches are some of the most frustrating bugs in C++, because everything compiles and your derived code simply never runs. The cure is the `override` keyword (25.3) — write it on every override and the compiler will reject a near-miss for you.

### Return types of an override

By default an override must declare the **same return type** as the base virtual:

```cpp
class Base
{
public:
    virtual int value() const { return 0; }
};

class Derived : public Base
{
public:
    int value() const override { return 1; }
};
```

There is one carefully-bounded exception, **covariant return types**, which we cover in 25.3.

### Do not call virtual functions from constructors or destructors

This rule surprises people, so let us reason it out rather than just memorize it.

An object is built base-part-first and torn down derived-part-first. So *during* the base constructor, the derived part does not exist yet; *during* the base destructor, the derived part is already gone. C++ handles this by treating the object as if its dynamic type were the class currently being constructed or destroyed. The upshot: a virtual call made from a base constructor or destructor resolves to the *base* version, not the derived override — the opposite of what polymorphism normally gives you.

```cpp
#include <iostream>

class Base
{
public:
    Base()
    {
        init(); // avoid: virtual call during construction
    }

    virtual void init()
    {
        std::cout << "Base init\n";
    }
};

class Derived : public Base
{
public:
    void init() override
    {
        std::cout << "Derived init\n";
    }
};
```

Constructing a `Derived` here prints `Base init`, not `Derived init`, because while the `Base` constructor runs, the object is still "just a `Base`." This is not a compiler bug; it is the language protecting you from calling derived code that operates on not-yet-constructed members.

> **Warning:** A virtual call from a constructor or destructor does *not* dispatch to derived overrides. It resolves to the version belonging to the class currently being constructed or destroyed.

The fix is to design construction so each constructor does its own layer's setup, and to call any "finish initializing" hook *after* the object is fully built:

```cpp
Derived d {};
d.init(); // safe: the whole object exists now
```

### The cost of virtual functions

Virtual functions are powerful, not free. The trade-offs:

- a virtual call is slightly more expensive than a direct call (we will see why in 25.6),
- a polymorphic object usually carries a hidden pointer to its dispatch data, so it is a little larger,
- control flow is less obvious — one line of source can run many different function bodies,
- over-used inheritance hierarchies can grow rigid and hard to change.

For CS6340 you should not fear virtual functions: LLVM's entire instruction model is a virtual hierarchy, and you will read it constantly. But treat `virtual` as a deliberate design choice, not the default for every member function.

## 25.3 — The `override` and `final` specifiers, and covariant return types

### `override`

`override` is a promise you make to the compiler about a derived function:

```text
This function is meant to override a virtual base function.
If it doesn't, make it a compile error.
```

It changes no behavior on its own. What it buys you is a *check*. Use it on every derived override and the entire class of "silent near-miss" bugs from 25.2 disappears.

```cpp
class Base
{
public:
    virtual void execute() const {}
};

class Derived : public Base
{
public:
    void execute() const override {} // verified to override
};
```

Without `override`, a dropped `const` silently makes a new, unrelated function — the bug compiles and the override never fires:

```cpp
class BadDerived : public Base
{
public:
    void execute() {} // missing const; does NOT override, but compiles
};
```

Add `override` and the compiler catches the same mistake at the declaration:

```cpp
class BetterDerived : public Base
{
public:
    void execute() override {} // compile error: does not override Base::execute() const
};
```

> **Best practice:** Put `virtual` on the base declaration and `override` on every derived override. You may also repeat `virtual` in the derived class, but `override` is strictly more useful because it *verifies* the override relationship instead of just asserting it.

```cpp
class Base
{
public:
    virtual void f();
};

class Derived : public Base
{
public:
    void f() override;
};
```

### `final`

`final` is the opposite kind of guarantee: it stops the chain. It says "no further overriding" on a function, or "no further inheriting" on a class.

Lock a single virtual function against further overriding:

```cpp
class Base
{
public:
    virtual void f() {}
};

class Derived : public Base
{
public:
    void f() final {} // a class derived from Derived may not override f
};
```

Lock a whole class against being inherited from:

```cpp
class FinalClass final
{
};
```

Reasonable uses of `final`:

- enforce a design boundary ("this hierarchy stops here"),
- document intent to future readers,
- occasionally let the compiler optimize, since it knows no further override exists.

Do not sprinkle `final` everywhere out of habit. Reach for it when the design genuinely requires the stop — for instance, the lab marks `BranchInst::opcodeName()` as `final` to say "this opcode name is fixed; no subclass may change it."

### Covariant return types

We said an override normally must return the same type as the base virtual. There is one exception, and it has a name: **covariant return types**.

A virtual function that returns a *pointer or reference to a base class* may be overridden by a function that returns a *pointer or reference to a more-derived class* in the same hierarchy.

```cpp
class Animal
{
public:
    virtual ~Animal() = default;

    virtual Animal* clone() const
    {
        return new Animal {};
    }
};

class Cat : public Animal
{
public:
    Cat* clone() const override   // returns Cat*, not Animal* — still a valid override
    {
        return new Cat {};
    }
};
```

`Cat*` is accepted because a `Cat*` *is-a* `Animal*` — it is strictly more specific, never less. The benefit is at the call site: when you hold a `Cat` and call `clone()`, you get back a `Cat*` directly, with no cast.

The exception applies only to pointers and references within a related hierarchy:

```text
Base*  ->  Derived*
Base&  ->  Derived&
```

It is not a general "return a related type" rule. You cannot, for example, override `virtual int value() const` with `long value() const` — those are unrelated types, not a base/derived pointer pair.

> **Note:** For introductory CS6340 work, you mostly need to *recognize* covariant return types when you read them. You will rarely need to design with them right away.

## 25.4 — Virtual destructors, virtual assignment, and overriding virtualization

### Virtual destructors

Here is a rule that prevents a real, silent resource leak. **If a class has virtual functions and might be deleted through a base pointer, its destructor must be virtual.**

Watch what goes wrong without it:

```cpp
#include <iostream>

class Base
{
public:
    virtual void f() {}

    ~Base()                       // NOT virtual — the bug
    {
        std::cout << "~Base\n";
    }
};

class Derived : public Base
{
public:
    ~Derived()
    {
        std::cout << "~Derived\n";
    }
};

int main()
{
    Base* ptr { new Derived {} };
    delete ptr;                   // only ~Base runs; ~Derived is skipped
}
```

When you `delete` through a `Base*`, the compiler looks at the *static type* of the pointer to decide which destructor to call — exactly the static-type rule from 25.1. Since `~Base` is not virtual, it calls `~Base` and stops. `~Derived` never runs. Any resource the derived part owned is leaked, and in real code this is a quietly devastating bug.

The fix is one keyword:

```cpp
class Base
{
public:
    virtual ~Base() = default;    // now deletion is polymorphic
    virtual void f() {}
};
```

Now `delete ptr` dispatches through the vtable like any other virtual call, and the destructors run in the correct order:

```text
delete (Base*) pointing at a Derived

Derived::~Derived()     <- derived part first
        |
        v
Base::~Base()           <- then the base part
```

> **Best practice:** If a class is meant to be used polymorphically — it has virtual functions and may be deleted through a base pointer — give it a virtual destructor. `virtual ~Base() = default;` is the usual idiom.

This is precisely what the chapter's lab proves with a counter: each derived destructor bumps `s_destroyed`, and the grader checks that the counter ticks up when an object is deleted through a `unique_ptr<Inst>` — direct evidence that the virtual destructor chain ran the derived destructor before the base one.

### Virtual assignment

Assignment through a base reference is a different animal, and it is usually not what you want.

```cpp
Base& baseRef { derived };
baseRef = otherDerived;   // assigns only the Base portion
```

The default assignment operator that gets selected here is `Base::operator=`, which copies only the base subobject. The derived part of `derived` is left untouched. Building genuinely polymorphic assignment is fiddly and easy to get wrong, and it tends to produce the half-updated objects we will name in 25.9.

For most code, the practical guidance is:

- avoid assigning polymorphic objects through base references,
- prefer plain value types when you want ordinary assignment,
- prefer owning (smart) pointers when you want polymorphism,
- reach for an explicit cloning/copy interface only when you truly need polymorphic copying.

### Ignoring virtualization on purpose

Sometimes a derived override wants to *extend* the base behavior rather than replace it. You can call the base version explicitly with the scope operator:

```cpp
#include <iostream>

class Derived : public Base
{
public:
    void f() override
    {
        Base::f();                            // run the base version first
        std::cout << "Derived extra behavior\n";
    }
};
```

The qualified name `Base::f()` deliberately bypasses virtual dispatch and calls that exact version. This "do the base work, then add to it" pattern is common and perfectly idiomatic.

You can even force the base version from *outside* the class:

```cpp
derived.Base::f(); // bypasses virtual dispatch for this one call
```

That outside form should be rare. It reads much more clearly inside an override (where it means "extend my base") than at a call site (where it looks like you are fighting the language).

### Should every destructor be virtual?

No — and this is a useful distinction to hold in your head:

```text
ordinary value class:
  no virtual functions
  not deleted through a base pointer
  destructor does NOT need to be virtual

polymorphic base class:
  has virtual functions
  used through Base*
  destructor SHOULD be virtual
```

A virtual destructor costs a little (it implies the object carries dispatch data) and it signals "this type is for polymorphic use." Add it when the design is polymorphic, leave it off when the type is a plain value.

## 25.5 — Early binding and late binding

Two words let us name the mechanism precisely. **Binding** is connecting a function call in your source to the actual function body that will run. **Dispatching** is the act of selecting and invoking that body.

### Early binding

**Early binding** (also called *static binding*) happens at compile time. The compiler knows, from the types alone, exactly which function the call refers to, and wires it in directly.

Overload resolution is early binding:

```cpp
#include <iostream>

void print(int)
{
    std::cout << "int\n";
}

void print(double)
{
    std::cout << "double\n";
}

print(3); // compiler binds this to print(int), here and now
```

Non-virtual member calls are early-bound too — the compiler picks based on the static type:

```cpp
Base& ref { derived };
ref.identify(); // if identify() is non-virtual, binds to Base::identify()
```

### Late binding

**Late binding** (also called *dynamic binding*) happens at runtime. The exact function is chosen while the program runs, based on the real object.

A virtual call through a base pointer or reference is late-bound:

```cpp
Base& ref { derived };
ref.identify(); // if virtual, chooses Derived::identify() at runtime
```

Why must this wait until runtime? Because the actual object can vary from call to call, and the compiler genuinely cannot know in advance:

```cpp
void run(const TestCase& test)
{
    test.execute(); // dynamic type might be CrashTest, CoverageTest, ...
}
```

The compiler knows `test` is a `TestCase&`. It does *not* know which concrete object every caller, present and future, will pass in. The decision has to be deferred to the moment of the call. That deferral is late binding — the cost, and the power, of virtual functions.

### A function pointer makes the idea concrete

Late binding can feel like magic until you connect it to something you already understand: a function pointer. A function pointer is just a variable that holds the address of a function, so the call goes "through" the variable rather than to a fixed name.

```cpp
#include <iostream>

int add(int a, int b)
{
    return a + b;
}

int (*operation)(int, int) { add };

std::cout << operation(1, 2) << '\n'; // calls whatever operation points at
```

Virtual dispatch is not literally a function pointer you wrote, but the mental model is right:

```text
the object carries a route to the correct function table
the call consults that table
the runtime picks the matching implementation
```

The next lesson opens up that "function table."

## 25.6 — The virtual table

How does a `Base&` actually find the derived override at runtime? The common implementation is the **virtual table**, usually shortened to **vtable**.

> **Note:** The C++ standard requires the *behavior* of virtual dispatch, not this specific machinery. But the vtable is the near-universal implementation, and it is the right mental model — it explains every cost and rule in this chapter at once.

### The basic idea

For each class that has virtual functions, the compiler builds a small table of function pointers — one slot per virtual function — pointing at the correct version for that class.

```cpp
class Base
{
public:
    virtual void f();
    virtual void g();
};

class Derived : public Base
{
public:
    void f() override;
    void g() override;
};
```

Conceptually, two tables exist:

```text
Base vtable                  Derived vtable
+-----+------------+         +-----+------------+
| f   | Base::f    |         | f   | Derived::f |
| g   | Base::g    |         | g   | Derived::g |
+-----+------------+         +-----+------------+
```

Each polymorphic *object* then carries a hidden pointer — the **vptr** — to its class's vtable. (This hidden pointer is why polymorphic objects are slightly larger than non-polymorphic ones.)

```text
Base object                 Derived object
+----------------+          +----------------+
| vptr ------+   |          | vptr ------+   |
| data       |   |          | Base data  |   |
+------------|---+          | Derived... |   |
             |              +------------|---+
             v                           v
        Base vtable                 Derived vtable
```

The vptr is set up by the constructor, so by the time an object is alive, it always points at the vtable of its real type.

When you make a virtual call:

```cpp
Base& ref { derived };
ref.f();
```

the runtime walks the chain:

```text
ref refers to a Derived object
    |
    v
the object's vptr points to the Derived vtable
    |
    v
the slot for f() holds Derived::f
    |
    v
call Derived::f
```

That is dynamic dispatch in full. Notice the table belongs to the *object*, which is why the static type of `ref` (`Base&`) is irrelevant — the route always starts from the real object's vptr.

### Why virtual calls cost a little more

A non-virtual call can often compile to a single direct jump:

```text
call Base::f
```

A virtual call generally has to:

```text
load the vptr from the object
look up the function pointer in the table
call through that function pointer
```

A couple of extra memory accesses and an indirect call. The overhead is small but real. When the compiler can *prove* the dynamic type — say, you call a virtual function on a local object whose type it knows exactly — it may "devirtualize" and skip the lookup. But in ordinary polymorphic code, where the dynamic type genuinely varies, dynamic dispatch costs that little bit.

### Why the vtable model is worth knowing

Hold this one picture and a lot of the chapter falls out of it:

- virtual functions need runtime dispatch data — that is the vtable and the vptr;
- polymorphic objects are larger — they carry a vptr;
- constructors and destructors behave specially with virtual calls — the vptr is being set up or torn down, so it points at the *current* class's table (25.2, 25.4);
- deleting through a base pointer needs a virtual destructor — otherwise the destructor call is not routed through the vtable (25.4);
- virtual calls are slightly slower — the lookup is the cost.

You do not need to memorize compiler ABI details for CS6340. You do need this conceptual model, because it makes every "why" in the chapter obvious.

## 25.7 — Pure virtual functions, abstract base classes, and interface classes

Sometimes a base class wants to declare *that* an operation exists without giving it any sensible default. "Every test must have a `name()`, but there is no generic name." For that, you make the function **pure virtual** by writing `= 0`:

```cpp
#include <string>

class TestCase
{
public:
    virtual ~TestCase() = default;
    virtual std::string name() const = 0;   // pure virtual
    virtual bool run() const = 0;            // pure virtual
};
```

A pure virtual function is a contract: it says

```text
Every concrete TestCase MUST provide name() and run().
```

Because `TestCase` now has unimplemented operations, it becomes an **abstract base class** — and you cannot create one directly:

```cpp
// TestCase test; // error: cannot instantiate an abstract class
```

That restriction is a feature. A bare `TestCase` would be meaningless (what would `name()` even return?), so the language refuses to let you make one. You can still use references and pointers to it, which is exactly how polymorphism wants you to work anyway:

```cpp
#include <iostream>

void report(const TestCase& test)
{
    std::cout << test.name() << ": " << test.run() << '\n';
}
```

### A concrete derived class

A derived class becomes **concrete** — instantiable — once it overrides *every* pure virtual it inherited:

```cpp
class CrashTest : public TestCase
{
public:
    std::string name() const override
    {
        return "crash test";
    }

    bool run() const override
    {
        return false;
    }
};
```

`CrashTest` implements both `name()` and `run()`, so you can create `CrashTest` objects and pass them wherever a `const TestCase&` is wanted.

### An abstract derived class

If a derived class leaves *any* inherited pure virtual unimplemented, it is still abstract and still cannot be instantiated:

```cpp
class NamedTest : public TestCase
{
public:
    std::string name() const override
    {
        return "named test";
    }

    // run() is left unimplemented -> NamedTest is still abstract
};
```

This is occasionally useful for sharing partial implementations down a hierarchy, but mostly it is a thing to recognize: "abstract" sticks until every pure virtual has a body.

### A pure virtual function may still have a definition

A subtle but legal point: `= 0` makes a function pure (so the class is abstract), yet you may *also* provide a definition for it, to be used as shared behavior an override can opt into:

```cpp
#include <iostream>

class Base
{
public:
    virtual ~Base() = default;
    virtual void f() = 0;   // pure: Base is abstract
};

void Base::f()              // ...but still defined
{
    std::cout << "shared base behavior\n";
}

class Derived : public Base
{
public:
    void f() override
    {
        Base::f();          // opt in to the shared behavior
        std::cout << "derived behavior\n";
    }
};
```

This is uncommon at the beginner level. Recognize it; you do not need to design with it yet.

### Interface classes

When an abstract base contains *only* a virtual destructor and pure virtual functions — no data, no implementation — we call it an **interface class**. It describes required behavior and nothing else.

```cpp
#include <string>

class IInputProvider
{
public:
    virtual ~IInputProvider() = default;
    virtual std::string nextInput() = 0;
    virtual bool hasMore() const = 0;
};
```

Many unrelated implementations can satisfy the same interface:

```cpp
class FileInputProvider : public IInputProvider
{
public:
    std::string nextInput() override;
    bool hasMore() const override;
};

class RandomInputProvider : public IInputProvider
{
public:
    std::string nextInput() override;
    bool hasMore() const override;
};
```

and your code can depend on the *interface* rather than on any one implementation:

```cpp
void fuzz(IInputProvider& inputs)
{
    while (inputs.hasMore())
        runTarget(inputs.nextInput());
}
```

`fuzz` works with a file source, a random source, a network source, or any provider written in the future — none of which existed when `fuzz` was compiled. This is one of the most powerful patterns in object-oriented C++.

> **Note:** For the lab you mostly need to *read* this comfortably and not be startled by `= 0`. The lab's `Inst` base is exactly this shape: a virtual destructor plus a pure virtual `opcodeName()`, which is what makes `Inst` abstract and forces every instruction class to name its opcode.

## 25.8 — Virtual base classes

This lesson uses the word `virtual` for something genuinely different, so read it with that in mind. **Virtual base classes** have nothing to do with virtual functions — they solve a multiple-inheritance problem called the **diamond**.

Picture a hierarchy where two parents share a grandparent:

```text
        PoweredDevice
        /           \
   Scanner        Printer
        \           /
          Copier
```

Without virtual inheritance, a `Copier` ends up with *two* separate `PoweredDevice` subobjects — one reached through `Scanner`, one through `Printer`:

```text
Copier
+-----------------------------+
| Scanner                     |
|   PoweredDevice subobject   |
+-----------------------------+
| Printer                     |
|   PoweredDevice subobject   |
+-----------------------------+
```

Now an innocent call is ambiguous — *which* `PoweredDevice` did you mean?

```cpp
copier.powerOn(); // ambiguous: there are two PoweredDevice parts
```

**Virtual inheritance** tells the compiler to share a single base subobject instead of duplicating it. You write `virtual` on the inheritance of the *intermediate* classes:

```cpp
class PoweredDevice
{
};

class Scanner : virtual public PoweredDevice
{
};

class Printer : virtual public PoweredDevice
{
};

class Copier : public Scanner, public Printer
{
};
```

Now `Copier` holds exactly one shared `PoweredDevice`, and the ambiguity is gone:

```text
Copier
+-----------------------------+
| Scanner part                |
| Printer part                |
| shared PoweredDevice part   |
+-----------------------------+
```

This is advanced, and it is unlikely to be central to your early lab work. The one thing to carry forward:

> **Key insight:** A *virtual base class* and a *virtual function* are unrelated features that happen to share a keyword. Virtual functions give you runtime dispatch; virtual base classes deduplicate a shared base in diamond inheritance.

## 25.9 — Object slicing

We have leaned hard on pointers and references for polymorphism. This lesson shows what happens when you don't — when you copy a derived object into a base object *by value* — and why that quietly destroys polymorphism.

It is called **object slicing**: copying a derived object into a base object slices the derived portion clean off.

```cpp
class Base
{
public:
    int base {};
};

class Derived : public Base
{
public:
    int derived {};
};

Derived d {};
d.base = 1;
d.derived = 2;

Base b { d }; // slicing: only the Base part is copied
```

The copy keeps only what fits in a `Base`:

```text
Derived source                       Base destination
+----------------+                   +--------------+
| Base: base=1   |  ---- copied ---> | Base: base=1 |
+----------------+                   +--------------+
| Derived: =2    |  ---- dropped
+----------------+

the derived part has no place to go in a Base
```

After the copy, `b` is *genuinely just a `Base`*. It has no `derived` member, and — crucially — its dynamic type is `Base`. There is no `Derived` underneath anymore, so virtual calls on `b` resolve to `Base` versions. The polymorphism is gone, silently, with no error.

### Slicing through function parameters

This is the most common way slicing sneaks in. A by-value parameter slices:

```cpp
void print(Base base) // by value: a Derived argument gets sliced into a Base copy
{
    base.identify();
}
```

Called with a derived object:

```cpp
Derived d {};
print(d); // print() receives a sliced Base copy; virtual calls hit Base versions
```

The cure is the same `const Base&` you have seen throughout the chapter — a reference (or pointer) refers to the original object and preserves its dynamic type:

```cpp
void print(const Base& base) // by reference: the real Derived survives
{
    base.identify();
}
```

> **Best practice:** Pass polymorphic objects by reference (`const Base&`, `Base&`) or by pointer (`Base*`), never by value. By-value parameters slice.

### Slicing into a vector

A subtler trap, and a common one:

```cpp
#include <vector>

std::vector<Base> objects {};

objects.push_back(Derived {}); // slices: only the Base part is stored
```

A `std::vector<Base>` stores actual `Base` objects, laid out back to back. There is no room for a `Derived` in a `Base`-sized slot, so every push slices:

```text
std::vector<Base>

[ Base ][ Base ][ Base ]      <- what you actually get

not

[ Base ][ Derived ][ Other ]  <- what you might have hoped for
```

For a polymorphic collection you must store *pointers*, because a pointer is the same size regardless of what it points at. Smart pointers give you that without manual `delete`:

```cpp
#include <memory>
#include <vector>

std::vector<std::unique_ptr<Base>> objects {};

objects.push_back(std::make_unique<Derived>());
```

Now the vector holds owning pointers, and each pointer can route to a full derived object living on the heap:

```text
vector of unique_ptr<Base>

[ptr] [ptr] [ptr]
  |     |     |
  v     v     v
Base  Derived Other        <- real objects, full dynamic type intact
```

This is exactly why the chapter's lab uses `std::vector<std::unique_ptr<Inst>>` for its instruction list: the full derived instruction lives on the heap, the smart pointer holds a typed handle to it, and virtual dispatch keeps working through that handle.

### The half-updated object

Assignment through a base reference produces its own broken result. It copies only the base part of the derived object, leaving the derived part stale:

```cpp
Derived d1 {};
Derived d2 {};

Base& ref { d1 };
ref = d2; // assigns the Base part of d2 into d1; the Derived part of d1 is untouched
```

`d1` is now a mismatched object: its base half came from `d2`, its derived half is the old `d1`.

```text
d1 after ref = d2:
  Base part    <- copied from d2
  Derived part <- still the original d1
```

Teaching materials sometimes call this a "Frankenobject." It is the assignment-through-a-base-reference hazard we flagged in 25.4, seen from the slicing side. The safe rule is blunt:

> **Warning:** Do not assign polymorphic objects through base references. You get a half-updated object, not a real copy.

### A short checklist to avoid slicing

Prefer:

```cpp
void f(const Base& base); // read polymorphically
void f(Base& base);       // mutate polymorphically
void f(Base* base);       // optional / nullable polymorphic parameter
```

Avoid:

```cpp
void f(Base base);        // slices
std::vector<Base> v;      // slices every derived insert
```

For ownership of polymorphic objects, reach for smart pointers:

```cpp
std::unique_ptr<Base>
std::vector<std::unique_ptr<Base>>
```

## 25.10 — Dynamic casting

So far polymorphism has flowed one way: you hold a `Base&` and call virtual functions, and the right override runs without you ever asking "what type is this, really?" That is the ideal. But occasionally you genuinely need to go the other way — you have a base handle and you need to know whether the object is a *specific* derived type, so you can use that type's extra interface. The tool for that is `dynamic_cast`.

`dynamic_cast` performs a **runtime-checked** conversion within a polymorphic hierarchy. It checks the object's real dynamic type and either gives you a valid derived handle or tells you "no."

```cpp
class Base
{
public:
    virtual ~Base() = default;   // a virtual function makes the class polymorphic
};

class Derived : public Base
{
public:
    void derivedOnly() {}
};
```

### The pointer form (the one to prefer)

With a pointer, failure is reported as `nullptr` — which you can test inline:

```cpp
void maybeUseDerived(Base* base)
{
    if (Derived* derived { dynamic_cast<Derived*>(base) })
    {
        derived->derivedOnly(); // runs only when base really points at a Derived
    }
}
```

If `base` really points at a `Derived`, the cast yields a valid `Derived*`. If it points at something else (or is `nullptr`), the cast yields `nullptr` and the `if` simply skips.

```text
Base* -> actual Derived object        Base* -> actual Other object
  |                                      |
  v                                      v
dynamic_cast<Derived*>                 dynamic_cast<Derived*>
  |                                      |
  v                                      v
valid Derived*                         nullptr
```

### The reference form

There is a reference form, but failure cannot be a "null reference" (no such thing exists), so it throws `std::bad_cast` instead — which forces a `try`/`catch`:

```cpp
#include <iostream>
#include <typeinfo>

try
{
    Derived& derived { dynamic_cast<Derived&>(baseRef) };
    derived.derivedOnly();
}
catch (const std::bad_cast&)
{
    std::cout << "not a Derived\n";
}
```

> **Best practice:** Prefer the pointer form of `dynamic_cast`. A `nullptr` you can test with a plain `if` is easier and clearer than catching `std::bad_cast`. The lab requires the pointer form for exactly this reason.

### When to reach for `dynamic_cast`

Use it sparingly. Each `dynamic_cast` is a small admission that your base interface did not, on its own, say everything you needed.

Reasonable reasons:

- working with an existing polymorphic API you do not control,
- one derived type genuinely has special behavior the base interface shouldn't carry,
- diagnostics and debugging,
- replacing a chain of *unsafe* casts with a checked one.

Smells worth questioning:

- dodging a virtual function that really ought to exist,
- many type checks in a row,
- a design where every operation begins by asking "what concrete type are you?"

> **Tip:** If your code constantly downcasts, treat it as a design signal: the base interface is probably missing a virtual function. Add the virtual function and the downcasts melt away.

### `dynamic_cast` versus `static_cast`

`static_cast` will *also* compile a downcast — but it performs **no runtime check**. If the object is not actually the target type, you get undefined behavior with no warning:

```cpp
Derived* derived { static_cast<Derived*>(base) }; // unsafe if base is not really a Derived
```

Use `dynamic_cast` when you need a *checked* downcast in a polymorphic hierarchy. (This is why the lab forbids `static_cast` down the `Inst` hierarchy.)

### The LLVM connection

In LLVM you will rarely see `dynamic_cast` itself. Instead LLVM ships its own faster, RTTI-free family of helpers:

```cpp
isa<T>(x)       // is x a T?  -> bool
cast<T>(x)      // x IS a T (assert it, then convert)
dyn_cast<T>(x)  // is x a T? if so convert, else nullptr
```

These are LLVM-specific, not standard C++ — but the *reasoning* is identical to `dynamic_cast`: inspect or convert from a general base-ish type to a more specific one when the runtime object actually supports it. Understanding `dynamic_cast` here is what makes `dyn_cast<CallInst>(&I)` read as ordinary code later.

## 25.11 — Printing inherited classes using `operator<<`

You learned in Chapter 21 to print a custom type by overloading `operator<<` as a *non-member* function:

```cpp
#include <ostream>

std::ostream& operator<<(std::ostream& out, const SomeType& value)
{
    out << value.something();
    return out;
}
```

Now combine that with polymorphism and a problem appears, because **a non-member function cannot be virtual.** Virtual-ness is a property of member functions only. So how do we get `std::cout << someBaseRef` to print the *derived* form?

### The problem, seen directly

Suppose we write a separate `operator<<` for the base and for the derived type:

```cpp
#include <iostream>

class Base
{
public:
    virtual ~Base() = default;
};

class Derived : public Base
{
};

std::ostream& operator<<(std::ostream& out, const Base&)
{
    return out << "Base";
}

std::ostream& operator<<(std::ostream& out, const Derived&)
{
    return out << "Derived";
}
```

Then:

```cpp
Derived d {};
Base& ref { d };

std::cout << ref << '\n'; // prints "Base"
```

We are back to the 25.1 trap. Overload resolution is a *compile-time* decision, and it sees the static type `Base&`, so it picks the `Base` overload. The real object never gets a vote. Writing more overloads will not fix this, because the choice is made before runtime.

### The fix: delegate to a virtual member

The trick is to write **one** non-member `operator<<` that does nothing but call a **virtual member** function. The non-member is not virtual, but the member it calls *is* — so the dispatch happens inside it.

```cpp
#include <iostream>
#include <ostream>

class Base
{
public:
    virtual ~Base() = default;

    virtual void print(std::ostream& out) const
    {
        out << "Base";
    }
};

class Derived : public Base
{
public:
    void print(std::ostream& out) const override
    {
        out << "Derived";
    }
};

std::ostream& operator<<(std::ostream& out, const Base& base)
{
    base.print(out); // virtual dispatch happens HERE
    return out;
}
```

Now there is a single `operator<<`, and it works polymorphically:

```cpp
Derived d {};
Base& ref { d };

std::cout << ref << '\n'; // prints "Derived"
```

The flow is worth tracing once:

```text
operator<<(ostream, const Base&)
    |
    v
base.print(out)
    |
    v
virtual dispatch on the real object
    |
    v
Derived::print(out)
```

The non-member operator stays simple and fixed; all the per-type variation lives in the virtual `print`, which each derived class overrides.

### A tidier variant

If you would rather not expose `print` as public behavior, make it private/protected and let `operator<<` be a `friend`:

```cpp
#include <iostream>
#include <ostream>

class Base
{
private:
    virtual void print(std::ostream& out) const
    {
        out << "Base";
    }

public:
    virtual ~Base() = default;

    friend std::ostream& operator<<(std::ostream& out, const Base& base)
    {
        base.print(out);
        return out;
    }
};
```

Derived classes still override `print` (an override may have different access from the base declaration), and callers still write the natural `out << obj`.

> **Key insight:** Non-member operator overloads are not virtual, so polymorphic behavior must be routed through a virtual *member*. The non-member operator becomes a one-line forwarder to that member.

For the lab, the point is not to memorize this exact code. It is to internalize the principle, because it generalizes: whenever you need polymorphism somewhere that can't be virtual, delegate to a virtual member that can.

## 25.x — Chapter 25 summary and quiz

The whole chapter, distilled:

- A base pointer or reference can bind to a derived object; it exposes only the *base* interface.
- Non-virtual calls are resolved by **static type**; virtual calls through a pointer/reference are resolved by **dynamic type**.
- A **virtual function** enables runtime dispatch: one base interface, many derived behaviors. That is runtime polymorphism.
- Use `override` on every derived override so the compiler catches near-miss signatures.
- Use `final` to stop further overriding (on a function) or inheriting (on a class) — deliberately, not by default.
- **Covariant return types** let an override return a more-derived pointer/reference than the base virtual.
- A polymorphic base class should have a **virtual destructor**, or deleting through a base pointer skips the derived destructor.
- Do not make virtual calls from constructors or destructors — they resolve to the current class, not derived overrides.
- **Early binding** is compile-time; **late binding** is runtime dispatch.
- The **vtable** (per-class table of function pointers) plus a per-object **vptr** is the standard implementation model — and it explains every cost in the chapter.
- A **pure virtual** function (`= 0`) makes a class **abstract** (non-instantiable); a class is abstract until every pure virtual is overridden.
- **Interface classes** are abstract bases of pure virtuals plus a virtual destructor — pure required behavior.
- **Virtual base classes** solve diamond duplication and are unrelated to virtual functions.
- **Object slicing** happens when a derived object is copied into a base by value; pass by reference/pointer and store `unique_ptr` to avoid it.
- `dynamic_cast` is a runtime-checked downcast (prefer the pointer form, which returns `nullptr` on failure).
- Non-member `operator<<` cannot be virtual; delegate to a virtual member for polymorphic printing.

### Quick self-check

A `Base` with a virtual `print()` and a `Derived` override. What does each call print, and why?

```cpp
Derived d {};
Base& ref { d };
Base  copy { d };   // slicing

ref.print();        // ?
copy.print();       // ?
```

`ref.print()` prints the **Derived** version — `ref` is a reference, so `d`'s dynamic type survives and virtual dispatch finds the override. `copy.print()` prints the **Base** version — the by-value copy sliced away the derived part, so `copy`'s dynamic type is genuinely `Base`.

## CS6340 patterns

### Base-reference dispatch

A function that operates on "any mutation" without knowing the concrete kind:

```cpp
#include <iostream>
#include <string>

class Mutation
{
public:
    virtual ~Mutation() = default;
    virtual std::string describe() const = 0;
};

class DeleteCharMutation : public Mutation
{
public:
    std::string describe() const override
    {
        return "delete char";
    }
};

void logMutation(const Mutation& mutation)
{
    std::cout << mutation.describe() << '\n';
}
```

`logMutation` never asks what kind of mutation it has. The virtual `describe()` answers for it.

### Polymorphic ownership

A heterogeneous list of mutations, stored without slicing:

```cpp
#include <iostream>
#include <memory>
#include <vector>

std::vector<std::unique_ptr<Mutation>> mutations {};

mutations.push_back(std::make_unique<DeleteCharMutation>());

for (const auto& mutation : mutations)
{
    std::cout << mutation->describe() << '\n';
}
```

`unique_ptr` keeps the full derived object on the heap, so the range-`for` dispatches correctly to each one.

### LLVM-style type inquiry

You will read this constantly in LLVM passes:

```cpp
if (auto* call = dyn_cast<CallInst>(&instruction))
{
    // use CallInst-specific API on 'call'
}
```

Read it as plain English:

```text
instruction is some general Instruction
dyn_cast asks: is it actually a CallInst?
if yes  -> use CallInst-specific operations
if no   -> get nullptr and skip the block
```

It is not literally standard `dynamic_cast`, but after this chapter the idea is no longer strange — it is the pointer-form downcast you already understand.

## Mini drill

Predict the output before you read the answer.

```cpp
#include <iostream>

class Base
{
public:
    virtual ~Base() = default;

    virtual void print() const
    {
        std::cout << "Base\n";
    }
};

class Derived : public Base
{
public:
    void print() const override
    {
        std::cout << "Derived\n";
    }
};

void byValue(Base base)
{
    base.print();
}

void byRef(const Base& base)
{
    base.print();
}

int main()
{
    Derived d {};

    byValue(d);
    byRef(d);

    return 0;
}
```

Answer:

```text
Base
Derived
```

Why:

- `byValue(d)` takes its parameter **by value**, so `d` is sliced into a `Base` copy. The copy's dynamic type is `Base`, and the virtual call prints `Base`.
- `byRef(d)` takes a **reference**, so the real `Derived` object survives. Virtual dispatch finds `Derived::print` and prints `Derived`.

That one-letter difference between a value parameter and a reference parameter is the whole chapter in miniature: references and pointers preserve dynamic type, by-value copies slice it away.
