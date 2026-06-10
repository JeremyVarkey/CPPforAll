# Chapter 23 — Object Relationships: Garage Simulation

> Reinforces [`../../notes/chapter-23.md`](../../notes/chapter-23.md) · LearnCpp [Chapter 23](https://www.learncpp.com/cpp-tutorial/object-relationships/)

## The project

You build a tiny **Garage Simulation** whose constructors and destructors write
to a shared trace log. That log is the whole point: every time an `Engine` is
created, a `Car` is created, or a `Mechanic` leaves, a string like
`"Engine built: V8-Turbo"` or `"Car destroyed: Sedan"` is appended to
`trace::log()`. The automated tests then assert the **exact contents and order**
of the log — so you cannot fake the result. You have to understand, and correctly
implement, the C++ lifetime rules that produce that specific sequence.

The four classes each model a different ownership relationship from notes 23.1–23.7:
`Engine` is a **composed** value-member of `Car` (it constructs before and destroys
after Car's own body — the standard's member-init-order rule made observable);
`Driver` is **aggregated** by `Car` (held as a `Driver*`, never deleted — the Car
has no authority over the Driver's lifetime); `Mechanic` is a **dependency** (passed
by reference to `Car::tuneUp()` and not stored — when the function returns, the
relationship is over); and `RouteList` is a **container class** supporting
`std::initializer_list` construction so you can write
`RouteList r { "Home", "Gas station", "Destination" }`.

The combination of live trace checks and scope blocks makes the abstract vocabulary
— composition, aggregation, dependency — *physical*: you write the code, run the
tests, and either the log says the right thing in the right order, or it does not.

## Concepts practiced

- **Composition** (`Engine` inside `Car` — owned part, value member): members
  initialise (in declaration order) BEFORE the constructor body runs, and destroy
  (in REVERSE declaration order) AFTER the destructor body runs (notes 23.2)
- **Aggregation** (`Driver*` inside `Car` — non-owning part): whole references
  external object; destroying `Car` must NOT destroy `Driver`; the Driver must
  outlive the Car in a correct program (notes 23.3)
- **Dependency** (`Mechanic&` parameter in `Car::tuneUp()` — temporary borrow):
  the mechanic is used for one call and not stored; contrast with an association
  which would store `Mechanic*` as a field (notes 23.5)
- **Container class** with `std::initializer_list<T>` constructor, enabling
  brace-init syntax for custom classes (notes 23.6, 23.7)
- **Trace-log technique** for asserting construction/destruction order (a pattern
  you reuse whenever lifetime bugs are hard to spot statically)
- **Member-init-list** to initialise composed members before the constructor body
  (notes 23.2 — the standard rule proven by the trace)
- Reused from earlier chapters: **classes, access specifiers, constructors,
  destructors** (Ch 14–15), **`std::string` / `std::string_view`** (Ch 5),
  **`std::vector`** (Ch 16), **`assert`** for bounds checks (Ch 9),
  **pointers and `nullptr`** (Ch 12), **`const` member functions** (Ch 14)

## Your tasks

The starter compiles immediately but every TASK block is a stub, so `make test`
starts **RED**. Fill in the five `>>> YOUR CODE HERE <<<` blocks in
[`starter/garage.cpp`](starter/garage.cpp). They ramp from simple to composite:

1. **`Engine` ctor/dtor (composition part).** Implement `Engine::Engine(model)`:
   initialise `m_model` and append `"Engine built: " + model` to `trace::log()`.
   Implement `Engine::~Engine()`: append `"Engine destroyed: " + m_model`.
   Implement `Engine::model()` to return `m_model`. This task practices the
   composition lifecycle: Engine is the owned part, and the trace sequence proves
   that it constructs/destroys around the Car's own ctor/dtor body.

2. **`Driver` ctor/dtor (aggregated entity).** Implement `Driver::Driver(name)`:
   initialise `m_name` and append `"Driver built: " + name`. Implement
   `Driver::~Driver()`: append `"Driver destroyed: " + m_name`. Implement
   `Driver::name()` to return `m_name`. This is the entity that Car will
   *reference but not own*. The test verifies the Driver is alive after the Car
   that referenced it is destroyed.

3. **`Mechanic` ctor/dtor and helpers (dependency entity).** Implement
   `Mechanic::Mechanic(name)` (trace: `"Mechanic on duty: " + name`),
   `Mechanic::~Mechanic()` (trace: `"Mechanic off duty: " + m_name`),
   `Mechanic::name()`, `Mechanic::tuneUpCount()`, and `Mechanic::recordTuneUp()`
   (increments `m_tuneUpCount`). The test calls `tuneUp()` three times through
   two different Cars and checks the counter accumulates correctly.

4. **`Car` ctor/dtor and all `Car` member functions (the composite whole).** This
   is the central TASK — all three relationships meet here:
   - **Constructor:** initialise `m_engine{engineModel}` and `m_make{make}` in the
     member-init list (declaration order); in the body, append
     `"Car built: " + make`. Because `m_engine` is a value member declared *before*
     `m_make`, its constructor runs first — but both run before your body code.
   - **Destructor:** append `"Car destroyed: " + m_make`. Do **NOT** call
     `delete m_driver` — the Car does not own the Driver (aggregation rule).
     After your destructor body, `m_engine` destructs automatically.
   - **`setDriver` / `driver()`:** store/return the pointer; no ownership transfer.
   - **`tuneUp(Mechanic&)`:** call `mechanic.recordTuneUp()`, append
     `"Tuned up by: " + mechanic.name()`. Do NOT store the mechanic in any field.
   - **`make()` / `engineModel()`:** return `m_make` and `m_engine.model()`.

5. **`RouteList` initializer_list constructor (container class).** Implement
   `RouteList::RouteList(std::initializer_list<std::string_view> waypoints)`:
   iterate the list with a range-for loop and `push_back` each waypoint (as a
   `std::string`) into `m_waypoints`. The other five member functions (`size`,
   `at`, `add`, `clear`) are already provided. This task proves that custom
   classes can support `{ "A", "B", "C" }` brace-init syntax (notes 23.7).

## Constraints

**Allowed:** `class`, constructors with member-init lists, destructors, `this`
(implicit), `std::string`, `std::string_view`, `std::vector`, `std::initializer_list`,
`assert`, raw pointers (`Driver*`, `nullptr`), range-for loops, `push_back`,
and anything from Chapters 1–22.

**Forbidden (not yet taught):** `virtual`, `override`, `dynamic_cast` (Ch 25);
inheritance `: public Base` (Ch 24); `std::weak_ptr` / `std::shared_ptr` ownership
tricks beyond Ch 22 coverage; `goto`; raw `new`/`delete` inside your task code
(the scaffolding already manages ownership correctly with value members and `vector`).

**Required idioms:**
- Use a **member-init list** (`: m_engine{engineModel}, m_make{make}`) in
  `Car`'s constructor — do not assign inside the body, and list members in their
  **declaration order** (notes 23.2; `-Wextra` warns about re-ordering).
- **Never `delete m_driver`** in `Car::~Car()` — this is the aggregation rule made
  mandatory.
- **Never store** `mechanic` in any `Car` data member — the dependency must be
  temporary.
- Use a **range-for loop** to iterate `std::initializer_list` (it has no `operator[]`).

## Build & run

```sh
make            # compile-check starter/garage.cpp (warning-clean)
make test       # grade your code  ->  RED until the TASK blocks are filled in
make solution   # run the reference solution (peek if stuck)
make clean      # remove build artifacts
```

`make test` is the grader. Each `FAIL:` line names the broken condition and its
line number in `tests/tests.cpp`. A `[trace dump]` section is printed when checks
fail, showing the actual log contents to help diagnose ordering mistakes.

## Success criteria

`make test` prints **PASS ✅ all garage checks passed.** and exits 0.

The checks that separate a correct implementation from a plausible near-miss:

- **Scenario A (composition order):** `trace[0] == "Engine built: V8-Turbo"` and
  `trace[1] == "Car built: Sedan"` — the Engine must trace BEFORE the Car body.
  On destruction: `trace[2] == "Car destroyed: Sedan"` THEN
  `trace[3] == "Engine destroyed: V8-Turbo"` — the Car dtor body fires FIRST.
  Swapping any pair fails this scenario.

- **Scenario B (aggregation isolation):** after `innerCar` goes out of scope,
  `alice.name() == "Alice"` must still hold. If `Car::~Car()` mistakenly calls
  `delete m_driver`, the program crashes or `alice` is corrupted. The test also
  confirms `Driver destroyed: Alice` appears in the trace AFTER all `Car destroyed`
  events, proving the Driver's lifetime extends past the Car's.

- **Scenario D (dependency reuse):** `bob.tuneUpCount() == 3` after three
  `tuneUp()` calls across two Cars. If `tuneUp()` does not call
  `mechanic.recordTuneUp()`, the count stays at 0.

- **Scenario E (initializer_list ctor):** `route.size() == 4` for
  `RouteList route { "Home", "Gas station", "Highway", "Destination" }`.
  If the initializer_list constructor stub is left empty, size stays 0 and every
  `at()` check reports the wrong result.

- **Scenario G (LIFO destruction order):** two Cars in the same scope destroy
  in reverse declaration order. The trace must show `second` destructs before
  `first`, and each Engine destructs immediately after its own Car body.

## Hints

<details><summary>Task 1 — Engine ctor: the member-init trap</summary>

You cannot call `trace::log().push_back(...)` inside the member-init expression
itself (`:m_model{model}, <can't call trace here>`). Write it in the constructor
body instead:

```cpp
Engine::Engine(std::string_view model)
    : m_model { model }
{
    trace::log().push_back("Engine built: " + std::string(model));
}
```

`std::string(model)` converts `string_view` to `string` for the `+` operator.
</details>

<details><summary>Task 4 — Car ctor: member-init order rule</summary>

Declare the init list in the **same order as the class member declarations** in
`garage.h` (`m_engine` first, then `m_make`). C++ always constructs in declaration
order regardless of init-list order, but `-Wextra` warns (and some compilers error)
when the list order doesn't match. The trace will show Engine first regardless:

```cpp
Car::Car(std::string_view make, std::string_view engineModel)
    : m_engine { engineModel }   // constructed FIRST (declared first in header)
    , m_make   { make }
{
    trace::log().push_back("Car built: " + std::string(make));
}
```

Note: `m_driver` is not listed — it is default-initialised to `nullptr` by the
`Driver* m_driver { nullptr };` in-class member initialiser.
</details>

<details><summary>Task 4 — Car dtor: the aggregation rule</summary>

```cpp
Car::~Car()
{
    trace::log().push_back("Car destroyed: " + std::string(m_make));
    // m_driver is NOT deleted — Car does not own the Driver.
    // After this body, m_engine's destructor fires automatically.
}
```

If you write `delete m_driver;` here, Scenario B crashes because the Driver is
a local variable in the test, not heap-allocated. Even if it were heap-allocated,
deleting it here would be wrong: the caller still holds a reference to it.
Aggregation means: the whole **observes** the part, but does not control its fate.
</details>

<details><summary>Task 5 — RouteList initializer_list ctor</summary>

`std::initializer_list<T>` supports range-for and iterators, but not `operator[]`.
The simplest body:

```cpp
RouteList::RouteList(std::initializer_list<std::string_view> waypoints)
{
    for (std::string_view wp : waypoints)
        m_waypoints.push_back(std::string(wp));
}
```

`std::string_view` is a lightweight view — `push_back(std::string(wp))` copies the
characters into an owned `std::string` stored in `m_waypoints`. Storing the
`string_view` directly would be a lifetime bug (the temporary brace list's storage
may not last past the constructor call).
</details>

<details><summary>Why is the trace order exactly what it is? (the deep answer)</summary>

C++ standard rules for object lifetime (23.2):

1. **Construction:** non-static data members are initialised in their **declaration
   order** (not init-list order), before the constructor body runs.
2. **Destruction:** the destructor body runs first; then members are destroyed in
   **reverse declaration order**.

So for `Car`:
- `m_engine` is declared before `m_make`, so `Engine::Engine` fires first in the
  init list, before the Car body's `trace::log().push_back("Car built: ...")`.
- On destruction, `Car::~Car()`'s body runs (appending "Car destroyed"), and then
  `m_engine`'s destructor fires (appending "Engine destroyed"). Hence the Car dtor
  body trace comes BEFORE the Engine dtor trace.

The test asserts this order exactly — not because of coincidence, but because it is
what the standard mandates. Understanding this rule is essential for writing RAII
types and composed objects correctly in real C++ code.
</details>

## Stretch goals (optional — some need later chapters)

- Add a `Garage` class that stores `std::vector<Car*>` (an **aggregation container**
  of non-owned Cars) and verify that destroying the `Garage` does not destroy any
  Cars (the same aggregation lifetime rule at container scale).
- Add a `Garage::addCar(Car&)` and `Garage::removeCar(Car&)` — bidirectional
  association where each `Car` could also hold a `Garage*`.
- Make `Driver` move-constructible and store the driver by
  `std::unique_ptr<Driver>` in a separate "driver registry" class — an example of
  composition through owning smart pointer (notes 23.2 variant, Ch 22 tools).
- Add `Car::lastMechanic()` returning a stored `Mechanic*` — observe how storing
  the mechanic transforms the **dependency** into an **association** (notes 23.5).
  Then remove the storage to put it back to a dependency and notice the design
  difference.
- Replace `RouteList`'s internal `std::vector<std::string>` with a manually
  managed `new[]`/`delete[]` array (Ch 19 tools) and implement a proper copy
  constructor and assignment operator to avoid shallow-copy bugs (notes 23.7's
  `TinyArray` example).
