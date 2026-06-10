# Chapter 26 — Templates and Classes

> Source: <https://www.learncpp.com/> (Chapter 26)
>
> Goal for CS6340: read class templates, template arguments, specializations,
> and pointer-heavy generic code without treating the angle brackets as noise.
> This is especially useful around STL containers and LLVM helper types.

## Contents

- [26.1 — Template classes](#261--template-classes)
- [26.2 — Template non-type parameters](#262--template-non-type-parameters)
- [26.3 — Function template specialization](#263--function-template-specialization)
- [26.4 — Class template specialization](#264--class-template-specialization)
- [26.5 — Partial template specialization](#265--partial-template-specialization)
- [26.6 — Partial template specialization for pointers](#266--partial-template-specialization-for-pointers)
- [26.x — Chapter 26 summary and quiz](#26x--chapter-26-summary-and-quiz)
- [CS6340 patterns](#cs6340-patterns)

## 26.1 — Template classes

### Why class templates exist

Function templates solve this kind of duplication:

```cpp
int max(int a, int b);
double max(double a, double b);
```

Class templates solve the same problem for types that store or organize data.
Without a class template, a "box of int" and a "box of std::string" often become
two almost identical classes.

```cpp
class IntBox
{
private:
    int m_value {};
};

class StringBox
{
private:
    std::string m_value {};
};
```

Only the contained type changes. A class template turns that type into a
placeholder:

```cpp
#include <iostream>
#include <string>

template <typename T>
class Box
{
private:
    T m_value {};

public:
    explicit Box(const T& value)
        : m_value { value }
    {
    }

    const T& get() const;
    void set(const T& value) { m_value = value; }
};

template <typename T>
const T& Box<T>::get() const
{
    return m_value;
}

int main()
{
    Box<int> count { 3 };
    Box<std::string> name { "seed-001" };

    std::cout << count.get() << '\n';
    std::cout << name.get() << '\n';
}
```

Read:

```text
template <typename T>
class Box
```

as:

```text
"Box is a pattern for making classes. T is the placeholder type."
```

Then:

```cpp
Box<int>
Box<std::string>
```

mean:

```text
Box<int>         -> Box with T replaced by int
Box<std::string> -> Box with T replaced by std::string
```

### A template is a stencil, not the final class

A class template is not itself a concrete class. It is a recipe the compiler can
instantiate when a concrete type is requested.

```text
source template:

template <typename T>
class Box { T m_value; };

requested uses:

Box<int>          Box<std::string>
   |                    |
   v                    v
compiler generates concrete class code for those requested types
```

If the program never uses `Box<double>`, the compiler does not need to generate
that version.

This is why template errors can look noisy. The compiler may report an error in
generated code such as `Box<std::vector<int>>::get()`, not just in the template
definition.

### Member functions outside the class

For a normal non-template class:

```cpp
class Counter
{
public:
    int value() const;
};

int Counter::value() const
{
    return 0;
}
```

For a class template, the out-of-class member definition needs the template
declaration too:

```cpp
template <typename T>
class Box
{
public:
    const T& get() const;
};

template <typename T>
const T& Box<T>::get() const
{
    return m_value;
}
```

The two important pieces are:

```cpp
template <typename T> // repeats the template parameter list
Box<T>::get           // names the class template instantiation pattern
```

Inside the class body, `Box` can usually refer to the current instantiation.
Outside the class body, use `Box<T>`.

### Template definitions usually live in headers

With ordinary classes, a common split is:

```text
Counter.h    -> class declaration
Counter.cpp  -> member function definitions
```

Class templates do not split this way by default. When a translation unit asks
for `Box<int>`, the compiler needs to see the full template definition right
there so it can instantiate the concrete `Box<int>` code.

If `main.cpp` sees only this:

```cpp
template <typename T>
class Box
{
public:
    const T& get() const;
};
```

but the definition of `Box<T>::get()` lives only in `Box.cpp`, `main.cpp` may
compile, but linking can fail because no concrete `Box<int>::get()` was
generated.

Practical rule:

```text
For class templates, put the class definition and member definitions in the
header, or put member definitions in an included .inl/.ipp file at the bottom
of the header.
```

Possible layouts:

```text
Simple:

Box.h
  template class definition
  template member definitions

Organized:

Box.h
  template class definition
  #include "Box.inl"

Box.inl
  template member definitions
```

Do not compile the `.inl` file as an independent source file. It is meant to be
included by the header.

### Explicit instantiation exists, but is more maintenance

There is a more advanced route where a `.cpp` file explicitly requests the
template instantiations the program will need:

```cpp
template class Box<int>;
template class Box<double>;
```

That can reduce repeated template instantiation work, but now someone must keep
that list current. For these course notes, prefer the header-only or
header-plus-included-implementation layout unless a project already uses
explicit instantiation.

### Containers are the natural class-template use case

Standard containers are class templates:

```cpp
std::vector<int>
std::vector<std::string>
std::array<double, 4>
```

The container behavior is mostly the same. The stored type changes.

CS6340-style code often has shapes like:

```cpp
std::vector<Instruction*> worklist;
std::vector<std::string> seeds;
std::unordered_map<std::string, int> hitCounts;
```

Read the template arguments as part of the type:

| Type | Meaning |
| --- | --- |
| `std::vector<Instruction*>` | dynamic array of pointers to `Instruction` |
| `std::vector<std::string>` | dynamic array of strings |
| `std::unordered_map<std::string, int>` | hash table from string keys to int values |

The outer name tells you the data structure. The angle brackets tell you what
kind of data it stores.

## 26.2 — Template non-type parameters

### Type parameters vs non-type parameters

So far, template parameters have mostly stood for types:

```cpp
template <typename T>
class Box;
```

`T` is replaced by a type such as `int` or `std::string`.

A template can also have a parameter that stands for a compile-time value:

```cpp
template <typename T, std::size_t Capacity>
class FixedBuffer
{
private:
    T m_items[Capacity] {};
    std::size_t m_size {};

public:
    bool push(const T& value)
    {
        if (m_size == Capacity)
            return false;

        m_items[m_size++] = value;
        return true;
    }

    std::size_t capacity() const { return Capacity; }
    std::size_t size() const { return m_size; }
};
```

Here:

```cpp
T             // template type parameter
Capacity      // template non-type parameter
```

Use:

```cpp
FixedBuffer<int, 8> smallInts;
FixedBuffer<int, 32> moreInts;
```

`8` and `32` are compile-time values, not runtime constructor arguments.

### Non-type template arguments must be constant expressions

This works:

```cpp
constexpr std::size_t seedLimit { 16 };

FixedBuffer<std::string, seedLimit> seeds;
```

This does not:

```cpp
std::size_t readLimitFromUser();

std::size_t n { readLimitFromUser() };
FixedBuffer<std::string, n> seeds; // error: n is not a constant expression
```

The compiler must know the non-type template argument while compiling because it
uses the value to generate a concrete type.

### The value is part of the type

These are different types:

```cpp
FixedBuffer<int, 4>
FixedBuffer<int, 8>
```

Even though both store `int`, their capacities differ at compile time.

```text
FixedBuffer<int, 4>  !=  FixedBuffer<int, 8>
        type includes the value argument
```

That is the same idea behind:

```cpp
std::array<int, 5>
std::array<int, 10>
```

`std::array` stores its length in the type, so the object can contain a fixed
array directly instead of allocating dynamically.

### Out-of-class definitions include both parameters

If a member function is defined outside the class:

```cpp
template <typename T, std::size_t Capacity>
class FixedBuffer
{
public:
    const T& at(std::size_t index) const;
};

template <typename T, std::size_t Capacity>
const T& FixedBuffer<T, Capacity>::at(std::size_t index) const
{
    return m_items[index];
}
```

The class name includes both template arguments:

```cpp
FixedBuffer<T, Capacity>
```

### What can be a non-type template parameter?

Common examples:

- Integral values such as `int`, `bool`, `std::size_t`
- Enumeration values
- Pointers and references to objects or functions with suitable linkage
- `std::nullptr_t`
- Floating-point values in C++20 and later

In everyday C++ study code, the most common case is a fixed size:

```cpp
template <typename T, std::size_t N>
class StaticVectorLikeThing;
```

### CS6340 reading pattern

If you see:

```cpp
SmallVector<Instruction*, 8> worklist;
```

read:

```text
SmallVector
  stores: Instruction*
  inline capacity / compile-time tuning value: 8
```

You do not need to know every implementation detail immediately. The type is
telling you both "what kind of thing" and often "how much fixed storage or
compile-time policy."

## 26.3 — Function template specialization

### The default template may not be right for every type

Suppose we have a generic printer:

```cpp
#include <iostream>

template <typename T>
void printValue(const T& value)
{
    std::cout << value << '\n';
}
```

This works for many types, but maybe booleans should print as words:

```text
true
false
```

instead of:

```text
1
0
```

### Prefer a non-template overload when it fits

The simplest solution is usually a normal overload:

```cpp
void printValue(bool value)
{
    std::cout << (value ? "true" : "false") << '\n';
}
```

Now:

```cpp
printValue(42);    // uses function template
printValue(true);  // uses non-template overload
```

When a non-template function is an equally good match, it wins over a function
template. Prefer this when you can because the signature can be whatever is most
natural.

### Full function template specialization

Function template specialization is another tool. A full specialization gives a
specific implementation for a specific set of template arguments.

```cpp
#include <iostream>
#include <string_view>

template <typename T>
void emit(const T& value)
{
    std::cout << value << '\n';
}

template <>
void emit<std::string_view>(const std::string_view& value)
{
    std::cout << '"' << value << '"' << '\n';
}
```

Parts:

```cpp
template <>                    // no remaining template parameters
void emit<std::string_view>(...) // specialization for T = std::string_view
```

The primary template must be declared before the specialization.

### Specializations must match the primary template signature

If the primary template is:

```cpp
template <typename T>
void emit(const T& value);
```

then the specialization for `double` must be:

```cpp
template <>
void emit<double>(const double& value);
```

It cannot silently switch to:

```cpp
void emit<double>(double value); // wrong for this specialization
```

because `const T&` becomes `const double&`.

If you want a different signature, use a non-template overload instead.

### Header warning: full specializations are not implicitly inline

Full function specializations behave like ordinary functions for the one
definition rule.

If this is in a header included by multiple `.cpp` files:

```cpp
template <>
void emit<double>(const double& value)
{
    std::cout << std::scientific << value << '\n';
}
```

you can get duplicate definition linker errors. Mark it `inline` if it must
live in a header:

```cpp
template <>
inline void emit<double>(const double& value)
{
    std::cout << std::scientific << value << '\n';
}
```

### You can delete a specialization

Sometimes a type should be rejected:

```cpp
template <typename T>
void sendMetric(const T& value)
{
    std::cout << value << '\n';
}

template <>
void sendMetric<const char*>(const char* const&) = delete;
```

That makes a call resolving to that specialization fail at compile time.

In modern code, a clearer constraint or `static_assert` is often better, but
deleted specializations are useful to recognize.

### Member functions of class templates are a different case

If you have:

```cpp
template <typename T>
class Cell
{
public:
    void print() const;
};
```

and want `Cell<double>::print()` to behave differently, that is not a standalone
function-template specialization in the usual sense. You are specializing a
member of a class template. That is covered in the next lesson.

## 26.4 — Class template specialization

### Full class specialization

A class template can have a completely different definition for a particular
type:

```cpp
#include <iostream>
#include <string>

template <typename T>
class Display
{
private:
    T m_value {};

public:
    explicit Display(const T& value)
        : m_value { value }
    {
    }

    void print() const
    {
        std::cout << m_value << '\n';
    }
};

template <>
class Display<bool>
{
private:
    bool m_value {};

public:
    explicit Display(bool value)
        : m_value { value }
    {
    }

    void print() const
    {
        std::cout << (m_value ? "yes" : "no") << '\n';
    }
};
```

Now:

```cpp
Display<int> count { 5 };      // uses primary template
Display<bool> enabled { true }; // uses full specialization
```

Syntax:

```cpp
template <>
class Display<bool>
```

means:

```text
"This is the complete Display class for T = bool."
```

### The specialization is an independent class

`Display<bool>` does not have to use the same private data layout as
`Display<T>`. It could store bits, cache strings, expose extra member functions,
or remove member functions.

That freedom is powerful but dangerous. If users expect all `Display<T>` types
to support the same interface, keep the public interface consistent unless there
is a strong reason not to.

```text
Primary template:
  Display<int>
  Display<double>
  Display<std::string>

Full specialization:
  Display<bool>   // separate class definition selected for bool
```

### The primary template comes first

The compiler must know the primary template before it can understand a
specialization:

```cpp
template <typename T>
class Display;

template <>
class Display<bool>;
```

In practice, define the full primary template first, then define the
specialization near it.

Do not rely on "include this extra header if you want the specialized behavior."
That can make behavior depend on accidental include order:

```text
translation unit A sees Display<bool> specialization -> specialized behavior
translation unit B does not see it                 -> primary template behavior
```

That is a debugging problem waiting to happen.

### Specializing one member function

Sometimes the whole class does not need a new definition. Only one member
function needs special behavior.

```cpp
#include <iostream>

template <typename T>
class Metric
{
private:
    T m_value {};

public:
    explicit Metric(T value)
        : m_value { value }
    {
    }

    void print() const
    {
        std::cout << m_value << '\n';
    }
};

template <>
inline void Metric<double>::print() const
{
    std::cout << std::scientific << m_value << '\n';
}
```

Here `Metric<double>` itself is still instantiated from the primary class
template. Only `Metric<double>::print()` gets a custom definition.

If this specialized member function is defined in a header, mark it `inline`
for the same reason as full function specializations.

### Use class specialization sparingly

Class specialization is best when a type truly needs different implementation
mechanics:

- Packing `bool` values into bits
- Treating `char` buffers as text instead of numeric elements
- Handling pointer types differently from value types
- Adapting a template to a type with unusual constraints

Do not specialize just to avoid a small `if constexpr` or ordinary overload if
those would be clearer.

## 26.5 — Partial template specialization

### Full specialization replaces every template parameter

Given:

```cpp
template <typename T, std::size_t N>
class StaticList;
```

a full specialization might be:

```cpp
template <>
class StaticList<char, 32>;
```

Both `T` and `N` are fixed.

That is too narrow if we want "all `char` static lists, regardless of size."

### Partial specialization fixes only part of the pattern

Partial specialization lets a class template specialize some template arguments
while leaving others as parameters:

```cpp
#include <cstddef>
#include <iostream>

template <typename T, std::size_t N>
class StaticList
{
private:
    T m_items[N] {};

public:
    T& operator[](std::size_t index) { return m_items[index]; }
    const T& operator[](std::size_t index) const { return m_items[index]; }

    void print() const
    {
        for (const auto& item : m_items)
            std::cout << item << ' ';

        std::cout << '\n';
    }
};

template <std::size_t N>
class StaticList<char, N>
{
private:
    char m_items[N] {};

public:
    char& operator[](std::size_t index) { return m_items[index]; }
    const char& operator[](std::size_t index) const { return m_items[index]; }

    void print() const
    {
        for (char ch : m_items)
            std::cout << ch;

        std::cout << '\n';
    }
};
```

Read:

```cpp
template <std::size_t N>
class StaticList<char, N>
```

as:

```text
"This is the StaticList specialization where T is char, but N is still a
template parameter."
```

Examples:

```cpp
StaticList<int, 4>    a; // primary template
StaticList<char, 4>   b; // partial specialization
StaticList<char, 128> c; // same partial specialization pattern
```

### Function templates cannot be partially specialized

This is not allowed:

```cpp
template <typename T, std::size_t N>
void print(const StaticList<T, N>& list);

template <std::size_t N>
void print<char, N>(const StaticList<char, N>& list); // not valid syntax
```

Functions can be overloaded, and function templates can be fully specialized,
but function templates cannot be partially specialized.

This works because it is just an overload:

```cpp
template <typename T, std::size_t N>
void print(const StaticList<T, N>& list)
{
    list.print();
}

template <std::size_t N>
void print(const StaticList<char, N>& list)
{
    list.print();
}
```

The function is not partially specialized. It is a function template overload
whose parameter type mentions a partially specialized class pattern.

### Member functions have the same limitation

This attempt does not work:

```cpp
template <std::size_t N>
void StaticList<double, N>::print() const; // trying to partially specialize a function
```

The fix is to partially specialize the class, then define the member function of
that specialized class:

```cpp
template <std::size_t N>
class StaticList<double, N>
{
public:
    void print() const;
};

template <std::size_t N>
void StaticList<double, N>::print() const
{
    // ordinary member of a partially specialized class
}
```

That can cause duplicated code. One common way to reduce duplication is a shared
base template:

```cpp
template <typename T, std::size_t N>
class StaticListBase
{
protected:
    T m_items[N] {};

public:
    T& operator[](std::size_t index) { return m_items[index]; }
    const T& operator[](std::size_t index) const { return m_items[index]; }
};

template <typename T, std::size_t N>
class StaticList : public StaticListBase<T, N>
{
public:
    void print() const
    {
        for (const auto& item : this->m_items)
            std::cout << item << ' ';
        std::cout << '\n';
    }
};

template <std::size_t N>
class StaticList<double, N> : public StaticListBase<double, N>
{
public:
    void print() const
    {
        for (double item : this->m_items)
            std::cout << std::scientific << item << ' ';
        std::cout << '\n';
    }
};
```

The `this->` before `m_items` is not cosmetic. In templates, names inherited
from dependent base classes often need qualification so lookup happens at the
right time.

### When partial specialization is worth it

Partial specialization is useful when there is a family-level distinction:

```text
all T* types
all char buffers of any length
all Pair<std::string, T> types
all StaticList<T, 0> types
```

If the distinction is only one exact type, full specialization may be enough. If
the distinction is only one function call, overloads or `if constexpr` may be
clearer.

## 26.6 — Partial template specialization for pointers

### Pointer types often need different behavior

Consider a simple value holder:

```cpp
#include <iostream>

template <typename T>
class Storage
{
private:
    T m_value {};

public:
    explicit Storage(T value)
        : m_value { value }
    {
    }

    void print() const
    {
        std::cout << m_value << '\n';
    }
};
```

This is fine for:

```cpp
Storage<int> count { 5 };
Storage<double> ratio { 0.75 };
```

But:

```cpp
int x { 5 };
Storage<int*> pointerStorage { &x };
pointerStorage.print();
```

prints the pointer value, not `5`, because `m_value` is an `int*`.

### Partial specialization for all pointer types

Instead of specializing separately for `int*`, `double*`, `Widget*`, and every
other pointer type, specialize the class for the `T*` pattern:

```cpp
template <typename T>
class Storage<T*>
{
private:
    T* m_value {};

public:
    explicit Storage(T* value)
        : m_value { value }
    {
    }

    void print() const
    {
        if (m_value)
            std::cout << *m_value << '\n';
        else
            std::cout << "<null>\n";
    }
};
```

Read:

```cpp
template <typename T>
class Storage<T*>
```

as:

```text
"For any pointer type, T is the pointed-to type. Use this specialized Storage."
```

So:

```cpp
Storage<int*>     -> T is int
Storage<double*>  -> T is double
Storage<Node*>    -> T is Node
```

The primary template must still be defined first.

### `T` is the pointee type, not the pointer type

Inside `Storage<T*>`, `T` is not `int*`; it is `int`.

That is why the member is written:

```cpp
T* m_value {};
```

not:

```cpp
T m_value {}; // would store the pointee object, not the pointer
```

### Ownership and lifetime are the real issue

The specialized pointer version above changed the semantics:

```text
Storage<int>   copies/stores an int value
Storage<int*>  stores a pointer to someone else's int
```

That means `Storage<int*>` can dangle:

```cpp
Storage<int*> makeStorage()
{
    int local { 42 };
    return Storage<int*> { &local }; // bad: pointer dangles after return
}
```

The biggest lesson of pointer specialization is not the syntax. It is that a
template specialization can accidentally change ownership semantics.

### Three sane policies

Policy 1: make the pointer version explicitly a view.

```text
Storage<T*> does not own the pointee.
Caller must keep the pointee alive.
```

This can be acceptable, but because the class name is still `Storage`, the view
semantics may be easy to miss.

Policy 2: reject pointer types.

```cpp
#include <type_traits>

template <typename T>
class Storage
{
    static_assert(!std::is_pointer_v<T>, "Storage<T*> is not allowed");

private:
    T m_value {};
};
```

This keeps the primary class's copy/value semantics honest.

If `nullptr` is also a concern:

```cpp
static_assert(!std::is_pointer_v<T> && !std::is_null_pointer_v<T>,
              "Storage requires a non-pointer value type");
```

Policy 3: make the pointer specialization own a copy.

```cpp
#include <memory>

template <typename T>
class Storage<T*>
{
private:
    std::unique_ptr<T> m_value {};

public:
    explicit Storage(T* value)
        : m_value { value ? std::make_unique<T>(*value) : nullptr }
    {
    }

    void print() const
    {
        if (m_value)
            std::cout << *m_value << '\n';
    }
};
```

Now `Storage<T*>` does not dangle when the caller's object dies, but it has
dynamic allocation and copy requirements.

### CS6340 pointer warning

LLVM-style code often uses raw pointers as non-owning references:

```cpp
Instruction* I;
BasicBlock* BB;
Module* M;
```

Do not assume a pointer template specialization owns those objects. In LLVM
APIs, ownership is usually explicit in the surrounding type or API contract.

When reading:

```cpp
std::vector<Instruction*> instructions;
```

think:

```text
container owns the list structure
container probably does not own the Instruction objects
```

## 26.x — Chapter 26 summary and quiz

### Summary

- A class template is a stencil for generating concrete classes.
- Template type parameters are placeholders for types.
- Template non-type parameters are placeholders for compile-time values.
- The compiler needs to see full template definitions when it instantiates them.
- Class templates usually keep member definitions in headers or included
  implementation files.
- Full function template specialization customizes a function template for one
  exact set of template arguments.
- Prefer a normal non-template overload over function template specialization
  when an overload can express the behavior cleanly.
- Full function specializations defined in headers should be marked `inline`.
- Full class specialization replaces the entire class definition for one exact
  set of template arguments.
- Partial class specialization replaces the class definition for a pattern such
  as `T*` or `char, N`.
- Function templates cannot be partially specialized.
- A member function of a partially specialized class is not itself partially
  specialized; it is an ordinary member of that specialized class.
- Pointer specializations are syntactically useful but semantically risky
  because they can change ownership and lifetime expectations.

### Quiz note

The LearnCpp chapter quiz covers class templates, multiple template
parameters, and a string-key/value specialization pattern. The quiz text is not
copied here; use the live LearnCpp page if you want to solve those prompts
directly.

## CS6340 patterns

### Reading template-heavy declarations

Break this:

```cpp
std::unordered_map<std::string, std::vector<Instruction*>> byFunction;
```

into:

```text
std::unordered_map<
    std::string,
    std::vector<Instruction*>
>
```

Meaning:

```text
hash table
  key: string
  value: vector of Instruction pointers
```

### Common mental translations

| C++ type | Plain reading |
| --- | --- |
| `std::vector<T>` | growable array of `T` |
| `std::array<T, N>` | fixed-size array of `N` `T` objects |
| `std::unique_ptr<T>` | owning pointer to one `T` |
| `std::optional<T>` | maybe contains a `T` |
| `std::pair<A, B>` | two values, first `A`, second `B` |
| `std::unordered_map<K, V>` | hash table from `K` to `V` |
| `SmallVector<T, N>` | LLVM-style vector with inline capacity `N` |

### What to ask when reading a template type

1. What is the outer abstraction?
2. Which template arguments are data types?
3. Which template arguments are compile-time values?
4. Does the type own the objects, or does it only point/view them?
5. Is a specialization likely changing behavior for a special family such as
   pointers, `bool`, or fixed-size buffers?

For Lab 1, this is enough to read most generic declarations before you fully
understand their implementation.
