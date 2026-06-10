// Chapter 28 — Input and Output · Project: The Report Engine   (GRADER)
// ─────────────────────────────────────────────────────────────────────────────
// A tiny no-framework unit-test harness (same style as drills/CLAUDE.md spec).
// It includes ../report.h and calls each API function across many inputs.
// Each CHECK that fails prints its expression and line number. Any failure ->
// non-zero exit -> `make test` is RED.
//
// The Makefile links this file against starter/report.cpp for `make test` and
// against solution/report.cpp for `make test-solution`.
//
// File I/O NOTE: Task 4 writes/reads tests/tmp_report.txt (relative to the
// Makefile's working directory, which is the chapter-28/ folder). `make clean`
// removes this file.

#include <iostream>
#include <sstream>   // std::istringstream
#include <string>
#include <vector>
#include "../report.h"

static int fails = 0;

// CHECK: assert a boolean condition; on failure, report what and where.
#define CHECK(cond) \
    do { if(!(cond)){ std::cerr << "FAIL: " #cond "  @line " << __LINE__ << "\n"; ++fails; } } while(0)

int main()
{
    // ── Task 1: parseScoreLine ────────────────────────────────────────────────
    // Normal cases
    {
        ScoreRecord r { parseScoreLine("Ada 92.5") };
        CHECK(r.ok    == true);
        CHECK(r.name  == "Ada");
        CHECK(r.score == 92.5);
    }
    {
        ScoreRecord r { parseScoreLine("Bob 88.0") };
        CHECK(r.ok    == true);
        CHECK(r.name  == "Bob");
        CHECK(r.score == 88.0);
    }
    // Leading / trailing whitespace — >> skips it (notes 28.2)
    {
        ScoreRecord r { parseScoreLine("   Zhao   71.3   ") };
        CHECK(r.ok    == true);
        CHECK(r.name  == "Zhao");
        CHECK(r.score == 71.3);
    }
    // Boundary scores: exactly 0.0 and 100.0 are in range
    {
        ScoreRecord r { parseScoreLine("Min 0.0") };
        CHECK(r.ok    == true);
        CHECK(r.score == 0.0);
    }
    {
        ScoreRecord r { parseScoreLine("Max 100.0") };
        CHECK(r.ok    == true);
        CHECK(r.score == 100.0);
    }
    // Edge: score out of range -> ok == false
    {
        ScoreRecord r { parseScoreLine("Over 101.0") };
        CHECK(r.ok == false);
    }
    {
        ScoreRecord r { parseScoreLine("Neg -1.0") };
        CHECK(r.ok == false);
    }
    // Edge: malformed lines (missing score, non-numeric) -> ok == false
    {
        ScoreRecord r { parseScoreLine("NoScore") };
        CHECK(r.ok == false);
    }
    {
        ScoreRecord r { parseScoreLine("BadScore abc") };
        CHECK(r.ok == false);
    }
    // Edge: trailing junk AFTER a valid name+score is ignored -> ok == true.
    // The contract (report.h TASK 1) only needs the first two fields; >> stops
    // after the score and never inspects the rest of the line. (notes 28.4)
    {
        ScoreRecord r { parseScoreLine("Cleo 64.0 extra junk 99") };
        CHECK(r.ok    == true);
        CHECK(r.name  == "Cleo");
        CHECK(r.score == 64.0);
    }
    // Edge: empty line -> ok == false
    {
        ScoreRecord r { parseScoreLine("") };
        CHECK(r.ok == false);
    }

    // ── Task 2: formatRow — exact-string asserts ──────────────────────────────
    // Layout: name left-justified in 10-char field, score right-justified in
    // 6-char field with 1 decimal place. Total row width = 16 chars.
    //
    // "Ada" + 7 spaces = 10 chars (name field, left in setw(10))
    // "  92.5"           = 6 chars (score field, right-aligned: 2 spaces + "92.5")
    // -> "Ada         92.5"  (16 chars total; 9 spaces between "Ada" and "92.5")
    //
    // Values chosen to be exactly representable in decimal with 1dp:
    //   92.5 = 185/2, 88.0 = exact integer, 71.3 rounds cleanly to "71.3"
    CHECK(formatRow("Ada",  92.5) == "Ada         92.5");   // Ada+7sp+2sp+92.5
    CHECK(formatRow("Bob",  88.0) == "Bob         88.0");
    CHECK(formatRow("Zhao", 71.3) == "Zhao        71.3");
    // Score that uses all 6 chars (100.0 = 5 chars, 1 padding space)
    CHECK(formatRow("Ali",  100.0) == "Ali        100.0");
    // Name exactly 10 chars -> no padding after name
    CHECK(formatRow("Aleksandra", 75.0) == "Aleksandra  75.0");
    // Minimum score: "   0.0" in 6-char right field = 3 spaces + 0.0
    CHECK(formatRow("X",   0.0) == "X            0.0");

    // ── Task 3: buildReport ───────────────────────────────────────────────────
    // Column layout constants (must match formatRow):
    //   Name column: 10 chars wide, left-aligned
    //   Score column: 6 chars wide, right-aligned
    //
    // Header: "Name      " (10 left) + " Score" (6 right) -> "Name       Score"
    // Sep:    "---------" (9 chars, left in setw(9)) + " " + "------" (6 right)
    //      -> "--------- ------"
    //
    // Empty records -> just 2-line header
    {
        std::vector<ScoreRecord> empty {};
        std::string rpt { buildReport(empty) };
        std::string expected { "Name       Score\n--------- ------\n" };
        CHECK(rpt == expected);
    }
    // Two records -> header + separator + 2 data rows
    {
        std::vector<ScoreRecord> recs {
            { true, "Ada", 92.5 },
            { true, "Bob", 88.0 }
        };
        std::string rpt { buildReport(recs) };
        // Header present
        CHECK(rpt.find("Name") != std::string::npos);
        CHECK(rpt.find("Score") != std::string::npos);
        // Data rows use formatRow output
        CHECK(rpt.find(formatRow("Ada", 92.5)) != std::string::npos);
        CHECK(rpt.find(formatRow("Bob", 88.0)) != std::string::npos);
        // Each data row is followed by '\n'
        std::string ada_row { formatRow("Ada", 92.5) + '\n' };
        CHECK(rpt.find(ada_row) != std::string::npos);
    }
    // Single record
    {
        std::vector<ScoreRecord> recs { { true, "Zhao", 71.3 } };
        std::string rpt { buildReport(recs) };
        CHECK(rpt.find("Zhao") != std::string::npos);
        CHECK(rpt.find(formatRow("Zhao", 71.3)) != std::string::npos);
    }
    // Full exact-string check for a two-record report
    {
        std::vector<ScoreRecord> recs {
            { true, "Ada", 92.5 },
            { true, "Bob", 88.0 }
        };
        std::string expected {
            "Name       Score\n"
            "--------- ------\n"
            "Ada         92.5\n"
            "Bob         88.0\n"
        };
        CHECK(buildReport(recs) == expected);
    }

    // ── Task 4: writeReport / readReportFirstLine ─────────────────────────────
    // File is written to tests/tmp_report.txt (relative to chapter-28/ where
    // make runs). `make clean` removes it.
    {
        std::string path { "tests/tmp_report.txt" };

        std::vector<ScoreRecord> recs {
            { true, "Ada", 92.5 },
            { true, "Bob", 88.0 }
        };
        std::string report { buildReport(recs) };

        // Write the report to the file
        writeReport(path, report);

        // Read back the first line and verify it matches the header
        // std::getline returns the line WITHOUT the trailing '\n'
        std::string first_line { readReportFirstLine(path) };
        CHECK(first_line == "Name       Score");

        // Edge: non-existent path -> readReportFirstLine returns ""
        std::string no_line { readReportFirstLine("tests/no_such_file_xyz.txt") };
        CHECK(no_line == "");
    }
    // Edge: writeReport to non-writable path -> should not crash (silent no-op)
    {
        // Passing an unwritable path; the function must return without throwing
        writeReport("/no_such_dir_xyz/report.txt", "test");
        // If we got here, it did not crash or throw — that is the test
        CHECK(true);
    }

    // ── Task 5: countValidInts ────────────────────────────────────────────────
    // Normal: all valid integers
    {
        std::istringstream in { "10 20 30" };
        CHECK(countValidInts(in) == 3);
    }
    // Junk mid-stream: stops at first non-int (notes 28.5 — failbit propagates)
    {
        std::istringstream in { "10 abc 30" };
        CHECK(countValidInts(in) == 1);   // "10" ok, "abc" fails -> stop before "30"
    }
    // Junk at start: zero valid
    {
        std::istringstream in { "abc 10 20" };
        CHECK(countValidInts(in) == 0);
    }
    // Empty stream: zero valid
    {
        std::istringstream in { "" };
        CHECK(countValidInts(in) == 0);
    }
    // Single valid int
    {
        std::istringstream in { "42" };
        CHECK(countValidInts(in) == 1);
    }
    // Negative integers are valid (notes 28.2 — >> parses sign)
    {
        std::istringstream in { "-5 -10 7" };
        CHECK(countValidInts(in) == 3);
    }
    // Many ints in a row
    {
        std::istringstream in { "1 2 3 4 5 6 7 8 9 10" };
        CHECK(countValidInts(in) == 10);
    }
    // Junk at end (after valid ints)
    {
        std::istringstream in { "100 200 bad" };
        CHECK(countValidInts(in) == 2);
    }
    // Whitespace only
    {
        std::istringstream in { "   " };
        CHECK(countValidInts(in) == 0);
    }

    // ── Summary ───────────────────────────────────────────────────────────────
    if (!fails)
        std::cout << "PASS \xE2\x9C\x85  all report-engine checks passed.\n";
    else
        std::cerr << "\nFAIL \xE2\x9D\x8C  " << fails
                  << " check(s) failed -- fill in the TASK blocks in report.cpp.\n";

    return fails ? 1 : 0;
}
