// coin_tray.h — the INTERFACE for the Vending-Machine Coin-Tray Auditor.
//
// This file is COMPLETE and CORRECT. Do not edit it. It contains only
// DECLARATIONS (the shape of each function) plus the four coin values. The
// bodies — where the bugs are hiding — live in starter/coin_tray.cpp.
//
// Read these declarations as a contract. Each comment states EXACTLY what the
// function is supposed to return. When you debug, that promise is your
// "expected"; the value the code actually produces is your "actual". The bug is
// always the first place those two disagree (see notes/chapter-03.md, §3.3).

#ifndef COIN_TRAY_H            // header guard (Chapter 2.12): include this once
#define COIN_TRAY_H

// ─────────────────────────────────────────────────────────────────────────
// Coin denominations, in CENTS. These constants are correct — trust them.
// (constexpr constants are safe to DEFINE in a header: each translation unit
//  gets its own private copy — unlike functions, which need one .cpp home.)
// ─────────────────────────────────────────────────────────────────────────
constexpr int kPenny   { 1 };
constexpr int kNickel  { 5 };
constexpr int kDime    { 10 };
constexpr int kQuarter { 25 };

// A roll of quarters from the bank holds exactly 40 quarters ($10.00).
constexpr int kQuartersPerRoll { 40 };

// ─────────────────────────────────────────────────────────────────────────
// The auditor's functions. EXPECTED behavior is spelled out per function.
// ─────────────────────────────────────────────────────────────────────────

// EXPECTED: the value, in cents, of `count` coins each worth `coinValue` cents.
//   coinWorth(3, kQuarter) -> 75   (three quarters = 75 cents)
//   coinWorth(0, kDime)    -> 0
int coinWorth(int count, int coinValue);

// EXPECTED: the total value, in cents, of a tray holding the given coin counts.
//   = quarters*25 + dimes*10 + nickels*5 + pennies*1
//   trayValue(2, 1, 0, 3) -> 50 + 10 + 0 + 3 = 63
int trayValue(int quarters, int dimes, int nickels, int pennies);

// EXPECTED: cents left to dispense as change after the customer overpays.
//   = amountPaid - price   (both already in cents; caller guarantees paid >= price)
//   changeOwed(100, 75) -> 25
int changeOwed(int amountPaid, int price);

// EXPECTED: the value, in cents, of `rolls` full rolls of quarters.
//   = rolls * kQuartersPerRoll * kQuarter
//   rollsValue(1) -> 1 * 40 * 25 = 1000   ($10.00)
int rollsValue(int rolls);

// EXPECTED: total cents the machine holds = loose-tray value + rolled quarters.
//   = trayValue(quarters, dimes, nickels, pennies) + rollsValue(rolls)
// This is the top-level audit number printed on the maintenance ticket.
int machineCents(int rolls, int quarters, int dimes, int nickels, int pennies);

#endif // COIN_TRAY_H
