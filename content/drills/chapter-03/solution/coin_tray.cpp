// coin_tray.cpp — Vending-Machine Coin-Tray Auditor  (REFERENCE SOLUTION)
//
// All six planted logical bugs corrected. Each fix is annotated with the bug
// CLASS it belonged to and the expected-vs-actual reasoning that exposes it.
// Peek only after you've tried — the skill this chapter trains is the hunt
// itself, not the answers.
//
// Compiles warning-clean under -Wall -Wextra; passes every check in tests/.

#include "../coin_tray.h"

// TASK 1 — bug class: WRONG OPERATOR.
//   Expected coinWorth(3, 25) == 75; the buggy `count + coinValue` gave 28.
//   Value of N coins is a product, not a sum.
int coinWorth(int count, int coinValue)
{
    return count * coinValue;
}

// TASK 2 — bug class: COPY-PASTE SLIP.
//   The dimes line had been copied from the nickels line and still multiplied
//   by kNickel (5) instead of kDime (10), so every dime was undercounted by 5c.
//   Expected trayValue(2,1,0,3) == 50+10+0+3 == 63. The raw starter printed 42
//   (this line's bug plus the upstream coinWorth bug); with coinWorth fixed but
//   this line still wrong it would read 58 — either way, 5c short per dime.
int trayValue(int quarters, int dimes, int nickels, int pennies)
{
    int q { coinWorth(quarters, kQuarter) };
    int d { coinWorth(dimes,    kDime)    };   // fixed: kNickel -> kDime
    int n { coinWorth(nickels,  kNickel)  };
    int p { coinWorth(pennies,  kPenny)   };
    return q + d + n + p;
}

// TASK 3 — bug class: SWAPPED OPERANDS.
//   Subtraction is not commutative. `price - amountPaid` returns the NEGATIVE
//   of the change. Expected changeOwed(100, 75) == 25; the bug gave -25.
int changeOwed(int amountPaid, int price)
{
    return amountPaid - price;
}

// TASK 4 — bug class: OFF-BY-ONE CONSTANT.
//   A roll holds 40 quarters, not 39. Using the named constant kQuartersPerRoll
//   both fixes the value and prevents the number from drifting again.
//   Expected rollsValue(1) == 1*40*25 == 1000. The raw starter printed 64 (the
//   upstream coinWorth bug dominates); with coinWorth fixed but 39 still here it
//   reads 975 == 39*25 — the off-by-one laid bare.
int rollsValue(int rolls)
{
    int quartersInRolls { rolls * kQuartersPerRoll };   // fixed: 39 -> kQuartersPerRoll
    return coinWorth(quartersInRolls, kQuarter);
}

// TASK 5 — bug classes: SWAPPED CALL ARGUMENTS + WRONG INITIAL VALUE.
//   (a) trayValue's parameter order is (quarters, dimes, nickels, pennies);
//       the buggy call passed dimes and quarters in the wrong slots, so the
//       two were valued at each other's denomination.
//   (b) the accumulator started at kQuarter (25) instead of 0, inflating every
//       audit by 25c. Accumulators must start empty.
//   Expected machineCents(1, 2, 1, 0, 3) == trayValue(2,1,0,3) + rollsValue(1)
//                                          == 63 + 1000 == 1063.
int machineCents(int rolls, int quarters, int dimes, int nickels, int pennies)
{
    int total { 0 };                                          // fixed: kQuarter -> 0
    int loose { trayValue(quarters, dimes, nickels, pennies) };   // fixed: argument order
    int rolled { rollsValue(rolls) };
    return total + loose + rolled;
}
