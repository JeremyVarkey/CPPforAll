// Chapter 25 — Virtual Functions · MINI-LLVM: inst.cpp   (REFERENCE SOLUTION)
// ─────────────────────────────────────────────────────────────────────────────
// Complete, correct, warning-clean implementation of ../inst.h.
// Peek only after taking a real swing at starter/inst.cpp — the learning is in
// writing the virtual dispatch yourself, then comparing.
//
// Design summary: every task here exercises ONE new concept from Ch 25:
//   Task 1 — virtual destructor (notes 25.4)
//   Task 2 — virtual/pure-virtual override (notes 25.2, 25.7)
//   Task 3 — derived destructor counter (proves vtable-through-base-ptr)
//   Task 4 — free-function polymorphic dispatch (notes 25.2) ← Ch24 cliff resolved
//   Task 5 — dynamic_cast pointer form (notes 25.10)
//   Task 6 — polymorphic vector walk; unique_ptr avoids slicing (notes 25.9)
// ─────────────────────────────────────────────────────────────────────────────

#include "../inst.h"
#include <string>

// ── Provided: static counter definition ─────────────────────────────────────
// Every translation unit that includes inst.h sees the DECLARATION of
// s_destroyed; this is the one-and-only DEFINITION (Ch 7 linkage rule).
int Inst::s_destroyed = 0;

// ─── TASK 1: Virtual base-class destructor ───────────────────────────────────
// The body is intentionally empty — Inst carries no raw resources.  What
// matters is that it EXISTS and is VIRTUAL (already declared 'virtual' in the
// header).
//
// MECHANISM: When you write  'delete base_ptr'  (or a unique_ptr<Inst> goes
// out of scope), the compiler does NOT call Inst::~Inst() directly. Instead,
// it follows the vtable pointer stored in the object, which leads to the
// DERIVED destructor (AddInst::~AddInst, etc.).  That derived destructor runs,
// then chains up to Inst::~Inst() automatically — exactly as intended.
//
// Without 'virtual' on ~Inst(), the vtable lookup never happens: the compiler
// calls Inst::~Inst() directly, skipping the derived body entirely.  That is
// the root cause of the classic "virtual destructor" bug.  (notes 25.4)
Inst::~Inst() = default; // empty body; let the compiler generate it cleanly

// ─── TASK 2: opcodeName() overrides ─────────────────────────────────────────
// Each concrete class returns its own opcode string.  The 'override' specifier
// in the header (and here, via the out-of-class definition) silently verifies
// the signature matches the base virtual — any typo (missing const, different
// return type) becomes a compile error rather than a silent shadow function.
// (notes 25.3)

std::string AddInst::opcodeName() const
{
    return "add";  // real LLVM: Instruction::getOpcodeName() on a BinaryOperator
}

std::string LoadInst::opcodeName() const
{
    return "load"; // real LLVM: llvm::LoadInst → getOpcodeName() = "load"
}

// BranchInst::opcodeName is marked 'final' in the header: no class that
// inherits from BranchInst can override opcodeName again — the hierarchy stops
// here.  Use 'final' as a design signal: "this version is definitive." (25.3)
std::string BranchInst::opcodeName() const
{
    return "br";   // real LLVM: llvm::BranchInst → getOpcodeName() = "br"
}

// ─── TASK 3: Derived destructors — the vtable-through-base-ptr proof ─────────
// Each body does ONE thing: ++Inst::s_destroyed.
//
// When the grader holds a std::unique_ptr<Inst> pointing at (say) an AddInst
// and lets it go out of scope, the sequence is:
//
//   1. unique_ptr calls  delete ptr   through an Inst*.
//   2. Virtual dispatch: vtable points at AddInst::~AddInst().
//   3. AddInst::~AddInst() runs -> s_destroyed++.
//   4. Base destructor chain: Inst::~Inst() runs automatically.
//
// If ~Inst() were NOT virtual, step 2 would be skipped and only Inst::~Inst()
// would run — s_destroyed would stay 0 and the grader would fail TASK 3.
// That is exactly why the check is here: to OBSERVE the virtual dtor, not just
// declare it and hope. (notes 25.4)

AddInst::~AddInst()
{
    ++Inst::s_destroyed;
}

LoadInst::~LoadInst()
{
    ++Inst::s_destroyed;
}

BranchInst::~BranchInst()
{
    ++Inst::s_destroyed;
}

// ─── TASK 4: describe() — runtime polymorphism through a base reference ───────
// This is the Chapter 24 cliffhanger RESOLVED.
//
// In Ch 24, functions that accept a Base& and call a non-virtual method always
// invoke Base's version — no matter what the actual object is.  The programmer
// expected polymorphic behavior but got static binding.
//
// Adding 'virtual' to opcodeName() fixes this: the call  inst.opcodeName()
// below dispatches through the vtable to whichever concrete class 'inst' is.
// describe() is written ONCE and works for AddInst, LoadInst, BranchInst, and
// any future derived class.  This is RUNTIME POLYMORPHISM. (notes 25.2)
std::string describe(const Inst& inst)
{
    // inst.opcodeName() — virtual late binding.
    // Even though 'inst' has static type 'const Inst&', the vtable pointer
    // stored in the actual object routes this call to the derived override.
    return "inst: " + inst.opcodeName();
}

// ─── TASK 5: asBranch() — LLVM-style safe downcast ──────────────────────────
// dynamic_cast<T*>(ptr) performs a RUNTIME type check:
//   • If the object is-a BranchInst (or a subtype of it) -> return T* (valid).
//   • Otherwise -> return nullptr.
//
// Always use the POINTER form when you want nullptr on failure; the REFERENCE
// form throws std::bad_cast instead — appropriate if failure is truly
// unexpected, but harder to handle in a general helper.  (notes 25.10)
//
// In real LLVM you write:
//     if (auto *BI = dyn_cast<BranchInst>(&I)) { use BI; }
// which is exactly this function's pattern.  (llvm-idioms.md)
//
// Note: dynamic_cast requires a POLYMORPHIC type (one with at least one
// virtual function) — Inst qualifies because of opcodeName() and ~Inst().
BranchInst* asBranch(Inst* inst)
{
    return dynamic_cast<BranchInst*>(inst); // returns nullptr on mismatch
}

// ─── TASK 6: countOpcode() — walk a polymorphic instruction vector ───────────
// Iterate over the vector of unique_ptr<Inst>.  Dereference each smart pointer
// to get a const Inst& and call opcodeName() — virtual dispatch picks the right
// concrete opcode string.  Count how many match the requested opcode.
//
// WHY unique_ptr<Inst> instead of std::vector<Inst>?
//   Storing by value would SLICE: when you push_back(AddInst{}) into a
//   vector<Inst>, only the Inst sub-object is copied; the AddInst-specific
//   vtable pointer and any AddInst data are left behind.  The object in the
//   vector becomes a plain Inst with no opcodeName override.  Using smart
//   pointers (owning pointers) avoids slicing: the full derived object lives on
//   the heap, and the pointer just references it.  (notes 25.9)
//
// In real LLVM:  for (Instruction &I : BB) { if (I.getOpcodeName() == opcode) ++n; }
// (llvm-idioms.md "Walk every Instruction in a BasicBlock")
int countOpcode(const std::vector<std::unique_ptr<Inst>>& module,
                std::string_view opcode)
{
    int count { 0 };
    for (const auto& ptr : module)   // ptr is const unique_ptr<Inst>&
    {
        // *ptr dereferences to const Inst& — virtual dispatch on opcodeName()
        if (ptr->opcodeName() == opcode)
            ++count;
    }
    return count;
}
