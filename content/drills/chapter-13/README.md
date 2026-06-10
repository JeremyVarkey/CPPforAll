# Chapter 13 — Compound Types: Enums & Structs · Project: Inventory Ledger

> Reinforces [`../../notes/chapter-13.md`](../../notes/chapter-13.md) · LearnCpp [Chapter 13](https://www.learncpp.com/)

## The project

You are building the **Inventory Ledger** — a small library that tracks game-shop
items by rarity tier. An item has a name, a `Rarity` (Common, Rare, or Epic), a
stock quantity, and a base unit value. The library computes how much a full stock
is worth, restocks shelves, and answers catalogue-identity queries.

The design enforces the two central lessons of Chapter 13:

1. **Scoped enumerations (`enum class`)** — `Rarity` is an `enum class`. Its
   enumerators live inside its own scope (`Rarity::Common`, not `Common`), it
   will not silently convert to `int`, and it switches cleanly on all three values.
   You will implement the enum-to-string and enum-to-multiplier helpers that every
   real codebase needs once it adopts scoped enums.

2. **Struct aggregates and their free-function API** — `Item` is a data-only
   `struct` with four members. All the logic that operates on items lives in
   **free functions** (outside the struct), not as member functions. That
   boundary is deliberate: Chapter 14 introduces member functions; keeping
   Chapter 13 structs data-only makes the transition from "bag of data" to
   "object with behaviour" vivid. You will practice aggregate initialization,
   passing structs by const reference, returning them by value, and mutating
   through a non-const reference.

CS6340 connection: LLVM uses this exact pairing constantly — an enum like
`Instruction::BinaryOps` or `AtomicOrdering` combined with free helper functions
that map enumerators to strings, costs, or legality tables. The `struct` pattern
appears in every value-carrying utility type (e.g. `DebugLoc`, `MaybeAlign`).

## Concepts practiced

**New this chapter (notes 13.1–13.12):**
- **`enum class` (scoped enum)** — enumerators scoped to the type, no implicit
  int conversion, switch with `default` arm (notes 13.6)
- Contrast with **unscoped `enum`** — name leakage and implicit int conversion
  are the problems `enum class` solves (notes 13.2, 13.3)
- **`struct` aggregate** — named members, member access with `.` (notes 13.7)
- **Aggregate initialization** — brace-init fills members in declaration order
  (notes 13.8)
- **Default member initializers** — members start in a safe state (notes 13.9)
- **Passing structs by `const` reference** — read-only, no copy (notes 13.10)
- **Returning structs by value** — clean API, compiler handles copy elision
  (notes 13.10)
- **Mutating through a non-const reference** — the `restock` pattern (notes 13.10, 13.12)
- **Enum-to-string helper** using a `switch` (notes 13.4)
- Struct containing a program-defined enum member (notes 13.11)

**Reused from earlier chapters:**
- `std::string` and `std::string_view` (Ch 5)
- `const` correctness (Ch 5)
- `if` / `switch` / `break` / `default` (Ch 8)
- References (`&`) and const references (`const &`) (Ch 12)
- `static_cast` used *only* in tests for out-of-range enum edges (Ch 4/10)

## Your tasks

The starter compiles immediately but every function returns a **placeholder**, so
`make test` starts **RED**. Fill in the six `>>> YOUR CODE HERE <<<` blocks in
[`starter/ledger.cpp`](starter/ledger.cpp). Tasks ramp from warm-up to capstone:

1. **`rarityLabel` — switch on a scoped enum.** Return `"Common"`, `"Rare"`,
   `"Epic"`, or `"Unknown"` for the `default` arm. Use a `switch` on the scoped
   enum — prefix each case label with the type name: `case Rarity::Common:`.
   Return `std::string_view` (string literals have static storage, so a non-owning
   view is safe and allocates nothing).

2. **`rarityMultiplier` — switch returning an int.** Map
   `Common → 1`, `Rare → 3`, `Epic → 10`, `default → 1`. Same switch pattern as
   Task 1. Keeping the multiplier in one helper means every caller stays in sync
   when the numbers change.

3. **`itemWorth` — const reference + member access.** Compute
   `item.quantity × item.unitValue × rarityMultiplier(item.rarity)`. The
   parameter is `const Item&` — a **const reference** avoids copying the
   `std::string` member and promises you will not mutate the caller's data.
   Access members with `.`. Delegate to `rarityMultiplier`; do not inline the
   numbers.

4. **`makeItem` — return a struct by value.** Aggregate-initialize an `Item`
   with the four supplied fields and return it. Member order must match the
   declaration in `ledger.h`: `name`, `rarity`, `quantity`, `unitValue`.
   Convert the `std::string_view name` parameter to `std::string` for the owning
   member.

5. **`restock` — mutate through a non-const reference.** Add `amount` to
   `item.quantity`. If `amount` is negative, clamp it to `0` first (a restock
   function should never accidentally reduce stock). The `Item& item` parameter —
   a **non-const reference** — is what makes the mutation visible to the caller.

6. **`isSameItem` — compare struct fields (the capstone).** Return `true` if
   `a.name == b.name` and `a.rarity == b.rarity`. Quantity and unit value are
   irrelevant to catalogue identity. Both parameters are `const Item&`. Scoped
   enum values compare with `==` directly — no cast needed.

You may **not** edit [`ledger.h`](ledger.h) or anything in `tests/` — those are
the contract and the grader.

## Constraints

**Allowed constructs:**
- `enum class`, `struct`, aggregate initialization with `{}`
- Member access with `.`
- `switch` / `case` / `default` / `break` (notes 13.4, 13.6)
- `if` / `else` for clamping
- `const Item&` and `Item&` parameters (Ch 12 refs)
- `std::string` and `std::string_view` (Ch 5)
- Integer arithmetic (`*`, `+`, `+=`) (Ch 1)
- `bool`, `int`, function templates already in scope (Ch 4, 11)

**Forbidden (not yet taught or explicitly out-of-scope):**
- **Member functions / methods** inside the struct — that is Chapter 14. Keep
  `Item` data-only.
- **Operator overloading** (`operator==`, `operator<<`) — Chapter 21.
- **`std::vector`** — Chapter 16.
- **Constructors** — Chapter 14. Use aggregate initialization (`{}` braces) only.
- Raw `int` magic numbers inlined into `itemWorth` — delegate to
  `rarityMultiplier` so the mapping is centralized.

**Required idioms:**
- Aggregate initialization everywhere: `Item x { ... };`, not `Item x; x.name = ...;`
- Default member initializers in `ledger.h` — already provided; do not remove them.
- `const Item&` for every read-only struct parameter; `Item&` (no `const`) only
  when mutation is the goal.

## Build & run

```sh
make            # compile-check starter/ledger.cpp  (warning-clean)
make test       # grade your code  ->  RED until the TASK blocks are filled in
make solution   # run the grader against the reference solution
make test-solution  # verify the reference passes all checks
make clean      # remove build artifacts
```

`make test` is the grader. `make solution` shows the reference output so you can
check your understanding — but peek at `solution/ledger.cpp` only after you have
made a genuine attempt.

## Success criteria

`make test` prints **PASS ✅  all ledger checks passed.** and exits 0. Until then
it prints one `FAIL: … @line N` per broken check (the condition and the line in
[`tests/tests.cpp`](tests/tests.cpp) that failed) and ends with
`FAIL ❌  N check(s) failed`. The grader covers:

- Every named Rarity round-trips through `rarityLabel` and `rarityMultiplier`.
- An out-of-range `static_cast<Rarity>(99)` hits the `default` arm — the switch
  must be exhaustive.
- `itemWorth` with **zero quantity** returns `0` (the zero-edge).
- `itemWorth` with **zero unit value** returns `0`.
- `makeItem` fields are set correctly in declaration order.
- `restock` with a **negative amount** does NOT reduce stock (clamp to 0).
- `isSameItem` returns `true` for two Items with the same name and rarity but
  **different** quantity/unitValue — those fields must be ignored.
- `isSameItem` returns `false` when only the rarity differs, and when only the
  name differs.

Turning that wall of red into a single green line is the whole exercise.

## Hints

<details><summary>Task 1 — switching on a scoped enum</summary>

```cpp
switch (rarity)
{
case Rarity::Common: return "Common";
case Rarity::Rare:   return "Rare";
case Rarity::Epic:   return "Epic";
default:             return "Unknown";
}
```

Each case label must use the full `Rarity::` prefix — that is what "scoped" means.
The `default` arm is essential: it satisfies `-Wswitch-default` and catches any
value that was created via `static_cast` from an integer.
</details>

<details><summary>Task 3 — why const reference and how to access members</summary>

The function signature is:
```cpp
int itemWorth(const Item& item)
```
`const` — you promise not to modify the item.
`&` — no copy; the parameter IS the caller's object (notes 13.10, 13.12).

Inside the body, access members with `.` (not `->`, which is for pointers):
```cpp
return item.quantity * item.unitValue * rarityMultiplier(item.rarity);
```
</details>

<details><summary>Task 4 — aggregate initialization and std::string_view → std::string</summary>

```cpp
Item makeItem(std::string_view name, Rarity rarity, int quantity, int unitValue)
{
    return Item{ std::string{name}, rarity, quantity, unitValue };
}
```
`std::string{name}` converts the non-owning view to an owning string for the
struct member — important so the member doesn't dangle (notes 13.11).
The brace list fills members in **declaration order**: name, rarity, quantity,
unitValue — exactly as they appear in the struct definition in `ledger.h`.
</details>

<details><summary>Task 5 — non-const reference and negative clamping</summary>

```cpp
void restock(Item& item, int amount)
{
    if (amount < 0)
        amount = 0;
    item.quantity += amount;
}
```
The `&` (no `const`) is the entire difference between "modifies the caller's
item" and "modifies a throwaway copy that vanishes when the function returns".
</details>

<details><summary>Task 6 — comparing enum class values with ==</summary>

```cpp
bool isSameItem(const Item& a, const Item& b)
{
    return a.name == b.name && a.rarity == b.rarity;
}
```
Scoped enum values support `==` directly. No `static_cast<int>` needed — the
whole point of `enum class` is that it gives you a proper type with proper
comparisons (notes 13.6).
</details>

## Stretch goals (optional — name later-chapter concepts)

- **Add a `printItem` function** that outputs `"Sword [Rare] qty=2 worth=240 coins"`.
  Right now you would pass an `std::ostream& out` parameter and call
  `rarityLabel`/`itemWorth` inside. In Chapter 21 you could overload `operator<<`
  so `std::cout << sword` just works.
- **Replace `isSameItem` with `operator==`** once you reach Chapter 21 operator
  overloading.
- **Add a `totalLedgerWorth(Item items[], int count)` function** once you learn
  C-style arrays (Ch 17) or `std::vector` (Ch 16). The loop accumulates
  `itemWorth(items[i])`.
- **Add a `Rarity fromString(std::string_view)` returning `std::optional<Rarity>`**
  using the `std::optional` pattern from Chapter 12 (already in scope!). This is
  the exact parse-from-CLI-arg pattern shown in notes 13.4.
