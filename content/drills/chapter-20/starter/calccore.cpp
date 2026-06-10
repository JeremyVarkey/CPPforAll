// ============================================================================
//  starter/calccore.cpp  —  the LEARNER's implementation file  (Chapter 20)
// ----------------------------------------------------------------------------
//  Fill in the six TASK blocks below. Each maps 1:1 to a task in the README
//  and to a declaration in ../calccore.h. The bodies currently return WRONG
//  PLACEHOLDERS so the file compiles immediately — that is why `make test` is
//  RED right now. Your job is to turn it GREEN.
//
//    make build        compile-check your code (should already succeed)
//    make test         grade your code  (RED until you fill these in)
//    make solution     run the reference solution to see expected output
//
//  Everything here is PURE: arguments in, value out. No std::cin/cout inside
//  the library functions — keep them deterministic and testable.
// ============================================================================

#include "../calccore.h"

// ─── MOVEMENT 1: RECURSION ──────────────────────────────────────────────────
// Anatomy review (notes 20.3):
//   BASE CASE    — the simplest input that can be answered directly (stops the
//                  recursion; EVERY path must reach it).
//   RECURSIVE CASE — reduce the problem, then call yourself on the smaller part.
// The call stack grows one frame per call; always know the maximum depth.

// ─── TASK 1: factorial — recursion on integers ───────────────────────────────
// Implement factorial(n) recursively.
//   Base case    : if n == 0, return 1   (0! == 1 by convention)
//   Recursive case: return n * factorial(n - 1)
//
// Return type is long long (can hold up to factorial(20) without overflow).
// Precondition: n >= 0.  You do NOT need to handle negative n.
//
//   >>> YOUR CODE HERE <<<
//
long long factorial(int n)
{
    (void)n;    // suppress unused-parameter warning on placeholder
    return 0;   // placeholder — wrong for every input (even factorial(0), which is 1)
}
// ─────────────────────────────────────────────────────────────────────────────

// ─── TASK 2: sumDigitsRecursive — same job as Ch8's loop, now recursive ───────
// Chapter 8 introduced sumOfDigits using a WHILE loop (notes 8, engine.h Task 3).
// Here you solve the same problem with recursion so you can feel the difference:
//
//   Base case    : n < 10 (single digit) — the digit IS the answer, return n.
//   Recursive case: (n % 10) + sumDigitsRecursive(n / 10)
//                    ^last digit  ^everything else, recurse on it
//
// Handle negative n by working on the magnitude:
//   sumDigitsRecursive(-123) must equal 6 (same as for 123).
// sumDigitsRecursive(0) must equal 0.
//
//   >>> YOUR CODE HERE <<<
//
int sumDigitsRecursive(int n)
{
    (void)n;    // suppress unused-parameter warning on placeholder
    return 0;   // placeholder — only correct for n == 0
}
// ─────────────────────────────────────────────────────────────────────────────

// ─── TASK 3: fibonacci — witness the exponential call tree ────────────────────
// Implement fibonacci(n) recursively.
//   Base cases   : fibonacci(0) == 0,  fibonacci(1) == 1
//   Recursive case: fibonacci(n) == fibonacci(n-1) + fibonacci(n-2)  for n >= 2
//
// *** WARNING (notes 20.3) *** naive recursive Fibonacci is EXPONENTIAL in the
// number of calls — fibonacci(n-1) and fibonacci(n-2) each re-solve overlapping
// sub-problems. This is intentional: you are meant to observe that recursion is
// not always the most efficient tool. Keep test values n <= 20 for this version.
//
//   >>> YOUR CODE HERE <<<
//
int fibonacci(int n)
{
    (void)n;    // suppress unused-parameter warning on placeholder
    return 0;   // placeholder — wrong for all inputs except fibonacci(0)
}
// ─────────────────────────────────────────────────────────────────────────────


// ─── MOVEMENT 2: FUNCTION POINTERS ──────────────────────────────────────────
// Syntax reminder (notes 20.1):
//
//   int (*name)(int, int)   — `name` is a POINTER TO a function that takes two
//                             ints and returns int.
//
//   BinaryIntOp op          — same thing, via the `using` alias in calccore.h.
//
// Calling:   op(3, 4)       — call the function pointed-to by op.
//            (*op)(3, 4)    — explicit dereference; same result.
// Null check: if (op) { ... }  before calling, if pointer may be null.

// Three plain free functions — addOp, subOp, mulOp ──────────────────────────
// These are ALREADY PROVIDED and complete.  Do not change them.
// They exist so a caller can take their address: BinaryIntOp p { &addOp };

int addOp(int a, int b) { return a + b; }
int subOp(int a, int b) { return a - b; }
int mulOp(int a, int b) { return a * b; }

// ─── TASK 4: apply — call op(a, b) through the function pointer ──────────────
// The classic "callback" pattern (notes 20.1): receive BEHAVIOR as a parameter
// and delegate to it without knowing the specific operation.
//
// Implement: call op(a, b) and return the result.
// Guard    : if op is nullptr, return 0 as a safe sentinel (do NOT call nullptr).
//
// You CALL a function pointer just like a regular function: op(a, b).
//
//   >>> YOUR CODE HERE <<<
//
int apply(int a, int b, BinaryIntOp op)
{
    (void)a; (void)b; (void)op;  // suppress unused-parameter warnings on placeholder
    return 0;   // placeholder — returns 0 for everything
}
// ─────────────────────────────────────────────────────────────────────────────

// ─── TASK 5: pickOp — RETURN a function pointer (mini jump table) ────────────
// Map a character to the address of the matching operation function.
//
// Return the ADDRESS of the right function — e.g. `return &addOp;` — or
// nullptr for an unrecognised character.
//   '+' -> &addOp
//   '-' -> &subOp
//   '*' -> &mulOp
//   anything else -> nullptr
//
// Use a switch or if/else chain. Return type is BinaryIntOp (see calccore.h).
// The caller will pass the returned pointer straight into apply().
//
//   >>> YOUR CODE HERE <<<
//
BinaryIntOp pickOp(char c)
{
    (void)c;    // suppress unused-parameter warning on placeholder
    return nullptr;   // placeholder — always returns null (unknown op)
}
// ─────────────────────────────────────────────────────────────────────────────


// ─── MOVEMENT 3: LAMBDAS + CAPTURES ─────────────────────────────────────────
// Reminder (notes 20.6, 20.7):
//
//   [](int x){ return x > 0; }       — non-capturing lambda, no outer state
//   [threshold](int x){ return x >= threshold; }  — capture `threshold` by VALUE
//   [&total](int x){ total += x; }   — capture `total` by REFERENCE
//
// std::function<bool(int)> can store ANY of the above forms, plus a plain
// function pointer. That flexibility is exactly why countMatching uses it.

// ─── TASK 6: countMatching — count elements satisfying predicate ──────────────
// Iterate over `values` with a range-for loop.
// For each element v, call predicate(v); if it returns true, increment a counter.
// Return the final count.
//
// The predicate is std::function<bool(int)> — it will be tested with:
//   • a plain function pointer  (no captures needed)
//   • a CAPTURING lambda that captures a local threshold variable
//
//   >>> YOUR CODE HERE <<<
//
int countMatching(const std::vector<int>& values,
                  std::function<bool(int)> predicate)
{
    (void)values; (void)predicate;  // suppress warnings on placeholder
    return 0;   // placeholder — always reports zero matches
}
// ─────────────────────────────────────────────────────────────────────────────
