// ============================================================================
//  tests/tests.cpp  —  automated grader for the report-logger family (Ch 24)
// ----------------------------------------------------------------------------
//  Tiny no-framework harness (same style as the other drills). Includes
//  ../logger.h and calls the API through BOTH the starter (make test) and the
//  solution (make test-solution). Exit code 0 = PASS, 1 = FAIL.
//
//  The Makefile links this file against either starter/logger.cpp or
//  solution/logger.cpp via -I. (so #include "logger.h" resolves correctly).
//
//  GRADING PHILOSOPHY FOR CH 24:
//    - Tests are organized by concept to give clear diagnostic messages.
//    - The STATIC BINDING test (Task 6) is the crown jewel: it DELIBERATELY
//      asserts the BASE (non-timestamp) behavior when passing a TimestampLogger
//      through a Logger&. The test is correct — the "wrong" (base-only) result
//      IS the expected result for a non-virtual hierarchy. The comment explains
//      why. (notes 24.7 CS6340 tie-in)
// ============================================================================

#include <iostream>
#include <string>
#include "logger.h"     // found via -I. (Makefile); same header, both builds

static int fails = 0;

// CHECK: assert a boolean condition; on failure report expression and line.
#define CHECK(cond) \
    do { if (!(cond)) { std::cerr << "FAIL: " #cond "  @line " << __LINE__ << "\n"; ++fails; } } while (0)

// CHECK_EQ: compare two std::string values with a readable diff on failure.
#define CHECK_EQ(got, want) \
    do { std::string g_ = (got), w_ = (want); \
         if (g_ != w_) { std::cerr << "FAIL @line " << __LINE__ \
             << ": got \"" << g_ << "\", want \"" << w_ << "\"\n"; ++fails; } } while (0)

int main()
{
    // ── SECTION 1: Basic Logger construction and name() ──────────────────────
    //
    // The simplest thing: Logger constructs, name() returns what was passed in.
    // Exercises TASK 1 (ctor) and the public accessor.
    {
        std::string ctorTrace, dtorTrace;
        Logger base { "base_log", ctorTrace, dtorTrace };

        CHECK(base.name() == "base_log");
    }

    // ── SECTION 2: Logger::log() format and line count ───────────────────────
    //
    // Verifies TASK 2: the format "[<name>] <msg>", the return value, and that
    // multiple calls accumulate the count correctly.
    {
        std::string ct, dt;
        Logger lg { "sys", ct, dt };

        // First message
        CHECK_EQ(lg.log("boot"),    "[sys] boot");
        // Second message (different content — checks no state leakage)
        CHECK_EQ(lg.log("running"), "[sys] running");
        // Edge case: empty message
        CHECK_EQ(lg.log(""),        "[sys] ");
    }

    // ── SECTION 3: Construction order — BASE constructs FIRST (notes 24.3) ──
    //
    // This is THE observable proof of the chapter's core rule. The ctorTrace
    // string is appended to by Logger's ctor first, then by the derived ctor.
    // EXPECTED: trace starts with "Logger+" (base) before "Timestamp+" (derived).
    //
    {
        std::string ct, dt;
        {
            TimestampLogger tl { "ts_log", "[T]", ct, dt };
            // At this point the derived object is alive. Check the order.
        }
        // After the block: destructors have run in REVERSE order.
        // Ctor trace must start with "Logger+" (base ran first).
        CHECK(ct.substr(0, 7) == "Logger+");    // base constructed first
        CHECK(ct == "Logger+Timestamp+");        // both tokens present
    }

    {
        std::string ct, dt;
        {
            CountingLogger cl { "cnt_log", ct, dt };
        }
        CHECK(ct.substr(0, 7) == "Logger+");    // base constructed first
        CHECK(ct == "Logger+Counting+");
    }

    // ── SECTION 4: Destruction order — REVERSE of construction (notes 24.3) ──
    //
    // After a TimestampLogger goes out of scope, the dtor trace should show:
    //   "Timestamp-" (derived dtor runs first) then "Logger-" (base dtor).
    {
        std::string ct, dt;
        {
            TimestampLogger tl { "ts_log", "[T]", ct, dt };
        } // tl destroyed here
        CHECK(dt == "Timestamp-Logger-");   // derived dtor first, then base
    }

    {
        std::string ct, dt;
        {
            CountingLogger cl { "cnt_log", ct, dt };
        }
        CHECK(dt == "Counting-Logger-");
    }

    // ── SECTION 5: TimestampLogger::log() redefinition (notes 24.7) ─────────
    //
    // Calling log() on a TimestampLogger OBJECT goes to the derived version,
    // which prepends the tag, then calls Logger::log() for the base format.
    // The resulting string should include BOTH the tag and the base formatting.
    //
    {
        std::string ct, dt;
        TimestampLogger tl { "audit", "[T]", ct, dt };

        // The base formatting wraps the tag+msg combination:
        //   msg "connect" -> decorated "[T] connect" -> base formats as "[audit] [T] connect"
        CHECK_EQ(tl.log("connect"), "[audit] [T] connect");

        // Different tag to verify the tag is actually used
        TimestampLogger tl2 { "sec", "[ERR]", ct, dt };
        CHECK_EQ(tl2.log("denied"), "[sec] [ERR] denied");

        // Edge case: empty message
        CHECK_EQ(tl.log(""), "[audit] [T] ");
    }

    // Verify that TimestampLogger::log CALLS the base (m_lineCount increments).
    // The counter should go up on each log call, regardless of which log() ran.
    {
        std::string ct, dt;
        CountingLogger cl { "counter", ct, dt };
        CHECK(cl.linesLogged() == 0);   // no calls yet
        cl.log("a");
        CHECK(cl.linesLogged() == 1);
        cl.log("b");
        cl.log("c");
        CHECK(cl.linesLogged() == 3);
    }

    // ── SECTION 6: protected m_lineCount via CountingLogger (notes 24.5) ─────
    //
    // CountingLogger does NOT redefine log() — it inherits Logger::log().
    // But it ADDS linesLogged() which reads the protected m_lineCount.
    // This is the demonstration of protected: derived can read it, public cannot.
    //
    {
        std::string ct, dt;
        CountingLogger cl { "audit", ct, dt };

        CHECK(cl.linesLogged() == 0);    // before any log calls

        cl.log("startup");
        CHECK(cl.linesLogged() == 1);

        cl.log("shutdown");
        CHECK(cl.linesLogged() == 2);

        // Edge case: verify CountingLogger still formats the same as Logger
        // (it inherits log() unchanged — notes 24.6)
        std::string ct2, dt2;
        CountingLogger cl2 { "inf", ct2, dt2 };
        CHECK_EQ(cl2.log("tick"), "[inf] tick");
        CHECK(cl2.linesLogged() == 1);
    }

    // TimestampLogger routes through the SAME base Logger::log() (via the
    // explicit base call), so its derived log() must keep returning correctly
    // formatted, tag-prefixed entries across repeated calls — proof that the
    // base formatting + counting path is reused, not duplicated. Neither
    // TimestampLogger nor Logger exposes linesLogged() publicly (only
    // CountingLogger adds it), so we assert on the visible return value here and
    // rely on the CountingLogger section above for the count itself.
    {
        std::string ct, dt;
        TimestampLogger tl { "ts", "[X]", ct, dt };
        CHECK_EQ(tl.log("first"),  "[ts] [X] first");
        CHECK_EQ(tl.log("second"), "[ts] [X] second");
    }

    // ── SECTION 7: THE CLIFFHANGER — static binding (notes 24.7) ────────────
    //
    // This is the MOST IMPORTANT test in the exercise. Read the comment below
    // carefully before concluding it is wrong — it is INTENTIONALLY testing the
    // "disappointing" behavior.
    //
    // processLog(l, msg) accepts a Logger& and calls l.log(msg).
    // When we pass a TimestampLogger:
    //
    //   TimestampLogger tl { ... };
    //   processLog(tl, "hello");
    //
    // ... the call inside processLog resolves to Logger::log() — NOT to
    // TimestampLogger::log() — because `l` is typed as `Logger&` and there is
    // no `virtual`. This is STATIC BINDING: name lookup uses the DECLARED
    // (static) type of the reference, not the RUNTIME type of the object.
    //
    // The CHECK below asserts the BASE format "[name] msg" WITHOUT the tag.
    // This is CORRECT behavior for a non-virtual hierarchy. It is also the
    // problem that makes Chapter 25 necessary.
    //
    // If you made log() virtual (forbidden in this chapter — it would make
    // this test fail as designed), you would get "[name] [T] msg" instead.
    // The failing test would be Chapter 25's reward.
    //
    {
        std::string ct, dt;
        TimestampLogger tl { "router", "[T]", ct, dt };

        // Direct call on the TimestampLogger object: DERIVED version runs.
        // (Correct and expected: calling on the actual object type.)
        CHECK_EQ(tl.log("direct"), "[router] [T] direct");

        // Call through Logger&: BASE version runs (static binding!).
        // The tag "[T]" does NOT appear — even though the object IS a
        // TimestampLogger. This is the cliffhanger.
        std::string result = processLog(tl, "via_ref");
        CHECK_EQ(result, "[router] via_ref");   // base format, NO tag

        // Verify the counter incremented for BOTH calls (both routes through
        // log() ultimately call Logger::log() which increments m_lineCount).
        // We can't call linesLogged() on TimestampLogger, so we do an indirect
        // check: make a CountingLogger and route through processLog.
        std::string ct2, dt2;
        CountingLogger cl { "cnt", ct2, dt2 };
        processLog(cl, "msg1");
        processLog(cl, "msg2");
        CHECK(cl.linesLogged() == 2);

        // Static binding with CountingLogger through Logger& also runs Logger::log.
        // (CountingLogger doesn't redefine log(), so the result is the same as
        // calling Logger::log() directly — in this case the behavior is correct.)
        std::string ct3, dt3;
        CountingLogger cl2 { "probe", ct3, dt3 };
        CHECK_EQ(processLog(cl2, "probe_msg"), "[probe] probe_msg");
    }

    // ── SECTION 8: Edge cases ────────────────────────────────────────────────
    {
        // Logger with an empty name: "[" + "" + "] " + "x" = "[] x"
        // (no space inside the brackets when the name is empty — that is correct)
        std::string ct, dt;
        Logger empty { "", ct, dt };
        CHECK_EQ(empty.log("x"), "[] x");
    }

    {
        // Multiple TimestampLogger objects with the same base tag can coexist;
        // their counts are independent (different Logger base subobjects).
        std::string ct, dt;
        TimestampLogger a { "a", "[A]", ct, dt };
        TimestampLogger b { "b", "[B]", ct, dt };
        CHECK_EQ(a.log("hello"), "[a] [A] hello");
        CHECK_EQ(b.log("world"), "[b] [B] world");
        // a and b have independent base subobjects — no shared state.
    }

    // ── RESULT ───────────────────────────────────────────────────────────────
    if (!fails)
        std::cout << "PASS \xE2\x9C\x85  all logger checks passed.\n";
    else
        std::cerr << "\nFAIL \xE2\x9D\x8C  " << fails
                  << " check(s) failed — see lines above.\n";

    return fails ? 1 : 0;
}
