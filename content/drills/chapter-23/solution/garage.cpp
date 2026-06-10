// ============================================================================
//  solution/garage.cpp  —  REFERENCE IMPLEMENTATION  (Chapter 23)
// ----------------------------------------------------------------------------
//  This is the COMPLETE, correct implementation of the Garage Simulation.
//  Read it after you have attempted the exercise yourself — or consult it if
//  you are stuck. Every TASK is richly commented with the WHY, not just the how.
//
//  CENTRAL LESSON MADE PHYSICAL:
//      The trace log in every constructor and destructor below is not decoration —
//      it is *evidence*. When the tests check trace::log() contents and ORDER,
//      they are asserting C++ lifetime rules that you cannot see in static code:
//
//      (a) COMPOSITION (23.2):
//          Engine constructs BEFORE the Car constructor body runs, and destroys
//          AFTER the Car destructor body. This is the standard's member-init-order
//          rule. The trace makes the ordering physical and testable.
//
//      (b) AGGREGATION (23.3):
//          Destroying a Car does NOT destroy the Driver. After the Car goes out of
//          scope, the Driver still exists. The test verifies this by reading the
//          Driver's name after the Car is gone.
//
//      (c) DEPENDENCY (23.5):
//          Mechanic is used inside tuneUp() but NOT stored. The Mechanic's lifetime
//          is controlled by the caller, not by the Car.
//
//      (d) CONTAINER + std::initializer_list (23.6, 23.7):
//          RouteList is a value container — it owns its waypoints. The braced-init
//          syntax {A, B, C} works because of the initializer_list constructor.
//
//  CS6340 / LLVM tie-in:
//      A typical analysis pass uses all four relationships in one file:
//          std::string m_passName;              // composition
//          std::vector<llvm::Instruction*> m_candidates; // aggregation (ptrs)
//          void analyze(const llvm::Function& F); // dependency (parameter)
//          llvm::Module* m_module;              // association (non-owning ptr)
//      Recognizing the pattern behind the syntax is what lets you read and
//      write LLVM code without making ownership mistakes.
// ============================================================================

#include "../garage.h"   // the shared contract — found via -I. in the Makefile

#include <cassert>       // assert — bounds checks (Ch 9)
#include <string>
#include <string_view>
#include <vector>
#include <initializer_list>

// ─── trace::log() / trace::clear() ───────────────────────────────────────────
//  We return a reference to a FUNCTION-LOCAL STATIC vector (Ch 7 "static
//  duration" — a preview; provided scaffolding). One canonical copy lives in
//  this translation unit; every call returns the same object. This is the
//  Meyers-singleton idiom — simple, thread-safe since C++11, no header-only ODR
//  risk.  (Provided as scaffolding — NOT a learner task.)

namespace trace {
    std::vector<std::string>& log()
    {
        static std::vector<std::string> s_log;
        return s_log;
    }

    void clear()
    {
        log().clear();
    }
}

// ─── Engine implementation ─────────────────────────────────────────────────────
//  TASK 1 in the starter.
//
//  Engine is the COMPOSED PART of Car. Its ctor/dtor append to the trace log so
//  that the tests can assert the exact order relative to Car's own ctor/dtor.
//
//  KEY RULE (23.2): Because m_engine is declared BEFORE m_make in Car's class
//  definition, Engine constructs first and destroys LAST (among Car's members).
//  The Car constructor body runs AFTER all members are constructed; the Car
//  destructor body runs BEFORE members are destroyed. So the trace for one
//  Car creation/deletion reads:
//
//      "Engine built"        ← member init (before Car ctor body)
//      "Car built: Sedan"    ← Car ctor body
//          [in-use events]
//      "Car destroyed: Sedan" ← Car dtor body  (before member dtors)
//      "Engine destroyed"    ← member dtor     (after Car dtor body)
//
//  Getting this order wrong is the canonical Chapter-23 trap. The tests check it.

Engine::Engine(std::string_view model)
    : m_model { model }     // owned copy of the model string
{
    // COMPOSITION trace: append to the shared log so tests can assert order.
    trace::log().push_back("Engine built: " + std::string(model));
}

Engine::~Engine()
{
    // Destruction trace: see the note above about order relative to Car's dtor body.
    trace::log().push_back("Engine destroyed: " + std::string(m_model));
}

std::string_view Engine::model() const
{
    return m_model;
}

// ─── Driver implementation ─────────────────────────────────────────────────────
//  TASK 2 in the starter.
//
//  Driver is an INDEPENDENT object. It exists on its own; Car can REFERENCE it
//  (aggregation) without owning it. The trace lets tests verify that a Driver
//  survives after a Car that held it is destroyed.

Driver::Driver(std::string_view name)
    : m_name { name }
{
    trace::log().push_back("Driver built: " + std::string(name));
}

Driver::~Driver()
{
    // If this ever ran WHILE the Car that referenced it still existed, that would
    // mean the Car outlived its aggregated Driver — a lifetime bug (notes 23.3).
    trace::log().push_back("Driver destroyed: " + std::string(m_name));
}

std::string_view Driver::name() const
{
    return m_name;
}

// ─── Mechanic implementation ───────────────────────────────────────────────────
//  TASK 3 in the starter.
//
//  Mechanic is a DEPENDENCY — it is passed by reference to Car::tuneUp() and used
//  for that call only. Mechanic is NOT stored in Car. If you stored it, the
//  relationship would become an association (notes 23.5 — "is the relationship
//  stored?"). The test verifies that tuneUpCount() increments correctly through
//  the dependency relationship.

Mechanic::Mechanic(std::string_view name)
    : m_name { name }
{
    trace::log().push_back("Mechanic on duty: " + std::string(name));
}

Mechanic::~Mechanic()
{
    trace::log().push_back("Mechanic off duty: " + std::string(m_name));
}

std::string_view Mechanic::name() const
{
    return m_name;
}

int Mechanic::tuneUpCount() const
{
    return m_tuneUpCount;
}

void Mechanic::recordTuneUp()
{
    ++m_tuneUpCount;
}

// ─── Car implementation ────────────────────────────────────────────────────────
//  TASK 4 in the starter.
//
//  Car is the WHOLE that owns Engine (composition) and references Driver
//  (aggregation). It also demonstrates the dependency relationship through tuneUp.
//
//  MEMBER-INIT-LIST ORDER:
//      m_engine is declared FIRST in the class, so it is initialized FIRST.
//      Even if you listed m_make before m_engine in the init list, the compiler
//      still initialises in declaration order (and -Wextra warns about reorder).
//      We list them in declaration order to be explicit.

Car::Car(std::string_view make, std::string_view engineModel)
    : m_engine { engineModel }   // COMPOSITION: Engine built before Car body runs
    , m_make   { make }          // owned value member
    // m_driver default-initialised to nullptr (set in class definition)
{
    // This line executes AFTER m_engine's constructor has already run.
    // The trace order: "Engine built" appears BEFORE "Car built".
    trace::log().push_back("Car built: " + std::string(make));
}

Car::~Car()
{
    // This line executes BEFORE m_engine's destructor runs.
    // The trace order: "Car destroyed" appears BEFORE "Engine destroyed".
    trace::log().push_back("Car destroyed: " + std::string(m_make));

    // CRITICAL: do NOT call "delete m_driver" here!
    // m_driver is an AGGREGATION — Car does not own the Driver.
    // The Driver must outlive the Car in a correct program (notes 23.3).
    // Deleting it here would be undefined behaviour if the caller still holds
    // a reference to the Driver object.
}

// ── Aggregation: assign / query the (non-owned) driver ──────────────────────
//  setDriver stores a NON-OWNING pointer. The contract (from the header comment)
//  states that the pointed-to Driver must remain alive as long as this Car uses
//  it. Car does nothing to enforce that — it is the CALLER's responsibility.
//  This is the aggregation lifetime risk described in notes 23.3.

void Car::setDriver(Driver* driver)
{
    m_driver = driver;   // store the pointer; do NOT allocate or take ownership
}

Driver* Car::driver() const
{
    return m_driver;
}

// ── Dependency: use a Mechanic for one call only ─────────────────────────────
//  tuneUp() is the DEPENDENCY relationship in action:
//      - Mechanic& is a PARAMETER (not stored in any member field).
//      - The mechanic is used (recordTuneUp() called) and then the reference goes
//        out of scope when tuneUp() returns.
//  Compare this to an ASSOCIATION, which would store `Mechanic* m_lastMechanic`
//  as a field. That would be a stored relationship — a dependency is temporary
//  (notes 23.5 distinction: "is the relationship stored?").

void Car::tuneUp(Mechanic& mechanic)
{
    mechanic.recordTuneUp();   // USE the mechanic (dependency: call through param)
    trace::log().push_back("Tuned up by: " + std::string(mechanic.name()));
    // mechanic is NOT stored. When this function returns, the dependency is over.
}

std::string_view Car::make() const
{
    return m_make;
}

std::string_view Car::engineModel() const
{
    return m_engine.model();   // delegate to the COMPOSED part
}

// ─── RouteList implementation ──────────────────────────────────────────────────
//  TASK 5 in the starter.
//
//  RouteList is a VALUE CONTAINER (23.6): it owns its waypoint strings.
//  The initializer_list constructor enables brace-init syntax (23.7):
//      RouteList r { "A", "B", "C" };
//
//  NOTE on std::initializer_list (23.7):
//    - The list is passed by VALUE (it is a lightweight view, like string_view).
//    - Iterating with a range-for loop or begin()/end() is the standard approach
//      (operator[] is NOT provided by initializer_list).
//    - Brace-init prefers the initializer_list ctor when it matches, so
//      `RouteList r { "A", "B" }` calls THIS ctor, not the default ctor.
//
//  CS6340 analogy:
//    std::vector<std::string> generatedInputs { "seed1", "seed2", "seed3" };
//  That is exactly RouteList's pattern applied to a standard container.

RouteList::RouteList(std::initializer_list<std::string_view> waypoints)
{
    // Iterate the lightweight view and copy each name into our owned vector.
    // We cannot use `m_waypoints { waypoints }` directly because waypoints is
    // initializer_list<string_view> but m_waypoints is vector<string> — a range
    // loop with explicit conversion is the clearest approach.
    for (std::string_view wp : waypoints)
    {
        m_waypoints.push_back(std::string(wp));
    }
}

std::size_t RouteList::size() const
{
    return m_waypoints.size();
}

std::string_view RouteList::at(std::size_t index) const
{
    assert(index < m_waypoints.size());   // bounds guard (Ch 9)
    return m_waypoints[index];
}

void RouteList::add(std::string_view waypoint)
{
    m_waypoints.push_back(std::string(waypoint));
}

void RouteList::clear()
{
    m_waypoints.clear();
}
