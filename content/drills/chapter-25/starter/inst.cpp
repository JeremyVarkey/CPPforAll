// Chapter 25 — Virtual Functions · MINI-LLVM: inst.cpp   (STARTER)
// ─────────────────────────────────────────────────────────────────────────────
// Fill in the six TASK blocks below.  Each maps 1:1 to a task in the README
// and to a declaration in ../inst.h.  The bodies currently return WRONG
// PLACEHOLDERS so the file compiles immediately — that is why `make test` is
// RED right now.  Your job is to turn it GREEN.
//
//     make build            compile your code (should already work)
//     make test             grade it          (RED until you fill these in)
//     make solution         run the grader against the reference if stuck
//
// KEY VOCABULARY (notes 25.2, 25.3, 25.7):
//   virtual    — enables dynamic dispatch through a base pointer/reference
//   override   — verifies the function actually overrides a base virtual
//   final      — prevents further overriding (on a function) or inheritance
//   = 0        — pure virtual; makes the class ABSTRACT (non-instantiable)
//   virtual ~  — virtual destructor; required when deleting through base ptr
// ─────────────────────────────────────────────────────────────────────────────

#include "../inst.h"
#include <string>

// ── Provided: definition of the static counter (do not change this) ──────────
// This counter lets the grader verify that deleting through a base pointer
// really calls the DERIVED destructor.  Each derived ~Dtor should ++s_destroyed.
int Inst::s_destroyed = 0;

// ─────────────────────────────────────────────────────────────────────────────
// TASK 1: Virtual base-class destructor
// ─────────────────────────────────────────────────────────────────────────────
// Define the body of Inst::~Inst().  It does not need to increment s_destroyed
// (the DERIVED destructors do that in TASK 3).  The body may be empty; what
// matters is that it EXISTS and is VIRTUAL (already declared virtual in inst.h).
//
// WHY: If ~Inst() were not virtual, 'delete base_ptr' would skip the derived
// destructor, leaving derived-only resources leaked and s_destroyed un-bumped.
// The virtual destructor is the FIRST thing to add to any polymorphic base.
// (notes 25.4)
//
//   >>> YOUR CODE HERE <<<
//
Inst::~Inst()
{
    // placeholder — body intentionally empty (correct behavior: do nothing extra)
}
// ─────────────────────────────────────────────────────────────────────────────


// ─────────────────────────────────────────────────────────────────────────────
// TASK 2: Override opcodeName() in each derived class
// ─────────────────────────────────────────────────────────────────────────────
// Each derived class must return its own opcode string so that virtual dispatch
// through an Inst& or Inst* picks the RIGHT concrete name at runtime.
//
// AddInst::opcodeName() must return "add"    (notes 25.2 — opcode dispatch)
// LoadInst::opcodeName() must return "load"
// BranchInst::opcodeName() must return "br"
//
// The 'override' specifier (already on the declarations in inst.h) makes a
// signature mismatch a compile error — your best friend against typos. (25.3)
//
//   >>> YOUR CODE HERE <<<
//
std::string AddInst::opcodeName() const
{
    return "???";   // placeholder — wrong; replace with the real opcode string
}

std::string LoadInst::opcodeName() const
{
    return "???";   // placeholder
}

std::string BranchInst::opcodeName() const
{
    return "???";   // placeholder
}
// ─────────────────────────────────────────────────────────────────────────────


// ─────────────────────────────────────────────────────────────────────────────
// TASK 3: Derived destructors — count to prove the virtual dtor fires
// ─────────────────────────────────────────────────────────────────────────────
// Define ~AddInst(), ~LoadInst(), and ~BranchInst().  Each body must execute:
//
//     ++Inst::s_destroyed;
//
// That is the ONLY thing needed in each body.  When the grader deletes an
// AddInst through a std::unique_ptr<Inst> (a base pointer), it checks that
// s_destroyed ticked upward — proof that the virtual destructor chain ran
// AddInst::~AddInst() before Inst::~Inst().  Without 'virtual ~Inst()', only
// Inst::~Inst() would run and the counter would stay at zero. (notes 25.4)
//
//   >>> YOUR CODE HERE <<<
//
AddInst::~AddInst()
{
    // placeholder — does NOT increment s_destroyed (tests will fail)
}

LoadInst::~LoadInst()
{
    // placeholder — does NOT increment s_destroyed (tests will fail)
}

BranchInst::~BranchInst()
{
    // placeholder — does NOT increment s_destroyed (tests will fail)
}
// ─────────────────────────────────────────────────────────────────────────────


// ─────────────────────────────────────────────────────────────────────────────
// TASK 4: describe() — the Ch24 cliffhanger RESOLVED
// ─────────────────────────────────────────────────────────────────────────────
// Return a string of the form "inst: <opcodeName>", e.g. "inst: add".
//
// The call inst.opcodeName() inside this function uses VIRTUAL LATE BINDING:
// even though the parameter type is 'const Inst&' (a base reference), the call
// dispatches to the most-derived override at runtime.  This is the solution to
// the Chapter 24 problem: in Ch24 all calls through a Base& hit the base
// version; adding 'virtual' sends each call to the right derived version.
// (notes 25.2)
//
// Hint: "inst: " + inst.opcodeName() gives you the right string.
//
//   >>> YOUR CODE HERE <<<
//
std::string describe(const Inst& /*inst*/)
{
    return "inst: ???";   // placeholder — real result uses virtual dispatch
}
// ─────────────────────────────────────────────────────────────────────────────


// ─────────────────────────────────────────────────────────────────────────────
// TASK 5: asBranch() — safe dynamic downcast (LLVM's dyn_cast<> in miniature)
// ─────────────────────────────────────────────────────────────────────────────
// Use dynamic_cast<BranchInst*>(inst) to attempt a downcast.
//   • If 'inst' actually points at a BranchInst, the cast succeeds and you get
//     a valid BranchInst*.
//   • If 'inst' points at something else (AddInst, LoadInst, nullptr), the cast
//     returns nullptr — safe, no exception.
//
// IMPORTANT: always use the POINTER form of dynamic_cast here (not the
// reference form); the reference form throws std::bad_cast on failure, which is
// harder to handle and NOT what LLVM's dyn_cast does.  (notes 25.10)
//
// In real LLVM code you would write:
//     if (auto *BI = dyn_cast<BranchInst>(&I)) { ... }
// This function is that pattern packaged as a helper.  (llvm-idioms.md)
//
//   >>> YOUR CODE HERE <<<
//
BranchInst* asBranch(Inst* /*inst*/)
{
    return nullptr;   // placeholder — always returns null (even for real BranchInsts)
}
// ─────────────────────────────────────────────────────────────────────────────


// ─────────────────────────────────────────────────────────────────────────────
// TASK 6: countOpcode() — walk a polymorphic instruction list
// ─────────────────────────────────────────────────────────────────────────────
// Iterate over every element of 'module' (a vector of unique_ptr<Inst>).
// For each element, dereference the unique_ptr to get an Inst&, then call
// opcodeName() on it.  Compare the result to 'opcode' (std::string_view
// compares naturally with ==).  Count and return how many match.
//
// Key insight — WHY unique_ptr<Inst> instead of a plain Inst value?
// If you stored Inst objects BY VALUE (std::vector<Inst>) and pushed an
// AddInst, the derived part would be SLICED OFF, turning it into a plain Inst
// with no override.  Storing unique_ptr<Inst> keeps the full derived object
// alive; the pointer knows nothing about the concrete type but the vtable
// still works.  This is the polymorphic-ownership idiom from notes 25.9.
//
// In real LLVM, this looks like:
//     for (auto &I : BB) { if (I.getOpcodeName() == "add") ++count; }
// (llvm-idioms.md "Walk every Instruction in a BasicBlock")
//
//   >>> YOUR CODE HERE <<<
//
int countOpcode(const std::vector<std::unique_ptr<Inst>>& /*module*/,
                std::string_view /*opcode*/)
{
    return 0;   // placeholder — always returns 0 (tests will fail for non-zero counts)
}
// ─────────────────────────────────────────────────────────────────────────────
