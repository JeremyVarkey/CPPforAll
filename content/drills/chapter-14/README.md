# Chapter 14 — Introduction to Classes: The Fraction Class

> Reinforces [`../../notes/chapter-14.md`](../../notes/chapter-14.md) · LearnCpp [Chapter 14](https://www.learncpp.com/cpp-tutorial/introduction-to-object-oriented-programming/)

## The project

You are building a `Fraction` class — a rational number stored as
`numerator / denominator` — the chapter's own running example (notes 14.2, 14.8,
14.9).  The class has a central **invariant**: the denominator is never zero and
is always kept positive (the numerator carries any negative sign).  Enforcing
that invariant is the job of the class's **constructors** and **private helpers**;
callers using the public interface cannot break it.

This is exactly the Chapter 14 central idea: a **class** wraps private data
behind a public interface, uses **constructors** (with member-initializer lists)
to guarantee valid initial state, and marks read-only operations as **const
member functions** so they work on const objects and const references.

Because Chapter 14 introduces member bodies written *inline* (inside the class
braces), you edit a single header, `starter/fraction.h`.  The `.h`/`.cpp`
member-definition split arrives in Chapter 15.

**CS6340 tie-in:** LLVM uses this pattern constantly. `llvm::APInt` stores an
arbitrary-precision integer with a private limb array; its constructors normalize
the representation, and many member functions are const.  Once you can read a
class with private data, member-init lists, and const member functions, you can
read `APInt.h` — and the `Instruction`, `Function`, and `Module` class APIs that
every analysis pass uses.

## Concepts practiced

- **`class` keyword**, `public:` / `private:` access specifiers (notes 14.2, 14.5)
- **Private data members** with `m_` prefix convention (notes 14.5)
- **Const member functions** (notes 14.4) — required for const objects and const
  references, which are the standard way to pass class objects to functions
- **Constructors + member-initializer lists** (notes 14.9, 14.10) — preferred
  over body assignment; certain members *must* use the list
- **`explicit` single-argument constructor** (notes 14.16) — blocks accidental
  implicit `int → Fraction` conversions
- **Private helper functions** (`gcd`, `reduce`) — encapsulating implementation
  details the caller never needs to see (notes 14.8)
- **Same-class private access** inside member functions (notes 14.5) — used in
  `equals()` to read another `Fraction`'s private fields
- **Return by value** from a const member function (`multipliedBy`) — C++17 copy
  elision (notes 14.15) makes this free
- Reused from earlier chapters: **loops** (`while`/`if` in `gcd` — Ch 8),
  **`int` arithmetic** (Ch 1), **`std::string` + `std::to_string`** (Ch 5),
  **`const` references** as parameters (Ch 5/12), **header guards** (Ch 2)

## Your tasks

The starter compiles immediately (all stubs return wrong-but-harmless values), so
`make build` is GREEN right now. `make test` is **RED** — your goal is to fill in
the six `>>> YOUR CODE HERE <<<` blocks in
[`starter/fraction.h`](starter/fraction.h) until it turns **GREEN**.

The tasks ramp from straightforward to the chapter's full idiom:

1. **Default constructor (0/1).**  Add a member-initializer list that sets
   `m_numerator` to `0` and `m_denominator` to `1`.  The body is empty.
   This is the simplest possible constructor; it just demonstrates the init-list
   syntax. (notes 14.10)

2. **Two-argument constructor `Fraction(int n, int d)`.**  Use a member-init
   list to store `n` and `d` into `m_numerator` / `m_denominator`, then in the
   body: guard against `d == 0` (set `m_denominator = 1` if so), then call
   `reduce()` (already provided) to normalize the sign and divide out the GCD.
   After construction, `Fraction{1,-2}` stores `-1/2`; `Fraction{4,6}` stores
   `2/3`. (notes 14.9, 14.10)

3. **Explicit whole-number constructor `explicit Fraction(int w)`.**  Initialize
   `m_numerator` to `w` and `m_denominator` to `1` via the member-init list.  The
   `explicit` keyword is already in the declaration skeleton — keep it.  Whole
   numbers are already reduced, so no `reduce()` call is needed.  (notes 14.16)

4. **Const accessor (getter) functions.**  Make `numerator()` return
   `m_numerator` and `denominator()` return `m_denominator`.  Both must be marked
   `const` (the keyword already appears in the skeleton).  Without `const`, calling
   them on a `const Fraction` or a `const Fraction&` fails at compile time.
   (notes 14.4)

5. **`equals(const Fraction& other) const`.**  Return `true` when `m_numerator ==
   other.m_numerator` AND `m_denominator == other.m_denominator`.  Because both
   fractions are fully reduced (constructors call `reduce()`), equal fractions have
   identical fields — no cross-multiplication needed.  You may access
   `other.m_numerator` and `other.m_denominator` directly: same-class private
   access is legal inside member functions. (notes 14.5)

6. **`multipliedBy(const Fraction& other) const`.**  Compute `(a/b) * (c/d) =
   (a*c) / (b*d)` and return a **new** `Fraction` built with the two-argument
   constructor (which calls `reduce()` automatically).  Do not modify `*this`.
   Return by value — C++17 copy elision (notes 14.15) makes this zero-overhead.

You do **not** edit `tests/tests.cpp`, the `Makefile`, or anything in `solution/`.

## Constraints

**Allowed:**
- `class`, `public:`, `private:`, member functions defined **inside** the class
  braces (the Ch 14 inline idiom)
- `const` member functions (keyword after the parameter list)
- Constructors with member-initializer lists
- `explicit` on single-argument constructors
- `if` / `else`, `while` loops, `int` arithmetic, `%` and `/` (Ch ≤ 8)
- `std::string`, `std::to_string`, `std::ostream` (Ch 5, already included)
- Same-class private access inside member functions

**Forbidden (not yet taught):**
- `ClassName::method` definitions *outside* the class braces — that is a Ch 15
  concept; keep all bodies inline
- `static` members or functions (Ch 15)
- `operator==`, `operator*`, or any other operator overloading (Ch 21) — use
  named methods like `equals()` and `multipliedBy()` instead
- Destructors (Ch 15), inheritance (Ch 24), `std::gcd` from `<numeric>`
- Changing the signatures of any provided functions

**Required idioms:**
- Member-initializer lists (not body assignment) for all constructors (notes 14.10)
- `explicit` on the single-argument whole-number constructor (notes 14.16)
- `const` on every member function that does not modify the object (notes 14.4)

## Build & run

```sh
make            # compile-check starter/fraction.h  (already GREEN)
make test       # grade your class  ->  RED until the TASK blocks are filled
make solution   # compile and run the reference solution's tests
make clean      # remove build artifacts
```

`make test` compiles `tests/tests.cpp` with `-Istarter` so it picks up your
header.  `make test-solution` does the same with `-Isolution` — that is the
**Style B2** trick from `CLAUDE.md`: no `.cpp` files to link, just an include-
path switch.

## Success criteria

`make test` prints `PASS ✅ all Fraction checks passed.` and exits 0.

Until then it prints one `FAIL: <expression>  @line N` per broken check. The
grader covers every task including edge cases that separate a correct
implementation from a plausible near-miss:

- `Fraction{4,6}` must produce `2/3` (reduction working)
- `Fraction{1,-2}` must produce `-1/2` (sign normalization)
- `Fraction{-3,-4}` must produce `3/4` (both signs cancel)
- `Fraction{0,7}` must produce `0/1` (zero numerator)
- `Fraction{5,5}` must produce `1/1` (numerator == denominator)
- `Fraction{3,0}` must have a non-zero denominator (zero-denominator guard)
- `const Fraction cf{2,3}; cf.numerator()` must compile (const member functions)
- `neg1.equals(neg2)` where one is `{-1,2}` and the other is `{1,-2}` — both
  normalize to `-1/2` so they must be equal
- `multipliedBy` must leave the original object unchanged
- `(3/4) * (4/3)` must produce `1/1`, not `12/12` (reduce on the way out)

## Hints

<details><summary>Task 1 — member-initializer list syntax</summary>

```cpp
Fraction()
    : m_numerator   { 0 }    // colon starts the init list
    , m_denominator { 1 }    // comma-separated; order matches declaration order
{
}                            // body is empty — everything was done in the list
```

The list runs *before* the constructor body. For simple values like `int`,
this is equivalent to assigning in the body, but the list form is preferred
(notes 14.10) because it initializes rather than default-initializes then assigns.
</details>

<details><summary>Task 2 — why call reduce() and what it does</summary>

`reduce()` (provided in the private section) does two things:

1. If `m_denominator < 0`, it negates both parts so the denominator is positive.
2. It computes `gcd(m_numerator, m_denominator)` and divides both by it.

You do not need to write this logic yourself — just call `reduce()` at the end of
the two-argument constructor body. The member-init list sets the raw values; the
body cleans them up:

```cpp
Fraction(int n, int d)
    : m_numerator   { n }
    , m_denominator { d }
{
    if (m_denominator == 0)
        m_denominator = 1;   // cannot store an invalid fraction
    reduce();                // normalize sign and reduce to lowest terms
}
```
</details>

<details><summary>Task 3 — what explicit actually blocks</summary>

Without `explicit`, a function declared as:

```cpp
bool isHalf(Fraction f);
```

could be called as:

```cpp
isHalf(2);   // int 2 is silently converted to Fraction{2} — surprising!
```

With `explicit Fraction(int w)`, that implicit path is blocked. The caller must
write `isHalf(Fraction{2})` — the intent is visible. Best practice from
notes 14.16: **make single-argument constructors `explicit` by default** unless
you specifically want the implicit conversion.
</details>

<details><summary>Task 4 — why const is required, not optional</summary>

Given:

```cpp
void printFraction(const Fraction& f)
{
    std::cout << f.numerator();  // is this legal?
}
```

If `numerator()` is NOT marked `const`, the compiler refuses this call — even
though `numerator()` obviously only reads. The rule: **a const object (or const
reference) may only call const member functions** (notes 14.4). The `const`
keyword is your promise to the compiler that the function does not modify the
object. Without it, the compiler must assume the worst.
</details>

<details><summary>Task 5 — same-class private access</summary>

Inside any `Fraction` member function, you can access the private data of *any*
`Fraction` object, not just `*this`:

```cpp
bool equals(const Fraction& other) const
{
    return (m_numerator   == other.m_numerator)   // other's PRIVATE field
        && (m_denominator == other.m_denominator);
}
```

Access control is per-**class**, not per-**object** (notes 14.5). Both
`other.m_numerator` and `m_numerator` are private to `Fraction`, and you are
inside `Fraction`'s member function, so both accesses are legal.
</details>

<details><summary>Task 6 — multiply and why the result is already reduced</summary>

```cpp
Fraction multipliedBy(const Fraction& other) const
{
    return Fraction { m_numerator   * other.m_numerator,
                     m_denominator * other.m_denominator };
}
```

The two-argument `Fraction(int, int)` constructor calls `reduce()`, so the
returned fraction is already in lowest terms — you do not need to call `reduce()`
yourself. Return by value: C++17 copy elision (notes 14.15) constructs the
result directly in the caller's storage, so there is no extra copy.

Note that you access `other.m_numerator` and `other.m_denominator` directly
(same-class private access again), rather than going through the public accessors.
Both styles are correct; direct field access is marginally faster and is the
style the reference solution uses.
</details>

<details><summary>Stuck on a compile error before a test failure?</summary>

- *"error: 'numerator' is a private member"* inside `equals()` or `multipliedBy()`:
  you may be trying to access `other.numerator()` via the public getter from a
  *non*-member context. Inside a `Fraction` member function, direct access to
  `other.m_numerator` is legal. Alternatively, calling `other.numerator()` (the
  public getter) is also legal — use whichever reads more clearly.
- *"error: member function 'numerator' not viable: 'this' argument has type
  'const Fraction'"*: you forgot the trailing `const` on `numerator()` or
  `denominator()`. Add `const` after the closing `)` of the parameter list.
- *"error: cannot convert 'int' to 'Fraction'"*: somewhere a raw `int` is being
  passed where a `Fraction` is expected. The `explicit` keyword on the one-arg
  constructor is doing its job. Wrap the int: `Fraction{someInt}`.
</details>

## Stretch goals (optional — most need later chapters)

- Add a `dividedBy(const Fraction& other) const` method: `(a/b) / (c/d) =
  (a*d) / (b*c)`. Guard against dividing by zero (other.numerator() == 0).
- Add `addedTo(const Fraction& other) const` to practice the common-denominator
  algorithm without operator overloading. It is more complex than `multipliedBy`
  because you need `lcm(d1, d2)`.
- Overload `operator*` and `operator==` using the named methods above (Ch 21) to
  see how the named versions become the building blocks.
- Extend the constructors to also accept a `double` and approximate the fraction
  (a fun continued-fractions exploration, though this goes well beyond Ch 14).
- Replace `isValid()` with a compile-time `static_assert` in the constructor body
  using `assert` (Ch 9) to catch zero-denominator bugs during development.
