// ============================================================================
//  solution/logger.cpp  —  REFERENCE IMPLEMENTATION   (Chapter 24)
// ----------------------------------------------------------------------------
//  Complete, correct, warning-clean bodies for the three-class logger family
//  declared in ../logger.h. Peek only after you've taken a real swing at
//  starter/logger.cpp — the learning is in building the hierarchy yourself.
//
//  Every comment here names the NOTES SECTION that motivates the choice so
//  you can cross-reference easily.
// ============================================================================

#include "logger.h"   // Chapter 2 best practice: include own paired header
                      // (the -I. flag in the Makefile makes this resolve to
                      //  the chapter root's logger.h regardless of whether
                      //  we're in solution/ or starter/)

// ─── Logger (base class) ─────────────────────────────────────────────────────

// TASK 1: Logger constructor.
//
// MEMBER INITIALIZER LIST (notes 24.4): the comma-separated entries after ':'
// initialize base-class and member data IN DECLARATION ORDER (not the order
// written here). They run BEFORE the constructor body { }.
//
// m_dtorTrace is a PROTECTED REFERENCE MEMBER so derived destructors can
// append their own token without storing a second copy of the reference.
// We initialize it first because members are initialized in declaration order.
//
// The constructor body appends "Logger+" to ctorTrace. This lets the tests
// verify that the base constructs FIRST by checking the trace prefix.
// (notes 24.3: "base classes construct first — the base portion must be
//  initialized before the derived portion can safely use it.")
//
Logger::Logger(const std::string& name,
               std::string& ctorTrace,
               std::string& dtorTrace)
    : m_lineCount  {}
    , m_dtorTrace  { dtorTrace }
    , m_name       { name }
    , m_log        {}
{
    // Append our construction token to the shared trace.
    // When constructing a TimestampLogger, this runs BEFORE the
    // TimestampLogger constructor body, so the trace starts with "Logger+".
    ctorTrace += "Logger+";
}

// Logger destructor.
// Destruction order is REVERSE of construction (notes 24.3):
//   derived destructor runs first -> base destructor runs last.
// For a TimestampLogger: trace reads "Timestamp-Logger-" (derived first).
Logger::~Logger()
{
    m_dtorTrace += "Logger-";
}

const std::string& Logger::name() const
{
    return m_name;
}

// TASK 2: log(msg).
//
// FORMAT: "[<name>] <msg>"
// This function owns the "formatted" token. Derived classes CALL this version
// (via Logger::log) to produce the base format, then decorate it.
//
// INCREMENTS m_lineCount — CountingLogger reads that protected value via
// linesLogged() to report how many messages have been processed. (notes 24.5:
// m_lineCount is protected so derived classes have direct read access.)
//
// Returns the formatted string so callers can check the output directly.
//
std::string Logger::log(const std::string& msg)
{
    // Build the formatted entry: "[<name>] <msg>"
    std::string entry { "[" + m_name + "] " + msg };

    // Update internal state.
    ++m_lineCount;
    m_log += entry + "\n";

    return entry;
}


// ─── TimestampLogger (derived class) ─────────────────────────────────────────

// TASK 3: TimestampLogger constructor.
//
// The DERIVED CONSTRUCTOR initializes:
//   1. The BASE PORTION: by calling Logger{ name, ctorTrace, dtorTrace } in the
//      initializer list. This is the ONLY way to initialize the base subobject —
//      you cannot reach Logger's private members directly. (notes 24.4:
//      "derived constructors choose the base constructor")
//   2. Its OWN data: m_tag { tag }.
//
// ORDER (notes 24.3 / 24.4): "Logger{ ... }" in the initializer list runs FIRST.
// Only after the base portion is fully constructed does m_tag get initialized.
// The constructor body then appends "Timestamp+". Because the base constructor
// already appended "Logger+", the final ctorTrace reads "Logger+Timestamp+" —
// the observable proof that base constructs before derived.
//
TimestampLogger::TimestampLogger(const std::string& name,
                                 const std::string& tag,
                                 std::string& ctorTrace,
                                 std::string& dtorTrace)
    : Logger { name, ctorTrace, dtorTrace }   // BASE portion constructed here
    , m_tag  { tag }                          // then our own member
{
    // Base already appended "Logger+"; now we add the derived token.
    ctorTrace += "Timestamp+";
}

// Destructor: derived runs BEFORE base (notes 24.3: reverse of construction).
// m_dtorTrace is a PROTECTED member in Logger, so we can access it here.
TimestampLogger::~TimestampLogger()
{
    m_dtorTrace += "Timestamp-";   // protected base member — accessible to derived
}

// TASK 4: TimestampLogger::log().
//
// This REDEFINES Logger::log() — the name "log" in TimestampLogger HIDES the
// base version. (notes 24.7 / 24.8: function hiding)
//
// IMPORTANT: call  Logger::log(decorated)  — NOT  log(decorated).
//   - log(decorated) would call THIS function recursively (infinite loop).
//   - Logger::log(decorated) calls the BASE class version explicitly.
//   (notes 24.7: "calling the base version explicitly — use Base::fn()")
//
// The base Logger::log() handles the "[name] ..." formatting AND increments
// m_lineCount, so CountingLogger still works correctly even on a
// TimestampLogger (the counter lives in the shared Logger base portion).
//
std::string TimestampLogger::log(const std::string& msg)
{
    // Step 1: decorate the message with our tag.
    std::string decorated { m_tag + " " + msg };

    // Step 2: hand off to the BASE implementation for formatting + counting.
    // The explicit Logger:: qualifier is REQUIRED (notes 24.7).
    return Logger::log(decorated);
}


// ─── CountingLogger (derived class) ──────────────────────────────────────────

// TASK 5: CountingLogger constructor + linesLogged().
//
// CountingLogger adds no new data — so the initializer list only calls the base
// constructor. The body appends "Counting+", giving the full ctorTrace
// "Logger+Counting+" — same base-first proof as TimestampLogger above.
//
CountingLogger::CountingLogger(const std::string& name,
                               std::string& ctorTrace,
                               std::string& dtorTrace)
    : Logger { name, ctorTrace, dtorTrace }
{
    ctorTrace += "Counting+";
}

CountingLogger::~CountingLogger()
{
    m_dtorTrace += "Counting-";   // protected base member — accessible to derived
}

// TASK 5 (continued): linesLogged() — reads the protected base member.
//
// Returns m_lineCount — the PROTECTED base-class member. (notes 24.5)
//
// This compiles because CountingLogger is a DERIVED CLASS of Logger, and
// m_lineCount is declared protected. Public code CANNOT write:
//   CountingLogger cl { ... };
//   cl.m_lineCount;   // ERROR: m_lineCount is protected
// But derived-class code (this body) can read it freely.
//
int CountingLogger::linesLogged() const
{
    return m_lineCount;   // PROTECTED base member — accessible from derived class
}


// ─── processLog (free function — the cliffhanger) ────────────────────────────

// TASK 6: processLog.
//
// Accepts a Logger BY REFERENCE and calls l.log(msg).
//
// THE CLIFFHANGER (notes 24.7 CS6340 tie-in):
//
//   Even if you pass a TimestampLogger object here:
//       TimestampLogger tl { ... };
//       processLog(tl, "hello");   // <-- what runs?
//
//   ... the call  l.log(msg)  resolves to Logger::log() — NOT
//   TimestampLogger::log(). WHY?
//
//   Because `l` is declared as `Logger&`. The compiler resolves the call
//   at COMPILE TIME using the STATIC (declared) type of the reference — Logger.
//   This is STATIC BINDING. The runtime object happens to be a TimestampLogger,
//   but WITHOUT `virtual`, the compiler has no mechanism to dispatch to the
//   derived version.
//
//   The test that passes a TimestampLogger through this function DELIBERATELY
//   asserts the BASE behavior — "[name] msg" WITHOUT the timestamp tag. This is
//   the "expected but disappointing" result that makes Chapter 25 necessary.
//
//   THE FIX (Chapter 25): add `virtual` to Logger::log(). Then the vtable
//   dispatches at RUNTIME using the DYNAMIC (actual) type of the object, and
//   TimestampLogger::log() runs. But that is Chapter 25's story.
//
std::string processLog(Logger& l, const std::string& msg)
{
    return l.log(msg);   // STATIC BINDING: always calls Logger::log(),
                         // even if `l` refers to a TimestampLogger.
}
