// ============================================================================
//  ledger.h  —  PUBLIC INTERFACE of the Inventory Ledger library  (Chapter 13)
// ----------------------------------------------------------------------------
//  This header is COMPLETE and PROVIDED. Do not edit it.
//
//  This header exercises the two central concepts of Chapter 13:
//
//    1) SCOPED ENUMERATION (enum class, notes 13.6)
//       Rarity is an `enum class` — a PROGRAM-DEFINED TYPE whose values are a
//       small, closed set of named alternatives. The `class` keyword means:
//         • enumerators live INSIDE the type's scope (Rarity::Common, not Common)
//         • NO implicit conversion to int — you must use static_cast if you ever
//           need the underlying integer (notes 13.6 covers static_cast<int>(...))
//         • No name-collision risk with unscoped enums (notes 13.2, 13.6)
//
//    2) STRUCT AGGREGATE (struct + aggregate initialization, notes 13.7–13.10)
//       Item is a DATA-ONLY struct — it has named MEMBERS and is initialized
//       with braces:  Item item { "Sword", Rarity::Rare, 5, 40 };
//       Important boundaries:
//         • structs stay DATA-ONLY here; member FUNCTIONS are Chapter 14
//         • the FREE FUNCTIONS below operate ON Items (extern to the struct)
//         • aggregate init fills members in DECLARATION ORDER
//
//  Header guard (notes 13.1 multi-file / Chapter 2): prevents double-inclusion.
// ============================================================================

#ifndef LEDGER_H
#define LEDGER_H

#include <string>        // std::string  — owning string member (Chapter 5)
#include <string_view>   // std::string_view — non-owning view for labels (Chapter 5)

// ─── enum class Rarity ───────────────────────────────────────────────────────
//
//  SCOPED ENUM (notes 13.6): enumerators are QUALIFIED by the type name.
//  Access them as:  Rarity::Common,  Rarity::Rare,  Rarity::Epic
//
//  CS6340 tie-in: LLVM itself uses scoped enums extensively — for example,
//  `Instruction::BinaryOps` and `AtomicOrdering` are `enum class` types that
//  describe what an `Instruction` does. Same concept, bigger domain.
//
//  FIXED UNDERLYING TYPE `: int` (notes 13.3 — "Underlying type"). This pins
//  Rarity's storage to `int`. It does NOT re-enable implicit enum->int
//  conversion (you still need static_cast<int> for the integer value), but it
//  DOES make one thing well-defined that the grader relies on:
//
//      static_cast<Rarity>(99)
//
//  Without a fixed underlying type, an enum's representable range is only the
//  smallest bit-field that holds its enumerators — here {0,1,2} fits in 2 bits,
//  giving the range [0..3]. Casting 99 (outside that range) into the enum is
//  UNDEFINED BEHAVIOR in C++17 (notes 13.3 warns parsing external integers can
//  produce values "that do not match any named enumerator"). With `: int`, any
//  value representable as int is a valid (if unnamed) Rarity, so the default arm
//  of a switch is reachable *and* well-defined — which is exactly what the
//  rarityLabel/rarityMultiplier "Unknown / safe fallback" tests exercise.
//
enum class Rarity : int
{
    Common,   // multiplier ×1  — everyday drops
    Rare,     // multiplier ×3  — uncommon finds
    Epic,     // multiplier ×10 — legendary gear
};

// ─── struct Item ─────────────────────────────────────────────────────────────
//
//  AGGREGATE (notes 13.7–13.9): a DATA-ONLY container with four named members.
//  Each member gets a DEFAULT INITIALIZER (notes 13.9) so a default-constructed
//  Item is in a safe, known state:
//
//    Item empty {};           // name="", rarity=Common, quantity=0, unitValue=0
//    Item sword { "Sword", Rarity::Rare, 5, 40 };  // fully specified
//
//  Member order is part of the aggregate API — callers rely on it.
//
struct Item
{
    std::string name     {};          // display name of the item (owned string)
    Rarity      rarity   { Rarity::Common };  // how rare is it?
    int         quantity { 0 };       // units in stock
    int         unitValue{ 0 };       // base value in coins per unit
};

// ─── Free-function API (notes 13.10 — passing/returning structs) ──────────────
//
//  These functions OPERATE ON Items but are NOT members of the struct.
//  That boundary — data here, functions there — is the Chapter 13 lesson.
//  Chapter 14 introduces member functions; resist the urge to move things there.

// ── rarityLabel ──────────────────────────────────────────────────────────────
// Returns a human-readable string for a Rarity value.
//   Common → "Common",  Rare → "Rare",  Epic → "Epic",  anything else → "Unknown"
// Implement with a SWITCH on the scoped enum (notes 13.4 / 13.6).
// Pass the enum BY VALUE (it's just an integer under the hood; no cost).
std::string_view rarityLabel(Rarity rarity);

// ── rarityMultiplier ─────────────────────────────────────────────────────────
// Returns the integer value multiplier for a Rarity:
//   Common → 1,  Rare → 3,  Epic → 10,  anything else → 1 (safe default).
int rarityMultiplier(Rarity rarity);

// ── itemWorth ────────────────────────────────────────────────────────────────
// Total coin value of the ENTIRE STOCK of this item:
//   item.quantity  ×  item.unitValue  ×  rarityMultiplier(item.rarity)
//
// PASS BY CONST REFERENCE (notes 13.10): the struct is read-only and we avoid
// copying the std::string member. This is the canonical pattern for read-only
// struct parameters. Return by VALUE (a computed int — nothing to alias).
int itemWorth(const Item& item);

// ── makeItem ─────────────────────────────────────────────────────────────────
// Factory helper: constructs and RETURNS an Item by value (notes 13.10).
//   makeItem("Potion", Rarity::Common, 10, 5)
//     => Item{ "Potion", Rarity::Common, 10, 5 }
// Modern C++ performs copy elision/NRVO, so this is cheap.
// Returning a struct by value is cleaner than multiple out-parameters.
Item makeItem(std::string_view name, Rarity rarity, int quantity, int unitValue);

// ── restock ──────────────────────────────────────────────────────────────────
// Add `amount` units to item.quantity. Mutates the struct THROUGH a non-const
// reference (notes 13.10, notes 13.12): the caller's object is modified in place.
// If `amount` is negative it should be treated as 0 (clamp to zero before adding).
void restock(Item& item, int amount);

// ── isSameItem ───────────────────────────────────────────────────────────────
// Returns true if a and b represent the SAME catalogue entry: same name AND same
// rarity. Does NOT compare quantity/unitValue (those can differ across locations).
// Both passed by const reference — read-only, no copy (notes 13.10, 13.12).
bool isSameItem(const Item& a, const Item& b);

#endif // LEDGER_H
