# Chapter 21 — Operator Overloading

> Source: <https://www.learncpp.com/> (Chapter 21)
> One file per chapter. Each lesson is a section below.

## Contents
- [21.1 — Introduction to operator overloading](#211--introduction-to-operator-overloading)
- [21.2 — Overloading the arithmetic operators using friend functions](#212--overloading-the-arithmetic-operators-using-friend-functions)
- [21.3 — Overloading operators using normal functions](#213--overloading-operators-using-normal-functions)
- [21.4 — Overloading the I/O operators](#214--overloading-the-io-operators)
- [21.5 — Overloading operators using member functions](#215--overloading-operators-using-member-functions)
- [21.6 — Overloading unary operators +, -, and !](#216--overloading-unary-operators-----and-)
- [21.7 — Overloading the comparison operators](#217--overloading-the-comparison-operators)
- [21.8 — Overloading the increment and decrement operators](#218--overloading-the-increment-and-decrement-operators)
- [21.9 — Overloading the subscript operator](#219--overloading-the-subscript-operator)
- [21.10 — Overloading the parenthesis operator](#2110--overloading-the-parenthesis-operator)
- [21.11 — Overloading typecasts](#2111--overloading-typecasts)
- [21.12 — Overloading the assignment operator](#2112--overloading-the-assignment-operator)
- [21.13 — Shallow vs. deep copying](#2113--shallow-vs-deep-copying)
- [21.14 — Overloading operators and function templates](#2114--overloading-operators-and-function-templates)
- [21.x — Chapter 21 summary and quiz](#21x--chapter-21-summary-and-quiz)
- [21.y — Chapter 21 project](#21y--chapter-21-project)

---

## 21.1 — Introduction to operator overloading

### Operators are function calls with special syntax

Built-in operators already behave like operations with operands:

```cpp
int x { 2 };
int y { 3 };

int z { x + y };
```

For program-defined types, C++ lets you define what many existing operators
mean. That is operator overloading.

```cpp
struct Point
{
    int x {};
    int y {};
};

Point operator+(const Point& a, const Point& b)
{
    return { a.x + b.x, a.y + b.y };
}

Point p { Point{1, 2} + Point{3, 4} }; // {4, 6}
```

The overloaded operator is a function named with the `operator` keyword:

```cpp
operator+
operator==
operator<<
```

### Why overload operators?

Operator overloads make class types feel like natural values when the operation
has an obvious meaning.

Good fits:

| Type | Operator | Intuitive meaning |
|---|---|---|
| `Fraction` | `+` | add two fractions |
| `Point` | `==` | same coordinates |
| `Duration` | `<` | shorter duration |
| `Matrix` | `()` | access row/column cell |
| `std::ostream` output helper | `<<` | print object |

Bad fit:

```cpp
recordA ^ recordB; // unclear unless the domain already gives ^ a meaning
```

If the operator would surprise a reader, use a named function:

```cpp
recordA.mergeWith(recordB);
```

### Operator overload limits

Operator overloading is intentionally constrained:

- You can overload only existing operators.
- You cannot create a new operator token.
- You cannot change an operator's precedence or associativity.
- You cannot change how many operands an operator has.
- At least one operand must be a program-defined type.
- Some operators are not overloadable, including `?:`, `sizeof`, `::`, `.`,
  `.*`, and `typeid`.

Example of the precedence trap:

```cpp
// Do not overload ^ for exponentiation.
// 2 + x ^ y would still parse using ^ precedence, not math exponent precedence.
```

### Return conventions

Match the behavior users expect from the built-in operators:

| Operator kind | Usually returns |
|---|---|
| non-mutating arithmetic (`+`, `-`, `*`) | new value by value |
| comparison (`==`, `<`) | `bool` |
| stream insertion/extraction (`<<`, `>>`) | stream by reference |
| mutating assignment-like operators (`=`, `+=`, prefix `++`) | left operand by reference |
| postfix `++` / `--` | old value by value |

The return type is part of the operator's usability. Returning the wrong thing
can break chaining or create needless copies.

### Member, friend, or normal function?

Overloaded operators can be implemented as:

| Form | Shape | Common use |
|---|---|---|
| member function | `lhs.operator+(rhs)` | unary ops, assignment, subscript, call |
| friend non-member | `operator+(lhs, rhs)` with private access | symmetric binary ops needing private data |
| normal non-member | `operator+(lhs, rhs)` using public API | symmetric binary ops when public API is enough |

Keep this distinction in mind; the rest of the chapter compares these forms.

CS6340 tie-in: LLVM and standard library code use overloaded operators for
streams, iterators, comparisons, smart pointers, and containers. You do not need
to overload operators for the lab, but you do need to read overload-heavy C++
fluently.

---

## 21.2 — Overloading the arithmetic operators using friend functions

### Friend overloads for symmetric binary operators

Arithmetic operators usually have two operands and do not modify either one:

```cpp
Money total { subtotal + tax };
```

For a type that hides its data, a friend function can access private members
while still behaving like a non-member operator.

```cpp
class Cents
{
private:
    int m_cents {};

public:
    explicit Cents(int cents)
        : m_cents { cents }
    {
    }

    int cents() const { return m_cents; }

    friend Cents operator+(const Cents& left, const Cents& right)
    {
        return Cents { left.m_cents + right.m_cents };
    }
};
```

Usage:

```cpp
Cents nickel { 5 };
Cents dime { 10 };

Cents total { nickel + dime };
```

The expression `nickel + dime` maps to:

```cpp
operator+(nickel, dime)
```

### Why friend functions are common here

For binary operators like `+`, neither operand is conceptually "in charge."
A non-member keeps the operation symmetric.

```cpp
left + right
```

With a member function, the left operand must be the class object:

```cpp
left.operator+(right)
```

That can be less flexible for mixed-type expressions where conversions may be
needed on the left side.

### Defining a friend inside the class

A friend operator can be defined directly inside the class:

```cpp
class Distance
{
private:
    int m_meters {};

public:
    explicit Distance(int meters)
        : m_meters { meters }
    {
    }

    friend Distance operator+(const Distance& a, const Distance& b)
    {
        return Distance { a.m_meters + b.m_meters };
    }
};
```

That is concise for small functions. Longer operators are usually easier to read
when declared in the class and defined below it.

### Mixed operands

If both `Distance + int` and `int + Distance` should work, define both overloads:

```cpp
class Distance
{
private:
    int m_meters {};

public:
    explicit Distance(int meters)
        : m_meters { meters }
    {
    }

    int meters() const { return m_meters; }

    friend Distance operator+(const Distance& d, int meters)
    {
        return Distance { d.m_meters + meters };
    }

    friend Distance operator+(int meters, const Distance& d)
    {
        return d + meters;
    }
};
```

The second overload reuses the first, avoiding duplicate arithmetic.

### Implement operators in terms of other operators

When possible, define one core operation and express related operations through
it.

```cpp
class Count
{
private:
    int m_value {};

public:
    explicit Count(int value)
        : m_value { value }
    {
    }

    friend Count operator+(const Count& a, const Count& b)
    {
        return Count { a.m_value + b.m_value };
    }

    friend Count operator-(const Count& a, const Count& b)
    {
        return Count { a.m_value + (-b.m_value) };
    }
};
```

The same idea is more common later with `+=` and `+`: implement `+=` once, then
make `+` create a copy and call `+=`.

### Avoid overloading arithmetic without domain meaning

Arithmetic operator overloads are good when the domain is value-like. They are
bad when the operation is really a command:

```cpp
jobA + jobB; // merge? schedule? concatenate logs? unclear
```

Use named functions when the operation is not mathematically or conventionally
obvious.

---

## 21.3 — Overloading operators using normal functions

### Non-member does not have to mean friend

If an operator can be implemented through the public interface, it can be a
normal non-member function.

```cpp
class Cents
{
private:
    int m_cents {};

public:
    explicit Cents(int cents)
        : m_cents { cents }
    {
    }

    int cents() const { return m_cents; }
};

Cents operator+(const Cents& left, const Cents& right)
{
    return Cents { left.cents() + right.cents() };
}
```

This keeps encapsulation stronger: the operator does not need privileged access
to private data.

### Friend vs normal function

| Question | Prefer |
|---|---|
| Can the operator be written cleanly using public API? | normal non-member |
| Would public getters expose awkward internals only for this operator? | friend |
| Does the operator need symmetric conversions? | non-member, friend if needed |
| Is the operator required to be a member? | member |

Friend is not evil, but it is a deliberate exception to ordinary access rules.

### Keep operators near the type

Even normal non-member operators should live close to the type they operate on,
usually in the same header/source pair and namespace.

```cpp
namespace units
{
    class Meters
    {
        // ...
    };

    Meters operator+(const Meters& a, const Meters& b);
}
```

This helps argument-dependent lookup find the operator and helps readers find
the implementation.

### Avoid adding operators for types you do not own

It is technically possible to write some overloads involving standard library
types, but it is usually a bad idea to invent new operators for types you do not
own.

```cpp
// Bad idea: surprising global behavior for standard library types.
// std::string operator*(int count, const std::string& text);
```

Prefer named helper functions in your own namespace.

---

## 21.4 — Overloading the I/O operators

### Output operator `<<`

The stream insertion operator is usually a non-member, often a friend, because
the left operand is `std::ostream`, not your class.

```cpp
#include <iostream>

class Point
{
private:
    int m_x {};
    int m_y {};

public:
    Point(int x, int y)
        : m_x { x }
        , m_y { y }
    {
    }

    friend std::ostream& operator<<(std::ostream& out, const Point& point)
    {
        out << '(' << point.m_x << ", " << point.m_y << ')';
        return out;
    }
};
```

Returning the stream by reference enables chaining:

```cpp
Point p { 2, 5 };
std::cout << "p = " << p << '\n';
```

Evaluation shape:

```
std::cout << "p = "  -> returns std::cout
std::cout << p      -> returns std::cout
std::cout << '\n'
```

### Input operator `>>`

The extraction operator reads from a stream into an existing object. It returns
the stream by reference for chaining and state checks.

```cpp
class Point
{
private:
    int m_x {};
    int m_y {};

public:
    Point(int x = 0, int y = 0)
        : m_x { x }
        , m_y { y }
    {
    }

    friend std::istream& operator>>(std::istream& in, Point& point)
    {
        int x {};
        int y {};

        if (in >> x >> y)
        {
            point.m_x = x;
            point.m_y = y;
        }

        return in;
    }
};
```

The object parameter is non-const because extraction modifies it.

### Guarding against partial extraction

Do not update half an object and then discover the second input failed.
Prefer reading into temporaries first:

```cpp
friend std::istream& operator>>(std::istream& in, Point& point)
{
    int x {};
    int y {};

    if (in >> x >> y)
        point = Point { x, y };

    return in;
}
```

This preserves the old `point` unless a complete replacement was read.

### Semantically invalid input

Stream extraction can succeed syntactically while the values are invalid for the
class invariant.

```cpp
class Fraction
{
private:
    int m_num {};
    int m_den { 1 };

public:
    Fraction(int num = 0, int den = 1)
        : m_num { num }
        , m_den { den }
    {
        assert(den != 0);
    }

    friend std::istream& operator>>(std::istream& in, Fraction& fraction)
    {
        int num {};
        int den {};
        char slash {};

        if (in >> num >> slash >> den && slash == '/' && den != 0)
            fraction = Fraction { num, den };
        else
            in.setstate(std::ios::failbit);

        return in;
    }
};
```

Setting `failbit` lets callers use normal stream error handling.

### Formatting is part of the contract

For output, choose a stable representation:

```cpp
std::cout << Point{2, 5}; // "(2, 5)"
```

For input, document or enforce the accepted representation:

```cpp
std::cin >> fraction; // expects something like 3/4
```

Do not make extraction guess too many formats unless there is a clear product
need; permissive parsers are harder to reason about.

---

## 21.5 — Overloading operators using member functions

### Member operator shape

A member operator uses the left operand as the implicit object:

```cpp
class Cents
{
private:
    int m_cents {};

public:
    explicit Cents(int cents)
        : m_cents { cents }
    {
    }

    Cents operator+(const Cents& other) const
    {
        return Cents { m_cents + other.m_cents };
    }
};
```

Expression mapping:

```cpp
a + b
a.operator+(b)
```

The left operand is `*this`. The right operand is the explicit parameter.

### When operators must be members

Some operators are required to be overloaded as members:

| Operator | Common meaning |
|---|---|
| `operator=` | assignment |
| `operator[]` | subscript |
| `operator()` | function call |
| `operator->` | member access through pointer-like type |

Even when not required, unary operators and assignment-like operators are often
best as members because they naturally operate on the current object.

### Member vs non-member tradeoff

Member operators are direct and can access private data, but they make the left
operand special.

```cpp
class Distance
{
public:
    Distance operator+(int meters) const;
};

Distance d { 10 };
d + 5;  // ok
5 + d;  // does not call a member of Distance because left operand is int
```

If both operand orders should be supported, a non-member is usually better.

### Const correctness

Operators that do not modify the object should be `const` member functions:

```cpp
bool isZero() const;
Cents operator-() const;
Cents operator+(const Cents& other) const;
```

Mutating operators are not `const`:

```cpp
Cents& operator+=(const Cents& other);
Cents& operator++();
```

Const correctness lets operators work on const objects and communicates whether
the operation changes the left operand.

---

## 21.6 — Overloading unary operators +, -, and !

### Unary operators have one operand

Unary `+`, unary `-`, and logical not `!` operate on a single object:

```cpp
-value
+value
!value
```

Because there is only one operand, these are commonly member functions.

```cpp
class Temperature
{
private:
    int m_degrees {};

public:
    explicit Temperature(int degrees)
        : m_degrees { degrees }
    {
    }

    Temperature operator-() const
    {
        return Temperature { -m_degrees };
    }

    bool operator!() const
    {
        return m_degrees == 0;
    }
};
```

### Unary plus

Unary plus usually returns a copy of the same value:

```cpp
Temperature operator+() const
{
    return *this;
}
```

It is rarely useful unless the type has a meaningful "positive" operation or is
mirroring built-in numeric behavior.

### Unary minus

Unary minus should produce the negated value without modifying the original:

```cpp
Temperature cold { -5 };
Temperature warm { -cold }; // +5
```

Returning by value is appropriate because this creates a new value.

### Logical not

`operator!` should answer a boolean-style question:

```cpp
class Connection
{
private:
    bool m_open {};

public:
    bool operator!() const
    {
        return !m_open;
    }
};
```

Only overload `!` when "not object" has an obvious interpretation. Otherwise,
use a named predicate:

```cpp
if (!connection)       // ok if this clearly means not open/valid
if (connection.closed()) // clearer in many domains
```

---

## 21.7 — Overloading the comparison operators

### Equality and inequality

Comparison operators should not modify either operand. They usually return
`bool`.

```cpp
class Token
{
private:
    int m_kind {};
    std::string m_text {};

public:
    Token(int kind, std::string text)
        : m_kind { kind }
        , m_text { std::move(text) }
    {
    }

    friend bool operator==(const Token& a, const Token& b)
    {
        return a.m_kind == b.m_kind && a.m_text == b.m_text;
    }

    friend bool operator!=(const Token& a, const Token& b)
    {
        return !(a == b);
    }
};
```

Implementing `!=` in terms of `==` keeps the definitions consistent.

### Ordering comparisons

Ordering should match the type's domain. For a simple value type:

```cpp
class Score
{
private:
    int m_value {};

public:
    explicit Score(int value)
        : m_value { value }
    {
    }

    friend bool operator<(const Score& a, const Score& b)
    {
        return a.m_value < b.m_value;
    }

    friend bool operator>(const Score& a, const Score& b)
    {
        return b < a;
    }

    friend bool operator<=(const Score& a, const Score& b)
    {
        return !(b < a);
    }

    friend bool operator>=(const Score& a, const Score& b)
    {
        return !(a < b);
    }
};
```

Define one primitive ordering operation and derive the rest.

### Minimize redundancy

The more comparison operators you hand-write independently, the easier it is to
make them disagree.

Bad pattern:

```cpp
operator<   // compares value
operator>   // compares absolute value by accident
operator<=  // copied from stale logic
```

Better pattern:

```cpp
== is the equality truth
<  is the ordering truth
!=, >, <=, >= are derived from those
```

### The spaceship operator

C++20 introduced `operator<=>`, the three-way comparison operator. It can express
ordering once and let the compiler generate related comparisons in many cases.

```cpp
#include <compare>

class Score
{
private:
    int m_value {};

public:
    explicit Score(int value)
        : m_value { value }
    {
    }

    auto operator<=>(const Score&) const = default;
};
```

For beginner C++ and older standards, hand-written `==` and `<` remain important
to understand. In modern codebases, expect to see `<=>` for simple value types.

CS6340 tie-in: comparison correctness matters for sorting, sets, maps, and
deduplication. A bad `operator<` can make `std::set` behave as if different
objects are the same or can make sorting unstable in surprising ways.

---

## 21.8 — Overloading the increment and decrement operators

### Prefix increment

Prefix `++x` modifies the object and returns the modified object by reference.

```cpp
class Counter
{
private:
    int m_value {};

public:
    explicit Counter(int value = 0)
        : m_value { value }
    {
    }

    Counter& operator++()
    {
        ++m_value;
        return *this;
    }

    int value() const { return m_value; }
};
```

Returning `*this` by reference enables chaining:

```cpp
+++counter;
```

This expression is unusual style, but the return convention should still match
built-in behavior.

### Postfix increment

Postfix `x++` must return the old value. C++ distinguishes postfix from prefix
by adding a dummy `int` parameter to the postfix overload.

```cpp
class Counter
{
private:
    int m_value {};

public:
    explicit Counter(int value = 0)
        : m_value { value }
    {
    }

    Counter& operator++()
    {
        ++m_value;
        return *this;
    }

    Counter operator++(int)
    {
        Counter old { *this };
        ++(*this);
        return old;
    }
};
```

The dummy parameter is not used. It is just a syntax marker.

### Decrement follows the same pattern

```cpp
Counter& operator--()
{
    --m_value;
    return *this;
}

Counter operator--(int)
{
    Counter old { *this };
    --(*this);
    return old;
}
```

### Prefix is usually cheaper

For class types, postfix often has to make a copy of the old value. Prefix can
modify and return the same object.

```cpp
++it; // prefer for iterators when old value is not needed
it++; // use when you need the previous value
```

That habit matters for iterators and other objects where copies may be more
expensive than copying an `int`.

---

## 21.9 — Overloading the subscript operator

### `operator[]` gives indexed access

The subscript operator must be a member function.

```cpp
class Scores
{
private:
    std::vector<int> m_values {};

public:
    explicit Scores(std::vector<int> values)
        : m_values { std::move(values) }
    {
    }

    int& operator[](std::size_t index)
    {
        return m_values[index];
    }
};
```

Usage:

```cpp
Scores scores { std::vector<int>{ 80, 90, 100 } };
scores[1] = 95;
```

### Return a reference for assignable access

`scores[1] = 95` works because `operator[]` returns an lvalue reference:

```cpp
int& operator[](std::size_t index);
```

If it returned by value, the assignment would target a temporary copy, not the
stored element.

```cpp
int operator[](std::size_t index); // read-only copy; cannot assign through it
```

### Const overload

Const objects need a const overload that does not allow mutation:

```cpp
class Scores
{
private:
    std::vector<int> m_values {};

public:
    int& operator[](std::size_t index)
    {
        return m_values[index];
    }

    const int& operator[](std::size_t index) const
    {
        return m_values[index];
    }
};
```

Now both of these compile with appropriate permissions:

```cpp
Scores scores { /* ... */ };
scores[0] = 10;

const Scores locked { /* ... */ };
std::cout << locked[0] << '\n';
// locked[0] = 10; // not allowed
```

### Avoid duplicate lookup logic

For more complex classes, the const and non-const overloads can duplicate logic.
One common pattern is to put bounds checking in a helper.

```cpp
void checkIndex(std::size_t index) const
{
    assert(index < m_values.size());
}

int& operator[](std::size_t index)
{
    checkIndex(index);
    return m_values[index];
}

const int& operator[](std::size_t index) const
{
    checkIndex(index);
    return m_values[index];
}
```

### Bounds checking policy

Built-in `[]` does not check bounds. Your overload can choose a policy:

| Policy | Example |
|---|---|
| unchecked | forward directly to underlying storage |
| debug assertion | `assert(index < size())` |
| exception | throw `std::out_of_range` |
| separate checked function | `at(index)` checks, `operator[]` is fast |

The standard containers use both styles: `operator[]` is unchecked for vectors,
while `.at()` checks and throws.

### Index type does not have to be integral

`operator[]` can take another type if the domain calls for it:

```cpp
class Environment
{
private:
    std::map<std::string, int> m_values {};

public:
    int& operator[](std::string_view name)
    {
        return m_values[std::string { name }];
    }
};
```

Use this sparingly. Subscript syntax should still mean "look up an element."

---

## 21.10 — Overloading the parenthesis operator

### `operator()` makes an object callable

The function-call operator must be a member function. It can take any number of
parameters.

```cpp
class AddBase
{
private:
    int m_base {};

public:
    explicit AddBase(int base)
        : m_base { base }
    {
    }

    int operator()(int value) const
    {
        return m_base + value;
    }
};

AddBase addTen { 10 };
std::cout << addTen(5) << '\n'; // 15
```

An object with `operator()` is often called a function object or functor.

### Why function objects matter

Function objects can carry state and still be called like functions:

```cpp
class AboveThreshold
{
private:
    int m_threshold {};

public:
    explicit AboveThreshold(int threshold)
        : m_threshold { threshold }
    {
    }

    bool operator()(int value) const
    {
        return value > m_threshold;
    }
};

std::vector<int> values { 2, 8, 5, 13 };
auto it { std::find_if(values.begin(), values.end(), AboveThreshold { 10 }) };
```

A lambda with captures is essentially a compiler-created function object.

### Multi-parameter call operator

`operator()` can model multidimensional access:

```cpp
class Grid
{
private:
    int m_rows {};
    int m_cols {};
    std::vector<int> m_cells {};

public:
    Grid(int rows, int cols)
        : m_rows { rows }
        , m_cols { cols }
        , m_cells(static_cast<std::size_t>(rows * cols))
    {
    }

    int& operator()(int row, int col)
    {
        return m_cells[static_cast<std::size_t>(row * m_cols + col)];
    }
};

Grid grid { 3, 4 };
grid(1, 2) = 7;
```

This avoids awkward nested dynamic arrays and gives the type control over
indexing.

### Use only when call syntax reads naturally

`object(args)` should mean "invoke this object" or "look up/evaluate using these
arguments." If the action is a command with side effects, a named member may be
clearer.

```cpp
filter(value);        // good: predicate-like object
matrix(row, col);     // good: row/column access
jobQueue(task);       // maybe unclear; enqueue(task) is clearer
```

---

## 21.11 — Overloading typecasts

### Conversion functions

A class can define how it converts to another type:

```cpp
class Cents
{
private:
    int m_cents {};

public:
    explicit Cents(int cents)
        : m_cents { cents }
    {
    }

    explicit operator int() const
    {
        return m_cents;
    }
};
```

Usage:

```cpp
Cents price { 250 };
int raw { static_cast<int>(price) };
```

Conversion operator syntax:

```cpp
operator int() const
```

There is no return type before `operator`; the target type is the return type.

### Prefer explicit conversions

Implicit conversions can make code surprising:

```cpp
void printCount(int);

Cents price { 250 };
// printCount(price); // if conversion is implicit, this might compile
```

Mark conversion operators `explicit` unless implicit conversion is clearly safe
and unsurprising.

```cpp
explicit operator int() const;
```

Then the caller has to ask:

```cpp
printCount(static_cast<int>(price));
```

### Converting constructor vs conversion operator

There are two directions:

```cpp
class Cents
{
public:
    explicit Cents(int cents); // int -> Cents
    explicit operator int() const; // Cents -> int
};
```

Use a converting constructor when the target type is your class. Use a
conversion operator when the source type is your class and the target type is
something else.

### Avoid conversion soup

Too many conversions make overload resolution hard to predict:

```cpp
class Id
{
public:
    operator int() const;
    operator std::string() const;
    operator bool() const;
};
```

That kind of type can silently flow into arithmetic, string concatenation, or
conditions. Prefer named accessors when conversion is not truly fundamental:

```cpp
id.value()
id.toString()
id.isValid()
```

### `operator bool`

Boolean conversion is common for validity checks:

```cpp
class Handle
{
private:
    void* m_ptr {};

public:
    explicit operator bool() const
    {
        return m_ptr != nullptr;
    }
};

if (handle)
{
    // handle is usable
}
```

Even here, `explicit` is important so the object does not accidentally become an
integer-like value.

---

## 21.12 — Overloading the assignment operator

### Copy constructor vs copy assignment

The copy constructor creates a new object from an existing object:

```cpp
Fraction a { 1, 2 };
Fraction b { a }; // copy constructor
```

Copy assignment replaces the contents of an already existing object:

```cpp
Fraction a { 1, 2 };
Fraction b { 3, 4 };

b = a; // copy assignment
```

Mental model:

```
copy construction:   no object there yet -> create from source
copy assignment:     object already alive -> replace its value from source
```

### Assignment must be a member

`operator=` is required to be a member function.

```cpp
class Fraction
{
private:
    int m_num {};
    int m_den { 1 };

public:
    Fraction& operator=(const Fraction& other)
    {
        m_num = other.m_num;
        m_den = other.m_den;
        return *this;
    }
};
```

Return `*this` by reference so chained assignment works:

```cpp
a = b = c;
```

The expression assigns `c` to `b`, then returns `b`, then assigns that result to
`a`.

### Self-assignment

Self-assignment means assigning an object to itself:

```cpp
value = value;
```

For classes containing only ordinary values, this is harmless:

```cpp
m_num = other.m_num;
m_den = other.m_den;
```

For resource-owning classes, self-assignment can be dangerous if the old resource
is destroyed before reading from the source object.

```cpp
Buffer& operator=(const Buffer& other)
{
    delete[] m_data;

    m_length = other.m_length;
    m_data = new int[m_length]{};

    for (int i { 0 }; i < m_length; ++i)
        m_data[i] = other.m_data[i]; // broken if this == &other

    return *this;
}
```

If `this == &other`, deleting `m_data` also deletes `other.m_data`, because they
are the same object.

### Detecting self-assignment

One direct guard is:

```cpp
Buffer& operator=(const Buffer& other)
{
    if (this == &other)
        return *this;

    delete[] m_data;

    m_length = other.m_length;
    m_data = new int[m_length]{};

    for (int i { 0 }; i < m_length; ++i)
        m_data[i] = other.m_data[i];

    return *this;
}
```

This fixes self-assignment but still has exception-safety problems. If `new`
throws after `delete[]`, the object has already lost its old data.

### Copy-and-swap idiom

The copy-and-swap idiom makes a copy first, then swaps resources:

```cpp
class Buffer
{
private:
    int* m_data {};
    int m_length {};

public:
    void swap(Buffer& other) noexcept
    {
        std::swap(m_data, other.m_data);
        std::swap(m_length, other.m_length);
    }

    Buffer& operator=(Buffer other)
    {
        swap(other);
        return *this;
    }
};
```

The parameter `other` is a copy. If copying fails, the original object has not
been changed. After the swap, `other` owns the old resource and cleans it up when
it is destroyed.

This pattern is most relevant for manual resource-owning classes. Classes built
from `std::vector`, `std::string`, and smart pointers often do not need custom
copy assignment at all.

### Implicit copy assignment

If you do not declare copy assignment, the compiler may generate one that
assigns each member. This is correct for many value-only classes:

```cpp
struct Point
{
    int x {};
    int y {};
};
```

It is often wrong for classes that own raw resources:

```cpp
class RawOwner
{
private:
    int* m_data {};
};
```

The generated assignment would copy the pointer value, not the pointed-to
dynamic array. That creates shared ownership by accident.

---

## 21.13 — Shallow vs. deep copying

### Shallow copying

A shallow copy copies member values directly. For ordinary values, that is fine:

```cpp
struct Point
{
    int x {};
    int y {};
};

Point a { 1, 2 };
Point b { a }; // x and y are copied
```

For a raw owning pointer, shallow copy copies only the address:

```cpp
class BadBuffer
{
private:
    int* m_data {};
    int m_length {};
};
```

After a shallow copy:

```
bufferA.m_data ----+
                   v
                 [ dynamic array ]
                   ^
bufferB.m_data ----+
```

Both objects appear to own the same allocation. When both destructors run, the
same memory may be deleted twice.

### Deep copying

A deep copy creates a new resource and copies the contents into it:

```
bufferA.m_data ---> [ 1 2 3 ]
bufferB.m_data ---> [ 1 2 3 ]
```

Separate arrays, same values.

```cpp
class Buffer
{
private:
    int* m_data {};
    int m_length {};

public:
    Buffer(const Buffer& other)
        : m_data { new int[other.m_length]{} }
        , m_length { other.m_length }
    {
        for (int i { 0 }; i < m_length; ++i)
            m_data[i] = other.m_data[i];
    }

    ~Buffer()
    {
        delete[] m_data;
    }
};
```

The copy constructor owns the job of creating a separate allocation.

### The rule of three

If a class needs a custom destructor, copy constructor, or copy assignment
operator, it likely needs all three.

Reason:

| Special member | Resource-owning responsibility |
|---|---|
| destructor | release owned resource |
| copy constructor | create new object with its own resource |
| copy assignment | replace existing object's resource safely |

If you write only one, the compiler-generated others may still do shallow
copies.

### Prefer the rule of zero

Modern C++ prefers designing classes so they do not need custom resource
management:

```cpp
class Buffer
{
private:
    std::vector<int> m_data {};

public:
    explicit Buffer(int length)
        : m_data(static_cast<std::size_t>(length))
    {
    }
};
```

`std::vector` already knows how to destroy, copy, assign, and move its dynamic
storage. The containing class can usually rely on generated special members.

Rule of zero:

```
No raw owning resource in your class
    -> no custom destructor
    -> no custom copy constructor
    -> no custom copy assignment
```

CS6340 tie-in: if you create helper types for lab code, prefer standard library
members (`std::vector`, `std::string`, `std::unique_ptr`) so copying behavior is
boring and correct.

---

## 21.14 — Overloading operators and function templates

### Templates assume operations exist

A function template is compiled for the concrete type it is instantiated with.
If the template body uses an operator, that operator must be valid for the
actual type.

```cpp
template <typename T>
const T& larger(const T& a, const T& b)
{
    return (a < b) ? b : a;
}
```

This works for `int` because `int` has `operator<`:

```cpp
int x { 3 };
int y { 7 };
std::cout << larger(x, y) << '\n';
```

For a custom type, the template requires that `operator<` exist:

```cpp
class Score
{
private:
    int m_value {};

public:
    explicit Score(int value)
        : m_value { value }
    {
    }

    friend bool operator<(const Score& a, const Score& b)
    {
        return a.m_value < b.m_value;
    }
};
```

Now `larger(Score{3}, Score{7})` can compile.

### Error messages point at the template body

If an operator is missing, the compiler often reports an error inside the
template:

```cpp
return (a < b) ? b : a;
//        ^ no viable operator< for T = Score
```

The real fix is usually at the type definition: either define the expected
operator or do not use that type with the template.

### Stream output and templates

A template that prints values requires `operator<<` for the concrete type:

```cpp
template <typename T>
void printLine(const T& value)
{
    std::cout << value << '\n';
}
```

Custom type support:

```cpp
class Id
{
private:
    int m_value {};

public:
    explicit Id(int value)
        : m_value { value }
    {
    }

    friend std::ostream& operator<<(std::ostream& out, const Id& id)
    {
        out << "Id(" << id.m_value << ')';
        return out;
    }
};
```

Then:

```cpp
printLine(Id { 42 });
```

### Operator overloads are part of a type's generic contract

When you overload an operator, you are saying your type supports a concept:

| Operator | Implied concept |
|---|---|
| `==` | equality comparable |
| `<` or `<=>` | orderable |
| `<<` | stream printable |
| `[]` | indexable |
| `()` | callable |
| `++` | incrementable / iterator-like |

Generic code can then rely on that concept.

### Do not overload just to satisfy a template

If `<` has no meaningful domain interpretation for a type, do not invent one
only to use a sorting or `max`-style template. Instead, pass an explicit
comparator:

```cpp
std::sort(records.begin(), records.end(), [](const Record& a, const Record& b)
{
    return a.timestamp() < b.timestamp();
});
```

This keeps the ordering local and named by context.

### Concepts in modern C++

C++20 concepts can make operator requirements explicit:

```cpp
#include <concepts>

template <typename T>
requires std::totally_ordered<T>
const T& larger(const T& a, const T& b)
{
    return (a < b) ? b : a;
}
```

The idea is the same even without concepts: templates compile only when the used
operations are available for the instantiated type.

---

## 21.x — Chapter 21 summary and quiz

### Operator form guide

| Operator family | Typical implementation |
|---|---|
| `+`, `-`, `*`, `/` | non-member; friend if private access is needed |
| `<<`, `>>` | non-member; often friend; return stream reference |
| unary `+`, `-`, `!` | member |
| `==`, `<` | non-member or defaulted member in C++20 |
| `++`, `--` | member; prefix returns reference; postfix returns old value |
| `[]` | member; return reference for mutable access |
| `()` | member; makes object callable |
| conversion operator | member; usually `explicit` |
| `=` | member; return `*this` by reference |

### Design rules

- Overload an operator only when the meaning is obvious for the type.
- Preserve normal operator expectations for mutability and return type.
- Use non-member operators for symmetric binary operations.
- Use member operators when the language requires it or when the operation
  naturally modifies/accesses the current object.
- Implement related operators in terms of a smaller core set.
- Be especially careful with raw owning pointers; shallow copying can create
  leaks, dangling pointers, and double deletes.
- Prefer standard library members so the rule of zero applies.

### Copying rules

```
value-only class:
    compiler-generated copy is usually fine

raw owning pointer class:
    destructor + copy constructor + copy assignment probably all needed

class using std::vector/std::string/smart pointers:
    generated special members are often correct
```

### CS6340 lens

Operator overloading matters most for reading C++:

```cpp
out << value;       // overloaded output
it != end;          // iterator comparison
++it;              // iterator increment
container[i];       // subscript overload
if (handle)         // explicit bool conversion
```

When debugging unfamiliar code, translate operator syntax back into function
calls:

```cpp
a + b        -> operator+(a, b) or a.operator+(b)
stream << x  -> operator<<(stream, x)
obj[index]   -> obj.operator[](index)
obj(args)    -> obj.operator()(args)
```

That translation makes overload-heavy code less magical and easier to inspect.

---

## 21.y — Chapter 21 project

LearnCpp includes a Chapter 21 project slot after the summary. Per the study
plan for these markdowns, this note does not reproduce project prompt text or
turn it into an exercise. Instead, preserve what the project slot is meant to
integrate: using operator overloads as part of a coherent class design.

### Concepts a Chapter 21 project is meant to combine

A larger operator-overloading project typically forces these ideas to work
together:

- define a class with meaningful invariants
- choose which operators are natural for that type
- implement symmetric binary operators as non-members where appropriate
- use friends only when private access is actually needed
- return stream references from `operator<<` / `operator>>`
- return `*this` by reference from assignment and prefix increment/decrement
- distinguish prefix and postfix `++` / `--`
- return references from mutable subscripting when assignment should be allowed
- avoid implicit conversions that make expressions surprising
- handle copying correctly when a class owns resources
- test expressions in combinations, not only one operator at a time

### Operator-overload integration map

```text
class invariant
    |
    v
constructors create valid objects
    |
    v
operators preserve valid objects
    |
    +-- arithmetic returns new values
    +-- assignment mutates existing object
    +-- comparison observes only
    +-- stream output observes only
    +-- subscript may observe or mutate an element
```

### Example project-shaped type

```cpp
class Counter
{
private:
    int m_value {};

public:
    explicit Counter(int value)
        : m_value { value }
    {
    }

    Counter& operator++()
    {
        ++m_value;
        return *this;
    }

    Counter operator++(int)
    {
        Counter old { *this };
        ++(*this);
        return old;
    }

    friend bool operator==(const Counter& a, const Counter& b)
    {
        return a.m_value == b.m_value;
    }

    friend std::ostream& operator<<(std::ostream& out, const Counter& counter)
    {
        out << counter.m_value;
        return out;
    }
};
```

This small type is not the LearnCpp project. It is a compact reminder of the
operator-design pressure points:

- prefix `++x` mutates and returns the current object by reference
- postfix `x++` returns the old value by value
- equality observes both operands
- stream output returns the stream so chaining still works

### How to evaluate a project solution

When reviewing a class that overloads operators, ask:

```text
Does this operator mean what C++ readers expect?
Does it preserve the object's invariant?
Does it return the conventional type?
Does it avoid unnecessary copies?
Does it work with const objects where it should?
Does it compose with other operators naturally?
```

That is the durable learning goal of the project slot.
