// Chapter 9 — Error Handling · Project: The Robust Input Desk
// ─────────────────────────────────────────────────────────────────────────────
// This header is the CONTRACT between you and the grader. It declares every
// function you must implement plus the named constants the desk runs on. DO NOT
// EDIT THIS FILE — the grader (tests/tests.cpp) includes it, and so do BOTH
// starter/desk.cpp (yours) and solution/desk.cpp (the reference). Change a
// signature here and nothing links.
//
// THE BIG IDEA (Chapter 9): a program that compiled is not a program that is
// correct. Users — and fuzzers — type garbage: letters where you wanted a
// number, values out of range, blank lines, trailing junk, end-of-file. A
// ROBUST program decides what is acceptable and handles everything else
// gracefully: no crash, a clear signal, a chance to recover.
//
// You will build the "input desk" of an order kiosk as a small library of PURE,
// TESTABLE functions (Tasks 1–5) plus ONE interactive recovery routine that
// survives bad std::cin input (Task 6).
//
//   • Pure validators/parsers  -> unit-tested with good AND malicious inputs.
//   • assert / static_assert   -> document impossible states (programmer bugs).
//   • readIntInRange           -> the cin.fail()/clear()/ignore() recovery loop,
//                                 tested by FEEDING it hostile input on stdin.
//
// Header guard (Chapter 2): stops this file being pasted in twice per build.
#ifndef DESK_H
#define DESK_H

#include <string_view>   // std::string_view — cheap, non-owning text view (Chapter 5)

// ─── The desk's configuration (named constants — Chapter 5) ──────────────────
// Real programs scatter "magic numbers" everywhere; named constexpr constants
// keep the rules in ONE place so validators and tests can't drift apart.
inline constexpr int kMinQuantity { 1 };    // can't order zero or fewer items
inline constexpr int kMaxQuantity { 99 };   // per-line order cap

// The kiosk menu has exactly this many items, numbered 1..kMenuItemCount.
inline constexpr int kMenuItemCount { 4 };

// Sentinel value (notes 9.4): a result OUTSIDE the normal set that means "no
// valid value." parseMenuChoice returns this for any unrecognized keystroke.
// It is deliberately not a real menu number (1..kMenuItemCount), so a caller
// can always tell "invalid" apart from a genuine choice.
inline constexpr int kInvalidChoice { -1 };

// ─── TASK 1 ──────────────────────────────────────────────────────────────────
// Range validation. Return true iff `quantity` is an orderable amount, i.e. it
// lies in [kMinQuantity, kMaxQuantity] INCLUSIVE. This is the simplest shape of
// input validation: "is this value even allowed?"
bool isValidQuantity(int quantity);

// ─── TASK 2 ──────────────────────────────────────────────────────────────────
// Clamp `value` into [lo, hi] (values below lo become lo; above hi become hi;
// in-range values are returned unchanged). PRECONDITION: lo <= hi. Document that
// impossible-state with assert(lo <= hi) — a caller passing lo > hi is a
// PROGRAMMER BUG, not user error, so an assertion (not a runtime check) is right.
int clampToRange(int value, int lo, int hi);

// ─── TASK 3 ──────────────────────────────────────────────────────────────────
// Parse a single keystroke into a menu choice. Map the digit characters
// '1'..'4' to the ints 1..4; return the SENTINEL kInvalidChoice for ANYTHING
// else (other digits, letters, punctuation, whitespace). This is error handling
// by sentinel: the function never crashes and never lies — bad input gets the
// out-of-band signal so the caller can react.
int parseMenuChoice(char key);

// ─── TASK 4 ──────────────────────────────────────────────────────────────────
// Stock lookup guarded by a COMPILE-TIME invariant. `itemNumber` is expected to
// be a valid menu number (1..kMenuItemCount) — the caller guarantees that, so an
// out-of-range itemNumber is a programmer bug: assert it. Inside, decide
// in-stock from this fixed table (item 3 is sold out):
//     item 1 -> in stock      item 3 -> OUT of stock
//     item 2 -> in stock      item 4 -> in stock
// The table has kMenuItemCount entries; a static_assert must prove that at
// COMPILE time so the table and the menu size can never silently disagree.
bool isInStock(int itemNumber);

// ─── TASK 5 ──────────────────────────────────────────────────────────────────
// Text validation: return true iff `text` is NON-EMPTY and EVERY character is a
// decimal digit '0'..'9'. (Empty text is NOT digits-only.) This is the
// "read a whole token, then validate it yourself" approach from notes 9.5 —
// stricter than letting operator>> stop at the first bad char. Category testing
// territory: empty, normal digits, a leading space, an embedded letter, signs.
bool digitsOnly(std::string_view text);

// ─── TASK 6 ──────────────────────────────────────────────────────────────────
// THE CAPSTONE. Robustly read one integer in [min, max] from std::cin, RETRYING
// until the user supplies a valid one. This is the full Chapter 9.5 recovery
// pattern: detect failed extraction, std::cin.clear() the error state, then
// ignore the rest of the line, and ALSO reject syntactically-valid-but-
// out-of-range numbers. It must survive letters, blank lines, trailing junk
// (e.g. "12x"), and out-of-range values WITHOUT crashing or looping forever.
// PRECONDITION (programmer's responsibility): min <= max — assert it.
// Prompts/messages go to std::cout; the accepted value is returned.
int readIntInRange(int min, int max);

#endif // DESK_H
