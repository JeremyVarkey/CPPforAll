// Chapter 13 — Compound Types: Enums & Structs · Inventory Ledger   (GRADER)
// ─────────────────────────────────────────────────────────────────────────────
// A tiny no-framework unit-test harness (same style as drills/CLAUDE.md spec).
// It includes ../ledger.h and calls each library function across many inputs.
// CHECK fails loudly with the expression and line number. Any failure ->
// non-zero exit -> `make test` is RED.
//
// The Makefile links this file against starter/ledger.cpp (your code) for
// `make test`, and against solution/ledger.cpp for `make test-solution`.
//
// Testing strategy:
//   • rarityLabel / rarityMultiplier: round-trip enum values; test default arm.
//   • itemWorth: verify formula across all three rarities; test zero-quantity edge.
//   • makeItem: struct returned by value; check every field.
//   • restock: mutation through reference; clamp negative amounts.
//   • isSameItem: same/different name; same/different rarity; field independence.

#include <iostream>
#include "../ledger.h"

static int fails = 0;

// CHECK: assert a boolean condition; on failure print expression + line number.
#define CHECK(cond) \
    do { if(!(cond)){ std::cerr << "FAIL: " #cond "  @line " << __LINE__ << "\n"; ++fails; } } while(0)

int main()
{
    // ── Task 1: rarityLabel — switch on a scoped enum ────────────────────────
    // Normal round-trip: every named enumerator must map to its label string.
    CHECK(rarityLabel(Rarity::Common) == "Common");
    CHECK(rarityLabel(Rarity::Rare)   == "Rare");
    CHECK(rarityLabel(Rarity::Epic)   == "Epic");
    // Edge: a cast-forged value that matches no named enumerator must hit default.
    CHECK(rarityLabel(static_cast<Rarity>(99)) == "Unknown");   // default arm

    // ── Task 2: rarityMultiplier — switch returning an int ───────────────────
    CHECK(rarityMultiplier(Rarity::Common) == 1);
    CHECK(rarityMultiplier(Rarity::Rare)   == 3);
    CHECK(rarityMultiplier(Rarity::Epic)   == 10);
    // Edge: out-of-range cast value must return the safe default of 1.
    CHECK(rarityMultiplier(static_cast<Rarity>(99)) == 1);

    // ── Task 3: itemWorth — formula + const-ref parameter ───────────────────
    // Common: 5 × 10 × 1 = 50
    Item potion { "Potion", Rarity::Common, 5, 10 };
    CHECK(itemWorth(potion) == 50);

    // Rare: 2 × 40 × 3 = 240
    Item sword { "Sword", Rarity::Rare, 2, 40 };
    CHECK(itemWorth(sword) == 240);

    // Epic: 1 × 100 × 10 = 1000
    Item artifact { "Artifact", Rarity::Epic, 1, 100 };
    CHECK(itemWorth(artifact) == 1000);

    // Edge: zero-quantity item is worth 0 regardless of rarity or unitValue.
    Item empty { "Ghost", Rarity::Epic, 0, 999 };
    CHECK(itemWorth(empty) == 0);           // 0 × 999 × 10 == 0

    // Edge: zero unitValue item is worth 0 regardless of quantity or rarity.
    Item junk { "Pebble", Rarity::Rare, 100, 0 };
    CHECK(itemWorth(junk) == 0);            // 100 × 0 × 3 == 0

    // Verify itemWorth does NOT modify the struct (const ref guarantee).
    CHECK(potion.quantity  == 5);           // must still be 5 after the call above
    CHECK(potion.unitValue == 10);

    // ── Task 4: makeItem — return struct by value ────────────────────────────
    Item ring { makeItem("Ring", Rarity::Rare, 3, 25) };
    CHECK(ring.name      == "Ring");
    CHECK(ring.rarity    == Rarity::Rare);
    CHECK(ring.quantity  == 3);
    CHECK(ring.unitValue == 25);

    // Edge: makeItem with zero quantity and Common rarity.
    Item coin { makeItem("Coin", Rarity::Common, 0, 1) };
    CHECK(coin.name      == "Coin");
    CHECK(coin.rarity    == Rarity::Common);
    CHECK(coin.quantity  == 0);
    CHECK(coin.unitValue == 1);

    // makeItem round-trip through itemWorth: 3 × 25 × 3 = 225
    CHECK(itemWorth(ring) == 225);

    // ── Task 5: restock — mutate through a non-const reference ──────────────
    Item chest { "Chest", Rarity::Common, 4, 20 };
    restock(chest, 6);
    CHECK(chest.quantity == 10);            // 4 + 6 = 10

    restock(chest, 0);
    CHECK(chest.quantity == 10);            // adding 0 changes nothing

    restock(chest, -5);
    CHECK(chest.quantity == 10);            // Edge: negative amount → clamped to 0; no change

    restock(chest, 1);
    CHECK(chest.quantity == 11);            // normal positive restock after clamp

    // Confirm restock does NOT affect other members.
    CHECK(chest.name      == "Chest");
    CHECK(chest.rarity    == Rarity::Common);
    CHECK(chest.unitValue == 20);

    // ── Task 6: isSameItem — catalogue identity by name + rarity ────────────
    Item a { makeItem("Shield", Rarity::Rare, 1, 50) };
    Item b { makeItem("Shield", Rarity::Rare, 9, 99) }; // different qty/value
    Item c { makeItem("Shield", Rarity::Common,  1, 50) }; // same name, diff rarity
    Item d { makeItem("Armor",  Rarity::Rare,    1, 50) }; // same rarity, diff name

    // Same name AND same rarity → same catalogue entry (regardless of stock).
    CHECK(isSameItem(a, b) == true);

    // Same name but DIFFERENT rarity → different items.
    CHECK(isSameItem(a, c) == false);

    // Different name but same rarity → different items.
    CHECK(isSameItem(a, d) == false);

    // Reflexive: an item is always the same as itself.
    CHECK(isSameItem(a, a) == true);

    // Edge: two default-constructed (empty) Items share the same identity.
    Item x {};
    Item y {};
    CHECK(isSameItem(x, y) == true);        // both "", Rarity::Common

    // ── Result ───────────────────────────────────────────────────────────────
    if (!fails)
        std::cout << "PASS \xE2\x9C\x85  all ledger checks passed.\n";
    else
        std::cerr << "\nFAIL \xE2\x9D\x8C  " << fails << " check(s) failed"
                  << " — fix the TASK blocks in starter/ledger.cpp.\n";

    return fails ? 1 : 0;
}
