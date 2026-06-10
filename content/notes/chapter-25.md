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

## 25.1 — Pointers and references to the base class of derived objects

Inheritance gives this relationship:

```text
Derived is-a Base
```

If `Derived` publicly inherits from `Base`, then a `Derived` object contains a
`Base` subobject.

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

Conceptually:

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

Because every `Derived` is also a `Base`, C++ allows a base pointer or base
reference to bind to a derived object:

```cpp
Derived d {};

Base& ref { d };
Base* ptr { &d };
```

This is the key doorway into polymorphism.

### Base reference sees the base interface

Even though `ref` refers to a `Derived` object, the static type of `ref` is
`Base&`.

```cpp
Derived d {};
Base& ref { d };

ref.m_base = 1;      // ok
// ref.m_derived = 2; // not ok: Base interface does not expose m_derived
```

The compiler uses the expression's static type to decide which members are
available.

```text
actual object: Derived
expression type: Base&
visible interface: Base's public/protected members
```

That is why a base reference can point at a derived object without exposing
everything about the derived type.

### Why this matters

Suppose a program has multiple derived run results:

```cpp
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

A function that only needs the common `Result` interface can accept:

```cpp
void printName(const Result& result)
{
    std::cout << result.name << '\n';
}
```

Then it can be called with either derived type:

```cpp
CrashResult crash {};
CoverageResult coverage {};

printName(crash);
printName(coverage);
```

This is useful because code can operate on "anything that is a Result" instead
of needing one overload per derived type.

### Base pointers/references do not automatically give virtual behavior

This example is intentionally non-virtual:

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

    ref.identify(); // Base, not Derived

    return 0;
}
```

Because `identify()` is not virtual, C++ chooses the function using the static
type of `ref`, which is `Base&`.

This is where Chapter 25 gets serious: virtual functions change that behavior.

## 25.2 — Virtual functions and polymorphism

A virtual function is a member function where a call through a base pointer or
base reference can dispatch to the most-derived override.

```cpp
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

Now:

```cpp
Derived d {};
Base& ref { d };

ref.identify(); // Derived
```

The call uses the actual object type, not just the expression type.

### Static type vs dynamic type

Two types matter:

```text
static type:
  the type known from the expression at compile time

dynamic type:
  the actual most-derived type of the object at runtime
```

Example:

```cpp
Derived d {};
Base& ref { d };
```

```text
Expression  Static type  Dynamic type
----------  -----------  ------------
d           Derived      Derived
ref         Base&        Derived
```

For non-virtual member calls, C++ uses the static type.

For virtual member calls through a pointer/reference, C++ uses the dynamic type.

### Virtual dispatch diagram

```text
Base& ref
   |
   v
+--------------------+
| Derived object     |
| Base subobject     |
| Derived override   |
+--------------------+
   |
   v
ref.identify()
   |
   v
Derived::identify()
```

This is dynamic dispatch.

### Polymorphism

Polymorphism means "many forms."

In this chapter, runtime polymorphism means one interface can refer to many
concrete derived types and call the correct behavior at runtime.

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

`printIdentity` is written once, but it works with multiple dynamic types.

### A more concrete example

```cpp
#include <iostream>
#include <string>
#include <vector>

class TestCase
{
public:
    virtual ~TestCase() = default;

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

Calls:

```cpp
CrashTest crash {};
CoverageTest coverage {};

report(crash);
report(coverage);
```

`report` accepts `const TestCase&`, but `name()` and `run()` dispatch to the
derived implementations.

### Virtual functions must match the signature

To override a base virtual function, the derived function must match the
signature closely.

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

This does not override:

```cpp
class BadDerived : public Base
{
public:
    void f(double) const {} // different parameter type
};
```

Nor this:

```cpp
class BadDerived2 : public Base
{
public:
    void f(int) {} // missing const
};
```

Use `override` so the compiler catches mistakes.

### Return types of virtual functions

Normally, an overriding function must have the same return type.

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

There is a special exception called covariant return types, covered in 25.3.

### Do not call virtual functions from constructors or destructors

During base construction, the derived part has not been constructed yet.

During base destruction, the derived part has already been destroyed.

That means virtual calls in constructors/destructors do not behave like normal
polymorphic calls.

Bad pattern:

```cpp
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

During `Base` construction, C++ treats the object as the base part being
constructed. The derived override is not safely available.

Better pattern:

```cpp
Derived d {};
d.init(); // call after construction if needed
```

Or design construction so base constructors do base setup and derived
constructors do derived setup.

### The downside of virtual functions

Virtual functions have costs:

- runtime dispatch is slightly more expensive than a direct call
- objects with virtual functions usually carry a hidden pointer to dispatch data
- virtual behavior makes control flow less obvious from one line of code
- inheritance hierarchies can become rigid if overused

For CS6340, do not fear virtual functions. LLVM itself uses class hierarchies
and type inquiry patterns. But treat virtual functions as a design tool, not a
default for every function.

## 25.3 — The `override` and `final` specifiers, and covariant return types

### `override`

`override` tells the compiler:

```text
This function is intended to override a virtual base function.
If it does not, make it a compile error.
```

Use it on every derived override.

```cpp
class Base
{
public:
    virtual void execute() const {}
};

class Derived : public Base
{
public:
    void execute() const override {}
};
```

Without `override`, this typo silently creates a new function:

```cpp
class BadDerived : public Base
{
public:
    void execute() {} // missing const; does not override
};
```

With `override`, the compiler catches it:

```cpp
class BetterDerived : public Base
{
public:
    void execute() override {} // compile error: does not override Base::execute() const
};
```

Best habit:

```text
virtual on the base declaration
override on the derived declaration
```

Example:

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

You can write `virtual` again in the derived class, but `override` is more useful
because it verifies the relationship.

### `final`

`final` prevents further overriding or inheritance.

Prevent overriding one virtual function:

```cpp
class Base
{
public:
    virtual void f() {}
};

class Derived : public Base
{
public:
    void f() final {}
};
```

Now a further derived class cannot override `f`.

Prevent inheritance from a class:

```cpp
class FinalClass final
{
};
```

Uses:

- enforce design boundaries
- communicate that a hierarchy stops here
- sometimes allow optimization

Do not sprinkle `final` everywhere. Use it when the design actually requires
the stop.

### Covariant return types

Normally, overriding functions must use the same return type.

Covariant return types allow a virtual function that returns a pointer or
reference to a base class to be overridden with a function returning a pointer or
reference to a derived class.

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
    Cat* clone() const override
    {
        return new Cat {};
    }
};
```

`Cat*` is allowed because it is more specific than `Animal*`.

This works only for pointers/references in a related inheritance hierarchy.

Allowed shape:

```text
Base*      -> Derived*
Base&      -> Derived&
```

Not the same idea:

```cpp
virtual int value() const;
// cannot override with long value() const
```

For beginner CS6340 work, recognize covariant return types if you see them; you
probably will not need to design with them immediately.

## 25.4 — Virtual destructors, virtual assignment, and overriding virtualization

### Virtual destructors

If a class has virtual functions and may be deleted through a base pointer, give
it a virtual destructor.

Bad:

```cpp
class Base
{
public:
    virtual void f() {}
    ~Base()
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
    delete ptr; // problem if Base destructor is not virtual
}
```

The derived destructor may not run correctly.

Good:

```cpp
class Base
{
public:
    virtual ~Base() = default;
    virtual void f() {}
};
```

Now:

```cpp
Base* ptr { new Derived {} };
delete ptr; // calls Derived destructor, then Base destructor
```

Destruction order:

```text
delete Base* pointing at Derived

Derived::~Derived()
    |
    v
Base::~Base()
```

Rule:

```text
If a class is intended to be used polymorphically, make its destructor virtual.
```

### Virtual assignment

Assignment through a base reference is tricky and usually not what you want.

```cpp
Base& baseRef { derived };
baseRef = otherDerived;
```

The assignment operator works on the base portion unless you build complicated
virtual assignment machinery. That can lead to partial assignment or surprising
behavior.

For most code:

- avoid assigning polymorphic objects through base references
- prefer value types for simple assignment
- prefer owning pointers/smart pointers for polymorphic objects
- use cloning/copying interfaces only when you really need polymorphic copying

### Ignoring virtualization

You can explicitly call a base implementation:

```cpp
class Derived : public Base
{
public:
    void f() override
    {
        Base::f(); // call base version intentionally
        std::cout << "Derived extra behavior\n";
    }
};
```

This is useful when derived behavior extends base behavior.

You can also force a call to the base version from outside:

```cpp
derived.Base::f();
```

That bypasses virtual dispatch for that call. It should be rare; it is usually
clearer inside an override than in external code.

### Should all destructors be virtual?

No.

Virtual destructors cost a little and communicate that the class is meant for
polymorphic use.

Use this distinction:

```text
ordinary value class:
  no virtual functions
  not deleted through base pointer
  destructor does not need to be virtual

polymorphic base class:
  has virtual functions
  can be used through Base*
  should have virtual destructor
```

## 25.5 — Early binding and late binding

Binding means connecting a function call to the function body that will run.

Dispatching means actually choosing/invoking the function implementation.

### Early binding

Early binding happens at compile time.

```cpp
void print(int)
{
    std::cout << "int\n";
}

void print(double)
{
    std::cout << "double\n";
}

print(3); // compiler binds this to print(int)
```

For non-virtual member functions:

```cpp
Base& ref { derived };
ref.identify(); // binds to Base::identify if non-virtual
```

The compiler can choose based on static type.

### Late binding

Late binding happens at runtime.

For virtual calls through base pointer/reference:

```cpp
Base& ref { derived };
ref.identify(); // chooses Derived::identify at runtime if virtual
```

Why runtime? Because the actual object might vary:

```cpp
void run(const TestCase& test)
{
    test.execute(); // dynamic type may be CrashTest, CoverageTest, etc.
}
```

The compiler knows `test` is a `TestCase&`, but it does not know which concrete
derived object every call site will pass.

### Function pointers as a bridge idea

A function pointer stores the address of a function.

```cpp
int add(int a, int b)
{
    return a + b;
}

int (*operation)(int, int) { add };

std::cout << operation(1, 2) << '\n';
```

The call goes through an address stored in a variable.

Virtual dispatch is not exactly this in source code, but the mental model helps:

```text
object carries route to correct function table
call consults table
runtime chooses implementation
```

## 25.6 — The virtual table

The virtual table, often called the vtable, is an implementation mechanism used
by many C++ compilers for virtual dispatch.

The C++ standard does not require this exact implementation, but it is the common
mental model.

### Basic idea

For a class with virtual functions, the compiler creates a table of function
pointers.

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

Conceptual tables:

```text
Base vtable
+-----+------------+
| f   | Base::f    |
| g   | Base::g    |
+-----+------------+

Derived vtable
+-----+------------+
| f   | Derived::f |
| g   | Derived::g |
+-----+------------+
```

Objects of polymorphic classes usually contain a hidden pointer to their vtable.

```text
Base object
+----------------+
| vptr --------+ |
| data         | |
+--------------|-+
               |
               v
          Base vtable

Derived object
+----------------+
| vptr --------+ |
| Base data    | |
| Derived data | |
+--------------|-+
               |
               v
          Derived vtable
```

When you call:

```cpp
Base& ref { derived };
ref.f();
```

Conceptually:

```text
ref points to Derived object
    |
    v
object's vptr points to Derived vtable
    |
    v
slot for f points to Derived::f
    |
    v
call Derived::f
```

### Why virtual calls have overhead

A non-virtual call can often be a direct jump:

```text
call Base::f
```

A virtual call may require:

```text
load vptr
look up function pointer in table
call through function pointer
```

This overhead is usually small, but it is real.

Optimization can sometimes remove it when the compiler can prove the dynamic
type. But in normal polymorphic code, dynamic dispatch costs a little.

### Why the vtable model matters

It explains:

- why virtual functions need runtime dispatch data
- why polymorphic objects are often larger than non-polymorphic objects
- why constructors/destructors have special virtual-call behavior
- why deleting through a base pointer requires a virtual destructor
- why virtual function calls can be slightly slower

You do not need to memorize ABI details for CS6340. You do need the conceptual
model.

## 25.7 — Pure virtual functions, abstract base classes, and interface classes

A pure virtual function is a virtual function with `= 0`.

```cpp
class TestCase
{
public:
    virtual ~TestCase() = default;
    virtual std::string name() const = 0;
    virtual bool run() const = 0;
};
```

This says:

```text
Every concrete TestCase must provide name() and run().
```

`TestCase` is now an abstract base class.

You cannot instantiate it directly:

```cpp
// TestCase test; // error: abstract class
```

But you can use references/pointers to it:

```cpp
void report(const TestCase& test)
{
    std::cout << test.name() << ": " << test.run() << '\n';
}
```

### Concrete derived class

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

Because `CrashTest` implements all pure virtual functions, it is concrete.

### Abstract derived class

If a derived class leaves any pure virtual function unimplemented, it remains
abstract.

```cpp
class NamedTest : public TestCase
{
public:
    std::string name() const override
    {
        return "named test";
    }

    // run() not implemented, so NamedTest is still abstract
};
```

### Pure virtual functions may have definitions

A pure virtual function can still have a definition outside the class.

```cpp
class Base
{
public:
    virtual ~Base() = default;
    virtual void f() = 0;
};

void Base::f()
{
    std::cout << "shared base behavior\n";
}
```

A derived override can call it:

```cpp
class Derived : public Base
{
public:
    void f() override
    {
        Base::f();
        std::cout << "derived behavior\n";
    }
};
```

This is less common at the beginner level, but useful to recognize.

### Interface classes

An interface class usually contains only:

- virtual destructor
- pure virtual member functions
- no data members, or almost none

Example:

```cpp
class IInputProvider
{
public:
    virtual ~IInputProvider() = default;
    virtual std::string nextInput() = 0;
    virtual bool hasMore() const = 0;
};
```

Then different implementations can exist:

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

Code can depend on the interface:

```cpp
void fuzz(IInputProvider& inputs)
{
    while (inputs.hasMore())
        runTarget(inputs.nextInput());
}
```

This is a powerful pattern, but for Lab 1 you mainly need to read it and avoid
being surprised by `= 0`.

## 25.8 — Virtual base classes

Virtual base classes solve a specific multiple-inheritance problem: the diamond.

The diamond:

```text
        PoweredDevice
        /           \
   Scanner        Printer
        \           /
          Copier
```

Without virtual inheritance, `Copier` may contain two separate `PoweredDevice`
subobjects:

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

That can make access ambiguous:

```cpp
copier.powerOn(); // which PoweredDevice part?
```

Virtual inheritance tells C++ to share one base subobject:

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

Now conceptual layout:

```text
Copier
+-----------------------------+
| Scanner part                |
| Printer part                |
| shared PoweredDevice part   |
+-----------------------------+
```

This is advanced and less likely to be central for Lab 1. The main point is:

```text
virtual base class != virtual function
```

They both use the word `virtual`, but they solve different problems.

## 25.9 — Object slicing

Object slicing happens when a derived object is copied into a base object by
value. The derived portion is sliced off.

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

Base b { d }; // slicing
```

Conceptual copy:

```text
Derived source
+----------------+
| Base: base=1   |  copied
+----------------+ ---------> Base destination
| Derived: =2    |            +--------------+
+----------------+            | Base: base=1 |
                              +--------------+

derived part is not present in Base destination
```

After slicing, `b` is just a `Base` object. It has no `derived` member and no
dynamic type of `Derived`.

### Slicing and functions

Bad:

```cpp
void print(Base base) // pass by value slices derived objects
{
    base.identify();
}
```

If called with:

```cpp
Derived d {};
print(d);
```

the function receives a sliced `Base` copy.

Better:

```cpp
void print(const Base& base)
{
    base.identify();
}
```

References and pointers preserve the dynamic object.

### Slicing vectors

This is a common trap:

```cpp
std::vector<Base> objects {};

objects.push_back(Derived {}); // slices
```

`std::vector<Base>` stores actual `Base` objects, not a mix of `Base` and
`Derived` objects.

```text
std::vector<Base>

[ Base ][ Base ][ Base ]

not:

[ Base ][ Derived ][ OtherDerived ]
```

For polymorphic collections, use pointers:

```cpp
std::vector<std::unique_ptr<Base>> objects {};

objects.push_back(std::make_unique<Derived>());
```

Then the vector stores owning pointers, and each pointer can point at a dynamic
derived object.

```text
vector of unique_ptr<Base>

[ptr] [ptr] [ptr]
  |     |     |
  v     v     v
Base  Derived OtherDerived
```

### The Frankenobject

Assigning through a base reference can update only the base part of a derived
object.

```cpp
Derived d1 {};
Derived d2 {};

Base& ref { d1 };
ref = d2; // assigns Base part only
```

Now `d1` can contain:

```text
Base part copied from d2
Derived part still from old d1
```

This mismatched object state is sometimes called a "Frankenobject" in teaching
materials.

The safe rule:

```text
Avoid assigning polymorphic objects through base references.
```

### Avoid slicing

Use:

```cpp
void f(const Base& base); // read polymorphically
void f(Base& base);       // mutate polymorphically
void f(Base* base);       // optional/nullable polymorphic parameter
```

Avoid:

```cpp
void f(Base base);        // slices
std::vector<Base> v;      // slices derived inserts
```

For ownership:

```cpp
std::unique_ptr<Base>
std::vector<std::unique_ptr<Base>>
```

## 25.10 — Dynamic casting

`dynamic_cast` safely converts within a polymorphic inheritance hierarchy at
runtime.

Common use: you have a `Base*` or `Base&`, but you need to check whether the
actual object is a particular derived type.

```cpp
class Base
{
public:
    virtual ~Base() = default;
};

class Derived : public Base
{
public:
    void derivedOnly() {}
};
```

Pointer cast:

```cpp
void maybeUseDerived(Base* base)
{
    if (Derived* derived { dynamic_cast<Derived*>(base) })
    {
        derived->derivedOnly();
    }
}
```

If `base` actually points to a `Derived`, the cast returns a valid `Derived*`.

If not, it returns `nullptr`.

ASCII:

```text
Base* -> actual Derived object
  |
  v
dynamic_cast<Derived*>
  |
  v
Derived* success

Base* -> actual Other object
  |
  v
dynamic_cast<Derived*>
  |
  v
nullptr
```

Reference cast:

```cpp
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

Pointer casts are usually easier because failure is `nullptr`.

### When to use dynamic_cast

Use it sparingly.

Good reasons:

- interacting with an existing polymorphic API
- one derived type really has special behavior
- debugging or diagnostics
- replacing a chain of unsafe casts

Questionable reasons:

- avoiding a virtual function that should exist
- doing many type checks in a row
- designing a hierarchy where every operation asks "what concrete type are you?"

If code constantly downcasts, ask whether the base interface is missing a
virtual function.

### `dynamic_cast` vs `static_cast`

`static_cast` does not perform the same runtime safety check.

```cpp
Derived* derived { static_cast<Derived*>(base) }; // unsafe if base is not really Derived
```

Use `dynamic_cast` when you need runtime checked downcasting in a polymorphic
hierarchy.

For LLVM specifically, you will often see LLVM's own casting helpers:

```cpp
isa<T>(x)
cast<T>(x)
dyn_cast<T>(x)
```

Those are LLVM-specific patterns, not standard C++ `dynamic_cast`, but the
reasoning is similar: inspect or convert from a general base-ish type to a more
specific type when the runtime object supports it.

## 25.11 — Printing inherited classes using `operator<<`

`operator<<` is normally a non-member function:

```cpp
std::ostream& operator<<(std::ostream& out, const SomeType& value)
{
    out << value.something();
    return out;
}
```

But non-member functions cannot be virtual.

This creates a challenge for polymorphic printing.

### The problem

```cpp
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

std::cout << ref << '\n'; // prints Base, because overload resolution uses static type
```

Overload resolution is compile-time. It sees `Base&`.

### A virtual member solution

Make the non-member `operator<<` call a virtual member function.

```cpp
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
    base.print(out); // virtual dispatch happens here
    return out;
}
```

Now:

```cpp
Derived d {};
Base& ref { d };

std::cout << ref << '\n'; // Derived
```

ASCII:

```text
operator<<(ostream, Base&)
    |
    v
base.print(out)
    |
    v
virtual dispatch
    |
    v
Derived::print(out)
```

### A more flexible solution

If the base class should not expose printing as public behavior, the print helper
can be protected and `operator<<` can be a friend.

```cpp
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

Then derived classes override `print`.

For Lab 1, the key lesson is not "use this exact pattern everywhere." The key
lesson is:

```text
non-member operator overloads are not virtual,
so polymorphic behavior usually needs to delegate to a virtual member.
```

## 25.x — Chapter 25 summary and quiz

- A base pointer/reference can bind to a derived object.
- A base pointer/reference exposes the base interface.
- Non-virtual function calls are resolved using static type.
- Virtual function calls through base pointers/references use dynamic type.
- Runtime polymorphism lets one base interface dispatch to many derived
  implementations.
- Use `override` on derived virtual function overrides.
- Use `final` when a function or class should not be further overridden or
  inherited.
- Covariant return types allow an override to return a more-derived pointer or
  reference type.
- Polymorphic base classes should have virtual destructors.
- Avoid virtual calls from constructors and destructors.
- Early binding is compile-time binding.
- Late binding is runtime dispatch.
- A vtable is the common implementation model for virtual dispatch.
- Pure virtual functions use `= 0`.
- A class with at least one unimplemented pure virtual function is abstract.
- Interface classes are abstract bases focused on required behavior.
- Virtual base classes solve diamond multiple-inheritance duplication.
- Object slicing occurs when derived objects are copied into base objects by
  value.
- Avoid `std::vector<Base>` for polymorphic derived objects.
- Use pointers, often smart pointers, for polymorphic collections.
- `dynamic_cast` performs checked runtime downcasting in polymorphic hierarchies.
- Non-member `operator<<` is not virtual; delegate to a virtual member for
  polymorphic printing.

## CS6340 patterns

### Base reference dispatch

```cpp
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

`logMutation` does not need to know the concrete mutation type.

### Polymorphic ownership

```cpp
std::vector<std::unique_ptr<Mutation>> mutations {};

mutations.push_back(std::make_unique<DeleteCharMutation>());

for (const auto& mutation : mutations)
{
    std::cout << mutation->describe() << '\n';
}
```

This avoids slicing.

### LLVM-style type inquiry mental model

In LLVM code, you may see:

```cpp
if (auto* call = dyn_cast<CallInst>(&instruction))
{
    // use call-specific API
}
```

Read it like:

```text
instruction is general
dyn_cast asks whether it is a CallInst
if yes, use CallInst-specific operations
if no, get nullptr and skip
```

This is not exactly standard `dynamic_cast`, but Chapter 25 makes the idea less
weird.

## Mini drill

Predict the output:

```cpp
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

- `byValue(d)` slices `d` into a `Base` object, so dynamic type becomes `Base`.
- `byRef(d)` preserves the actual object, so virtual dispatch calls
  `Derived::print`.

