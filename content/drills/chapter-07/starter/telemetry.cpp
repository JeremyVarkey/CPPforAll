// starter/telemetry.cpp — YOU implement the Pass Telemetry library here.
//
// This file already COMPILES (every function returns a placeholder), but the
// placeholders are wrong, so `make test` is RED. Fill in the five TASK blocks to
// turn it GREEN. Build / grade with:
//     make            compile the starter library + a tiny demo
//     make test       grade your code (red until you implement the tasks)
//     make solution   run the reference if you get stuck
//
// This is the IMPLEMENTATION translation unit; it #includes its paired header so
// the compiler checks your definitions against the promised declarations.

#include "../telemetry.h"

// ─── TASK 2 (part A): define the global ────────────────────────────────────
// The header DECLARED `extern int g_buildTag;` — a promise that the object exists
// somewhere. Exactly ONE .cpp must DEFINE it. Define it here and initialize it to
// 6340. (Notes 7.7: non-const globals have external linkage; one definition only.)
//
//   >>> YOUR CODE HERE <<<
int g_buildTag {};   // <-- replace the {} (zero) with the real value 6340
// ───────────────────────────────────────────────────────────────────────────

// ─────────────────────────────────────────────────────────────────────────────
//  Internal-linkage helpers live in this UNNAMED (anonymous) namespace: visible in
//  THIS file only, invisible to the rest of the program. (Notes 7.6 / 7.14.)
//  s_runCount is the pass's private run-counter — namespace-scope, STATIC duration
//  (lives the whole program), INTERNAL linkage. It is provided for you; recordRun()
//  will bump it and runCount()/isFirstRun() will read it.
// ─────────────────────────────────────────────────────────────────────────────
namespace
{
    int s_runCount { 0 };

    // ─── TASK 5 (part A): internal helper ──────────────────────────────────
    // Return true exactly when `runNumber` is the FIRST run (i.e. equals 1).
    // It stays in this anonymous namespace on purpose: it is an implementation
    // detail, not part of the library's public interface.
    //
    //   >>> YOUR CODE HERE <<<
    bool isFirstRunNumber(int runNumber)
    {
        (void)runNumber;   // remove this line once you use the parameter
        return false;      // <-- replace with the real check
    }
    // ───────────────────────────────────────────────────────────────────────
}

namespace telemetry
{
    // ─── TASK 1: configuredFirstId ─────────────────────────────────────────
    // Return the shared config constant config::firstCounterId. Reach into the
    // nested namespace with the scope-resolution operator `::`. (Notes 7.2.)
    //
    //   >>> YOUR CODE HERE <<<
    int configuredFirstId()
    {
        return 0;   // <-- replace with config::firstCounterId
    }
    // ───────────────────────────────────────────────────────────────────────

    // ─── TASK 3: recordRun + runCount ──────────────────────────────────────
    // recordRun(): increment the shared s_runCount and return the NEW total. Since
    // s_runCount has static duration, the count must PERSIST across calls — call
    // once → 1, again → 2, again → 3. runCount(): return s_runCount WITHOUT
    // changing it. (Notes 7.11.)
    //
    //   >>> YOUR CODE HERE <<<
    int recordRun()
    {
        (void)s_runCount;   // placeholder: delete once you really use s_runCount
        return 0;           // <-- bump s_runCount, then return it
    }

    int runCount()
    {
        return 0;   // <-- return s_runCount (no mutation)
    }
    // ───────────────────────────────────────────────────────────────────────

    // ─── TASK 4: nextCounterId ─────────────────────────────────────────────
    // A unique-ID dispenser. Use a STATIC LOCAL initialized to config::firstCounterId
    // the first time control reaches it; then hand out firstCounterId, +1, +2, ...
    // (one per call, never repeating). Hint: `static int s_nextId { ... };` plus
    // post-increment `s_nextId++` returns the current value then advances. (Notes 7.11.)
    //
    //   >>> YOUR CODE HERE <<<
    int nextCounterId()
    {
        return 0;   // <-- use a static local; return then advance
    }
    // ───────────────────────────────────────────────────────────────────────

    // ─── TASK 5 (part B): isFirstRun ───────────────────────────────────────
    // Report whether the current run is the pass's first, by passing s_runCount to
    // your internal helper isFirstRunNumber(). Before any run is recorded
    // (s_runCount == 0) this must return false.
    //
    //   >>> YOUR CODE HERE <<<
    bool isFirstRun()
    {
        (void)isFirstRunNumber;   // placeholder: delete once you call the helper
        return false;             // <-- return isFirstRunNumber(s_runCount)
    }
    // ───────────────────────────────────────────────────────────────────────
}

// ─── TASK 2 (part B): shadowing + the `::` global form ──────────────────────
// This function lives in the GLOBAL namespace (note: NO `telemetry::`). Inside it,
// declare a LOCAL variable named `g_buildTag` (any int value) — it will SHADOW the
// global one. Then return the GLOBAL `g_buildTag` using `::g_buildTag`. The whole
// point: prove you can reach past a shadowing local to the global. (Notes 7.5 / 7.2.)
//
//   >>> YOUR CODE HERE <<<
int globalBuildTag()
{
    return 0;   // <-- declare a local g_buildTag that shadows, then return ::g_buildTag
}
// ───────────────────────────────────────────────────────────────────────────
