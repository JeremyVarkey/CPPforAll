// Chapter 12 — Compound Types: References and Pointers · Alias Workshop (SOLUTION)
// ─────────────────────────────────────────────────────────────────────────────
// One complete, correct, warning-clean implementation of ../aliases.h.
// Peek only after you've taken a real swing at starter/aliases.cpp — the deep
// understanding comes from wiring the aliasing yourself, then comparing.
//
// KEY INSIGHT running through every function here:
//   Modifying a REFERENCE or DEREFERENCED POINTER modifies the ORIGINAL object.
//   There is no copy. The tests prove this by checking the original variables —
//   not via a return value but via the side effects on the caller's stack.

#include "../aliases.h"

// ─── TASK 1: swapByRef ────────────────────────────────────────────────────────
// The classic three-step swap: save one value in a local, copy across, restore.
// `a` and `b` are REFERENCES — writing to them writes directly to the caller's
// ints (notes 12.3: "a reference is another name for the object").
// The temporary `tmp` is a plain local int — just a value holder, not a ref.
//
// Why references here, not pointers? Because the API guarantees valid objects:
//   "must exist -> reference" (notes 12.8 quick-reference table).
void swapByRef(int& a, int& b)
{
    int tmp { a };   // save a's current value in a fresh local
    a = b;           // a now holds b's old value  (writes through the reference)
    b = tmp;         // b now holds a's old value  (writes through the reference)
    // After this, the caller's two variables have been exchanged.
}

// ─── TASK 2: swapByPtr ────────────────────────────────────────────────────────
// Same three-step swap, but the caller passes ADDRESSES instead of references.
// The pointer-specific addition: a null-check guard at the top (notes 12.8).
//
// Why pointers here? The API documents that null IS a valid input. Passing
// nullptr is how the caller says "I don't have a second value to swap". Safe
// null handling is one of the key pointer disciplines in notes 12.10.
void swapByPtr(int* a, int* b)
{
    // ── Null-safety guard (notes 12.8) ─────────────────────────────────────
    // If EITHER pointer is null, there is no object to swap — do nothing.
    // `!a` is shorthand for `a == nullptr` (both work; `!a` is idiomatic).
    if (!a || !b)
        return;

    // ── Dereference to reach the pointed-to ints, then swap ────────────────
    // `*a` is the int that `a` points at; `*b` is the int that `b` points at.
    int tmp { *a };  // save *a
    *a = *b;         // overwrite the int a points at
    *b = tmp;        // restore to the int b points at
}

// ─── TASK 3: maxOf ────────────────────────────────────────────────────────────
// Return a REFERENCE to the larger of the two int parameters.
//
// This is return-by-reference (notes 12.12). The returned reference aliases
// either `a` or `b` — which are themselves references to the caller's variables.
// So the chain is: return value -> parameter reference -> caller's variable.
// Writing through the returned reference writes all the way back to the caller.
//
// SAFETY: `a` and `b` are references to objects the caller owns. Returning one
// of them returns an alias to an object that OUTLIVES this function call. This
// is the safe pattern from notes 12.12: "return a reference to a parameter".
//
// DANGER PATTERN (never do this):
//   int& bad() { int local{5}; return local; }  // local dies -> dangling ref!
int& maxOf(int& a, int& b)
{
    // If a >= b, the max is a (or either for ties); otherwise the max is b.
    // Return the reference, not the value — this is the whole point.
    if (a >= b)
        return a;
    return b;
}

// ─── TASK 4: describePointer ──────────────────────────────────────────────────
// Inspect a pointer-to-const-int and categorize it.
//
// `const int*` means "pointer to const int" (notes 12.9): we cannot modify the
// int through this pointer, but we CAN change where the pointer points.
// "pointer to const" is the right choice for a read-only inspection function.
//
// Null-check FIRST — dereferencing nullptr is undefined behavior (notes 12.8).
// Here we don't even need to dereference: the fact of null vs. non-null is
// enough to produce the label.
std::string_view describePointer(const int* ptr)
{
    if (ptr == nullptr)
        return "null";    // pointer has no target
    return "value";       // pointer has a target (the value exists)
}

// ─── TASK 5: addBonusInPlace ──────────────────────────────────────────────────
// The textbook IN/OUT parameter (notes 12.13):
//   `score`  — non-const reference: function reads it AND writes it.
//   `bonus`  — passed by value (in only): just a number to add.
//
// The += writes through the reference, changing the caller's variable directly.
// There is no return value; the modification IS the output.
//
// Notes 12.13 caution: prefer return values when practical (clearer data flow).
// In/out refs are appropriate when mutation is the central operation, exactly
// as in LLVM: `void rewriteInstruction(Instruction &I)`.
void addBonusInPlace(int& score, int bonus)
{
    score += bonus;   // writes through the reference — the caller's int changes
}

// ─── TASK 6: findFirst ────────────────────────────────────────────────────────
// A linear scan returning std::optional<int> (notes 12.15).
//
// std::optional<T> makes "no result" part of the type — the caller CANNOT
// ignore it (unlike a -1 sentinel where the caller must just know the convention).
//
// notes 12.15 explains the trade-off: optional is slightly more syntax but far
// clearer about the "maybe-empty" contract than magic sentinels.
//
// CS6340 tie-in (from the notes):
//   std::optional<CoveragePoint> parseCoverageLine(std::string_view line);
//   -> cleaner than "return {0,0}" to signal parse failure.
std::optional<int> findFirst(std::string_view text, char target)
{
    // Walk every character. `i` is signed so we can return it as int cleanly.
    for (int i { 0 }; i < static_cast<int>(text.length()); ++i)
    {
        if (text[static_cast<std::size_t>(i)] == target)
            return i;   // implicit construction: wraps the int in optional<int>
    }

    return std::nullopt;   // the "no value" sentinel (notes 12.15)
}
