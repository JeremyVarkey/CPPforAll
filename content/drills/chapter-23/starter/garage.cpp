// ============================================================================
//  starter/garage.cpp  —  YOUR WORKSPACE  (Chapter 23)
// ----------------------------------------------------------------------------
//  Fill in the five TASK blocks below. Each maps 1:1 to a numbered task in
//  the README and to a declaration in ../garage.h. The file compiles right now
//  (stubs return placeholders), but `make test` is RED. Your goal: GREEN.
//
//      make build         compile your code (should already work)
//      make test          grade it  (RED until you fill these in)
//      make solution      run the grader against the reference if you get stuck
//
//  BEFORE YOU CODE — read these four things in order:
//    1.  ../garage.h          — the contract (all class declarations + comments)
//    2.  The README (tasks)   — what each TASK asks for
//    3.  The notes            — ../../notes/chapter-23.md
//    4.  The tests            — tests/tests.cpp  (so you know what is checked)
//
//  CENTRAL IDEA:
//    The trace::log() calls inside every constructor and destructor you write
//    are NOT optional decoration — the TESTS CHECK THEM. The trace makes
//    C++ lifetime rules PHYSICAL: you can't guess the construction/destruction
//    order; you have to know the rule and get the strings exactly right.
//
//    Key rule (23.2): data MEMBERS construct BEFORE the constructor body runs
//    (in declaration order) and destroy AFTER the destructor body (in reverse
//    declaration order). Engine is declared first in Car, so Engine always
//    constructs before and destroys after the Car's own ctor/dtor body.
//
//  RELATIONSHIP VOCABULARY (fill in from notes 23.1 as you read):
//    — Engine inside Car:           COMPOSITION  (owns lifetime, value member)
//    — Driver* inside Car:          AGGREGATION  (does NOT own, pointer)
//    — Mechanic& in tuneUp():       DEPENDENCY   (temporary borrow, not stored)
//    — RouteList owns its strings:  CONTAINER    (value container, 23.6 / 23.7)
// ============================================================================

#include "../garage.h"   // the shared contract — found via -I. in the Makefile

#include <cassert>
#include <string>
#include <string_view>
#include <vector>
#include <initializer_list>

// ─── trace::log() / trace::clear() ───────────────────────────────────────────
//  PROVIDED — do NOT modify. A function-static vector persists for the program's
//  lifetime. (This uses Ch 7 "static duration" — a preview; provided scaffolding
//  so you don't need to know it yet.) Your ctors/dtors call trace::log().push_back().

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

// ─── TASK 1: Engine constructor and destructor ────────────────────────────────
//
//  Engine is the COMPOSITION part of Car (notes 23.2). It is created and
//  destroyed together with the Car that owns it — a value member.
//
//  Implement:
//    Engine::Engine(std::string_view model)
//        — Store `model` in m_model (use std::string(model) to convert).
//        — Append exactly this string to trace::log():
//              "Engine built: " + model
//          e.g. for model "V8-Turbo" → trace gets "Engine built: V8-Turbo".
//
//    Engine::~Engine()
//        — Append to trace::log():
//              "Engine destroyed: " + m_model
//
//    Engine::model() const   → return m_model
//
//  TRAP: do NOT call trace from inside m_model's init — it is not yet populated
//  when the member-init runs. Append to the log INSIDE the constructor body {}.
//
//   >>> YOUR CODE HERE <<<
//

Engine::Engine(std::string_view /*model*/)
    // Hint: initialise m_model here with : m_model { model }
{
    // TODO: push "Engine built: <model>" onto trace::log()
}

Engine::~Engine()
{
    // TODO: push "Engine destroyed: <m_model>" onto trace::log()
}

std::string_view Engine::model() const
{
    return m_model;   // placeholder: returns m_model (which is empty until Task 1 done)
}
// ─────────────────────────────────────────────────────────────────────────────


// ─── TASK 2: Driver constructor and destructor ────────────────────────────────
//
//  Driver is an INDEPENDENT entity that Car AGGREGATES (notes 23.3).
//  Car holds a Driver* but NEVER deletes it.
//
//  Implement:
//    Driver::Driver(std::string_view name)
//        — Store name in m_name.
//        — Append to trace::log():
//              "Driver built: " + name
//
//    Driver::~Driver()
//        — Append to trace::log():
//              "Driver destroyed: " + m_name
//
//    Driver::name() const  → return m_name
//
//   >>> YOUR CODE HERE <<<
//

Driver::Driver(std::string_view /*name*/)
{
    // TODO: store name, push "Driver built: <name>"
}

Driver::~Driver()
{
    // TODO: push "Driver destroyed: <m_name>"
}

std::string_view Driver::name() const
{
    return m_name;   // placeholder: returns empty string until Task 2 done
}
// ─────────────────────────────────────────────────────────────────────────────


// ─── TASK 3: Mechanic constructor, destructor, and helpers ───────────────────
//
//  Mechanic is used as a DEPENDENCY — passed by reference to Car::tuneUp(),
//  NEVER stored inside Car (notes 23.5).
//
//  Implement:
//    Mechanic::Mechanic(std::string_view name)
//        — Store name in m_name.
//        — Append to trace::log():
//              "Mechanic on duty: " + name
//
//    Mechanic::~Mechanic()
//        — Append to trace::log():
//              "Mechanic off duty: " + m_name
//
//    Mechanic::name() const        → return m_name
//    Mechanic::tuneUpCount() const → return m_tuneUpCount
//    Mechanic::recordTuneUp()      → increment m_tuneUpCount by 1
//
//   >>> YOUR CODE HERE <<<
//

Mechanic::Mechanic(std::string_view /*name*/)
{
    // TODO: store name, push "Mechanic on duty: <name>"
}

Mechanic::~Mechanic()
{
    // TODO: push "Mechanic off duty: <m_name>"
}

std::string_view Mechanic::name() const
{
    return m_name;   // placeholder: returns empty string until Task 3 done
}

int Mechanic::tuneUpCount() const
{
    return m_tuneUpCount;   // placeholder: always 0 until recordTuneUp implemented
}

void Mechanic::recordTuneUp()
{
    // TODO: increment m_tuneUpCount
}
// ─────────────────────────────────────────────────────────────────────────────


// ─── TASK 4: Car constructor, destructor, and all Car member functions ────────
//
//  Car is the composite WHOLE that demonstrates three relationships at once:
//    1. COMPOSITION with Engine (m_engine — value member; Engine is built first,
//       destroyed last among Car's members, notes 23.2).
//    2. AGGREGATION with Driver (m_driver — pointer; Car does NOT delete it,
//       notes 23.3). The Car can have nullptr driver (unoccupied).
//    3. DEPENDENCY on Mechanic (parameter in tuneUp — NOT stored, notes 23.5).
//
//  Implement:
//
//    Car::Car(std::string_view make, std::string_view engineModel)
//        — Use a MEMBER-INIT LIST to construct m_engine{engineModel} and m_make{make}.
//          (m_driver default-initialises to nullptr from the class definition.)
//        — In the constructor BODY, append to trace::log():
//              "Car built: " + make
//          NOTE: m_engine's constructor already fired BEFORE this body runs.
//          The trace will read: "Engine built: ..." then "Car built: ...".
//
//    Car::~Car()
//        — Append to trace::log():
//              "Car destroyed: " + m_make
//        — DO NOT call delete on m_driver — Car does not own the Driver.
//          NOTE: after this body runs, m_engine's destructor will fire next.
//          The trace will read: "Car destroyed: ..." then "Engine destroyed: ...".
//
//    Car::setDriver(Driver* driver) — store the pointer (no ownership transfer)
//    Car::driver() const            — return m_driver
//    Car::tuneUp(Mechanic& mechanic)
//        — Call mechanic.recordTuneUp() (the dependency use).
//        — Append to trace::log():
//              "Tuned up by: " + mechanic.name()
//        — Do NOT store the mechanic anywhere.
//    Car::make() const              — return m_make
//    Car::engineModel() const       — delegate to m_engine.model()
//
//   >>> YOUR CODE HERE <<<
//

Car::Car(std::string_view /*make*/, std::string_view /*engineModel*/)
    : m_engine { "" }   // placeholder: passes empty string — TASK is to fix this
    // Hint: also initialise m_make here
{
    // TODO: push "Car built: <make>" onto trace::log()
}

Car::~Car()
{
    // TODO: push "Car destroyed: <m_make>"
    // DO NOT delete m_driver — aggregation means Car does not own the Driver.
}

void Car::setDriver(Driver* driver)
{
    m_driver = driver;   // placeholder already correct — store pointer, no ownership
}

Driver* Car::driver() const
{
    return m_driver;   // placeholder already correct
}

void Car::tuneUp(Mechanic& mechanic)
{
    // TODO: call mechanic.recordTuneUp() and push "Tuned up by: <name>" onto trace
    (void)mechanic;   // suppress unused-parameter warning until Task 4 is filled in
}

std::string_view Car::make() const
{
    return m_make;   // placeholder: returns empty until Task 4 done
}

std::string_view Car::engineModel() const
{
    return m_engine.model();   // delegation to COMPOSED part (already correct shape)
}
// ─────────────────────────────────────────────────────────────────────────────


// ─── TASK 5: RouteList initializer_list constructor ──────────────────────────
//
//  RouteList is a VALUE CONTAINER (notes 23.6): it owns its waypoint strings.
//  The initializer_list constructor lets callers write:
//      RouteList r { "A", "B", "C" };
//  which is more readable than calling add() three times.
//
//  Implement:
//    RouteList::RouteList(std::initializer_list<std::string_view> waypoints)
//        — For each std::string_view in waypoints, push a std::string copy into
//          m_waypoints. (Range-for loop is cleanest — initializer_list does not
//          provide operator[], but it does support begin()/end() and range-for.)
//        — Notes 23.7: "Accessing initializer_list elements — use a range-for loop."
//
//  The other five member functions (size, at, add, clear) are provided below
//  and already correct — you only implement the constructor.
//
//   >>> YOUR CODE HERE <<<
//

RouteList::RouteList(std::initializer_list<std::string_view> /*waypoints*/)
{
    // TODO: iterate over waypoints and push each one into m_waypoints
    // Hint:  for (std::string_view wp : waypoints)
    //            m_waypoints.push_back(std::string(wp));
}
// ─────────────────────────────────────────────────────────────────────────────

// ── Provided: the rest of RouteList is already implemented ──────────────────
//  These bodies are CORRECT and should NOT be modified.

std::size_t RouteList::size() const
{
    return m_waypoints.size();
}

std::string_view RouteList::at(std::size_t index) const
{
    assert(index < m_waypoints.size());
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
