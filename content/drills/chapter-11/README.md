# Chapter 11 — Function Overloading and Function Templates: The Generic Toolkit

> Reinforces [`../../notes/chapter-11.md`](../../notes/chapter-11.md) · LearnCpp [Chapter 11](https://www.learncpp.com/)

## The project

You will build a small **generic toolkit mini-library** — a set of functions
that could plausibly live in any real C++ utility header. The key design goal is
making **overload resolution** and **template instantiation** *observable*: rather
than just writing code that compiles, you write code whose runtime output proves
*which* overload or instantiation the compiler selected.

The library has four overloaded `describe()` functions (one for `int`, one for
`double`, one for `bool`, and one for `std::string_view`) that each return a
short labeled string like `"int:42"` or `"bool:true"`. A test that expects
`describe(true) == "bool:true"` will fail with `"int:1"` if the `bool` overload
is missing — because `bool` promotes to `int` (notes 11.3), and the test tells
you exactly which wrong overload fired. You also implement four function
templates (`myMin<T>`, `myClamp<T>`, `repeatChar<N>`, `scaled<T,U>`) and one
function that demonstrates a default argument (`formatCount`).

Why this design? In CS6340 LLVM passes you will read APIs like:

```cpp
void instrument(Function& function);      // overloaded for Function
void instrument(BasicBlock& block);       // overloaded for BasicBlock
void instrument(Instruction& instr);      // overloaded for Instruction
```

and generic utilities like `SmallVector<T>`, `ArrayRef<T>`, and
`PointerIntPair<T,N>`. The patterns you practice here — overloads, templates,
non-type template params — are the same patterns those APIs use.

## Concepts practiced

- **Overloaded functions** and what differentiates them (notes 11.1, 11.2)
- **Overload resolution** — exact match beats promotion beats conversion (notes 11.3)
- **bool overload** preventing silent `bool` → `int` promotion (notes 11.3)
- **`= delete` for forbidden conversions** — shown in comments (notes 11.4)
- **Default arguments** — right-aligned, in declarations (notes 11.5)
- **Function templates** (`template <typename T>`) as type-parameterized code
  patterns (notes 11.6, 11.7)
- **Template argument deduction** — the compiler infers `T` from arguments (11.7)
- **Explicit template arguments** — `myMin<double>(3, 4.5)` forces `T=double` (11.7)
- **Multiple template type parameters** — `template <typename T, typename U>` for
  mixed-type operations (notes 11.8)
- **`auto` return type** — deduced from the return expression (notes 11.8)
- **Non-type template parameters** — `template <int N>` for compile-time constants
  (notes 11.9)
- **Per-instantiation static locals** — each `repeatChar<N>` instantiation gets
  its own static buffer (notes 11.7)
- **Templates in headers** — why the full definition must be visible to callers
  (notes 11.10)
- Reused from earlier chapters: **`std::string_view`** (Ch 5), **`switch`** /
  `if/else` / `for` (Ch 4, 8), **`const`** correctness (Ch 5), **`inline`** (Ch 7),
  **`static_cast`** (Ch 10), **header guards** (Ch 2)

## Your tasks

The starter compiles immediately but every function returns a **placeholder**,
so `make test` starts **RED**. Fill in the `>>> YOUR CODE HERE <<<` blocks in
[`starter/toolkit.h`](starter/toolkit.h). They ramp from warm-up to capstone:

1. **`describe(int)` — the int overload.** Return `"int:<value>"` for each
   expected input (0, 1, -1, 42, -99, 100) using a `switch` statement.
   Return `"int:?"` for any other value.

2. **`describe(double)` — the double overload.** Return `"double:<value>"` for
   each expected input using an `if` / `else if` chain (doubles cannot be used
   as `switch` case constants). Return `"double:?"` for unknown values.

3. **`describe(bool)` — the bool overload.** Return `"bool:true"` or
   `"bool:false"`. This is the **key overload-resolution task**: without it,
   `describe(true)` calls `describe(int)` because `bool` promotes to `int`
   (notes 11.3). The test checks for `"bool:true"`, not `"int:1"` — that
   difference is the lesson made visible. Use a ternary `?:` or `if/else`.

4. **`describe(std::string_view)` — the string overload.** Return `"str:<value>"`
   for `"hello"`, `"LLVM"`, and `""` (empty). Use an `if` / `else if` chain.
   Note: the grader passes `"hello"sv` (the `sv` suffix makes the argument
   already a `string_view` — an exact match at step 1) to show why this matters.

5. **`myMin<T>` — a function template.** Write a single template that returns
   the smaller of two same-type values. Use `operator<` and a ternary. Test
   it with both `int` and `double`. The grader also calls `myMin<double>(3, 4.5)`
   to exercise **explicit template arguments** — the int `3` is converted to `3.0`
   because you forced `T = double` (notes 11.7).

6. **`myClamp<T>` — another template with boundary logic.** Clamp `value` into
   `[lo, hi]`: if `value < lo` return `lo`, if `value > hi` return `hi`,
   otherwise return `value` unchanged. An `if` / `else if` / `else` chain is
   the right shape. Works for int, double, and any T with `<`.

7. **`repeatChar<N>` — non-type template parameter.** Return a `string_view` of
   `N` identical copies of `ch`. `N` is an `int` non-type template parameter —
   a compile-time constant that sizes a `static char buf[N + 1]` array. Each
   distinct `N` (e.g. `repeatChar<3>` vs `repeatChar<5>`) is a **separate
   instantiation** with its own static buffer (notes 11.9, 11.7).

8. **`scaled<T,U>` — two-type template with auto return.** Multiply `a` (type T)
   by `b` (type U) and return the result. Use **two type parameters** so `int`
   and `double` arguments can be mixed without explicit casts. Use `auto` as the
   return type so the compiler deduces it from `a * b` — for `int * double` the
   result is `double`, preserving the fractional part (notes 11.8).

9. **`formatCount` — default argument.** Return a count string: singular
   (`"1 item"`) when `count == 1`, plural (`"3 items"`) otherwise. The `suffix`
   parameter has a default value of `"s"`. The default is already declared in the
   header — **do not repeat it** in the function body (notes 11.5). Use two static
   char buffers (64 bytes each) to keep the returned `string_view` valid.

## Constraints

**Allowed** (Chapter ≤ 11):
- `template <typename T>` and `template <int N>` function templates
- Overloaded functions differentiated by parameter type
- `= delete` (shown in comments; tests cannot require a compile error)
- Default arguments (right-aligned, in declarations)
- `switch`, `if/else`, `for`, `while`, `?:` — all control flow from Ch 4–8
- `std::string_view`, `const`, `auto`, `static_cast`, `inline`, `static`
- `<string_view>`, `<cmath>` (for `std::abs` in tests only)

**Forbidden** (not yet taught):
- Class templates (Ch 26) — use only **function** templates
- Abbreviated auto parameters `auto add(auto a, auto b)` (C++20 — notes 11.8
  mentions this but CLAUDE.md scope forbids using it)
- `std::string`, `std::to_string`, `std::snprintf` — stay on-scope; manual
  char copying is intentional here
- `std::vector`, `std::array`, pointers (Ch 12+), `struct`/`class` (Ch 13+)
- `goto`, unnamed namespaces inside function bodies, global mutable state

**Required idioms:**
- Templates **defined in the header** (not a `.cpp` file) — notes 11.10
- Default argument in the **declaration**, not the definition — notes 11.5
- Each template function uses only the concepts it documents

## Build & run

```sh
make            # compile-check starter/toolkit.h AND build the test binary
make run        # same as make test (no separate driver for this lab)
make test       # grade your toolkit  ->  RED until TASK blocks are filled in
make solution   # build + run the reference solution
make clean      # remove build artifacts
```

`make test` links `tests/tests.cpp` against **your** `starter/toolkit.h` (via
`-Istarter`). `make test-solution` uses `solution/toolkit.h` instead — same
grader, different include path.

## Success criteria

`make test` prints **PASS ✅ all toolkit checks passed.** and exits 0. Until then
it prints one `FAIL: …  @line N` per broken check. The grader exercises each
function across many inputs — including the edge cases that separate a correct
implementation from a plausible-looking near-miss:

- `describe(true) == "bool:true"` and `describe(true) != "int:1"` — the **bool
  overload** must be the exact match, not the int promotion path (notes 11.3)
- `myMin(5, 3) == 3` — the template must compare, not just return `a`
- `myMin<double>(3, 4.5) ~= 2.7` — **explicit template argument** must force
  `T=double` so `int` is converted before comparison (notes 11.7)
- `myClamp(0, 1, 10) == 1` — below lo must clamp to lo, not pass through
- `myClamp(0, 3, 3) == 3` — lo == hi edge case must clamp to that single value
- `repeatChar<3>('*') == "***"` — three chars, correct content, size == 3
- `scaled(3, 2.5) ~= 7.5` — `auto` return must be `double`, not truncated `int`
- `formatCount("item", 1) == "1 item"` — singular, no suffix
- `formatCount("item", 3) == "3 items"` — plural with default suffix `"s"`
- `formatCount("pass", 1, "es") == "1 pass"` — singular ignores suffix

## Hints

<details><summary>Task 1 — switch on int values</summary>

```cpp
inline std::string_view describe(int value)
{
    switch (value)
    {
    case  0:   return "int:0";
    case  1:   return "int:1";
    case -1:   return "int:-1";
    case  42:  return "int:42";
    case -99:  return "int:-99";
    case  100: return "int:100";
    default:   return "int:?";
    }
}
```
Each `return` exits the `switch` and the function — no `break` needed when you
use `return` in each arm. `default` catches anything the grader doesn't expect.
</details>

<details><summary>Task 3 — the bool overload and why it matters</summary>

The ternary form is cleanest:

```cpp
inline std::string_view describe(bool value)
{
    return value ? "bool:true" : "bool:false";
}
```

To *see* what happens without this overload: comment it out and run `make test`.
You'll see `describe(true)` returns `"int:1"` — the bool promoted to int (step 2
of resolution) and the int overload won. With the bool overload the compiler stops
at step 1 (exact match) and never tries a promotion.
</details>

<details><summary>Task 4 — why use the sv suffix in tests</summary>

A plain `"hello"` has type `const char*`. Converting `const char*` to `bool` is
a **standard conversion** (non-null pointer → true, step 3). Converting
`const char*` to `std::string_view` is a **user-defined conversion** (step 4).
Standard conversions beat user-defined conversions in overload resolution, so
`describe("hello")` would call `describe(bool)` and return `"bool:true"` — wrong!

The `"hello"sv` suffix creates a `std::string_view` literal directly, which is
an **exact match** (step 1) for `describe(std::string_view)`. This is the
resolution rule made physical: always check what TYPE an expression actually has.
</details>

<details><summary>Task 5 — myMin template and explicit T</summary>

```cpp
template <typename T>
T myMin(T a, T b)
{
    return (a < b) ? a : b;
}
```

For `myMin<double>(3, 4.5)`: the explicit `<double>` tells the compiler `T=double`
before it looks at the arguments. So `int 3` is converted to `double 3.0` as part
of passing it to the `double` parameter. Without `<double>`, the compiler would
see one argument suggesting `T=int` and another suggesting `T=double` and report
an error — it cannot deduce a single `T` from two different types (notes 11.7).
</details>

<details><summary>Task 6 — myClamp with operator< only</summary>

```cpp
template <typename T>
T myClamp(T value, T lo, T hi)
{
    if (value < lo)
        return lo;
    else if (hi < value)
        return hi;
    else
        return value;
}
```

Note: `hi < value` rather than `value > hi` — both work for built-in types, but
writing it consistently as `<` means the template works for any type that only
defines `operator<` (a common convention in the standard library).
</details>

<details><summary>Task 7 — non-type parameter and static per-instantiation buffer</summary>

```cpp
template <int N>
std::string_view repeatChar(char ch)
{
    static char buf[N + 1];   // sized at compile time; ONE per <N> instantiation
    for (int i = 0; i < N; ++i)
        buf[i] = ch;
    buf[N] = '\0';
    return std::string_view{buf, N};
}
```

`N` is baked into the type at the call site: `repeatChar<3>('*')` and
`repeatChar<5>('-')` are *different functions* with *different static buffers*.
If you called `repeatChar<3>` twice with different chars the second call
overwrites the buffer — but the test only checks the return value immediately,
so that is fine here.
</details>

<details><summary>Task 8 — two-type template and auto return</summary>

```cpp
template <typename T, typename U>
auto scaled(T a, U b)
{
    return a * b;
}
```

`auto` deduces the return type from the expression `a * b`. For `int * double`
the standard arithmetic conversion rules promote `int` to `double` and the result
is `double`. If you wrote `T` instead of `auto` and called `scaled<int, double>`,
the `double` result would be narrowed to `int` — losing the fractional part.
</details>

<details><summary>Task 9 — formatCount with static buffers</summary>

The singular branch is straightforward — copy `"1 "` + label chars into a static
array. For the plural branch:

```cpp
static char plural[64];
int i = 0;
// Write count (works for 0–99)
if (count >= 10) plural[i++] = static_cast<char>('0' + (count / 10) % 10);
plural[i++] = static_cast<char>('0' + count % 10);
plural[i++] = ' ';
for (std::size_t k = 0; k < label.size() && i < 60; ++k)
    plural[i++] = label[k];
for (std::size_t k = 0; k < suffix.size() && i < 63; ++k)
    plural[i++] = suffix[k];
plural[i] = '\0';
return std::string_view{plural, static_cast<std::size_t>(i)};
```

The static buffer lives for the whole program, so returning a `string_view` into
it is safe. (This is a preview of lifetime thinking — formally Chapter 12.)
</details>

## Stretch goals (optional — most need later chapters)

- Add a `describe(char)` **deleted overload** (notes 11.4) and observe that
  `describe('A')` becomes a compile error instead of silently calling
  `describe(int)`. (Just uncomment the line in `starter/toolkit.h`.)
- Extend `myMin` to a `myMax`, then build `myClamp` by calling `myMax(lo, myMin(value, hi))`
  — shows how templates compose just like overloads do.
- Add a third type parameter to `scaled` and generalize to three-value multiply
  (compare with the three-template-type overload in notes 11.8).
- Replace the manual integer-to-char conversion in `formatCount` with
  `std::to_string` (Ch 5) or `std::snprintf` (Ch 28) once those chapters land.
- Add an `isInsideClosedRange<T>(value, lo, hi)` template (from notes 11.7) and
  test it with int, double, and char — showing that `<` has the expected meaning
  for all three.
