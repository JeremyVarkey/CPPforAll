// ============================================================================
//  starter/logger.cpp  —  LEARNER IMPLEMENTATION   (Chapter 24)
// ----------------------------------------------------------------------------
//  Fill in the SIX TASK blocks below. Each maps 1:1 to a task in the README
//  and to a declaration in ../logger.h. The bodies currently return
//  PLACEHOLDERS so the file compiles immediately — that is why `make test` is
//  RED right now. Your job is to turn it GREEN.
//
//      make build         compile-check your code (should already work)
//      make test          grade it          (RED until you fill these in)
//      make solution      run the reference demo if you get stuck
//      make test-solution verify the solution is green
//
//  HOW TO READ THIS FILE: search for ">>> YOUR CODE HERE <<<" to jump to
//  each task. Read the surrounding comments carefully — the pedagogy is there.
// ============================================================================

#include "logger.h"   // Chapter 2 best practice: include own paired header.
                      // The -I. Makefile flag makes "logger.h" resolve to the
                      // chapter root's header. The compiler checks your bodies
                      // against the contract declared in that header.

// ─── Logger (base class) ─────────────────────────────────────────────────────


// ─── TASK 1: Logger constructor ────────────────────────────────────────────
//
// Initialize the Logger base class:
//   - m_lineCount  initialize to zero (use the member-initializer list syntax)
//   - m_dtorTrace  bind to the dtorTrace reference parameter (stored as a
//                  protected member so derived destructors can append to it)
//   - m_name       initialize from the `name` parameter
//   - m_log        default-initialize to an empty string
//
// In the constructor BODY, append "Logger+" to ctorTrace.
// This proves the base constructs before any derived class portion.
// (notes 24.3: "base classes construct first")
//
// SYNTAX REMINDER (notes 24.4):
//   Constructor members are initialized in the member-initializer list,
//   NOT in the body. The body runs AFTER all members are initialized.
//
//   Logger::Logger(const std::string& name, std::string& ctorTrace, std::string& dtorTrace)
//       : m_lineCount {}         // zero-initialize
//       , m_dtorTrace { dtorTrace }
//       , m_name { name }
//       , m_log {}
//   {
//       ctorTrace += "Logger+";  // body: append our token
//   }
//
Logger::Logger(const std::string& name,
               std::string& /*ctorTrace*/,
               std::string& dtorTrace)
    : m_lineCount  {}
    , m_dtorTrace  { dtorTrace }   // must initialize the reference member
    , m_name       { name }        // name stored correctly
    , m_log        {}
{
    // >>> YOUR CODE HERE <<<
    // Append "Logger+" to ctorTrace in this body.
    // (Hint: ctorTrace += "Logger+";)
}
// ─────────────────────────────────────────────────────────────────────────────

Logger::~Logger()
{
    m_dtorTrace += "Logger-";   // PROVIDED: base destructor appends its token
}

const std::string& Logger::name() const
{
    return m_name;   // PROVIDED: returns the private name
}


// ─── TASK 2: Logger::log(msg) ──────────────────────────────────────────────
//
// Produce a formatted log entry and update internal state:
//   1. Build the formatted string:  "[" + m_name + "] " + msg
//   2. Increment m_lineCount.
//   3. Append (entry + "\n") to m_log.
//   4. Return the entry string.
//
// WHY IT MATTERS:
//   - TimestampLogger::log() will CALL this version via Logger::log(msg).
//     (notes 24.7: "calling the base version explicitly")
//   - CountingLogger::linesLogged() reads m_lineCount, which this function
//     increments. (notes 24.5: m_lineCount is protected)
//
//   >>> YOUR CODE HERE <<<
//
std::string Logger::log(const std::string& /*msg*/)
{
    // placeholder — returns the name only; wrong format, count not updated.
    return "[" + m_name + "] ";
}
// ─────────────────────────────────────────────────────────────────────────────


// ─── TimestampLogger (derived class) ─────────────────────────────────────────


// ─── TASK 3: TimestampLogger constructor ────────────────────────────────────
//
// Initialize the DERIVED class:
//   1. In the member-initializer list, CALL the base constructor:
//          Logger { name, ctorTrace, dtorTrace }
//      This is the ONLY way to initialize the base subobject. (notes 24.4)
//      The base constructor runs FIRST — before m_tag is initialized.
//   2. Initialize m_tag { tag }.
//   3. In the body, append "Timestamp+" to ctorTrace.
//      (The base ctor already appended "Logger+", so the final trace is
//       "Logger+Timestamp+" — base before derived. notes 24.3)
//
// SYNTAX:
//   TimestampLogger::TimestampLogger(...)
//       : Logger { name, ctorTrace, dtorTrace }   // call base ctor in init list
//       , m_tag  { tag }
//   {
//       ctorTrace += "Timestamp+";
//   }
//
//   >>> YOUR CODE HERE <<<
//
TimestampLogger::TimestampLogger(const std::string& name,
                                 const std::string& /*tag*/,
                                 std::string& ctorTrace,
                                 std::string& dtorTrace)
    : Logger { name, ctorTrace, dtorTrace }   // call base ctor (required)
    , m_tag  {}                               // placeholder: ignores `tag`
{
    ctorTrace += "Timestamp+";   // placeholder body — tag ignored
    // >>> YOUR CODE HERE <<<
    // Replace m_tag {} above with m_tag { tag } to capture the tag parameter.
}
// ─────────────────────────────────────────────────────────────────────────────

TimestampLogger::~TimestampLogger()
{
    // TASK 3 (part 2 — PROVIDED): derived destructor appends its token.
    // m_dtorTrace is a PROTECTED member in Logger, so this compiles.
    // Destruction is REVERSE of construction: this runs BEFORE Logger::~Logger.
    // (notes 24.3: "destruction happens in reverse")
    m_dtorTrace += "Timestamp-";
}


// ─── TASK 4: TimestampLogger::log(msg) ──────────────────────────────────────
//
// REDEFINE Logger::log() to prefix msg with the tag stored in m_tag.
//
//   Step 1: Build a decorated string:  m_tag + " " + msg
//   Step 2: Call the BASE version:     Logger::log(decorated)
//              ↑ MUST use Logger:: qualifier — plain log(decorated)
//                would recurse into THIS function. (notes 24.7)
//   Step 3: Return the result from the base call.
//
// WHY THIS PATTERN (notes 24.7):
//   The derived function ADDS behavior (the tag) and DELEGATES the rest to the
//   base. This is "calling inherited functions and overriding behavior".
//
//   >>> YOUR CODE HERE <<<
//
std::string TimestampLogger::log(const std::string& msg)
{
    // placeholder — calls the base directly without the tag.
    // (Correct call site, wrong decoration — replace with the real two steps.)
    return Logger::log(msg);
}
// ─────────────────────────────────────────────────────────────────────────────


// ─── CountingLogger (derived class) ──────────────────────────────────────────


// ─── TASK 5: CountingLogger constructor + linesLogged() ─────────────────────
//
// Part A (constructor) — CountingLogger adds no new data, so:
//   - Call Logger { name, ctorTrace, dtorTrace } in the initializer list.
//   - Append "Counting+" in the body. (This part is already provided below.)
//
// Part B (linesLogged) — see the function body after the destructor.
//
CountingLogger::CountingLogger(const std::string& name,
                               std::string& ctorTrace,
                               std::string& dtorTrace)
    : Logger { name, ctorTrace, dtorTrace }   // provided: base call required
{
    ctorTrace += "Counting+";   // provided: appends derived token
}

CountingLogger::~CountingLogger()
{
    m_dtorTrace += "Counting-";   // PROVIDED
}


// ─── TASK 5 (continued): CountingLogger::linesLogged() ─────────────────────
//
// Return the number of lines logged so far.
// Read the PROTECTED base member m_lineCount directly. (notes 24.5)
//
// WHY THIS WORKS: CountingLogger is a DERIVED CLASS of Logger and m_lineCount
// is declared `protected` in Logger. Derived class code can access protected
// members — that is the whole point of protected. Public users cannot write
// `cl.m_lineCount` from outside the class.
//
//   >>> YOUR CODE HERE <<<
//
int CountingLogger::linesLogged() const
{
    return 0;   // placeholder — always returns zero; replace with m_lineCount
}
// ─────────────────────────────────────────────────────────────────────────────


// ─── TASK 6: processLog (free function — THE CLIFFHANGER) ────────────────────
//
// Accept a Logger BY REFERENCE (Logger& l) and call l.log(msg).
// Return the result.
//
// THIS IS THE MOST IMPORTANT TASK: once you implement it, the test DELIBERATELY
// checks that passing a TimestampLogger through a Logger& runs the BASE log()
// — NOT the derived TimestampLogger::log(). This is STATIC BINDING:
//
//   "the compiler resolves the function call at COMPILE TIME using the
//    DECLARED type of the reference (Logger), not the runtime type of the
//    object (TimestampLogger)."   (notes 24.7 CS6340 tie-in)
//
// The tag does NOT appear in the output. This is expected-but-disappointing.
// Chapter 25 solves this with `virtual`.
//
//   >>> YOUR CODE HERE <<<
//
std::string processLog(Logger& /*l*/, const std::string& /*msg*/)
{
    return {};   // placeholder — returns empty; replace with l.log(msg)
}
// ─────────────────────────────────────────────────────────────────────────────
