// ============================================================================
//  Chapter 4 — Numeric Types Lab · demo driver (for `make run` / `make solution`).
// ============================================================================
//  Not a grader — just prints what your functions DO, so the type behavior is
//  physical: watch an unsigned value wrap, a char become an int and back, and a
//  classification chain pick a branch. Links against starter/ or solution/.
// ----------------------------------------------------------------------------
#include <iostream>
#include "../numeric_lab.h"

int main()
{
    std::cout << std::boolalpha;  // print bools as true/false, not 1/0 (4.9)

    std::cout << "== Numeric Types Lab ==\n\n";

    std::cout << "Task 1  sizeof(int32) ....... " << int32ByteWidth()
              << " bytes\n";

    std::cout << "Task 2  shiftChar('A', 1) ... '" << shiftChar('A', 1)
              << "'  (char<->int round-trip)\n";

    std::cout << "Task 3  wrapsAround(255, 1) . " << wrapsAround(255, 1)
              << "   (8-bit unsigned 255+1 -> 0)\n";

    std::cout << "Task 4  0.1 + 0.2 ~= 0.3 .... "
              << nearlyEqual(0.1 + 0.2, 0.3, 1e-9)
              << "   (== would say " << (0.1 + 0.2 == 0.3) << "!)\n";

    std::cout << "Task 5  isValidIndex(-1, 5) . " << isValidIndex(-1, 5)
              << "   (negative guarded before cast)\n";

    std::cout << "Task 6  classifyNumber(2500) '" << classifyNumber(2500.0)
              << "'  (Z/N/B/P branch tags)\n";

    std::cout << "\n(Run `make test` to grade these against the spec.)\n";
    return 0;
}
