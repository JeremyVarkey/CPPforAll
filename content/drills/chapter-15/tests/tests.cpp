// Chapter 15 — More on Classes · Project: IdCard Badge Printer   (GRADER)
// ─────────────────────────────────────────────────────────────────────────────
// Tiny no-framework unit-test harness matching the drills/CLAUDE.md spec.
// Includes ../idcard.h (the contract) and calls the API across many inputs.
// The Makefile links this against starter/idcard.cpp (your code) for `make test`
// and against solution/idcard.cpp for `make test-solution`.
//
// Each CHECK that fails prints its expression and source line. Any failure causes
// a non-zero exit, so `make test` turns RED until every check passes.
//
// WHAT IS BEING TESTED
//
//   Task 1 — Constructor: serial auto-assignment, s_liveCount increments.
//   Task 2 — Destructor: s_liveCount decrements when objects leave scope.
//            The key trick: we create cards in nested { } scopes and verify
//            the count falls at the exact point the scope closes. No heap,
//            no `new` — just automatic variables with deterministic lifetimes
//            (notes 15.4: "scope-based destruction").
//   Task 3 — setOwner/setTitle/setRole: correct mutation + chaining identity.
//            The chaining test takes the ADDRESS of the returned reference and
//            verifies it equals the address of the original card — same object,
//            not a copy (notes 15.1: "return *this by reference").
//   Task 4 — Const accessors: values roundtrip correctly; called on const refs.
//   Task 5 — liveCount() / nextSerial(): static functions, no object needed.
// ─────────────────────────────────────────────────────────────────────────────

#include <iostream>
#include "../idcard.h"

static int fails = 0;

// CHECK: assert a condition; on failure, report expression + line number.
#define CHECK(cond) \
    do { if(!(cond)){ std::cerr << "FAIL: " #cond "  @line " << __LINE__ << "\n"; ++fails; } } while(0)

int main()
{
    // ──────────────────────────────────────────────────────────────────────────
    // Task 5 sanity: static functions work without any object existing.
    // s_liveCount starts at 0; s_nextSerial starts at 1.
    // ──────────────────────────────────────────────────────────────────────────
    CHECK(IdCard::liveCount()  == 0);   // no objects yet
    CHECK(IdCard::nextSerial() == 1);   // first card will be serial 1

    // ──────────────────────────────────────────────────────────────────────────
    // Task 1 + 5: constructor increments live count; serial is auto-assigned.
    // ──────────────────────────────────────────────────────────────────────────
    {
        IdCard a { "Ada Lovelace", "Chief Analyst" };

        CHECK(IdCard::liveCount()  == 1);   // one card alive
        CHECK(IdCard::nextSerial() == 2);   // next card will be serial 2
        CHECK(a.serial() == 1);             // first card gets serial 1

        {
            IdCard b { "Charles Babbage", "Engine Designer" };

            CHECK(IdCard::liveCount()  == 2);
            CHECK(IdCard::nextSerial() == 3);
            CHECK(b.serial() == 2);         // second card gets serial 2

            // ── Task 2: destructor test — nested scope ────────────────────────
            // When b leaves THIS inner scope its destructor runs, decrementing
            // s_liveCount. After the closing brace, a is still alive (count = 1).
        }

        CHECK(IdCard::liveCount() == 1);    // b is gone; only a survives

        // Third card inside a's scope: gets serial 3.
        {
            IdCard c { "Grace Hopper", "Compiler Pioneer" };
            CHECK(c.serial() == 3);
            CHECK(IdCard::liveCount() == 2);
        }
        CHECK(IdCard::liveCount() == 1);    // c gone, a still alive
    }
    CHECK(IdCard::liveCount() == 0);        // a gone — count back to zero

    // ──────────────────────────────────────────────────────────────────────────
    // Task 4: default-argument constructor ("Unnamed" / "Untitled").
    // ──────────────────────────────────────────────────────────────────────────
    {
        IdCard def {};                           // uses both defaults
        CHECK(def.owner() == "Unnamed");
        CHECK(def.title() == "Untitled");
        CHECK(def.role()  == IdCard::Role::Other);
    }
    CHECK(IdCard::liveCount() == 0);

    // ──────────────────────────────────────────────────────────────────────────
    // Task 4: const accessor roundtrip — called on a const reference.
    // ──────────────────────────────────────────────────────────────────────────
    {
        IdCard card { "Linus Torvalds", "Kernel Hacker" };
        const IdCard& cref { card };        // const reference — only const methods OK

        CHECK(cref.owner()  == "Linus Torvalds");
        CHECK(cref.title()  == "Kernel Hacker");
        CHECK(cref.role()   == IdCard::Role::Other);
        CHECK(cref.serial() >= 1);          // some valid serial was assigned
    }

    // ──────────────────────────────────────────────────────────────────────────
    // Task 3: setOwner / setTitle — mutation correctness.
    // ──────────────────────────────────────────────────────────────────────────
    {
        IdCard card { "Original Name", "Original Title" };
        card.setOwner("Updated Name");
        card.setTitle("Updated Title");

        CHECK(card.owner() == "Updated Name");
        CHECK(card.title() == "Updated Title");

        card.setRole(IdCard::Role::Engineer);
        CHECK(card.role() == IdCard::Role::Engineer);

        card.setRole(IdCard::Role::Manager);
        CHECK(card.role() == IdCard::Role::Manager);
    }

    // ──────────────────────────────────────────────────────────────────────────
    // Task 3: method chaining — the chained calls must mutate the SAME object.
    //
    // This is the CRITICAL chaining test: we capture the address of the object
    // returned by setOwner and verify it equals &card. If the method returned a
    // COPY instead of a reference, the addresses would differ and this check fails.
    // Then we chain all three setters and confirm the final state is correct.
    // ──────────────────────────────────────────────────────────────────────────
    {
        IdCard card { "Wrong Name", "Wrong Title" };

        // Verify setOwner returns a reference to the SAME object (not a copy).
        IdCard& returned = card.setOwner("Right Name");
        CHECK(&returned == &card);          // same address = same object

        // Full chained call: all three setters in one expression.
        card.setOwner("Alan Turing")
            .setTitle("Cryptanalyst")
            .setRole(IdCard::Role::Engineer);

        CHECK(card.owner() == "Alan Turing");
        CHECK(card.title() == "Cryptanalyst");
        CHECK(card.role()  == IdCard::Role::Engineer);
    }

    // ──────────────────────────────────────────────────────────────────────────
    // Task 3 + 1: chaining from construction — set fields right after creation.
    // ──────────────────────────────────────────────────────────────────────────
    {
        int serialBefore = IdCard::nextSerial();
        IdCard card { "Temp", "Temp" };
        card.setOwner("Margaret Hamilton").setTitle("Software Engineer");

        CHECK(card.owner()  == "Margaret Hamilton");
        CHECK(card.title()  == "Software Engineer");
        CHECK(card.serial() == serialBefore);   // serial assigned at construction
    }

    // ──────────────────────────────────────────────────────────────────────────
    // Task 5: static functions accessible without an object.
    // ──────────────────────────────────────────────────────────────────────────
    {
        int countBefore  = IdCard::liveCount();
        int serialBefore = IdCard::nextSerial();

        {
            IdCard x { "X", "X" };
            IdCard y { "Y", "Y" };

            CHECK(IdCard::liveCount()  == countBefore + 2);
            CHECK(IdCard::nextSerial() == serialBefore + 2);
            CHECK(x.serial() == serialBefore);
            CHECK(y.serial() == serialBefore + 1);
        }

        CHECK(IdCard::liveCount()  == countBefore);   // both x and y destroyed
        CHECK(IdCard::nextSerial() == serialBefore + 2); // serial dispenser never resets
    }

    // ──────────────────────────────────────────────────────────────────────────
    // Edge case: deeply nested scopes — count tracks every level.
    // ──────────────────────────────────────────────────────────────────────────
    {
        CHECK(IdCard::liveCount() == 0);
        {
            IdCard a { "A", "A" };
            CHECK(IdCard::liveCount() == 1);
            {
                IdCard b { "B", "B" };
                CHECK(IdCard::liveCount() == 2);
                {
                    IdCard c { "C", "C" };
                    CHECK(IdCard::liveCount() == 3);
                }                           // c destroyed here
                CHECK(IdCard::liveCount() == 2);
            }                               // b destroyed here
            CHECK(IdCard::liveCount() == 1);
        }                                   // a destroyed here
        CHECK(IdCard::liveCount() == 0);
    }

    // ──────────────────────────────────────────────────────────────────────────
    // Edge case: nested type is scoped — IdCard::Role::Intern, not just Role::Intern.
    // ──────────────────────────────────────────────────────────────────────────
    {
        IdCard intern { "Sam", "Intern" };
        intern.setRole(IdCard::Role::Intern);
        CHECK(intern.role() == IdCard::Role::Intern);

        intern.setRole(IdCard::Role::Other);
        CHECK(intern.role() == IdCard::Role::Other);
    }

    // ──────────────────────────────────────────────────────────────────────────
    // Final liveCount must be 0 — all scopes above are closed.
    // ──────────────────────────────────────────────────────────────────────────
    CHECK(IdCard::liveCount() == 0);

    // ── Result ────────────────────────────────────────────────────────────────
    if (!fails)
        std::cout << "PASS ✅  all badge-printer checks passed.\n";
    else
        std::cerr << "\nFAIL ❌  " << fails
                  << " check(s) failed — fix the TASK blocks in starter/idcard.cpp.\n";

    return fails ? 1 : 0;
}
