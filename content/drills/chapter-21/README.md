# Chapter 21 — Operator Overloading: Fraction v2

[Chapter notes](../../notes/chapter-21.md) · [LearnCpp Chapter 21](https://www.learncpp.com/cpp-tutorial/introduction-to-operator-overloading/)

---

## The project

You built `Fraction` in Chapter 14: constructors, `equals()`, `multipliedBy()`,
print helpers. It worked, but it felt clunky — you had to write `a.multipliedBy(b)`
instead of `a * b`, and `a.equals(b)` instead of `a == b`.

**Fraction v2** fixes that. You will add nine families of operator overloads so
that `Fraction` feels like a built-in numeric type:

```cpp
Fraction a{1,2}, b{1,3};
Fraction c  { a * b };       // operator*
Fraction d  { a + b };       // operator+
bool same   { a == b };      // operator==
bool diff   { a != b };      // operator!=
bool less   { b < a };       // operator<
Fraction neg{ -a };          // unary operator-
++a;                         // prefix operator++
a++;                         // postfix operator++
std::cout << a;              // operator<<
```

The `Fraction` class is interesting precisely because it has a **class
invariant** — the stored form is always fully reduced, and the denominator is
always positive. Every operator you write must _preserve_ that invariant. That
is the real payoff: once the invariant holds, `operator==` is a trivial field
comparison with no cross-multiplication, and `operator<` only needs to
cross-multiply once.

---

## Concepts practiced

**New (Chapter 21):**
- Operator overloading — making user types feel built-in
- Friend non-member operators for symmetric binary operations (notes 21.2)
- Non-member `operator<<` with stream-reference return (notes 21.4)
- Member vs. friend decision (notes 21.1, 21.5)
- Unary operator- as const member (notes 21.6)
- Prefix `operator++`: mutates and returns `*this` by reference (notes 21.8)
- Postfix `operator++`: dummy-int signature, copy-then-increment, return old value (notes 21.8)
- Comparison operators — reduced-form equality, cross-multiply ordering (notes 21.7)
- Implementing one operator in terms of another to minimize redundancy

**Reused from earlier chapters:**
- Class invariants, constructors, member-initializer lists (Chapter 14)
- `const` member functions, access specifiers, `explicit` (Chapter 14)
- Friend declarations for private-data access (Chapter 15)
- `std::string` and `std::to_string` (Chapter 5)

---

## Your tasks

Work through `starter/fraction.cpp` in order. Each `TASK n` block maps 1:1 to
one of the numbered tasks below.

**Task 1 — `operator*(Fraction, Fraction)`**
Fraction multiplication: `(a/b) * (c/d) = (a*c)/(b*d)`. Construct and return
a `Fraction{a*c, b*d}` — the two-argument constructor calls `reduce()` for you.
Both operands are `const Fraction&`; return a new `Fraction` by value.

**Task 2 — `operator*(Fraction, int)` and `operator*(int, Fraction)`**
Treat the `int` as a whole-number fraction `n/1`. Implement `Fraction * int`
directly; make `int * Fraction` delegate to it (one-liner: `return f * n;`).

**Task 3 — `operator+(Fraction, Fraction)`**
Standard formula: `(a/b) + (c/d) = (a*d + b*c) / (b*d)`. Again, pass through
the two-argument constructor so `reduce()` runs on the result.

**Task 4 — `operator==` and `operator!=`**
Because both fractions are fully reduced, equal fractions have _identical_ stored
fields. `operator==` is a field comparison; `operator!=` is `!(a == b)`.

**Task 5 — `operator<`**
Cross-multiplication: `a/b < c/d` iff `a*d < b*c`. Both denominators are always
positive (invariant), so multiplying both sides does not flip the inequality.
Keep test values small — the grader uses `|num| ≤ 20`, `|den| ≤ 20`.

**Task 6 — unary `operator-`**
Returns `Fraction{-m_numerator, m_denominator}`. Does not modify `*this`
(const member). One operand → member function.

**Task 7 — prefix `operator++`**
Add exactly 1: `(num/den) + 1 = (num + den) / den`. Update `m_numerator`
in-place, call `reduce()`, and return `*this` by reference.

**Task 8 — postfix `operator++`**
Three-step pattern: (1) copy `*this` into `old`; (2) call prefix `++(*this)`;
(3) return `old` by value. The dummy `int` parameter marks postfix — ignore it.

**Task 9 — `operator<<`**
Print `"num/den"` to the stream, then return the stream by reference. Must be
a non-member (left operand is `std::ostream`). Access `m_numerator` and
`m_denominator` through the `friend` declaration in `fraction.h`.

---

## Constraints

**Allowed:**
- All C++ ≤ Chapter 21 concepts (constructors, const, friend, references, basic standard library)
- Calling `reduce()` inside any operator that produces a new `Fraction`
- Delegating `operator!=` to `operator==`; delegating `int*Fraction` to `Fraction*int`
- Delegating postfix `++` to prefix `++`

**Forbidden:**
- `operator<=>` (C++20 spaceship — explicitly out of scope for this course level)
- `operator[]` and `operator()` (subscript/call operators — out of scope for this exercise)
- Conversion operators (`operator int()` etc.) — out of scope
- Altering `fraction.h`, `tests/tests.cpp`, or `Makefile`
- Any solution that passes tests by hard-coding expected values

**Required idioms:**
- `operator!=` must be expressed as `!(lhs == rhs)` — not a hand-rolled comparison
- Postfix `++` must save a copy _before_ incrementing, then return that copy
- `operator<<` must return `std::ostream&` (for chaining)

---

## Build & run

```bash
# Compile-check the starter (must succeed, zero warnings)
make build

# Run the grader against your starter code (RED until all tasks complete)
make test

# Peek at the reference solution output
make solution

# Verify the reference passes (must be GREEN — our proof the exercise is solvable)
make test-solution

# Remove build artifacts
make clean
```

---

## Success criteria

You have finished when:

1. `make build` compiles `starter/fraction.cpp` with **zero warnings** under `-Wall -Wextra`.
2. `make test` exits **GREEN** — all checks pass.
3. You can explain why `postfix x++` and `prefix ++x` return different things, and why `operator<<` cannot be a member of `Fraction`.

---

## Hints

<details>
<summary>Hint 1 — Task 1 (operator* Fraction*Fraction)</summary>

The formula is `(a*c)/(b*d)`. You have full access to both operands' private
fields because this is a `friend` function. Construct and return:

```cpp
return Fraction { lhs.m_numerator * rhs.m_numerator,
                  lhs.m_denominator * rhs.m_denominator };
```

The two-argument constructor calls `reduce()`, so the result is already in
lowest terms.
</details>

<details>
<summary>Hint 2 — Task 3 (operator+ addition formula)</summary>

To add fractions with different denominators, find a common denominator first.
The simplest (not necessarily the _lowest_) common denominator is `b*d`:

```
(a/b) + (c/d) = (a*d)/(b*d) + (b*c)/(b*d) = (a*d + b*c) / (b*d)
```

Pass those two expressions to `Fraction{...}` and `reduce()` does the rest.
</details>

<details>
<summary>Hint 3 — Task 4 (operator== reduced-form insight)</summary>

Because `reduce()` is always called in the two-argument constructor, two
mathematically equal fractions will have the **same stored fields**. So:

```cpp
return (lhs.m_numerator   == rhs.m_numerator)
    && (lhs.m_denominator == rhs.m_denominator);
```

Then `operator!=` is a one-liner: `return !(lhs == rhs);`
</details>

<details>
<summary>Hint 4 — Task 5 (operator< cross-multiplication)</summary>

Cross-multiply to avoid dividing (which would need `double`):

```
a/b < c/d  iff  a*d < b*c   (valid when b > 0 and d > 0)
```

The class invariant guarantees both denominators are positive, so the comparison
direction is preserved. With the small test values used, there is no overflow risk.
</details>

<details>
<summary>Hint 5 — Task 7 vs Task 8 (prefix vs postfix ++)</summary>

**Prefix** (`++x`): increment _then_ return the new value.
```cpp
Fraction& Fraction::operator++()
{
    m_numerator += m_denominator;
    reduce();
    return *this;   // ref to the updated object
}
```

**Postfix** (`x++`): save old, increment, return old.
```cpp
Fraction Fraction::operator++(int)   // the `int` is just a syntax marker
{
    Fraction old { *this };   // copy before any change
    ++(*this);                // call prefix++ (reuse!)
    return old;               // return the original value (by value, not ref)
}
```

The test checks: `Fraction old { f++ };` — `old` must equal the _pre_-increment
value, while `f` itself must have advanced by 1.
</details>

<details>
<summary>Hint 6 — Task 9 (operator<< non-member requirement)</summary>

`std::cout << f` desugars to `operator<<(std::cout, f)`. The left operand is
`std::ostream`, so the function cannot be a member of `Fraction` (that would
require `Fraction::operator<<(std::ostream&)`, which would need to be called
as `f << std::cout` — backwards).

The `friend` declaration in `fraction.h` already grants access to private
members. The body is two lines:
```cpp
out << f.m_numerator << '/' << f.m_denominator;
return out;
```
</details>

---

## Stretch goals

These extend beyond the required tasks — use concepts from later chapters if you
want to get ahead, and note which chapter each feature belongs to.

1. **`operator-` (binary subtraction):** `a - b = a + (-b)`. Delegate to
   `operator-` (unary) and `operator+`.

2. **`operator+=`:** a compound-assignment variant. Should modify `*this` and
   return `*this&`. Then rewrite `operator+` in terms of `operator+=` using
   a copy (the idiomatic implementation order).

3. **`operator>`**, **`operator<=`**, **`operator>=`:** derive all three from
   `operator<` using the same minimize-redundancy principle.

4. **`operator>>`** (stream extraction, Chapter 28 I/O): read a fraction from
   `"num/den"` input, validate that denominator is non-zero, set `failbit` on
   bad input. (Formally a Chapter 28 concept — notes 21.4 mentions it briefly.)

5. **`std::sort` compatibility:** since `operator<` is defined, a
   `std::vector<Fraction>` can be sorted with `std::sort` (Chapter 18 algorithms).
   Try it.
