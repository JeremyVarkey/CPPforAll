// Chapter 13 — Compound Types: Enums & Structs · Inventory Ledger  (REFERENCE SOLUTION)
// ─────────────────────────────────────────────────────────────────────────────
// Complete, correct, warning-clean implementation of ../ledger.h.
// Peek only after you have made a genuine attempt in starter/ledger.cpp — the
// learning is in wrestling with the struct/enum API yourself, then comparing.
//
// Every function here demonstrates a different Chapter-13 pattern; the comments
// name the concept and note the section where it is taught.

#include "../ledger.h"

// ─── TASK 1: rarityLabel — switch on a scoped enum (notes 13.4, 13.6) ────────
//
// KEY TERM: SCOPED ENUM — the enumerators live inside Rarity's scope, so every
// case label is fully qualified: Rarity::Common, Rarity::Rare, Rarity::Epic.
// There is no implicit conversion to int; the switch just works on the enum
// type directly, which is cleaner and type-safe.
//
// The `default` case catches any enumerator value that is not explicitly handled
// — important for future-proofing and for -Wswitch (see notes 13.6 best-practice).
//
// Return type: std::string_view because the labels are string LITERALS with static
// storage duration — they outlive every call, so returning a non-owning view is
// safe and cheap (no allocation, no copy). (notes 13.4)
//
std::string_view rarityLabel(Rarity rarity)
{
    switch (rarity)
    {
    case Rarity::Common: return "Common";
    case Rarity::Rare:   return "Rare";
    case Rarity::Epic:   return "Epic";
    default:             return "Unknown";   // future enumerators land here safely
    }
}

// ─── TASK 2: rarityMultiplier — switch returning an int (notes 13.6) ─────────
//
// Same switch pattern as rarityLabel, but the result is an integer.
// We keep the multiplier in ONE place — if a game designer changes Epic from 10x
// to 15x, they edit only this function. itemWorth (below) and any future code
// that calls rarityMultiplier get the new value automatically.
//
// CS6340 tie-in: LLVM passes use exactly this pattern — a helper that maps an
// enum (e.g. `Instruction::Opcode`) to a property (cost, latency, string name)
// so the mapping stays in one spot and callers stay readable.
//
int rarityMultiplier(Rarity rarity)
{
    switch (rarity)
    {
    case Rarity::Common: return 1;
    case Rarity::Rare:   return 3;
    case Rarity::Epic:   return 10;
    default:             return 1;   // safe fallback: no additional multiplier
    }
}

// ─── TASK 3: itemWorth — const reference parameter + member access (notes 13.10)
//
// KEY TERM: CONST REFERENCE — `const Item&` is the standard way to pass a
// struct for READ-ONLY access without copying it.
//   • Without the `&`: the entire Item (including its std::string member) would
//     be copied on every call — expensive and pointless.
//   • Without `const`: the function could accidentally modify the caller's data.
//   • With `const Item&`: zero-copy, read-only. This is the canonical pattern
//     for read-only struct parameters (notes 13.10).
//
// Member access uses `.` — the item is a reference, not a pointer, so `.` is
// correct (not `->`, which is for pointers — see notes 13.12).
//
int itemWorth(const Item& item)
{
    return item.quantity * item.unitValue * rarityMultiplier(item.rarity);
    //       ^^^^^^^^^     ^^^^^^^^^^^     ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
    //       member         member          delegate to the multiplier helper —
    //       access         access          do NOT inline 1/3/10 here!
}

// ─── TASK 4: makeItem — returning a struct by value (notes 13.8, 13.10) ───────
//
// AGGREGATE INITIALIZATION: brace-initialize an Item using member values in
// DECLARATION ORDER (name, rarity, quantity, unitValue) — exactly the order
// they appear in the struct definition in ledger.h (notes 13.8).
//
// WHY RETURN BY VALUE: structs are just data. The compiler performs NRVO (Named
// Return Value Optimization) or copy elision in C++17, so this is typically
// zero-copy at the machine level. Returning structs avoids messy out-parameters
// and clearly communicates "this function produces an Item" (notes 13.10).
//
// std::string_view -> std::string: The `name` parameter is a non-owning view.
// Constructing `std::string{name}` materializes an owning copy for the struct's
// member. (The struct must OWN its string so it doesn't dangle — notes 13.11.)
//
Item makeItem(std::string_view name, Rarity rarity, int quantity, int unitValue)
{
    return Item{ std::string{name}, rarity, quantity, unitValue };
    //           ^^^^^^^^^^^^^^^^   ^^^^^^  ^^^^^^^^  ^^^^^^^^^
    //           aggregate init: fills members in declaration order
}

// ─── TASK 5: restock — non-const reference (mutation through a ref) ─────────
//
// KEY TERM: NON-CONST REFERENCE — `Item& item` (no `const`) lets us modify the
// caller's struct in place. This is the canonical idiom for "in/out parameters"
// and for functions that logically mutate an object (notes 13.10, 13.12).
//
// Without the `&`, we would modify a LOCAL copy and the caller would never see
// the change — the classic Ch 12 mistake. The `&` is the whole point here.
//
// Clamping negative amounts to 0 prevents accidentally *reducing* stock via the
// restock path (a different function would handle selling/consuming).
//
void restock(Item& item, int amount)
{
    if (amount < 0)
        amount = 0;            // treat negative restock as no-op (clamp to 0)
    item.quantity += amount;   // mutate through the reference — caller sees this
}

// ─── TASK 6: isSameItem — comparing struct fields (notes 13.7, 13.10) ────────
//
// BOTH parameters are `const Item&` — read-only references; no copies needed.
// We compare the CATALOGUE IDENTITY fields: name and rarity.
//   • `std::string` supports `==` for lexicographic comparison.
//   • `enum class` supports `==` directly — no cast needed (notes 13.6).
//     Compare with `a.rarity == b.rarity` (not static_cast<int>).
//
// Why NOT compare quantity and unitValue? Two stockrooms can carry different
// amounts of the same item. Identity is about WHAT the item is, not HOW MANY.
//
bool isSameItem(const Item& a, const Item& b)
{
    return a.name == b.name && a.rarity == b.rarity;
    //     ^^^^^^^^^^^^^^^^    ^^^^^^^^^^^^^^^^^^^^^^
    //     string comparison   enum comparison — works directly on scoped enums
}
