# Chapter 2 — Functions and Files · Project: the `geo` mini-library

> Reinforces [`../../notes/chapter-02.md`](../../notes/chapter-02.md) · LearnCpp [Chapter 2](https://www.learncpp.com/)

## The project
You're completing a tiny geometry library, `geo`, that other programs can link
against to compute areas, perimeters, hypotenuses, and diagonals of basic
shapes. The library's **public interface** already exists — a header,
[`geo.h`](geo.h), that *declares* five functions inside `namespace geo` behind a
header guard. Your job is to write the **bodies** in
[`starter/geo.cpp`](starter/geo.cpp). A tiny program ([`demo.cpp`](demo.cpp)) and
the grader ([`tests/tests.cpp`](tests/tests.cpp)) both `#include "geo.h"` and call
your functions through `geo::` — they never see how the functions work, only what
the header promises.

That split is the entire point of the chapter. Chapter 1 lived inside one
`main()`; here you practice the real shape of a C++ program: **declarations in a
header**, **definitions in a `.cpp`**, **everything namespaced**, compiled as
**multiple files** and joined by the **linker**. This is also the exact file
layout of every LLVM pass you'll write in CS6340 — a `Foo.h` that declares an API
inside a namespace, and a `Foo.cpp` that defines it. You're building that muscle
on something you can check by hand.

## Concepts practiced
- **User-defined functions**: definitions, **parameters**, and **return values** (2.1, 2.2, 2.4)
- The **header / source split** — declarations in `geo.h`, definitions in `geo.cpp` (2.8, 2.11)
- **Forward declarations** (prototypes): the header *is* a set of them (2.7)
- **Header guards** (`#ifndef / #define / #endif`) (2.10, 2.12)
- A **user-defined namespace** (`namespace geo`) and the scope-resolution operator `::` (2.9)
- **Functions calling functions** — `rectangleDiagonal` reuses `hypotenuse` (2.1, 2.6, DRY)
- Reused from Chapter 1: variable **initialization** with `{}`, **expressions**, `std::cout`, `int`
- A first taste of `double` and `std::sqrt` (kept to whole-number answers on purpose)

## Your tasks
The starter compiles and runs immediately, but every function returns a wrong
placeholder, so `make test` is **RED**. Fill in the five `>>> YOUR CODE HERE <<<`
blocks in [`starter/geo.cpp`](starter/geo.cpp) — each maps 1:1 to a numbered task
here, and they ramp from easy to a small stretch:

1. **`rectangleArea(width, height)`** — return `width * height`. Both are `int`;
   the product is `int`. (Warm-up.)
2. **`rectanglePerimeter(width, height)`** — return the distance around all four
   sides: `2 * width + 2 * height`.
3. **`rightTriangleArea(a, b)`** — return half the bounding rectangle: `(a * b) / 2`,
   as a **`double`**. Mind the types: for an odd product like `3 * 5 = 15` the
   answer is **7.5**, not 7. Make the division happen in floating point (e.g.
   divide by `2.0`) so the half isn't chopped off by integer division.
4. **`hypotenuse(a, b)`** — return `std::sqrt(a*a + b*b)` (Pythagoras). `<cmath>`
   is already included for you.
5. **`rectangleDiagonal(width, height)`** — the diagonal equals the hypotenuse of
   the right triangle whose legs are the rectangle's sides. **Compose**: *call*
   your own `hypotenuse(width, height)` and return its result — don't re-derive
   the square root. (Inside `namespace geo`, call it unqualified.)

You do **not** edit `geo.h`, `demo.cpp`, or the tests — only `starter/geo.cpp`.

## Constraints
- **Allowed:** `int` and `double`; the four arithmetic operators; `std::sqrt`
  from `<cmath>`; calling your own `geo` functions.
- **Required idioms:** keep every body inside `namespace geo { ... }` so its full
  name matches the prototype in `geo.h`; leave `#include "geo.h"` at the top of
  `geo.cpp` (so the compiler checks your bodies against the contract).
- **Forbidden (not taught yet):** `if`/branches (Ch 4/8), loops (Ch 8), classes,
  `std::string` scanning. You don't need any of them.
- **Don't** put a function *definition* in `geo.h`, and **don't** `#include` any
  `.cpp` file — the Makefile adds the right `.cpp` to the build instead (2.8, 2.11).
- Task 5 must **reuse** `hypotenuse` rather than calling `std::sqrt` a second time.

## Build & run
```sh
make           # compile demo.cpp + starter/geo.cpp  ->  starter/app
make run       # run the demo (numbers are wrong until you finish the tasks)
make test      # grade your geo.cpp against the unit tests   <-- the real goal
make solution  # build + run the reference library's demo
make clean
```
The grader compiles `tests/tests.cpp` **together with** your `starter/geo.cpp`
into one program and runs it — that two-file compile, joined by the linker, is
the chapter's whole idea.

## Success criteria
`make test` prints **PASS ✅  all geo checks passed.** It calls every function
through the `geo::` namespace and checks results, including edge cases:
- a **zero-width** rectangle (area `0`; perimeter is just the two heights),
- the **odd-product** right triangle `3, 5` → **7.5** (catches the integer-division
  trap in Task 3),
- a **cross-check** that `rectangleDiagonal(w, h) == hypotenuse(w, h)` — proving
  Task 5 really delegates instead of re-deriving.

Until then `make test` prints **FAIL ❌** and lists each failing check with its
line number. Turning that red into green — by writing five correct function
bodies in the right namespace — is the exercise.

## Hints
<details><summary>Task 1 & 2 — the rectangle functions</summary>

These are one-liners. `return width * height;` and `return 2 * width + 2 * height;`.
The placeholders currently `return width + height;` (addition) on purpose — that's
why area and perimeter come out wrong.
</details>

<details><summary>Task 3 — why <code>(a * b) / 2</code> gives the wrong answer</summary>

`a` and `b` are `int`, so `a * b` is an `int`, and `int / int` does **integer
division** — it throws away the remainder *before* the value ever becomes a
`double`. `15 / 2` is `7`, so `rightTriangleArea(3, 5)` would return `7.0`. Divide
by `2.0` (a `double` literal) instead: that forces the division into floating
point and keeps the `.5`. Return type is `double`, so `(a * b) / 2.0` is what you
want.
</details>

<details><summary>Task 4 — calling <code>std::sqrt</code></summary>

`#include <cmath>` is already at the top of the file. Then
`return std::sqrt(a * a + b * b);`. The argument `a*a + b*b` is an `int`
expression; `std::sqrt` takes a `double`, so the `int` is converted automatically
and you get a `double` back.
</details>

<details><summary>Task 5 — composition (functions calling functions)</summary>

You already wrote `hypotenuse`. The rectangle's diagonal is exactly the
hypotenuse of the triangle formed by its two sides, so just hand them over:
```cpp
double rectangleDiagonal(int width, int height)
{
    return hypotenuse(width, height); // no geo:: prefix needed inside namespace geo
}
```
This is "Don't Repeat Yourself" (2.2): one definition of the Pythagoras math,
reused. If you instead pasted the `std::sqrt` formula again, the cross-check test
would still pass, but you'd be maintaining the same math in two places — exactly
what functions exist to prevent.
</details>

<details><summary>Stuck on a build error instead of a test failure?</summary>

- *"`area` was not declared in this scope"* — make sure your body is **inside**
  `namespace geo { ... }`. Outside it, you'd be defining a *different* global
  `area`, and `geo::area` would still be undefined (a **linker** error).
- *"`geo.h` file not found"* — build through the Makefile; it passes `-I.` so the
  simple-name include resolves.
- A **return-type or signature mismatch** is caught at **compile** time precisely
  because `geo.cpp` includes its own header — fix the body to match the prototype
  in `geo.h` (that's the safety the `.cpp`-includes-its-`.h` rule buys you).
</details>

## Stretch goals (optional — some need later chapters)
- Add `circleArea(int radius)` and `circleCircumference(int radius)` using a
  `geo::pi` constant — practice adding to a namespace and to the header (you can
  do this now with `const`/`constexpr` arriving fully in Ch 5).
- Add `squareArea(int side)` implemented by **calling** `rectangleArea(side, side)`
  — more composition.
- Return both a shape's area *and* perimeter from one call (needs a `struct`/class,
  Ch 13) — for now, notice *why* a function returns exactly one value (2.2).
- Reject negative sides with a message (needs `if`, Ch 4/8) — then validate inputs
  properly (Ch 9).
- Split `geo` into two headers (`rect.h`, `triangle.h`) and watch how header
  guards and include order (2.11, 2.12) keep everything from colliding.
