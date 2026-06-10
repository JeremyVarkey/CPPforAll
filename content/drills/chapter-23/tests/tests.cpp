// ============================================================================
//  tests/tests.cpp  —  Automated grader for the Garage Simulation  (Chapter 23)
// ----------------------------------------------------------------------------
//  A tiny no-framework unit-test harness (same style as drills/CLAUDE.md).
//  The Makefile compiles this file against EITHER starter/garage.cpp (`make test`)
//  OR solution/garage.cpp (`make test-solution`). The shared contract is the
//  header at ../garage.h.
//
//  WHAT THESE TESTS ASSERT:
//
//  The TRACE LOG is the spine of this grader. By appending events in ctors/dtors,
//  the code makes C++ lifetime rules OBSERVABLE at runtime. The tests check:
//
//    1. COMPOSITION order (23.2):
//         Engine constructs BEFORE the Car body, and destroys AFTER. The trace
//         sequence proves that the standard's member-init-order rule holds.
//
//    2. AGGREGATION isolation (23.3):
//         Destroying a Car does NOT destroy the Driver. After the Car scope ends,
//         the Driver is still alive (its name() is still readable).
//
//    3. DEPENDENCY scope (23.5):
//         tuneUp() increments Mechanic::tuneUpCount() via a temporary borrow.
//         The Mechanic is not stored; its lifetime is the caller's responsibility.
//
//    4. CONTAINER + std::initializer_list (23.6 / 23.7):
//         RouteList can be brace-initialised and supports add/at/size/clear.
//
//  EDGE CASES:
//    - Car with no driver (m_driver == nullptr).
//    - RouteList constructed empty vs. brace-initialised.
//    - Multiple tune-ups accumulate on one Mechanic (the dependency is reusable).
//    - Trace order across two Cars in the same scope (stack-like LIFO destruction).
// ============================================================================

#include <iostream>
#include <string>
#include <vector>
#include "../garage.h"

static int fails = 0;

// ── CHECK helpers ──────────────────────────────────────────────────────────────
#define CHECK(cond) \
    do { if(!(cond)){ std::cerr << "FAIL: " #cond "  @line " << __LINE__ << "\n"; ++fails; } } while(0)

// Check that the Nth event in trace::log() equals `expected`.
#define CHECK_TRACE(n, expected) \
    do { \
        auto& t = trace::log(); \
        if ((n) >= (int)t.size()) { \
            std::cerr << "FAIL: trace event " << (n) << " missing (log has " \
                      << t.size() << " entries)  @line " << __LINE__ << "\n"; \
            ++fails; \
        } else if (t[(n)] != (expected)) { \
            std::cerr << "FAIL: trace[" << (n) << "] == \"" << t[(n)] \
                      << "\"  want \"" << (expected) << "\"  @line " << __LINE__ << "\n"; \
            ++fails; \
        } \
    } while(0)

// Check trace log has exactly `n` events.
#define CHECK_TRACE_SIZE(n) \
    do { \
        if ((int)trace::log().size() != (n)) { \
            std::cerr << "FAIL: trace has " << trace::log().size() \
                      << " events, want " << (n) << "  @line " << __LINE__ << "\n"; \
            ++fails; \
        } \
    } while(0)

// ── Helper: dump the trace for debugging failed tests ──────────────────────────
static void dumpTrace()
{
    const auto& t = trace::log();
    std::cerr << "  [trace dump — " << t.size() << " events]\n";
    for (std::size_t i = 0; i < t.size(); ++i)
        std::cerr << "    [" << i << "] \"" << t[i] << "\"\n";
}

int main()
{
    // ══════════════════════════════════════════════════════════════════════════
    // SCENARIO A — Composition: member init/destroy ORDER
    // ══════════════════════════════════════════════════════════════════════════
    //  This is the CRITICAL test for Task 1 + Task 4.
    //
    //  Rule (23.2): members construct BEFORE the ctor body; destroy AFTER the dtor
    //  body, in REVERSE declaration order. Engine is declared FIRST in Car, so:
    //
    //    construct phase:  Engine ctor  → Car ctor body
    //    destruct phase:   Car dtor body → Engine dtor
    //
    //  We wrap the Car in a scope block to force its destructor to fire before we
    //  check the trace.
    {
        trace::clear();
        {
            Car car { "Sedan", "V8-Turbo" };

            // After construction: engine first, then car body.
            CHECK_TRACE(0, "Engine built: V8-Turbo");   // member init runs first
            CHECK_TRACE(1, "Car built: Sedan");         // ctor body runs after
            CHECK_TRACE_SIZE(2);
        }
        // Car went out of scope — destructor fired.
        // Car dtor body runs BEFORE m_engine's dtor.
        CHECK_TRACE(2, "Car destroyed: Sedan");         // dtor body fires first
        CHECK_TRACE(3, "Engine destroyed: V8-Turbo");   // member dtor fires after
        CHECK_TRACE_SIZE(4);
    }

    // ══════════════════════════════════════════════════════════════════════════
    // SCENARIO B — Aggregation: Driver survives Car destruction (Task 2 + Task 4)
    // ══════════════════════════════════════════════════════════════════════════
    //  KEY PROPERTY (23.3): The whole (Car) does NOT own the part (Driver).
    //  After the Car is destroyed, the Driver must still be usable.
    //  If Car's destructor called `delete m_driver`, this test would either
    //  crash or report use-after-free under a sanitizer.
    {
        trace::clear();

        {
            Driver alice { "Alice" };

            Car car { "Hatchback", "I4-Electric" };
            car.setDriver(&alice);

            // Verify aggregation is set up correctly.
            CHECK(car.driver() == &alice);
            CHECK(car.driver()->name() == "Alice");

            // Inner scope ends — Car destroyed BEFORE Alice goes out of scope.
            {
                // A tighter scope so Car dies while alice is still alive.
                Car innerCar { "Coupe", "V6-Hybrid" };
                innerCar.setDriver(&alice);
                // innerCar destroyed here:
                // "Engine built: V6-Hybrid" / "Car built: Coupe" happened;
                // now "Car destroyed: Coupe" / "Engine destroyed: V6-Hybrid"
            }
            // Alice is still alive — her destructor has NOT run yet.
            CHECK(alice.name() == "Alice");   // driver survives car destruction
        }
        // Now alice goes out of scope — Driver destroyed AFTER Car.
        // The trace should contain "Driver destroyed: Alice" at some point,
        // but NOT paired with a Car destruction (no delete m_driver in Car::~Car).
        bool carDestroyedCoupeSeen { false };
        bool driverDestroyedAfterCar { false };
        bool carDestroyedAtAll { false };
        for (std::size_t i = 0; i < trace::log().size(); ++i)
        {
            if (trace::log()[i] == "Car destroyed: Coupe")
            {
                carDestroyedCoupeSeen = true;
                carDestroyedAtAll = true;
            }
            if (trace::log()[i] == "Driver destroyed: Alice" && carDestroyedCoupeSeen)
                driverDestroyedAfterCar = true;
        }
        CHECK(carDestroyedAtAll);          // Car was definitely destroyed
        CHECK(driverDestroyedAfterCar);    // Driver lived on past the Car
    }

    // ══════════════════════════════════════════════════════════════════════════
    // SCENARIO C — Null driver (edge case for aggregation)
    // ══════════════════════════════════════════════════════════════════════════
    //  A Car with no driver set should return nullptr from driver().
    //  This verifies that m_driver is default-initialised to nullptr, not garbage.
    {
        trace::clear();
        Car loner { "Truck", "Diesel-V10" };
        CHECK(loner.driver() == nullptr);   // no driver assigned -> nullptr
    }

    // ══════════════════════════════════════════════════════════════════════════
    // SCENARIO D — Dependency: tuneUp() borrows and uses Mechanic (Task 3 + Task 4)
    // ══════════════════════════════════════════════════════════════════════════
    //  WHAT WE'RE TESTING (23.5):
    //    - Mechanic::tuneUpCount() increments each time Car::tuneUp() is called.
    //    - The Mechanic is NOT stored in Car — each call is an independent borrow.
    //    - Multiple Cars can use the same Mechanic (the dependency is re-entrant).
    //    - Trace contains "Tuned up by: <name>" entries in order.
    {
        trace::clear();

        Mechanic bob { "Bob" };
        CHECK(bob.tuneUpCount() == 0);   // fresh mechanic, no work done yet

        Car car1 { "Roadster", "EV-Motor" };
        Car car2 { "Minivan",  "V6-Base"  };

        car1.tuneUp(bob);
        CHECK(bob.tuneUpCount() == 1);   // one tune-up recorded

        car2.tuneUp(bob);
        CHECK(bob.tuneUpCount() == 2);   // second tune-up on same mechanic

        car1.tuneUp(bob);
        CHECK(bob.tuneUpCount() == 3);   // third — dependency is reusable

        // Verify the trace contains the expected "Tuned up by" events in order.
        // Count them (exact positions vary because of ctor events interleaved).
        int tuneUpCount { 0 };
        for (const auto& event : trace::log())
            if (event == "Tuned up by: Bob")
                ++tuneUpCount;
        CHECK(tuneUpCount == 3);

        // Mechanic is NOT stored in any Car — after tuneUp() returns, Car has
        // no reference to Bob. This is confirmed by the fact that Bob's
        // tuneUpCount() is only accessible through the original `bob` variable.
        CHECK(car1.driver() == nullptr);   // tuneUp does not set driver
    }

    // ══════════════════════════════════════════════════════════════════════════
    // SCENARIO E — RouteList: container + std::initializer_list (Task 5)
    // ══════════════════════════════════════════════════════════════════════════
    {
        // Default-constructed: empty.
        RouteList empty;
        CHECK(empty.size() == 0);

        // Brace-initialised: four waypoints (23.7 initializer_list ctor).
        // Guard at() calls with a size check so the starter does not abort.
        RouteList route { "Home", "Gas station", "Highway", "Destination" };
        CHECK(route.size() == 4);
        if (route.size() >= 4)
        {
            CHECK(route.at(0) == "Home");
            CHECK(route.at(1) == "Gas station");
            CHECK(route.at(2) == "Highway");
            CHECK(route.at(3) == "Destination");
        }

        // Single-element list.
        RouteList solo { "Airport" };
        CHECK(solo.size() == 1);
        if (solo.size() >= 1)
            CHECK(solo.at(0) == "Airport");

        // Add appends to an existing list.
        route.add("Parking lot");
        CHECK(route.size() == 5);
        if (route.size() >= 5)
            CHECK(route.at(4) == "Parking lot");

        // Clear empties the container.
        route.clear();
        CHECK(route.size() == 0);

        // Add to a cleared list still works.
        route.add("Restart");
        CHECK(route.size() == 1);
        if (route.size() >= 1)
            CHECK(route.at(0) == "Restart");

        // Empty-brace list: 0 elements (edge case — braces with zero args).
        // Note: RouteList r {}; would call the DEFAULT ctor, not initializer_list,
        // because the empty list matches the default ctor (23.7 brace-init rules).
        // But an explicit empty initializer_list<string_view> call works:
        RouteList emptyBrace(std::initializer_list<std::string_view>{});
        CHECK(emptyBrace.size() == 0);
    }

    // ══════════════════════════════════════════════════════════════════════════
    // SCENARIO F — Composition detail: engine model is accessible after build
    // ══════════════════════════════════════════════════════════════════════════
    {
        trace::clear();
        Car sports { "SportsCar", "Twin-Turbo-V12" };
        CHECK(sports.make() == "SportsCar");
        CHECK(sports.engineModel() == "Twin-Turbo-V12");
    }

    // ══════════════════════════════════════════════════════════════════════════
    // SCENARIO G — Two Cars: LIFO stack destruction order
    // ══════════════════════════════════════════════════════════════════════════
    //  When two Cars are declared in the same scope, they destroy in REVERSE
    //  order (stack / LIFO discipline). Each Car still has its Engine destroy
    //  AFTER the Car dtor body. Test the combined trace.
    {
        trace::clear();
        {
            Car first  { "First",  "Engine-A" };
            Car second { "Second", "Engine-B" };
            // Destruction order on scope exit: second then first (LIFO).
        }
        // Expected trace order:
        //   [0] "Engine built: Engine-A"    — first Car member init
        //   [1] "Car built: First"          — first Car ctor body
        //   [2] "Engine built: Engine-B"    — second Car member init
        //   [3] "Car built: Second"         — second Car ctor body
        //   [4] "Car destroyed: Second"     — second Car dtor body (LIFO)
        //   [5] "Engine destroyed: Engine-B"— second Car member dtor
        //   [6] "Car destroyed: First"      — first Car dtor body
        //   [7] "Engine destroyed: Engine-A"— first Car member dtor
        CHECK_TRACE(0, "Engine built: Engine-A");
        CHECK_TRACE(1, "Car built: First");
        CHECK_TRACE(2, "Engine built: Engine-B");
        CHECK_TRACE(3, "Car built: Second");
        CHECK_TRACE(4, "Car destroyed: Second");
        CHECK_TRACE(5, "Engine destroyed: Engine-B");
        CHECK_TRACE(6, "Car destroyed: First");
        CHECK_TRACE(7, "Engine destroyed: Engine-A");
        CHECK_TRACE_SIZE(8);
    }

    // ══════════════════════════════════════════════════════════════════════════
    // SCENARIO H — Driver trace integrity (Task 2 constructor/destructor strings)
    // ══════════════════════════════════════════════════════════════════════════
    {
        trace::clear();
        {
            Driver charlie { "Charlie" };
        }
        CHECK_TRACE(0, "Driver built: Charlie");
        CHECK_TRACE(1, "Driver destroyed: Charlie");
        CHECK_TRACE_SIZE(2);
    }

    // ══════════════════════════════════════════════════════════════════════════
    // SCENARIO I — Mechanic trace (Task 3 constructor/destructor strings)
    // ══════════════════════════════════════════════════════════════════════════
    {
        trace::clear();
        {
            Mechanic dave { "Dave" };
            CHECK_TRACE(0, "Mechanic on duty: Dave");
        }
        CHECK_TRACE(1, "Mechanic off duty: Dave");
        CHECK_TRACE_SIZE(2);
    }

    // ══════════════════════════════════════════════════════════════════════════
    // Summary
    // ══════════════════════════════════════════════════════════════════════════
    if (!fails)
        std::cout << "PASS \xe2\x9c\x85  all garage checks passed.\n";
    else
    {
        if (fails > 0) dumpTrace();
        std::cerr << "\nFAIL \xe2\x9d\x8c  " << fails
                  << " check(s) failed — fix the TASK blocks in garage.cpp.\n";
    }
    return fails ? 1 : 0;
}
