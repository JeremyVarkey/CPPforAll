// Chapter 8 — Control Flow · Project: The Number-Guessing Engine   (GRADER)
// ─────────────────────────────────────────────────────────────────────────────
// A tiny no-framework unit-test harness (same style as the drills/CLAUDE.md
// spec). It includes ../engine.h and calls each engine function across MANY
// inputs — fully deterministic, because the engine has no <random> and no I/O.
// Each CHECK that fails prints its expression and line number. Any failure ->
// non-zero exit -> `make test` is RED.
//
// The Makefile links this file against starter/engine.cpp (your code) for
// `make test`, and against solution/engine.cpp for `make test-solution`.

#include <iostream>
#include <string>
#include "../engine.h"

static int fails = 0;

// CHECK: assert a boolean condition; on failure, report what and where.
#define CHECK(cond) \
    do { if(!(cond)){ std::cerr << "FAIL: " #cond "  @line " << __LINE__ << "\n"; ++fails; } } while(0)

int main()
{
    // ── Task 1: judgeGuess — the if/else chain (sign = direction) ────────────
    CHECK(judgeGuess(50, 30) == -1);   // 30 < 50 -> too low
    CHECK(judgeGuess(50, 80) == +1);   // 80 > 50 -> too high
    CHECK(judgeGuess(50, 50) ==  0);   // equal    -> correct
    CHECK(judgeGuess(50, 49) == -1);   // boundary just below
    CHECK(judgeGuess(50, 51) == +1);   // boundary just above
    CHECK(judgeGuess(0, 0)   ==  0);   // edge: secret 0
    CHECK(judgeGuess(-5, -9) == -1);   // edge: negatives still order correctly
    CHECK(judgeGuess(-5, -1) == +1);

    // ── Task 2: hintText — the switch (incl. default) ────────────────────────
    CHECK(hintText(-1) == "too low");
    CHECK(hintText(0)  == "correct");
    CHECK(hintText(1)  == "too high");
    CHECK(hintText(2)  == "invalid");    // edge: default catches out-of-range…
    CHECK(hintText(-2) == "invalid");    // …on both sides
    CHECK(hintText(99) == "invalid");
    // The engine round-trips: judging then labelling should read naturally.
    CHECK(hintText(judgeGuess(7, 4)) == "too low");
    CHECK(hintText(judgeGuess(7, 7)) == "correct");

    // ── Task 3: sumOfDigits — counting loop with % and / ─────────────────────
    CHECK(sumOfDigits(0)    == 0);     // edge: loop runs zero times
    CHECK(sumOfDigits(5)    == 5);     // single digit
    CHECK(sumOfDigits(123)  == 6);     // 1+2+3
    CHECK(sumOfDigits(99)   == 18);
    CHECK(sumOfDigits(1000) == 1);     // zeros contribute nothing
    CHECK(sumOfDigits(-123) == 6);     // edge: negative uses |n|, same as 123
    CHECK(sumOfDigits(-7)   == 7);     // edge: single negative digit
    CHECK(sumOfDigits(2147483647) == 46);  // edge: INT_MAX, no overflow in the sum

    // ── Task 4: countdownString — while loop that builds a string ────────────
    CHECK(countdownString(3) == "3..2..1..liftoff");
    CHECK(countdownString(1) == "1..liftoff");
    CHECK(countdownString(2) == "2..1..liftoff");
    CHECK(countdownString(0) == "liftoff");      // edge: body runs zero times
    CHECK(countdownString(-4) == "liftoff");     // edge: negative start, still just finale
    CHECK(countdownString(5) == "5..4..3..2..1..liftoff");

    // ── Task 5: promptsUntilValid — do-while validation loop ─────────────────
    CHECK(promptsUntilValid("4", 1, 6)   == 1);  // valid on the first prompt
    CHECK(promptsUntilValid("093", 1, 6) == 3);  // 0 bad, 9 bad, 3 good -> 3
    CHECK(promptsUntilValid("80261", 1, 6) == 3);// 8 bad, 0 bad, 2 good -> 3
    CHECK(promptsUntilValid("7", 1, 6)   == 1);  // edge: only value, invalid -> still 1 consumed
    CHECK(promptsUntilValid("999", 1, 6) == 3);  // edge: none valid -> total consumed (3)
    CHECK(promptsUntilValid("16", 1, 6)  == 1);  // first already valid -> stop at 1 (do-while body once)
    CHECK(promptsUntilValid("5", 5, 5)   == 1);  // edge: single-value range, hit it

    // ── Task 6: playRound — the capstone (for + if + break + continue) ───────
    CHECK(playRound(5, "375", 3) == 3);  // guesses 3,7,5 -> 3rd attempt wins
    CHECK(playRound(5, "5",   3) == 1);  // first counted attempt wins
    CHECK(playRound(5, "12",  3) == 0);  // never guesses 5 within budget -> loss
    CHECK(playRound(5, "405", 3) == 2);  // edge: 0 is a skipped sentinel; 4 misses, 5 wins on attempt 2
    CHECK(playRound(5, "0005", 3) == 1); // edge: three skips, then 5 wins on counted attempt 1
    CHECK(playRound(5, "125", 2) == 0);  // edge: budget 2 spent before the winning 5 is judged
    CHECK(playRound(9, "123459", 6) == 6);// six real guesses; the winning 9 is the 6th counted attempt
    CHECK(playRound(9, "123459", 5) == 0);// same guesses, budget 5 -> 9 is out of budget -> loss
    CHECK(playRound(3, "13", 5) == 2);   // 1 misses, 3 wins on the 2nd counted attempt

    if (!fails)
        std::cout << "PASS ✅  all engine checks passed.\n";
    else
        std::cerr << "\nFAIL ❌  " << fails << " check(s) failed — fix the TASK blocks in engine.cpp.\n";

    return fails ? 1 : 0;
}
