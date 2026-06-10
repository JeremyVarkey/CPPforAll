# Chapter 27 — Exceptions · Project: SafeConfig Parser

> Reinforces [`../../notes/chapter-27.md`](../../notes/chapter-27.md) · LearnCpp [Chapter 27](https://www.learncpp.com/)

## The project

You are building a **SafeConfig parser** — a thin wrapper around a string→string
map that reads configuration entries and validates them with exceptions. The
parser exposes two typed exception classes, a port-parsing function that throws
the *right* type for each failure mode, a `noexcept` boundary wrapper that
contains exceptions completely, and an `Unwinder` probe that makes **stack
unwinding physically observable**.

Why config parsing? Because it hits every exception scenario at once: a missing
key is a *structurally different failure* from a bad-format value, which is
different from an out-of-range integer. Those differences belong in exception
*types*, not in error-code integers — and `std::exception`'s `what()` carries
the human-readable context that an error code can never carry. The project also
demonstrates the two main exception policies — propagate freely through the call
graph, or absorb at a boundary and return a plain bool — so you can recognize
both in real code.

The `Unwinder` probe (Task 4) is the chapter's central "make it physical"
moment: you will *see* a counter increment during stack unwinding even though
the function that owned the object never reached a `return` statement.

## Concepts practiced

**New in Chapter 27:**
- **`throw`** with a typed exception object — throwing the *right* type (notes 27.2)
- **`try` / `catch` by const reference** — avoids slicing, avoids copies (notes 27.2)
- **Stack unwinding** — destructors run for all locals, even when an exception
  propagates past them (notes 27.3)
- **`std::exception` hierarchy** — `what()`, deriving from `std::runtime_error`,
  catching derived before base (notes 27.5)
- **Custom exception classes** — `ConfigError : std::runtime_error`,
  `MissingKeyError : ConfigError` (notes 27.5)
- **Catch ordering** — most-derived handler first, base handler later (notes 27.5)
- **Rethrow with a new exception** — `throw NewType{...}` to layer context (notes 27.6)
- **`noexcept` function declaration** — a real, enforced contract; `catch (...)` inside
  makes the promise true (notes 27.9)

**Reused from earlier chapters:**
- Functions, headers, header guard (Ch 2)
- `std::string`, `std::string_view` (Ch 5)
- `const`-correctness (Ch 5)
- Member functions, access specifiers, constructors (Ch 14)
- `= delete` to prevent copying (Ch 14)
- `std::optional` (available from Ch 12; not used here but noted in the header)
- `std::unordered_map` (a preview — formally containers Chapter 17+; provided
  as scaffolding so you don't need to implement it)
- `std::move` in constructor for efficient map transfer (Ch 22; provided scaffolding)

## Your tasks

The starter compiles immediately but every function body returns a **placeholder**,
so `make test` starts **RED**. Fill in the five `>>> YOUR CODE HERE <<<` blocks in
[`starter/config_parser.cpp`](starter/config_parser.cpp). They ramp from warm-up
to capstone:

1. **`getOrThrow` — throw the specific derived type** (Task 1).
   Look up `key` in `m_entries`. If absent, `throw MissingKeyError { key }`.
   If present, return the value. One `find()` + `if (== end())` + `throw` is enough.

2. **`parsePort` — three distinct failure modes, three distinct types** (Task 2).
   Call `getOrThrow(key)` first (let `MissingKeyError` propagate naturally — no
   catch here). Next, **reject an empty value** with `throw ConfigError{...}` —
   a per-character loop passes *vacuously* for `""`, and `std::stoi("")` would
   leak `std::invalid_argument`, a type that's not in the contract. Then loop
   over the value's characters: if any is not a digit, `throw ConfigError{...}`.
   Convert with `std::stoi`, then range-check `[1, 65535]`: if out of range,
   `throw std::out_of_range{...}`. On success, return the int.

3. **`tryParsePort` — the `noexcept` boundary wrapper** (Task 3).
   Wrap a call to `cfg.parsePort(key)` in a `try` block. On success, store the
   result in `outPort` and return `true`. In a `catch (...)` block, return `false`
   and leave `outPort` unchanged. The `noexcept` in the signature is already there;
   your body makes the promise real.

4. **`Unwinder` constructor, destructor, and `throwingWork`** (Task 4).
   - Constructor: increment `*entered` to record that the probe was created.
   - Destructor: increment `*destroyed`. This must not throw (notes 27.8).
   - `throwingWork`: create a local `Unwinder uw{pEntered, pDestroyed}`, then
     `throw ConfigError{"work failed"}`. The destructor runs *during* unwinding,
     before the `catch` block in the test — that's the observable behaviour.

5. **`rethrowOuter` — catch, augment, re-raise** (Task 5).
   Call `throwingWork` inside a `try` block. Catch `const ConfigError& e`.
   Build a new message: `"outer: " + e.what()`. `throw ConfigError{newMessage}` —
   this raises a *new* exception object with the layered context. Do not use bare
   `throw;` here (that would re-raise the original; we want a different object).

You may **not** edit `config_parser.h` or anything in `tests/` — those are the
contract and the grader.

## Constraints

**Allowed:** everything from Chapters 1–27. Key constructs:
`throw`, `try`, `catch (const T&)`, `catch (...)`, bare `throw;` to rethrow,
`std::runtime_error`, `std::out_of_range`, `std::exception`,
custom exception classes deriving from the standard hierarchy,
`noexcept` on a function that genuinely cannot propagate.

**Required idioms (per notes 27.2–27.9):**
- Catch class-type exceptions **by `const` reference** (avoids slicing and copy).
- Place **most-derived** catch handlers before base class handlers.
- Never throw from a destructor (notes 27.8 — the runtime calls `std::terminate`
  if a destructor throws while another exception is being handled).
- The `noexcept` specifier must be a **real promise** — back it with `catch (...)`.

**Forbidden (out of scope):**
- Function try blocks (mentioned in notes 27.7 but not a learner task here).
- `noexcept` on a function that can actually propagate — the lie causes `std::terminate`.
- Deprecated dynamic exception specifications (`throw(...)` — pre-C++11 syntax).
- `std::nested_exception` / `std::throw_with_nested` — beyond Ch 27 scope.

## Build & run

```sh
make           # compile-check starter/config_parser.cpp (warning-clean)
make test      # grade your code  →  RED until TASK blocks are filled in
make solution  # build + run the reference solution if you get stuck
make test-solution  # verify the reference is green (our proof it's solvable)
make clean     # remove build artifacts
```

`make test` is the grader — it supplies its own `main()` in `tests/tests.cpp`.

## Success criteria

`make test` prints **PASS ✅ all SafeConfig checks passed.** and exits 0.
Until then it prints one `FAIL: …  @line N` per broken check and ends with
`FAIL ❌  N check(s) failed`.

The grader covers every failure mode and ordering rule:
- `getOrThrow("timeout")` throws `MissingKeyError` whose `what()` names `"timeout"`
- Catching `MissingKeyError` fires before `ConfigError` for the same exception
- `parsePort` with `"abc"` throws `ConfigError`, *not* `MissingKeyError`
- `parsePort` with an **empty value** throws `ConfigError` — not a leaked
  `std::invalid_argument` from `std::stoi("")` (the vacuous-loop trap)
- `parsePort` with `"0"` or `"65536"` throws `std::out_of_range`, *not* `ConfigError`
- `tryParsePort` returns `false` for every failure mode and never propagates
- `throwingWork` increments both counters; destroyed increments *before* the catch runs
- `rethrowOuter` throws a `ConfigError` whose `what()` starts with `"outer: "`

Turning that wall of red into a single green line is the whole exercise.

## Hints

<details><summary>Task 1 — finding a key in an unordered_map</summary>

`std::unordered_map::find` returns an iterator. If the key is absent, it equals
`m_entries.end()`. A `std::string_view` cannot directly key the map (C++17
doesn't have heterogeneous lookup for `unordered_map` by default), so convert
first: `auto it { m_entries.find(std::string{key}) };`.

```cpp
auto it { m_entries.find(std::string{key}) };
if (it == m_entries.end())
    throw MissingKeyError { std::string{key} };
return it->second;
```
</details>

<details><summary>Task 2 — validating digits and converting to int</summary>

Guard the empty case first, then walk the value string checking each character:
```cpp
if (value.empty())
    throw ConfigError { "port value is empty" };
for (char ch : value)
    if (!std::isdigit(static_cast<unsigned char>(ch)))
        throw ConfigError { "port value is not numeric: '" + value + "'" };
int port { std::stoi(value) };
if (port < 1 || port > 65535)
    throw std::out_of_range { "port " + std::to_string(port) + " is outside [1, 65535]" };
return port;
```
Why the empty guard must come first: `""` sails through the loop untouched
(zero characters → zero rejections), and `std::stoi("")` throws
`std::invalid_argument` — which is **not** a `ConfigError` (it lives on the
`std::logic_error` branch of the hierarchy), so a caller following this
header's contract would never catch it. The `static_cast<unsigned char>`
before `std::isdigit` is correct practice to avoid UB on char values above
127 (char can be signed on some platforms).
</details>

<details><summary>Task 3 — the noexcept boundary</summary>

```cpp
try {
    outPort = cfg.parsePort(key);
    return true;
}
catch (...) {
    return false;
}
```
The `catch (...)` "catch-all" (notes 27.4) absorbs every possible exception so
none can escape to the `noexcept` boundary. If the boundary were violated,
`std::terminate()` would be called — a hard crash with no recovery.
</details>

<details><summary>Task 4 — making unwinding physical</summary>

```cpp
// Constructor
Unwinder::Unwinder(int* pEntered, int* pDestroyed)
    : entered{pEntered}, destroyed{pDestroyed}
{ ++(*entered); }

// Destructor — no throw allowed
Unwinder::~Unwinder() { ++(*destroyed); }

// throwingWork
void throwingWork(int* pEntered, int* pDestroyed) {
    Unwinder uw{pEntered, pDestroyed};   // enters: *pEntered becomes 1
    throw ConfigError{"work failed"};    // uw's dtor runs HERE, during unwind
}
```
The destructor runs *before* the caller's catch block — that is the observable
proof of stack unwinding.
</details>

<details><summary>Task 5 — layering context on an exception</summary>

```cpp
try {
    throwingWork(pEntered, pDestroyed);
}
catch (const ConfigError& e) {
    throw ConfigError { std::string{"outer: "} + e.what() };
}
```
This is `throw NewObject{...}`, not bare `throw;`. Bare `throw;` would re-raise
the *original* `ConfigError{"work failed"}` unchanged. We want a *new* object
with the composed message, so we construct and throw explicitly. The original
Unwinder destructor already ran before this catch block, so `*destroyed` is
already correct when we re-raise.
</details>

## Stretch goals (optional — some need later chapters)

- Add a `parsePositiveInt` function that throws `ConfigError` for non-numeric
  values and `std::out_of_range` for values ≤ 0. Reuse your digit-check loop.
- In `rethrowOuter`, switch from "throw new" to bare `throw;` and observe that
  the `what()` message no longer starts with `"outer: "` — that's the slicing
  vs. rethrow difference made physical.
- Add a `getOptional` method that returns `std::optional<std::string>` instead
  of throwing — then compare: which callers are cleaner with `optional`, which
  with exceptions? (Motivates notes 27.1's "not every error should be an
  exception" discussion.)
- Wrap `main` of a small driver in `try { ... } catch (const std::exception& e)
  { std::cerr << "fatal: " << e.what() << '\n'; return 1; }` — the CS6340
  top-level tool pattern from the notes' CS6340 patterns section.
