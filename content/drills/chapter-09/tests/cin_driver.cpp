// Chapter 9 — Error Handling · Project: The Robust Input Desk   (GRADER — std::cin recovery)
// ─────────────────────────────────────────────────────────────────────────────
// readIntInRange (Task 6) reads std::cin, so it can't be checked by a normal
// CHECK(expr) call — we have to actually feed it input. This little driver does
// exactly that: it calls readIntInRange TWICE (proving the stream stays usable
// after the first recovery), then prints each accepted value on its own
// "ACCEPTED=<n>" line so the Makefile can verify the results, no matter how many
// retry/error messages the function printed in between.
//
// The Makefile (`cin-test` target) pipes a HOSTILE script into this program
// (tests/cin_input.txt). Traced against a correct readIntInRange(1, 99):
//
//   FIRST call must return 7:
//     abc      not a number          -> extraction fails -> clear+ignore, retry
//     200x     200 is out of range   -> rejected; the trailing "x" is discarded
//     7        valid                 -> ACCEPTED=7
//   SECOND call must return 50:
//     !!!      not a number          -> extraction fails -> clear+ignore, retry
//     0        below the min of 1    -> rejected
//     (blank)  whitespace only       -> >> skips it harmlessly, no crash
//     50       valid                 -> ACCEPTED=50
//
// Note the "200x" line does double duty: it proves an out-of-range value is
// rejected AND that leftover junk after the number is dropped (had we used a
// trailing-junk line whose number was IN range, like "12x", the reader would
// correctly accept 12 — so we deliberately make the prefix out of range here).
//
// If readIntInRange crashes, loops forever, or returns a wrong/out-of-range value,
// the two ACCEPTED lines won't read "7" then "50", and `make test` stays RED.
//
// The Makefile links this against starter/desk.cpp (your code) for `make test`,
// and against solution/desk.cpp for `make test-solution`.

#include <iostream>
#include "../desk.h"

int main()
{
    // Read two integers in [1, 99] through the robust reader. All the hostile
    // lines in between must be absorbed and recovered from without a crash.
    const int first  { readIntInRange(1, 99) };
    const int second { readIntInRange(1, 99) };

    // Machine-readable results on their own lines (normal output -> std::cout).
    // The leading '\n' guarantees the first ACCEPTED= line starts cleanly, even
    // though the last prompt readIntInRange printed had no trailing newline.
    std::cout << '\n';
    std::cout << "ACCEPTED=" << first << '\n';
    std::cout << "ACCEPTED=" << second << '\n';
    std::cout << "DONE\n";

    return 0;
}
