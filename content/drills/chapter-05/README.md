# Chapter 5 — Constants and Strings · Project: Name-Badge / Greeting Formatter

> Reinforces [`../../notes/chapter-05.md`](../../notes/chapter-05.md) · LearnCpp [Chapter 5](https://www.learncpp.com/)

## The project
You're writing the text engine behind a conference **name-badge printer**. Given a
person's first and last name, it assembles every string the badge needs: the org
header across the top, the full name `"Ada Lovelace"`, a friendly greeting
`"Hello, my name is Ada Lovelace"`, a login `"Ada.Lovelace"`, monogram initials
`"A.L."`, and the name's length for layout. It's a little library of six small,
const-correct functions — no `main`, no I/O — graded by unit tests.

The whole chapter is in the **types**. Every input a function only *reads* is a
`std::string_view` (a cheap read-only window — no copying), every function that
*builds* text returns an owning `std::string`, and the org name, separator, and
badge width are `constexpr` **named constants** instead of literals sprinkled
through the logic. And because Chapter 5 has no loops yet, you build each result
by **assembling** strings — concatenate with `+=`, peek the first letter with
`.front()`, measure with `.size()` — never by scanning character-by-character.

## Concepts practiced
- `constexpr` **named constants**, including a `constexpr std::string_view` string label (5.1, 5.2, 5.6, 5.8)
- Killing **magic numbers / literals** by naming them (the separator, the width) (5.2)
- `std::string` — owning, growable text you **build and return** (5.7)
- `std::string_view` — cheap, read-only **parameters** that accept literals, strings, or views (5.8, 5.9)
- Why a view does **not** implicitly become an owning string — explicit `std::string { view }` (5.8)
- `.front()`, `.size()`, `.empty()`, and string concatenation with `+=`
- **Reused from earlier chapters:** functions / headers / header guards (Ch 2), `if` and `static_cast` (Ch 4), value-initialization with `{}` (Ch 1)

## Your tasks
The starter compiles immediately — but every function returns a placeholder, so
`make test` fails. Fill in the six `>>> YOUR CODE HERE <<<` blocks in
[`starter/badge.cpp`](starter/badge.cpp). They map 1:1 to these tasks and ramp up:

1. **`badgeHeader()`** — return the org name. It's the `kOrgName` constant (a
   `std::string_view`); the function returns a `std::string`, so you copy the
   view into an owning string: `std::string { kOrgName }`.
2. **`fullName(first, last)`** — build `"First Last"` (one space between) by
   concatenating into a `std::string`.
3. **`greeting(first, last)`** — build `"Hello, my name is First Last"`. **Reuse
   `fullName`** — don't re-join the names by hand.
4. **`username(first, last)`** — build `"first.last"`, joining with the
   **`kUsernameSep` constant** (not a hard-coded `'.'`).
5. **`initials(first, last)`** — build `"F.L."` from each name's `.front()`.
   **Edge case:** `.front()` on an empty view is undefined behavior, so guard
   each name with `if (name.size() > 0)` and contribute nothing for an empty one.
6. **`nameLength(first, last)`** — return the length of the full name via
   `.size()`. It's unsigned, so `static_cast<int>(...)` it.

## Constraints
- **Implement the bodies only.** Don't touch [`badge.h`](badge.h) — it's the
  contract the grader relies on (function names, parameter and return types).
- Read-only parameters stay `std::string_view`; text you build is returned as a
  `std::string`. Don't change the signatures.
- Use the **named constants** from `badge.h` (`kOrgName`, `kUsernameSep`) instead
  of re-typing their literal values.
- **No loops** (Chapter 8) — every task is fixed string assembly. Allowed tools:
  `+` / `+=`, `.front()`, `.size()`, `if` with a relational test (`> 0`), `static_cast`.
- Don't add I/O — this is a pure library. The grader calls your functions directly.

## Build & run
```sh
make           # compile starter/badge.cpp  (it builds out of the box)
make test      # grade YOUR code against the unit tests
make solution  # run the grader against the reference solution
make run       # same as `make test` (a library has no program of its own)
make clean
```

## Success criteria
`make test` prints **`PASS ✅  all checks`** and exits 0. The grader in
[`tests/tests.cpp`](tests/tests.cpp) calls every function with normal inputs
**and edge cases** — empty names (`initials("", "") == ""`), single-character
names (`fullName("A","B") == "A B"`), and the just-a-space full name
(`nameLength("", "") == 1`). Until you fill in the tasks, the stubs return junk
and `make test` shows **`FAIL ❌`** listing each failing `CHECK` and its line.
Turning that red into green is the whole exercise.

## Hints
<details><summary>Task 1 — view vs. owning string</summary>

`kOrgName` is a `std::string_view`, but the return type is `std::string`. A view
does *not* silently become an owning string (that would hide an expensive copy),
so make the copy explicit: `return std::string { kOrgName };`.
</details>

<details><summary>Tasks 2 & 4 — concatenating into a std::string</summary>

Seed an owning string from the first view, then append onto it. A
`std::string_view` *and* a `char` both append with `+=`:
```cpp
std::string result { first };
result += ' ';        // a char        (Task 2: the space)
result += last;       // a string_view
return result;
```
For Task 4 the join character is the named constant: `result += kUsernameSep;`.
</details>

<details><summary>Task 3 — compose, don't repeat</summary>

Build the fixed prefix, then append the result of `fullName`:
```cpp
std::string result { "Hello, my name is " };
result += fullName(first, last);   // fullName returns a std::string
return result;
```
</details>

<details><summary>Task 5 — the empty-name guard</summary>

`.front()` reads the first character but is **undefined behavior on an empty
view**. Guard each name independently so an empty one simply contributes nothing:
```cpp
std::string result {};
if (first.size() > 0) { result += first.front(); result += kUsernameSep; }
if (last.size() > 0)  { result += last.front();  result += kUsernameSep; }
return result;
```
(`.empty()` would read more nicely, but `> 0` keeps us to the relational
operators already seen in Chapter 4.)
</details>

<details><summary>Task 6 — size() is unsigned</summary>

`.size()` returns `std::size_t` (unsigned). Returning it directly as `int` warns
under `-Wall -Wextra`, so convert deliberately:
`return static_cast<int>(fullName(first, last).size());`
</details>

## Stretch goals (optional — some need later chapters)
- **Make the username lowercase** (`"Ada.Lovelace"` → `"ada.lovelace"`). That
  needs to walk each character, i.e. a **loop** (Chapter 8) plus `std::tolower`.
- Add `padBadge(text)` that centers `text` inside `kBadgeWidth` columns — the
  `constexpr int kBadgeWidth` is already declared in `badge.h` waiting for you.
- Trim surrounding spaces from a name with `string_view`'s `.remove_prefix()` /
  `.remove_suffix()` before building (notes 5.9) — needs a loop to find the spaces.
- Validate input and reject empty *both* names with an error path (Chapter 9's
  `assert` / error handling).
