// Chapter 8 — Control Flow · Project: The Number-Guessing Engine   (DRIVER / main)
// ─────────────────────────────────────────────────────────────────────────────
// This is the PLAYABLE game that wraps your pure engine. It is NOT graded and you
// do NOT need to edit it — it's here so you can actually PLAY once your engine
// works (`make run`), and so you can see the chapter's <random> tools in action.
//
// Why is all of this OUT of engine.cpp? Because <random> and std::cin make code
// non-deterministic — you can't unit-test "did the RNG pick 42?". So we keep the
// randomness and the keyboard HERE, in main, and keep the decision logic in the
// pure engine the grader hammers. That separation is the whole lesson (and it's
// exactly how you'd make a fuzzer's logic testable in CS6340).
//
//   make run     build the starter engine + this driver, then play
//
// NOTE: until you implement starter/engine.cpp, the game "works" but plays badly
// (every guess is judged "correct"); finish the engine and it becomes a real game.

#include <iostream>   // std::cin, std::cout   (kept OUT of the engine on purpose)
#include <random>     // std::mt19937, std::uniform_int_distribution (8.13 / 8.14)
#include "../engine.h"

int main()
{
    // ── Set up the random number generator (Chapter 8.13–8.14) ───────────────
    // An ENGINE produces raw pseudo-random bits; a DISTRIBUTION maps them into a
    // useful range. Seed the engine ONCE (std::random_device gives a varying
    // seed) and then reuse it — re-seeding inside a loop is the classic bug that
    // makes every "random" number come out the same.
    std::random_device rd;
    std::mt19937 rng { rd() };                       // seed once, reuse forever
    std::uniform_int_distribution<int> secretDist { 1, 9 };  // pick a secret 1..9
    std::uniform_int_distribution<int> guessDist  { 1, 9 };

    const int secret { secretDist(rng) };            // the number to guess (hidden)
    const int maxAttempts { 5 };

    std::cout << "Guessing engine online. I'm thinking of a number from 1 to 9.\n"
              << "Type a guess (1-9) each round, or 'r' to let me guess randomly,\n"
              << "or 'q' to quit.\n";

    int attempts { 0 };
    while (attempts < maxAttempts)
    {
        std::cout << "\nAttempt " << (attempts + 1) << " of " << maxAttempts << " > ";

        char command {};
        if (!(std::cin >> command))     // EOF / closed input -> leave the loop
            break;

        int guess { 0 };
        if (command == 'q')
        {
            std::cout << "Bye! The number was " << secret << ".\n";
            return 0;
        }
        else if (command == 'r')
        {
            guess = guessDist(rng);                  // let the RNG pick this guess
            std::cout << "  (engine rolls " << guess << ")\n";
        }
        else if (command >= '1' && command <= '9')
        {
            guess = command - '0';                   // the typed digit as a number
        }
        else
        {
            std::cout << "  Please enter 1-9, 'r', or 'q'.\n";
            continue;                                // don't spend an attempt
        }

        ++attempts;

        // Everything below routes through the PURE engine — no game logic lives
        // in main itself. judgeGuess decides the direction; hintText labels it.
        const int verdict { judgeGuess(secret, guess) };
        std::cout << "  " << guess << " is " << hintText(verdict) << ".\n";

        if (verdict == 0)
        {
            std::cout << "You got it in " << attempts << " attempt(s)! "
                      << "(digit sum of the secret is " << sumOfDigits(secret) << ")\n";
            return 0;
        }
    }

    // Out of attempts: a tiny countdown flourish, again from the pure engine.
    std::cout << "\nOut of attempts. " << countdownString(3) << "!\n"
              << "The number was " << secret << ".\n";
    return 0;
}
