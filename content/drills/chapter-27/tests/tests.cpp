// Chapter 27 — Exceptions · Project: SafeConfig Parser   (GRADER)
// ─────────────────────────────────────────────────────────────────────────────
// Tiny no-framework unit-test harness — same style as CLAUDE.md.
// Includes ../config_parser.h and exercises every public function across
// many inputs, including the exception-specific edge cases that separate
// correct exception handling from plausible-looking near-misses.
//
// The Makefile links this against starter/config_parser.cpp (make test) or
// solution/config_parser.cpp (make test-solution). EXIT CODE 1 = failures.
//
// ── ABOUT THE EXCEPTION-CHECK PATTERN ────────────────────────────────────────
// A function that is SUPPOSED to throw cannot be checked with a single macro
// expression. The canonical shape used throughout this file is:
//
//     try {
//         call_that_should_throw();
//         CHECK(false && "should have thrown");   // ← if we reach this, it didn't
//     } catch (const ExactType& e) {
//         CHECK(std::string{e.what()}.find("keyword") != std::string::npos);
//     }
//
// The `CHECK(false && ...)` trick makes the grader fail if the throw was
// missing. The catch block verifies the TYPE is right (the handler is only
// reached for ExactType) and checks the message content.
// ─────────────────────────────────────────────────────────────────────────────

#include <iostream>
#include <string>
#include "../config_parser.h"

static int fails = 0;

#define CHECK(cond) \
    do { if(!(cond)){ \
        std::cerr << "FAIL: " #cond "  @line " << __LINE__ << "\n"; \
        ++fails; \
    } } while(0)

// A helper that runs the exception-check pattern and returns true if the
// right exception was thrown. Reduces boilerplate in the tests below.
// (Inline template — a preview of Chapter 26; provided as scaffolding so
//  the learner only fills in the TASK bodies, not the test harness.)
template<typename ExType, typename Fn>
bool throws_as(Fn fn)
{
    try { fn(); return false; }
    catch (const ExType&) { return true; }
    catch (...) { return false; }   // threw, but wrong type
}

int main()
{
    // ── Task 1: getOrThrow ─────────────────────────────────────────────────
    {
        SafeConfig cfg { {{"host", "localhost"}, {"port", "8080"}} };

        // Happy path: key exists → correct value
        CHECK(cfg.getOrThrow("host") == "localhost");
        CHECK(cfg.getOrThrow("port") == "8080");

        // Missing key → MissingKeyError (the SPECIFIC derived type, not base)
        try
        {
            cfg.getOrThrow("timeout");
            CHECK(false && "getOrThrow should have thrown for missing key");
        }
        catch (const MissingKeyError& e)
        {
            // Correct type caught. Verify the what() mentions the key name.
            CHECK(std::string { e.what() }.find("timeout") != std::string::npos);
            // Also verify the .key() accessor (it is tested separately).
            CHECK(e.key() == "timeout");
        }

        // A MissingKeyError is also a ConfigError (IS-A).
        // Catch ordering: derived BEFORE base (notes 27.5).
        bool caught_as_config { false };
        try
        {
            cfg.getOrThrow("missing");
        }
        catch (const MissingKeyError&)
        {
            caught_as_config = true;   // preferred handler fires first
        }
        catch (const ConfigError&)
        {
            // If the ordering were wrong this would fire — then caught_as_config
            // stays false and the CHECK below would flag it.
        }
        CHECK(caught_as_config);

        // Edge: empty key string treated as absent
        CHECK(throws_as<MissingKeyError>([&]{ cfg.getOrThrow(""); }));
    }

    // ── Task 2: parsePort — success ───────────────────────────────────────
    {
        SafeConfig cfg { {{"p", "80"}, {"max", "65535"}, {"min", "1"}} };

        CHECK(cfg.parsePort("p")   == 80);
        CHECK(cfg.parsePort("max") == 65535);   // boundary: top of range
        CHECK(cfg.parsePort("min") == 1);        // boundary: bottom of range
    }

    // ── Task 2: parsePort — MissingKeyError ───────────────────────────────
    {
        SafeConfig cfg { {{"x", "9000"}} };

        // Missing key → MissingKeyError must propagate through parsePort
        try
        {
            cfg.parsePort("absent");
            CHECK(false && "parsePort should throw MissingKeyError for absent key");
        }
        catch (const MissingKeyError& e)
        {
            CHECK(std::string { e.what() }.find("absent") != std::string::npos);
        }
    }

    // ── Task 2: parsePort — ConfigError for non-numeric value ─────────────
    {
        SafeConfig cfg { {{"p", "abc"}, {"q", "80x"}, {"r", "12.3"}} };

        // Alphabetic value → ConfigError
        CHECK(throws_as<ConfigError>([&]{ cfg.parsePort("p"); }));

        // Trailing non-digit → ConfigError
        CHECK(throws_as<ConfigError>([&]{ cfg.parsePort("q"); }));

        // Decimal point → ConfigError (not a valid integer port string)
        CHECK(throws_as<ConfigError>([&]{ cfg.parsePort("r"); }));

        // A ConfigError for bad format is NOT a MissingKeyError.
        CHECK(!throws_as<MissingKeyError>([&]{ cfg.parsePort("p"); }));

        // EDGE: an EMPTY value is malformed too. The digit loop passes
        // VACUOUSLY for "" (no characters to reject) — without an explicit
        // empty check, std::stoi("") would throw std::invalid_argument: the
        // WRONG type, absent from the contract, and invisible to a caller
        // catching ConfigError. The contract demands ConfigError here.
        SafeConfig cfg_empty { {{"p", ""}} };
        CHECK(throws_as<ConfigError>([&]{ cfg_empty.parsePort("p"); }));
        // ...and specifically NOT a leaked std::invalid_argument from stoi.
        CHECK(!throws_as<std::invalid_argument>([&]{ cfg_empty.parsePort("p"); }));

        // Verify the what() message mentions the bad value.
        try
        {
            cfg.parsePort("p");
            CHECK(false && "should have thrown");
        }
        catch (const ConfigError& e)
        {
            CHECK(std::string { e.what() }.find("abc") != std::string::npos);
        }
    }

    // ── Task 2: parsePort — std::out_of_range ─────────────────────────────
    {
        // Port 0 is below the minimum (1).
        SafeConfig cfg_zero  { {{"p", "0"}} };
        // Port 65536 is above the maximum (65535).
        SafeConfig cfg_high  { {{"p", "65536"}} };
        // Port 99999 is well above the maximum.
        SafeConfig cfg_huge  { {{"p", "99999"}} };

        // Must throw std::out_of_range — the EXACT type, not just ConfigError.
        CHECK(throws_as<std::out_of_range>([&]{ cfg_zero.parsePort("p"); }));
        CHECK(throws_as<std::out_of_range>([&]{ cfg_high.parsePort("p"); }));
        CHECK(throws_as<std::out_of_range>([&]{ cfg_huge.parsePort("p"); }));

        // out_of_range is NOT a ConfigError (it derives from std::logic_error,
        // not from std::runtime_error in the standard hierarchy). Verify the
        // catch ordering matters: a ConfigError catch should NOT swallow it.
        CHECK(!throws_as<ConfigError>([&]{ cfg_zero.parsePort("p"); }));
    }

    // ── Task 3: tryParsePort ──────────────────────────────────────────────
    {
        SafeConfig good { {{"port", "8080"}, {"tls", "443"}} };
        SafeConfig bad  { {{"port", "abc"},  {"over", "99999"}} };

        int out { -1 };

        // Success case → returns true, fills outPort
        CHECK(tryParsePort(good, "port", out) == true);
        CHECK(out == 8080);

        CHECK(tryParsePort(good, "tls", out) == true);
        CHECK(out == 443);

        // MissingKeyError → returns false, outPort unchanged
        out = -1;
        CHECK(tryParsePort(good, "missing", out) == false);
        CHECK(out == -1);   // outPort must not be modified on failure

        // ConfigError (bad format) → returns false
        out = -1;
        CHECK(tryParsePort(bad, "port", out) == false);
        CHECK(out == -1);

        // out_of_range → returns false
        out = -1;
        CHECK(tryParsePort(bad, "over", out) == false);
        CHECK(out == -1);
    }

    // ── Task 4: Unwinder — stack unwinding is observable ─────────────────
    {
        int entered   { 0 };
        int destroyed { 0 };

        // throwingWork creates an Unwinder and then throws.
        // The Unwinder destructor MUST run during unwinding before the catch.
        try
        {
            throwingWork(&entered, &destroyed);
            CHECK(false && "throwingWork should have thrown");
        }
        catch (const ConfigError&)
        {
            // Exception caught. Now verify the Unwinder lifecycle.
            CHECK(entered   == 1);   // constructor ran: Unwinder was created
            CHECK(destroyed == 1);   // destructor ran: unwinding cleaned it up
        }

        // Normal return path (no throw): destructor still runs.
        // We test this by catching the exception, but the dtor is what matters.
        // The counts above already prove it for the unwinding path.

        // Edge: a second call accumulates independently.
        int e2 { 0 }, d2 { 0 };
        try { throwingWork(&e2, &d2); } catch (...) {}
        CHECK(e2 == 1);
        CHECK(d2 == 1);
    }

    // ── Task 5: rethrowOuter — catch, augment, re-raise ──────────────────
    {
        int entered   { 0 };
        int destroyed { 0 };

        // rethrowOuter must:
        //   1. Propagate a ConfigError (the re-raised one).
        //   2. Its what() starts with "outer: ".
        //   3. The Unwinder inside throwingWork still cleaned up.
        try
        {
            rethrowOuter(&entered, &destroyed);
            CHECK(false && "rethrowOuter should have thrown");
        }
        catch (const ConfigError& e)
        {
            // Correct type
            std::string msg { e.what() };
            CHECK(msg.find("outer: ") == 0);             // starts with "outer: "
            CHECK(msg.find("work failed") != std::string::npos); // original text
        }

        // Unwinder inside throwingWork must have been destroyed.
        CHECK(entered   == 1);
        CHECK(destroyed == 1);

        // The exception that escapes is a ConfigError, NOT a MissingKeyError.
        bool caught_mis { false };
        try
        {
            int e3 { 0 }, d3 { 0 };
            rethrowOuter(&e3, &d3);
        }
        catch (const MissingKeyError&) { caught_mis = true; }
        catch (const ConfigError&)     { /* expected */ }
        CHECK(!caught_mis);   // rethrowOuter throws ConfigError, not derived
    }

    // ── Summary ────────────────────────────────────────────────────────────
    if (!fails)
        std::cout << "PASS ✅  all SafeConfig checks passed.\n";
    else
        std::cerr << "\nFAIL ❌  " << fails << " check(s) failed"
                     " — fix the TASK blocks in starter/config_parser.cpp.\n";

    return fails ? 1 : 0;
}
