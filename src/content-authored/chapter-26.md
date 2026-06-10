# Chapter 26 — Templates and Classes

> Source: <https://www.learncpp.com/> (Chapter 26)
> One file per chapter. Each lesson is a section below.

## Contents

- [26.1 — Template classes](#261--template-classes)
- [26.2 — Template non-type parameters](#262--template-non-type-parameters)
- [26.3 — Function template specialization](#263--function-template-specialization)
- [26.4 — Class template specialization](#264--class-template-specialization)
- [26.5 — Partial template specialization](#265--partial-template-specialization)
- [26.6 — Partial template specialization for pointers](#266--partial-template-specialization-for-pointers)
- [26.x — Chapter 26 summary and quiz](#26x--chapter-26-summary-and-quiz)
- [CS6340 patterns](#cs6340-patterns)

---

## 26.1 — Template classes

Back in Chapter 11 you met **function templates**, and they solved a very specific annoyance. You had a function that did the same thing for several types, and the only way to support all of them was to write the function out once per type:

```cpp
int max(int a, int b);
double max(double a, double b);
```

Two functions, identical except for one word. A function template let you write the logic once, with the type left as a blank to be filled in later. The compiler then stamped out a real `max` for each type you actually used.

The same duplication shows up — even more painfully — in classes that *store* or *organize* data. Suppose you want a tiny wrapper that holds a single value. If you need one for `int` and one for `std::string`, you end up writing two classes that are almost letter-for-letter the same:

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

Only the contained type differs. Every method you add — a getter, a setter, a print — you now have to add *twice*, keep in sync forever, and debug in two places. That is exactly the situation a **class template** was invented to eliminate.

### From a fixed type to a placeholder

A class template turns the type that varies into a named placeholder. You write the class once, in terms of that placeholder, and let the compiler fill it in:

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

The first line is the whole idea. Read:

```text
template <typename T>
class Box
```

out loud as: *"`Box` is a pattern for making classes, and `T` is the placeholder type."* The keyword `typename` announces that `T` stands for a type — any type — that the caller will choose later. (You will also see `class` used in the same spot: `template <class T>` means exactly the same thing here. `typename` is the more descriptive choice and the one we prefer.)

When you then write `Box<int>` or `Box<std::string>`, you are asking the compiler to take the pattern and substitute your type in for every `T`:

```text
Box<int>         -> Box with T replaced by int
Box<std::string> -> Box with T replaced by std::string
```

`Box<int>` and `Box<std::string>` are two genuinely different classes, generated on demand, with no duplicated source code on your side.

### A template is a stencil, not the finished class

This is the single most important mental model for the chapter, so let us make it concrete with an analogy. A class template is a **stencil**. The stencil is not a painting; it is the cut-out shape you press paint through to *make* a painting. You can make a red one and a blue one from the same stencil, but the stencil itself is neither red nor blue.

`Box` is the stencil. `Box<int>` is what you get when you press `int` through it. The compiler only does that pressing when you actually ask for a particular type:

```text
source template (the stencil):

    template <typename T>
    class Box { T m_value; };

requested uses:

    Box<int>          Box<std::string>
       |                    |
       v                    v
    compiler generates concrete class code for each requested type
```

The act of generating a concrete class from the template is called **instantiation**. If your program never mentions `Box<double>`, the compiler never instantiates it — that version simply does not exist in your binary.

This stencil model also explains a frustration you will eventually hit: **template error messages can be enormous and point at code you never wrote.** When something is wrong with `Box<std::vector<int>>::get()`, the error may be reported inside the *generated* code for that instantiation, not at your template definition. The compiler is telling you "when I pressed `std::vector<int>` through your stencil, the result didn't compile." Once you expect that, the wall of text becomes much easier to read: find the instantiation it was building, and the real problem is usually nearby.

> **Key insight:** A class template is not a class. It is a recipe the compiler uses to *make* classes, one per type you actually use. Nothing exists until you ask for a specific instantiation like `Box<int>`.

### Defining members outside the class

For an ordinary, non-template class, you have been defining members outside the class body since Chapter 15 like this:

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

A class template needs two small additions, and the `Box::get` example above already showed them:

```cpp
template <typename T>
const T& Box<T>::get() const
{
    return m_value;
}
```

The two pieces that change are:

```cpp
template <typename T> // 1. repeat the template parameter list
Box<T>::get           // 2. qualify with Box<T>, not just Box
```

You repeat `template <typename T>` because each out-of-class definition is, itself, a template — it has to know about `T` to talk about it. And you write `Box<T>::`, with the angle brackets, because `Box` alone is the stencil's name; `Box<T>` is the name of the (generic) class the member belongs to.

A small convenience: *inside* the class body, you can usually write `Box` by itself and the compiler understands you mean the current instantiation. It is only *outside* the body that you must spell out `Box<T>`.

### Templates usually live entirely in the header

Here is where class templates break a habit you have built up. With an ordinary class, the tidy arrangement is to split it:

```text
Counter.h    -> the class declaration
Counter.cpp  -> the member function definitions
```

That split works because the compiler can compile `Counter.cpp` once, on its own, into object code, and the linker stitches it in wherever `Counter` is used.

A template cannot be compiled on its own, because there is nothing concrete to compile until a type is supplied. When `main.cpp` writes `Box<int>`, the compiler has to *see the full template definition right there* in order to instantiate `Box<int>`. If `main.cpp` sees only the class shape:

```cpp
template <typename T>
class Box
{
public:
    const T& get() const;
};
```

but the body of `Box<T>::get()` lives off in a separate `Box.cpp`, then `main.cpp` compiles fine — and then **linking fails**, because no concrete `Box<int>::get()` was ever generated. Nobody had both the request (`Box<int>`) and the recipe (the body of `get`) in the same place at the same time.

The practical rule is short:

> **Best practice:** For class templates, put both the class definition and its member function definitions in the header. Any translation unit that uses the template then has everything it needs to instantiate it.

If you want to keep the header tidy, a common pattern is to move the member definitions into a separate implementation file and `#include` it at the bottom of the header:

```text
Simple layout:

    Box.h
      template class definition
      template member definitions

Organized layout:

    Box.h
      template class definition
      #include "Box.inl"

    Box.inl
      template member definitions
```

The `.inl` (or `.ipp`) file is just text pulled into the header by `#include`. It is **not** a standalone source file — do not add it to your build as something to compile on its own. It exists only to be included.

> **Note:** This header-only constraint is the whole point of your chapter lab. You will build `Stack<T, Capacity>` as a class template living entirely in `stack.h` — the header *is* the implementation, including the one member you define out-of-class. Treat that as the lesson, not an inconvenience.

### Explicit instantiation exists (but is extra bookkeeping)

There is a more advanced route. A `.cpp` file can explicitly *request* the instantiations the program needs, which lets you keep member bodies in a `.cpp` after all:

```cpp
template class Box<int>;
template class Box<double>;
```

This can cut down on repeated instantiation work in very large builds, but it has a cost: someone now has to maintain that list and add to it every time a new instantiation is needed. For learning — and for most projects — prefer the header-only or header-plus-`.inl` layout. Reach for explicit instantiation only if a codebase already commits to it.

### Containers are the everyday class template

You have already been using class templates without naming them. Every standard container is one:

```cpp
std::vector<int>
std::vector<std::string>
std::array<double, 4>
```

The *behavior* of a vector — grow, index, iterate — is the same regardless of what it holds. Only the stored type changes. That is precisely the duplication class templates were built to remove, which is why the standard library implements its containers exactly this way.

When you read declarations like these in real code, read the angle brackets as part of the type:

| Type | Meaning |
| --- | --- |
| `std::vector<Instruction*>` | dynamic array of pointers to `Instruction` |
| `std::vector<std::string>` | dynamic array of strings |
| `std::unordered_map<std::string, int>` | hash table from string keys to int values |

The outer name tells you the **data structure**; the angle brackets tell you **what it stores**. That habit — read the container, then read its contents — will carry you through almost every template-heavy declaration you meet.

## 26.2 — Template non-type parameters

Every template parameter you have seen so far stands for a *type*:

```cpp
template <typename T>
class Box;
```

`T` gets replaced by `int`, `std::string`, or whatever you choose. But a template parameter can also stand for a **compile-time value** rather than a type. This is called a **non-type template parameter**, and it is how a class can bake a fixed number into its very identity.

### A fixed-capacity buffer

Imagine a small buffer that holds a compile-time-fixed number of items in a plain array — no dynamic allocation. The capacity is not data the object carries around; it is part of *what kind of buffer this is*:

```cpp
#include <cstddef>

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

The parameter list now carries one of each kind:

```cpp
T             // a template TYPE parameter   (a placeholder for a type)
Capacity      // a template NON-TYPE parameter (a placeholder for a value)
```

You supply both when you use the class:

```cpp
FixedBuffer<int, 8>  smallInts;
FixedBuffer<int, 32> moreInts;
```

The `8` and `32` are not constructor arguments and they do not arrive at runtime. They are settled at *compile time*, and the compiler uses them to size the internal array `m_items[Capacity]` directly into the object.

### The argument must be a constant expression

Because the compiler needs the value while it is still compiling, a non-type template argument has to be a **constant expression** — something whose value is known at compile time. A `constexpr` works:

```cpp
constexpr std::size_t seedLimit { 16 };

FixedBuffer<std::string, seedLimit> seeds; // fine: seedLimit is a compile-time constant
```

A value that only becomes known when the program runs does not:

```cpp
std::size_t readLimitFromUser();

std::size_t n { readLimitFromUser() };
FixedBuffer<std::string, n> seeds; // error: n is not a constant expression
```

The compiler cannot generate the concrete type `FixedBuffer<std::string, n>` because it does not know `n` yet — and it must, since `n` determines the array's size and therefore the object's layout.

### The value becomes part of the type

This is the subtle, important consequence. Because the capacity is a template argument, it is woven into the type's identity. These two are *different, unrelated types*:

```cpp
FixedBuffer<int, 4>
FixedBuffer<int, 8>
```

Both store `int`, but they are no more interchangeable than `int` and `double`:

```text
FixedBuffer<int, 4>  !=  FixedBuffer<int, 8>
            the type includes the value argument
```

You have already used a standard type that works exactly this way: `std::array`.

```cpp
std::array<int, 5>
std::array<int, 10>
```

`std::array` encodes its length in the type. That is what lets it store its elements inline, right inside the object, with no heap allocation — the length is known at compile time, so the storage can be reserved up front. This is precisely why your chapter lab builds `Stack` on top of `std::array<T, Capacity>`: the non-type `Capacity` flows straight through into the array's size, and `Stack<int, 4>` and `Stack<int, 8>` come out as different types, just as you would want.

> **Key insight:** A non-type template parameter makes a value part of the type. `FixedBuffer<int, 4>` and `FixedBuffer<int, 8>` are as distinct as any two unrelated classes — the compiler treats the value as identity, not as data.

### Out-of-class definitions name every parameter

When you define a member outside a multi-parameter template, the qualified name must mention *all* the template arguments, in order:

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

Notice the class name in the qualifier: `FixedBuffer<T, Capacity>`, carrying both the type parameter and the non-type parameter. This is the exact shape your lab asks you to write for `Stack<T, Capacity>::pop` — repeat the full parameter list, then qualify with both parameters.

### What is allowed as a non-type parameter?

The common, useful cases are:

- Integral values such as `int`, `bool`, and `std::size_t`
- Enumeration values
- Pointers and references to objects or functions with suitable linkage
- `std::nullptr_t`
- Floating-point values, as of C++20

In everyday study code, by far the most frequent use is a fixed size:

```cpp
template <typename T, std::size_t N>
class StaticVectorLikeThing;
```

### Reading these in the wild

When you encounter something like this in LLVM-style code:

```cpp
SmallVector<Instruction*, 8> worklist;
```

decode it in two halves:

```text
SmallVector
  stores: Instruction*                       <- the type parameter
  inline capacity / compile-time tuning: 8   <- the non-type parameter
```

You do not have to understand every implementation detail to read the declaration. The type is announcing both *what kind of thing it holds* and, through its non-type parameter, *how much fixed storage or compile-time policy* it carries.

## 26.3 — Function template specialization

Let us step back to function templates for a moment, because they expose an idea we will then apply to classes. A single template gives *one* implementation that serves *every* type. That is usually what you want — but not always. Sometimes one particular type needs to be handled differently from the rest.

### When the general rule does not fit one type

Here is a generic printer:

```cpp
#include <iostream>

template <typename T>
void printValue(const T& value)
{
    std::cout << value << '\n';
}
```

This is fine for numbers and strings. But a `bool` printed this way comes out as `1` or `0`, when you would probably rather see:

```text
true
false
```

The general implementation is right for almost everything and wrong for this one case. We need a way to override behavior for a specific type.

### First choice: an ordinary overload

The simplest fix is usually not a template feature at all. It is a plain, non-template overload:

```cpp
void printValue(bool value)
{
    std::cout << (value ? "true" : "false") << '\n';
}
```

Now overload resolution does the work for you:

```cpp
printValue(42);    // no better match -> uses the function template
printValue(true);  // an exact non-template match -> uses the overload
```

There is a rule worth memorizing here: when a non-template function is an *equally good* match as a function template, the **non-template function wins**. That makes overloads a clean, predictable tool. And because an overload is free to have whatever signature reads best, it is almost always the most natural choice.

> **Best practice:** When you want different behavior for one type, reach for an ordinary overload first. It is simpler than specialization, wins resolution against the template, and lets you choose the most natural signature.

### Full function template specialization

The other tool is **full function template specialization**: you supply a dedicated implementation for one exact set of template arguments.

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

Read the two new pieces of syntax:

```cpp
template <>                        // empty list: no template parameters remain
void emit<std::string_view>(...)   // this is emit for T = std::string_view, specifically
```

The empty `template <>` is the signal "this is a specialization — every parameter has already been pinned down." And the primary template must be declared *before* its specialization; the compiler has to know the general case exists before you can say "here is the special case of it."

### The signature must match the primary template

A specialization is not a fresh overload — it is the *same* function with a substituted type, so its signature must be what you get by substituting that type into the primary template. If the primary is:

```cpp
template <typename T>
void emit(const T& value);
```

then for `double`, the parameter type is `const T&` with `T = double`, which is `const double&`:

```cpp
template <>
void emit<double>(const double& value); // correct
```

You cannot quietly change the parameter to by-value:

```cpp
void emit<double>(double value); // wrong — does not match const T& with T = double
```

If you genuinely want a different signature — say, by-value — then you do not want a specialization at all. You want an overload, which is allowed to have any signature it likes.

### A header trap: specializations are not implicitly inline

This one bites people, so notice it now. A full function specialization behaves like an *ordinary function* for the One Definition Rule — unlike the primary template, it is not implicitly inline. If this lives in a header that more than one `.cpp` includes:

```cpp
template <>
void emit<double>(const double& value)
{
    std::cout << std::scientific << value << '\n';
}
```

each translation unit gets its own copy of the definition, and the linker reports a duplicate-definition error. If a full specialization must live in a header, mark it `inline`:

```cpp
template <>
inline void emit<double>(const double& value)
{
    std::cout << std::scientific << value << '\n';
}
```

> **Warning:** A full function (or member) specialization is not implicitly inline. If it sits in a header included by multiple source files, mark it `inline` or you will get duplicate-definition linker errors.

### You can delete a specialization to forbid a type

Sometimes the right behavior for a type is "no behavior — refuse to compile." You can `= delete` a specialization to make any call that would resolve to it a compile error:

```cpp
template <typename T>
void sendMetric(const T& value)
{
    std::cout << value << '\n';
}

template <>
void sendMetric<const char*>(const char* const&) = delete;
```

Now `sendMetric` of a `const char*` is rejected at compile time rather than doing something you did not intend. In modern code a constraint or a `static_assert` is often a clearer way to express "this type is not allowed," but deleted specializations are a pattern you should recognize when you read it.

### Members of class templates are a different story

One thing this lesson does *not* cover: customizing a single member of a class template. If you have

```cpp
template <typename T>
class Cell
{
public:
    void print() const;
};
```

and you want `Cell<double>::print()` to behave specially, that is not a standalone function-template specialization. You are specializing a *member of a class template* — a related but distinct mechanism. That is the subject of the next lesson.

## 26.4 — Class template specialization

The previous lesson specialized functions. Now we do the same for classes — with one extra power. Because a class is a whole bundle of state and behavior, a class specialization can replace not just one function but the **entire class definition** for a chosen type.

### Full class specialization

Here is a display wrapper whose general version just prints its value, plus a `bool` version that prints words:

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

Using it, the compiler picks the right definition for you:

```cpp
Display<int> count { 5 };       // uses the primary template
Display<bool> enabled { true }; // uses the full specialization
```

The syntax of that second definition:

```cpp
template <>
class Display<bool>
```

reads as: *"This is the complete `Display` class for `T = bool`."* The empty `template <>` again means "no parameters remain — everything is fixed."

### A specialization is an independent class

Here is the crucial difference from anything you have seen so far. `Display<bool>` does **not** have to resemble `Display<T>` at all. It does not inherit from the primary template; it *replaces* it for that one type. It can store different data, add members, remove members — it is a separate class definition that merely shares a name.

```text
Primary template, used for most types:
    Display<int>
    Display<double>
    Display<std::string>

Full specialization, used only for bool:
    Display<bool>   <- a separate, independent class definition
```

That freedom is powerful, and it is also a foot-gun. If callers expect every `Display<T>` to offer the same interface — the same `print()`, the same constructor shape — then a specialization that quietly drops or renames a member will surprise them.

> **Warning:** A class specialization is a wholly independent class. Unless you have a strong reason, keep its public interface consistent with the primary template, so `Display<bool>` is still usable everywhere a `Display<T>` is expected.

### The primary template must come first

The compiler cannot make sense of a specialization until it knows the general template it specializes. So the primary template (at least a declaration of it) must appear before the specialization:

```cpp
template <typename T>
class Display;       // primary template declared first

template <>
class Display<bool>; // then the specialization
```

In practice, define the full primary template, then put the specialization right next to it. And resist a tempting-but-dangerous pattern: making the specialization available only if some extra header is included. Then behavior depends on *include order*:

```text
translation unit A includes the specialization -> gets "yes/no" behavior
translation unit B does not                     -> gets primary-template behavior
```

Two parts of the same program now disagree about what `Display<bool>` *is*. That is a violation of the One Definition Rule and a debugging nightmare. Keep the specialization visible wherever the primary template is.

### Specializing just one member function

Replacing the entire class is often more than you need. Frequently only *one* member should behave differently. You can specialize a single member of a class template, leaving the rest to come from the primary template:

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

Read this carefully: `Metric<double>` as a *class* still comes from the primary template. It has the same data member, the same constructor, the same everything — *except* that its `print()` uses this custom definition. You have overridden exactly one method.

And notice the `inline`. This is the same trap as in 26.3: a specialized member function defined in a header is not implicitly inline, so include it in more than one source file without `inline` and the linker complains about duplicates.

### Use class specialization sparingly

Full class specialization earns its keep when a type genuinely needs *different machinery*, not just a slightly different output:

- packing `bool` values into individual bits to save space
- treating a `char` buffer as text rather than as a sequence of numbers
- handling pointer types differently from value types (the subject of 26.6)
- adapting a template to a type with unusual constraints

What it should *not* be is a heavyweight substitute for a small `if` or an ordinary overload. If a one-line difference is all you need, a regular overload — or `if constexpr` once you learn it — will be clearer than a second copy of the class.

> **Best practice:** Reach for full class specialization only when a type needs fundamentally different implementation mechanics. For a small behavioral tweak, an overload or a single specialized member is lighter and clearer.

## 26.5 — Partial template specialization

A full specialization pins down *every* template parameter — it applies to one exact combination of arguments and nothing else. That is often too narrow. **Partial specialization** lets you fix *some* of the parameters while leaving others open, so a single specialized definition covers a whole *family* of types.

### Full specialization is all-or-nothing

Given a template with a type and a size:

```cpp
template <typename T, std::size_t N>
class StaticList;
```

a full specialization nails down both:

```cpp
template <>
class StaticList<char, 32>;
```

That covers `char` lists of length exactly 32 — and no other length. If what you actually want is "*all* `char` lists, whatever their size," full specialization cannot express it. You would have to write one for length 1, one for length 2, and so on forever.

### Fixing part of the pattern

Partial specialization solves exactly this. You pin `T` to `char` but leave `N` as a parameter:

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

The head of the specialization tells the whole story:

```cpp
template <std::size_t N>     // N is still a parameter...
class StaticList<char, N>    // ...but T is fixed to char
```

Read it as: *"This is the `StaticList` for when `T` is `char`, but `N` is still a free template parameter."* One definition, covering every `char` list:

```cpp
StaticList<int, 4>    a; // primary template (T is not char)
StaticList<char, 4>   b; // partial specialization
StaticList<char, 128> c; // same partial specialization, different N
```

When you ask for a type, the compiler checks which definition fits *best*: `StaticList<char, 128>` matches the `char` specialization more specifically than the primary template, so the specialization wins.

### Functions cannot be partially specialized — only overloaded

A rule that trips up many people: **function templates cannot be partially specialized.** This is not valid syntax:

```cpp
template <typename T, std::size_t N>
void print(const StaticList<T, N>& list);

template <std::size_t N>
void print<char, N>(const StaticList<char, N>& list); // not valid C++
```

Functions can be fully specialized, and they can be *overloaded* — and overloading turns out to give you everything you wanted anyway:

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

The second one is **not** a partial specialization. It is a separate function template overload that happens to accept a `char` list. Overload resolution prefers the more specific match, so it behaves the way you hoped — but the mechanism is overloading, not specialization.

> **Key insight:** Classes can be partially specialized; functions cannot. When you need "the same function, but special for a family of types," write an overload — overload resolution picks the more specific one.

### Members inherit the same limitation

Because a member function is still a function, you cannot partially specialize one in isolation. This does not work:

```cpp
template <std::size_t N>
void StaticList<double, N>::print() const; // illegal: partially specializing a function
```

The fix is to partially specialize the *class*, and then define the member of that specialized class normally:

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
    // an ordinary member of a partially specialized class
}
```

The catch is that specializing the whole class to change one method can force you to duplicate everything *else* the class has. A common remedy is to factor the shared machinery into a base template and let each specialization inherit it:

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

The `this->` before `m_items` is doing real work, not decoration. When a name lives in a *dependent* base class — a base whose type involves the template parameters — the compiler does not look it up in that base by default. Qualifying with `this->` tells it to defer the lookup until the type is known, so `m_items` is found. You will not need this until you inherit from templated bases, but recognize it when you see it.

### When the effort is worth it

Partial specialization pays off when there is a genuine *family-level* distinction to capture:

```text
all T* pointer types
all char buffers of any length
all Pair<std::string, T> types
all StaticList<T, 0> empty lists
```

If the distinction is one *exact* type, full specialization is simpler. If it is one *function call*, an overload or `if constexpr` is clearer. Save partial specialization for "this whole family of types needs different machinery." (Your chapter lab deliberately stops short of this — partial specialization is here for reading comprehension, not for the exercise — but the very next lesson shows the single most common place you will actually meet it: pointers.)

## 26.6 — Partial template specialization for pointers

The textbook example of a useful partial specialization — and one you will see constantly in low-level C++ — is specializing for *pointer types*. It also carries a lesson that has nothing to do with syntax and everything to do with safety, so stay for the second half.

### Why pointers need their own behavior

Start with a plain value holder:

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

For values, it does the obvious thing:

```cpp
Storage<int> count { 5 };
Storage<double> ratio { 0.75 };
```

But hand it a pointer and the "obvious thing" is probably not what you meant:

```cpp
int x { 5 };
Storage<int*> pointerStorage { &x };
pointerStorage.print();
```

This prints an *address*, not `5`, because with `T = int*`, the member `m_value` is an `int*` and `std::cout` prints the pointer itself. If you wanted to see the pointed-to value, you need different behavior for pointers.

### One specialization for *all* pointer types

You could specialize for `int*`, then `double*`, then `Widget*`, forever. Instead, partially specialize for the *pattern* `T*`, which matches any pointer at all:

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

Read the head:

```cpp
template <typename T>
class Storage<T*>
```

as: *"For any pointer type, `T` is the pointed-to type — use this version of `Storage`."* So:

```cpp
Storage<int*>     -> T is int
Storage<double*>  -> T is double
Storage<Node*>    -> T is Node
```

One definition covers every pointer. (And, as always, the primary template must be defined first.)

### `T` is the pointee, not the pointer

This is the easiest detail to get backwards, so look closely. Inside `Storage<T*>`, the parameter `T` is **not** the pointer type — it is the type being *pointed at*. For `Storage<int*>`, `T` is `int`. That is why the member is declared as:

```cpp
T* m_value {};  // T is int, so this is an int* — correct
```

and not:

```cpp
T m_value {};   // T is int, so this would store a bare int, not the pointer
```

The `*` in `T* m_value` rebuilds the pointer from the pointee type the pattern captured.

### The real lesson is ownership, not syntax

Now the part that matters. Look at what the pointer specialization quietly changed:

```text
Storage<int>   copies and owns an int value
Storage<int*>  stores a pointer to an int that belongs to someone else
```

The value version *owns* its data — the `int` lives inside the object. The pointer version owns nothing; it just *refers* to an `int` that lives elsewhere and could be destroyed at any time. That means `Storage<int*>` can **dangle**:

```cpp
Storage<int*> makeStorage()
{
    int local { 42 };
    return Storage<int*> { &local }; // BUG: local dies when the function returns
}
```

The returned `Storage<int*>` holds the address of a local variable that no longer exists. The class compiles, the call compiles, and the program is broken.

> **Warning:** A pointer specialization can silently change a class's *ownership* semantics. `Storage<int>` owns its value; `Storage<int*>` only borrows one and can dangle. The dangerous part is that both are still spelled `Storage`, so the change is easy to overlook.

### Three honest policies

When you find yourself specializing for pointers, pick a policy deliberately rather than drifting into one.

**Policy 1 — be an explicit non-owning view.** Decide that `Storage<T*>` does not own the pointee, and document it loudly. The caller must keep the pointee alive. This is legitimate (it is what `std::string_view` does), but because the type is still named `Storage`, the borrowing is easy to miss — so the documentation has to carry weight.

**Policy 2 — reject pointers outright.** If your class is meant to *own* its data, forbid pointer types so nobody accidentally gets borrowing semantics:

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

This keeps the value/copy semantics honest — the compiler refuses any pointer. If `nullptr` is also a concern, widen the guard:

```cpp
static_assert(!std::is_pointer_v<T> && !std::is_null_pointer_v<T>,
              "Storage requires a non-pointer value type");
```

**Policy 3 — own a copy of the pointee.** If you want pointer-shaped construction but value semantics, make the specialization deep-copy what it is given:

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

Now `Storage<T*>` no longer dangles when the caller's object dies — it made its own copy — at the cost of a heap allocation and the requirement that `T` be copyable. There is no free lunch; you are choosing which cost to pay.

### A note for the LLVM-flavored code ahead

In LLVM-style code, raw pointers are the *normal* way to refer to objects you do not own:

```cpp
Instruction* I;
BasicBlock* BB;
Module* M;
```

So do not assume that because some template stores a `T*`, it owns the pointee. In LLVM the ownership lives in the surrounding type or the API contract, not in the pointer itself. When you read:

```cpp
std::vector<Instruction*> instructions;
```

think:

```text
the vector owns the list structure (the array of pointers)
the vector almost certainly does NOT own the Instruction objects
```

The container manages its own storage; the things it points at are managed by someone else. Keeping those two ownerships separate in your head will save you a lot of confusion — and a few crashes.

## 26.x — Chapter 26 summary and quiz

### Summary

Class templates extend the function-template idea from single operations to whole data structures, and the rest of the chapter is about bending that mechanism for special cases.

- A **class template** is a stencil for generating classes — one concrete class per type you actually use. Nothing exists until you request an instantiation like `Box<int>`.
- **Type parameters** (`typename T`) are placeholders for types; **non-type parameters** (`std::size_t N`) are placeholders for compile-time values, and the value becomes part of the type.
- Because instantiation needs the full definition on hand, class templates normally keep their member definitions in the **header** (or an `.inl` included by it), not a separate `.cpp`.
- Out-of-class member definitions **repeat the template parameter list** and **qualify the class name with every parameter** (`Box<T>::get`, `FixedBuffer<T, Capacity>::at`).
- **Full function template specialization** customizes a function template for one exact set of arguments — but prefer a plain **overload** when one can express the behavior, since the non-template overload wins resolution and can use any signature.
- A full function (or member) specialization in a header must be marked **`inline`** — it is not implicitly inline.
- **Full class specialization** replaces the entire class for one exact type; the specialization is an *independent* class, so keep its public interface consistent.
- **Partial class specialization** matches a *family* — `T*`, or `char, N` — by fixing some parameters and leaving others open.
- **Function templates cannot be partially specialized**; use overloads instead. A member of a partially specialized class is just an ordinary member of that specialized class.
- **Pointer specializations** are syntactically handy but semantically risky: they can silently turn an owning class into a borrowing one, with dangling as the consequence.

### Quiz note

The LearnCpp chapter quiz exercises class templates, multiple template parameters, and a key/value specialization pattern. We have not reproduced the quiz prompts here — if you want to solve them directly, work them on the live LearnCpp page. The best practice for this chapter, though, is in your hands already: the `Stack<T, Capacity>` lab walks you through a class template, a non-type parameter, an out-of-class member definition, and a set of full specializations, end to end.

## CS6340 patterns

The payoff for this chapter, in the context of reading real analysis code, is fluency with template-heavy declarations. You do not need to *implement* `std::unordered_map` to *read* one correctly — you need a reliable way to parse the angle brackets.

### Reading template-heavy declarations

Take a dense declaration and peel it apart from the outside in:

```cpp
std::unordered_map<std::string, std::vector<Instruction*>> byFunction;
```

Indenting the arguments by hand makes the structure visible:

```text
std::unordered_map<
    std::string,
    std::vector<Instruction*>
>
```

and now it reads in plain English:

```text
a hash table
  key:   a string
  value: a vector of Instruction pointers
```

That is the whole skill: identify the outer container, then read each template argument as its own (possibly nested) type.

### Common mental translations

Build a vocabulary so these resolve at a glance rather than needing to be decoded each time:

| C++ type | Plain reading |
| --- | --- |
| `std::vector<T>` | growable array of `T` |
| `std::array<T, N>` | fixed-size array of `N` `T` objects |
| `std::unique_ptr<T>` | owning pointer to one `T` |
| `std::optional<T>` | maybe contains a `T` |
| `std::pair<A, B>` | two values, first `A`, second `B` |
| `std::unordered_map<K, V>` | hash table from `K` to `V` |
| `SmallVector<T, N>` | LLVM-style vector with inline capacity `N` |

### Five questions for any template type

When a declaration stops you, ask these in order:

1. What is the outer abstraction — what data structure or wrapper is this?
2. Which template arguments are **data types** (what it stores)?
3. Which template arguments are **compile-time values** (sizes, policies)?
4. Does the type **own** its objects, or only point at / view them?
5. Is a **specialization** likely changing behavior for a special family — pointers, `bool`, fixed-size buffers?

For Lab 1, that is enough to read almost any generic declaration confidently, long before you understand its implementation. And it is exactly the muscle the rest of this chapter built: a class template is a stencil, its arguments are types and values, and a specialization may be quietly rewriting the rules for one family of those arguments.
