# Chapter 14 — Introduction to Classes

> Source: <https://www.learncpp.com/> (Chapter 14)
> One file per chapter. Each lesson is a section below.

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

### Procedural programming

Procedural programming organizes a program around functions and the sequence of
steps they perform.

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

Data and behavior are separate:

```
Point data       movePoint function
   |                    |
   +--------- used by --+
```

This is not bad. It is often simple and appropriate.

### Object-oriented programming

Object-oriented programming organizes code around **objects** that combine data
and behavior.

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

Now the object owns both:

```
Point object
+---------------------+
| data: x, y          |
| behavior: move()    |
+---------------------+
```

### Why OOP helps

OOP can:

- group related data and operations,
- hide implementation details,
- enforce invariants,
- reduce accidental misuse,
- make large programs easier to reason about.

Java bridge: you already know the conceptual side. The C++ challenge is the
mechanics: value semantics, references, `const`, constructors, destructors,
copying, and access control.

### The term "object"

In C++, an **object** is a region of storage that can hold a value. This is
broader than the Java sense. An `int` variable is also an object in C++.

```cpp
int x { 5 }; // x is an object
Point p {};  // p is also an object
```

---

## 14.2 — Introduction to classes

### The class invariant problem

An **invariant** is a condition that should always be true for an object to be
valid.

Example:

```cpp
struct Fraction
{
    int numerator {};
    int denominator {};
};
```

Invariant:

```
denominator must not be 0
```

With a public struct, anyone can break it:

```cpp
Fraction f { 1, 2 };
f.denominator = 0; // invalid fraction
```

### Classes protect invariants

A class can make data private and force changes through member functions.

```cpp
class Fraction
{
public:
    void setDenominator(int denominator)
    {
        if (denominator != 0)
            m_denominator = denominator;
    }

private:
    int m_numerator {};
    int m_denominator { 1 };
};
```

Now callers cannot directly write invalid internal data.

### Class definition

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

A class defines a new type. Objects of that type can be created:

```cpp
Date today {};
```

### Standard library classes

Many standard library types are classes:

```cpp
std::string
std::vector<int>
std::optional<int>
std::mt19937
```

CS6340 tie-in: LLVM APIs are class-heavy. `Module`, `Function`, `Instruction`,
`DebugLoc`, and `FunctionCallee` are class types with member functions and
invariants.

---

## 14.3 — Member functions

### Properties vs actions

Data members store object state. Member functions define object behavior.

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

### Calling member functions

```cpp
Counter counter {};
counter.increment();
std::cout << counter.value() << '\n';
```

The object before the dot is the **implicit object**. Inside the member function,
unqualified member names refer to that object.

```cpp
void increment()
{
    ++m_value; // means: increment this object's m_value
}
```

Diagram:

```
counter.increment()
   |
   v
inside increment(), implicit object is counter
```

### Member functions are declared inside the class

They can be defined inside:

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

Or declared inside and defined later:

```cpp
class Counter
{
public:
    void increment();

private:
    int m_value {};
};

void Counter::increment()
{
    ++m_value;
}
```

`Counter::increment` means "the `increment` member function belonging to
`Counter`."

### Member order

Member functions can use data members declared later in the class. Still, order
the class for human readers:

1. public interface,
2. private implementation/data.

### Overloaded member functions

```cpp
class Printer
{
public:
    void print(int value);
    void print(std::string_view value);
};
```

Same name, different parameter lists.

### Structs can have member functions

C++ structs can have member functions too. The technical difference is default
access:

| Type keyword | Default member access |
|---|---|
| `struct` | public |
| `class` | private |

Use `struct` for passive data aggregates. Use `class` for objects with invariants
and behavior.

---

## 14.4 — Const class objects and const member functions

### Const objects cannot be modified

```cpp
const Counter c {};
// c.increment(); // should not be allowed: modifies object
```

C++ needs to know which member functions promise not to modify the object.

### Const member functions

Put `const` after the parameter list:

```cpp
class Counter
{
public:
    int value() const
    {
        return m_value;
    }

private:
    int m_value {};
};
```

This says `value()` will not modify the object.

Syntax:

```cpp
return_type function_name(parameters) const
```

### Const objects may only call const member functions

```cpp
const Counter c {};
std::cout << c.value(); // ok if value() is const
```

Non-const objects can call both const and non-const member functions.

### Const reference parameters

This matters when passing class objects by const reference:

```cpp
void printCounter(const Counter& c)
{
    std::cout << c.value() << '\n'; // value() must be const
}
```

If `value()` is not marked `const`, this fails even though it logically only
reads.

### Const/non-const overloads

Classes can provide both:

```cpp
class Buffer
{
public:
    std::string& text() { return m_text; }
    const std::string& text() const { return m_text; }

private:
    std::string m_text {};
};
```

The const version is called on const objects; the non-const version is called on
modifiable objects.

---

## 14.5 — Public and private members and access specifiers

### Access levels

Access control determines who can use a member.

| Access | Who can access |
|---|---|
| `public` | anyone |
| `private` | member functions and friends of the same class |
| `protected` | class and derived classes; covered later |

### Struct default vs class default

```cpp
struct S
{
    int x {}; // public by default
};

class C
{
    int x {}; // private by default
};
```

Most class definitions spell access explicitly:

```cpp
class Fuzzer
{
public:
    void run();

private:
    int m_count {};
};
```

### Private member naming

LearnCpp commonly uses an `m_` prefix for private data members:

```cpp
private:
    int m_count {};
    std::string m_name {};
```

This distinguishes data members from local variables and parameters.

### Per-class access

Objects of the same class can access each other's private members inside member
functions:

```cpp
class Counter
{
public:
    bool sameValue(const Counter& other) const
    {
        return m_value == other.m_value; // ok: same class
    }

private:
    int m_value {};
};
```

### Struct vs class practical rule

Use `struct` when:

- all members are public,
- it mostly groups data,
- no strong invariant needs protection.

Use `class` when:

- data should be private,
- behavior guards invariants,
- there is a meaningful public interface.

---

## 14.6 — Access functions

### Getters and setters

Access functions expose controlled access to private members.

```cpp
class Person
{
public:
    std::string_view name() const
    {
        return m_name;
    }

    void setName(std::string name)
    {
        m_name = name;
    }

private:
    std::string m_name {};
};
```

Nomenclature:

- getter/accessor: reads a value,
- setter/mutator: changes a value.

### Naming accessors

C++ often uses either:

```cpp
name()
getName()
```

Use the local codebase style.

### Return by value or const reference

For cheap types:

```cpp
int count() const { return m_count; }
```

For expensive owned members:

```cpp
const std::string& name() const { return m_name; }
```

But returning references exposes lifetime/encapsulation concerns. For strings,
`std::string_view` can be a useful read-only view if lifetime is clear.

### Why private data if getters exist?

Because getters/setters can:

- validate values,
- compute values,
- log/debug access,
- preserve invariants,
- allow implementation changes later.

Do not mindlessly create setters for every member. A setter is part of the
public interface and should preserve the object's validity.

---

## 14.7 — Member functions returning references to data members

### Returning by value can copy

```cpp
std::string name() const
{
    return m_name; // copies
}
```

For large members, copying can be expensive.

### Returning by const reference

```cpp
const std::string& name() const
{
    return m_name;
}
```

No copy, but the caller receives a reference into the object.

Lifetime diagram:

```
Person object: [---------------- alive ----------------]
returned ref:       [ valid only while Person alive ]
```

If the object dies, the reference dangles.

### Match the member type

If returning a reference to a member, the return type should match the member's
type closely:

```cpp
const std::string& name() const; // m_name is std::string
```

### Do not return non-const references to private data

Bad:

```cpp
std::string& name()
{
    return m_name; // caller can mutate private data freely
}
```

This breaks encapsulation:

```cpp
person.name().clear(); // bypasses validation
```

Return non-const references only when you intentionally want caller mutation and
are comfortable with invariant consequences.

### Rvalue implicit objects

Be careful calling reference-returning member functions on temporaries:

```cpp
const std::string& ref { Person{}.name() }; // dangerous if name returns member ref
```

The temporary object dies quickly; references to its members dangle.

---

## 14.8 — The benefits of data hiding (encapsulation)

### Interface vs implementation

The **interface** is what callers can use. The **implementation** is how the
class does it internally.

```cpp
class Timer
{
public:
    void start();
    double elapsedSeconds() const;

private:
    // implementation details hidden
};
```

Callers should depend on the interface, not the internals.

### Data hiding

Data hiding means keeping data private and exposing controlled operations.

Benefits:

- simpler public surface,
- fewer ways to misuse the object,
- invariants can be enforced,
- invalid values can be rejected early,
- internals can change without breaking callers,
- debugging has fewer mutation paths.

### Maintaining invariants

```cpp
class Fraction
{
public:
    Fraction(int numerator, int denominator)
        : m_numerator { numerator }
        , m_denominator { denominator == 0 ? 1 : denominator }
    {
    }

private:
    int m_numerator {};
    int m_denominator { 1 };
};
```

The class centralizes denominator validation.

### Prefer non-member functions when behavior does not need internals

If a function can be written using the public interface, consider making it a
non-member function.

```cpp
bool isEmpty(const Buffer& b)
{
    return b.size() == 0;
}
```

This keeps the class interface smaller.

### Member declaration order

A common class order:

```cpp
class Thing
{
public:
    // public types/functions

private:
    // private helper functions/data
};
```

Put the public interface first so users can see what matters.

---

## 14.9 — Introduction to constructors

### Constructor basics

A **constructor** is a special member function used to initialize objects.

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

Constructor name matches the class name and has no return type.

```cpp
Fraction f { 1, 2 };
```

### Constructors establish valid state

The constructor is the front door for object creation. Its job is to create a
valid object.

```
raw storage -> constructor -> valid object
```

### Constructors are not const

A constructor initializes the object. It cannot be `const` because the object is
not fully constructed yet.

### Constructors vs setters

A constructor creates a valid initial state. Setters modify an already-existing
object. If a value is required for validity, prefer requiring it in a
constructor.

---

## 14.10 — Constructor member initializer lists

### Member initializer list

Preferred constructor form:

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

The part after `:` initializes members before the constructor body runs.

### Initialization order

Members are initialized in the order they are declared in the class, not the
order listed in the member initializer list.

```cpp
class Example
{
private:
    int m_a {};
    int m_b {};

public:
    Example()
        : m_b { 2 }
        , m_a { m_b } // m_a actually initializes before m_b
    {
    }
};
```

Best practice: list initializers in the same order as member declarations.

### Initializer list vs assignment in body

Less ideal:

```cpp
Example(int x)
{
    m_x = x; // assignment after default initialization
}
```

Better:

```cpp
Example(int x)
    : m_x { x } // direct initialization
{
}
```

Some members, like const members and references, must be initialized in the
member initializer list.

### Constructor body

Use the body for validation or setup that cannot be done directly in the
initializer list.

```cpp
Fraction(int n, int d)
    : m_numerator { n }
    , m_denominator { d }
{
    if (m_denominator == 0)
        m_denominator = 1;
}
```

Better still: validate before storing when possible.

---

## 14.11 — Default constructors and default arguments

### Default constructor

A default constructor can be called with no arguments.

```cpp
class Counter
{
public:
    Counter() = default;

private:
    int m_value {};
};

Counter c {};
```

### Value initialization vs default initialization

Prefer braces:

```cpp
Counter c {}; // value initialization
```

This is clearer and safer than:

```cpp
Counter c; // default initialization
```

### Constructors with default arguments

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

Now all are valid:

```cpp
Fraction a {};
Fraction b { 3 };
Fraction c { 3, 4 };
```

### Implicit default constructor

If a class has no user-declared constructors, C++ may generate a default
constructor. If you define another constructor, the implicit default constructor
may no longer be available.

Use `= default` when you want to explicitly request the default behavior:

```cpp
Counter() = default;
```

Only create a default constructor when a default state actually makes sense.

---

## 14.12 — Delegating constructors

### Calling a constructor in the body creates a temporary

This does not initialize the current object:

```cpp
class Foo
{
public:
    Foo()
    {
        Foo(1); // creates a temporary Foo, then discards it
    }

    Foo(int x)
        : m_x { x }
    {
    }

private:
    int m_x {};
};
```

### Delegating constructor syntax

A constructor can delegate to another constructor in its member initializer list:

```cpp
class Foo
{
public:
    Foo()
        : Foo { 1 }
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

### Default arguments can reduce overloads

Sometimes this is simpler:

```cpp
Foo(int x = 1)
    : m_x { x }
{
}
```

Use delegating constructors when different construction paths share setup logic
that is clearer as another constructor.

---

## 14.13 — Temporary class objects

### Temporary objects

A temporary class object has no name and usually lives until the end of the full
expression.

```cpp
std::string { "hello" }; // temporary std::string
```

Temporaries often appear in function calls:

```cpp
printName(std::string { "Ada" });
```

### Return by value

Returning by value often involves temporary-looking objects:

```cpp
Fraction makeHalf()
{
    return Fraction { 1, 2 };
}
```

Modern C++ can often construct the result directly in the caller's storage.

### `static_cast` vs explicit temporary construction

Both can produce a converted value:

```cpp
auto x { static_cast<int>(3.5) };
auto y { int(3.5) };
```

Prefer `static_cast` when you want to emphasize conversion. Prefer normal
construction syntax when creating class objects.

### Lifetime warning

Do not return references/views to temporaries unless the lifetime rules make it
safe. This connects back to `std::string_view` dangling problems.

---

## 14.14 — Introduction to the copy constructor

### Copy constructor

A copy constructor initializes an object from another object of the same type.

```cpp
class Fraction
{
public:
    Fraction(const Fraction& other)
        : m_numerator { other.m_numerator }
        , m_denominator { other.m_denominator }
    {
    }

private:
    int m_numerator {};
    int m_denominator { 1 };
};
```

Parameter must be a reference, usually `const ClassName&`. If it were pass by
value, copying the parameter would require calling the copy constructor, which
would recurse forever.

```
copy ctor taking Fraction by value
  -> needs copy to pass argument
  -> calls copy ctor
  -> needs copy...
```

### Implicit copy constructor

C++ can generate a copy constructor that copies each member. Prefer the implicit
copy constructor unless you need custom copy behavior.

```cpp
Fraction a { 1, 2 };
Fraction b { a }; // copy
```

### `= default` and `= delete`

Request default copy:

```cpp
Fraction(const Fraction&) = default;
```

Prevent copying:

```cpp
Fraction(const Fraction&) = delete;
```

Deleting copy is useful for types that should not be copied, such as unique
resource owners.

---

## 14.15 — Class initialization and copy elision

### Unnecessary copies

Older mental model:

```cpp
Fraction make()
{
    return Fraction { 1, 2 };
}

Fraction f { make() };
```

You might imagine:

```
construct temporary -> copy to return value -> copy to f
```

Modern C++ often avoids those copies.

### Copy elision

**Copy elision** means the compiler omits unnecessary copy construction and
constructs the object directly where it needs to live.

```
instead of:
  temp -> copy -> f

compiler does:
  construct f directly
```

C++17 makes some copy elision mandatory.

### Why it matters

Returning objects by value is often clean and efficient:

```cpp
CoveragePoint makePoint(int line, int column)
{
    return CoveragePoint { line, column };
}
```

Do not contort simple code into out parameters just to avoid imagined copies.

---

## 14.16 — Converting constructors and the explicit keyword

### Converting constructors

A constructor callable with a single argument can be used for implicit
conversion.

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

process(5); // int implicitly converted to Token
```

This can be convenient, but also surprising.

### `explicit`

Use `explicit` to prevent implicit construction:

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

// process(5);        // error
process(Token { 5 }); // clear
```

Best practice: make single-argument constructors `explicit` unless you really
want implicit conversion.

### Only one user-defined conversion

C++ limits implicit conversion chains involving user-defined conversions. This
prevents some wild conversion paths, but you should still avoid designing APIs
that rely on surprise conversions.

---

## 14.17 — Constexpr aggregates and classes

### Constexpr member functions

A member function can be `constexpr` if it can be evaluated at compile time when
called in a constant-expression context.

```cpp
class Point
{
public:
    constexpr Point(int x, int y)
        : m_x { x }
        , m_y { y }
    {
    }

    constexpr int x() const { return m_x; }

private:
    int m_x {};
    int m_y {};
};

constexpr Point p { 1, 2 };
static_assert(p.x() == 1);
```

### Constexpr objects

For a class object to be `constexpr`, construction and the relevant operations
must be valid at compile time.

This is useful for small value-like types and compile-time configuration.

### Constexpr does not always mean const

A `constexpr` member function may be const or non-const depending on the rules
and standard version. For learning purposes, most accessor-style constexpr
member functions should also be `const`:

```cpp
constexpr int value() const;
```

---

## 14.x — Chapter 14 summary and quiz

### Core takeaways

- OOP groups data and behavior into objects.
- In C++, even fundamental variables are objects, but class-type objects add
  member data/functions.
- Classes help preserve invariants.
- Member functions operate on an implicit object.
- Mark read-only member functions `const`.
- `struct` defaults public; `class` defaults private.
- Use private data and public interfaces when invariants matter.
- Access functions can expose controlled reads/writes, but do not generate
  getters/setters mindlessly.
- Returning references to data members avoids copies but creates lifetime and
  encapsulation risks.
- Constructors initialize objects and should establish valid state.
- Prefer member initializer lists over assignment in constructor bodies.
- Members initialize in declaration order.
- Use `= default` to request generated default behavior.
- Delegating constructors call another constructor from the initializer list.
- Temporary objects have short lifetimes.
- Prefer implicit copy constructors unless custom copy behavior is required.
- Copy elision makes return-by-value practical.
- Use `explicit` on single-argument constructors by default.
- `constexpr` classes/functions can participate in compile-time evaluation.

### Java-to-C++ bridge

| Java intuition | C++ correction |
|---|---|
| Class variables are references to objects | Class objects can live directly as values |
| Constructor initializes object | Same idea, but member initializer list matters |
| Methods can be called on final refs | C++ has `const` objects and `const` member functions |
| Objects copied by reference assignment | C++ class objects can copy by value |
| Garbage collector handles lifetime | C++ object lifetime is deterministic and scope-based |

### LLVM decoder

```cpp
DebugLoc Loc = I.getDebugLoc();
```

`getDebugLoc()` is a member function call on `I`.

```cpp
auto *M = F.getParent();
```

`getParent()` is a member function that returns a pointer.

```cpp
M->getOrInsertFunction(...);
```

`->` calls a member function through a pointer.

### Mini drill

```cpp
#include <iostream>
#include <string>
#include <string_view>

class CampaignStats
{
public:
    explicit CampaignStats(std::string name)
        : m_name { name }
    {
    }

    void recordTest(bool crashed)
    {
        ++m_tests;
        if (crashed)
            ++m_crashes;
    }

    std::string_view name() const
    {
        return m_name;
    }

    int tests() const
    {
        return m_tests;
    }

    int crashes() const
    {
        return m_crashes;
    }

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

What this reinforces:

- class with private members,
- constructor member initializer list,
- `explicit` constructor,
- mutating member function,
- const accessor member functions,
- `std::string` for owned name,
- `std::string_view` for read-only accessor.
