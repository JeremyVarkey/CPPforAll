// ============================================================================
//  solution/dynbuf.cpp  —  reference IMPLEMENTATION of the DynBuffer Workbench
// ----------------------------------------------------------------------------
//  One complete, correct, warning-clean implementation of ../dynbuf.h.
//  Peek only after you have attempted starter/dynbuf.cpp — the learning is in
//  writing the new[]/delete[] bookkeeping yourself, then comparing.
//
//  Every new[] in this file has exactly one matching delete[].  There are no
//  leaks, no double-frees, and no dangling pointers visible to callers.
//  A Valgrind/ASan run on the test binary will report zero errors.
//
//  CS6340 lens: this entire file is the skeleton of a resource-owning class.
//  If you wrapped these functions into a class with a destructor, you'd have
//  a handwritten RAII buffer — exactly what std::vector does (notes 19.3).
// ============================================================================

#include "../dynbuf.h"   // paired-header include (ch-02 / 2.11): compile-time
                          // contract check — bodies must match declarations.

// ─── TASK 1: makeBuffer ──────────────────────────────────────────────────────
// new int[n]  requests `n` consecutive ints from the HEAP (free store, notes
// 19.1 "The heap"; the array form of new is notes 19.2).
// The {} value-initialises them to 0; the loop then writes `fill`.
// (We could skip the value-init since we overwrite every element, but starting
// from 0 and overwriting is clearer and never leaks information.)
//
// Ownership: we allocate and IMMEDIATELY hand the pointer back to the caller.
// From this moment, the CALLER is the owner responsible for delete[].
int* makeBuffer(int n, int fill)
{
    int* buf { new int[n]{} };   // heap-allocate n ints, zero-initialised (19.2)

    for (int i { 0 }; i < n; ++i)
        buf[i] = fill;           // overwrite every slot with the requested value

    return buf;                  // transfer ownership to the caller
}

// ─── TASK 2: destroyBuffer ───────────────────────────────────────────────────
// The `int*&` (reference to pointer) is what makes this an in-out parameter.
// Without the `&` the assignment `p = nullptr` would modify only a local copy
// of the pointer, and the caller's variable would still hold the dangling
// address — a classic silent-but-deadly bug (notes 19.1 dangling pointers,
// notes 12 ref-to-pointer out-param pattern).
//
// delete[] nullptr is explicitly harmless (C++ standard); no guard needed.
void destroyBuffer(int*& p)
{
    delete[] p;   // release the heap storage; calls no dtors (plain int array)
    p = nullptr;  // null out the caller's copy so it can't dangle (notes 19.1)
}

// ─── TASK 3: cloneBuffer ─────────────────────────────────────────────────────
// This is a DEEP copy: two independent heap allocations.  After this function
// returns, mutating one array does NOT affect the other.
//
// Contrast with: `int* alias { src };` — that is a SHALLOW copy (alias copy).
// Two "owners" of the same memory means two future delete[]s — undefined
// behaviour: double-free (notes 19.2 "copying the pointer does not copy the
// elements").
int* cloneBuffer(const int* src, int n)
{
    int* dst { new int[n]{} };   // fresh array, independent of `src` (19.2)

    for (int i { 0 }; i < n; ++i)
        dst[i] = src[i];         // element-by-element copy: a real deep copy

    return dst;                  // caller owns this new allocation
}

// ─── TASK 4: resizeBuffer ────────────────────────────────────────────────────
// This is the exact algorithm std::vector uses when it grows (notes 19.2
// "Resizing means allocating a new array").  Order matters critically:
//
//   WRONG order:   delete[] p;  then  p = fresh;
//   — After delete[] you have freed p's memory.  Assigning p to fresh is fine,
//     but you can no longer read old elements because the memory is gone.
//     We already saved them above, so the COPY must happen BEFORE the delete[].
//
//   RIGHT order (implemented below):
//     1. new[]  — fresh allocation is independent; old allocation untouched.
//     2. copy   — read from old allocation while it is still live.
//     3. delete[] — now it is safe to free the old allocation.
//     4. p = fresh — redirect caller's pointer to the new home.
void resizeBuffer(int*& p, int oldN, int newN)
{
    // Step 1 — allocate the new, correctly-sized array, zero-initialised.
    //          Extra slots (when newN > oldN) are already 0 from the `{}`.
    int* fresh { new int[newN]{} };

    // Step 2 — copy the SMALLER of oldN and newN elements from the old array.
    //          If shrinking: we keep only the prefix.
    //          If growing:   we keep everything and the new tail stays 0.
    int copyCount { (oldN < newN) ? oldN : newN };   // min(oldN, newN)
    for (int i { 0 }; i < copyCount; ++i)
        fresh[i] = p[i];   // read from OLD allocation — it is still valid here

    // Step 3 — free the OLD allocation.  We've already extracted what we need.
    delete[] p;            // p is still the old pointer here — safe to delete

    // Step 4 — redirect the caller's pointer to the new allocation.
    p = fresh;             // caller now owns `fresh`; old memory is released
}

// ─── TASK 5: bufferSum ───────────────────────────────────────────────────────
// A pure observer: reads but never modifies or owns.  The `const int*` in the
// signature is the machine-readable promise that we will not write through `buf`
// (notes 12 const correctness; notes 19.1 observing vs owning pointers).
//
// No allocation, no deallocation.  The only resource here is CPU time.
int bufferSum(const int* buf, int n)
{
    int total { 0 };
    for (int i { 0 }; i < n; ++i)
        total += buf[i];
    return total;
}
