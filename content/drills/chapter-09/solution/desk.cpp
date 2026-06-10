// Chapter 9 — Error Handling · Project: The Robust Input Desk   (REFERENCE SOLUTION)
// ─────────────────────────────────────────────────────────────────────────────
// One complete, correct, warning-clean implementation. Peek only after you've
// taken a real swing at starter/desk.cpp — the learning is in handling the bad
// input yourself, then comparing.
//
// Notice the two distinct error tools, used for two distinct jobs (notes 9.6):
//   • assert(...)      — for conditions that are IMPOSSIBLE in correct code
//                        (a caller broke a precondition = a programmer bug).
//   • runtime handling — for conditions that DO happen in normal operation
//                        (a user typed letters = expected reality, recover).

#include "../desk.h"

#include <cassert>   // assert                     (Task 2, 4, 6)  — notes 9.6
#include <iostream>  // std::cin / std::cout       (Task 6)        — notes 9.5
#include <limits>    // std::numeric_limits        (Task 6 ignore) — notes 9.5

// TASK 1 — range validation.
// The simplest validator: a value is acceptable iff it sits inside the allowed
// closed interval. Both bounds are INCLUSIVE, so use <= / >= (an off-by-one here
// would wrongly reject kMaxQuantity itself — notes 9.3 boundary errors).
bool isValidQuantity(int quantity)
{
    return (quantity >= kMinQuantity) && (quantity <= kMaxQuantity);
}

// TASK 2 — clamp with a precondition assertion.
// lo > hi is not a user mistake the desk should "handle" — it means whoever
// CALLED clampToRange passed nonsense, i.e. a bug in the program. assert documents
// that impossible state: in a debug build it aborts with a clear diagnostic; in a
// release build (NDEBUG) it compiles out. The string literal is always true, so it
// doesn't change the logic — it just rides along into the failure message (9.6).
int clampToRange(int value, int lo, int hi)
{
    assert(lo <= hi && "clampToRange precondition: lo must be <= hi");

    if (value < lo)
        return lo;
    if (value > hi)
        return hi;

    return value;   // already in range — return UNCHANGED (the easy-to-miss case)
}

// TASK 3 — parse a keystroke into a menu choice, with a sentinel for bad input.
// '1'..'4' map to 1..4; everything else returns kInvalidChoice. Because the menu
// digits are CONSECUTIVE characters, (key - '0') converts a digit char to its int
// value — but ONLY after we've confirmed key is one of our menu digits. Returning
// a sentinel (not aborting, not throwing) lets the caller decide what to do, which
// is the right call for ordinary "the user typed something odd" input (9.4).
int parseMenuChoice(char key)
{
    if (key >= '1' && key <= '0' + kMenuItemCount)   // i.e. '1'..'4'
        return key - '0';

    return kInvalidChoice;   // any other char: out-of-band "no valid choice"
}

// TASK 4 — stock lookup guarded by a compile-time invariant.
// The lookup table must have exactly one entry per menu item. static_assert
// proves that at COMPILE time (no runtime cost, can't be disabled by NDEBUG —
// notes 9.6): if someone later edits the table or changes kMenuItemCount, the
// build breaks loudly instead of reading past the end at runtime.
//
// itemNumber is the CALLER's responsibility to keep in 1..kMenuItemCount, so an
// out-of-range value is a programmer bug -> assert (a precondition), not handling.
bool isInStock(int itemNumber)
{
    // inStock[i] describes menu item (i + 1). Item 3 is sold out.
    // (A fixed C-style array — formally Chapter 17 — used here as the natural
    //  table for static_assert to guard.)
    constexpr bool inStock[] { true, true, false, true };
    static_assert(sizeof(inStock) / sizeof(inStock[0]) == kMenuItemCount,
                  "stock table must have exactly kMenuItemCount entries");

    assert(itemNumber >= 1 && itemNumber <= kMenuItemCount
           && "isInStock precondition: itemNumber must be a valid menu number");

    // Shift the 1-based menu number to a 0-based array index.
    return inStock[itemNumber - 1];
}

// TASK 5 — strict text validation (read-then-validate, notes 9.5).
// "Digits only" means: at least one character, and EVERY character in '0'..'9'.
// We compare the raw char against the digit range rather than std::isdigit so the
// rule is explicit and has no locale/sign-extension footguns. An empty view has
// no digits, so it must return false (the easy-to-forget boundary — 9.2 category
// testing lists "empty string" first for a reason).
bool digitsOnly(std::string_view text)
{
    if (text.empty())
        return false;

    for (char c : text)            // range-for (a preview — formally Ch 16.8; an index loop works too)
    {
        if (c < '0' || c > '9')    // any non-digit disqualifies the whole token
            return false;
    }

    return true;
}

// TASK 6 — the capstone: robustly read an int in [min, max], retrying on bad input.
// This is the complete Chapter 9.5 recovery loop. Walk through what can go wrong
// and how each case is handled — none of them may crash or spin forever:
//
//   "abc"   -> extraction FAILS; std::cin goes into a fail state. We must clear()
//              it and discard the buffered bad characters, else every future read
//              fails instantly (an accidental infinite loop — notes 9.3/9.5).
//   "12x"   -> extraction SUCCEEDS reading 12 and leaves "x\n" in the buffer. We
//              discard that leftover so it can't corrupt the next prompt.
//   "999"   -> extraction succeeds but the value is OUT OF RANGE: syntactically
//              valid, semantically rejected. Re-prompt.
//   ""/EOF  -> if the input stream closes (end-of-file), clearing and retrying
//              would loop forever, so we stop and return min as a safe fallback.
int readIntInRange(int min, int max)
{
    assert(min <= max && "readIntInRange precondition: min must be <= max");

    while (true)   // retry loop — owned here because input collection owns the policy (9.4)
    {
        std::cout << "Enter a number [" << min << ", " << max << "]: ";

        int value {};
        std::cin >> value;

        // If the stream has hit end-of-file, no more input is coming. Clearing and
        // retrying here would spin forever (9.5 EOF warning). Bail out safely.
        if (std::cin.eof())
        {
            std::cout << "Input closed; using " << min << ".\n";
            return min;
        }

        // Snapshot whether extraction succeeded BEFORE we clear the state. Then
        // ALWAYS clear() and discard the rest of the line — this resets a failed
        // stream AND throws away leftover junk like the "x\n" after "12x".
        const bool extracted { static_cast<bool>(std::cin) };
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        if (!extracted)
        {
            std::cout << "That was not a whole number. Try again.\n";
            continue;   // back to the prompt
        }

        if (value < min || value > max)
        {
            std::cout << "That number is out of range. Try again.\n";
            continue;
        }

        return value;   // happy path: a valid, in-range integer
    }
}
