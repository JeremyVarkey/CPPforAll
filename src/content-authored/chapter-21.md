# Chapter 21 — Operator Overloading

> Source: <https://www.learncpp.com/> (Chapter 21)

Back in Chapter 14 you built a `Fraction` class, and it worked — but it never quite *felt* like a number. To add two fractions you wrote `a.add(b)`; to compare them you wrote `a.equals(b)`; to print one you called a helper. Compare that to `int`, where you simply write `a + b`, `a == b`, and `std::cout << a` and the code reads like the math it represents. The gap between those two experiences is not about capability — your `Fraction` could do everything `int` could — it is about *syntax*. Built-in types get the nice notation; your types get method calls.

This chapter closes that gap. **Operator overloading** lets you teach C++ what `+`, `==`, `<<`, `[]`, and many other operators *mean* for your own types, so that a `Fraction`, a `Matrix`, or a `Money` value can be written with the same clean notation as a built-in. The mechanism is surprisingly simple once you see it: an operator is just a function with an unusual name and unusual call syntax. `a + b` is shorthand for a function call, and you get to write the function.

We'll start with the core idea and its limits, then work through the operator families one at a time — arithmetic, stream I/O, comparison, increment, subscript, the function-call operator, conversions, and assignment. Two questions recur throughout, and they are worth holding in mind from the start: *which form* should an operator take (member function, friend, or plain non-member), and *what should it return* so that it behaves the way readers expect. The final two lessons — shallow versus deep copying, and how operators interact with templates — are where overloading meets the rest of C++: get them right and your types slot into the standard library as if they had always belonged there.

This is also the chapter that pays off in *reading* code. The standard library, iterators, smart pointers, and almost any large C++ codebase are saturated with overloaded operators. Once you can translate `++it`, `out << x`, and `if (handle)` back into the function calls they really are, that code stops being magic.

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

### An operator is a function in disguise

Here is the single insight that makes this entire chapter easy. When you write `2 + 3`, the compiler does not treat `+` as some special syntactic incantation — it treats it as a *call to a function* that happens to be written with infix notation. For built-in types that function is baked into the language. But the notation is general, and C++ lets you supply the function yourself when one of the operands is a type *you* defined.

```cpp
int x { 2 };
int y { 3 };

int z { x + y };  // the built-in operator+ for two ints
```

Now suppose you have a small `Point` type and want `+` to mean "add the coordinates componentwise." You write a function whose name is the keyword `operator` followed by the symbol:

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

Point p { Point{ 1, 2 } + Point{ 3, 4 } }; // p is { 4, 6 }
```

That `operator+` is an ordinary function in every respect — it has parameters, a return type, and a body. The only thing unusual is its *name* and the fact that the compiler will let you call it with infix syntax. When the compiler sees `Point{1,2} + Point{3,4}`, it looks for an `operator+` that accepts two `Point`s, finds yours, and calls it. The name itself follows a fixed pattern:

```cpp
operator+
operator==
operator<<
```

> **Key insight:** `a @ b` (for some operator `@`) is shorthand for a function call. Overloading an operator means writing that function. Everything else in this chapter is detail about *where* to write it and *what* it should return.

### Why overload operators at all?

Because well-chosen operator notation makes code read like the domain it models. When a type behaves like a value — a fraction, a vector, a duration, a money amount — operators let expressions in that type look the way they would on paper.

| Type | Operator | What a reader expects it to mean |
|---|---|---|
| `Fraction` | `+` | add two fractions |
| `Point` | `==` | same coordinates |
| `Duration` | `<` | shorter duration |
| `Matrix` | `()` | access the cell at a row/column |
| a printable type | `<<` | write the object to a stream |

The catch is the flip side: notation that *surprises* a reader is worse than no notation at all. If you overload `^` on a `Record` type, nobody reading `recordA ^ recordB` can guess what it does — merge? diff? combine logs? — because `^` carries no agreed meaning for records.

```cpp
recordA ^ recordB;          // unclear: what does ^ mean for records?
recordA.mergeWith(recordB); // a named function says exactly what it does
```

> **Best practice:** Overload an operator only when its meaning for your type is obvious and matches what built-in operators do. When the operation is really a command or has no conventional symbol, use a named function instead.

### The limits of overloading

Operator overloading is powerful but deliberately fenced in, so that the basic grammar of C++ stays stable no matter what types you define. The rules:

- You can overload only operators that **already exist**. You cannot invent a new operator token like `**`.
- You cannot change an operator's **precedence** or **associativity**. `*` will always bind tighter than `+`, for every type.
- You cannot change an operator's **arity** — the number of operands. Binary `+` stays binary; unary `-` stays unary.
- At least **one operand must be a program-defined type**. You cannot redefine `int + int`.
- A handful of operators **cannot be overloaded at all**: `?:`, `sizeof`, `::`, `.`, `.*`, and `typeid`.

The precedence rule is the one that bites people. A classic mistake is overloading `^` to mean exponentiation, because it looks like the math "to the power of." But precedence is fixed, so an expression does not parse the way the math would:

```cpp
// Tempting but wrong: overloading ^ for "to the power of".
// 2 + x ^ y would still parse as 2 + (x ^ y) using ^'s precedence —
// which is LOWER than +, not higher. The math meaning expects x^y first.
```

The notation would silently compute the wrong thing, and no amount of clever overloading can fix it, because precedence is not yours to change.

### What an operator should return

Half of writing a good operator is choosing its return type. The guiding principle is to *match what users already expect from the built-in version*, so your type composes the same way `int` does.

| Operator kind | Conventionally returns |
|---|---|
| non-mutating arithmetic (`+`, `-`, `*`) | a **new value, by value** |
| comparison (`==`, `<`) | `bool` |
| stream insertion / extraction (`<<`, `>>`) | the **stream, by reference** |
| assignment-like mutators (`=`, `+=`, prefix `++`) | the **left operand, by reference** |
| postfix `++` / `--` | the **old value, by value** |

These are not arbitrary. Returning the stream by reference is what lets `std::cout << a << b` chain. Returning the left operand by reference is what lets `a = b = c` chain. Return the wrong thing and you either break chaining or force needless copies. We'll see each case in detail as we go.

### Member, friend, or non-member?

There are three places an overloaded operator can live, and choosing among them is a recurring decision in this chapter:

| Form | How `a + b` resolves | Typical use |
|---|---|---|
| **member function** | `a.operator+(b)` | unary operators, `=`, `[]`, `()` |
| **friend** non-member | `operator+(a, b)`, with access to privates | symmetric binary ops that need private data |
| **normal** non-member | `operator+(a, b)`, using only the public API | symmetric binary ops the public API can express |

Don't worry about memorizing this table — the next several lessons exist precisely to build your intuition for it. The short version: operators where the left operand is naturally "in charge" (assignment, subscript) tend to be members; symmetric binary operators tend to be non-members; and *friend* is just the non-member option when the operator needs to reach private data.

> **Note:** You will spend far more time *reading* overloaded operators than writing them. The standard library uses them everywhere — streams, iterators, comparisons, smart pointers, containers. Even if you rarely overload an operator yourself, fluency in reading overload-heavy C++ is essential.

---

## 21.2 — Overloading the arithmetic operators using friend functions

### The shape of a symmetric binary operator

Arithmetic operators like `+`, `-`, and `*` share two properties: they take **two operands**, and they **modify neither** of them — they compute a fresh result and leave both inputs alone.

```cpp
Money total { subtotal + tax }; // subtotal and tax are unchanged; total is new
```

The complication is encapsulation. If your type hides its data behind `private`, an ordinary outside function can't reach in to do the arithmetic. The classic answer is a **friend function**: a non-member function that the class explicitly grants access to its private members. It is *written* like a free function (which keeps the operation symmetric, as we'll see) but *sees* the class internals as if it were a member.

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

Now the natural notation works:

```cpp
Cents nickel { 5 };
Cents dime { 10 };

Cents total { nickel + dime }; // total holds 15
```

When the compiler meets `nickel + dime`, it rewrites it as a plain two-argument call:

```cpp
operator+(nickel, dime)
```

Both operands take `const Cents&` — we don't intend to modify either, and passing by const reference avoids copying. The function returns a brand-new `Cents` by value, exactly as the table in 21.1 prescribes.

### Why a non-member keeps things symmetric

You might wonder why we don't just make `operator+` a member function. The answer is *symmetry*. With a non-member, the two operands are simply two arguments, neither special:

```cpp
left + right; // operator+(left, right) — both arguments treated alike
```

With a *member*, the left operand becomes the implicit object — `*this` — and only the right operand is a real parameter:

```cpp
left.operator+(right); // left must already be a Cents; right is the parameter
```

That asymmetry matters most when mixed types and implicit conversions enter the picture, because the compiler can apply conversions to a *function argument* but not to the object a member function is called on. A non-member treats both sides as arguments, so conversions can apply on either side. Keeping symmetric arithmetic as a non-member is the conventional choice for exactly this reason.

### Defining a friend inline, or out of line

A friend operator can be defined right inside the class body, as above. That is clean and convenient for short functions. For longer operators, many people prefer to *declare* the friend inside the class and *define* it afterward, which keeps the class definition readable:

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

    friend Distance operator+(const Distance& a, const Distance& b);
};

// Defined outside the class, but still a friend with private access:
Distance operator+(const Distance& a, const Distance& b)
{
    return Distance { a.m_meters + b.m_meters };
}
```

Either way the function is a non-member; the `friend` keyword only grants access, it does not make the function a member.

### Supporting mixed operands

What if you want both `Distance + int` and `int + Distance` to compile? Each operand order is a *separate* overload, because the argument types differ, so you write both — and you make the second delegate to the first so the arithmetic lives in exactly one place:

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
        return d + meters; // reuse the first overload — no duplicated arithmetic
    }
};
```

The second overload contains no arithmetic of its own; it just flips the arguments and calls the first. This "implement one, derive the other" habit is the theme of the next subsection, and it runs through the whole chapter.

### Implement operators in terms of other operators

The fewer places a piece of logic lives, the fewer places it can be wrong. So when several operators are related, define one *core* operation and express the rest through it. For arithmetic, subtraction can lean on the idea of adding a negation:

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
        return Count { a.m_value + (-b.m_value) }; // expressed via addition
    }
};
```

You'll meet the most important instance of this pattern later: implement `+=` once, then make `+` simply copy the left operand and apply `+=`. That keeps the two operators guaranteed-consistent.

> **Best practice:** When operators are related, implement a small core set directly and define the rest in terms of them. Duplicated logic is duplicated bugs.

### Don't overload arithmetic without a domain meaning

Arithmetic overloads earn their keep when the type genuinely behaves like a value you'd add or subtract. They become traps when the "operation" is really a command:

```cpp
jobA + jobB; // merge? schedule both? concatenate their logs? nobody can tell
```

If a reader cannot predict what `+` does from the types alone, reach for a named function. The notation should *clarify*, never obscure.

---

## 21.3 — Overloading operators using normal functions

### Non-member does not have to mean friend

In the last lesson we used `friend` because the operator needed private data. But friendship is a real concession — it punches a hole in your encapsulation — so don't reach for it reflexively. If the operator can do its job entirely through the class's **public interface**, make it a plain non-member function with no special access at all:

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

    int cents() const { return m_cents; } // public accessor
};

// A normal non-member: no friendship needed, uses only the public API.
Cents operator+(const Cents& left, const Cents& right)
{
    return Cents { left.cents() + right.cents() };
}
```

This version is identical in usage to the friend version, but it asks for *less* — it touches no private member, so it cannot accidentally depend on or corrupt your internals. Stronger encapsulation for the same result.

### Choosing between friend and normal

So when should an operator be a friend rather than a plain non-member? A short decision guide:

| Question | Lean toward |
|---|---|
| Can the operator be written cleanly through the public API? | normal non-member |
| Would exposing public getters *just* for this operator be awkward? | friend |
| Is the operator required by the language to be a member? | member |

The principle is least privilege: prefer the form that needs the least access. A normal non-member needs none. A friend needs private access but stays a non-member (preserving symmetry). A member is for the cases where the language insists or where the left operand is genuinely the object being acted on.

> **Best practice:** Prefer a normal non-member operator when the public interface is enough. Use `friend` only when the operator genuinely needs private access. Friendship is a deliberate exception to your access rules, not a default.

### Keep operators close to their type

Whether friend or normal, a non-member operator should live in the **same header/source and the same namespace** as the type it serves:

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

There are two reasons. The practical one is **argument-dependent lookup (ADL)**: when you write `a + b` for two `units::Meters`, the compiler automatically searches the `units` namespace for a matching `operator+`. Put the operator beside the type and it is found without any qualification. The human reason is just as important — anyone reading the type will find its operators right there, not scattered across the codebase.

### Don't add operators to types you don't own

It is technically possible to write some overloads involving standard-library types, but it is almost always a bad idea to invent new operator meanings for types you didn't define:

```cpp
// Bad idea: this would give EVERY std::string a surprising new operator
// std::string operator*(int count, const std::string& text);
```

Other code may rely on `std::string` *not* having that operator, and your definition becomes a surprising piece of action-at-a-distance. Keep such helpers as named functions in your own namespace, where they belong to you and surprise no one.

---

## 21.4 — Overloading the I/O operators

### Output with `operator<<`

You have written `std::cout << x` since Chapter 1. Now you'll learn what it actually is — and how to make your own types printable with the same notation.

The stream insertion operator `<<` is almost always a **non-member**, and usually a **friend**. The reason is structural: in `std::cout << point`, the *left* operand is a `std::ostream`, not your class. A member `operator<<` would have to live in `std::ostream` — which you can't modify — and would be called backwards. So `<<` is a free function taking the stream on the left and your object on the right:

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

Two details carry the design. The stream parameter is `std::ostream&` — a non-const reference, because writing to a stream *changes* it. And the function **returns that same stream by reference**, which is what enables chaining:

```cpp
Point p { 2, 5 };
std::cout << "p = " << p << '\n';
```

Here is why the return matters. The expression `<<` is left-associative, so the line above is really a chain of calls, each handing the stream to the next:

```text
std::cout << "p = "  -> returns std::cout
          << p       -> returns std::cout
          << '\n'    -> returns std::cout
```

If `operator<<` returned `void`, the second `<<` would have nothing to attach to and the chain would not compile. Returning the stream by reference is the mechanism behind every chained print you have ever written.

### Input with `operator>>`

The extraction operator `>>` is the mirror image: it *reads* from a stream into an object. Like `<<`, it is a non-member friend that returns the stream by reference, so input can chain (`std::cin >> a >> b`) and so callers can test the stream's state.

```cpp
#include <iostream>

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

Two differences from output. The stream is now `std::istream&`, and crucially the *object* parameter is a **non-const** `Point&` — extraction modifies the object, so it cannot be const.

### Don't half-fill an object on failure

The version above already does the right thing, and it is worth being explicit about why: we read into local `x` and `y` *first*, and only copy them into `point` if **both** reads succeeded (the `if (in >> x >> y)` test). If we had instead written straight into `point.m_x` and then the read for `m_y` failed, the object would be left half-updated — corrupted with a new x but an old y.

You can make that all-or-nothing intent even clearer by building a fresh value and assigning it in one step:

```cpp
friend std::istream& operator>>(std::istream& in, Point& point)
{
    int x {};
    int y {};

    if (in >> x >> y)
        point = Point { x, y }; // replace wholesale, only on full success

    return in;
}
```

> **Best practice:** Extract into temporaries first, then commit to the object only when the whole read succeeds. Never leave an object partially overwritten by a failed extraction.

### When input is valid syntax but invalid meaning

A stream read can succeed *syntactically* — the right tokens were there — and still produce values that violate your class's invariant. Consider reading a `Fraction` in the form `num/den`. The stream might happily hand you a denominator of zero, which the type must never hold. The fix is to validate, and to set the stream's failure state yourself when the values are unacceptable:

```cpp
#include <iostream>
#include <cassert>

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
            in.setstate(std::ios::failbit); // mark the stream as failed

        return in;
    }
};
```

Calling `in.setstate(std::ios::failbit)` puts the stream into a failed state, which means the caller's ordinary error handling — `if (std::cin >> fraction)` — catches the bad input automatically. You don't invent a new error channel; you use the one the stream already provides.

> **Note:** The extraction operator here is a *preview* of Chapter 28's I/O material. For this chapter, the takeaway is the shape: read into temporaries, validate, and signal failure through the stream's own state.

### Formatting is part of the contract

The representation your operators read and write is an interface, so choose it deliberately and keep it stable. For output, pick one clear format:

```cpp
std::cout << Point{ 2, 5 }; // always "(2, 5)"
```

For input, document and enforce the format you accept:

```cpp
std::cin >> fraction; // expects exactly "num/den", e.g. 3/4
```

Resist the urge to make extraction guess among many formats. A permissive parser is harder to reason about and easier to fool; a strict one fails loudly and predictably.

---

## 21.5 — Overloading operators using member functions

### The shape of a member operator

We've leaned on non-members for symmetric operators. Now let's see the *member* form, which is the right choice for a different family of operators. In a member operator, the left operand isn't a parameter at all — it *is* the object the function is called on, available as `*this`:

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

The compiler rewrites the infix expression as a member call:

```cpp
a + b;            // becomes...
a.operator+(b);   // 'a' is the implicit object (*this); 'b' is 'other'
```

So a member binary operator has only *one* explicit parameter — the right operand — because the left operand is implied. Notice this `operator+` does not need `friend`: as a member it already has access to private data, including the `other.m_cents` of the same class.

### Operators that must be members

Some operators are *required* by the language to be member functions — they simply cannot be written as non-members:

| Operator | Meaning |
|---|---|
| `operator=` | assignment |
| `operator[]` | subscript |
| `operator()` | function call |
| `operator->` | member access through a pointer-like type |

There is a logic to the list: each of these has the object itself as an inseparable left operand, so the language ties them to the object. We'll cover `=`, `[]`, and `()` in their own lessons.

### The cost of being a member: an asymmetric left operand

The trade-off with a member operator is exactly the one we flagged in 21.2 — the left operand must already be an object of your class, and conversions cannot rescue it:

```cpp
class Distance
{
public:
    Distance operator+(int meters) const;
};

Distance d { 10 };
d + 5;  // OK: d is a Distance, 5 becomes the int parameter
// 5 + d;  // ERROR: left operand 5 is an int — there is no int::operator+(Distance)
```

`d + 5` works because `d` is the implicit object and `5` matches the parameter. But `5 + d` cannot find a member operator, because `5` is an `int` and `int` has no member that adds a `Distance`. If you need both operand orders to work, that is a sign to use a non-member (as in 21.2), where conversions can apply to either argument.

> **Key insight:** A member operator makes its left operand special — it must already be your type, with no implicit conversion. For binary operators that should work symmetrically (including mixed-type expressions), prefer a non-member.

### Const-correctness in operators

The `const`-correctness rules you learned for member functions apply directly to member operators, and they carry real meaning. An operator that does not modify the object should be a `const` member:

```cpp
bool isZero() const;
Cents operator-() const;                 // negation: reads, doesn't change *this
Cents operator+(const Cents& other) const;
```

An operator that *does* modify the object is non-const:

```cpp
Cents& operator+=(const Cents& other);   // changes *this
Cents& operator++();                      // changes *this
```

Marking the non-mutating ones `const` does two jobs at once. It lets the operator be used on `const` objects (you couldn't add two `const Cents` otherwise), and it documents, in a way the compiler enforces, whether the operation alters the left operand.

---

## 21.6 — Overloading unary operators +, -, and !

### One operand, so naturally a member

Unary operators act on a *single* operand: unary plus, unary minus, and logical NOT.

```cpp
-value
+value
!value
```

Because there is only one operand — and it is the object itself — these are most naturally written as **member functions**, with the operand available as `*this` and no explicit parameters at all.

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

    Temperature operator-() const   // unary minus: return the negation
    {
        return Temperature { -m_degrees };
    }

    bool operator!() const          // logical NOT: a boolean-style question
    {
        return m_degrees == 0;
    }
};
```

Note that these take *no* parameters: `operator-()` is the unary minus, and the object it negates is `*this`. (Binary minus, `operator-(const Temperature&)`, would be a separate function with one parameter.)

### Unary minus

Unary minus should produce the negated value and leave the original untouched — which is why it is `const` and returns a new value by value:

```cpp
Temperature cold { -5 };
Temperature warm { -cold }; // warm holds +5; cold is still -5
```

Returning by value is correct here precisely *because* you are creating a new value, not modifying the existing one.

### Unary plus

Unary plus, by convention, returns a copy of the value unchanged:

```cpp
Temperature operator+() const
{
    return *this;
}
```

It is rarely worth defining. It only earns its place when your type mirrors built-in numeric behavior closely enough that a reader would expect `+x` to be legal, or when it has some genuine "make positive" meaning.

### Logical NOT

`operator!` should answer a yes/no question about the object — typically "is this empty, zero, closed, or invalid?":

```cpp
class Connection
{
private:
    bool m_open {};

public:
    bool operator!() const
    {
        return !m_open; // true when the connection is NOT open
    }
};
```

As always, overload it only when "not this object" has an obvious reading. If it doesn't, a named predicate communicates far better:

```cpp
if (!connection)         // fine only if this clearly means "not open / not valid"
if (connection.closed()) // unambiguous in almost any domain
```

> **Best practice:** Overload unary `!` only when the negation of your object has a single obvious meaning. Otherwise a named predicate like `.closed()` or `.empty()` reads more clearly.

---

## 21.7 — Overloading the comparison operators

### Equality and inequality

Comparison operators observe their operands and report a verdict — they modify nothing and return `bool`. Equality is the natural starting point:

```cpp
#include <string>
#include <utility>

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
        return !(a == b); // derived from operator==, never independent
    }
};
```

Notice `operator!=` does not re-derive its own notion of inequality — it is *defined as* the negation of `operator==`. This guarantees the two can never disagree: there is exactly one definition of "equal," and "not equal" is its logical opposite by construction.

### Ordering comparisons

For ordering, the same minimize-redundancy principle is even more valuable, because there are four ordering operators (`<`, `>`, `<=`, `>=`) and keeping them consistent by hand is error-prone. Define one *primitive* — conventionally `operator<` — and build the other three from it:

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
        return a.m_value < b.m_value; // the single source of ordering truth
    }

    friend bool operator>(const Score& a, const Score& b)  { return b < a; }
    friend bool operator<=(const Score& a, const Score& b) { return !(b < a); }
    friend bool operator>=(const Score& a, const Score& b) { return !(a < b); }
};
```

Read those derivations slowly, because they are exactly correct: `a > b` is the same as `b < a`; `a <= b` means "not `b < a`"; `a >= b` means "not `a < b`." Every one bottoms out in the single `operator<`. Change the ordering rule once, and all four stay consistent automatically.

### Why minimize redundancy

The danger of writing each comparison independently is *drift*. Each hand-written operator is a chance to encode slightly different logic, and the bugs that result are nasty because the operators *look* like they agree:

```text
operator<   // compares m_value          -- correct
operator>   // compares |m_value|         -- a typo nobody notices
operator<=  // copied from an old version -- now stale
```

Now `a < b`, `a > b`, and `a <= b` can give mutually contradictory answers, and code that relies on them — sorting, sets, maps — behaves bizarrely. Deriving everything from one primitive makes that class of bug impossible.

> **Best practice:** Define `==` as your equality truth and `<` as your ordering truth, then derive `!=`, `>`, `<=`, and `>=` from those two. Independent hand-written comparisons drift apart.

### The spaceship operator (a glimpse of C++20)

C++20 introduced `operator<=>`, the **three-way comparison operator**, nicknamed the spaceship. It lets you express ordering *once* and have the compiler synthesize the related comparisons for you. For a simple value type, you can often just default it:

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

    auto operator<=>(const Score&) const = default; // compiler derives <, <=, >, >=
};
```

This course targets C++17, so the hand-written `==` and `<` you saw above remain the techniques to know — and understanding them is exactly what makes `<=>` make sense. But you'll meet `<=>` constantly in modern codebases, so it's worth recognizing.

> **Note:** A wrong `operator<` does quiet damage. `std::set`, `std::map`, and `std::sort` all rely on it; if it doesn't impose a consistent order, a set may treat distinct objects as duplicates, or a sort may produce nonsense. Comparison correctness is not a nicety — containers depend on it.

---

## 21.8 — Overloading the increment and decrement operators

### Two operators, one symbol

`++` and `--` are unusual: each symbol names *two* different operators — a **prefix** form (`++x`) and a **postfix** form (`x++`) — that return different things. Getting both right, and distinguishing them, is the whole content of this lesson.

### Prefix increment

Prefix `++x` does the increment and then yields the *updated* object. So it modifies `*this` and returns `*this` **by reference**:

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

    Counter& operator++()   // prefix: no parameter
    {
        ++m_value;
        return *this;       // return the modified object, by reference
    }

    int value() const { return m_value; }
};
```

Returning `*this` by reference matches the built-in behavior — for an `int`, `++x` is an lvalue you can keep operating on — and it is what makes the result chainable and assignable.

### Postfix increment, and the dummy parameter

Postfix `x++` is different: it must return the value as it was *before* the increment. But both forms are spelled `operator++`, so how does the compiler tell them apart? By a deliberate piece of syntax: the **postfix overload takes an extra, unused `int` parameter**. That parameter is never read; it exists only as a marker that says "this is the postfix one."

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

    Counter& operator++()       // prefix
    {
        ++m_value;
        return *this;
    }

    Counter operator++(int)     // postfix: the int marks it; returns by VALUE
    {
        Counter old { *this };  // 1. save the current value
        ++(*this);              // 2. reuse prefix++ to do the increment
        return old;             // 3. return the saved (pre-increment) value
    }
};
```

Read the postfix body as three steps: **copy** the current state into `old`, **increment** `*this` (by delegating to prefix `++`, so the increment logic lives in one place), then **return the copy**. The result is the value from *before* the increment — exactly what `x++` promises — while `x` itself has advanced.

Notice two things. The postfix version returns `Counter` **by value**, not by reference: it must hand back the old state, which no longer exists in `*this`. And it delegates the actual increment to prefix `++`, so there is one definition of "increment by one."

### Decrement is the same pattern

Decrement mirrors increment exactly — same two forms, same dummy-`int` trick, same return conventions:

```cpp
Counter& operator--()   // prefix
{
    --m_value;
    return *this;
}

Counter operator--(int) // postfix
{
    Counter old { *this };
    --(*this);
    return old;
}
```

### Prefer prefix when you don't need the old value

Here is a habit worth forming. Postfix `++` has to make a *copy* of the old value to return it; prefix `++` does not — it just modifies and returns a reference. For an `int` that copy is free, so nobody cares. But for a `Counter`, an iterator, or any heavier type, that copy has a cost:

```cpp
++it; // prefer this when you don't need the previous value
it++; // use only when you genuinely need the value from before
```

That is why idiomatic C++ loops are written `for (auto it = v.begin(); it != v.end(); ++it)` — prefix increment, because the old iterator value is thrown away anyway and there's no reason to pay for a copy of it.

> **Best practice:** Use prefix `++`/`--` by default. Reach for postfix only when you actually need the pre-increment value, since postfix must copy it.

---

## 21.9 — Overloading the subscript operator

### Indexed access with `operator[]`

The subscript operator gives your type the familiar `container[index]` notation. It **must be a member function**, and in its simplest form it forwards an index to underlying storage:

```cpp
#include <vector>
#include <utility>

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

Usage looks exactly like indexing a vector:

```cpp
Scores scores { std::vector<int>{ 80, 90, 100 } };
scores[1] = 95; // works because operator[] returns a reference — see below
```

### Return a reference so the result is assignable

Look closely at that return type: `int&`, a reference, not `int`. This is the crux of subscripting. The whole point of `scores[1] = 95` is that the subscript expression names the *actual stored element*, so assigning to it changes the container. A reference return does exactly that — `scores[1]` *is* the element.

If `operator[]` returned a plain `int` (a copy), then `scores[1]` would be a temporary value, and `scores[1] = 95` would assign to that throwaway temporary and vanish — in fact the compiler would reject it:

```cpp
int  operator[](std::size_t index); // read-only: returns a COPY, can't assign through it
int& operator[](std::size_t index); // read-write: returns the element itself
```

> **Key insight:** Returning a reference from `operator[]` is what makes `obj[i] = value` work. The reference *is* the stored element; a by-value return would be a disconnected copy.

### A const overload for const objects

A single non-const `operator[]` can't be called on a `const` object — and it shouldn't be, since it hands out a mutable reference. So you typically provide *two* overloads: a non-const one returning `int&` for mutable objects, and a const one returning `const int&` for const objects:

```cpp
#include <vector>

class Scores
{
private:
    std::vector<int> m_values {};

public:
    int& operator[](std::size_t index)
    {
        return m_values[index];
    }

    const int& operator[](std::size_t index) const // note: const member, const return
    {
        return m_values[index];
    }
};
```

The compiler picks the right overload based on whether the object is const:

```cpp
Scores scores { /* ... */ };
scores[0] = 10;                    // non-const object -> non-const operator[], OK

const Scores locked { /* ... */ };
std::cout << locked[0] << '\n';    // const object -> const operator[], read OK
// locked[0] = 10;                 // ERROR: const operator[] returns const int&
```

### Don't duplicate the lookup logic

For a trivial body the two overloads barely differ, but in a real container the index math or bounds checking can be substantial — and duplicating it across both overloads invites them to drift apart. A simple remedy is to factor the shared part into a helper:

```cpp
#include <cassert>

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

### Choosing a bounds-checking policy

The built-in `[]` does no bounds checking, and your overload gets to decide its own policy. The standard library itself offers a useful model: `std::vector::operator[]` is unchecked and fast, while `std::vector::at()` checks and throws `std::out_of_range`. Your options:

| Policy | What it does |
|---|---|
| unchecked | forward straight to storage; fastest, undefined behavior on a bad index |
| debug assertion | `assert(index < size())`; catches bugs in debug builds, free in release |
| exception | throw `std::out_of_range` on a bad index |
| split it | fast `operator[]` plus a checked `at()`, like the standard containers |

Pick consciously. There is no single right answer — only a right answer for your type's intended use.

### The index doesn't have to be an integer

`operator[]` takes one parameter, but nothing requires it to be a number. If your type is a lookup table, a string key is perfectly natural:

```cpp
#include <map>
#include <string>
#include <string_view>

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

Use this judgment sparingly: subscript syntax should still *mean* "look up an element." When `env["PATH"]` reads as a lookup, it's a good fit; when `[]` would mean something surprising, use a named function instead.

---

## 21.10 — Overloading the parenthesis operator

### Making an object callable with `operator()`

The function-call operator `()` lets you call an *object* as if it were a function — `obj(args)`. Like subscript, it **must be a member function**, and unlike every other operator it can take **any number of parameters**:

```cpp
#include <iostream>

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

int main()
{
    AddBase addTen { 10 };
    std::cout << addTen(5) << '\n'; // calls addTen.operator()(5) -> 15
    return 0;
}
```

An object that defines `operator()` is called a **function object**, or **functor**. The expression `addTen(5)` looks like a function call but is really `addTen.operator()(5)` — a member call on the object.

### Why function objects matter: callable state

A plain function can't remember anything between calls. A function object can, because it's an object — it can *carry state* in its members and use that state every time it's called:

```cpp
#include <algorithm>
#include <vector>

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
        return value > m_threshold; // uses the stored threshold
    }
};

int main()
{
    std::vector<int> values { 2, 8, 5, 13 };
    // The functor remembers its threshold of 10 across every call std::find_if makes:
    auto it { std::find_if(values.begin(), values.end(), AboveThreshold { 10 }) };
    return 0;
}
```

This is the deep connection back to Chapter 20: a **lambda with captures is essentially a function object the compiler writes for you**. When you capture a variable in a lambda, the compiler generates a hidden class with that variable as a member and an `operator()` holding the body — exactly the `AboveThreshold` pattern above. Now you know what a capturing lambda *is* under the hood.

### A multi-parameter call operator

Because `operator()` accepts any number of arguments, it is the idiomatic way to give a type multi-dimensional indexing — something `operator[]`, with its single parameter, cannot do cleanly:

```cpp
#include <vector>

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

int main()
{
    Grid grid { 3, 4 };
    grid(1, 2) = 7; // row 1, column 2 — clean two-argument indexing
    return 0;
}
```

The grid stores its cells in a single flat vector and maps `(row, col)` to a linear index. The `operator()` returns a reference, so `grid(1, 2) = 7` writes through to storage — the same reference-return idea you saw with subscript.

### Use it only when call syntax reads naturally

`object(args)` should read as "invoke this object" or "evaluate/look up using these arguments." It is perfect for predicates and for indexing-style access. It is a poor fit when the action is really a command with side effects, where a named member states the intent better:

```cpp
filter(value);    // good: a predicate object answering a question
matrix(row, col); // good: row/column access
jobQueue(task);   // unclear — enqueue(task) says what it does
```

---

## 21.11 — Overloading typecasts

### Teaching your type how to convert

Back in Chapter 14 you saw the *converting constructor*, which teaches the language how to make your type *from* another (`int` → `Cents`). A **conversion operator** is the other direction: it teaches the language how to turn your type *into* another (`Cents` → `int`).

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

    explicit operator int() const // convert a Cents to an int
    {
        return m_cents;
    }
};
```

The syntax is unusual, so look at it carefully. The function is named `operator int`, and there is **no return type written before `operator`** — the target type *is* the return type. It takes no parameters and is `const`, because converting reads the object without changing it.

```cpp
Cents price { 250 };
int raw { static_cast<int>(price) }; // raw is 250
```

### Prefer explicit conversions

Notice the `explicit` keyword on `operator int` above. Without it, the conversion would be *implicit* — the compiler would silently insert it wherever an `int` is expected and a `Cents` is supplied. That sounds convenient, and it is exactly the problem:

```cpp
void printCount(int);

Cents price { 250 };
// printCount(price); // if the conversion were implicit, this would silently compile —
//                    // passing a money amount where a count was expected
```

Marking the conversion `explicit` forces the caller to *ask* for it, which both prevents accidental conversions and documents intent at the call site:

```cpp
printCount(static_cast<int>(price)); // the conversion is now visible and deliberate
```

> **Best practice:** Make conversion operators `explicit` unless implicit conversion is genuinely safe and unsurprising. An implicit conversion that fires where you didn't intend it is a classic source of baffling bugs.

### Two directions, two tools

It's worth seeing both conversion directions side by side, because they are easy to confuse:

```cpp
class Cents
{
public:
    explicit Cents(int cents);     // int  -> Cents  (converting constructor)
    explicit operator int() const; // Cents -> int   (conversion operator)
};
```

The rule of thumb: use a **converting constructor** when the *target* of the conversion is your class, and a **conversion operator** when the *source* is your class and the target is some other type.

### Avoid conversion soup

Resist defining many conversions on one type. Each one is another path the compiler can take during overload resolution, and a type with several can flow silently into arithmetic, string operations, and conditions in ways nobody intended:

```cpp
class Id
{
public:
    operator int() const;
    operator std::string() const;
    operator bool() const;
};
```

An `Id` like that can accidentally be added, concatenated, or used as a condition, and good luck predicting which conversion fires where. When a conversion isn't truly fundamental to the type, prefer named accessors that say exactly what they return:

```cpp
id.value();    // instead of operator int
id.toString(); // instead of operator std::string
id.isValid();  // instead of operator bool
```

### The one conversion worth its weight: `operator bool`

The most common and most defensible conversion operator is to `bool`, for types that have a natural notion of "valid" or "usable" — a handle, a smart pointer, an optional-like wrapper. It lets the object be tested directly in a condition:

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

int main()
{
    Handle handle { /* ... */ };
    if (handle) // explicit operator bool is still allowed in a condition
    {
        // handle is usable
    }
    return 0;
}
```

The `explicit` here is important, and a little subtle: `explicit operator bool` is still usable in conditions like `if`, `while`, and `&&`/`||` (the language carves out an exception for boolean contexts), but it *won't* silently turn into an integer in arithmetic. That is exactly the behavior you want — testable as a truth value, not accidentally a number. This is precisely how `std::optional`, `std::unique_ptr`, and stream objects let you write `if (ptr)` and `if (std::cin >> x)`.

---

## 21.12 — Overloading the assignment operator

### Copy construction versus copy assignment

Two operations look similar but happen at different moments in an object's life, and telling them apart is the foundation of this lesson.

The **copy constructor** builds a *brand-new* object as a copy of an existing one. There was no object before; one comes into being:

```cpp
Fraction a { 1, 2 };
Fraction b { a };   // copy CONSTRUCTOR: b is created as a copy of a
```

**Copy assignment** replaces the contents of an object that *already exists*:

```cpp
Fraction a { 1, 2 };
Fraction b { 3, 4 };

b = a;              // copy ASSIGNMENT: b already exists; its value is replaced
```

The distinction in one line:

```text
copy construction:  no object yet      -> create a new one from the source
copy assignment:    object already alive -> overwrite its value from the source
```

### Assignment must be a member, and returns `*this`

`operator=` is one of the operators the language *requires* to be a member function. A straightforward version copies each member across:

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
        return *this;        // return the assigned-to object, by reference
    }
};
```

The return type is `Fraction&`, and the function returns `*this`. This mirrors the built-in `=`, where assignment is an expression that yields the assigned-to object — and it is what makes chained assignment work:

```cpp
a = b = c; // evaluates as a = (b = c): c is assigned to b, then b is returned and assigned to a
```

If `operator=` returned `void`, `a = b = c` would not compile, because the inner `b = c` would have no value to feed the outer assignment.

### The self-assignment hazard

What happens when an object is assigned to itself — `value = value`? For a class made only of plain values, nothing bad: copying `m_num` onto itself and `m_den` onto itself is harmless. But for a class that **owns a resource** through a raw pointer, naive self-assignment can be catastrophic:

```cpp
class Buffer
{
private:
    int* m_data {};
    int m_length {};

public:
    Buffer& operator=(const Buffer& other)
    {
        delete[] m_data;                  // free our old array

        m_length = other.m_length;
        m_data = new int[m_length]{};

        for (int i { 0 }; i < m_length; ++i)
            m_data[i] = other.m_data[i];  // DISASTER if this == &other

        return *this;
    }
};
```

Trace `b = b`. The `delete[] m_data` frees the array — but if `this == &other`, then `other.m_data` *was that same array*. The subsequent loop reads `other.m_data[i]` from freed memory. The object destroys the very data it is about to copy from.

### Guarding against self-assignment

The most direct fix is to detect self-assignment by comparing addresses and bail out early:

```cpp
Buffer& operator=(const Buffer& other)
{
    if (this == &other) // are we assigning to ourselves?
        return *this;   // if so, there is nothing to do

    delete[] m_data;

    m_length = other.m_length;
    m_data = new int[m_length]{};

    for (int i { 0 }; i < m_length; ++i)
        m_data[i] = other.m_data[i];

    return *this;
}
```

This cures the self-assignment bug, but it still has a subtler flaw. If `new` throws *after* the `delete[]`, the object has already lost its old data and now has none — it's left in a broken state. We can do better.

### The copy-and-swap idiom

A more robust approach is **copy-and-swap**: make the copy *first*, and only after it succeeds do you touch the current object. The trick is to take the parameter **by value** (so the copy happens automatically as the argument is passed) and then swap the new copy's resources with your own:

```cpp
#include <utility> // std::swap

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

    Buffer& operator=(Buffer other) // NOTE: by value — 'other' is already a copy
    {
        swap(other);   // take the copy's resources; 'other' takes our old ones
        return *this;
    }
};
```

This is elegant once you see it. The parameter `other` is a fresh copy of the right-hand side, made *before* the function body runs — so if copying throws, it throws at the call boundary and your object is untouched. Inside, `swap` exchanges the two objects' resources: `*this` ends up with the new data, and `other` ends up with our *old* data, which it cleans up automatically when it goes out of scope at the end of the function. Self-assignment is handled for free — assigning to yourself just copies, swaps with the copy, and destroys the copy. No address check needed.

> **Note:** Copy-and-swap matters for classes that manage a raw resource by hand. Classes built from `std::vector`, `std::string`, and smart pointers usually need no custom assignment at all — their members already do the right thing. This is the rule of zero, which the next lesson develops.

### When the compiler writes assignment for you

If you don't declare a copy-assignment operator, the compiler generates one that assigns each member in turn. For a class of plain values, that generated version is exactly right:

```cpp
struct Point
{
    int x {};
    int y {}; // compiler-generated operator= assigns x and y — perfect
};
```

But for a class holding a raw owning pointer, the generated version is *wrong*:

```cpp
class RawOwner
{
private:
    int* m_data {}; // generated operator= copies the POINTER, not what it points to
};
```

Member-wise assignment copies the pointer *value* — the address — so two objects end up pointing at the same array. Now they share ownership by accident, and when both destructors run, the same memory is freed twice. That failure mode is the subject of the next lesson.

---

## 21.13 — Shallow vs. deep copying

### Shallow copying

A **shallow copy** copies each member's value directly, field by field. For ordinary values, that is precisely what you want:

```cpp
struct Point
{
    int x {};
    int y {};
};

Point a { 1, 2 };
Point b { a }; // x and y are copied — two independent, correct points
```

The trouble begins when a member is a **raw pointer that owns a resource**. A shallow copy duplicates the *pointer* — the address — not the thing it points to:

```cpp
class BadBuffer
{
private:
    int* m_data {};   // owns a dynamic array
    int m_length {};
};
```

After a shallow copy of a `BadBuffer`, both objects hold the *same* address, pointing at the *same* array:

```text
bufferA.m_data ----+
                   v
                 [ dynamic array ]
                   ^
bufferB.m_data ----+
```

This is a time bomb. The two objects look independent, but they share one allocation. When both destructors run, that single array is `delete[]`'d twice — a double-free, which is undefined behavior. Modify one and the other changes too. Destroy one and the other dangles.

### Deep copying

A **deep copy** is the fix: instead of copying the pointer, allocate a *new* resource and copy the *contents* into it. Now each object owns its own array:

```text
bufferA.m_data ---> [ 1 2 3 ]
bufferB.m_data ---> [ 1 2 3 ]   (a separate array, same values)
```

You implement deep copying in the special member functions — here, the copy constructor:

```cpp
class Buffer
{
private:
    int* m_data {};
    int m_length {};

public:
    Buffer(const Buffer& other)
        : m_data { new int[other.m_length]{} } // allocate a SEPARATE array
        , m_length { other.m_length }
    {
        for (int i { 0 }; i < m_length; ++i)
            m_data[i] = other.m_data[i];       // copy the contents into it
    }

    ~Buffer()
    {
        delete[] m_data;
    }
};
```

The copy constructor takes on the job of giving the new object its own allocation, so the two buffers never share storage.

### The rule of three

There's a pattern lurking here. A class that owns a raw resource needs *three* coordinated special members, and the rule of three names it:

> If a class needs a custom **destructor**, **copy constructor**, or **copy assignment operator**, it almost certainly needs all three.

The reason is that each handles one moment in the resource's life:

| Special member | Its responsibility |
|---|---|
| destructor | release the owned resource |
| copy constructor | create a new object with *its own* resource |
| copy assignment | replace an existing object's resource safely |

The trap is that writing only *one* leaves the other two as compiler-generated *shallow* copies — so you carefully deep-copy in the constructor, then the generated assignment operator quietly shares a pointer and double-frees anyway. They come as a set.

### Prefer the rule of zero

Here's the better news, and the direction modern C++ pulls you. The cleanest way to handle resource management is to *not write any of those three functions* — by building your class out of members that already manage themselves:

```cpp
#include <vector>

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

`std::vector` already knows how to destroy, copy, assign, and move its storage correctly. Because this `Buffer` holds a vector rather than a raw `int*`, the compiler-generated destructor, copy constructor, and copy assignment all do the right thing automatically — they delegate to the vector's. You write *zero* special members.

```text
no raw owning resource in your class
    -> no custom destructor
    -> no custom copy constructor
    -> no custom copy assignment
    = the rule of zero
```

> **Best practice:** Reach for the rule of zero first. Build classes from `std::vector`, `std::string`, and smart pointers so copying and destruction are correct by default. Hand-write the rule of three only when you truly must manage a raw resource — which, in modern C++, is rare.

---

## 21.14 — Overloading operators and function templates

### Templates assume the operations they use exist

A function template is a *recipe*; the compiler bakes a concrete function from it for each type you instantiate it with. If the recipe uses an operator, that operator must be valid for the actual type — and the compiler only finds out when it tries to instantiate. Consider a `larger` template:

```cpp
template <typename T>
const T& larger(const T& a, const T& b)
{
    return (a < b) ? b : a; // requires operator< for T
}
```

For `int` this compiles, because `int` has a built-in `operator<`:

```cpp
int x { 3 };
int y { 7 };
std::cout << larger(x, y) << '\n'; // fine: int < int exists
```

For one of *your* types, the template instantiates only if that type provides the operator it needs — here, `operator<`:

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

With that `operator<` in place, `larger(Score{ 3 }, Score{ 7 })` compiles and works. Without it, it wouldn't — and the error appears not at your call but inside the template body, at the line that uses `<`. That's worth being ready for: when a template fails to compile for your type, the message often points *into* the template, but the real fix is at *your* type — supply the operator the template expects, or don't use that type with the template.

### Stream output through a template

The same principle covers printing. A template that streams its argument needs `operator<<` to exist for the concrete type:

```cpp
template <typename T>
void printLine(const T& value)
{
    std::cout << value << '\n'; // requires operator<< for T
}
```

Give your type an `operator<<` and it becomes printable through any such template:

```cpp
#include <iostream>

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

// ...
printLine(Id{ 42 }); // prints: Id(42)
```

### Operators are how your type joins a generic contract

Here is the larger idea this lesson is building toward. When you overload an operator, you are quietly announcing that your type *satisfies a concept* — a capability that generic code can rely on:

| Operator | The capability it announces |
|---|---|
| `==` | equality-comparable |
| `<` or `<=>` | orderable |
| `<<` | stream-printable |
| `[]` | indexable |
| `()` | callable |
| `++` | incrementable / iterator-like |

This is why operator overloading is the gateway to the standard library. Give your type `<`, and `std::sort` and `std::set` can order it. Give it `++` and `!=` and `*`, and it can act as an iterator. The operators are the contract; the library's generic machinery is the customer.

### Don't overload just to satisfy a template

The flip side, and a real temptation: if `<` has no meaningful ordering for your type, do **not** invent one merely so you can call `std::sort` or `std::max`. A meaningless `operator<` is a landmine — every other piece of code that orders your type inherits the nonsense. Pass an explicit comparator instead, and keep the ordering local and named:

```cpp
#include <algorithm>
#include <vector>

std::sort(records.begin(), records.end(),
    [](const Record& a, const Record& b)
    {
        return a.timestamp() < b.timestamp(); // ordering stated right here
    });
```

> **Best practice:** Overload an operator only when it has a genuine, single meaning for your type. When you need an ad-hoc ordering for one algorithm, pass a comparator rather than baking a dubious `operator<` into the type.

### Concepts make the requirement explicit (C++20)

C++17 templates express their requirements *implicitly* — through the operations they happen to use, with errors surfacing at instantiation. C++20 added **concepts**, which let a template state its requirements up front and produce clearer errors:

```cpp
#include <concepts>

template <typename T>
requires std::totally_ordered<T>
const T& larger(const T& a, const T& b)
{
    return (a < b) ? b : a;
}
```

The underlying idea is unchanged from C++17, and it's the whole point of this lesson: a template compiles only when the operations it uses are available for the type you give it. Operator overloads are how you make your types eligible.

---

## 21.x — Chapter 21 summary and quiz

### The operator-form guide

When you reach for an operator, this table captures the conventional choice of form — distilled from everything above:

| Operator family | Typical form |
|---|---|
| `+`, `-`, `*`, `/` | non-member; friend if it needs private access |
| `<<`, `>>` | non-member, usually friend; return the stream by reference |
| unary `+`, `-`, `!` | member |
| `==`, `<` | non-member (or a defaulted member in C++20) |
| `++`, `--` | member; prefix returns a reference, postfix returns the old value |
| `[]` | member; return a reference for mutable access |
| `()` | member; makes the object callable |
| conversion operator | member; usually `explicit` |
| `=` | member; return `*this` by reference |

### The design rules, in one place

- Overload an operator only when its meaning for your type is **obvious**.
- **Preserve expectations**: match the built-in operator's mutability and return type.
- Use **non-member** operators for symmetric binary operations; use **member** operators when the language requires it or when the operation naturally acts on the current object.
- Implement related operators in terms of a **small core set** (`!=` from `==`; `>`/`<=`/`>=` from `<`; `+` from `+=`).
- Be especially careful with **raw owning pointers**: shallow copying invites leaks, dangling pointers, and double-frees.
- Prefer **standard-library members** so the rule of zero applies and copying is correct by default.

### The copying rules, summarized

```text
value-only class:
    the compiler-generated copy is usually fine

class owning a raw pointer:
    you probably need all three — destructor, copy constructor, copy assignment

class built from std::vector / std::string / smart pointers:
    the generated special members are usually correct (rule of zero)
```

### Reading overloaded code

The most durable payoff of this chapter is fluency in *reading* operator-heavy C++. The standard library is built from it:

```cpp
out << value;  // overloaded stream insertion
it != end;     // iterator comparison
++it;          // iterator increment
container[i];  // subscript overload
if (handle)    // explicit operator bool
```

When unfamiliar code overwhelms you with operators, translate each one back into the function call it really is, and the magic evaporates:

```cpp
a + b        ->  operator+(a, b)     or  a.operator+(b)
stream << x  ->  operator<<(stream, x)
obj[index]   ->  obj.operator[](index)
obj(args)    ->  obj.operator()(args)
```

Every operator in this chapter is a function. Once you can see the function, you can read the code.

---

## 21.y — Chapter 21 project

This chapter's lab, **Fraction v2**, returns to the `Fraction` class you built back in Chapter 14 and finally gives it the notation it deserves. Where you once wrote `a.multipliedBy(b)` and `a.equals(b)`, you'll now write `a * b` and `a == b` — by adding the operator overloads this chapter taught.

### What the project pulls together

A good operator-overloading project forces every idea in this chapter to cooperate around a single type:

- a class with a **real invariant** — `Fraction` always stores a fully reduced value with a positive denominator
- choosing **which operators** are natural for the type (`*`, `+`, `==`, `!=`, `<`, unary `-`, `++`, `<<`)
- writing symmetric binary operators (`*`, `+`, `==`, `<`) as **non-member friends**
- making `operator<<` a non-member that **returns the stream by reference**
- returning `*this` by reference from **prefix `++`**, and the **old value by value** from postfix `++`
- implementing one operator in terms of another (`!=` from `==`; `int * Fraction` from `Fraction * int`; postfix `++` from prefix `++`)

The unifying discipline is the **invariant**. Every operator that produces a new `Fraction` runs it through the reducing constructor, so the result is always in lowest terms. That single guarantee is what makes the other operators trivial: because equal fractions are stored identically, `operator==` is a plain field comparison — no cross-multiplication needed — and `operator<` needs only one cross-multiply, with no risk of a flipped inequality because the denominator is always positive.

### The integration map

```text
class invariant (always reduced, positive denominator)
    |
    v
constructors create valid Fractions
    |
    v
operators preserve the invariant
    |
    +-- arithmetic (* + unary-) returns NEW reduced values
    +-- prefix/postfix ++ mutate or copy, then stay reduced
    +-- comparison (== != <) only observes
    +-- stream output (<<) only observes
```

### A compact reminder of the pressure points

The lab's `Fraction` is larger, but this miniature `Counter` captures the operator-design decisions you'll be making — and is worth re-reading before you start:

```cpp
#include <iostream>

class Counter
{
private:
    int m_value {};

public:
    explicit Counter(int value)
        : m_value { value }
    {
    }

    Counter& operator++()        // prefix: mutate, return *this by reference
    {
        ++m_value;
        return *this;
    }

    Counter operator++(int)      // postfix: copy old, increment, return old by value
    {
        Counter old { *this };
        ++(*this);
        return old;
    }

    friend bool operator==(const Counter& a, const Counter& b)
    {
        return a.m_value == b.m_value; // comparison only observes
    }

    friend std::ostream& operator<<(std::ostream& out, const Counter& counter)
    {
        out << counter.m_value;
        return out; // return the stream so chaining works
    }
};
```

Every choice in that little class recurs in the lab: prefix mutates and returns a reference, postfix returns the old value by value, comparison only reads, and stream output returns the stream so `std::cout << a << b` chains.

### How to know your operators are right

When you've finished the lab — or are reviewing any class that overloads operators — run each operator past this checklist:

```text
Does this operator mean what a C++ reader expects?
Does it preserve the type's invariant?
Does it return the conventional type (new value / bool / stream& / *this&)?
Does it avoid needless copies?
Does it work on const objects where it should?
Does it compose naturally with the other operators?
```

If every operator on your `Fraction` can answer "yes" to all six, you've internalized the durable lesson of this chapter: an overloaded operator is just a function — and a *good* one behaves exactly as its built-in namesake would lead a reader to expect.
