// solution/telemetry.cpp — REFERENCE implementation of the Pass Telemetry library.
//
// One correct way to do it. Peek only if you're stuck; you learn far more by
// turning `make test` from red to green on your own first.
//
// This is the IMPLEMENTATION translation unit. It #includes its own header so the
// compiler can check every definition matches the promised declaration.

#include "../telemetry.h"

// ─────────────────────────────────────────────────────────────────────────────
//  TASK 2 (part A) — the GLOBAL definition.
//
//  The header DECLARED `extern int g_buildTag;` (a promise). Exactly one .cpp must
//  DEFINE it (create the object). This is that file. Non-const globals have
//  external linkage, so this single definition is the one symbol the whole program
//  shares. (Notes 7.7.)
// ─────────────────────────────────────────────────────────────────────────────
int g_buildTag { 6340 };

// ─────────────────────────────────────────────────────────────────────────────
//  Internal-linkage helpers, hidden in an UNNAMED (anonymous) namespace. Names in
//  here are usable in THIS translation unit only — other .cpp files can't see them
//  and can't collide with them. This is the modern replacement for file-scope
//  `static` helpers. (Notes 7.6 / 7.14.) In real LLVM passes, the pass's private
//  helpers live here too.
// ─────────────────────────────────────────────────────────────────────────────
namespace
{
    // The pass's private run-counter. It is a NAMESPACE-SCOPE variable with
    // STATIC duration (lives for the whole program) and INTERNAL linkage (this TU
    // only). recordRun() bumps it; runCount()/isFirstRun() read it.
    //
    // Could this be a static local inside recordRun()? Yes — but then runCount()
    // and isFirstRun() couldn't see it. Sharing it across a few functions in ONE
    // file, while keeping it invisible to the rest of the program, is exactly what
    // an internal-linkage namespace variable is for.
    int s_runCount { 0 };

    // Internal helper: is the given run number the very first run?
    bool isFirstRunNumber(int runNumber)
    {
        return runNumber == 1;
    }
}

namespace telemetry
{
    // ── TASK 1 ───────────────────────────────────────────────────────────────
    // Reach into the nested config namespace with `::`.
    int configuredFirstId()
    {
        return config::firstCounterId;
    }

    // ── TASK 3 ───────────────────────────────────────────────────────────────
    // Bump the shared internal counter and return the new total. Because
    // s_runCount has STATIC duration, its value survives between calls: the tests
    // call recordRun() repeatedly and watch 1, 2, 3, ... climb.
    int recordRun()
    {
        ++s_runCount;
        return s_runCount;
    }

    // Read-only peek at the counter — no mutation.
    int runCount()
    {
        return s_runCount;
    }

    // ── TASK 4 ───────────────────────────────────────────────────────────────
    // A unique-ID dispenser using a STATIC LOCAL. s_nextId is initialized ONCE
    // (the first time control reaches this line) to the configured base, then
    // retains its value between calls. Post-increment returns the current value
    // and then advances, so IDs come out as firstCounterId, +1, +2, ... with no
    // repeats. The NAME s_nextId is visible only inside this function (block
    // scope) — perfect encapsulation of remembered state.
    int nextCounterId()
    {
        static int s_nextId { config::firstCounterId };
        return s_nextId++;
    }

    // ── TASK 5 ───────────────────────────────────────────────────────────────
    // Use the internal-namespace helper to classify the current run. Before any
    // run is recorded, s_runCount is 0 and isFirstRunNumber(0) is false.
    bool isFirstRun()
    {
        return isFirstRunNumber(s_runCount);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  TASK 2 (part B) — the global-namespace function that demonstrates SHADOWING.
//
//  Inside this function we declare a LOCAL named `g_buildTag`. From here on, the
//  unqualified name `g_buildTag` means the LOCAL (it shadows / hides the global).
//  To reach the GLOBAL one we prefix it with `::` (the global-namespace form of
//  scope resolution). (Notes 7.2 / 7.5.)
// ─────────────────────────────────────────────────────────────────────────────
int globalBuildTag()
{
    int g_buildTag { -1 };   // a local that SHADOWS the global ::g_buildTag
    (void)g_buildTag;        // it exists only to prove the shadow; silence "unused"
    return ::g_buildTag;     // `::` skips the local and reaches the GLOBAL (6340)
}
