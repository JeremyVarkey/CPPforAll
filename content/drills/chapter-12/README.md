# Chapter 12 — Compound Types: References and Pointers · Alias Workshop

> Reinforces [`../../notes/chapter-12.md`](../../notes/chapter-12.md) · LearnCpp [Chapter 12](https://www.learncpp.com/)

## The project

In every chapter before this one, every function received a **copy** of its
argument: changes to the parameter stayed inside the function, invisible to the
caller. Chapter 12 changes that. References and pointers let two names refer to
the **same memory** — and that aliasing is physical, not just a naming trick.

You will build the **Alias Workshop**: six small functions that collectively cover
every major reference/pointer pattern in the chapter. Each function either accepts
a reference or pointer (so it can modify the caller's variable) or *returns* one
(so the caller can modify the original through the return value). The automated
grader does not just check return values — it checks that the **original
caller-side variables actually changed**. It also compares raw memory addresses
(`&maxOf(a, b) == &b`) to prove aliasing is physical.

This matters immediately for CS6340: LLVM pass signatures like
`bool runOnFunction(Function &F)` and `Module *M { F.getParent() }` are
impossible to read fluently until references and pointers click.

## Concepts practiced

- **Lvalue references (`int& r`)** — another name for an existing object; cannot
  be null, cannot be reseated (notes 12.3)
- **Non-const references as in/out parameters** — the caller's variable is
  modified in-place through the reference (notes 12.5, 12.13)
- **Pass by address (`int*`)** — pass the object's address so the function can
  modify the pointed-to value (notes 12.10)
- **Null pointer safety (`nullptr`)** — always check a pointer before
  dereferencing; use `if (!ptr) return;` as the guard (notes 12.8)
- **`const int*` — pointer to const** — read-only pointer; the function cannot
  modify the object through it (notes 12.9)
- **Return by reference** — return an alias so the caller can write through it;
  the returned reference must alias something that outlives the function (notes
  12.12)
- **Dangling reference trap** — never return a reference to a local variable;
  it dies when the function returns, leaving a dangling alias (notes 12.3, 12.12)
- **`std::optional<T>`** — express "maybe no value" as part of the type, without
  a magic sentinel (notes 12.15)
- Reused from earlier chapters: `std::string_view` for read-only string
  parameters (Ch 5), `static_cast<std::size_t>` at sign-conversion boundaries
  (Ch 10), header guard and header/impl split (Ch 2), `static_assert` on types
  (Ch 7)

## Your tasks

The starter compiles immediately but every function returns a **placeholder**, so
`make test` starts **RED**. Fill in the six `>>> YOUR CODE HERE <<<` blocks in
[`starter/aliases.cpp`](starter/aliases.cpp). They ramp from warm-up to capstone:

1. **`swapByRef(int& a, int& b)` — swap through non-const lvalue references.**
   Exchange the values of two ints using a temporary local. `a` and `b` are
   references — writing to them writes to the caller's variables. No return value;
   the side-effect IS the output. (notes 12.3, 12.5)

2. **`swapByPtr(int* a, int* b)` — swap through pointers, null-safe.**
   Same three-step swap, but the caller passes **addresses** instead of
   references. The pointer-specific addition: if either pointer is `nullptr`, do
   **nothing** — return immediately. Otherwise dereference with `*a` and `*b` to
   reach the ints and swap. (notes 12.7, 12.8, 12.10)

3. **`maxOf(int& a, int& b) → int&` — return a modifiable reference.**
   Return a **reference** to the larger of the two parameters. The returned
   reference aliases whichever of `a` or `b` is larger — the grader writes
   through it and checks that the **original** changed. TRAP: do **not** create a
   local variable and return a reference to it — that is a dangling reference (UB).
   It is safe to return a reference to one of the parameters because they alias
   the caller's objects and outlive this call. (notes 12.12)

4. **`describePointer(const int* ptr) → std::string_view` — inspect a pointer.**
   Return `"null"` if `ptr == nullptr`, otherwise return `"value"`. The parameter
   is `const int*` — you are not allowed to modify the int through it. Check for
   null **before** doing anything else. (notes 12.8, 12.9)

5. **`addBonusInPlace(int& score, int bonus)` — textbook in/out parameter.**
   Add `bonus` to `score` in place by writing through the `score` reference.
   The caller's variable changes; there is no return value. (notes 12.13)

6. **`findFirst(std::string_view text, char target) → std::optional<int>` —
   optional search result.**
   Search `text` for the first occurrence of `target` and return its 0-based
   index as `std::optional<int>`. Return `std::nullopt` if not found. Use a `for`
   loop and cast the index to `std::size_t` when indexing (`text.length()` returns
   `std::size_t`). `std::optional` is covered in notes 12.15 — the note covers it
   fully, so it is in scope. (notes 12.15)

You may **not** edit [`aliases.h`](aliases.h) or anything in `tests/` — those are
the contract and the grader.

## Constraints

**Allowed:**
- `int&`, `const int&` — lvalue references (Ch 12)
- `int*`, `const int*` — pointers (Ch 12)
- `nullptr` — null pointer constant (Ch 12)
- `*ptr` — dereference operator (Ch 12)
- `&variable` — address-of operator (Ch 12)
- `std::optional<T>`, `std::nullopt`, `.has_value()`, `*opt` (Ch 12)
- `std::string_view` for read-only string parameters (Ch 5)
- `static_cast<std::size_t>` for sign-conversion (Ch 10)
- `if`, local `int` variables, `for` loops — all from earlier chapters

**Forbidden (not taught yet or explicitly out-of-scope):**
- `new` / `delete` / `new[]` / `delete[]` — dynamic allocation (Ch 19)
- `std::vector`, `std::array` — containers (Ch 16, 17)
- Pointer arithmetic (`ptr + 1`, `ptr++`) — Ch 17
- Smart pointers (`std::unique_ptr`, `std::shared_ptr`) — Ch 22
- `->` member access through pointer — you don't need it here (formally Ch 13+)

**Required idioms:**
- Always null-check a pointer **before** dereferencing it (notes 12.8)
- Never return a reference to a local variable (notes 12.12 — the chapter's
  number-one trap)
- Use `const int*` for read-only pointer parameters (notes 12.9)
- Prefer `nullptr` over `0` or `NULL` for null pointers (notes 12.8)

## Build & run

```sh
make            # compile-check starter/aliases.cpp (already warning-clean)
make test       # grade your code -> RED until the TASK blocks are filled in
make solution   # run the grader against the reference implementation
make clean      # remove build artifacts
```

`make test` is the grader — it calls your functions across many inputs, including
every edge case listed in the success criteria below.

## Success criteria

`make test` prints **PASS ✅ all alias-workshop checks passed.** and exits 0.
Until then it prints one `FAIL: … @line N` per broken check and ends with
`FAIL ❌ N check(s) failed`.

The grader exercises every function across many inputs, with edge cases that
separate correct aliasing from plausible-looking stubs:

- `swapByRef`: checks the **originals** changed (not just that the function ran)
- `swapByPtr`: null pointer on either side must NOT crash; both-null must NOT crash
- `maxOf`: **address comparison** `&maxOf(a,b) == &b` proves the reference aliases
  the correct variable (not a copy with the same value); write-through confirms
  the original changes
- `maxOf`: equal values — the returned reference must alias one of the two
  original variables, not a third object
- `describePointer(nullptr)` must return `"null"` — the stub always returns `"value"`
- `describePointer(&zero)` where zero `== 0` must return `"value"` (zero is a
  valid int, not a null pointer)
- `addBonusInPlace` with negative bonus — verifies decrement works (not just +=0)
- `findFirst` on an empty string — must return `nullopt` without crashing
- `findFirst` with repeated characters — must return the **first** index (0, not later)

## Hints

<details><summary>Task 1 — three-step swap with references</summary>

```cpp
void swapByRef(int& a, int& b)
{
    int tmp { a };   // save a's current value
    a = b;           // a = b's old value (writes through the reference)
    b = tmp;         // b = a's old value (writes through the reference)
}
```

`tmp` is a plain local `int` — just a value holder. `a` and `b` are references,
so writing to them writes to the caller's variables. This is the whole point of
pass-by-reference: the side-effect escapes the function.
</details>

<details><summary>Task 2 — null guard + dereference</summary>

```cpp
void swapByPtr(int* a, int* b)
{
    if (!a || !b)    // either null -> do nothing
        return;
    int tmp { *a };  // dereference to get the value
    *a = *b;
    *b = tmp;
}
```

`!a` is shorthand for `a == nullptr`. Checking BOTH before doing ANYTHING is the
safe pattern (notes 12.8). The dereference `*a` then reaches the int that `a`
points at — same three-step swap as Task 1, just spelled with `*`.
</details>

<details><summary>Task 3 — returning a reference (and avoiding the trap)</summary>

```cpp
int& maxOf(int& a, int& b)
{
    if (a >= b)
        return a;   // returns a reference to the caller's `a`
    return b;       // returns a reference to the caller's `b`
}
```

The TRAP is this:
```cpp
int& bad() {
    int local { 5 };
    return local;   // UNDEFINED BEHAVIOR: local dies when the function returns
}
```
`local` is destroyed at the closing brace; the returned reference dangles. The
safe pattern: return a reference to a PARAMETER that was passed in by reference —
the caller's object outlives the call (notes 12.12).
</details>

<details><summary>Task 4 — null check before anything else</summary>

```cpp
std::string_view describePointer(const int* ptr)
{
    if (ptr == nullptr)
        return "null";
    return "value";
}
```

`const int*` — you cannot write `*ptr = 5` here; the compiler forbids it. You CAN
check `ptr == nullptr` and you CAN read `*ptr` (if it's not null), but here you
don't even need to read the value — just its null-ness (notes 12.9).
</details>

<details><summary>Task 5 — in/out parameter with +=</summary>

```cpp
void addBonusInPlace(int& score, int bonus)
{
    score += bonus;  // writes through the reference
}
```

One line. `score` is an in/out reference: the function reads its current value
(the "in" side) and modifies it (the "out" side). `bonus` is passed by value — a
copy — because the function doesn't need to modify it. (notes 12.13)
</details>

<details><summary>Task 6 — linear search returning std::optional</summary>

```cpp
std::optional<int> findFirst(std::string_view text, char target)
{
    for (int i { 0 }; i < static_cast<int>(text.length()); ++i)
    {
        if (text[static_cast<std::size_t>(i)] == target)
            return i;           // wraps `i` in optional<int>
    }
    return std::nullopt;        // "no value"
}
```

`return i` implicitly constructs `std::optional<int>{i}`. `return std::nullopt`
produces the empty optional. The caller checks with `if (result)` or
`.has_value()` and accesses the value with `*result` (notes 12.15).
</details>

## Stretch goals (optional — some need later chapters)

- **Write `maxByPtr(int*, int*) -> int*`** that mirrors Task 3 but with pointers
  and a nullptr return for the "both null" case — reinforcing the
  "must-exist → reference, may-be-absent → pointer" rule from notes 12.8.
- **Add a `clamp(int& val, int lo, int hi)`** in/out function (notes 12.13):
  clamp `val` to `[lo, hi]` in place. Tests write `val` outside the range and
  check it was corrected.
- **Extend `findFirst` to return `std::optional<std::string_view>`** of the
  matching substring (requires `std::string_view::substr` — preview of Ch 5
  string ops — and shows that optional isn't just for index types).
- **Replace the `describePointer` label with the actual value** by dereferencing
  the non-null pointer: `return std::to_string(*ptr)` (requires `<string>`,
  which is `≤ Ch 5` — in scope).
- **Add `int* findFirstPtr(int*, std::size_t len, int target)`** that returns a
  pointer (not `std::optional`) to the first match in a raw C-array of given
  length, or `nullptr` if not found. This is a preview of pointer-as-cursor
  (formally Ch 17 pointer arithmetic), so label it as such if you attempt it.
