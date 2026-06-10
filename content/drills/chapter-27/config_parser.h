// ============================================================================
//  config_parser.h  —  the PUBLIC CONTRACT for the SafeConfig parser  (Ch 27)
// ----------------------------------------------------------------------------
//  This header is COMPLETE and PROVIDED. Do not edit it. Your job is to
//  implement the bodies in starter/config_parser.cpp (or solution/).
//
//  THE BIG IDEA: structured error propagation.
//
//  return-code style forces the caller to check a status after every call, and
//  there is no clean channel for a constructor to report failure. Exceptions
//  solve both problems by separating "something went wrong" (the throw site)
//  from "what do we do about it" (the matching catch block), potentially
//  several frames up the call stack.
//
//  This lab makes that physical:
//    • Two custom exception CLASSES (notes 27.5) so catch blocks can discriminate
//      on TYPE — not just on an int error code.
//    • A port-parsing function that throws the RIGHT type depending on what
//      failed (bad characters vs. out-of-range value).
//    • A key-lookup that throws a narrower derived class.
//    • A noexcept BOUNDARY wrapper that contains exceptions completely — the
//      caller sees only bool+value, never a propagating exception (notes 27.9).
//    • An Unwinder struct whose destructor increments a counter so a test can
//      VERIFY that stack unwinding ran the destructor (notes 27.3).
//
//  Header guard (Chapter 2): prevents double-inclusion.
#ifndef CONFIG_PARSER_H
#define CONFIG_PARSER_H

#include <stdexcept>      // std::runtime_error, std::out_of_range
#include <string>         // std::string
#include <string_view>    // std::string_view — cheap view, no copy (Chapter 5)
#include <unordered_map>  // std::unordered_map — key-value store (a preview;
                          // formally Chapter 17+ / standard containers)
#include <optional>       // std::optional — "a value or nothing" (Chapter 12)

// ─── Exception Hierarchy ─────────────────────────────────────────────────────
//
// Why derive from std::runtime_error instead of std::exception directly?
// (notes 27.5)
//   • std::runtime_error already stores a message string and returns it from
//     what(). Deriving from it gives us that for free.
//   • Code that catches (const std::exception&) — e.g., a top-level handler —
//     still handles our errors: IS-A inheritance works here.
//
// CS6340 lens: the LLVM Instruction class hierarchy is structured the same way —
// derived types for specificity, base type for generic traversal.

// ConfigError  — the base class for all config-parsing failures.
// Catches anything that goes wrong while parsing a configuration.
class ConfigError : public std::runtime_error
{
public:
    // Forwarding constructor: pass the message straight to std::runtime_error
    // so what() returns it unchanged (notes 27.5).
    explicit ConfigError(const std::string& message)
        : std::runtime_error { message }
    {
    }
};

// MissingKeyError : ConfigError  — thrown when a key that must exist is absent.
// A caller can catch (const ConfigError&) for all config errors, or catch
// (const MissingKeyError&) if it needs to handle "not found" specifically.
// Derived BEFORE base in any catch sequence — see notes 27.5 "Catch derived
// exceptions before base exceptions."
class MissingKeyError : public ConfigError
{
public:
    // Stores the key that was missing so the caller can name it in a message.
    explicit MissingKeyError(const std::string& key)
        : ConfigError { "missing required key: " + key }
        , m_key { key }
    {
    }

    // The absent key, separate from the full what() message.
    const std::string& key() const noexcept { return m_key; }

private:
    std::string m_key;
};

// ─── SafeConfig ───────────────────────────────────────────────────────────────
//
// A thin wrapper around a string→string map that provides throwing accessors.
// Think of it as a minimal stand-in for a real config-file parser.
class SafeConfig
{
public:
    // Construct from any existing key-value map.
    explicit SafeConfig(std::unordered_map<std::string, std::string> entries)
        : m_entries { std::move(entries) }   // move: no copy (Chapter 22)
    {
    }

    // ── TASK 1 ───────────────────────────────────────────────────────────────
    // getOrThrow — return the value for `key`, or throw MissingKeyError if the
    // key is not present.
    //
    // IMPLEMENTATION FILE: starter/config_parser.cpp  (in the SafeConfig section)
    std::string getOrThrow(std::string_view key) const;

    // ── TASK 2 ───────────────────────────────────────────────────────────────
    // parsePort — find `key`, then parse its value as a port number.
    //
    // Throws:
    //   MissingKeyError   — key is not in the map
    //   ConfigError       — value is EMPTY, or contains non-digit characters
    //   std::out_of_range — value is numeric but outside [1, 65535]
    //
    // (The empty case must be checked explicitly: a per-character digit loop
    //  passes vacuously for "" — and std::stoi("") would then leak
    //  std::invalid_argument, a type this contract does not allow.)
    //
    // (notes 27.2: throw the RIGHT type — callers can then catch only the
    //  exceptions they know how to handle.)
    //
    // IMPLEMENTATION FILE: starter/config_parser.cpp
    int parsePort(std::string_view key) const;

private:
    std::unordered_map<std::string, std::string> m_entries;
};

// ─── tryParsePort — the noexcept boundary wrapper ────────────────────────────
//
// A "boundary" is a point in the call graph where exceptions are ABSORBED and
// translated into a non-exception result (notes 27.9, CS6340 patterns).
//
// tryParsePort wraps SafeConfig::parsePort with noexcept so that no exception
// can escape to its caller. If parsePort throws for any reason, outPort is left
// unchanged and the function returns false. On success it stores the port and
// returns true.
//
// DESIGN NOTE: the noexcept promise is real here because we catch everything
// inside the body. Do NOT add noexcept to a function unless you truly guarantee
// no exception escapes — the compiler lets you lie, but std::terminate() runs
// if the lie is exposed (notes 27.9).
//
// ── TASK 3 ───────────────────────────────────────────────────────────────────
// IMPLEMENTATION FILE: starter/config_parser.cpp
bool tryParsePort(const SafeConfig& cfg, std::string_view key,
                  int& outPort) noexcept;

// ─── Unwinder — the RAII stack-unwinding probe ───────────────────────────────
//
// A tiny struct whose constructor increments *entered and whose destructor
// increments *destroyed. When a function throws PAST an Unwinder local, the
// destructor MUST run during stack unwinding (notes 27.3). The test confirms
// this by observing the counter: if stack unwinding skipped destructors, the
// destroyed count would be wrong.
//
// This is the "make it physical" principle from CLAUDE.md: you don't just read
// that destructors run during unwinding — you OBSERVE the counter change.
//
// ── TASK 4 ───────────────────────────────────────────────────────────────────
// Implement the constructor and destructor in starter/config_parser.cpp.
// Constructor: increment *entered.
// Destructor:  increment *destroyed. (Do NOT throw from a destructor —
//              notes 27.8 "Destructors should not throw".)
struct Unwinder
{
    int* entered   {};   // pointer to a caller-owned counter
    int* destroyed {};   // pointer to a caller-owned counter

    // Increment *entered to record that this Unwinder was created.
    explicit Unwinder(int* pEntered, int* pDestroyed);

    // Increment *destroyed to record that cleanup ran — even during unwinding.
    ~Unwinder();

    // Non-copyable: a probe object should not be accidentally duplicated.
    // (= delete syntax — Chapter 14; provided here as scaffolding.)
    Unwinder(const Unwinder&)            = delete;
    Unwinder& operator=(const Unwinder&) = delete;
};

// ─── throwingWork — helper that exercises unwinding ──────────────────────────
//
// Creates an Unwinder local, then throws ConfigError so the Unwinder is
// destroyed by stack unwinding rather than by a normal return.
//
// ── TASK 4 (continued) ───────────────────────────────────────────────────────
// The test drives this by calling throwingWork inside a try block, catching
// the exception, and checking that *pDestroyed incremented. DO NOT change
// the signature.
// IMPLEMENTATION FILE: starter/config_parser.cpp
void throwingWork(int* pEntered, int* pDestroyed);

// ─── rethrowOuter — demonstrates the catch / rethrow idiom ──────────────────
//
// Calls throwingWork inside a try block. Catches the exception, prepends
// "outer: " to the what() string, and re-throws a NEW ConfigError with that
// composed message. (notes 27.6 — this is "throw a new exception", not bare
// throw; — we intentionally raise a different object here.)
//
// ── TASK 5 ───────────────────────────────────────────────────────────────────
// IMPLEMENTATION FILE: starter/config_parser.cpp
void rethrowOuter(int* pEntered, int* pDestroyed);

#endif // CONFIG_PARSER_H
