// Chapter 13 — Compound Types: Enums & Structs · Inventory Ledger   (STARTER)
// ─────────────────────────────────────────────────────────────────────────────
// Fill in the six TASK blocks below. Each maps 1:1 to a task in the README and
// to a declaration in ../ledger.h.  The function bodies currently return STUB
// VALUES so the file compiles immediately — that is why `make test` is RED right
// now.  Your job is to implement the real logic and turn it GREEN.
//
//     make build          compile your code (should already work as-is)
//     make test           grade it          (RED until you fill these in)
//     make solution       build + run the reference solution if you get stuck
//     make test-solution  verify the reference passes all checks
//
// SCOPE REMINDER (notes 13.1–13.12):
//   • Allowed up through Ch 12: refs, const refs, static_cast, std::string,
//     std::string_view, int arithmetic, if/switch/for (Ch 8), bool (Ch 4).
//   • NEW this chapter: enum class, struct, aggregate init, member access with .,
//     pass/return by const ref or value (notes 13.6–13.10).
//   • FORBIDDEN here: member functions/methods (Ch 14), operator overloading (Ch 21),
//     std::vector (Ch 16). Keep the struct DATA-ONLY. That boundary IS the lesson.
//
// KEY TERMS to keep in mind as you read the scaffolding:
//   SCOPED ENUM — enum class with :: accessor (notes 13.6)
//   AGGREGATE   — data-only struct, initialized with {} braces (notes 13.7–13.9)
//   CONST REF   — const Item& avoids copying the std::string member (notes 13.10)

#include "../ledger.h"

// ─── TASK 1: rarityLabel — switch on a scoped enum ───────────────────────────
//
// Return a std::string_view label for `rarity`:
//   Rarity::Common -> "Common"
//   Rarity::Rare   -> "Rare"
//   Rarity::Epic   -> "Epic"
//   anything else  -> "Unknown"
//
// Use a SWITCH statement (practicing switch on a scoped enum is the point):
//   switch (rarity)
//   {
//   case Rarity::Common: return "Common";
//   ...
//   default: return "Unknown";
//   }
//
// DO NOT convert to int. Scoped enums switch cleanly (notes 13.6).
//
//   >>> YOUR CODE HERE <<<
//
std::string_view rarityLabel(Rarity /*rarity*/)
{
    return "Unknown";   // stub — replace with the switch above
}
// ─────────────────────────────────────────────────────────────────────────────

// ─── TASK 2: rarityMultiplier — switch returning an int ──────────────────────
//
// Return the integer value multiplier for `rarity`:
//   Rarity::Common -> 1
//   Rarity::Rare   -> 3
//   Rarity::Epic   -> 10
//   default        -> 1   (safe fallback for any future enumerator)
//
// Use a SWITCH on the scoped enum, matching the style in Task 1.
//
//   >>> YOUR CODE HERE <<<
//
int rarityMultiplier(Rarity /*rarity*/)
{
    return 1;   // stub — correct only for Common; Rare and Epic will be wrong
}
// ─────────────────────────────────────────────────────────────────────────────

// ─── TASK 3: itemWorth — pass by const ref, multiply three values ─────────────
//
// Total coin value of all stock of this item:
//   item.quantity  ×  item.unitValue  ×  rarityMultiplier(item.rarity)
//
// IMPORTANT: the parameter is `const Item& item` — a CONST REFERENCE.
//   • `const` means you cannot modify the struct (read-only).
//   • `&` (reference) means no copy is made — especially important for the
//     std::string member inside Item (notes 13.10, 13.12).
//
// Access members with the DOT operator: item.quantity, item.rarity, etc.
// Reuse rarityMultiplier(...) — do not inline the multiplier values here.
//
//   >>> YOUR CODE HERE <<<
//
int itemWorth(const Item& /*item*/)
{
    return 0;   // stub — always 0 (correct only for quantity-0 edge case)
}
// ─────────────────────────────────────────────────────────────────────────────

// ─── TASK 4: makeItem — return a struct by value ──────────────────────────────
//
// Construct and RETURN an Item with the four supplied fields.
// Use AGGREGATE INITIALIZATION with braces (notes 13.8):
//   return Item{ std::string{name}, rarity, quantity, unitValue };
//
// Notes:
//   • `name` is a std::string_view; the Item's std::string member needs a copy.
//     Write  std::string{name}  to construct the owning string from the view.
//     The std::string-from-string_view constructor is EXPLICIT, so a bare
//     `name` will NOT implicitly convert — you must spell out std::string{name}.
//   • Return by VALUE (notes 13.10): the compiler performs copy elision (NRVO),
//     so there is no copy overhead in practice.
//
//   >>> YOUR CODE HERE <<<
//
Item makeItem(std::string_view /*name*/, Rarity /*rarity*/,
              int /*quantity*/, int /*unitValue*/)
{
    return Item{};   // stub — returns a default-constructed empty Item
}
// ─────────────────────────────────────────────────────────────────────────────

// ─── TASK 5: restock — mutate through a non-const reference ──────────────────
//
// Add `amount` units to item.quantity. If `amount` is negative, treat it as 0
// (do NOT decrease the stock count below what was there).
//
// PASS BY NON-CONST REFERENCE `Item& item` — that is what lets us modify the
// caller's object. The & is essential: without it, we'd modify a local copy and
// the caller would never see the change (notes 13.10, 13.12).
//
// Pattern:
//   if (amount < 0) amount = 0;
//   item.quantity += amount;
//
//   >>> YOUR CODE HERE <<<
//
void restock(Item& /*item*/, int /*amount*/)
{
    // stub — does nothing; the quantity will never change in tests
}
// ─────────────────────────────────────────────────────────────────────────────

// ─── TASK 6: isSameItem — compare two items by name and rarity ───────────────
//
// Return true if a and b have the same name AND the same rarity.
// The quantity and unitValue do NOT matter for catalogue identity.
//
// Both parameters are `const Item&` — read-only references; no copies needed.
// Access members with `.`:  a.name,  a.rarity,  b.name,  b.rarity
// std::string supports == for lexicographic comparison.
// enum class values support == directly (no cast needed, notes 13.6).
//
//   >>> YOUR CODE HERE <<<
//
bool isSameItem(const Item& /*a*/, const Item& /*b*/)
{
    return false;   // stub — always says "different" (wrong for matching items)
}
// ─────────────────────────────────────────────────────────────────────────────
