// Chapter 8 — Control Flow · Project: The Number-Guessing Engine  (REFERENCE SOLUTION)
// ─────────────────────────────────────────────────────────────────────────────
// One complete, correct, warning-clean implementation of ../engine.h. Peek only
// after you've taken a real swing at starter/engine.cpp — the learning is in
// wiring the control flow yourself, then comparing.
//
// Everything here is PURE: arguments in, value out. No <random>, no std::cin/cout.

#include "../engine.h"

// ─── TASK 1: if / else if / else chain ───────────────────────────────────────
// The three cases are mutually exclusive, so a CHAIN (not three separate ifs) is
// the right tool: the first branch that matches wins and the rest are skipped.
// The returned sign is the "which way to move" signal the player reads.
int judgeGuess(int secret, int guess)
{
    if (guess < secret)
        return -1;          // guess is below the secret -> too low
    else if (guess > secret)
        return +1;          // guess is above the secret -> too high
    else
        return 0;           // guess == secret -> correct
}

// ─── TASK 2: switch statement ────────────────────────────────────────────────
// One controlling expression (`code`) compared against discrete values — the
// textbook case for `switch`. We assign the label in each case and `break` so
// execution doesn't fall through into the next case; `default` catches anything
// that isn't a valid three-way code. (Assigning + breaking, rather than an early
// return, lets you SEE the `break` doing its job.)
std::string_view hintText(int code)
{
    std::string_view label {};

    switch (code)
    {
    case -1:
        label = "too low";
        break;              // stop here — do NOT fall into case 0
    case 0:
        label = "correct";
        break;
    case 1:
        label = "too high";
        break;
    default:
        label = "invalid";  // not a value judgeGuess can produce
        break;
    }

    return label;
}

// ─── TASK 3: a counting loop with % and / ────────────────────────────────────
// Peel digits off the right: `n % 10` is the last digit, `n / 10` drops it.
// We work on the MAGNITUDE so negatives match their positive twin; a single
// negation up front avoids a negative remainder from confusing the sum.
int sumOfDigits(int n)
{
    if (n < 0)
        n = -n;             // digit sum is defined on |n|

    int sum { 0 };
    while (n > 0)           // when n is 0 the body never runs -> sum stays 0
    {
        sum += n % 10;      // add the current last digit
        n /= 10;            // shift right by one decimal place
    }
    return sum;
}

// ─── TASK 4: while loop that BUILDS a string ─────────────────────────────────
// Count down from `start` to 1, appending the ".." separator only BETWEEN
// numbers. We special-case the very first append (no leading ".."); every later
// number gets a ".." in front of it. After the loop, tack on "liftoff".
std::string countdownString(int start)
{
    std::string out {};

    int n { start };
    while (n >= 1)          // start <= 0 -> body never runs -> just "liftoff"
    {
        if (n != start)
            out += "..";    // separator goes between counts, not before the first

        out += std::to_string(n);
        --n;                // count DOWN; this is the loop's exit story
    }

    if (!out.empty())
        out += "..";        // separator between the last number and the finale

    out += "liftoff";
    return out;
}

// ─── TASK 5: do-while validation loop ────────────────────────────────────────
// A do-while runs the body at least once THEN tests — exactly how "prompt, then
// validate" works. We always consume one "typed" digit before we can judge it.
// `count` is the 1-based number of the digit we're currently looking at.
int promptsUntilValid(std::string_view typed, int low, int high)
{
    int count { 0 };
    int value { 0 };

    do
    {
        // Read the next "typed" value: the digit char at position `count`, turned
        // into its int value. (count is 0-based as an index, but we bump it to a
        // 1-based prompt counter on the same line.)
        char ch { typed[static_cast<std::size_t>(count)] };
        value = ch - '0';   // '0'..'9' -> 0..9
        ++count;            // we have now consumed one more prompt
    }
    while ((value < low || value > high)            // keep going while out of range…
           && count < static_cast<int>(typed.size())); // …but never run past the end

    return count;
}

// ─── TASK 6: for loop + if + break + continue (the capstone) ─────────────────
// Walk the guess characters. The sentinel 0 is a "misclick": skip it WITHOUT
// counting (continue). Every other digit is a counted attempt — judge it; a 0
// from judgeGuess means a correct guess, so break and report which counted
// attempt won. Stop once the attempt budget is spent (a loss -> return 0).
int playRound(int secret, std::string_view guesses, int maxAttempts)
{
    int attempts { 0 };

    for (std::size_t i { 0 }; i < guesses.size(); ++i)
    {
        int guess { guesses[i] - '0' };   // digit char -> numeric guess

        if (guess == 0)
            continue;                     // sentinel: skip, do NOT count it

        ++attempts;                       // a real, counted attempt
        if (attempts > maxAttempts)
            break;                        // budget exhausted before this guess

        if (judgeGuess(secret, guess) == 0)
            return attempts;              // correct -> the winning attempt number
    }

    return 0;                             // never guessed it within the budget
}
