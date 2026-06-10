# Chapter 20 — Functions II: the `calc-core` Lab

> Reinforces [`../../notes/chapter-20.md`](../../notes/chapter-20.md) · LearnCpp [Chapter 20](https://www.learncpp.com/)

## The project

`calc-core` is a small arithmetic library built in **three movements**, each
targeting one of Chapter 20's central ideas:

**Movement 1 — Recursion** (notes 20.3).  You implement `factorial`, a
recursive digit-sum (contrasted with Chapter 8's loop version), and a naive
`fibonacci` that makes the *inefficiency* of recursive overlap visible.  Each
function forces you to identify the **base case** and the **recursive case** —
the two-part anatomy that every recursive function requires.

**Movement 2 — Function pointers** (notes 20.1).  You implement `apply`,
which receives a binary operation as a **function pointer** and calls it —
the classic "pass behavior as a parameter" pattern.  You also implement
`pickOp`, which maps a character to the *address* of the right operation
function and **returns that pointer**, giving you a tiny "jump table" whose
two steps (dispatch and execute) are decoupled by design.

**Movement 3 — Lambdas and captures** (notes 20.6, 20.7).  You implement
`countMatching`, which accepts any callable — a plain function pointer, a
non-capturing lambda, or a **capturing lambda** that closes over a local
variable — through a `std::function<bool(int)>` parameter.  The capturing-
lambda test is the payoff: it is the one that would fail to compile if the
parameter were a raw function pointer, because capturing lambdas are closure
objects, not free functions.

An ungraded [`demo.cpp`](demo.cpp) (built by `make run`) ties all three
movements together in one program and shows CLI argument handling (notes 20.4)
while it's there.

## Concepts practiced

- **Recursion** — base case / recursive case anatomy, call-stack growth,
  depth limits (notes 20.3)
- **Function pointers** — type syntax `int (*name)(int, int)`, passing as
  parameter (`apply`), returning from a function (`pickOp`), null-pointer
  guard (notes 20.1)
- **`using` type alias** for a function-pointer type (`BinaryIntOp`) — makes
  gnarly pointer syntax readable (notes 20.1, Ch 10 reuse)
- **Lambdas** — unnamed function objects, non-capturing and capturing forms
  (notes 20.6, 20.7)
- **Lambda captures by value** — `[threshold](int x){ … }` captures a local
  variable; the lambda stores its own copy (notes 20.7)
- **`std::function`** — type-erased callable wrapper that accepts pointers,
  non-capturing lambdas, and capturing lambdas uniformly (notes 20.1, 20.6)
- Reused from earlier chapters: `std::vector` and range-`for` (Ch 16), `using`
  aliases (Ch 10), header guards and multi-file layout (Ch 2), `long long`
  arithmetic (Ch 4), control flow (`if`/`switch`) (Ch 8)

## Your tasks

The starter compiles immediately but every function returns a **wrong
placeholder**, so `make test` starts **RED**. Fill in the six
`>>> YOUR CODE HERE <<<` blocks in
[`starter/calccore.cpp`](starter/calccore.cpp). They ramp from warm-up to
capstone:

1. **`factorial(n)` — basic recursion.**  Return `n!` recursively.  Your base
   case must be `n == 0` returning `1`.  Your recursive case returns
   `n * factorial(n - 1)`.  Return type is `long long` so results up to
   `factorial(20)` don't overflow.

2. **`sumDigitsRecursive(n)` — same job as the Chapter 8 loop, now recursive.**
   Chapter 8's `sumOfDigits` used a `while` loop; here you use recursion
   instead.  Base case: `n < 10` (single digit; return `n`).  Recursive case:
   `(n % 10) + sumDigitsRecursive(n / 10)`.  Handle negative `n` by working on
   `|n|`: call yourself with `-n` when `n < 0`.

3. **`fibonacci(n)` — observe the exponential call tree.**  Two base cases:
   `fibonacci(0) == 0`, `fibonacci(1) == 1`.  Recursive case:
   `fibonacci(n-1) + fibonacci(n-2)`.  Notes 20.3 flag this pattern as
   INEFFICIENT (exponential calls from overlapping sub-problems) — keep test
   values at `n <= 20`.

4. **`apply(a, b, op)` — call through a function pointer.**  Dispatch to the
   operation pointed-to by `op`: `return op(a, b);`.  **Guard first**: if `op`
   is `nullptr`, return `0` without calling (calling a null function pointer
   is undefined behavior).

5. **`pickOp(c)` — RETURN a function pointer (mini jump table).**  Map a
   character to the address of the matching free function and return it.
   `'+'` → `&addOp`, `'-'` → `&subOp`, `'*'` → `&mulOp`, anything else →
   `nullptr`.  Use a `switch` or `if/else` chain.  The caller passes the result
   straight to `apply`.

6. **`countMatching(values, predicate)` — count elements satisfying a
   callable.** Iterate over `values` with a range-`for` loop.  For each element
   `v`, call `predicate(v)`; if it returns `true`, increment a counter.  Return
   the count.  The predicate is `std::function<bool(int)>` — it must accept
   **both** a plain function pointer (`&isEven`) and a **capturing lambda**
   (`[threshold](int x){ return x >= threshold; }`).

You may **not** edit [`calccore.h`](calccore.h) or anything in `tests/` — those
are the contract and the grader.

## Constraints

**Allowed:** recursion (`if` for base case, recursive call for recursive case);
raw function pointers (`BinaryIntOp` / `int (*)(int, int)`); `std::function`;
lambdas with value captures (`[var](...){ ... }`); `std::vector`; range-`for`;
`switch`/`if`; `long long`; `<functional>` (already included via `calccore.h`).

**Forbidden (not taught yet or out of scope):**
- `std::bind` (not taught in Ch 20, inferior to lambdas)
- variadic templates or generic `auto` lambdas as *required* syntax (mentioned
  in notes 20.6 but not required here — `std::function` is the correct tool for
  the parameter type)
- smart pointers (Ch 22)
- `goto`

**Required idioms (from notes 20.3):** every recursive function must have a
clearly reachable base case; every recursive call must move strictly toward that
base case.

**Recursion depth:** keep `n <= 20` for `factorial` and `fibonacci` in any
code you add — this avoids stack overflow on reasonable systems.

## Build & run

```sh
make            # compile-check starter/calccore.cpp + build the demo
make run        # run the ungraded demo  (shows all three movements live)
make test       # grade your code  -> RED until TASK blocks are filled in
make solution   # build + run the reference demo
make clean
```

`make test` is the real goal — it compiles the grader together with your
`starter/calccore.cpp` and exits non-zero on any failure.

## Success criteria

`make test` prints **PASS ✅  all calc-core checks passed.** and exits 0.

Until then it prints one `FAIL: … @line N` per broken check and ends with
`FAIL ❌  N check(s) failed`. Key edge cases in the grader:

- `factorial(0) == 1` — the **base case** (0! is 1, not 0)
- `sumDigitsRecursive(0) == 0` and `sumDigitsRecursive(7) == 7` — single-digit
  base cases land in the right branch
- `sumDigitsRecursive(-123) == 6` — **magnitude** handling for negatives
- `fibonacci(0) == 0` and `fibonacci(1) == 1` — **both** base cases are correct
- `apply(10, 3, nullptr) == 0` — the **null guard** must not crash the program
- `pickOp('?') == nullptr` — the **default** case returns null for unknowns
- `apply(6, 2, pickOp('/')) == 0` — full null-dispatch path is safe end-to-end
- The **capturing-lambda test** for `countMatching` with `threshold` captured
  from a local variable — this is the test that requires `std::function`

## Hints

<details><summary>Task 1 — factorial: anatomy of a recursive function</summary>

```cpp
long long factorial(int n)
{
    if (n == 0)                   // BASE CASE — no call, direct answer
        return 1;
    return n * factorial(n - 1);  // RECURSIVE CASE — smaller sub-problem
}
```

Mental model: `factorial(3)` waits for `factorial(2)`, which waits for
`factorial(1)`, which waits for `factorial(0)` → 1. Then answers unwind in
reverse: 1, 1, 2, 6.  The call stack grows one frame per call, so depth == n.
</details>

<details><summary>Task 2 — sumDigitsRecursive vs. the Chapter 8 loop</summary>

Chapter 8 loop version:
```cpp
int sumOfDigits(int n) {
    if (n < 0) n = -n;
    int sum { 0 };
    while (n > 0) { sum += n % 10; n /= 10; }
    return sum;
}
```

Recursive version:
```cpp
int sumDigitsRecursive(int n)
{
    if (n < 0) return sumDigitsRecursive(-n);  // magnitude
    if (n < 10) return n;                      // BASE CASE: single digit
    return (n % 10) + sumDigitsRecursive(n / 10);  // RECURSIVE
}
```

Both peel digits with `% 10` / `/ 10`. The loop uses a counter variable to
track progress; the recursion shrinks the problem through the argument.
The maximum call depth equals the number of digits — at most 10 for a 32-bit
int — so this recursion is shallow and safe.
</details>

<details><summary>Task 3 — why naive recursive fibonacci is slow</summary>

`fibonacci(5)` calls `fibonacci(4)` AND `fibonacci(3)`. `fibonacci(4)` calls
`fibonacci(3)` AND `fibonacci(2)`. Notice: `fibonacci(3)` is computed TWICE.
At `n=20` this duplication reaches ~6765 calls. This is the "exponential call
tree" the notes warn about (notes 20.3 — "Recursion can be inefficient").
The correct recursive skeleton is:
```cpp
int fibonacci(int n) {
    if (n == 0) return 0;
    if (n == 1) return 1;
    return fibonacci(n - 1) + fibonacci(n - 2);
}
```
Keep test values ≤ 20 to avoid waiting. An iterative loop would be O(n)
instead of O(2^n).
</details>

<details><summary>Task 4 — calling through a function pointer</summary>

A function pointer is called just like a regular function:
```cpp
int apply(int a, int b, BinaryIntOp op)
{
    if (!op)           // null guard — ALWAYS check before calling
        return 0;
    return op(a, b);   // call through the pointer
}
```
`BinaryIntOp` is the alias from `calccore.h` for `int (*)(int, int)`. Reading
the raw form: "`op` is a pointer to a function that takes two `int`s and
returns `int`." The `*op` grouping is mandatory for this meaning — without the
parentheses, `int* op(int, int)` means "function returning `int*`" instead.
</details>

<details><summary>Task 5 — returning a function pointer from a function</summary>

```cpp
BinaryIntOp pickOp(char c)
{
    switch (c)
    {
    case '+': return &addOp;
    case '-': return &subOp;
    case '*': return &mulOp;
    default:  return nullptr;
    }
}
```
You return the **address** of the function. The caller gets a `BinaryIntOp`
back and passes it to `apply` — the dispatch and the execution are decoupled.
This two-step pattern (get pointer, then call through it) is common in
dispatch tables, plugin loaders, and LLVM pass pipelines.
</details>

<details><summary>Task 6 — why std::function, and what a capture looks like</summary>

A raw function pointer (`bool (*)(int)`) can only point to **plain, free
functions**.  A capturing lambda like `[threshold](int x){ return x >= threshold; }`
is a *closure object* — the compiler generates a class with a data member for
`threshold`.  A raw pointer can't hold it.

`std::function<bool(int)>` uses **type erasure** to store any callable with the
right call signature — plain pointers, non-capturing lambdas, and capturing
lambdas all work.

```cpp
int countMatching(const std::vector<int>& values,
                  std::function<bool(int)> predicate)
{
    int count { 0 };
    for (int v : values)
        if (predicate(v)) ++count;
    return count;
}
```

Calling it with a capturing lambda:
```cpp
int threshold { 7 };
int n { countMatching(values, [threshold](int x){ return x >= threshold; }) };
```
The lambda captures `threshold` **by value** — it stores its own copy, so even
if `threshold` goes out of scope, the lambda's copy is safe.
</details>

## Stretch goals (optional — some need later chapters)

- Add `power(base, exp)` using recursion: `power(b, 0) == 1`,
  `power(b, n) == b * power(b, n-1)`.  Observe the depth is O(exp).
- Make `fibonacci` non-exponential by adding memoisation with a `static
  std::vector<int>` cache (notes 20.3 — "a loop or memoisation is usually
  better").
- Add a `divOp` and `modOp` to the function-pointer movement and update
  `pickOp` to dispatch `'/'` and `'%'` (add a zero-denominator guard).
- In `countMatching`, measure the overhead of `std::function` vs. a template
  parameter: replace `std::function<bool(int)>` with `template <typename Pred>`
  and benchmark the two (template version — a preview of Ch 26 — must live in
  the header, so you'd move the body to `calccore.h`).
- Extend the demo to accept multiple integers on the command line and print a
  table of factorial / fibonacci / digit-sum for each (needs `argc` / `argv`
  loop iteration — notes 20.4).
