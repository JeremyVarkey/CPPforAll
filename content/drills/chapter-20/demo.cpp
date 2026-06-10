// ============================================================================
//  demo.cpp  —  ungraded demonstration driver  (Chapter 20)
// ----------------------------------------------------------------------------
//  This file is NOT graded. It exists to show the library in action and to
//  demonstrate Chapter 20 features that don't belong in the pure calc-core API:
//    • CLI arguments (notes 20.4) — the user can pass a number on the command
//      line and see recursive computations on it.
//    • Lambda captures observed live — a local `threshold` captured into a
//      lambda passed to countMatching.
//    • The full pickOp -> apply dispatch chain end-to-end.
//
//  Build and run with `make run`.  You do NOT need to edit this file.
// ============================================================================

#include <iostream>
#include <string>
#include <vector>
#include "calccore.h"

// A plain (non-capturing) predicate — demonstrates that countMatching accepts
// both a raw function pointer and a lambda (notes 20.1, 20.6).
static bool isEven(int x) { return x % 2 == 0; }

int main(int argc, char* argv[])
{
    // ── CLI argument parsing (notes 20.4) ────────────────────────────────────
    // Parse an optional integer from argv[1]; default to 7 if not provided.
    // The notes 20.4 CLI example guards std::stoi with try/catch because stoi
    // throws on non-numeric text.  try/catch is a PREVIEW here — exceptions are
    // formally Chapter 27.  It lives only in this ungraded demo (never in the
    // graded library), so you are not asked to write it yourself.
    int n { 7 };   // default demo value
    if (argc >= 2)
    {
        try   // (a preview — formally Chapter 27)
        {
            n = std::stoi(argv[1]);
        }
        catch (...)
        {
            std::cerr << "usage: calc-demo [integer]\n";
            return 1;
        }
    }

    std::cout << "=== calc-core demo (n = " << n << ") ===\n\n";

    // ── MOVEMENT 1: Recursion ────────────────────────────────────────────────
    std::cout << "--- Recursion ---\n";
    if (n >= 0 && n <= 20)
        std::cout << "factorial(" << n << ")          = " << factorial(n)       << "\n";
    else
        std::cout << "(factorial demo skipped: keep n in 0..20)\n";

    std::cout << "sumDigitsRecursive(" << n << ") = " << sumDigitsRecursive(n) << "\n";

    if (n >= 0 && n <= 20)
        std::cout << "fibonacci("  << n << ")          = " << fibonacci(n)        << "\n";
    else
        std::cout << "(fibonacci demo skipped: keep n in 0..20)\n";

    std::cout << "\n";

    // ── MOVEMENT 2: Function Pointers ────────────────────────────────────────
    std::cout << "--- Function Pointers ---\n";
    int a { n }, b { n / 2 + 1 };

    // Apply uses a function POINTER passed as a parameter.
    std::cout << "apply(" << a << ", " << b << ", +) = " << apply(a, b, &addOp) << "\n";
    std::cout << "apply(" << a << ", " << b << ", -) = " << apply(a, b, &subOp) << "\n";
    std::cout << "apply(" << a << ", " << b << ", *) = " << apply(a, b, &mulOp) << "\n";

    // pickOp returns a function pointer; pass it straight to apply.
    for (char op : { '+', '-', '*', '?' })
    {
        BinaryIntOp fn { pickOp(op) };
        if (fn)
            std::cout << "pickOp('" << op << "') -> apply(" << a << "," << b << ") = "
                      << apply(a, b, fn) << "\n";
        else
            std::cout << "pickOp('" << op << "') -> nullptr (unknown op)\n";
    }

    std::cout << "\n";

    // ── MOVEMENT 3: Lambdas + Captures ───────────────────────────────────────
    std::cout << "--- Lambdas + Captures ---\n";
    std::vector<int> values { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

    // Plain function pointer as predicate
    std::cout << "countMatching(1..10, isEven)   = " << countMatching(values, &isEven) << "\n";

    // Capturing lambda: `threshold` is a LOCAL variable captured by value.
    // (notes 20.7) — the lambda stores its own copy of `threshold`.
    int threshold { (n > 0 && n <= 10) ? n : 5 };
    std::cout << "threshold = " << threshold << "\n";
    std::cout << "countMatching(1..10, x >= " << threshold << ") = "
              << countMatching(values, [threshold](int x){ return x >= threshold; })
              << "\n";

    return 0;
}
