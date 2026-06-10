// Chapter 28 — Input and Output · Project: The Report Engine  (REFERENCE SOLUTION)
// ─────────────────────────────────────────────────────────────────────────────
// One complete, correct, warning-clean implementation of ../report.h.
// Peek only after you've taken a real swing at starter/report.cpp — the learning
// is in wiring the stream operations yourself, then comparing.
//
// Every function demonstrates the UNIFORM-ABSTRACTION principle:
//   • Tasks 1-3: string streams (istringstream/ostringstream) — treat a std::string
//     like a stream, parse from it or build into it, no file system involved.
//   • Task 4: file streams (ifstream/ofstream) — the exact same << / >> / getline
//     operations, but now they go to/from disk.
//   • Task 5: accepts std::istream& — works with ANY stream kind the caller supplies
//     (istringstream in tests, std::cin in a real program, ifstream for files).
//
// CS6340 / Lab 1 connection: this is exactly the pattern for loading a seed corpus
// (ifstream -> getline) and logging results (ofstream << formatted line).

#include "../report.h"

#include <fstream>   // std::ifstream, std::ofstream         (notes 28.6)
#include <iomanip>   // std::setw, std::setprecision, etc.   (notes 28.3)
#include <sstream>   // std::istringstream, std::ostringstream (notes 28.4)

// ─── TASK 1: parseScoreLine — std::istringstream extraction + state check ────
//
// std::istringstream is the "read-from-a-string" stream (notes 28.4).
// Constructing one with a std::string sets its internal buffer to that string.
// After that, all the usual input-stream operations (>>, getline, .fail(), etc.)
// work exactly as if you were reading from std::cin or an ifstream.
//
// The key state-check idiom (notes 28.5):
//   if (in >> name >> score)
// This evaluates the expression in a boolean context. The expression:
//   1. Tries to extract `name` (a string token, stops at whitespace).
//   2. Tries to extract `score` (a double).
//   3. Returns the stream — which converts to `true` if BOTH succeeded.
// If EITHER extraction fails (malformed line, non-numeric score, EOF mid-line),
// the whole condition is false and we fall into the else branch.
ScoreRecord parseScoreLine(const std::string& line)
{
    // Create an input string stream from the raw line.
    // Think of `in` as a tiny std::cin that reads from `line` instead of the
    // keyboard — the same extraction operators apply. (notes 28.4)
    std::istringstream in { line };

    std::string name  {};
    double      score { 0.0 };

    // The single most important idiom in this lab: use the extraction result
    // as a boolean. If the stream enters a failed state at ANY point during
    // this chained extraction, the whole condition is false. (notes 28.5)
    if (in >> name >> score)
    {
        // Additional RANGE validation: stream state can be good but the value
        // semantically invalid for our domain. Type validation (parse) and
        // range/semantic validation are separate concerns. (notes 28.5)
        if (score >= 0.0 && score <= 100.0)
            return ScoreRecord{ true, name, score };
    }

    // Anything not explicitly returned above is a failed parse.
    return ScoreRecord{};
}

// ─── TASK 2: formatRow — std::ostringstream + <iomanip> ──────────────────────
//
// std::ostringstream is the "write-to-a-string" stream (notes 28.4).
// We build the formatted row in it, then return the accumulated string via .str().
// This avoids manual string concatenation with ugly std::to_string calls.
//
// <iomanip> manipulator cheat-sheet (notes 28.3):
//   std::left            — left-align following insertions
//   std::right           — right-align following insertions (default for numbers)
//   std::setw(N)         — NEXT insertion fills a field of exactly N chars
//                          (RESETS after each insertion — must re-apply every time)
//   std::fixed           — use fixed-point decimal notation (PERSISTS)
//   std::setprecision(N) — N digits after the decimal point when std::fixed is
//                          active (PERSISTS)
//
// Expected exact output (tests assert char-for-char) — each row is 16 chars:
//   "Ada         92.5"   (name "Ada" left in setw(10) = "Ada"+7 spaces;
//                         score right in setw(6) = 2 spaces+"92.5" -> 9-space gap)
//   "Bob         88.0"
//   "Zhao        71.3"   ("Zhao"+6 spaces, then "  71.3")
std::string formatRow(const std::string& name, double score)
{
    std::ostringstream out {};

    // ── Name: left-aligned in a 10-char wide field ───────────────────────────
    // std::left + std::setw(10) together mean: pad name on the RIGHT to 10 chars.
    // std::setw resets after this insertion, so it won't affect the score below.
    out << std::left << std::setw(10) << name;

    // ── Score: right-aligned in a 6-char field, 1 decimal place ─────────────
    // std::right puts any padding on the LEFT (so the number hugs the right wall).
    // std::fixed + std::setprecision(1) pin exactly one digit after the decimal.
    // Both std::fixed and std::setprecision(1) persist, but that is fine — we
    // only format one number per call.
    out << std::right << std::fixed << std::setprecision(1) << std::setw(6) << score;

    return out.str();
}

// ─── TASK 3: buildReport — assemble the whole table ──────────────────────────
//
// One ostringstream, three phases: header, separator, data rows.
// Using a single oss avoids repeatedly concatenating heap strings.
//
// IMPORTANT MANIPULATOR ORDERING NOTE:
// After the first call to formatRow, the oss has std::fixed + std::setprecision(1)
// active on it (those manipulators persist). The header and separator are written
// BEFORE any formatRow call, so they aren't affected. If you were to write the
// header AFTER data rows, you'd need to save/restore the flags — which is why
// we write the header first.
std::string buildReport(const std::vector<ScoreRecord>& records)
{
    std::ostringstream out {};

    // ── Phase 1: header row ───────────────────────────────────────────────────
    // "Name      " (10-char left field) + "     Score" (6-char right field)
    // We apply the same layout widths as the data rows so columns align.
    out << std::left  << std::setw(10) << "Name"
        << std::right << std::setw(6)  << "Score" << '\n';

    // ── Phase 2: separator row ────────────────────────────────────────────────
    // 9 dashes for the name column + space + 6 dashes for the score column.
    // (std::setw(9) for 9-char left field, then 1 literal space, then setw(6)
    // for the 6-char score dashes field.)
    out << std::left  << std::setw(9) << "---------"
        << ' '
        << std::right << std::setw(6) << "------" << '\n';

    // ── Phase 3: data rows ────────────────────────────────────────────────────
    // Reuse formatRow — one call per record, one '\n' per line.
    // Range-for (Chapter 16) over the vector of ScoreRecords.
    for (const ScoreRecord& r : records)
        out << formatRow(r.name, r.score) << '\n';

    return out.str();
}

// ─── TASK 4a: writeReport — std::ofstream ────────────────────────────────────
//
// std::ofstream opens a file for writing (notes 28.6). Constructing it with a
// path tries to open that file immediately; if the file doesn't exist it is
// created; if it does exist it is truncated (default std::ios::trunc behavior).
// The destructor automatically flushes and closes the file — no close() needed.
//
// CRITICAL pattern: ALWAYS check `if (!file)` after opening. On autograders and
// CI boxes, missing permissions or wrong working directories cause silent data
// loss if you don't check. (notes 28.6)
void writeReport(const std::string& path, const std::string& report)
{
    std::ofstream file { path };

    if (!file)
        return;   // could not open: wrong path, permissions, etc. -> silent bail

    // The << operator on an ofstream behaves identically to std::cout << or
    // ostringstream <<. The same stream abstraction at work. (notes 28.1)
    file << report;

    // `file` goes out of scope here; its destructor calls close(), which flushes
    // any buffered data to the OS. If you ever need to reuse the file handle in
    // the same scope, call file.close() explicitly.
}

// ─── TASK 4b: readReportFirstLine — std::ifstream + std::getline ─────────────
//
// std::ifstream opens a file for reading. std::getline(file, line) reads one
// line (up to but not including the '\n'), exactly as getline(std::cin, line).
// This is the stream-polymorphism payoff: the SAME getline function that reads
// interactive input also reads from files. (notes 28.2, 28.6)
std::string readReportFirstLine(const std::string& path)
{
    std::ifstream file { path };

    if (!file)
        return "";   // could not open

    std::string line {};
    std::getline(file, line);   // read just the first line (the header row)
    return line;
}

// ─── TASK 5: countValidInts — stream-state loop ──────────────────────────────
//
// The idiomatic stream-state loop (notes 28.5):
//
//   while (in >> val) { ... }
//
// When >> fails:
//   • EOF reached -> eofbit + failbit set -> stream converts to false -> loop exits
//   • Bad token   -> failbit set          -> stream converts to false -> loop exits
//
// Once failbit is set, every subsequent >> on the SAME stream immediately returns
// false without modifying `val`. This means if we see "10 abc 30", we count 10
// (success), then stop at "abc" (failure) — we never reach 30. That is the
// CORRECT behavior for a "count valid ints until first failure" function.
//
// CS6340 insight: the same pattern lets you drain a fuzzer's coverage-count list
// from a file until the format changes or the file ends — one loop, no bookkeeping.
int countValidInts(std::istream& in)
{
    int count { 0 };
    int val   {};

    // The loop condition IS the extraction. Beautiful C++ idiom:
    // "keep extracting as long as you can; stop the moment you can't."
    while (in >> val)
        ++count;

    return count;
}
