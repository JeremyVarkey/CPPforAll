# Chapter 4 — Fundamental Data Types · Project: Numeric Types Lab

> Reinforces [`../../notes/chapter-04.md`](../../notes/chapter-04.md) · LearnCpp [Chapter 4](https://www.learncpp.com/)

## The project
The fundamental types — `int`, `unsigned`, the fixed-width integers, `double`,
`bool`, `char` — all *look* like plain numbers until they bite you: a counter
wraps to zero, `0.1 + 0.2` refuses to equal `0.3`, a negative index turns into a
gigantic positive one. This lab makes that behavior **physical**. You implement
six small functions, and the grader proves the types behave exactly as the notes
warned — overflow wraps, chars are numbers in disguise, floats are approximate,
and signed→unsigned casts need a guard.

Each function isolates one idea from Chapter 4, and the tasks **ramp up**: Task 1
is a one-liner about `sizeof`; Task 6 is a small `if`/`else-if` classifier that
ties bool logic and branches back to the CS6340 "why" (every `if` is a
control-flow path that coverage analysis measures). The tests assert only
**platform-independent facts** — fixed-width sizes and relationships, never an
absolute `sizeof(int)` — so a green result means the same thing on every machine.

## Concepts practiced
- **`sizeof`** and the fixed-width integers `std::int32_t` / `std::uint8_t` (4.3, 4.6)
- **Unsigned wraparound** — defined, modular, and surprising (4.5)
- **Floating-point approximation** and epsilon comparison instead of `==` (4.8)
- **`bool`** values and `std::boolalpha` output (4.9)
- **`char` ↔ `int`** round-tripping; chars are numeric underneath (4.11)
- **`static_cast`** for explicit, searchable conversions; the negative-to-`size_t` trap (4.12)
- **`if` / `else if` / `else`** chains and ordered conditions (4.10)
- *Reused from earlier chapters:* user-defined **functions** across a `.h`/`.cpp` split with a **header guard**, and `return` (Ch 2); arithmetic **expressions** (Ch 1)

## Your tasks
The starter compiles and the demo runs immediately — but every function returns a
deliberately wrong placeholder, so `make test` starts **RED**. Fill in the six
`>>> YOUR CODE HERE <<<` blocks in
[`starter/numeric_lab.cpp`](starter/numeric_lab.cpp). Each maps 1:1 to a `TASK`
block there and to a declaration in [`numeric_lab.h`](numeric_lab.h) (read those
comments first — they are the contract).

1. **`int32ByteWidth()`** — return how many **bytes** a `std::int32_t` occupies.
   Use the `sizeof` operator (it already yields a `std::size_t`).
2. **`shiftChar(char ch, int delta)`** — shift `ch` by `delta` in its encoding.
   Convert char→int, add, convert int→char; use `static_cast` for **both**.
   E.g. `shiftChar('A', 1) == 'B'`.
3. **`wrapsAround(uint8_t start, uint8_t addend)`** — return `true` exactly when
   `start + addend` overflows an 8-bit unsigned (true sum `> 255`). Observe the
   real wrap; don't use signed overflow.
4. **`nearlyEqual(double a, double b, double epsilon)`** — return `|a - b| <= epsilon`.
   Never compare doubles with `==`.
5. **`isValidIndex(int index, size_t length)`** — return `true` iff
   `0 <= index < length`. **Check `index >= 0` first**, then `static_cast` to
   `std::size_t` and compare with `length`.
6. **`classifyNumber(double value)`** *(capstone)* — return a `char` tag using an
   ordered `if`/`else-if` chain: `'Z'` if (nearly) zero, `'N'` if negative,
   `'B'` if `>= 1000.0`, `'P'` otherwise. Reuse your `nearlyEqual` for the zero test.

## Constraints
- **Allowed:** `int` / `unsigned` / fixed-width ints, `float`/`double`, `bool`,
  `char`, `sizeof`, `static_cast`, and `if` / `else` (4.10). `std::abs` from
  `<cmath>` is already included for you.
- **Forbidden (not taught yet):** loops, `switch`, `std::string`, `?:`, `&&`/`||`
  chained logic — none are needed. Solve every task with the tools above.
- **Always `static_cast`** for conversions you intend (char↔int, signed→`size_t`).
  No C-style casts.
- **Guard before you cast** a possibly-negative `int` to `std::size_t` (Task 5).
- Do **not** edit `numeric_lab.h`, the tests, or the demo — only fill in
  `starter/numeric_lab.cpp`.

## Build & run
```sh
make           # compile starter/ + demo  ->  starter/app
make run       # run the demo: see your functions' behavior printed
make test      # grade your code (unit tests) — RED until the TASKs are done
make solution  # build + run the reference demo if you get stuck
make clean
```

## Success criteria
`make test` prints **PASS ✅  all checks**. It compiles
[`tests/tests.cpp`](tests/tests.cpp) against your `starter/numeric_lab.cpp` and
runs ~30 `CHECK`s covering every function **plus edge cases** — the `255 + 1`
overflow boundary, the `0.1 + 0.2 != 0.3` float trap, a negative index, and the
ordering of the classifier. Until you fill in the tasks, the placeholders fail
and you'll see `FAIL: <expr> @line N` for each, then **FAIL ❌**. Turning that red
into green is the whole exercise. (Our proof it's solvable:
`make test-solution` is green.)

## Hints
<details><summary>Task 1 — sizeof</summary>

`sizeof(std::int32_t)` *is* a `std::size_t`, so `return sizeof(std::int32_t);`
is the whole body. (It's `4` — 32 bits ÷ 8 bits/byte — on every platform that
has the type, which is why the test can hard-code `== 4`.)
</details>
<details><summary>Task 2 — char ↔ int round-trip</summary>

```cpp
int code { static_cast<int>(ch) };
return static_cast<char>(code + delta);
```
Do the `+ delta` in `int`, then cast the sum back to `char`. Two explicit casts.
</details>
<details><summary>Task 3 — why a plain <code>start + addend > 255</code> won't work</summary>

`std::uint8_t` is narrower than `int`, so `start + addend` is **promoted to
`int`** and never wraps — the comparison would always be false for the cases that
matter. Force the result back into the 8-bit type and check if it shrank:
```cpp
std::uint8_t wrapped { static_cast<std::uint8_t>(start + addend) };
return wrapped < start;
```
</details>
<details><summary>Task 4 — epsilon compare</summary>

`return std::abs(a - b) <= epsilon;`. Use `<=` (not `<`) so a gap *exactly*
equal to `epsilon` still counts as "near" — one of the edge cases is built to
check that.
</details>
<details><summary>Task 5 — order of operations is the lesson</summary>

```cpp
if (index < 0) { return false; }
return static_cast<std::size_t>(index) < length;
```
The `index < 0` check must come **first**. Cast a negative `int` to `std::size_t`
*before* checking and it becomes a huge value that slides right past `length`.
</details>
<details><summary>Task 6 — ordered if/else-if</summary>

Test the cases in the README's order; the first match wins:
```cpp
if (nearlyEqual(value, 0.0, 1e-9)) return 'Z';
else if (value < 0.0)             return 'N';
else if (value >= 1000.0)         return 'B';
else                              return 'P';
```
Reuse your own `nearlyEqual` for the zero test instead of `value == 0.0`.
</details>

## Stretch goals (optional — some need later chapters)
- Add a `classifyNumber` band for "huge" values (`>= 1e6`) using scientific
  notation literals (`1e6`).
- Replace the `if (index < 0)` guard with a single combined condition once you've
  met `&&` (Chapter 6): `index >= 0 && static_cast<std::size_t>(index) < length`.
- Print `min`/`max` of each fixed-width type with `<limits>`
  (`std::numeric_limits<std::int32_t>::max()`), and confirm `wrapsAround` agrees
  at the boundary.
- Generalize `nearlyEqual` to a **relative** tolerance (scale `epsilon` by the
  magnitude of the inputs) so it works for very large values too.
