# Chapter 15 — More on Classes: IdCard Badge Printer

> Reinforces [`../../notes/chapter-15.md`](../../notes/chapter-15.md) · LearnCpp [Chapter 15](https://www.learncpp.com/)

## The project

You are building an `IdCard` class — a simple employee-badge system — but **the
right way for a real codebase**: the class *declaration* lives in `idcard.h`
(provided, complete, do not edit), and you implement every member body in
`starter/idcard.cpp` using the `ClassName::method()` scope-resolution syntax.

Along the way, five Chapter-15 concepts become physical rather than abstract:

- The `.h`/`.cpp` member split (notes 15.2) — the layout every production C++
  project uses; exactly how LLVM headers like `llvm/IR/Value.h` are organised.
- `this` used explicitly in the setters — the idiomatic `this->m_x = x;` form
  from notes 15.1 (a clarity convention here; see Task 3 for when it is mandatory).
- Method chaining — `setOwner`/`setTitle` return `IdCard&` so calls can chain:
  `card.setOwner("Ada").setTitle("Eng").setRole(IdCard::Role::Engineer);`
- Static members — `s_liveCount` and `s_nextSerial` track counts and auto-issue
  serial numbers class-wide, not per-object (notes 15.6, 15.7).
- Destructors (notes 15.4) — the destructor decrements `s_liveCount`; tests
  create cards in **nested `{ }` scopes** so you can *see* the count fall exactly
  when each scope closes. No heap, no `new` — pure automatic-storage RAII.

## Concepts practiced

- **Out-of-class member definitions** with `ClassName::method()` syntax (notes 15.2)
- **`this` pointer** — explicit `this->member = param;` idiom (notes 15.1)
- **Returning `*this` by reference** for method chaining (notes 15.1)
- **Static member variables** shared across all instances (notes 15.6)
- **Static member functions** — `liveCount()` / `nextSerial()`, no `this` (notes 15.7)
- **Destructor** — `~IdCard()` for deterministic cleanup / bookkeeping (notes 15.4)
- **Nested type** — `IdCard::Role` enum class lives inside the class (notes 15.3)
- **Default arguments** declared once in the header, absent from the `.cpp` (notes 15.2)
- Reused from earlier: **constructors / member-init lists** (Ch 14), **`const` member
  functions** (Ch 14), **`std::string` / `std::string_view`** (Ch 5), **`enum class`**
  (Ch 13), **header guards** (Ch 2)

## Your tasks

The starter compiles immediately, but every function returns a **placeholder**, so
`make test` starts **RED**. Fill in the five `>>> YOUR CODE HERE <<<` blocks in
[`starter/idcard.cpp`](starter/idcard.cpp). They ramp from warm-up to the chaining
proof:

1. **Constructor — serial assignment + live count** (notes 15.2, 15.6).
   Initialise `m_owner` and `m_title` with a member-initialiser list. In the body,
   assign `s_nextSerial` to `m_serial`, then increment `s_nextSerial` (so the next
   card gets a different number), and increment `s_liveCount`. Use the
   `IdCard::IdCard(...)` out-of-class syntax. Do **not** repeat the default
   arguments from the header.

2. **Destructor — decrement the live count** (notes 15.4).
   Write `~IdCard()` in out-of-class form. The only job: `--s_liveCount`. Tests
   create cards in nested `{ }` scopes and verify the count falls as each scope
   closes — destructors made **physical** without `new` or `delete`.

3. **Setters with explicit `this` + method chaining** (notes 15.1).
   Implement `setOwner`, `setTitle`, and `setRole`. Write `this->m_xxx = xxx;` —
   the explicit-`this` idiom from notes 15.1 (use these exact parameter names:
   `owner`, `title`, `role`). Because members here are `m_`-prefixed, the parameter
   `owner` and member `m_owner` are *different* names, so `this->` is a clarity
   convention, not a requirement — it only becomes mandatory when a parameter has
   the **same** name as a member. Return `*this` by reference (`IdCard&`) so callers
   can chain. The grader takes the *address* of the returned reference and confirms
   it equals `&card` — the proof that no copy was made.

4. **Const accessors** (Ch 14 — reused). Implement `owner()`, `title()`,
   `role()`, and `serial()` — each `const`, returning the matching private member.
   They are called on a `const IdCard&` in the tests, so `const` on the function
   signature is required.

5. **Static member functions** (notes 15.7). Implement `liveCount()` and
   `nextSerial()`. They have no `this` pointer and may only read the static
   members `s_liveCount` and `s_nextSerial`. Remember: `static` appears in the
   **declaration** (in `.h`) only — do not write it in the definition.

You may **not** edit `idcard.h` or anything in `tests/` — those are the contract
and the grader.

## Constraints

**Allowed:** `class`, member-init lists, `const` member functions, `std::string`,
`std::string_view`, `enum class` (Ch 13), `static` members and static member
functions, `this`, `*this`, header guards, the full Chapter ≤ 14 toolkit.

**Forbidden (not taught yet):** `operator<<` overloading (Ch 21), `virtual` /
`override` (Ch 25), `new`/`delete` (Ch 19), `std::vector` (Ch 16). No `static`
keyword in out-of-class *definitions* (it belongs only in the *declaration*).

**Required idioms:**
- Every out-of-class definition uses `ClassName::method(...)` scope resolution.
- Default arguments appear in the header declaration **only** (notes 15.2).
- `setOwner`/`setTitle`/`setRole` must return `IdCard&` (reference, not a copy).
- Use `this->m_owner = owner;` style in the setters — the explicit-`this` idiom
  from notes 15.1. (Here it is a clarity convention, since `m_owner` and `owner`
  differ; it is *required* only when a parameter and member share a name.)
- `static` omitted from the `.cpp` definitions of `liveCount()`/`nextSerial()`.

## Build & run

```sh
make            # compile-check starter/idcard.cpp (warning-clean)
make test       # grade your code  ->  RED until the TASK blocks are filled in
make solution   # run the grader against the reference (see what GREEN looks like)
make test-solution  # same as solution but an explicit green-proof step
make clean      # remove build artifacts
```

## Success criteria

`make test` prints **PASS ✅ all badge-printer checks passed.** and exits 0.

Until then it prints one `FAIL: … @line N` per broken check. The most revealing:

- `liveCount() == 0` before any object exists — static init working.
- `serial() == 1` on the first card; `serial() == 2` on the second — auto-issue.
- After a `{ }` block closes, `liveCount()` falls by exactly the number of
  cards that lived in that block — destructor + `--s_liveCount` working.
- `&returned == &card` — `setOwner` returns a reference to the **same** object,
  not a copy (method chaining identity test).
- Chained `card.setOwner("Alan Turing").setTitle("Cryptanalyst").setRole(…)` leaves
  the card with all three fields correctly set.
- Deep three-level nesting: count goes 0 → 1 → 2 → 3 → 2 → 1 → 0 deterministically.

## Hints

<details><summary>Task 1 — constructor syntax and static bookkeeping</summary>

Out-of-class constructor syntax:

```cpp
IdCard::IdCard(std::string_view owner, std::string_view title)
    : m_owner { owner }, m_title { title }   // member-init list
{
    m_serial = s_nextSerial;   // claim this card's number
    ++s_nextSerial;            // advance the dispenser
    ++s_liveCount;             // one more live card
}
```

`s_nextSerial` and `s_liveCount` are `static` members — they belong to the class,
not to `*this`, so you access them by name with no prefix (or `IdCard::s_…`).
</details>

<details><summary>Task 2 — destructor shape</summary>

```cpp
IdCard::~IdCard()
{
    --s_liveCount;
}
```

No return type, no parameters. The `~` prefix is the entire signature. C++ calls
this automatically when the object's lifetime ends — you never call it directly.
</details>

<details><summary>Task 3 — explicit `this` and returning *this</summary>

```cpp
IdCard& IdCard::setOwner(std::string_view owner)
{
    this->m_owner = owner;   // explicit-this idiom (m_owner != owner, so optional)
    return *this;            // *this is the object; return by reference (IdCard&)
}
```

`this` is a pointer (`IdCard*`). `*this` dereferences it to the object itself.
Returning by reference means the caller gets back the same object — not a copy.
Without the `&` in the return type, a copy would be made and chaining would
operate on a temporary, never reaching the original card.
</details>

<details><summary>Task 4 — const accessors</summary>

```cpp
std::string_view IdCard::owner() const { return m_owner; }
std::string_view IdCard::title() const { return m_title; }
IdCard::Role     IdCard::role()  const { return m_role;  }
int              IdCard::serial() const { return m_serial; }
```

The `const` after `()` matches the header declaration and allows calling these on
a `const IdCard&`. Outside the class, note that `Role` must be qualified:
`IdCard::Role` — it is a nested type (notes 15.3).
</details>

<details><summary>Task 5 — static member functions (no `this`)</summary>

```cpp
int IdCard::liveCount()   { return s_liveCount;   }
int IdCard::nextSerial()  { return s_nextSerial;   }
```

Two things to remember: (a) do **not** write `static` in the definition — it
belongs only in the declaration; (b) there is no implicit object, so only static
members are accessible by name.
</details>

## Stretch goals (optional — most use later chapters)

- Add a `static void resetSerials()` that resets `s_nextSerial` to 1 — useful for
  test isolation. (Stays within Ch 15 scope; calls the chapter's static-function
  lesson from the other direction.)
- Add a `friend std::ostream& operator<<(std::ostream&, const IdCard&)` that
  prints `"[123] Ada Lovelace — Chief Analyst (Engineer)"` (notes 15.8 — a
  preview; `operator<<` is formally Chapter 21).
- Make `IdCard` non-copyable by `= delete`ing the copy constructor and copy
  assignment operator (Ch 14 / 22) — prevents accidental `s_liveCount`
  mis-counting when a card is copied.
- Store all live cards in a `static std::vector<IdCard*>` registry and add a
  static `printAll()` that lists every live card — a preview of `std::vector`
  (Ch 16) and the LLVM-style "context owns all Values" pattern (Ch 25).
