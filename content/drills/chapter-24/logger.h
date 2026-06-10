// ============================================================================
//  logger.h  —  PUBLIC INTERFACE of the report-logger family   (Chapter 24)
// ----------------------------------------------------------------------------
//  This header is COMPLETE and PROVIDED. Do not edit it. Read it closely —
//  it is the contract every file in this exercise (starter, solution, tests)
//  includes. Both the learner's starter/logger.cpp AND solution/logger.cpp
//  include this file; tests/tests.cpp includes it too and calls the API.
//
//  THREE CLASSES ARE DECLARED HERE — study their is-a hierarchy:
//
//      Logger               (base class — owns a name, counts lines logged)
//         |
//         +-- TimestampLogger   (derived — REDEFINES log() with a prefix tag,
//         |                       then calls the base version via Logger::log())
//         |
//         +-- CountingLogger    (derived — adds linesLogged() that reads the
//                                protected m_lineCount from the base class)
//
//  KEY VOCABULARY (notes 24.1 – 24.2):
//
//    - INHERITANCE (": public Base"): a derived class inherits the accessible
//      members of the base class and can add or redefine behavior.
//    - BASE/DERIVED (or parent/child, superclass/subclass — all mean the same).
//    - PUBLIC INHERITANCE: the normal "is-a" form; a derived object CAN be used
//      wherever a base-class REFERENCE OR POINTER is expected. (notes 24.2)
//
//  HEADER GUARD (Chapter 2): the #ifndef / #define / #endif sandwich prevents
//  this file's contents from being pasted in twice in the same translation unit.
//
//  CS6340 / LLVM TIE-IN: LLVM's instruction hierarchy has exactly this shape —
//  llvm::Instruction is the base; llvm::BranchInst, llvm::CallInst, etc. are
//  derived classes that inherit behavior and add type-specific operations. You
//  will read code like this every day of CS6340. (notes 24.1)
// ============================================================================

#ifndef LOGGER_H
#define LOGGER_H

#include <string>   // std::string  (Chapter 5)

// ─── Base class: Logger ──────────────────────────────────────────────────────
//
// Logger is the general concept: any log sink that has a name, produces a
// formatted log string for a message, and tracks how many lines it has logged.
//
// DESIGN NOTES (notes 24.5):
//   - m_name    is PRIVATE:    only Logger's own code touches it. Derived
//               classes get the name through name() (the public accessor).
//   - m_lineCount is PROTECTED: derived classes (e.g. CountingLogger) are
//               allowed to READ it directly — that is precisely what protected
//               is for. Public users cannot touch it.
//   - log()     is PUBLIC:     the service this class provides. A derived class
//               may REDEFINE it (a same-name function in the derived class hides
//               the base version for calls on a derived object — notes 24.7).
//
// CONSTRUCTION-ORDER OBSERVABLE (notes 24.3 – 24.4):
//   The constructors append tokens to a shared trace string so the tests can
//   PROVE that the base constructs first, then the derived portion.
//   The pattern: pass trace by reference to both ctor and dtor (stored as a
//   protected member so derived classes can also append without storing a
//   second copy of the same reference).
//
class Logger
{
public:
    // ── Constructor / destructor ─────────────────────────────────────────────
    //
    // explicit: this is a multi-argument constructor, so it can never be picked
    // for a single-value implicit conversion (there is no lone value that turns
    // into a Logger). What `explicit` blocks here is COPY-LIST-INITIALIZATION,
    // e.g.  Logger l = { name, ct, dt };  — that form is rejected, while the
    // direct form  Logger l { name, ct, dt };  is still fine. It is good habit
    // to mark constructors explicit by default. (Chapter 14 — provided
    // scaffolding, not a learner TASK.)
    //
    // ctorTrace / dtorTrace are output parameters: each constructor and
    // destructor body APPENDS a short token so the caller can observe the
    // construction order WITHOUT std::cout. (notes 24.3: "the base portion must
    // be initialized before the derived portion can safely use it.")
    //
    explicit Logger(const std::string& name,
                    std::string& ctorTrace,
                    std::string& dtorTrace);

    ~Logger();

    // ── Public interface ─────────────────────────────────────────────────────

    // name() — read the logger's name (Chapter 15: const member function).
    const std::string& name() const;

    // log(msg) — format msg as "[<name>] <msg>", APPEND that string to the
    // internal log, INCREMENT m_lineCount, and RETURN the formatted string.
    // Derived classes may REDEFINE this function to change the format while
    // still calling this version for the base formatting work. (notes 24.7)
    std::string log(const std::string& msg);

protected:
    // ── Protected members — visible to derived classes, hidden from public ────
    //
    // m_lineCount is protected so CountingLogger can READ it directly
    // (demonstrating notes 24.5: "protected exists mainly for inheritance").
    int m_lineCount {};

    // m_dtorTrace is protected so derived destructors can append their token
    // without storing a second copy of the reference. Destructors run in
    // REVERSE order (notes 24.3): derived destructor appends FIRST, then the
    // base destructor appends.
    std::string& m_dtorTrace;

private:
    // ── Private data — Logger's own state, not accessible to derived classes ─
    std::string  m_name;
    std::string  m_log;      // accumulated log text
};


// ─── Derived class: TimestampLogger ──────────────────────────────────────────
//
// TimestampLogger IS-A Logger with a twist: its log() PREPENDS a fixed tag
// (the "timestamp" — e.g. "[T]") before delegating to the base Logger::log().
//
// KEY CONCEPTS demonstrated here (notes 24.7):
//
//   1. FUNCTION REDEFINITION (hiding): TimestampLogger::log() has the same name
//      as Logger::log(). Calls on a TimestampLogger OBJECT go to the derived
//      version — but calls through a Logger& still go to Logger::log().
//      (This is STATIC BINDING — notes 24.7 CS6340 tie-in. Task 6 makes it
//       concrete and disappointing on purpose.)
//
//   2. BASE CALL SYNTAX: inside TimestampLogger::log(), you must call the base
//      version as  Logger::log(msg)  — NOT just  log(msg)  (which would
//      recurse into itself). (notes 24.7)
//
//   3. DERIVED CONSTRUCTOR: initializes its OWN data AND calls the base ctor
//      in its MEMBER INITIALIZER LIST. (notes 24.4)
//
class TimestampLogger : public Logger   // ": public Logger" = public inheritance
{
public:
    // The derived constructor MUST name the base constructor in the initializer
    // list. The base portion constructs first (notes 24.3 / 24.4).
    TimestampLogger(const std::string& name,
                    const std::string& tag,
                    std::string& ctorTrace,
                    std::string& dtorTrace);

    ~TimestampLogger();

    // REDEFINES Logger::log(). Call the base version with Logger::log().
    // (notes 24.7: "calling the base version explicitly")
    std::string log(const std::string& msg);

private:
    std::string m_tag;   // the prefix tag, e.g. "[T]"
};


// ─── Derived class: CountingLogger ───────────────────────────────────────────
//
// CountingLogger IS-A Logger that adds one public getter: linesLogged().
// It does NOT redefine log() — it inherits the base behavior unchanged.
//
// KEY CONCEPT: adding new functionality (notes 24.6) and accessing a
// PROTECTED BASE MEMBER (notes 24.5: m_lineCount is directly readable
// because it is protected).
//
class CountingLogger : public Logger
{
public:
    CountingLogger(const std::string& name,
                   std::string& ctorTrace,
                   std::string& dtorTrace);

    ~CountingLogger();

    // linesLogged() returns the number of messages logged so far.
    // It accesses the base class's protected m_lineCount directly — that is
    // the whole point of protected (notes 24.5).
    int linesLogged() const;
};


// ─── Free function: processLog ────────────────────────────────────────────────
//
// Accepts a Logger BY REFERENCE and calls l.log(msg).
//
// TASK 6 (THE CLIFFHANGER): the parameter type is "Logger&". When you pass a
// TimestampLogger through a Logger& here, STATIC BINDING ensures that
// Logger::log() runs — NOT TimestampLogger::log(). The object IS a
// TimestampLogger, but the call is resolved at compile time using the
// DECLARED (static) type of the reference. This is the central limitation that
// Chapter 25 — virtual functions — exists to solve. (notes 24.7 CS6340 tie-in)
//
std::string processLog(Logger& l, const std::string& msg);

#endif // LOGGER_H
