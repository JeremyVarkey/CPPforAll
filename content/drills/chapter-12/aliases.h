// ============================================================================
//  aliases.h  —  the PUBLIC INTERFACE of the "Alias Workshop"  (Chapter 12)
// ----------------------------------------------------------------------------
//  This header is COMPLETE and PROVIDED. Do NOT edit it.
//
//  Chapter 12 is the chapter where C++ stops copying everything and lets two
//  names refer to the SAME memory. This file is the proof: every function here
//  either accepts a reference / pointer so it can modify the caller's variable,
//  or RETURNS one so the caller can modify the original through the return value.
//
//  HEADER GUARD (Chapter 2): the #ifndef / #define sandwich below stops the
//  contents being pasted in twice if two translation units both #include it.
//
//  CS6340 LLVM tie-in:
//    bool runOnFunction(Function &F)        <- pass-by-reference, non-const
//    void instrumentCoverage(Instruction &I)
//    Module *M { F.getParent() }            <- may be null → pointer semantics
//  Every signature in this file is a miniature of those LLVM patterns.
// ============================================================================

#ifndef ALIASES_H
#define ALIASES_H

#include <optional>      // std::optional<T>  (12.15)
#include <string_view>   // std::string_view  (Ch 5, reused here)

// ─── TASK 1: swapByRef ───────────────────────────────────────────────────────
// Swap the values of two ints through NON-CONST LVALUE REFERENCES (notes 12.3,
// 12.5). After the call, `a` holds b's old value and `b` holds a's old value.
//
// Design note: references are the right tool here — an object MUST exist (null
// is impossible). The caller's variables are modified in-place; no pointers
// needed. Compare this to swapByPtr below, where the caller may pass nullptr.
//
// CS6340 parallel: like `void Instrument::modifyInstruction(Instruction &I)` —
// the instruction must exist, so we take a reference, not a pointer.
void swapByRef(int& a, int& b);

// ─── TASK 2: swapByPtr ───────────────────────────────────────────────────────
// Swap the values pointed to by two int POINTERS (notes 12.7, 12.10). If EITHER
// pointer is nullptr, do NOTHING — this is the null-safety contract.
//
// Design note: pointers signal "null is a valid caller-supplied state" (notes
// 12.8: "may be absent → pointer"). Always null-check before dereferencing.
//
// CS6340 parallel: like `Module *M { F.getParent() }` — getParent may return
// nullptr if the function isn't attached to a module; check before using.
void swapByPtr(int* a, int* b);

// ─── TASK 3: maxOf ───────────────────────────────────────────────────────────
// Return a MODIFIABLE REFERENCE to the larger of two ints (notes 12.12).
// The reference aliases whichever of `a` or `b` is larger — the caller can
// assign through it to change the original:
//
//     int x{3}, y{7};
//     maxOf(x, y) = 100;   // y (the max) is now 100
//     assert(y == 100);    // the ORIGINAL changed — aliasing is physical
//
// DANGER — THE CHAPTER'S NUMBER-ONE TRAP (notes 12.12):
//   Never return a reference to a local variable.
//   A local dies when the function returns; the reference would DANGLE.
//   This function is safe because it returns a reference to one of its
//   PARAMETERS, which are themselves references to the caller's variables
//   (the aliased objects outlive this call).
//
// Equal values: return a reference to `a` (either is correct).
int& maxOf(int& a, int& b);

// ─── TASK 4: describePointer ─────────────────────────────────────────────────
// Inspect a POINTER TO CONST INT (notes 12.9) and return a string_view label:
//   nullptr   ->  "null"
//   anything  ->  "value"        (the pointed-to value is some integer)
//
// The parameter is `const int*` — "pointer to const int" (notes 12.9). The
// function only reads, never writes through the pointer. You are NOT allowed to
// dereference a null pointer; check first (notes 12.8).
//
// Returning std::string_view of a string literal is safe — literals have
// static storage duration (they outlive everything).
std::string_view describePointer(const int* ptr);

// ─── TASK 5: addBonusInPlace ─────────────────────────────────────────────────
// A textbook IN/OUT PARAMETER (notes 12.13): `score` is read and modified
// (it is the in/out parameter), `bonus` is read only (it is the in parameter).
// Add `bonus` to `score` in place. The caller's variable changes.
//
// Notes 12.13 contrast: using a non-const reference signals "I will modify this
// value" — it documents the mutation at the call site. Compare:
//   addBonusInPlace(score, 10)  <-- `score` is an out param (visibly modified)
//   score += 10                 <-- same effect, but local, not via function
//
// CS6340 parallel: like `void normalize(Instruction &I)` — the instruction is
// read and rewritten in one pass.
void addBonusInPlace(int& score, int bonus);

// ─── TASK 6: findFirst (std::optional) ───────────────────────────────────────
// Search `text` for the first occurrence of `target` character and return its
// 0-based index as std::optional<int> (notes 12.15).
//
// Return std::nullopt if `target` does not appear.
// Return the 0-based index (an int) if it does.
//
// std::optional<T> makes "no result" EXPLICIT IN THE TYPE — the caller must
// handle the maybe-empty case (notes 12.15). The sentinel -1 approach forces
// the caller to remember the convention; optional enforces it.
//
// CS6340 tie-in (from notes 12.15):
//   std::optional<CoveragePoint> parseCoverageLine(std::string_view line);
//
// Parameter is std::string_view: cheap, non-owning, read-only (notes 12.6).
// Cast index to std::size_t when indexing to satisfy -Wsign-conversion (Ch 10).
std::optional<int> findFirst(std::string_view text, char target);

#endif // ALIASES_H
