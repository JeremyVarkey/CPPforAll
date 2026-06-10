// ============================================================================
//  garage.h  —  PUBLIC INTERFACE for the Garage Simulation  (Chapter 23)
// ----------------------------------------------------------------------------
//  This header is COMPLETE and PROVIDED. You do NOT edit it. Read it carefully
//  before you implement anything in starter/garage.cpp — a header is the
//  CONTRACT your implementation must honour.
//
//  WHAT THIS FILE DEMONSTRATES (while you read it):
//
//    — OBJECT RELATIONSHIP VOCABULARY (23.1):
//        Every class below models a distinct relationship; the in-class comment
//        above each member calls it out by name. Identify them before coding.
//
//    — LIFETIME TRACING (23.2 / 23.3):
//        A global Trace log collects "Engine built", "Car destroyed" etc. at
//        runtime, making construction/destruction ORDER physically visible.
//        The tests assert the exact trace contents — you must get the order right.
//
//    — MEMBER CONSTRUCTION/DESTRUCTION ORDER (23.2):
//        C++ standard rule: data members construct BEFORE the constructor body
//        runs (in DECLARATION ORDER), and destroy AFTER the destructor body
//        (in REVERSE declaration order). The trace proves this — be precise.
//
//    — SCOPE DISCIPLINE (23.1 — "covers everything except inheritance"):
//        Only Chapter ≤ 23 features appear here. Inheritance (Ch 24) and
//        virtual (Ch 25) are forbidden.
//
//  CS6340/LLVM tie-in: every analysis pass you write in CS6340 mixes these
//  relationships exactly:
//      std::string name            — composition/owned value
//      llvm::Function& F           — dependency (parameter, borrowed)
//      llvm::Instruction*          — aggregation/association (not owned)
//  Reading the *ownership story* behind a signature is the skill this lab builds.
// ============================================================================

#ifndef GARAGE_H
#define GARAGE_H

#include <string>          // std::string — owned, mutable text (Ch 5)
#include <string_view>     // std::string_view — cheap, non-owning view (Ch 5)
#include <vector>          // std::vector — dynamic sequence container (Ch 16)
#include <initializer_list> // std::initializer_list — brace-init support (23.7)

// ─── Trace Log ───────────────────────────────────────────────────────────────
//  A shared append-only log that constructors and destructors write to.
//  Using a function returning a static reference lets the log exist exactly
//  once across all translation units without a separate global definition.
//  (A preview of Ch 7 linkage rules — provided scaffolding, not your task.)

namespace trace {
    // Returns the one shared log vector. Append events here.
    std::vector<std::string>& log();

    // Clears all recorded events — call before each test scenario.
    void clear();
}

// ─── class Engine ─────────────────────────────────────────────────────────────
//  Engine is a COMPOSED PART of Car (23.2): it is built when Car is built and
//  destroyed when Car is destroyed. It has no knowledge of the Car it belongs to.
//
//  Relationship: COMPOSITION — Engine is "part of" Car; Car OWNS it exclusively.
//  C++ form: Engine is a direct VALUE MEMBER inside Car (no pointer, no new/delete).

class Engine
{
public:
    // Constructor: records "Engine built" in trace::log().
    explicit Engine(std::string_view model);

    // Destructor: records "Engine destroyed" in trace::log().
    ~Engine();

    // Returns the model string passed at construction.
    std::string_view model() const;

    // Engine is not copyable (it belongs to one Car at a time — composition rule).
    Engine(const Engine&)            = delete;
    Engine& operator=(const Engine&) = delete;

private:
    std::string m_model;
};

// ─── class Driver ─────────────────────────────────────────────────────────────
//  Driver exists independently of any Car. Multiple Cars could share a Driver.
//
//  Relationship: INDEPENDENT ENTITY — aggregated by Car (Car holds a Driver* but
//  does NOT own the Driver's lifetime). Destroying a Car must not destroy the Driver.

class Driver
{
public:
    // Constructor: records "Driver built: <name>" in trace::log().
    explicit Driver(std::string_view name);

    // Destructor: records "Driver destroyed: <name>" in trace::log().
    ~Driver();

    std::string_view name() const;

private:
    std::string m_name;
};

// ─── class Mechanic ───────────────────────────────────────────────────────────
//  Mechanic is never stored — it is only passed to Car::tuneUp() for that call.
//
//  Relationship: DEPENDENCY — Car *uses* a Mechanic temporarily (via parameter).
//  C++ form: passed by reference to the function that needs it; not stored anywhere.

class Mechanic
{
public:
    // Constructor: records "Mechanic on duty: <name>" in trace::log().
    explicit Mechanic(std::string_view name);

    // Destructor: records "Mechanic off duty: <name>" in trace::log().
    ~Mechanic();

    std::string_view name() const;

    // Returns the number of times this mechanic has been asked to tune up a car.
    int tuneUpCount() const;

    // Called by Car::tuneUp to increment this mechanic's work counter.
    void recordTuneUp();

private:
    std::string m_name;
    int m_tuneUpCount { 0 };
};

// ─── class Car ────────────────────────────────────────────────────────────────
//  Car demonstrates THREE relationships at once:
//
//  1. COMPOSITION with Engine:
//       m_engine is a VALUE MEMBER (not a pointer). The Engine is built before
//       Car's constructor body runs and destroyed after Car's destructor body.
//       The trace ORDER proves this — the standard guarantees it (23.2).
//
//  2. AGGREGATION with Driver:
//       m_driver is a POINTER. Car holds a reference to an external Driver but
//       does NOT create or destroy it. If m_driver is nullptr, the car is
//       unoccupied. Destroying a Car must NOT call delete on m_driver (23.3).
//
//  3. DEPENDENCY on Mechanic (via tuneUp):
//       Car borrows a Mechanic for one function call only; the Mechanic is NOT
//       stored as a member (23.5).

class Car
{
public:
    // Build a Car with a given make and engine model.
    // Records "Car built: <make>" in trace::log() — AFTER Engine already built.
    // m_driver starts as nullptr (no driver yet).
    Car(std::string_view make, std::string_view engineModel);

    // Records "Car destroyed: <make>" in trace::log() — BEFORE Engine destroys.
    // DOES NOT delete m_driver — Car does not own the Driver.
    ~Car();

    // ── Aggregation: assign / query the (non-owned) driver ────────────────────
    // setDriver: store the pointer (not a copy, not a new allocation).
    // The pointed-to Driver must remain alive as long as this Car uses it.
    void setDriver(Driver* driver);

    // Returns the current driver pointer (may be nullptr).
    Driver* driver() const;

    // ── Dependency: borrow a Mechanic for one function call ────────────────────
    // Calls mechanic.recordTuneUp() (uses the mechanic), then records
    // "Tuned up by: <mechanic.name()>" in trace::log().
    // The Mechanic is NOT stored — this is a DEPENDENCY, not an association (23.5).
    void tuneUp(Mechanic& mechanic);

    // ── Queries ───────────────────────────────────────────────────────────────
    std::string_view make() const;
    std::string_view engineModel() const;   // delegates to m_engine

private:
    // ── COMPOSITION member (owns lifetime) ────────────────────────────────────
    // Declared FIRST: Engine constructs first, destroys last.
    // (C++ constructs members in DECLARATION ORDER; destroys in REVERSE order.)
    Engine m_engine;

    // ── Owned data ────────────────────────────────────────────────────────────
    std::string m_make;

    // ── AGGREGATION member (does NOT own lifetime) ─────────────────────────────
    // Pointer: may be null; NEVER call delete on this.
    Driver* m_driver { nullptr };
};

// ─── class RouteList ──────────────────────────────────────────────────────────
//  A value container for a sequence of named waypoints (strings).
//  Demonstrates: container class (23.6) + std::initializer_list ctor (23.7).
//
//  Relationship: RouteList OWNS its waypoints (VALUE container — copies each
//  string in). Destroying RouteList destroys all waypoints automatically (via
//  std::vector).
//
//  Usage:
//      RouteList r { "Home", "Gas station", "Highway", "Destination" };
//      r.size();          // 4
//      r.at(0);           // "Home"

class RouteList
{
public:
    // Default: empty route.
    RouteList() = default;

    // ── TASK 5: implement this constructor (23.7) ──────────────────────────────
    // Accepts a brace-list of waypoint names:
    //     RouteList r { "A", "B", "C" };
    // Stores each name in m_waypoints (in order).
    explicit RouteList(std::initializer_list<std::string_view> waypoints);

    // Number of waypoints.
    std::size_t size() const;

    // Access the Nth waypoint (0-based). Asserts index < size().
    std::string_view at(std::size_t index) const;

    // Append one waypoint to the end.
    void add(std::string_view waypoint);

    // Remove all waypoints.
    void clear();

private:
    // VALUE container: RouteList owns these strings.
    std::vector<std::string> m_waypoints;
};

#endif // GARAGE_H
