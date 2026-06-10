// tests/tests.cpp — the automated grader for the Pass Telemetry library.
//
// Tiny no-framework harness (same style as the other drills). It is linked against
// EITHER starter/telemetry.cpp (`make test`) or solution/telemetry.cpp
// (`make test-solution`) — see the Makefile.
//
// The chapter's big idea is STATIC DURATION: a static variable REMEMBERS its value
// between calls. We can't write a loop yet (Chapter 8), so we make persistence
// visible the honest way — by CALLING the functions several times in a row and
// asserting the numbers climb. If recordRun()/nextCounterId() used an ordinary
// (automatic) local, every call would reset and these checks would fail.

#include <iostream>
#include <string_view>
#include "../telemetry.h"

static int fails = 0;
#define CHECK(cond) do { if(!(cond)){ std::cerr << "FAIL: " #cond " @line " << __LINE__ << "\n"; ++fails; } } while(0)

int main()
{
    // ── Compile-time config contract (Task: header static_assert / inline constexpr).
    // These are checked by the COMPILER; if the constants were wrong this test file
    // would not even build. We mirror the key invariant here for documentation.
    static_assert(telemetry::config::firstCounterId >= 0,
                  "config base id must be non-negative");
    static_assert(telemetry::config::passName.size() > 0,
                  "pass name should not be empty");

    // ── TASK 1: configuredFirstId reaches the nested config constant via `::`.
    CHECK(telemetry::configuredFirstId() == 1000);
    // and it must actually equal the shared constant, not a hard-coded duplicate:
    CHECK(telemetry::configuredFirstId() == telemetry::config::firstCounterId);

    // ── TASK 2: shadowing — globalBuildTag() returns the GLOBAL ::g_buildTag (6340),
    // NOT the local that shadows it.
    CHECK(globalBuildTag() == 6340);
    CHECK(globalBuildTag() == g_buildTag);   // g_buildTag here is the global object

    // ── TASK 3 (+5): EDGE CASE — before any run is recorded, the static counter is
    // still at its initial 0, and "is this the first run?" is false (run #1 hasn't
    // happened yet). This catches the classic bug of seeding the counter at 1.
    CHECK(telemetry::runCount() == 0);
    CHECK(telemetry::isFirstRun() == false);

    // ── TASK 3: PERSISTENCE across calls. The repetition that a loop would do is
    // done here by hand, on purpose. Each call must return the NEW running total.
    CHECK(telemetry::recordRun() == 1);   // 1st run
    CHECK(telemetry::isFirstRun() == true);   // now run #1 → first run
    CHECK(telemetry::recordRun() == 2);   // 2nd run  (proves the value SURVIVED)
    CHECK(telemetry::recordRun() == 3);   // 3rd run
    CHECK(telemetry::isFirstRun() == false);  // no longer the first run

    // runCount() is a read-only peek: calling it must NOT advance the counter.
    CHECK(telemetry::runCount() == 3);
    CHECK(telemetry::runCount() == 3);    // called twice; still 3 → no mutation

    // ── TASK 4: the unique-ID dispenser is INDEPENDENT static state. Its first
    // output is the configured base, then it climbs by exactly 1 each call.
    int id0 { telemetry::nextCounterId() };
    int id1 { telemetry::nextCounterId() };
    int id2 { telemetry::nextCounterId() };
    CHECK(id0 == telemetry::config::firstCounterId);  // first ID == 1000
    CHECK(id1 == 1001);
    CHECK(id2 == 1002);

    // EDGE CASE — uniqueness: consecutive IDs must differ (never hand the same ID
    // out twice) and increase by exactly one.
    CHECK(id0 != id1);
    CHECK(id1 != id2);
    CHECK((id1 - id0) == 1);
    CHECK((id2 - id1) == 1);

    // The ID dispenser and the run counter are SEPARATE statics: dispensing IDs
    // must not have disturbed the run count (still 3 from above).
    CHECK(telemetry::runCount() == 3);

    if (!fails) std::cout << "PASS ✅ all checks\n";
    return fails ? 1 : 0;
}
