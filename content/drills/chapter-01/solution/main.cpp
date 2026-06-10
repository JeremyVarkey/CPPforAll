// Chapter 1 — C++ Basics · Project: Cash Register  (REFERENCE SOLUTION)
//
// One possible complete solution. Peek only if you're stuck — you'll learn far
// more by getting your own version to pass `make test` first.

#include <iostream>   // std::cin, std::cout

int main()
{
    int unitPrice {};
    int quantity {};
    int amountPaid {};

    // TASK 1 — read all three with one chained extraction (whitespace-separated).
    std::cin >> unitPrice >> quantity >> amountPaid;

    // TASK 2 — an expression can be placed directly inside the {} initializer.
    int subtotal { unitPrice * quantity };

    // TASK 3 — reuse subtotal; amountPaid is left intact for clarity.
    int change { amountPaid - subtotal };

    std::cout << "Items: " << quantity << " x $" << unitPrice << '\n';
    std::cout << "Subtotal: $" << subtotal << '\n';
    std::cout << "Change: $" << change << '\n';

    return 0;
}
