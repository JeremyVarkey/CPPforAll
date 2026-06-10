// Chapter 8 — Control Flow · Project: The Number-Guessing Engine   (STARTER)
// ─────────────────────────────────────────────────────────────────────────────
// Fill in the six TASK blocks below. Each maps 1:1 to a task in the README and
// to a declaration in ../engine.h. The bodies currently return PLACEHOLDERS so
// the file compiles immediately — that's why `make test` is RED right now. Your
// job is to turn it GREEN by wiring up the real control flow.
//
//     make build         compile your code (should already work)
//     make test          grade it          (RED until you fill these in)
//     make solution      run the grader against the reference if you get stuck
//
// Everything here is PURE: arguments in, value out. No <random>, no std::cin/cout
// — that machinery lives in main.cpp and is NOT graded. Keep it that way so your
// engine stays deterministic and testable.

#include "../engine.h"

// ─── TASK 1: judge a guess with an if / else if / else chain ─────────────────
// Return -1 if guess < secret (too low), +1 if guess > secret (too high), or 0
// if they're equal (correct). The three cases are mutually exclusive, so use ONE
// if / else if / else chain (not three independent `if`s). No loops here.
//
//   >>> YOUR CODE HERE <<<
//
int judgeGuess(int /*secret*/, int /*guess*/)
{
    return 0;   // placeholder — always says "correct" (wrong for most inputs)
}
// ─────────────────────────────────────────────────────────────────────────────

// ─── TASK 2: turn a code into a label with a SWITCH ──────────────────────────
// switch on `code`:  -1 -> "too low",  0 -> "correct",  +1 -> "too high",
// anything else -> "invalid" (use `default`). Put a `break` after each case so
// you don't fall through into the next one. Please use `switch` here, not if/else
// — practicing the switch statement is the point of this task.
//
//   >>> YOUR CODE HERE <<<
//
std::string_view hintText(int /*code*/)
{
    return "invalid";   // placeholder
}
// ─────────────────────────────────────────────────────────────────────────────

// ─── TASK 3: digit sum with a loop, % and / ──────────────────────────────────
// Add up the decimal digits of `n` using a `while` (or `for`) loop. Peel the
// last digit with `n % 10`, then drop it with `n /= 10`, until nothing is left.
// Use the MAGNITUDE so negatives match: sumOfDigits(-123) == sumOfDigits(123).
// sumOfDigits(0) must be 0 (the loop body should run zero times).
//
//   >>> YOUR CODE HERE <<<
//
int sumOfDigits(int /*n*/)
{
    return 0;   // placeholder — correct only for n == 0
}
// ─────────────────────────────────────────────────────────────────────────────

// ─── TASK 4: build a countdown STRING with a while loop ──────────────────────
// Count DOWN from `start` to 1, appending ".." only BETWEEN numbers, then append
// "liftoff":  countdownString(3) == "3..2..1..liftoff",  countdownString(0) ==
// "liftoff". Build into a std::string (std::to_string(int) makes "3" from 3, and
// `out += "..";` appends). Return the string — do NOT print it.
//
//   >>> YOUR CODE HERE <<<
//
std::string countdownString(int /*start*/)
{
    return "liftoff";   // placeholder — only correct when start <= 0
}
// ─────────────────────────────────────────────────────────────────────────────

// ─── TASK 5: do-while validation loop ────────────────────────────────────────
// Walk the "typed" digit characters left-to-right with a `do { … } while (…)`
// loop. Convert each char to its value with `ch - '0'`. Count how many you
// consume until one lands IN RANGE [low, high]; return that 1-based count. If
// none are valid, return the total number consumed. A do-while fits because you
// must read at least ONE value before you can test it. Stop at the end of the
// string so you never read past it. `typed` is non-empty and all '0'..'9'.
//
//   >>> YOUR CODE HERE <<<
//
int promptsUntilValid(std::string_view /*typed*/, int /*low*/, int /*high*/)
{
    return 1;   // placeholder — pretends the first prompt is always valid
}
// ─────────────────────────────────────────────────────────────────────────────

// ─── TASK 6: play one bounded round (for + if + break + continue) ────────────
// Loop over the guess characters of `guesses` with a `for` loop. For each:
//   • Convert the char to its value: `guesses[i] - '0'`.
//   • If that value is 0, it's a "misclick" SENTINEL: `continue` WITHOUT counting.
//   • Otherwise it's a counted attempt (++attempts). If you've now exceeded
//     `maxAttempts`, `break` (the budget is spent — a loss).
//   • Judge it with judgeGuess; if the result is 0 (correct), return the 1-based
//     count of the COUNTED attempt that won.
// Return 0 if the player never guesses `secret` within the budget.
// Index a string_view with `guesses[i]`; its length is `guesses.size()` (a
// std::size_t — fine to compare an `i` of the same type against it).
//
//   >>> YOUR CODE HERE <<<
//
int playRound(int /*secret*/, std::string_view /*guesses*/, int /*maxAttempts*/)
{
    return 0;   // placeholder — always reports a loss
}
// ─────────────────────────────────────────────────────────────────────────────
