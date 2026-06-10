# Chapter 22 — Move Semantics & Smart Pointers: TrackedBuffer

> Reinforces [`../../notes/chapter-22.md`](../../notes/chapter-22.md) · LearnCpp [Chapter 22](https://www.learncpp.com/)

---

## The project

You will complete `TrackedBuffer` — a class that **owns** a `new[]`-allocated
`int` array (the Chapter 19 payoff) and **instruments every special member
function** with static counters so you can *observe* at runtime exactly how many
copies and how many moves occurred. You implement the copy constructor, move
constructor, copy assignment, and move assignment, then use `std::unique_ptr` to
manage `TrackedBuffer` objects without writing a single manual `delete`.

Why this project? Because move semantics are most vivid when you can *see* them
fire. A copy increments `s_copies`; a move increments `s_moves`. The tests pass a
named object through `std::move` and verify that **only `s_moves` went up** and
that the source's pointer is now `nullptr`. There is no ambiguity — either you
stole the pointer (correct) or you copied it (wrong counter, wrong state). The
tests also verify `std::unique_ptr`'s ownership guarantee: once you `std::move` a
`unique_ptr` into a function, your local handle becomes null.

This maps directly to LLVM pass helper classes that own a dynamic buffer (a
`BranchCountMap`, a `SlotTracker`, a `CoverageDB`). Those classes must move
cheaply during vector reallocation and transfer ownership cleanly through factory
functions. The Rule of Five, `noexcept` move ops, and `make_unique` are the same
idioms you need there.

---

## Concepts practiced

- **Move constructor** (`T&&` parameter, steal + null) — new in Ch 22 (notes 22.3)
- **Move assignment** (release old, steal new, guard self-move) — Ch 22 (notes 22.3)
- **`std::move`** as an ownership-transfer cast, not a move itself — Ch 22 (notes 22.4)
- **Moved-from state**: `size() == 0`, `data() == nullptr` — Ch 22 (notes 22.3)
- **`noexcept`** on move operations — Ch 22 (notes 22.3)
- **`std::unique_ptr<T>`** + `std::make_unique` — Ch 22 (notes 22.5)
- Passing `unique_ptr` by value to transfer ownership — Ch 22 (notes 22.5)
- **`std::shared_ptr`** copy and `use_count` — Ch 22 (notes 22.6)
- C++17 **mandatory copy elision** trap: never assert counters on prvalue returns
- Reused from Ch 19: `new[]` / `delete[]`, dangling-pointer discipline
- Reused from Ch 15: `static` class members — definitions in `.cpp`
- Reused from Ch 14: constructors, member-initialiser lists, `const` members
- Reused from Ch 18: `std::copy_n` from `<algorithm>`
- Reused from Ch 5–9: `const`, header guards, assertions

---

## Your tasks

The starter compiles immediately and every function body is a **placeholder**, so
`make test` starts **RED**. Fill in the six `>>> YOUR CODE HERE <<<` blocks in
[`starter/tracked_buffer.cpp`](starter/tracked_buffer.cpp). They ramp from
warm-up to capstone:

1. **Copy constructor — deep copy** (`const TrackedBuffer&`).
   Allocate a *fresh* `new int[m_size]{}` array and copy every element with
   `std::copy_n`. Set `m_size` and `m_data` from `other`. Increment `s_copies`.
   Constraint: the new object must own its **own** array — not share `other`'s.

2. **Move constructor — steal pointer** (`TrackedBuffer&&`, `noexcept`).
   Copy `other.m_size` and `other.m_data` into `this`. Then **zero** the source:
   `other.m_size = 0; other.m_data = nullptr`. Increment `s_moves`.
   Constraint: mark `noexcept`. No allocation, no `std::copy_n` — that is the
   entire point.

3. **Copy assignment** (`const TrackedBuffer&`).
   Guard with `if (this == &other) return *this;`. Release the *current* array
   (`delete[] m_data`). Then deep-copy exactly as in Task 1. Increment `s_copies`.
   Return `*this`.

4. **Move assignment** (`TrackedBuffer&&`, `noexcept`).
   Guard with `if (this == &other) return *this;`. Release the current array.
   Steal `other`'s pointer and size. Zero `other`. Increment `s_moves`. Return
   `*this`. The self-move guard (`b = std::move(b)`) must leave the object valid.

5. **`makeTracked` factory** — one line.
   Return `std::make_unique<TrackedBuffer>(size)`. The caller receives exclusive
   heap ownership; they never call `delete` manually.

6. **`takeOwnership`** — accept a `std::unique_ptr<TrackedBuffer>` by value.
   If the buffer is non-empty, write `42` into element `[0]`. The `unique_ptr`
   destructs at the end of the function, automatically deleting the buffer. The
   caller **must** `std::move` their local pointer at the call site; forgetting
   `std::move` is a compile error (unique_ptr's copy constructor is deleted).

---

## Constraints

**Allowed:** everything from Chapters ≤ 22; `new int[size]{}`, `delete[]`;
`std::copy_n` (`<algorithm>`); `std::unique_ptr`, `std::make_unique`,
`std::shared_ptr`, `std::make_shared` (`<memory>`); `std::move`.

**Required idioms:**
- Move constructor and move assignment **must be marked `noexcept`** (notes 22.3).
- Move constructor must **leave the source empty**: `size() == 0`, `data() == nullptr`.
- Both assignment operators must have a **self-assignment guard** (`this == &other`).
- Task 5 must use **`std::make_unique`** (not `new TrackedBuffer` + wrapping).
- Task 6 must accept a **`unique_ptr` by value** (transfer semantics, not `get()`).

**Forbidden (not yet taught or out of scope):**
- `std::weak_ptr` deep dives or custom deleters (Ch 22 scope boundary).
- Manual `delete` or `delete[]` anywhere except the provided destructor and the
  assignment operators' "release current" step.
- Returning an rvalue reference (`T&&`) from any function — dangle risk
  (notes 22.2).

**C++17 elision trap:** Do NOT try to assert `s_moves` on the return value of
`makeTracked()`. In C++17 that return is a **prvalue** — the compiler constructs
the object directly in its destination (mandatory copy elision). No move
constructor fires. Only test `s_moves` on **named objects** wrapped in
`std::move`. (notes 22.3, 22.4)

---

## Build & run

```sh
make               # compile-check starter/tracked_buffer.cpp (warning-clean)
make test          # grade your code  →  RED until the TASK blocks are filled in
make test-solution # run the grader against the reference solution (always green)
make solution      # build + run the reference solution
make clean         # remove all build artifacts
```

`make test` is the grade. The starter **compiles** immediately (`make` is green
from the start) but all checks fail. Fill in the tasks; `make test` turns green.

---

## Success criteria

`make test` prints **PASS ✅  all TrackedBuffer checks passed.** and exits 0.

Until then, each failing check prints its expression and line number. Key things
the grader will catch:

- `s_moves == 1`, `s_copies == 0` after `TrackedBuffer b2 { std::move(b1) }` —
  the **move path, not the copy path** (notes 22.3)
- `b1.size() == 0` and `b1.data() == nullptr` after the move — the **moved-from
  state** (notes 22.3)
- Deep copy independence: `b2[0] = 99` must NOT change `b1[0]` (notes 22.3)
- Separate array addresses after copy (`b2.data() != b1.data()`)
- Pointer was **stolen** not copied in the move: `b2.data() == originalAddress`
- Copy assignment does NOT corrupt data when `b = b` (self-assignment guard)
- Move assignment leaves `b1` empty after `b2 = std::move(b1)`
- `self = std::move(self)` is safe (self-move guard, notes 22.3)
- `makeTracked(n)` returns a **non-null** `unique_ptr` with `size() == n`
- After `q = std::move(p)`, `p` is **null** (notes 22.5)
- After `takeOwnership(std::move(p))`, `p` is **null** (notes 22.5)
- `shared_ptr` `use_count` increments on copy and decrements when an owner
  goes out of scope (notes 22.6)

---

## Hints

<details><summary>Task 1 — the copy constructor: what does "deep copy" mean?</summary>

The key is that *two objects must own separate arrays*. The compiler-generated
copy would copy `m_data` (a pointer), leaving both objects pointing at the same
memory — double delete on destruction. Instead:

```cpp
TrackedBuffer::TrackedBuffer(const TrackedBuffer& other)
    : m_size { other.m_size }
    , m_data { other.m_size > 0 ? new int[other.m_size]{} : nullptr }
{
    if (m_size > 0)
        std::copy_n(other.m_data, m_size, m_data);
    ++s_copies;
}
```

The member-initialiser list does the allocation; the body copies the elements.
`std::copy_n(src, count, dst)` copies `count` elements from `src` into `dst`.
</details>

<details><summary>Task 2 — the move constructor: steal and zero</summary>

The pointer steal is two assignments; zeroing the source is two more:

```cpp
TrackedBuffer::TrackedBuffer(TrackedBuffer&& other) noexcept
    : m_size { other.m_size }
    , m_data { other.m_data }
{
    other.m_size = 0;
    other.m_data = nullptr;
    ++s_moves;
}
```

After these four lines the source is in the **moved-from state**: safe to
destruct (`delete[] nullptr` is a no-op), but no longer owns data. The
`noexcept` is not optional — leave it off and `std::vector` will copy instead
of move during reallocation.

Why is `std::move(b1)` needed at the call site? Because `b1` is a *named*
variable — a *named* variable is always an lvalue expression, even if its
declared type is `T&&` (notes 22.2). `std::move` is a cast that says
"treat this lvalue as movable." It does not itself move anything.
</details>

<details><summary>Task 3 — copy assignment: release before you assign</summary>

Unlike a copy constructor, the destination *already owns resources*. Release
them first — otherwise you leak the old array:

```cpp
TrackedBuffer& TrackedBuffer::operator=(const TrackedBuffer& other)
{
    if (this == &other) return *this;   // self-assignment guard

    delete[] m_data;                    // release old resource

    m_size = other.m_size;
    m_data = (m_size > 0) ? new int[m_size]{} : nullptr;
    if (m_size > 0)
        std::copy_n(other.m_data, m_size, m_data);

    ++s_copies;
    return *this;
}
```

Without the self-assignment guard, `b = b` would delete `m_data` and then try
to copy from the already-freed memory — undefined behaviour.
</details>

<details><summary>Task 4 — move assignment: the order matters</summary>

```cpp
TrackedBuffer& TrackedBuffer::operator=(TrackedBuffer&& other) noexcept
{
    if (this == &other) return *this;   // self-move guard

    delete[] m_data;       // release CURRENT resource

    m_size = other.m_size; // steal from source
    m_data = other.m_data;

    other.m_size = 0;      // zero source
    other.m_data = nullptr;

    ++s_moves;
    return *this;
}
```

The self-move guard is important even though `b = std::move(b)` is unusual
code. Without it, you would `delete[] m_data` and then try to read `other.m_data`
from freed memory — since `this == &other`, they are the same pointer.
</details>

<details><summary>Task 5 — makeTracked: one line with make_unique</summary>

```cpp
std::unique_ptr<TrackedBuffer> makeTracked(std::size_t size)
{
    return std::make_unique<TrackedBuffer>(size);
}
```

`std::make_unique<TrackedBuffer>(size)` calls `TrackedBuffer(size)` on the heap
and wraps the result in a `unique_ptr`. The return is a prvalue — C++17 mandatory
copy elision applies, so no copy or move constructor fires on the return. **Do not
add `std::move(...)` around the return expression — it would interfere with
elision.** (notes 22.4)
</details>

<details><summary>Task 6 — takeOwnership: by value = ownership transfer</summary>

```cpp
void takeOwnership(std::unique_ptr<TrackedBuffer> p)
{
    if (p && !p->empty())
        (*p)[0] = 42;
}  // p destructs here — TrackedBuffer is deleted automatically
```

The function signature `std::unique_ptr<TrackedBuffer> p` (by value) is the
convention for "I take ownership." The caller writes:

```cpp
takeOwnership(std::move(myPtr));  // myPtr is null after this line
```

`std::move` casts `myPtr` to an rvalue reference so the `unique_ptr` move
constructor fires. Trying to pass without `std::move` is a **compile error** —
`unique_ptr` explicitly deletes its copy constructor to enforce unique ownership.
(notes 22.5)
</details>

---

## Stretch goals (optional — some need later chapters)

- Make `TrackedBuffer` printable: add `operator<<(std::ostream&, const TrackedBuffer&)`
  (reuses Ch 21 operator overloading) that prints `[1, 2, 3]`.
- Add `TrackedBuffer(std::initializer_list<int>)` that fills the buffer from a
  brace-list — a Ch 23 preview of `std::initializer_list`.
- Add `swap(TrackedBuffer& a, TrackedBuffer& b) noexcept` using three
  `std::move`s — demonstrate that a swap based on moves does **zero** heap
  allocations.
- Add move-only semantics: `= delete` the copy constructor and copy assignment,
  making `TrackedBuffer` communicate "there is exactly one owner" at compile
  time (Ch 22: move-only types, notes 22.3).
- Use `std::unique_ptr<int[]>` as the internal storage type instead of a raw
  `int*` — then the destructor becomes trivial and the Rule of Five reduces to
  three members (move ctor, move assign; copy remains deleted). Compare the
  complexity with and without the raw pointer.
</content>
