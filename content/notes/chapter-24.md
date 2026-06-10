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

### Composition vs inheritance

Composition builds a class by giving it parts. It models a **has-a** or
**part-of** relationship.

```cpp
class FunctionReport
{
private:
    std::string m_name;
    std::vector<int> m_coveredBlocks;
};
```

A `FunctionReport` has a name and has covered block IDs.

Inheritance builds a class by acquiring the properties and behavior of a more
general class. It models an **is-a** relationship.

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

A `CoveragePass` is an `InstrumentationPass` with more specialized behavior.

### Hierarchies move from general to specific

Inheritance naturally creates hierarchies:

```
AnalysisPass
  |
  +-- CoverageAnalysisPass
  |
  +-- MutationAnalysisPass
```

The base class represents the common idea. Derived classes represent more
specific versions of that idea.

Another example:

```
Shape
  |
  +-- Rectangle
  |     |
  |     +-- Square
  |
  +-- Triangle
```

If every rectangle has a width and height, a square inherits that general shape
concept and adds stricter meaning: equal sides.

### Inheritance is powerful but not automatic good design

Use inheritance when "is-a" is true in the program's model:

```cpp
class Employee : public Person {};     // employee is a person
```

Do not use inheritance merely because one class wants to reuse code from another
class:

```cpp
class CsvWriter : public std::vector<std::string> {}; // usually poor design
```

A CSV writer is not a vector. It may have a vector, or it may depend on a vector
temporarily.

### CS6340 tie-in

LLVM itself uses class hierarchies heavily. You will see code that relies on
questions like:

```cpp
llvm::Instruction* instruction = ...;

if (auto* branch = llvm::dyn_cast<llvm::BranchInst>(instruction))
{
    // branch is a more specific kind of instruction
}
```

Conceptually:

```
Instruction
  |
  +-- BranchInst
  |
  +-- BinaryOperator
  |
  +-- CallInst
```

The derived object is still an instruction, but it has additional type-specific
operations.

---

## 24.2 - Basic inheritance in C++

### Base and derived classes

In an inheritance relationship:

| Term | Meaning |
|---|---|
| Base class | class being inherited from |
| Parent class | another name for base class |
| Superclass | another name for base class |
| Derived class | class doing the inheriting |
| Child class | another name for derived class |
| Subclass | another name for derived class |

Syntax:

```cpp
class Base
{
};

class Derived : public Base
{
};
```

The `public` keyword here means public inheritance. Public inheritance is the
normal choice for "is-a" relationships.

### Derived objects contain a base subobject

The base members are not textually copied into the derived class. A derived
object contains a base-class portion plus its own derived-class portion.

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

Object layout conceptually:

```
CoveragePass object
  |
  +-- Pass portion
  |     +-- m_name
  |
  +-- CoveragePass portion
        +-- m_numCounters
```

### Derived classes inherit accessible behavior

```cpp
CoveragePass pass { "branch coverage", 12 };

std::cout << pass.name() << '\n';        // inherited from Pass
std::cout << pass.numCounters() << '\n'; // defined in CoveragePass
```

`CoveragePass` can use `Pass` behavior because every `CoveragePass` object has a
`Pass` portion.

### Sibling classes are not directly related

```cpp
class CoveragePass : public Pass {};
class MutationPass : public Pass {};
```

Both are passes, but neither is a kind of the other:

```
Pass
  |
  +-- CoveragePass
  |
  +-- MutationPass
```

Shared base does not mean sibling classes can be used interchangeably with each
other. It means each can be used where a `Pass` is valid, subject to the rules in
later lessons.

### Inheritance chains

A class can derive from a class that is already derived:

```cpp
class Pass {};
class FunctionPass : public Pass {};
class CoverageFunctionPass : public FunctionPass {};
```

`CoverageFunctionPass` has a `FunctionPass` portion, which has a `Pass` portion.

```
Pass
  |
  +-- FunctionPass
        |
        +-- CoverageFunctionPass
```

This is useful when each level adds a genuine specialization. It becomes harmful
when the hierarchy exists only to share a few helper functions.

---

## 24.3 - Order of construction of derived classes

### Base classes construct first

When a derived object is constructed, C++ constructs the object from the most
base class down to the most derived class.

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

The base portion must be initialized before the derived portion can safely use
it.

### Inheritance-chain construction

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

Construction order:

```text
A
B
C
```

Diagram:

```
requested: construct C

1. construct A portion
2. construct B portion
3. construct C portion
```

### Destruction happens in reverse

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

For `B b {};`:

```text
A ctor
B ctor
B dtor
A dtor
```

The most-derived part is destroyed first, then the base part.

### Why this order is necessary

The derived class may call inherited functions or rely on base-class state.
Those base-class members must already be initialized.

The base class should not depend on the derived class. During base construction,
the derived portion is not ready yet.

### CS6340 tie-in

If a course or LLVM example derives from a framework class, its construction
order is still base first:

```cpp
class MyPass : public SomeFrameworkPass
{
public:
    MyPass()
    {
        // SomeFrameworkPass portion is already constructed here.
    }
};
```

Do not try to use derived-class state from a base-class constructor. In large
frameworks, this is a common source of confusing initialization bugs.

---

## 24.4 - Constructors and initialization of derived classes

### Derived constructors choose the base constructor

The derived constructor initializes the derived portion. It can also specify
which immediate base-class constructor should initialize the base portion.

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
        : Pass { std::move(name) },
          m_priority { priority }
    {
    }
};
```

`Pass { std::move(name) }` constructs the `Pass` portion. `m_priority` initializes
the `FunctionPass` portion.

### Derived constructors cannot initialize base members directly

This is wrong:

```cpp
class FunctionPass : public Pass
{
public:
    FunctionPass(std::string name)
        : m_name { std::move(name) } // error: m_name belongs to Pass
    {
    }
};
```

Even if the base member were accessible, the base constructor is responsible for
initializing the base subobject. The derived class should call an appropriate
base constructor.

### Full construction sequence

For:

```cpp
FunctionPass pass { "coverage", 10 };
```

the sequence is:

```
1. allocate enough memory for FunctionPass object
2. enter FunctionPass constructor
3. construct Pass portion using Pass{"coverage"}
4. initialize FunctionPass members, such as m_priority
5. run FunctionPass constructor body
6. return to caller
```

Base-constructor calls appear in the derived initializer list, but they execute
before the derived members.

### Immediate-parent rule

In an inheritance chain, a constructor can directly call only its immediate base
class constructor.

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
        : A { a }
    {
    }
};

class C : public B
{
public:
    C(int a, double b, char)
        : B { a, b }     // C calls B, not A directly
    {
    }
};
```

`C` passes information to `B`; `B` decides how to initialize `A`.

### Private base members stay private

Derived classes cannot directly access private base members.

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
public:
    FunctionPass()
        : Pass { "function" }
    {
        // m_name = "other"; // error: m_name is private in Pass
    }
};
```

Use base constructors and public/protected member functions instead of direct
private-member access.

### Missing default constructor errors

If the derived constructor does not name a base constructor, C++ tries to call
the base default constructor.

```cpp
class Pass
{
public:
    explicit Pass(std::string name) {}
};

class BadPass : public Pass
{
public:
    BadPass()
    {
    }
};
```

`BadPass` will not compile because `Pass` has no default constructor. Fix it by
calling the available base constructor:

```cpp
class GoodPass : public Pass
{
public:
    GoodPass()
        : Pass { "good" }
    {
    }
};
```

### Base destructors

When a derived object is destroyed, destructors run from most derived to most
base:

```
~FunctionPass()
~Pass()
```

If a base class is intended to be used polymorphically through base pointers or
references, virtual destructors become important. That topic belongs to the next
chapter on virtual functions, but the warning starts here.

---

## 24.5 - Inheritance and access specifiers

### Three member access levels

| Access specifier | Accessible from same class? | Accessible from derived class? | Accessible from public code? |
|---|---:|---:|---:|
| `public` | Yes | Yes | Yes |
| `protected` | Yes | Yes | No |
| `private` | Yes | No | No |

`protected` exists mainly for inheritance. It lets derived classes access a base
member while keeping public users out.

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
        ++m_runCount;       // ok: protected base member
    }
};
```

### Prefer private members when possible

Protected data creates tighter coupling between base and derived classes. If the
base changes the meaning or representation of a protected member, derived
classes may break.

Prefer:

```cpp
class Pass
{
private:
    int m_runCount {};

protected:
    void incrementRunCount()
    {
        ++m_runCount;
    }
};
```

over exposing the raw member directly:

```cpp
protected:
    int m_runCount {};
```

Protected member functions are often a better compromise than protected data.

### Three inheritance types

Classes can inherit publicly, protectedly, or privately.

```cpp
class PublicDerived : public Base {};
class ProtectedDerived : protected Base {};
class PrivateDerived : private Base {};
```

If you omit the inheritance access for a `class`, it defaults to private:

```cpp
class Derived : Base {}; // private inheritance
```

For normal "is-a" relationships, use public inheritance.

### How inheritance type changes inherited access

| Base member access | Public inheritance | Protected inheritance | Private inheritance |
|---|---|---|---|
| `public` | public in derived | protected in derived | private in derived |
| `protected` | protected in derived | protected in derived | private in derived |
| `private` | inaccessible | inaccessible | inaccessible |

Private base members are not directly accessible to derived classes, no matter
which inheritance type is used.

### Public inheritance preserves is-a

```cpp
class Pass {};
class FunctionPass : public Pass {};
```

Public inheritance says external code can treat a `FunctionPass` as a `Pass`
where appropriate. The base public interface remains public on the derived
object.

### Private inheritance is implementation reuse

Private inheritance makes the base interface private inside the derived object.
It does **not** model a public is-a relationship.

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
        reset(); // Benchmark can use Timer internally
    }
};
```

Public users of `Benchmark` cannot call `reset()` through the benchmark object.

In most cases, composition is clearer than private inheritance:

```cpp
class Benchmark
{
private:
    Timer m_timer;
};
```

### CS6340 tie-in

When reading inherited course or LLVM code, distinguish two questions:

1. Can this derived class implementation access the member?
2. Can public code access the member through the derived object?

Those are not always the same. A derived class can often use protected members
internally while public users cannot.

---

## 24.6 - Adding new functionality to a derived class

### Derived classes can add members

The simplest extension is adding new data or functions that exist only on the
derived type.

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

`CoveragePass` inherits `name()` and adds `branchCounters()`.

### Add functionality at the right level

If a function applies to all derived classes, it belongs in the base class.

```cpp
class Pass
{
public:
    void printName() const;
};
```

If a function applies only to one specialization, keep it in the derived class.

```cpp
class CoveragePass : public Pass
{
public:
    void printCoverageTable() const;
};
```

Putting derived-specific functions in the base class bloats the base interface
and forces unrelated derived classes to carry meaningless operations.

### Derived code can use inherited public/protected behavior

```cpp
class CoveragePass : public Pass
{
public:
    void describe() const
    {
        std::cout << "coverage pass: " << name() << '\n';
    }
};
```

`name()` is inherited from `Pass`. The derived class does not need to duplicate
the base implementation.

### CS6340 tie-in

If you create small helper hierarchies, keep the base honest:

```cpp
class MutationOperator
{
public:
    std::string_view name() const;
};

class ArithmeticMutationOperator : public MutationOperator
{
public:
    bool canMutate(const llvm::Instruction& instruction) const;
};
```

`canMutate` is specific to mutation operators that inspect instructions. It does
not necessarily belong on every base class in a broader analysis hierarchy.

---

## 24.7 - Calling inherited functions and overriding behavior

### Function lookup starts in the most-derived class

When calling a member function on a derived object, the compiler looks for a
matching function name starting at the most-derived class. If no function with
that name is found there, it walks up the inheritance chain.

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
pass.identify();        // finds Pass::identify()
```

### Redefining behavior in the derived class

If the derived class defines a function with the same name, that function is
chosen for calls through a derived object.

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
pass.identify();        // CoveragePass::identify()
```

In this chapter, this is ordinary name hiding/redefinition. Runtime polymorphism
with `virtual`, `override`, and base-class pointers is covered in the next
chapter.

### Calling the base version explicitly

Derived functions can reuse base behavior by qualifying the base function name.

```cpp
class CoveragePass : public Pass
{
public:
    void identify() const
    {
        std::cout << "CoveragePass\n";
        Pass::identify();
    }
};
```

Do not write:

```cpp
void identify() const
{
    identify();         // calls itself recursively
}
```

Use `Pass::identify()` when you mean the inherited function.

### Calling base friend operators

Friend functions such as `operator<<` are not members of the base class. You
cannot call them with `Base::operator<<`. Instead, cast the derived object to a
base reference so overload resolution selects the base overload.

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
        out << static_cast<const Pass&>(pass);
        return out;
    }
};
```

Use a reference cast, not a value cast, so you do not copy/slice the base
portion.

### Overload hiding surprises

If the derived class has any function with a given name, base overloads with the
same name are hidden from unqualified lookup.

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
d.print(5);     // calls Derived::print(double), not Base::print(int)
```

Why? The compiler finds `print` in `Derived`, so it considers the `Derived`
overload set. It does not keep searching into `Base` for a better overload.

### Bring base overloads into scope with using

```cpp
class Derived : public Base
{
public:
    using Base::print;           // make Base overloads visible

    void print(double)
    {
        std::cout << "Derived double\n";
    }
};
```

Now `d.print(5)` can call `Base::print(int)`.

### CS6340 tie-in

When derived helper classes "override" names without virtual functions, calls
are resolved by static type and name lookup rules, not by runtime object type.
LLVM polymorphism uses virtual functions and type inquiry in many places, but
this chapter's examples are the simpler compile-time lookup foundation.

---

## 24.8 - Hiding inherited functionality

### Changing inherited access with using declarations

A derived class can change the access level of an inherited member it can
already access by placing a `using` declaration under a new access specifier.

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
    using BaseCounter::printCount; // inherited protected function is public here
};
```

Call:

```cpp
PublicCounter counter {};
counter.printCount();             // ok
```

No parentheses appear in the `using` declaration.

### You cannot expose private base members

Derived classes do not have access to private base members. Therefore they
cannot re-expose them:

```cpp
class Base
{
private:
    void secret();
};

class Derived : public Base
{
public:
    // using Base::secret; // error
};
```

If derived classes need controlled access, the base should provide a protected
or public member function.

### Hiding public inherited members

A derived class can make an inherited public member private:

```cpp
class Base
{
public:
    int value {};
};

class Derived : public Base
{
private:
    using Base::value;
};
```

Now public code cannot write `derived.value`.

This is sometimes used to hide poor base-class design, but it is not perfect
encapsulation. The member is still public through the `Base` interface:

```cpp
Derived d {};
Base& b { d };
b.value = 5;         // still allowed through Base&
```

### Deleting inherited-looking functionality

A derived class can declare a function as deleted to prevent calls through the
derived type.

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

Then:

```cpp
Derived d {};
// d.value();        // compile error

std::cout << d.Base::value(); // explicit base call still works
```

Deleting a derived function blocks that call syntax, but it does not erase the
base member from the base class.

### Overload access changes apply to the name

If a base class has overloads:

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
    using Base::configure;
};
```

makes all inherited `configure` overloads private through `Derived`. You cannot
use that `using` declaration to change access for only one overload.

### Design warning

If a derived class needs to hide a lot of base behavior, the inheritance
relationship may be wrong. Consider composition instead.

```cpp
class Adapter
{
private:
    Base m_base;       // expose only the operations Adapter wants
};
```

Composition lets the wrapper choose its public interface without pretending to
be a full replacement for the base class.

---

## 24.9 - Multiple inheritance

### Multiple inheritance syntax

C++ allows a class to inherit from more than one base class.

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

Conceptual layout:

```
TimedPass
  |
  +-- Named portion
  |
  +-- Timed portion
  |
  +-- TimedPass portion
```

### Mixins

A mixin is a small base class intended to add focused behavior to derived
classes.

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

Mixin classes should be small and sharply scoped. They are not usually meant to
stand alone as domain objects.

### Ambiguous names

If multiple base classes provide a member with the same name, an unqualified call
can be ambiguous.

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
// adapter.id();              // error: ambiguous

std::cout << adapter.UsbDevice::id();
std::cout << adapter.NetworkDevice::id();
```

Scope qualification resolves the ambiguity, but repeated disambiguation becomes
maintenance overhead.

### The diamond problem

The diamond problem occurs when a derived class inherits from two classes that
both inherit from the same base.

```cpp
class Device {};

class Scanner : public Device {};
class Printer : public Device {};

class Copier : public Scanner, public Printer {};
```

Shape:

```
      Device
      /    \
 Scanner  Printer
      \    /
      Copier
```

By default, `Copier` contains two `Device` base subobjects: one through
`Scanner`, one through `Printer`.

Questions become complicated:

- Should there be one device identity or two?
- Which base path should a member access use?
- How should construction initialize the shared conceptual base?

C++ has virtual base classes to address some diamond cases, but that is covered
later. The immediate lesson is that multiple inheritance can quickly complicate
object layout and name lookup.

### When multiple inheritance is reasonable

Multiple inheritance can be appropriate when:

- the base classes are small mixins,
- the relationships are independent and genuinely useful,
- ambiguity is low,
- alternatives would create more complexity.

Avoid it when:

- a single-inheritance hierarchy would work,
- composition would express the design better,
- base classes have overlapping public interfaces,
- ownership/lifetime becomes unclear,
- the hierarchy forms diamonds without a clear plan.

### CS6340 tie-in

LLVM and C++ framework code may use multiple inheritance or mixin-like patterns
internally. For course work, you usually do not need to design a multiple
inheritance hierarchy. Prefer direct helper functions, small structs, and
composition unless the framework requires inheritance.

---

## 24.x - Chapter 24 summary and quiz

### Summary

- Inheritance models an is-a relationship.
- The inherited-from class is the base/parent/superclass.
- The inheriting class is the derived/child/subclass.
- Public inheritance is the normal form for is-a relationships.
- A derived object contains a base-class portion and a derived-class portion.
- Construction runs from most base to most derived.
- Destruction runs from most derived to most base.
- A derived constructor can choose which immediate base constructor to call.
- Derived constructors cannot directly initialize private base members.
- `protected` members are accessible to derived classes but not public users.
- Prefer private data and protected/public functions over protected data.
- Public, protected, and private inheritance transform inherited access
  differently; public inheritance preserves the base public interface.
- Derived classes can add new members and redefine inherited behavior.
- Use `Base::function()` to call a hidden base implementation explicitly.
- A derived function name hides the base overload set unless `using Base::name`
  brings the base overloads into scope.
- `using Base::member` can change access for inherited members the derived class
  can already access.
- Deleted functions in a derived class can block calls through derived objects,
  but the base member still exists.
- Multiple inheritance is available but should be used sparingly because of
  ambiguity and diamond-shaped hierarchy problems.

### Inheritance design checklist

Before using inheritance, ask:

1. Is the derived object truly a kind of the base object?
2. Should public users be able to use the derived object as a base object?
3. Is shared code the only motivation? If yes, consider composition.
4. Does the base class expose a stable interface derived classes can rely on?
5. Are base destructors and virtual behavior relevant? If yes, continue into
   Chapter 25 before designing around base pointers.

### CS6340 reading checklist

When reading C++/LLVM inheritance code:

```cpp
class BranchInst : public Instruction
```

translate it as:

```
BranchInst is an Instruction.
BranchInst has an Instruction base portion.
Instruction behavior may be callable on BranchInst.
BranchInst may add branch-specific operations.
```

When a helper accepts a base reference:

```cpp
void inspect(const llvm::Instruction& I);
```

it can receive any object that is an `Instruction`, including derived instruction
types. The next chapter explains how virtual functions and polymorphism affect
which implementation runs at runtime.
