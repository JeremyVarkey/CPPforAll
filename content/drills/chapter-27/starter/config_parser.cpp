// Chapter 27 — Exceptions · Project: SafeConfig Parser   (STARTER)
// ─────────────────────────────────────────────────────────────────────────────
// Fill in the five TASK blocks below. Each maps 1:1 to a task in the README
// and to a declaration in ../config_parser.h. The bodies currently return
// PLACEHOLDERS so the file compiles immediately — that's why `make test` is
// RED right now. Your job is to turn it GREEN by wiring up the real exception
// handling.
//
//     make build       compile your code (should work right now)
//     make test        grade it          (RED until you fill these in)
//     make solution    run the reference if you get stuck
//
// Key terms you are making physical (see notes 27.2–27.9):
//   throw        — raise an exception object
//   try/catch    — mark a region + handle a matching type
//   stack unwind — destructors of locals run as frames are popped
//   noexcept     — a promise that no exception escapes to the caller
//   rethrow      — throw; (bare) to re-raise the current exception unchanged,
//                  or throw NewException{...} to raise a different object

#include "../config_parser.h"

#include <string>         // std::string, std::stoi
#include <string_view>    // std::string_view
#include <cctype>         // std::isdigit

// ─── SafeConfig::getOrThrow (TASK 1) ─────────────────────────────────────────
// Return the value stored under `key` in m_entries. If the key is not present,
// throw a MissingKeyError (notes 27.5: throw the SPECIFIC derived type so the
// caller can catch it by type — not just a generic ConfigError).
//
// Hint: std::unordered_map has a .find(key) that returns an iterator; compare
// to .end() to test for absence. Or use .count(key) == 0.
// std::string_view cannot be used directly as an unordered_map key with the
// default hasher in C++17; convert with std::string{key} first.
//
// ─── TASK 1: getOrThrow ──────────────────────────────────────────────────────
// Throw MissingKeyError when the key is absent; otherwise return the value.
//
//   >>> YOUR CODE HERE <<<
//
// ─────────────────────────────────────────────────────────────────────────────
std::string SafeConfig::getOrThrow(std::string_view key) const
{
    (void)key;
    return "";   // placeholder — always returns empty string (never throws)
}

// ─── SafeConfig::parsePort (TASK 2) ──────────────────────────────────────────
// Parse a port number from the config key `key`.
//
//  Step 1: call getOrThrow(key) — let MissingKeyError propagate naturally
//          (you do NOT need to catch it here; it is already the right type).
//
//  Step 2: reject an EMPTY value: throw ConfigError if value.empty().
//          THE TRAP: a per-character digit loop passes VACUOUSLY for "" (there
//          are no characters to reject) — and std::stoi("") would then throw
//          std::invalid_argument, the WRONG type (not in the contract; a caller
//          catching ConfigError would miss it). Check .empty() FIRST.
//
//  Step 3: validate that every character of the value is a digit. If any
//          non-digit is found, throw ConfigError with a meaningful message.
//          (notes 27.2: throw the right type.)
//
//  Step 4: convert to int (std::stoi does this), then check the range [1, 65535].
//          If out of range, throw std::out_of_range (it is part of the declared
//          throws contract — tests verify the EXACT type, not just "threw").
//
// Hint: std::isdigit(ch) checks a single character.
// std::stoi(str) converts a std::string to int (may throw std::invalid_argument
// or std::out_of_range, but your empty + digit checks prevent invalid_argument).
//
// ─── TASK 2: parsePort ───────────────────────────────────────────────────────
// Throw MissingKeyError / ConfigError / std::out_of_range per the contract above.
//
//   >>> YOUR CODE HERE <<<
//
// ─────────────────────────────────────────────────────────────────────────────
int SafeConfig::parsePort(std::string_view key) const
{
    (void)key;
    return 0;   // placeholder — always returns 0 (never throws)
}

// ─── tryParsePort (TASK 3) ────────────────────────────────────────────────────
// A NOEXCEPT BOUNDARY wrapper around SafeConfig::parsePort (notes 27.9).
//
// Contract:
//   • Call cfg.parsePort(key) inside a try block.
//   • If it succeeds, store the result in outPort and return true.
//   • If it throws ANYTHING, leave outPort unchanged and return false.
//   • No exception must escape — the function is marked noexcept.
//
// Why is this useful? The caller can check a bool without dealing with
// exceptions. This is the "exceptions contained at a policy boundary" pattern
// from the notes' CS6340 section. A top-level tool driver uses exactly this
// shape to translate library exceptions into exit codes.
//
// ─── TASK 3: tryParsePort ────────────────────────────────────────────────────
// Catch ALL exceptions and return false; return true on success.
// (Hint: catch (...) catches every exception type — notes 27.4.)
//
//   >>> YOUR CODE HERE <<<
//
// ─────────────────────────────────────────────────────────────────────────────
bool tryParsePort(const SafeConfig& cfg, std::string_view key,
                  int& outPort) noexcept
{
    (void)cfg;
    (void)key;
    (void)outPort;
    return false;   // placeholder — always reports failure
}

// ─── Unwinder constructor / destructor (TASK 4) ──────────────────────────────
// The Unwinder struct is a RAII PROBE: it makes stack unwinding OBSERVABLE.
//
// Constructor: save the two pointers, then increment *pEntered so the test
//              can confirm the object was created.
// Destructor:  increment *destroyed. This runs whether the function returns
//              normally OR throws past this Unwinder (notes 27.3).
//
// IMPORTANT — never throw from a destructor (notes 27.8):
//   If an exception is already being handled when a second exception tries to
//   escape a destructor, std::terminate() is called immediately. Keep the
//   destructor body to simple, non-throwing operations only.
//
// ─── TASK 4a: Unwinder constructor ──────────────────────────────────────────
//
//   >>> YOUR CODE HERE <<<
//
// ─────────────────────────────────────────────────────────────────────────────
Unwinder::Unwinder(int* pEntered, int* pDestroyed)
    : entered   { pEntered   }
    , destroyed { pDestroyed }
{
    // placeholder — should increment *entered
}

// ─── TASK 4b: Unwinder destructor ────────────────────────────────────────────
//
//   >>> YOUR CODE HERE <<<
//
// ─────────────────────────────────────────────────────────────────────────────
Unwinder::~Unwinder()
{
    // placeholder — should increment *destroyed
}

// ─── throwingWork (TASK 4c) ───────────────────────────────────────────────────
// Create a local Unwinder, then throw a ConfigError. The exception unwinds the
// Unwinder (calling its destructor) before the throw reaches the caller's
// catch block. The test checks that *pDestroyed incremented even though the
// function never reached a normal return.
//
//   >>> YOUR CODE HERE <<<
//
// ─────────────────────────────────────────────────────────────────────────────
void throwingWork(int* pEntered, int* pDestroyed)
{
    (void)pEntered;
    (void)pDestroyed;
    // placeholder — should create an Unwinder and then throw ConfigError
}

// ─── rethrowOuter (TASK 5) ────────────────────────────────────────────────────
// Demonstrates the "catch, augment, re-raise" idiom (notes 27.6).
//
// Call throwingWork in a try block. When it throws, catch the exception by
// CONST REFERENCE (notes 27.2 — avoids slicing and unnecessary copies).
// Build a new message: "outer: " + e.what(). Then throw a NEW ConfigError
// with that composed message (this is different from bare `throw;` — we
// deliberately raise a different exception object here to layer context).
//
// The test verifies:
//   • The exception that escapes rethrowOuter IS a ConfigError.
//   • Its what() starts with "outer: ".
//   • *pDestroyed incremented (the Unwinder inside throwingWork still ran).
//
// ─── TASK 5: rethrowOuter ────────────────────────────────────────────────────
// Catch (const ConfigError& e), then throw ConfigError{"outer: " + e.what()}.
//
//   >>> YOUR CODE HERE <<<
//
// ─────────────────────────────────────────────────────────────────────────────
void rethrowOuter(int* pEntered, int* pDestroyed)
{
    (void)pEntered;
    (void)pDestroyed;
    // placeholder — should call throwingWork and re-throw with layered message
}
