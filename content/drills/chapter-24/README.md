# Chapter 24 — Inheritance: Report-Logger Family

> Reinforces [`../../notes/chapter-24.md`](../../notes/chapter-24.md) · LearnCpp [Chapter 24](https://www.learncpp.com/)

## The project

You'll build a small **report-logger family** — a base class `Logger` and two
derived classes — using *non-virtual* public inheritance. The family models a
real pattern: a base class provides core behavior (formatting, counting) and
derived classes specialize it. Every key Chapter 24 concept is observable in
the output, not just readable in the code.

The exercise ends with a **cliffhanger**: a free function `processLog` accepts a
`Logger&` and calls `log()`. When you pass a `TimestampLogger` through that
reference, the *base* `Logger::log()` runs — even though the object *is* a
`TimestampLogger`. The tests assert this "expected-but-disappointing" behavior
on purpose, and the last line of this README explains why it exists. This is the
problem Chapter 25 — virtual functions — was built to solve.

The class hierarchy mirrors LLVM's own `Instruction` family: a base class owns
common behavior; derived classes specialize it; helper functions accept base
references and (pre-Chapter 25) only reach the base behavior. You will read code
exactly like this in every CS6340 assignment. (notes 24.1)

## Concepts practiced

**New in Chapter 24:**
- **Public inheritance** (`: public Base`) — the "is-a" relationship (notes 24.2)
- **Base / derived construction order** — base constructs first; destruction
  reverses (notes 24.3); made *observable* through trace strings
- **Derived constructors** — calling the immediate base constructor in the
  member-initializer list; cannot directly initialize private base members
  (notes 24.4)
- **`protected` access** — `m_lineCount` is protected so `CountingLogger` can
  read it directly; public users cannot (notes 24.5)
- **Adding new functionality** — `CountingLogger` adds `linesLogged()` without
  redefining `log()` (notes 24.6)
- **Function redefinition (hiding)** — `TimestampLogger` redefines `log()`,
  hiding the base version for calls on a `TimestampLogger` object (notes 24.7)
- **Calling the base version explicitly** — `Logger::log(msg)` inside
  `TimestampLogger::log()` (notes 24.7)
- **Static binding's limits** — a call through `Logger&` always resolves to
  `Logger::log()` regardless of the runtime object type (notes 24.7 CS6340 tie-in)

**Reused from earlier chapters:**
- `class`, constructors, member-initializer lists, access specifiers (Ch 14)
- `const` member functions (Ch 14–15)
- `std::string` and `+=` concatenation (Ch 5)
- Reference parameters and returning by value (Ch 12)
- Header guards and the `.h`/`.cpp` split (Ch 2)

## Your tasks

The starter compiles immediately but most functions return **placeholders**, so
`make test` is **RED**. Fill in the six `>>> YOUR CODE HERE <<<` blocks in
[`starter/logger.cpp`](starter/logger.cpp). They ramp from warm-up to cliffhanger:

1. **Logger constructor (TASK 1).** Initialize all data members in the
   member-initializer list (`m_lineCount`, `m_dtorTrace`, `m_name`, `m_log`).
   In the constructor body, append `"Logger+"` to `ctorTrace`. This is the
   observable proof that the base constructs first (notes 24.3).

2. **`Logger::log(msg)` (TASK 2).** Return the formatted string
   `"[" + m_name + "] " + msg`. Increment `m_lineCount` and append
   `entry + "\n"` to `m_log`. Return the entry. This function is called by
   `TimestampLogger::log()` via the explicit `Logger::log(msg)` syntax and also
   inherited unchanged by `CountingLogger`.

3. **`TimestampLogger` constructor (TASK 3).** In the member-initializer list,
   call `Logger { name, ctorTrace, dtorTrace }` (the ONLY way to initialize
   the base subobject — notes 24.4) and initialize `m_tag { tag }`. Append
   `"Timestamp+"` in the body. Full trace becomes `"Logger+Timestamp+"`.

4. **`TimestampLogger::log(msg)` (TASK 4).** Prepend the tag: build
   `decorated = m_tag + " " + msg`. Then call `Logger::log(decorated)` —
   the **explicit base-call syntax** is required; plain `log(decorated)` would
   recurse forever (notes 24.7). Return the result from the base.

5. **`CountingLogger::linesLogged()` (TASK 5).** Return `m_lineCount` — the
   **protected** base-class member. This compiles because `CountingLogger` is a
   derived class; public users cannot access `m_lineCount` directly (notes 24.5).
   (The `CountingLogger` constructor body is provided, already correct.)

6. **`processLog(l, msg)` — the cliffhanger (TASK 6).** A one-liner:
   `return l.log(msg);`. The test *intentionally* passes a `TimestampLogger`
   through `Logger& l` and asserts the **base** format — no tag — because static
   binding resolves the call using `l`'s declared type (`Logger`), not the
   runtime type of the object. This is the correct, expected, but limited result.

## Constraints

- **Allowed:** everything through Chapter 23; public inheritance (`: public Base`);
  `protected` members; calling `Base::fn()` explicitly; adding new members to
  derived classes; `std::string` and `+=`/`+`.
- **Forbidden (not taught yet — notes 24 scope):** `virtual` / `override` /
  `final` (Chapter 25 — using them would break the cliffhanger test by design),
  `dynamic_cast` (Chapter 25), multiple inheritance in your implementation
  (the README mentions it in notes 24.9 but the exercise avoids it to keep scope
  clean).
- **Required idioms:**
  - Call the base constructor in the member-initializer list, not in the body.
  - Use `Logger::log(msg)` (not `log(msg)`) when calling the base from a derived
    function — this is the explicit-base-call syntax from notes 24.7.
  - Do not bypass the base constructor by initializing `Logger`'s private
    members directly from a derived class (notes 24.4 — it would not compile).

## Build & run

```sh
make           # compile-check starter/logger.cpp  ->  OK or error
make test      # grade your code  ->  RED until the TASK blocks are filled in
make solution  # build + run the reference demo
make test-solution  # proof the solution is green (used by the author)
make run       # run the starter demo (fun once your code works)
make clean     # remove build artifacts
```

The grader (`tests/tests.cpp`) compiles `starter/logger.cpp` together with the
test file and runs assertions on the API. No `main()` in your `.cpp` needed.

## Success criteria

`make test` prints **PASS ✅  all logger checks passed.** and exits 0. Until then
it prints one `FAIL: …  @line N` per failing check. The grader covers every
concept and several edge cases:

- `ct.substr(0, 7) == "Logger+"` — base token appears *before* derived token in
  the construction trace (notes 24.3)
- `ct == "Logger+Timestamp+"` — both tokens present, correct order
- `dt == "Timestamp-Logger-"` — destruction reverses construction order (notes 24.3)
- `tl.log("connect") == "[audit] [T] connect"` — derived `log()` prepends tag then
  calls base; tag AND base format both present (notes 24.7)
- `cl.linesLogged() == 2` after two calls — protected `m_lineCount` readable from
  derived (notes 24.5)
- `processLog(tl, "via_ref") == "[router] via_ref"` — **NO tag** — static binding
  runs `Logger::log()` even when the object is a `TimestampLogger` (notes 24.7;
  this is the cliffhanger — correct and intentional)

## Hints

<details><summary>Task 1 — member-initializer list syntax and order</summary>

Members are initialized in the order they are *declared* in the class, not the
order they appear in the list. The `Logger` header declares:
```
m_lineCount, m_dtorTrace, m_name, m_log  (in that order)
```
A matching initializer list:
```cpp
Logger::Logger(const std::string& name,
               std::string& ctorTrace,
               std::string& dtorTrace)
    : m_lineCount {}
    , m_dtorTrace { dtorTrace }
    , m_name      { name }
    , m_log       {}
{
    ctorTrace += "Logger+";
}
```
Reference members (`m_dtorTrace`) must be initialized in the list — you cannot
assign a reference in the body.
</details>

<details><summary>Task 2 — build the formatted string</summary>

```cpp
std::string Logger::log(const std::string& msg)
{
    std::string entry { "[" + m_name + "] " + msg };
    ++m_lineCount;
    m_log += entry + "\n";
    return entry;
}
```
Note the space after `"]"`: the format is `"[name] msg"`, not `"[name]msg"`.
</details>

<details><summary>Task 3 — calling the base constructor from the derived initializer list</summary>

The base constructor call goes in the derived constructor's initializer list,
**before** the derived member:
```cpp
TimestampLogger::TimestampLogger(const std::string& name,
                                 const std::string& tag,
                                 std::string& ctorTrace,
                                 std::string& dtorTrace)
    : Logger { name, ctorTrace, dtorTrace }   // base first
    , m_tag  { tag }                          // then derived member
{
    ctorTrace += "Timestamp+";   // base already appended "Logger+"
}
```
`Logger { name, ctorTrace, dtorTrace }` runs the `Logger` constructor, which
itself appends `"Logger+"`. Then the body appends `"Timestamp+"`, giving
`"Logger+Timestamp+"` — base before derived (notes 24.3).
</details>

<details><summary>Task 4 — why Logger::log() and not log()</summary>

```cpp
std::string TimestampLogger::log(const std::string& msg)
{
    std::string decorated { m_tag + " " + msg };
    return Logger::log(decorated);   // REQUIRED: explicit base scope
}
```

`Logger::log(decorated)` tells the compiler: "look up `log` in the `Logger`
scope, not starting from the current class." Without the `Logger::` qualifier,
`log(decorated)` finds `TimestampLogger::log` (the current function) and calls
it recursively — an infinite loop. The explicit qualifier is the syntax from
notes 24.7: "use `Base::fn()` when you mean the inherited function."
</details>

<details><summary>Task 5 — reading a protected base member</summary>

```cpp
int CountingLogger::linesLogged() const
{
    return m_lineCount;   // m_lineCount is protected in Logger
}
```

`m_lineCount` is declared `protected` in `Logger`. That means derived class
*bodies* (like this one) can read and write it — but public users cannot. If you
wrote `cl.m_lineCount` in `main()`, the compiler would reject it. Inside a
member function of `CountingLogger`, it is accessible (notes 24.5).
</details>

<details><summary>Task 6 — the cliffhanger and why the test is correct</summary>

```cpp
std::string processLog(Logger& l, const std::string& msg)
{
    return l.log(msg);
}
```

The test passes a `TimestampLogger` object through `Logger& l`. Inside this
function, `l.log(msg)` resolves to `Logger::log()` — NOT `TimestampLogger::log()`.
This is STATIC BINDING: without `virtual`, function calls are resolved at compile
time using the *declared* type of the reference (`Logger`).

The test:
```cpp
CHECK_EQ(result, "[router] via_ref");   // NO tag "[T]" — static binding
```
is **intentionally asserting the base behavior**. This is correct for a
non-virtual hierarchy. The missing `[T]` is the observable symptom of static
binding — the "problem" this test is designed to reveal. Chapter 25 fixes it.
</details>

## Stretch goals (optional — some need later chapters)

- Change `m_lineCount` from a `protected` data member to a private counter with
  a `protected` accessor function (`int lineCount() const`), and update
  `CountingLogger::linesLogged()` to call it. This demonstrates the Chapter 24
  note that "protected member functions are often a better compromise than
  protected data" (notes 24.5).
- Add a `using Logger::log;` declaration to `TimestampLogger` to explore what
  happens to the base overload set when a derived class introduces its own `log`
  (notes 24.7, 24.8).
- Add `virtual` to `Logger::log()` (Chapter 25 preview) and watch the cliffhanger
  test flip: the `processLog` test will now fail because `TimestampLogger::log()`
  runs and the tag reappears. That is the fix Chapter 25 exists to provide.
- Build a three-level chain: derive `TimestampCountingLogger` from
  `TimestampLogger`, adding both the tag and `linesLogged()`. Verify the
  construction trace becomes `"Logger+Timestamp+TSCounting+"` — three levels,
  base-first (notes 24.3, 24.4 immediate-parent rule).

---

**The cliffhanger.** `processLog` accepts a `Logger&` and calls `l.log(msg)`.
When the object passed is a `TimestampLogger`, you might expect the timestamp
tag to appear — after all, the object *is* a `TimestampLogger`. But without
`virtual`, the compiler resolves the call using the *declared* type of the
reference (`Logger`), not the runtime type of the object. The tag does not
appear. This is STATIC BINDING. It is the correct behavior for a non-virtual
hierarchy, and it is the reason Chapter 25 exists.
