// Chapter 25 — Virtual Functions · MINI-LLVM   (GRADER)
// ─────────────────────────────────────────────────────────────────────────────
// Tiny no-framework unit-test harness (same style as drills/CLAUDE.md).
// Includes ../inst.h and calls the API across many deterministic inputs.
// Each failing CHECK prints its expression and line number.
// Any failure -> non-zero exit -> `make test` is RED.
//
// The Makefile links this file against starter/inst.cpp (`make test`) or
// solution/inst.cpp (`make test-solution`).
// ─────────────────────────────────────────────────────────────────────────────

#include <iostream>
#include <memory>
#include <string>
#include "../inst.h"

static int fails = 0;

// CHECK: assert a boolean condition; on failure, report what and where.
#define CHECK(cond) \
    do { if(!(cond)){ std::cerr << "FAIL: " #cond "  @line " << __LINE__ << "\n"; ++fails; } } while(0)

int main()
{
    // ─────────────────────────────────────────────────────────────────────────
    // TASK 2: opcodeName() — virtual dispatch returns the correct string
    // ─────────────────────────────────────────────────────────────────────────
    // First, call directly on concrete objects (static binding — baseline).
    {
        AddInst    add {};
        LoadInst   load {};
        BranchInst br {};

        CHECK(add.opcodeName()  == "add");
        CHECK(load.opcodeName() == "load");
        CHECK(br.opcodeName()   == "br");
    }

    // Now call through a BASE POINTER — the whole point of virtual dispatch.
    // Even though the static type of 'p' is Inst*, the vtable must route each
    // call to the most-derived override.  (notes 25.2)
    {
        Inst* p1 = new AddInst {};
        Inst* p2 = new LoadInst {};
        Inst* p3 = new BranchInst {};

        CHECK(p1->opcodeName() == "add");   // Inst* -> AddInst override
        CHECK(p2->opcodeName() == "load");  // Inst* -> LoadInst override
        CHECK(p3->opcodeName() == "br");    // Inst* -> BranchInst override

        // Clean up via base pointer — requires virtual destructor (TASK 1/3).
        // If ~Inst() is not virtual these deletes only call Inst::~Inst() and
        // s_destroyed stays 0.
        delete p1;
        delete p2;
        delete p3;
    }

    // ─────────────────────────────────────────────────────────────────────────
    // TASK 1 + 3: Virtual destructor proof
    // Delete through a BASE POINTER and verify the DERIVED dtor ran.
    //
    // We use unique_ptr<Inst> here (Ch 22 smart pointer — in scope, provided as
    // scaffolding) which calls  delete base_ptr  exactly as raw new/delete would,
    // but safely.  unique_ptr is NOT ahead of scope: Ch 22 <= Ch 25.
    // ─────────────────────────────────────────────────────────────────────────
    Inst::s_destroyed = 0;   // reset before the proof sequence
    {
        // NOTE the STATIC type: std::unique_ptr<Inst>, not <AddInst>.  The held
        // object is an AddInst, but the unique_ptr's deleter calls
        //   delete (Inst*)raw
        // i.e. delete through a BASE pointer.  Only a VIRTUAL ~Inst() routes that
        // to ~AddInst().  (If we wrote unique_ptr<AddInst>, the static type would
        // already be AddInst and the test would pass even with a non-virtual base
        // dtor — proving nothing.  The base-pointer type is what makes this a real
        // proof.)
        std::unique_ptr<Inst> p = std::make_unique<AddInst>();
        CHECK(Inst::s_destroyed == 0);  // dtor hasn't run yet
    } // unique_ptr<Inst> out of scope -> delete Inst* -> vtable -> ~AddInst() -> ++s_destroyed
    CHECK(Inst::s_destroyed == 1);  // derived dtor MUST have fired through the base ptr

    {
        std::unique_ptr<Inst> p = std::make_unique<LoadInst>();
    }
    CHECK(Inst::s_destroyed == 2);  // second derived dtor, also via unique_ptr<Inst>

    {
        std::unique_ptr<Inst> p = std::make_unique<BranchInst>();
    }
    CHECK(Inst::s_destroyed == 3);  // third derived dtor, also via unique_ptr<Inst>

    // Edge: delete nullptr through a base pointer — should be a no-op.
    {
        Inst* null_ptr = nullptr;
        delete null_ptr; // well-defined; should NOT increment s_destroyed
    }
    CHECK(Inst::s_destroyed == 3);  // unchanged after deleting nullptr

    // ─────────────────────────────────────────────────────────────────────────
    // TASK 4: describe() — polymorphic free function (Ch24 cliff resolved)
    // ─────────────────────────────────────────────────────────────────────────
    {
        AddInst    add {};
        LoadInst   load {};
        BranchInst br {};

        // Call through concrete objects directly.
        CHECK(describe(add)  == "inst: add");
        CHECK(describe(load) == "inst: load");
        CHECK(describe(br)   == "inst: br");

        // Call through a BASE REFERENCE — this is where virtual matters.
        // Static type: const Inst&; dynamic type: AddInst/LoadInst/BranchInst.
        Inst& ref_add  = add;
        Inst& ref_load = load;
        Inst& ref_br   = br;

        CHECK(describe(ref_add)  == "inst: add");
        CHECK(describe(ref_load) == "inst: load");
        CHECK(describe(ref_br)   == "inst: br");
    }

    // ─────────────────────────────────────────────────────────────────────────
    // TASK 5: asBranch() — dynamic_cast pointer form
    // ─────────────────────────────────────────────────────────────────────────
    {
        AddInst    add {};
        LoadInst   load {};
        BranchInst br {};

        // Non-branch instructions must yield nullptr — no crash, just null.
        CHECK(asBranch(&add)  == nullptr);
        CHECK(asBranch(&load) == nullptr);

        // A BranchInst must succeed and return a valid pointer.
        BranchInst* result = asBranch(&br);
        CHECK(result != nullptr);
        // Verify we got the right object back (pointer identity).
        CHECK(result == &br);
        // Exercise BranchInst-specific API through the downcast pointer.
        CHECK(result->isUnconditional() == true);

        // Edge: nullptr input must not crash; return nullptr.
        CHECK(asBranch(nullptr) == nullptr);

        // Through a base pointer (the typical real-world pattern).
        Inst* base_ptr_br  = &br;
        Inst* base_ptr_add = &add;
        CHECK(asBranch(base_ptr_br)  != nullptr);   // is a BranchInst
        CHECK(asBranch(base_ptr_add) == nullptr);   // is NOT a BranchInst
    }

    // ─────────────────────────────────────────────────────────────────────────
    // TASK 6: countOpcode() — walk a polymorphic instruction vector
    // ─────────────────────────────────────────────────────────────────────────
    {
        std::vector<std::unique_ptr<Inst>> module {};
        module.push_back(std::make_unique<AddInst>());
        module.push_back(std::make_unique<AddInst>());
        module.push_back(std::make_unique<LoadInst>());
        module.push_back(std::make_unique<BranchInst>());

        CHECK(countOpcode(module, "add")  == 2);
        CHECK(countOpcode(module, "load") == 1);
        CHECK(countOpcode(module, "br")   == 1);
        CHECK(countOpcode(module, "mul")  == 0);   // not present -> 0

        // Edge: empty module.
        std::vector<std::unique_ptr<Inst>> empty {};
        CHECK(countOpcode(empty, "add") == 0);

        // Edge: all same opcode.
        std::vector<std::unique_ptr<Inst>> all_adds {};
        all_adds.push_back(std::make_unique<AddInst>());
        all_adds.push_back(std::make_unique<AddInst>());
        all_adds.push_back(std::make_unique<AddInst>());
        CHECK(countOpcode(all_adds, "add")  == 3);
        CHECK(countOpcode(all_adds, "load") == 0);

        // Edge: single instruction.
        std::vector<std::unique_ptr<Inst>> singleton {};
        singleton.push_back(std::make_unique<BranchInst>());
        CHECK(countOpcode(singleton, "br") == 1);
        CHECK(countOpcode(singleton, "add") == 0);
    }

    // ─────────────────────────────────────────────────────────────────────────
    // OBJECT SLICING DEMO — no graded task; demonstrates WHY slicing would
    // break everything and why the vector stores pointers, not values.
    //
    // We cannot put an abstract-base Inst by value in a vector<Inst> because
    // Inst has pure virtual functions — the compiler would reject it outright.
    // The section in the README shows slicing with a CONCRETE pair (notes 25.9)
    // so the learner can SEE the behavior without UB.
    // ─────────────────────────────────────────────────────────────────────────

    // ─────────────────────────────────────────────────────────────────────────
    // Integrated dispatch round-trip: combine describe + countOpcode
    // ─────────────────────────────────────────────────────────────────────────
    {
        // Build a mini "basic block": two adds, one load, one branch.
        std::vector<std::unique_ptr<Inst>> bb {};
        bb.push_back(std::make_unique<AddInst>());
        bb.push_back(std::make_unique<LoadInst>());
        bb.push_back(std::make_unique<AddInst>());
        bb.push_back(std::make_unique<BranchInst>());

        // describe() must work through the unique_ptr's operator->
        CHECK(describe(*bb[0]) == "inst: add");
        CHECK(describe(*bb[1]) == "inst: load");
        CHECK(describe(*bb[2]) == "inst: add");
        CHECK(describe(*bb[3]) == "inst: br");

        // countOpcode round-trip.
        CHECK(countOpcode(bb, "add")  == 2);
        CHECK(countOpcode(bb, "load") == 1);
        CHECK(countOpcode(bb, "br")   == 1);

        // asBranch must succeed for the branch and fail for others.
        CHECK(asBranch(bb[0].get()) == nullptr);  // AddInst
        CHECK(asBranch(bb[3].get()) != nullptr);  // BranchInst
    }

    // ─────────────────────────────────────────────────────────────────────────
    if (!fails)
        std::cout << "PASS \xE2\x9C\x85  all MINI-LLVM checks passed.\n";
    else
        std::cerr << "\nFAIL \xE2\x9D\x8C  " << fails
                  << " check(s) failed \xe2\x80\x94 fix the TASK blocks in starter/inst.cpp.\n";

    return fails ? 1 : 0;
}
