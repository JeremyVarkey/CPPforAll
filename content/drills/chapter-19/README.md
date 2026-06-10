# Chapter 19 — Dynamic Allocation: Project: DynBuffer Workbench

> Reinforces [`../../notes/chapter-19.md`](../../notes/chapter-19.md) · LearnCpp [Chapter 19](https://www.learncpp.com/)

## The project

You are building the **DynBuffer Workbench** — a small library of five free
functions that manage a raw `int` array on the heap using `new[]` and
`delete[]`.  No `std::vector`, no smart pointers, no `malloc`.  Just you,
the heap, and a pointer.

The point is to feel the fragility firsthand: every `new[]` needs a matching
`delete[]`, every function must leave no dangling address in the caller's
pointer variable, a deep copy means two independent heap allocations (not two
pointers to the same array), and "resize" means allocate-copy-delete-redirect.
Once you have done all of that by hand, you will understand exactly what
`std::vector` automates for you — and why LLVM's `SmallVector` exists.

This is also the lab where **references to pointers** (ch-12) pay off in a
visible way: `destroyBuffer` and `resizeBuffer` take `int*&` so they can write
`nullptr` or a new address back into the caller's variable.  If they took
`int*` by value, the caller's pointer would never change.

## Concepts practiced

- **`new[]` / `delete[]`** — array form of dynamic allocation (notes 19.2)
- **`new` / `delete`** (scalar form, context) — distinct from array form; mixing them is UB (notes 19.2)
- **Dangling pointers** — what happens after `delete[]` if you don't null out (notes 19.1)
- **Memory leaks** — losing the only owning pointer before cleanup (notes 19.1)
- **Null-out-after-delete** — the defensive habit that makes bugs detectable (notes 19.1)
- **Deep copy vs shallow copy** — two separate allocations vs one aliased pointer (notes 19.2)
- **The resize algorithm** — allocate-copy-delete-redirect (notes 19.2 "Resizing means allocating a new array")
- **Owning vs observing pointers** — `int*` owns, `const int*` observes (notes 19.1)
- Reused from ch-12: **ref-to-pointer out-param** (`int*&`) — the mechanism that lets a function write back a new pointer to the caller's variable
- Reused from ch-08: **`for` loops** for array traversal; range bounds via a separate `int n` parameter
- Reused from ch-02: **header / `.cpp` split** — declarations in `dynbuf.h`, definitions in `dynbuf.cpp`
- CS6340 tie-in: reading LLVM C-style pointer+length signatures — `void foo(const int* buf, int n)` — as exactly this contract

## Your tasks

The starter compiles and runs immediately, but every function is a
**wrong-but-compiling placeholder**, so `make test` starts **RED**.  Fill in
the five `>>> YOUR CODE HERE <<<` blocks in
[`starter/dynbuf.cpp`](starter/dynbuf.cpp).  They ramp from straightforward to
the stretch:

1. **`makeBuffer(n, fill)`** — allocate a heap array of `n` ints with `new[]`,
   fill every element with `fill` in a loop, and `return` the owning pointer.
   Precondition: `n >= 0`.

2. **`destroyBuffer(p)`** — call `delete[]` on `p`, then assign `nullptr` to
   `p` **through the reference**.  The `int*&` signature is what makes the
   write-back possible; without the `&` the caller's pointer would never see
   the change.  Precondition: `p` is a valid owning pointer or `nullptr`.

3. **`cloneBuffer(src, n)`** — **deep copy**: allocate a fresh array of `n`
   ints, copy `src[0..n-1]` element-by-element, and return the new owning
   pointer.  After cloneBuffer, mutating either array must NOT affect the other.
   Precondition: `src != nullptr` if `n > 0`.

4. **`resizeBuffer(p, oldN, newN)`** — the four-step algorithm:
   (1) `new[]` a fresh array of `newN` ints (value-init to 0).
   (2) Copy `min(oldN, newN)` elements from the old array into the new one.
   (3) `delete[]` the OLD array.
   (4) Set `p` to point at the new array through the reference.
   If `newN > oldN`, the extra slots are already 0.  If `newN < oldN`, only
   the prefix survives.  **Order matters:** step 2 must happen before step 3
   because you read from the old allocation in step 2.

5. **`bufferSum(buf, n)`** — loop over `buf[0..n-1]` and return the sum.  No
   allocation, no deallocation.  The `const int*` parameter is the
   machine-readable promise that you won't modify the array.

You may **not** edit `dynbuf.h` or anything in `tests/` — those are the
contract and the grader.

## Constraints

**Allowed:**
- `new[]` / `delete[]` (the chapter's new tools)
- `new` / `delete` scalar form (allowed, but you don't need them here)
- `for` loops, `int` arithmetic, `nullptr`
- `int*&` (ref-to-pointer out-param, ch-12)
- `std::vector` in your own test experiments only — not inside the library functions

**Forbidden (not taught yet or defeats the point):**
- `std::unique_ptr` / `std::shared_ptr` (Chapter 22) — not yet, and they hide the point
- `malloc` / `free` / `realloc` — these are C-style allocators, not the C++ `new` path
- Placement `new` — beyond scope
- `std::vector` inside the library functions — that defeats the whole exercise

**Required idioms:**
- Every `new[]` has exactly one matching `delete[]`
- Set pointers to `nullptr` after `delete[]` through the ref-param where appropriate
- Pass `n >= 0` as a precondition; do not add `if (n < 0)` guards (out of scope)
- Deep copy in `cloneBuffer`: two independent heap allocations, not pointer aliasing

## Build & run

```sh
make          # compile-check starter/dynbuf.cpp (already works)
make test     # grade your code  ->  RED until the TASK blocks are filled in
make solution # build + run the reference against the tests
make test-solution  # proof the reference passes
make clean    # remove build artifacts
```

The grader compiles `tests/tests.cpp` together with your `starter/dynbuf.cpp`
and runs it.  There is no interactive driver (`make run` just prints a reminder)
because a raw-pointer library is not safely runnable without a real harness.

## Success criteria

`make test` prints **PASS ✅  all dynbuf checks passed.** and exits 0.  Until
then it prints one `FAIL: …  @line N` per broken check.

The grader covers every function and several edge cases that separate a correct
implementation from a plausible-looking near-miss:

- `makeBuffer(5, 7)` — all five elements must be **7**, not 0 (tests the fill loop)
- `makeBuffer(0, 42)` — length-0 allocation must not crash (legal `new int[0]{}`)
- `destroyBuffer(buf)` → `buf == nullptr` — the **ref write-back** must happen
- `destroyBuffer(nullptr)` — must be a **no-op**, not a crash
- Clone independence: mutate the original after `cloneBuffer` → clone is **unchanged** (deep copy)
- Shrink `resizeBuffer(buf, 5, 3)` → only the **prefix** of 3 elements survives
- Grow `resizeBuffer(buf, 3, 6)` → new slots are **0**, old prefix intact
- `resizeBuffer(buf, 3, 0)` → shrink to 0; `destroyBuffer` then nulls the pointer
- `bufferSum` with all-zeros (length 0), negatives, and mixed-sign arrays
- Integration: clone → resize original → both arrays remain **independent**

## Hints

<details><summary>Task 1 — makeBuffer (allocate + fill)</summary>

```cpp
int* buf { new int[n]{} };   // heap-allocate n ints, zero-init (notes 19.2)
for (int i { 0 }; i < n; ++i)
    buf[i] = fill;
return buf;
```

`new int[n]{}` value-initialises all elements to 0 (the `{}` is the key).
You overwrite them with `fill` in the loop.  The caller gets the pointer and
owns the cleanup — make sure you document that expectation (the header already
does).
</details>

<details><summary>Task 2 — destroyBuffer (delete[] + null-out)</summary>

```cpp
delete[] p;   // release heap storage (notes 19.2)
p = nullptr;  // null out through the ref — prevents dangling (notes 19.1)
```

The `int*&` in the signature is what makes `p = nullptr` write into the
**caller's variable**.  If the parameter were `int*` (by value), assigning
`nullptr` would only change a local copy — the caller's pointer would still
hold the freed address (a dangling pointer).

No `if (p)` guard needed: `delete[] nullptr` is defined as a no-op.
</details>

<details><summary>Task 3 — cloneBuffer (deep copy)</summary>

```cpp
int* dst { new int[n]{} };
for (int i { 0 }; i < n; ++i)
    dst[i] = src[i];
return dst;
```

The classic mistake is `int* dst { src }` — that copies the **pointer** (a
shallow copy).  Both `src` and `dst` now point at the **same** memory.  The
first `delete[]` frees it; the second `delete[]` is a double-free — undefined
behaviour.  The deep copy above allocates separate memory so the two arrays are
fully independent.
</details>

<details><summary>Task 4 — resizeBuffer (the four steps, ORDER matters)</summary>

```cpp
int* fresh { new int[newN]{} };                    // Step 1: new allocation
int copyCount { (oldN < newN) ? oldN : newN };     // min(oldN, newN)
for (int i { 0 }; i < copyCount; ++i)
    fresh[i] = p[i];                               // Step 2: copy from OLD p
delete[] p;                                        // Step 3: free OLD allocation
p = fresh;                                         // Step 4: redirect caller's ptr
```

**Why the order matters:** step 2 reads from `p` (the old array).  If you do
step 3 before step 2, you read from freed memory — undefined behaviour.  Save
the new pointer in `fresh`, copy, then delete the old, then redirect.

The `(oldN < newN) ? oldN : newN` idiom is `min(oldN, newN)` without including
`<algorithm>` — fine at this chapter level.
</details>

<details><summary>Task 5 — bufferSum (pure observer)</summary>

```cpp
int total { 0 };
for (int i { 0 }; i < n; ++i)
    total += buf[i];
return total;
```

The `const int*` parameter is a compile-enforced promise that you won't write
through `buf`.  If you accidentally write `buf[i] = ...`, the compiler will
reject it — that's the whole point of `const` on a pointer.
</details>

<details><summary>Stuck on a warning or a build error instead of a test failure?</summary>

- *"unused parameter"* — if your placeholder comments out a parameter name
  (e.g. `int /*n*/`), that suppresses the warning without changing behaviour.
  Once you write the real body, un-comment the name.
- *"deleting a pointer to incomplete type"* — you called `delete` (scalar) on
  something allocated with `new[]` (array).  Use `delete[]`.
- *"bus error" or segfault* — an out-of-bounds access or a double-free.  Check
  that your `makeBuffer` returns an array of size `n`, not 0.
- *Clone independence test fails* — you returned `src` directly instead of
  allocating a new array.  See Task 3 hint.
</details>

## Stretch goals (optional — some need later chapters)

- Wrap the five functions into a **class** `DynBuffer` with a constructor and
  a **destructor** that calls `delete[]` automatically — that's RAII (notes
  19.3), and it is exactly what `std::vector` does under the hood.  (Needs
  Chapter 14/15.)
- Add a **copy constructor** and **copy-assignment operator** to `DynBuffer`
  that perform deep copies (notes 19.2).  Without them, copying a `DynBuffer`
  would do a shallow copy of the pointer — a double-free waiting to happen.
  (Needs Chapter 14; the full "Rule of Three/Five" is Chapter 22.)
- Try running `make test` under **Valgrind** (`valgrind --leak-check=full
  ./tests/run`) or with **AddressSanitizer** (`-fsanitize=address` added to
  CXXFLAGS).  A correct solution reports zero leaks and zero invalid accesses.
  The starter's `resizeBuffer` placeholder deliberately leaks — watch it show
  up.
- Replace the free functions with a `std::vector<int>` implementation and
  observe how all the bookkeeping disappears — that's the *lesson*, made
  concrete.
- Add a `bufferFind(const int* buf, int n, int target)` function that returns
  the index of the first occurrence of `target`, or `-1` if not found.
  (Pure observer, same pattern as Task 5; uses `std::find` from Chapter 18 if
  you want to compare.)
