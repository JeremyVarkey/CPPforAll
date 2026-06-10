// Chapter 25 — Virtual Functions · MINI-LLVM: inst.h
// ─────────────────────────────────────────────────────────────────────────────
// PUBLIC INTERFACE — declarations only. The learner implements these in
// starter/inst.cpp; tests and the demo include this header and call through it.
//
// DESIGN NOTE: This file intentionally mirrors LLVM's own Instruction
// hierarchy (llvm::Instruction -> llvm::BinaryOperator, llvm::LoadInst,
// llvm::BranchInst …). The names you learn here map directly to the real API
// you will read in CS6340 labs:
//
//   This exercise       Real LLVM
//   ──────────────────  ─────────────────────────────────────────────
//   Inst                llvm::Instruction
//   Inst::opcodeName()  Instruction::getOpcodeName()
//   AddInst             llvm::BinaryOperator (with Add opcode)
//   LoadInst            llvm::LoadInst
//   BranchInst          llvm::BranchInst
//   asBranch(p)         llvm::dyn_cast<BranchInst>(&I)  [llvm-idioms.md]
//   countOpcode(…)      a walk of Module → Function → BB → Instruction
//
// ─────────────────────────────────────────────────────────────────────────────

#ifndef INST_H
#define INST_H

#include <memory>      // std::unique_ptr  (Ch 22 — provided scaffolding)
#include <string>      // std::string
#include <string_view> // std::string_view
#include <vector>      // std::vector      (Ch 16)

// ─────────────────────────────────────────────────────────────────────────────
// Inst — abstract base class for every MINI-LLVM instruction.
//
// "Abstract" means it has at least one pure virtual function (= 0), so you
// cannot instantiate Inst directly — you can only hold Inst* or Inst& pointing
// at a CONCRETE derived object. (notes 25.7)
//
// The VIRTUAL DESTRUCTOR is mandatory for any class deleted through a base
// pointer. Without it, 'delete base_ptr' would call only ~Inst(), leaking the
// derived object's resources. (notes 25.4)
// ─────────────────────────────────────────────────────────────────────────────
class Inst
{
public:
    // ── Virtual destructor (TASK 1) ──────────────────────────────────────────
    // Must be virtual so that 'delete inst_ptr' calls the derived destructor.
    // Declare it here; define the body in inst.cpp.  Track destruction with the
    // static counter s_destroyed so tests can PROVE it fired through a base ptr.
    virtual ~Inst();

    // ── Pure virtual function (TASK 2) ───────────────────────────────────────
    // Every concrete Inst MUST supply its opcode name.
    // "Pure virtual" (= 0) makes Inst an ABSTRACT BASE CLASS. (notes 25.7)
    virtual std::string opcodeName() const = 0;

    // ── describe() free function (declared below) will call this through Inst& ─

    // ── Static destroyed counter — lets tests verify the virtual dtor ran ────
    // NOT a TASK block; this is provided scaffolding for the test harness.
    static int s_destroyed; // definition lives in inst.cpp
};

// ─────────────────────────────────────────────────────────────────────────────
// AddInst — represents an integer addition instruction.
//   In real LLVM: llvm::BinaryOperator with opcode Instruction::Add
// ─────────────────────────────────────────────────────────────────────────────
class AddInst : public Inst
{
public:
    // ── TASK 2 (continued) ───────────────────────────────────────────────────
    // Override opcodeName() to return "add".
    // The 'override' specifier tells the compiler to verify this matches the
    // base virtual signature — a typo (wrong const, wrong return type) becomes
    // a compile error, not a silent new function. (notes 25.3)
    std::string opcodeName() const override;

    // ── TASK 3 ───────────────────────────────────────────────────────────────
    // Override ~AddInst() — it must increment Inst::s_destroyed so the grader
    // can confirm the derived dtor ran when deleted through a base pointer.
    ~AddInst() override;
};

// ─────────────────────────────────────────────────────────────────────────────
// LoadInst — represents a memory-load instruction.
//   In real LLVM: llvm::LoadInst
// ─────────────────────────────────────────────────────────────────────────────
class LoadInst : public Inst
{
public:
    // ── TASK 2 (continued) ───────────────────────────────────────────────────
    std::string opcodeName() const override;

    // ── TASK 3 ───────────────────────────────────────────────────────────────
    ~LoadInst() override;
};

// ─────────────────────────────────────────────────────────────────────────────
// BranchInst — represents a branch instruction; always the last in a block.
//   In real LLVM: llvm::BranchInst
//
// 'final' on the override means NO class may further override opcodeName()
// by inheriting from BranchInst.  Use it when the hierarchy should stop here.
// (notes 25.3)
// ─────────────────────────────────────────────────────────────────────────────
class BranchInst : public Inst
{
public:
    // ── TASK 2 (continued) ───────────────────────────────────────────────────
    // 'final' locks this override — no subclass of BranchInst can change it.
    std::string opcodeName() const override final;

    // ── TASK 3 ───────────────────────────────────────────────────────────────
    ~BranchInst() override;

    // BranchInst-specific API (downcast target in TASK 5):
    bool isUnconditional() const { return true; } // simplified — always true here
};

// ─────────────────────────────────────────────────────────────────────────────
// describe() — free function that accepts any Inst by CONST REFERENCE and
// returns a human-readable string like "add: <opcode>".
//
// The call 'inst.opcodeName()' inside describe() dispatches POLYMORPHICALLY
// (virtual late binding through the reference) — this is the Ch24 cliffhanger
// RESOLVED.  In Ch24, non-virtual functions on a Base& would always call the
// base version; with 'virtual', the actual derived override runs. (notes 25.2)
// ─────────────────────────────────────────────────────────────────────────────
// ── TASK 4 (declared here, implemented in inst.cpp) ──────────────────────────
std::string describe(const Inst& inst);

// ─────────────────────────────────────────────────────────────────────────────
// asBranch() — safe downcast from Inst* to BranchInst*.
//
// Returns a valid BranchInst* when the pointed-at object IS a BranchInst;
// returns nullptr otherwise.  This is the C++ equivalent of LLVM's own
// dyn_cast<BranchInst>(&inst) — see llvm-idioms.md "dyn_cast<>".
//
// IMPORTANT: we use the POINTER form of dynamic_cast so failure gives nullptr,
// not an exception.  (The reference form throws std::bad_cast — notes 25.10.)
// ─────────────────────────────────────────────────────────────────────────────
// ── TASK 5 (declared here, implemented in inst.cpp) ──────────────────────────
BranchInst* asBranch(Inst* inst);

// ─────────────────────────────────────────────────────────────────────────────
// countOpcode() — walk a "module" (a vector of owned instructions) and count
// how many have a given opcode name.
//
// This mirrors Lab-0's Module → BB → Instruction walk without needing LLVM.
// The vector stores std::unique_ptr<Inst> (Ch 22) to OWN the objects and avoid
// slicing: std::vector<Inst> would SLICE derived objects on insertion, erasing
// the dynamic type entirely. (notes 25.9)
// ─────────────────────────────────────────────────────────────────────────────
// ── TASK 6 (declared here, implemented in inst.cpp) ──────────────────────────
int countOpcode(const std::vector<std::unique_ptr<Inst>>& module,
                std::string_view opcode);

#endif // INST_H
