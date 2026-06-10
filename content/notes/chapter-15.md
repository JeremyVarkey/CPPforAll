# Chapter 15 — More on Classes

> Source: <https://www.learncpp.com/> (Chapter 15)
> One file per chapter. Each lesson is a section below.

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

### The hidden `this` pointer

When you call a member function:

```cpp
stats.recordTest(true);
```

C++ supplies a hidden pointer to the object being operated on. Inside the member
function, that pointer is named `this`.

```cpp
class Counter
{
public:
    void increment()
    {
        ++m_value;       // shorthand
        // ++this->m_value; // explicit form
    }

private:
    int m_value {};
};
```

Mental model:

```
counter.increment()

roughly:
Counter::increment(&counter)
                    ^
                    this
```

`this` always points to the object whose member function is currently running.

### Explicit `this`

Usually omit `this->` unless it clarifies a name conflict or is required in
template contexts.

```cpp
class Person
{
public:
    void setName(std::string name)
    {
        this->m_name = name; // clarifies member vs parameter
    }

private:
    std::string m_name {};
};
```

### Returning `*this`

`this` is a pointer. `*this` is the object itself.

Returning `*this` by reference enables chaining:

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

Counter c {};
c.increment().increment().reset();
```

Flow:

```
c.increment() returns c
  .increment() called on c again
    .reset() called on c
```

### `this` and const objects

Inside a const member function, `this` points to const:

```cpp
int value() const
{
    return m_value;
}
```

Conceptually:

```cpp
const Counter* this
```

That is why const member functions cannot modify data members unless those
members are specially marked `mutable` (not a tool to reach for casually).

### Why `this` is a pointer

Historically and mechanically, `this` is pointer-like. You use `this->member`
instead of `this.member`.

---

## 15.2 — Classes and header files

### Class definitions usually live in headers

If multiple files need to use a class type, put the class definition in a header:

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

Member function definitions can go in a `.cpp`:

```cpp
// CampaignStats.cpp
#include "CampaignStats.h"

CampaignStats::CampaignStats(std::string name)
    : m_name { name }
{
}
```

### Naming

Common style:

```
CampaignStats.h
CampaignStats.cpp
```

The header contains declarations/type definition. The `.cpp` contains
out-of-class member definitions.

### Does a class definition in a header violate ODR?

No, as long as the same class definition appears identically in each translation
unit. Include guards prevent multiple inclusion within one translation unit.

### Inline member functions

Member functions defined inside the class definition are implicitly inline:

```cpp
class Counter
{
public:
    int value() const { return m_value; }

private:
    int m_value {};
};
```

"Inline" here primarily affects linkage/ODR rules. It does not guarantee the
compiler will perform inline expansion.

### Why not put everything in headers?

Headers with lots of implementation:

- increase compile times,
- expose implementation details,
- make changes ripple through more files,
- can create dependency tangles.

Keep tiny/simple functions in headers when useful. Put heavier implementation in
`.cpp` files unless templates require header definitions.

### Default arguments

Default arguments for member functions should appear in the declaration, not
duplicated in the definition:

```cpp
// header
void resize(int width, int height = 100);

// cpp
void Window::resize(int width, int height)
{
}
```

---

## 15.3 — Nested types (member types)

### Nested types

A class can define types inside itself:

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

Use:

```cpp
Fuzzer::Campaign campaign { Fuzzer::Campaign::MutationA };
```

Nested types are useful when the type belongs strongly to the class.

### Nested aliases

```cpp
class CoverageTracker
{
public:
    using Point = std::pair<int, int>;
    using PointSet = std::set<Point>;
};
```

This puts helper type names near the abstraction that owns them.

### Access control applies

Nested types can be public or private:

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
};
```

If callers do not need the type, keep it private.

### Nested classes and outer members

A nested class is a separate class. It does not automatically get an implicit
object of the outer class. Do not assume Java-like inner-class behavior.

---

## 15.4 — Introduction to destructors

### The cleanup problem

Some objects acquire resources:

- memory,
- file handles,
- sockets,
- locks,
- temporary files,
- database connections.

Those resources need cleanup.

### Destructor

A **destructor** is a special member function called when an object is destroyed.

```cpp
class Trace
{
public:
    ~Trace()
    {
        std::cout << "destroyed\n";
    }
};
```

Name:

```cpp
~ClassName()
```

No return type. No parameters.

### Scope-based destruction

```cpp
{
    Trace t {};
} // t's destructor runs here
```

Diagram:

```
enter scope -> construct t -> use t -> leave scope -> destructor runs
```

This deterministic cleanup is a huge C++ concept. It underpins RAII: Resource
Acquisition Is Initialization.

### Implicit destructor

If you do not define a destructor, C++ generates one. For simple classes with
standard library members, the implicit destructor is usually enough because
members clean themselves up.

```cpp
class SeedCorpus
{
private:
    std::vector<std::string> m_seeds {};
}; // implicit destructor destroys vector, which destroys strings
```

### Warning about `std::exit`

`std::exit()` can skip destruction of local automatic objects. That means RAII
cleanup may not run. Prefer normal returns/exceptions unless process halt is
intentional.

---

## 15.5 — Class templates with member functions

### Template class with member function inside

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

Use:

```cpp
Box<int> i { 5 };
Box<std::string> s { "hello" };
```

### Defining member functions outside the template class

You need the template declaration again:

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

The repeated `template <typename T>` is not decoration; it tells the compiler
these are template member definitions.

### Injected class name

Inside `Box<T>`, you can often write `Box` instead of `Box<T>`. LearnCpp calls
this an injected class name. Outside, be explicit unless the context clearly
allows deduction.

### Templates usually live in headers

Template definitions must generally be visible where instantiated. That is why
template class definitions and member definitions often live in headers.

---

## 15.6 — Static member variables

### Static data member

A static member variable belongs to the class itself, not to each object.

```cpp
class Counter
{
public:
    Counter()
    {
        ++s_created;
    }

    static int s_created;
};

int Counter::s_created { 0 };
```

Object members:

```
Counter a: m_value
Counter b: m_value

Class Counter: s_created shared by all
```

### Access

```cpp
std::cout << Counter::s_created << '\n';
```

Use the class name for static members. Accessing through an object may compile
but is less clear.

### Initialization

Non-inline static data members usually need a definition outside the class.

Modern C++ also supports inline static members:

```cpp
class Counter
{
public:
    inline static int s_created { 0 };
};
```

### Static constants

Static constexpr members are common for class-level constants:

```cpp
class Config
{
public:
    static constexpr int maxTests { 10000 };
};
```

---

## 15.7 — Static member functions

### Static member function

A static member function belongs to the class, not to an object.

```cpp
class Random
{
public:
    static int get(int min, int max);
};

int value { Random::get(1, 6) };
```

### No `this`

Static member functions have no `this` pointer because there is no implicit
object.

They can access:

- static members,
- passed-in objects,
- globals.

They cannot directly access non-static members:

```cpp
class Counter
{
public:
    static int created()
    {
        return s_created; // ok
        // return m_value; // error: no object
    }

private:
    int m_value {};
    inline static int s_created {};
};
```

### Pure static class vs namespace

If a class has only static members and no object state, a namespace may be more
appropriate:

```cpp
namespace Random
{
    int get(int min, int max);
}
```

C++ does not support static constructors like some languages. Static
initialization has its own rules and pitfalls.

---

## 15.8 — Friend non-member functions

### Friendship

A friend function can access private/protected members of a class even though it
is not a member.

```cpp
class Point
{
public:
    Point(int x, int y)
        : m_x { x }
        , m_y { y }
    {
    }

    friend void print(const Point& p);

private:
    int m_x {};
    int m_y {};
};

void print(const Point& p)
{
    std::cout << p.m_x << ',' << p.m_y << '\n';
}
```

Friendship is granted by the class. It is not requested from outside.

### Friend for operators

Friend non-member functions are common for operators:

```cpp
class Point
{
public:
    friend std::ostream& operator<<(std::ostream& out, const Point& p)
    {
        return out << p.m_x << ',' << p.m_y;
    }

private:
    int m_x {};
    int m_y {};
};
```

### Does friendship break encapsulation?

It can if abused. But a small friend operator or helper can be part of the class
interface. The class explicitly chooses to expose private access to that
function.

Best practice:

- prefer non-friend functions when public interface is enough,
- use friend when the operation is naturally non-member but needs internals,
- keep the friend set small.

---

## 15.9 — Friend classes and friend member functions

### Friend class

A class can grant another class access to its private members:

```cpp
class Storage
{
    friend class StorageInspector;

private:
    int m_secret {};
};
```

`StorageInspector` can access `Storage` internals.

Use sparingly. Friend classes create tight coupling.

### Friend member function

You can friend a single member function instead of an entire class.

Conceptual form:

```cpp
class A;

class Inspector
{
public:
    void inspect(const A& a);
};

class A
{
    friend void Inspector::inspect(const A& a);

private:
    int m_value {};
};
```

This is more precise but requires careful ordering/forward declarations.

Rule of thumb: if friend declarations start making the code contorted, reconsider
the design.

---

## 15.10 — Ref qualifiers

### The problem

Member functions can be called on lvalue objects and rvalue temporary objects:

```cpp
std::string s {};
s.size();                  // lvalue object
std::string { "hi" }.size(); // rvalue temporary
```

Sometimes returning references from temporary objects is dangerous.

### Ref-qualified member functions

A member function can be qualified to only run on lvalue or rvalue objects:

```cpp
class Buffer
{
public:
    const std::string& data() const& // only lvalue objects
    {
        return m_data;
    }

    std::string data() && // rvalue objects
    {
        return std::move(m_data);
    }

private:
    std::string m_data {};
};
```

Syntax:

| Qualifier | Callable on |
|---|---|
| `&` | lvalue implicit object |
| `&&` | rvalue implicit object |

### Why not use everywhere?

Ref qualifiers are powerful but advanced. They add complexity and are usually
unnecessary for beginner code. Recognize them when reading library code; reach
for them only when you are managing value categories deliberately.

Connection to Chapter 14: ref qualifiers help prevent bad patterns like returning
a reference to data inside a temporary object.

---

## 15.x — Chapter 15 summary and quiz

### Core takeaways

- Member functions receive a hidden `this` pointer.
- `this->member` explicitly accesses the current object's member.
- Returning `*this` by reference enables member function chaining.
- Class definitions used across files belong in headers.
- Member functions defined inside class definitions are implicitly inline.
- Heavy implementations usually belong in `.cpp` files unless templates require
  header definitions.
- Nested types scope helper types inside a class.
- Destructors run when objects are destroyed and support deterministic cleanup.
- Template member functions require template syntax on out-of-class definitions.
- Static data members belong to the class, not individual objects.
- Static member functions have no `this`.
- A namespace may be better than a pure-static class.
- Friend functions/classes can access private members but should be rare and
  intentional.
- Ref qualifiers control whether member functions can be called on lvalue or
  rvalue objects.

### Class mechanics map

```cpp
class Example
{
public:
    Example();              // constructor
    ~Example();             // destructor

    Example& reset();       // can return *this
    int value() const;      // const member function

    static int created();   // no this pointer

private:
    int m_value {};
    inline static int s_created {};
};
```

### CS6340 relevance

LLVM and standard library code use:

- member calls: `I.getDebugLoc()`,
- pointer member calls: `M->getOrInsertFunction(...)`,
- nested types and aliases,
- static helpers/constants,
- class templates (`std::vector<T>`, `std::optional<T>`),
- destructors/RAII inside library objects,
- friend/operator patterns in streamable types.

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
        ++s_created;
    }

    ~CampaignStats()
    {
        --s_created;
    }

    CampaignStats& record(bool crashed)
    {
        ++m_tests;
        if (crashed)
            ++m_crashes;

        return *this;
    }

    std::string_view name() const
    {
        return m_name;
    }

    static int created()
    {
        return s_created;
    }

    friend std::ostream& operator<<(std::ostream& out, const CampaignStats& stats)
    {
        return out << stats.m_name << ": "
                   << stats.m_crashes << '/'
                   << stats.m_tests;
    }

private:
    std::string m_name {};
    int m_tests {};
    int m_crashes {};
    inline static int s_created {};
};

int main()
{
    CampaignStats stats { "MutationA" };
    stats.record(false).record(true);

    std::cout << stats << '\n';
    std::cout << "live stats objects: " << CampaignStats::created() << '\n';
}
```

What this reinforces:

- constructor/destructor pair,
- `inline static` class-level counter,
- `this`/`*this` chaining,
- const accessor,
- friend stream operator,
- static member function called through class name.
