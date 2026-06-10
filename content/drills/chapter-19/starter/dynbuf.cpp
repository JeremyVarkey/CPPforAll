// ============================================================================
//  starter/dynbuf.cpp  —  the DynBuffer Workbench implementation  (STARTER)
// ----------------------------------------------------------------------------
//  Fill in the five TASK blocks below.  Each maps 1:1 to a task in the README
//  and to a declaration in ../dynbuf.h.  The bodies currently return
//  PLACEHOLDERS so the file compiles immediately — that is why `make test` is
//  RED right now.  Your job is to turn it GREEN by writing real new[]/delete[]
//  code.
//
//      make build          compile-check your starter (already works)
//      make test           grade it  ->  RED until the TASK blocks are filled
//      make test-solution  run the grader against the reference
//
//  IMPORTANT RULES FOR THIS LAB
//  ──────────────────────────────────────────────────────────────────────────
//  • Match every new[]  with exactly one  delete[]  (not scalar delete).
//  • Match every new    with exactly one  delete    (not delete[]).
//  • After delete[] a pointer, set it to nullptr through the ref param so
//    callers can detect the freed state.  (notes 19.1 dangling-pointer danger)
//  • Do NOT use smart pointers (unique_ptr, shared_ptr) — those are Chapter 22.
//  • Do NOT use malloc/free or placement new.
//  • Do NOT use std::vector to implement the functions (that defeats the point).
//    std::vector may appear in tests/ only.
// ============================================================================

#include "../dynbuf.h"   // always include our own paired header first (ch-02 / 2.11)
                          // — the compiler checks our bodies match the contract.

// ─── TASK 1: makeBuffer — allocate and fill ──────────────────────────────────
// Allocate a heap array of `n` ints with new[], then loop over all n elements
// and set each one to `fill`.  Return the owning pointer.
//
// Key terms: new[] allocates on the HEAP (free store, notes 19.1/19.2); the
// returned pointer is the only handle to that memory — if you lose it you have
// a MEMORY LEAK (notes 19.1).  The caller is responsible for calling delete[]
// when done.
//
// Hint: int* buf { new int[n]{} };  value-initialises all elements to 0, then
// you overwrite them in a loop.  OR  new int[n]  leaves them uninitialised and
// you set them in the loop — either is fine here.
//
//   >>> YOUR CODE HERE <<<
//
int* makeBuffer(int n, int /*fill*/)
{
    // Placeholder: allocates the right number of elements but leaves them
    // zero-initialised instead of filling with `fill`.
    // The allocation is correct (uses n); the fill loop is MISSING.
    // Tests that check buf[i] == fill will fail; tests that check buf[i] == 0
    // (the fill-with-0 case) will pass by accident — which is fine since the
    // grader has the non-zero fill cases too. Replace with the real loop.
    return new int[n]{};
}
// ─────────────────────────────────────────────────────────────────────────────

// ─── TASK 2: destroyBuffer — delete[] and null out ───────────────────────────
// Call delete[] on `p`, then assign nullptr to `p` through the reference.
// That single write-back is the whole reason the parameter is `int*&` instead
// of `int*`  — a ref-to-pointer lets us reach back into the caller's variable.
//
// Key term: DANGLING POINTER — after delete[] the raw address still sits in the
// caller's variable.  It looks valid but points at freed memory.  Setting it to
// nullptr makes any accidental use-after-free detectable as a null dereference
// instead of silent heap corruption.  (notes 19.1)
//
// Don't add an  if (p)  guard — deleting nullptr is a no-op and harmless.
//
//   >>> YOUR CODE HERE <<<
//
void destroyBuffer(int*& p)
{
    // Placeholder: does nothing — a silent memory leak.
    // Fill this in to actually call delete[] and null the pointer.
    (void)p;   // suppress unused-parameter warning until you write the real code
}
// ─────────────────────────────────────────────────────────────────────────────

// ─── TASK 3: cloneBuffer — deep copy ─────────────────────────────────────────
// Allocate a brand-new array of `n` ints and copy every element from
// src[0..n-1] into it.  Return the new owning pointer.
//
// "Deep copy" means two separate heap allocations.  Copying the pointer (a
// "shallow copy") would give two owners sharing one array — the SECOND delete[]
// would be a double-free, causing undefined behaviour.  (notes 19.2)
//
// Hint: allocate, then use a for loop:
//     for (int i { 0 }; i < n; ++i)  dst[i] = src[i];
//
//   >>> YOUR CODE HERE <<<
//
int* cloneBuffer(const int* /*src*/, int n)
{
    // Placeholder: returns a value-initialised zero array, ignoring src.
    // All elements are 0, so a clone of a non-zero array will fail the tests.
    return new int[n]{};
}
// ─────────────────────────────────────────────────────────────────────────────

// ─── TASK 4: resizeBuffer — grow-or-shrink the allocation ────────────────────
// Implement the four-step resize sequence from notes 19.2:
//   Step 1.  Allocate a NEW array of `newN` ints (value-init to 0 with {}).
//   Step 2.  Copy min(oldN, newN) elements from `p` into the new array.
//   Step 3.  delete[] the OLD array.
//   Step 4.  Set `p` (through the ref) to the new array pointer.
//
// Careful with step 3: you must NOT use `p` after delete[].  Save the new
// pointer in a local first, then delete[] the old one, then reassign `p`.
// Doing it in the wrong order is a classic use-after-free bug.
//
// For the element-count: if newN > oldN, you copy oldN elements (the rest are
// already 0 from value-init).  If newN < oldN, you copy only newN elements
// (the truncated tail is gone — you already new[]'d a shorter array).
//
//   >>> YOUR CODE HERE <<<
//
void resizeBuffer(int*& p, int oldN, int newN)
{
    // Placeholder: allocates the new array but neither copies the data nor
    // deletes the old allocation — a memory leak plus missing copy.
    // Replace this with the real four-step sequence.
    int* fresh { new int[newN]{} };
    (void)oldN;
    p = fresh;   // WRONG: the old `p` was never deleted (leak) and data not copied
}
// ─────────────────────────────────────────────────────────────────────────────

// ─── TASK 5: bufferSum — read-only observer ──────────────────────────────────
// Loop over buf[0..n-1] and return the sum of all elements.  No allocation.
// No deallocation.  `buf` is const so you can't accidentally modify elements.
//
// Key term: OWNING vs OBSERVING pointers (notes 19.1): this function is an
// OBSERVER.  It sees the data but has no responsibility for the lifetime of the
// array.  The `const int*` in the parameter makes that intent machine-checkable.
//
//   >>> YOUR CODE HERE <<<
//
int bufferSum(const int* /*buf*/, int /*n*/)
{
    return 0;   // placeholder — correct only for an all-zeros buffer
}
// ─────────────────────────────────────────────────────────────────────────────
