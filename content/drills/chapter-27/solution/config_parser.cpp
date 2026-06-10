// Chapter 27 — Exceptions · Project: SafeConfig Parser   (REFERENCE SOLUTION)
// ─────────────────────────────────────────────────────────────────────────────
// Complete, correct, warning-clean reference implementation of ../config_parser.h.
// Peek only after you've taken a real swing at starter/config_parser.cpp.
// The learning is in wiring the throw/catch logic yourself, then comparing.

#include "../config_parser.h"

#include <string>
#include <string_view>
#include <cctype>         // std::isdigit

// ─── SafeConfig::getOrThrow ───────────────────────────────────────────────────
// TASK 1: return the value or throw MissingKeyError (the specific derived type).
//
// Key technique (notes 27.2):
//   We throw MissingKeyError, not just ConfigError. The catch block in a test
//   that uses `catch (const MissingKeyError&)` will match; one that uses
//   `catch (const ConfigError&)` ALSO matches because MissingKeyError IS-A
//   ConfigError — the standard exception hierarchy at work. The type carries
//   information that a bare int code never could.
std::string SafeConfig::getOrThrow(std::string_view key) const
{
    // Convert string_view to std::string for use as unordered_map key.
    auto it { m_entries.find(std::string { key }) };

    if (it == m_entries.end())
        throw MissingKeyError { std::string { key } };   // THROW: key absent

    return it->second;
}

// ─── SafeConfig::parsePort ────────────────────────────────────────────────────
// TASK 2: parse a port number, throwing the RIGHT type for each failure mode.
//
// Three distinct failure modes → three distinct exception types (notes 27.2):
//   1. Key absent              → MissingKeyError  (propagates from getOrThrow)
//   2. Empty / non-digit value → ConfigError      (bad format)
//   3. Out-of-range int        → std::out_of_range (value not in [1, 65535])
//
// CS6340 lens: this mirrors an LLVM pass checking IR semantics — different
// "bad" conditions get different error types so the recovery point can be
// specific. A generic "something failed" error loses information.
int SafeConfig::parsePort(std::string_view key) const
{
    // Step 1: look up the key — let MissingKeyError propagate naturally
    // (no try/catch here: we don't handle it, so don't intercept it).
    std::string value { getOrThrow(key) };

    // Step 2: reject an EMPTY value before the digit loop. The loop below
    // passes VACUOUSLY for "" (no characters to reject), and std::stoi("")
    // would then throw std::invalid_argument — a type that is NOT in our
    // contract, and one a caller catching ConfigError would miss entirely
    // (invalid_argument derives from std::logic_error, not runtime_error).
    // Guarding here keeps every malformed-value path on the documented type
    // (notes 27.2: throw the RIGHT type — never leak an implementation
    // detail's exception through your interface).
    if (value.empty())
        throw ConfigError { "port value is empty" };

    // Step 3: every character must be a digit — no leading '-', no spaces.
    for (char ch : value)
    {
        if (!std::isdigit(static_cast<unsigned char>(ch)))
        {
            // THROW ConfigError: bad format. The message names the offender.
            throw ConfigError {
                "port value is not numeric: '" + value + "'"
            };
        }
    }

    // Step 4: convert and range-check. stoi handles the integer conversion
    // (safe now: the value is non-empty and all digits, so invalid_argument
    // cannot occur).
    int port { std::stoi(value) };

    if (port < 1 || port > 65535)
    {
        // THROW std::out_of_range: the notes mention this type for
        // "index/key outside valid range" (notes 27.5, table).
        throw std::out_of_range {
            "port " + std::to_string(port) + " is outside [1, 65535]"
        };
    }

    return port;
}

// ─── tryParsePort ─────────────────────────────────────────────────────────────
// TASK 3: noexcept boundary — absorb all exceptions, return bool+out-ref.
//
// Why noexcept here? (notes 27.9)
//   The caller is a top-level driver that cannot handle exceptions itself.
//   Marking the boundary noexcept makes the promise explicit and prevents
//   accidents where a future code change adds a throw path that slips through.
//   The noexcept promise is REAL because we catch (...) inside — every possible
//   exception is handled before reaching the function boundary.
//
// catch (...) — the "catch all" handler (notes 27.4). Use it sparingly; here
// it is exactly right because the boundary must contain ANY failure.
bool tryParsePort(const SafeConfig& cfg, std::string_view key,
                  int& outPort) noexcept
{
    try
    {
        outPort = cfg.parsePort(key);   // may throw MissingKeyError, ConfigError,
                                        // std::out_of_range, or anything else
        return true;                    // only reached on success
    }
    catch (...)
    {
        // All exceptions are swallowed here. outPort is unchanged.
        return false;
    }
}

// ─── Unwinder constructor ─────────────────────────────────────────────────────
// TASK 4a: save pointers and record construction.
//
// The increment of *entered proves the object was alive when throwingWork ran.
Unwinder::Unwinder(int* pEntered, int* pDestroyed)
    : entered   { pEntered   }
    , destroyed { pDestroyed }
{
    ++(*entered);   // record: this Unwinder was created
}

// ─── Unwinder destructor ──────────────────────────────────────────────────────
// TASK 4b: record destruction — normal path OR unwinding path.
//
// Destructor rule (notes 27.8): do NOT throw here. If this destructor ran
// because an exception was already being handled and we threw again, the
// runtime would call std::terminate(). A counter increment is always safe.
Unwinder::~Unwinder()
{
    ++(*destroyed);   // record: destructor ran (may be unwinding)
}

// ─── throwingWork ─────────────────────────────────────────────────────────────
// TASK 4c: create Unwinder local, then throw PAST it.
//
// Stack unwinding path (notes 27.3):
//   1. Unwinder uw is constructed — *pEntered increments.
//   2. throw ConfigError{"work failed"} executes.
//   3. Stack unwinds: uw goes out of scope, Unwinder::~Unwinder runs.
//   4. *pDestroyed increments DURING UNWINDING — before the catch block runs.
//   5. ConfigError propagates to the caller's catch block.
//
// This makes unwinding PHYSICAL: the test will verify *pDestroyed changed
// even though throwingWork never reached a normal return statement.
void throwingWork(int* pEntered, int* pDestroyed)
{
    Unwinder uw { pEntered, pDestroyed };   // RAII probe: dtor runs on throw

    // Throw ConfigError — the Unwinder dtor runs BEFORE this reaches the caller.
    throw ConfigError { "work failed" };
}   // <- uw's destructor would also run here on normal exit (never reached)

// ─── rethrowOuter ─────────────────────────────────────────────────────────────
// TASK 5: catch an exception, compose a richer message, throw a new one.
//
// Two rethrowing patterns (notes 27.6):
//   bare `throw;`   — re-raises the original exception UNCHANGED (no slicing)
//   throw New{...}  — raises a DIFFERENT exception object (this function)
//
// We use the second form to layer diagnostic context. This is the LLVM
// "wrapping errors" pattern: inner functions report raw facts; outer functions
// add callsite context before propagating.
//
// Catch by CONST REFERENCE (notes 27.2 — avoids slicing and unnecessary copy).
void rethrowOuter(int* pEntered, int* pDestroyed)
{
    try
    {
        throwingWork(pEntered, pDestroyed);   // throws ConfigError{"work failed"}
    }
    catch (const ConfigError& e)
    {
        // Compose a new message and throw a different exception object.
        // "throw e;" here would SLICE if e were a derived type (notes 27.6).
        // "throw;" (bare) would re-raise the exact original object.
        // We intentionally want the NEW, composed message, so we throw a new
        // ConfigError with a layered what() string.
        throw ConfigError { std::string { "outer: " } + e.what() };
    }
}
