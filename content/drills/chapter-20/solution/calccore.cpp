// ============================================================================
//  solution/calccore.cpp  —  REFERENCE SOLUTION  (Chapter 20)
// ----------------------------------------------------------------------------
//  Complete, correct, warning-clean.  Peek only after you have made a real
//  attempt at starter/calccore.cpp — the learning is in wiring up the recursion
//  and function-pointer mechanics yourself, then comparing.
// ============================================================================

#include "../calccore.h"

// ─── MOVEMENT 1: RECURSION ──────────────────────────────────────────────────

// ─── TASK 1: factorial ───────────────────────────────────────────────────────
// The key anatomy (notes 20.3):
//   BASE CASE     — n==0, no recursive call, immediately returns 1.
//   RECURSIVE CASE — n * factorial(n-1): one smaller sub-problem per call.
//
// Call-stack view for factorial(3):
//   factorial(3)
//     -> 3 * factorial(2)
//               -> 2 * factorial(1)
//                         -> 1 * factorial(0)
//                                   -> return 1      (base)
//                         return 1*1 = 1
//               return 2*1 = 2
//     return 3*2 = 6
//
// CS6340: the call graph of a recursive function is literally a tree — exactly
// what static analysis tools trace when they compute "how deep can this call
// go?" or "can this path loop forever?"
long long factorial(int n)
{
    if (n == 0)               // BASE CASE: 0! == 1 by convention
        return 1;

    return n * factorial(n - 1);  // RECURSIVE CASE: shrink toward base
}

// ─── TASK 2: sumDigitsRecursive ──────────────────────────────────────────────
// Compare with Chapter 8's sumOfDigits (engine.h Task 3) which used a WHILE
// loop: `while (n > 0) { sum += n % 10; n /= 10; }`.  Here the "shrinking"
// happens through the recursive call rather than through a loop variable.
//
// BASE CASE: n < 10 — the number IS a single digit; no more splitting needed.
// RECURSIVE: peel the last digit (n % 10), recurse on the remaining digits
//            (n / 10). The call stack depth = number of digits — shallow.
//
// Magnitude handling: negate once at the top so the '%' and '/' that follow
// always work on a non-negative value (a negative dividend produces a negative
// remainder in C++17, which would corrupt the sum).
int sumDigitsRecursive(int n)
{
    if (n < 0)                          // handle negative input: work on |n|
        return sumDigitsRecursive(-n);

    if (n < 10)                         // BASE CASE: single digit
        return n;                       // digit IS the answer (0 lands here too -> 0)

    return (n % 10) + sumDigitsRecursive(n / 10);  // RECURSIVE CASE
}

// ─── TASK 3: fibonacci ───────────────────────────────────────────────────────
// Two base cases because the Fibonacci definition uses the two immediately
// preceding values: fib(0)=0, fib(1)=1.
//
// INEFFICIENCY NOTE (notes 20.3): fibonacci(n-1) and fibonacci(n-2) each
// redundantly recompute overlapping sub-problems. fibonacci(5) alone creates
// 15 recursive calls. For n=20 it's ~6765 calls; for n=40 it would be ~300M.
// The lesson: recursion is a TOOL, not always the right one for efficiency.
// A loop or memoisation would be O(n) instead of O(2^n).
int fibonacci(int n)
{
    if (n == 0) return 0;      // BASE CASE 1
    if (n == 1) return 1;      // BASE CASE 2

    return fibonacci(n - 1) + fibonacci(n - 2);  // RECURSIVE CASE
}


// ─── MOVEMENT 2: FUNCTION POINTERS ──────────────────────────────────────────

// Free functions (provided — unchanged from starter) ─────────────────────────
// These are PLAIN functions: no captures, concrete signatures, addressable.
// A pointer-to-function can point to any of them because they all match the
// `int(int, int)` signature captured by the `BinaryIntOp` alias.
int addOp(int a, int b) { return a + b; }
int subOp(int a, int b) { return a - b; }
int mulOp(int a, int b) { return a * b; }

// ─── TASK 4: apply ───────────────────────────────────────────────────────────
// The canonical "callback" pattern (notes 20.1): the algorithm is fixed (call
// op with a and b), but the BEHAVIOR is variable (which function op points to).
//
// Null check first: calling through a null function pointer is UNDEFINED
// BEHAVIOR (notes 20.1) — the program would crash in a deeply confusing way.
// Return a safe sentinel instead.
//
// Calling through a function pointer reads identically to calling a function
// directly:  op(a, b)  dereferences the pointer and invokes the pointed-to
// function.  The explicit form  (*op)(a, b)  means the same thing.
int apply(int a, int b, BinaryIntOp op)
{
    if (!op)           // guard against null — never call a null function pointer
        return 0;

    return op(a, b);   // call through the pointer: dispatch to addOp/subOp/mulOp
}

// ─── TASK 5: pickOp ──────────────────────────────────────────────────────────
// A mini "jump table": map a character selector to the ADDRESS of the right
// operation function and RETURN that pointer.  The caller gets back a
// `BinaryIntOp` (i.e., `int (*)(int, int)`) and passes it straight to apply().
//
// This two-step — pickOp(...) then apply(...) — decouples dispatch from
// execution, the same pattern as a dispatch table in a virtual machine or an
// LLVM pass pipeline.
BinaryIntOp pickOp(char c)
{
    switch (c)
    {
    case '+': return &addOp;   // return the ADDRESS of addOp
    case '-': return &subOp;
    case '*': return &mulOp;
    default:  return nullptr;  // unknown operation -> null pointer
    }
}


// ─── MOVEMENT 3: LAMBDAS + CAPTURES ─────────────────────────────────────────

// ─── TASK 6: countMatching ───────────────────────────────────────────────────
// The predicate parameter is `std::function<bool(int)>` — the flexible wrapper
// (notes 20.1, 20.6) that can store:
//   • a plain function pointer  (e.g. &isEven)
//   • a NON-capturing lambda    ([](int x){ return x > 0; })
//   • a CAPTURING lambda        ([threshold](int x){ return x >= threshold; })
//
// A RAW function pointer (`bool (*)(int)`) would NOT accept the capturing
// lambda because captures turn a lambda into a closure object, not a free
// function.  std::function handles type erasure so we can write one parameter
// type that works for all three forms.
//
// CS6340 tie-in: std::find_if, std::count_if, and every LLVM traversal helper
// that takes a predicate follow this exact shape — you pass a small, local
// predicate that captures the context it needs.
int countMatching(const std::vector<int>& values,
                  std::function<bool(int)> predicate)
{
    int count { 0 };

    for (int v : values)        // range-for over the vector (Chapter 16)
    {
        if (predicate(v))       // call through std::function — works for any callable
            ++count;
    }

    return count;
}
