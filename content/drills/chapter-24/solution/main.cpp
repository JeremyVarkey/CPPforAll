// ============================================================================
//  solution/main.cpp  —  DEMO DRIVER for the report-logger family  (Ch 24)
// ----------------------------------------------------------------------------
//  This is an UNGRADED demo; the grade comes from `make test-solution`.
//  Run it with: make solution
//
//  The demo is deliberately designed to make EVERY key Ch 24 concept
//  OBSERVABLE in the output — you can SEE the construction order, the
//  different log formats, and the static-binding cliffhanger.
// ============================================================================

#include <iostream>
#include <string>
#include "logger.h"

int main()
{
    std::cout << "============================================================\n";
    std::cout << "  Chapter 24 — Inheritance: Report-Logger Family Demo\n";
    std::cout << "============================================================\n\n";

    // ── 1. Basic Logger ───────────────────────────────────────────────────────
    std::cout << "── 1. Basic Logger ─────────────────────────────────────────\n";
    {
        std::string ct, dt;
        Logger base { "system", ct, dt };

        std::cout << "  Construction trace: \"" << ct << "\"\n";
        std::cout << "  name(): " << base.name() << "\n";
        std::cout << "  log(\"boot\"):    " << base.log("boot") << "\n";
        std::cout << "  log(\"running\"): " << base.log("running") << "\n";
        // At end of block: destructor runs, dtorTrace updated.
    }
    // (dtorTrace goes out of scope with the Logger — demo only observes ctor side)

    std::cout << "\n";

    // ── 2. Construction order proof (notes 24.3) ──────────────────────────────
    std::cout << "── 2. Construction / destruction order (notes 24.3) ────────\n";
    {
        std::string ct, dt;
        {
            std::cout << "  Constructing TimestampLogger...\n";
            TimestampLogger tl { "audit", "[T]", ct, dt };
            std::cout << "  ctor trace: \"" << ct << "\"\n";
            std::cout << "  (base \"Logger+\" appears BEFORE derived \"Timestamp+\")\n";
        }
        std::cout << "  dtor trace: \"" << dt << "\"\n";
        std::cout << "  (derived \"Timestamp-\" appears BEFORE base \"Logger-\")\n";
    }

    std::cout << "\n";

    // ── 3. TimestampLogger::log() (notes 24.7) ────────────────────────────────
    std::cout << "── 3. TimestampLogger::log() redefinition (notes 24.7) ─────\n";
    {
        std::string ct, dt;
        TimestampLogger tl { "router", "[T]", ct, dt };
        std::cout << "  Direct call on TimestampLogger object:\n";
        std::cout << "    tl.log(\"connect\") = \"" << tl.log("connect") << "\"\n";
        std::cout << "  Tag \"[T]\" appears because the DERIVED log() runs.\n";
    }

    std::cout << "\n";

    // ── 4. CountingLogger (notes 24.5, 24.6) ─────────────────────────────────
    std::cout << "── 4. CountingLogger (notes 24.5 / 24.6) ───────────────────\n";
    {
        std::string ct, dt;
        CountingLogger cl { "counter", ct, dt };
        cl.log("startup");
        cl.log("shutdown");
        std::cout << "  After 2 log calls, linesLogged() = " << cl.linesLogged() << "\n";
        std::cout << "  (linesLogged() reads protected m_lineCount directly)\n";
    }

    std::cout << "\n";

    // ── 5. THE CLIFFHANGER — static binding (notes 24.7 CS6340 tie-in) ────────
    std::cout << "── 5. THE CLIFFHANGER: static binding (notes 24.7) ─────────\n";
    {
        std::string ct, dt;
        TimestampLogger tl { "router", "[T]", ct, dt };

        std::cout << "  Direct call on TimestampLogger OBJECT (derived log):\n";
        std::cout << "    tl.log(\"direct\") = \"" << tl.log("direct") << "\"\n";
        std::cout << "    -> tag \"[T]\" present: derived TimestampLogger::log() ran\n\n";

        std::cout << "  Call via processLog(Logger& l, msg) (static binding!):\n";
        std::string result = processLog(tl, "via_ref");
        std::cout << "    processLog(tl, \"via_ref\") = \"" << result << "\"\n";
        std::cout << "    -> NO tag \"[T]\": Logger::log() ran (static type = Logger&)\n\n";

        std::cout << "  The OBJECT is still a TimestampLogger, but the call\n";
        std::cout << "  resolved at COMPILE TIME using the declared type Logger.\n";
        std::cout << "  This is STATIC BINDING — the problem Chapter 25 solves\n";
        std::cout << "  with `virtual`. Add `virtual` to Logger::log() and the\n";
        std::cout << "  tag reappears. That is Chapter 25.\n";
    }

    std::cout << "\n============================================================\n";
    std::cout << "  End of demo. Run `make test-solution` to grade the solution.\n";
    std::cout << "============================================================\n";

    return 0;
}
