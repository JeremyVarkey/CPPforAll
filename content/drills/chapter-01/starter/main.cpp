// Chapter 1 — C++ Basics · Project: Cash Register  (STARTER)
//
// Fill in the three TODO blocks below. Build/run/grade with:
//     make        build
//     make run    run (then type:  5 3 20  and press Enter)
//     make test   grade against the expected output
//
// Everything lives in main() on purpose — defining your own functions is Chapter 2.

#include <iostream>   // std::cin, std::cout

int main()
{
    // The three inputs. Chapter 1's golden rule: initialize on definition.
    int unitPrice {};
    int quantity {};
    int amountPaid {};

    // ─── TASK 1: read the three inputs from std::cin, IN THIS ORDER ─────
    //   unitPrice, then quantity, then amountPaid.
    // Hint: one chained extraction reads them all:  std::cin >> a >> b >> c;
    //
    //   >>> YOUR CODE HERE <<<
    //
    // ───────────────────────────────────────────────────────────────────

    // ─── TASK 2: compute the subtotal = unitPrice * quantity ───────────
    // Replace the empty {} with the real expression.
    //
    //   >>> YOUR CODE HERE <<<
    int subtotal {};
    // ───────────────────────────────────────────────────────────────────

    // ─── TASK 3: compute the change = amountPaid - subtotal ────────────
    // Start from amountPaid below and subtract the subtotal.
    //
    //   >>> YOUR CODE HERE <<<
    int change { amountPaid };
    // ───────────────────────────────────────────────────────────────────

    // The receipt is printed for you — do NOT change these lines or the format.
    std::cout << "Items: " << quantity << " x $" << unitPrice << '\n';
    std::cout << "Subtotal: $" << subtotal << '\n';
    std::cout << "Change: $" << change << '\n';

    return 0;
}
