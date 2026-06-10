// telemetry.h — the public INTERFACE of the Pass Telemetry library.
//
// This header is COMPLETE. You don't edit it — you implement the bodies it
// promises (in starter/telemetry.cpp). Read it top-to-bottom: almost every idea
// in Chapter 7 is sitting right here, labelled.
//
// The story: imagine a (pretend) LLVM optimization pass. Each time it runs over a
// function it wants to (a) know how many times it has run so far this process,
// and (b) hand out a fresh unique integer ID for each counter it inserts. Both of
// those need state that SURVIVES BETWEEN CALLS — exactly what a `static` local
// gives you. The tests will call your functions several times and watch the
// numbers climb. (No loops — Chapter 8 — the *repetition lives in the tests*.)

#ifndef TELEMETRY_H            // ── header guard ──────────────────────────────
#define TELEMETRY_H            // include this file's contents at most once per TU

#include <string_view>        // std::string_view — for compile-time string labels

// ─────────────────────────────────────────────────────────────────────────────
//  Shared compile-time CONFIG, the C++17 way: `inline constexpr` in a namespace
//  in a header. (Notes 7.9 / 7.10.)
//
//  - `constexpr`  → a true compile-time constant (usable in static_assert, array
//                   sizes, etc.).
//  - `inline`     → "multiple identical definitions across translation units are
//                   allowed" — so EVERY .cpp that #includes this header shares ONE
//                   entity (external linkage), instead of each getting a private
//                   copy. This is the modern fix for sharing constants.
//
//  A plain `constexpr` global (without `inline`) has INTERNAL linkage — fine, but
//  each TU gets its own copy. `inline constexpr` is the preferred shared form.
// ─────────────────────────────────────────────────────────────────────────────
namespace telemetry::config
{
    // The first unique counter ID we hand out. IDs start here and climb.
    inline constexpr int firstCounterId { 1000 };

    // A human-readable name for this pass, shared by every file that includes us.
    inline constexpr std::string_view passName { "cs6340-counter-pass" };

    // Compile-time sanity check (a PREVIEW of Chapter 9's static_assert, notes
    // 9.6): if someone "fixes" the base ID to a negative number, refuse to
    // compile rather than hand out nonsense IDs later.
    // static_assert is checked by the COMPILER — a failure is a build error, not a
    // runtime surprise. This line needs `firstCounterId` to be a constexpr value;
    // that's exactly why config constants are constexpr.
    static_assert(firstCounterId >= 0, "firstCounterId must be non-negative");
}

// ─────────────────────────────────────────────────────────────────────────────
//  The public API lives in `namespace telemetry`. Functions have EXTERNAL linkage
//  by default, so these declarations in the header connect to the definitions you
//  write in telemetry.cpp — the linker wires the two translation units together.
//  (Notes 7.7.) C++17 lets us spell the nested namespace compactly with `::`.
// ─────────────────────────────────────────────────────────────────────────────
namespace telemetry
{
    // TASK 1 — return config::firstCounterId. Trivial, but it makes you reach into
    // a nested namespace with the scope-resolution operator `::`.
    int configuredFirstId();

    // TASK 3 — the heart of the chapter. Increments a STATIC LOCAL counter and
    // returns the new running total. Call it once → 1. Call it again → 2. The
    // count must PERSIST across calls (static duration), even though the variable's
    // NAME is visible only inside the function body (block scope).
    int recordRun();

    // TASK 3 (companion) — read the current run count WITHOUT changing it.
    int runCount();

    // TASK 4 — a unique-ID dispenser. Returns config::firstCounterId on the first
    // call, then firstCounterId+1, +2, ... Each ID is handed out exactly once.
    int nextCounterId();

    // TASK 5 — for an active run (recordRun() already called at least once), report
    // whether this is the pass's "first run" (run #1) using the internal helper.
    // Returns false before any run has been recorded.
    bool isFirstRun();
}

// A deliberately-named GLOBAL variable in the GLOBAL namespace, with external
// linkage. The library's `g_buildTag` and a same-named local inside one of your
// functions will collide by name — Task 2 shows you how `::` reaches past a
// shadowing local to the global. (Notes 7.5 / 7.2.) Defined once in telemetry.cpp.
extern int g_buildTag;

// TASK 2 — lives in the GLOBAL namespace (no `telemetry::`). It intentionally
// declares a LOCAL variable named `g_buildTag` that SHADOWS the global above, then
// must return the GLOBAL one via `::g_buildTag`. Proves you understand shadowing.
int globalBuildTag();

#endif // TELEMETRY_H
