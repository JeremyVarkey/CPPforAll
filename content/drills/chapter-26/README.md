# Chapter 26 — Templates and Classes: `Stack<T>`

> Reinforces [`../../notes/chapter-26.md`](../../notes/chapter-26.md) · LearnCpp [Chapter 26](https://www.learncpp.com/)

## The project

You're building a **bounded stack** as a class template, `Stack<T, Capacity>`.
The project forces you to confront the central constraint of class templates
head-on: because the compiler must stamp out a concrete `Stack<int>` or
`Stack<std::string>` at the point of use, the full template definition — data
layout, member functions, and all — must live in the **header**. There is no
`.cpp` split here; the header *is* the implementation. That constraint is not a
limitation to work around; it is the lesson.

The grader instantiates the stack with **two element types** — `Stack<int>` and
`Stack<std::string>` — so any accidental hardcoded-`int` assumption will surface
as a type error or a wrong result. A companion `TypeLabel<T>` trait demonstrates
**full class specialization**: the primary template says `"unknown"` for any
unrecognized type, while three explicit `template <>` specializations override
it for `int`, `double`, and `std::string_view`. Both together are small but
complete examples of the patterns LLVM uses for its container and type-system
helpers (`SmallVector<T, N>`, type traits, diagnostic utilities).

## Concepts practiced

- **Class templates** — `template <typename T, int Capacity = 8> class Stack`
  as a reusable stencil (notes 26.1)
- **Non-type template parameters** — `Capacity` as a compile-time integer that
  sizes `std::array<T, Capacity>` (notes 26.2); the value is part of the type
  (`Stack<int, 4>` ≠ `Stack<int, 8>`)
- **In-class member function definitions** — push, top, size, isEmpty, isFull
  defined inside the class body (notes 26.1)
- **Out-of-class member definition syntax** — `template <typename T, int Capacity>
  void Stack<T, Capacity>::pop()` practiced in isolation (notes 26.1 / 26.2)
- **Header-only layout** — why templates cannot split into `.h`/`.cpp` by
  default, and why this file is both the declaration and the definition (notes
  26.1)
- **Full class template specialization** — `template <> struct TypeLabel<int>`
  overrides the primary template for one exact type (notes 26.4)
- **Precondition enforcement with assert** — top() and pop() on an empty stack
  are undefined; assert guards them (reused from Ch 9)
- **`std::array<T, N>`** as fixed backing storage (reused from Ch 17)
- Reused: `const T&` parameters, `constexpr`, `std::string_view`, `bool` return
  values, member-init `{}` zero-init (Ch 5, 10, 14)

## Your tasks

The starter `starter/stack.h` compiles immediately but every body returns a
**wrong placeholder**, so `make test` starts **RED**. Fill in the four
`>>> YOUR CODE HERE <<<` blocks. They ramp from scaffolded warmup to the full
capstone:

1. **Write the `Stack` class template (in-class bodies).** Replace the
   placeholder `template <typename T, int Capacity = 8> class Stack` with a
   complete implementation. Declare `pop` in-class; leave its body for Task 3.
   Constraints: use `std::array<T, Capacity>` and `int m_size` as private data;
   keep all bodies (except `pop`) inside the class; `push` returns `false` when
   full; `top` asserts non-empty.

2. **Write the `TypeLabel` primary template.** Define `template <typename T>
   struct TypeLabel` with a `static constexpr std::string_view name { "unknown" }`.
   This is the catch-all that fires for any type you haven't specialized.

3. **Define `Stack<T, Capacity>::pop` out-of-class.** Write the out-of-class
   definition below the class body, using the pattern:
   `template <typename T, int Capacity> void Stack<T, Capacity>::pop() { … }`.
   The body should assert non-empty then decrement `m_size`. This is the only
   place in the exercise you write the full `Stack<T, Capacity>::` qualification.

4. **Add three full specializations of `TypeLabel`.** Using the
   `template <> struct TypeLabel<XYZ> { … }` syntax, make
   `TypeLabel<int>::name == "int"`,
   `TypeLabel<double>::name == "double"`, and
   `TypeLabel<std::string_view>::name == "string_view"`.

You do **not** edit `tests/tests.cpp` or the `Makefile` — those are the
contract and the grader.

## Constraints

**Allowed:** `std::array`, `assert`, `constexpr`, `std::string_view`, in-class
member definitions, out-of-class member definitions with the
`template <typename T, int Capacity>` head, `template <>` full specializations,
default template arguments, `static_cast<std::size_t>` for indexing.

**Required idioms:**
- `push` returns `bool` (false-on-full) — no exceptions (Ch 27), no assert
- `top` and `pop` use `assert(!isEmpty())` for the precondition (Ch 9)
- All member bodies except `pop` are defined **in-class**; `pop` is defined
  **out-of-class** using `Stack<T, Capacity>::pop` (the point of Task 3)
- The full template definition — including the out-of-class `pop` — stays
  inside this header (no `.cpp` companion file)

**Forbidden (not taught yet or out of scope):**
- `new` / `delete` / dynamic allocation (Ch 19 — use `std::array`, not `T*`)
- `std::vector` as the backing store (that uses dynamic allocation)
- Partial specialization (notes 26.5 — present in the notes for reading
  comprehension, but not required here)
- `std::is_same`, `if constexpr`, concepts / SFINAE (beyond Ch 26 scope)
- `throw` / `try` / `catch` (Ch 27)

## Build & run

```sh
make             # compile-check starter/stack.h  (should already work)
make test        # grade your stack.h  ->  RED until the TASK blocks are done
make test-solution   # run the reference  ->  always GREEN
make solution    # show the reference output
make clean       # remove build artifacts
```

`make test` and `make test-solution` use **-Istarter** and **-Isolution**
respectively, so the same `tests/tests.cpp` file grades either copy (Style B2).

## Success criteria

`make test` prints **PASS ✅ all stack checks passed.** and exits 0. Until then
it prints one `FAIL: … @line N` per broken check. The grader covers:

- `Stack<int>` push / top / pop lifecycle — size and top value after each op
- `push` returning `false` when full (`Stack<int, 3>` filled to capacity)
- **`Stack<std::string>`** — the second instantiation catches any int-only assumptions
- `Stack<std::string, 2>` capacity enforcement
- `Stack<int, 1>` single-slot edge case
- Push-pop-push reuse: a freed slot must accept a new push
- Default capacity (8): fill all eight slots, then pop in LIFO order
- `TypeLabel` primary template: unrecognized type → `"unknown"`
- `TypeLabel<int>`, `TypeLabel<double>`, `TypeLabel<std::string_view>` specializations

## Hints

<details><summary>Task 1 — class template skeleton and non-type parameter</summary>

```cpp
template <typename T, int Capacity = 8>
class Stack
{
    std::array<T, Capacity> m_data {};
    int                     m_size {};

public:
    bool push(const T& value)
    {
        if (isFull()) return false;
        m_data[static_cast<std::size_t>(m_size)] = value;
        ++m_size;
        return true;
    }

    T top() const
    {
        assert(!isEmpty() && "Stack::top called on empty stack");
        return m_data[static_cast<std::size_t>(m_size - 1)];
    }

    int  size()    const { return m_size;            }
    bool isEmpty() const { return m_size == 0;        }
    bool isFull()  const { return m_size == Capacity; }

    void pop();   // declaration only — body in Task 3
};
```

`Capacity` is a **non-type template parameter** (notes 26.2): it is a
compile-time integer that becomes part of the type.
`Stack<int, 4>` and `Stack<int, 8>` are different types, just like
`std::array<int, 4>` and `std::array<int, 8>`.
</details>

<details><summary>Task 2 — primary template syntax</summary>

```cpp
template <typename T>
struct TypeLabel
{
    static constexpr std::string_view name { "unknown" };
};
```

This is the "catch-all" (notes 26.4): it fires whenever no specialization
matches.  `constexpr` makes `name` a compile-time constant — zero runtime cost.
</details>

<details><summary>Task 3 — out-of-class syntax (the tricky one)</summary>

The out-of-class definition must repeat the full template parameter list AND
qualify the class name with both parameters (notes 26.1 / 26.2):

```cpp
template <typename T, int Capacity>
void Stack<T, Capacity>::pop()
{
    assert(!isEmpty() && "Stack::pop called on empty stack");
    --m_size;
}
```

Compare to a non-template out-of-class definition:
```cpp
void Counter::decrement() { --m_count; }  // no template head, no <T>
```

The two changes for a class template:
1. Add `template <typename T, int Capacity>` before the return type.
2. Change `Stack::pop` to `Stack<T, Capacity>::pop`.

That is the entire difference — everything else is normal member-function syntax.
</details>

<details><summary>Task 4 — full specialization syntax</summary>

```cpp
template <>
struct TypeLabel<int>
{
    static constexpr std::string_view name { "int" };
};
```

`template <>` means "all template parameters are now fixed — none remain"
(notes 26.4).  The primary template must be defined first (Task 2).
Add one `template <>` block for each of `int`, `double`, and `std::string_view`.

The specialization is an independent class definition: it does not "inherit"
from the primary template; it replaces it for that exact type.
</details>

<details><summary>Stuck on a compile error?</summary>

- *"expected unqualified-id"* near `Stack<int>` — the placeholder class in the
  starter is not a template. Make sure your Task 1 replacement starts with
  `template <typename T, int Capacity = 8>`.
- *"m_size was not declared"* in the out-of-class pop — your Task 1 Stack must
  have `int m_size {};` as a private data member; pop accesses it via `this`.
- *"no member named name"* for `TypeLabel<int>` — you have not added the
  `template <>` specialization yet (Task 4), or you added it before the primary
  template was defined (reorder so Task 2 comes first, Task 4 second).
- *"-Wunused-parameter"* warning — make sure you remove the `/*comment-out*/`
  syntax from parameters once you use them (the stubs use `/*value*/` to avoid
  unused-parameter warnings; real implementations use the parameter directly).
</details>

## Stretch goals (optional)

- Add a **non-type template default** for Capacity so `Stack<double>` uses 8
  slots but `Stack<double, 16>` uses 16 (already done in the design — but try
  adding `Stack<int, 4>` to your own test driver and see the type is different).
- Add `void clear()` — reset `m_size` to 0 — practicing in-class definition
  style on a new member.
- Add `template <> struct TypeLabel<float>` and `TypeLabel<bool>` as additional
  full specializations (notes 26.4 — same syntax as Task 4).
- Read notes 26.5 and think about how you would write a **partial specialization**
  `Stack<T*, N>` that stores raw pointers — then read the ownership warning in
  notes 26.6 about why that is more dangerous than it looks.
- Flip the backing store to `std::vector<T>` (Ch 16) and remove `Capacity` —
  observe how removing the non-type param simplifies the type signature but
  adds a dynamic-allocation cost.
