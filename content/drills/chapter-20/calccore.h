// ============================================================================
//  calccore.h  —  PUBLIC INTERFACE of the calc-core library   (Chapter 20)
// ----------------------------------------------------------------------------
//  This header is COMPLETE and PROVIDED. Do not edit it. Read it closely —
//  it is a contract: every .cpp and the tests include this exact file.
//  Change a signature and nothing links.
//
//  WHAT THIS CHAPTER IS ABOUT — "functions as values + functions that call
//  themselves":
//
//    1) RECURSION     — a function that solves a problem by calling itself on a
//                       smaller version of the same problem (notes 20.3).
//                       Key anatomy: BASE CASE (stops the calls) + RECURSIVE
//                       CASE (shrinks toward the base).
//
//    2) FUNCTION POINTERS — store the ADDRESS of a compiled function so you can
//                       pass behavior as a parameter (notes 20.1). A raw function
//                       pointer can point to any ordinary (non-capturing) function
//                       whose signature matches.
//
//                       Syntax cheat-sheet (keep this in mind as you read below):
//
//                         int (*name)(int, int)     pointer to int(int,int)
//                         ^--- return type
//                                 ^--- this * must be grouped with the name!
//                                          ^--- parameter types
//                                          (reading pattern: "name is a pointer
//                                           to a function taking two ints,
//                                           returning int")
//
//                       Using  `int* name(int,int)`  instead means "function
//                       returning int*" — a completely different thing!
//
//    3) LAMBDAS + CAPTURES — unnamed function-like objects written inline (20.6,
//                       20.7). A capturing lambda copies/references nearby
//                       variables and behaves like a small object. Store one
//                       callable slot that accepts BOTH a lambda and a plain
//                       function using `std::function` (notes 20.1, 20.6).
//
//  CS6340 / LLVM tie-in: LLVM passes are full of "visit each instruction,
//  apply this predicate, call this callback." Function pointers, lambdas, and
//  `std::function` are the modern C++ way to wire those predicates in. You are
//  building that muscle here.
//
//  Header guard (Chapter 2 recap): stops the file from being pasted in twice
//  per translation unit when more than one file includes it.
// ============================================================================

#ifndef CALCCORE_H
#define CALCCORE_H

#include <functional>   // std::function (notes 20.1)
#include <vector>       // std::vector   (Chapter 16)

// ─── MOVEMENT 1: RECURSION ──────────────────────────────────────────────────

// ─── TASK 1 ─────────────────────────────────────────────────────────────────
// factorial(n) — compute n! using RECURSION.
//
//   Base case    : factorial(0) == 1   (by mathematical convention)
//   Recursive case: factorial(n) == n * factorial(n-1)  for n > 0
//
// Precondition: n >= 0.  n==0 MUST be your base case — check it first.
// Return type is long long so results up to factorial(20) don't overflow.
long long factorial(int n);

// ─── TASK 2 ─────────────────────────────────────────────────────────────────
// sumDigitsRecursive(n) — sum the decimal digits of n using RECURSION.
//
// Contrast: Chapter 8 introduced sumOfDigits with a LOOP (notes 8 / engine.h
// Task 3). Here you do the same job with recursion so you can feel the
// difference:
//
//   Base case    : single digit (n < 10) — the digit IS the answer.
//   Recursive case: last digit (n % 10) + sumDigitsRecursive(n / 10)
//
// Work on the magnitude so negative n matches its positive twin:
//   sumDigitsRecursive(-123) == 6  (same as sumDigitsRecursive(123)).
// sumDigitsRecursive(0) == 0.
int sumDigitsRecursive(int n);

// ─── TASK 3 ─────────────────────────────────────────────────────────────────
// fibonacci(n) — return the nth Fibonacci number using RECURSION.
//
//   fibonacci(0) == 0
//   fibonacci(1) == 1
//   fibonacci(n) == fibonacci(n-1) + fibonacci(n-2)  for n >= 2
//
// Note: the notes (20.3) point out that naive recursive Fibonacci is
// INEFFICIENT because it recomputes sub-problems (exponential calls for large
// n). That's intentional here — witnessing the call-tree growth teaches you
// *why* we mention memoization and iterative alternatives. Keep n small in
// tests (n <= 20 is safe for this naive version).
//
// Precondition: n >= 0.
int fibonacci(int n);


// ─── MOVEMENT 2: FUNCTION POINTERS ──────────────────────────────────────────

// A type alias (Chapter 10's `using`) that names the "int binary operator"
// signature so we don't have to spell out `int (*)(int, int)` everywhere.
// (notes 20.1 — "Type aliases make this readable")
using BinaryIntOp = int (*)(int, int);

// Three free functions — addOp, subOp, mulOp — implemented in calccore.cpp.
// They exist so a caller can take their ADDRESS and store it in a BinaryIntOp.
int addOp(int a, int b);   // return a + b
int subOp(int a, int b);   // return a - b
int mulOp(int a, int b);   // return a * b

// ─── TASK 4 ─────────────────────────────────────────────────────────────────
// apply(a, b, op) — call the function pointed-to by `op` with arguments a, b.
//
// This is the archetypal "callback" pattern (notes 20.1 — "Passing behavior
// into a function"): one function receives BEHAVIOR (an op) as a parameter and
// delegates to it without knowing which specific operation was passed.
//
// `op` has type `BinaryIntOp` (== `int (*)(int, int)`).  Check for null before
// calling: if op is nullptr, return 0 as a safe sentinel.
int apply(int a, int b, BinaryIntOp op);

// ─── TASK 5 ─────────────────────────────────────────────────────────────────
// pickOp(c) — RETURN a function pointer based on a character selector.
//
// This is a tiny "jump table": a char goes in, a POINTER TO THE RIGHT FUNCTION
// comes out.
//
//   '+' -> &addOp
//   '-' -> &subOp
//   '*' -> &mulOp
//   anything else -> nullptr   (unknown operation)
//
// Return type is `BinaryIntOp` (i.e., `int (*)(int, int)`).  The caller then
// passes the returned pointer into `apply` — a two-step dispatch.
BinaryIntOp pickOp(char c);


// ─── MOVEMENT 3: LAMBDAS + CAPTURES ─────────────────────────────────────────

// ─── TASK 6 ─────────────────────────────────────────────────────────────────
// countMatching(values, predicate) — count how many elements of `values`
// satisfy `predicate`.
//
// The predicate parameter is `std::function<bool(int)>` — the flexible callable
// wrapper (notes 20.1, 20.6) that accepts:
//   • a plain function pointer  (e.g. &isEven)
//   • a non-capturing lambda    ([](int x){ return x > 0; })
//   • a CAPTURING lambda        ([threshold](int x){ return x >= threshold; })
//
// Using std::function here is the correct choice for a parameter that must
// accept ALL of those forms — including capturing lambdas, which a raw
// function pointer CANNOT hold (notes 20.1 tradeoff table).
//
// Implement with a range-for loop over `values`, calling predicate(v) for each
// element and incrementing a counter when it returns true. Return the count.
int countMatching(const std::vector<int>& values,
                  std::function<bool(int)> predicate);

#endif // CALCCORE_H
