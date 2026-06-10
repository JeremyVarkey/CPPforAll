// demo.cpp — a tiny program that USES the Pass Telemetry library so you can watch
// the static-duration state with your own eyes. You don't edit this; it's the
// "consumer" translation unit. `make run` builds it against your starter library;
// `make solution` builds it against the reference. It's the same API the tests use,
// just printed instead of asserted.
//
// Notice there are NO loops here — we observe persistence by CALLING the functions
// several times in a row, exactly as the grader does.

#include <iostream>
#include "telemetry.h"

int main()
{
    std::cout << "pass name        : " << telemetry::config::passName << '\n';
    std::cout << "configured firstId: " << telemetry::configuredFirstId() << '\n';
    std::cout << "global build tag : " << globalBuildTag() << '\n';

    std::cout << "before any run   : runCount=" << telemetry::runCount()
              << " isFirstRun=" << std::boolalpha << telemetry::isFirstRun() << '\n';

    // Repeated calls — the static counter REMEMBERS between them.
    std::cout << "recordRun() -> " << telemetry::recordRun() << '\n';   // 1
    std::cout << "recordRun() -> " << telemetry::recordRun() << '\n';   // 2
    std::cout << "recordRun() -> " << telemetry::recordRun() << '\n';   // 3
    std::cout << "after 3 runs     : runCount=" << telemetry::runCount()
              << " isFirstRun=" << telemetry::isFirstRun() << '\n';

    // The unique-ID dispenser: a separate static, climbing from the configured base.
    std::cout << "nextCounterId() -> " << telemetry::nextCounterId() << '\n';  // 1000
    std::cout << "nextCounterId() -> " << telemetry::nextCounterId() << '\n';  // 1001
    std::cout << "nextCounterId() -> " << telemetry::nextCounterId() << '\n';  // 1002

    return 0;
}
