# Chapter 28 — Input and Output: The Report Engine

> Reinforces [`../../notes/chapter-28.md`](../../notes/chapter-28.md) · LearnCpp [Chapter 28](https://www.learncpp.com/)

## The project

Streams are one of C++'s most elegant ideas: a single interface — `<<`, `>>`,
`getline` — that works identically whether the bytes are going to the terminal,
a file, or a string in memory. This lab makes that abstraction **physical** by
building a "report engine": a small library that parses raw score lines, formats
aligned table rows, assembles a full report, writes it to a real file, and reads
it back — plus a utility that counts valid integers in a stream until it hits
junk or EOF.

You implement five functions in [`starter/report.cpp`](starter/report.cpp):

- **Task 1** — parse a `"name score"` line with `std::istringstream`
- **Task 2** — format one aligned row with `std::ostringstream` + `<iomanip>`
- **Task 3** — assemble the full multi-line table
- **Task 4** — write the table to a real file and read it back
- **Task 5** — count valid integers in any stream until EOF/junk

Every function exercises a distinct stream concept; together they cover the full
chapter. The CS6340 connection is direct: Lab 1 loads a seed corpus from a file
(`ifstream` + `getline`), writes coverage results to another file (`ofstream <<`),
and passes `std::ostream&` around so the same function works for both console
and file output. You are building exactly those habits here.

## Concepts practiced

- **Stream hierarchy** — `istream` / `ostream` as the uniform base; `cin`, `cout`,
  `ifstream`, `ofstream`, `istringstream`, and `ostringstream` all descend from
  the same base (notes 28.1). Accepting `std::istream&` or `std::ostream&` in
  a function makes it stream-polymorphic.
- **`std::istringstream`** — parse structured fields out of a `std::string`
  using the same `>>` operator you use on `std::cin` (notes 28.4)
- **`std::ostringstream`** — build a formatted `std::string` without manual
  concatenation; retrieve it with `.str()` (notes 28.4)
- **`<iomanip>` manipulators** — `std::left`, `std::right`, `std::setw`,
  `std::fixed`, `std::setprecision` for column-aligned table formatting;
  knowing which manipulators persist vs. reset (notes 28.3)
- **Stream states** — `goodbit`, `failbit`, `eofbit`; using the extraction
  result as a boolean (`if (in >> name >> score)`); the while-extraction idiom
  `while (in >> val)` (notes 28.5)
- **`std::ifstream` / `std::ofstream`** — file I/O with open-success check
  (`if (!file) return;`), `getline` on a file, sequential read/write (notes 28.6)
- Reused from earlier chapters: **structs** (Ch 13), **`std::vector`** / range-`for`
  (Ch 16), **`std::string`** manipulation (Ch 5), `const`-correctness (Ch 5)

## Your tasks

The starter compiles immediately but every function returns a **placeholder**, so
`make test` starts **RED**. Fill in the five `>>> YOUR CODE HERE <<<` blocks in
[`starter/report.cpp`](starter/report.cpp). They ramp from warm-up to capstone:

1. **Parse a score line (`std::istringstream` + state check).** `parseScoreLine(line)`
   creates a `std::istringstream` from the raw string, extracts the name token and
   double score with `>>`, checks the stream state, and validates the range
   `[0.0, 100.0]`. Returns a `ScoreRecord` with `ok = true` on success, `ok = false`
   on any failure (malformed input, out-of-range score). The key idiom is
   `if (in >> name >> score)` — using the extraction result as a boolean (notes 28.5).

2. **Format a table row (`std::ostringstream` + `<iomanip>`).** `formatRow(name, score)`
   builds the aligned row `"Ada         92.5"` (name left-justified in a 10-char
   field; score right-justified in a 6-char field with 1 decimal) using an
   `ostringstream` and the manipulators `std::left`, `std::right`, `std::setw`,
   `std::fixed`, `std::setprecision`. The tests assert exact character-for-character
   equality — the tricky part is knowing that `std::setw` resets after each
   insertion while `std::fixed` and `std::setprecision` persist (notes 28.3).

3. **Assemble the full table (`ostringstream`).** `buildReport(records)` writes a
   header row, a separator row, and one `formatRow` call per record into a single
   `ostringstream`, then returns `.str()`. Use `formatRow` for data rows; do not
   re-implement the alignment. An empty `records` vector produces only the 2-line
   header + separator.

4. **Round-trip through a real file (`ofstream` / `ifstream`).** `writeReport(path, report)`
   opens an `ofstream`, checks `if (!file) return;`, writes the report string.
   `readReportFirstLine(path)` opens an `ifstream`, checks open success, reads
   the first line with `std::getline`, and returns it (notes 28.6). The tests
   write to `tests/tmp_report.txt` and verify the header line comes back correctly.

5. **Stream-state loop (`while (in >> val)`).** `countValidInts(in)` counts
   integers extracted from any `std::istream&` until the first extraction failure
   (bad token or EOF). The canonical one-liner idiom: `while (in >> val) ++count;`.
   Do NOT call `in.clear()` — the function stops at the first failure, per the
   stream-state contract (notes 28.5).

You may **not** edit [`report.h`](report.h) or anything in `tests/` — those are
the contract and the grader.

## Constraints

- **Allowed:** `std::istringstream`, `std::ostringstream`, `std::ifstream`,
  `std::ofstream`, `<iomanip>` manipulators, `std::getline`, `if (!stream)` open
  checks, `std::vector` + range-`for`, `std::string`, `struct ScoreRecord`,
  `while` / `for` loops, `if`/`else`, `double` arithmetic and comparisons.
- **Required idioms:** `if (in >> name >> score)` for simultaneous extraction +
  state check; `while (in >> val)` for the counting loop; `if (!file) return;`
  after every file open; `out.str()` to retrieve ostringstream contents.
- **Forbidden (not yet taught or not the point):** raw C-style I/O (`printf`,
  `scanf`, `fopen`, `fclose`), manual string-padding with loops (use `std::setw`),
  `std::stringstream` where `istringstream` or `ostringstream` is specific enough
  (prefer the more specific type — it communicates intent), calling `in.clear()`
  inside `countValidInts` (it must stop at the first failure, not recover).
- **File paths:** `writeReport` and `readReportFirstLine` use the path as given;
  the tests pass `"tests/tmp_report.txt"` which is relative to the chapter-28/
  directory (the Makefile's working directory). Do not hard-code paths.

## Build & run

```sh
make            # compile-check starter/report.cpp (should work immediately)
make test       # grade your code  ->  RED until the TASK blocks are filled in
make solution   # run the grader against the reference (peek if stuck)
make clean      # remove build artifacts (including tests/tmp_report.txt)
```

`make test` is the grader — it calls every function across many inputs. There
is no interactive `make run` for this lab (streams are the output; the grader
shows everything you need).

## Success criteria

`make test` prints **PASS ✅  all report-engine checks passed.** and exits 0.
Until then it prints one `FAIL: …  @line N` per broken check. The grader covers:

- `parseScoreLine`: valid lines, leading/trailing whitespace (>> skips it), boundary
  scores 0.0 and 100.0, out-of-range scores (101.0, −1.0), malformed input
  (missing score, non-numeric token), and empty string — all must return the
  correct `ok` flag.
- `formatRow`: exact character-for-character string equality for names of varying
  length (3, 4, 10 chars), scores from 0.0 to 100.0 — the alignment must be
  pixel-perfect.
- `buildReport`: exact 2-line header for empty input, presence of data rows from
  `formatRow`, and a full exact-string match for a 2-record report.
- `writeReport` / `readReportFirstLine`: first line of the written file matches
  the expected header; non-existent file path returns `""`; invalid write path
  does not crash.
- `countValidInts`: all-valid stream (3), junk mid-stream stopping before the
  next valid int (1), junk at start (0), empty stream (0), negative ints (valid),
  10 ints, junk at end (stops correctly), whitespace-only stream (0).

## Hints

<details><summary>Task 1 — istringstream construction and the if-extraction idiom</summary>

```cpp
std::istringstream in { line };          // "wrap" the string in an input stream
std::string name {};
double      score {};

if (in >> name >> score)                 // true if BOTH extractions succeeded
{
    if (score >= 0.0 && score <= 100.0)
        return ScoreRecord{ true, name, score };
}
return ScoreRecord{};                    // default has ok = false
```

`std::istringstream` is in `<sstream>`. Once constructed, it behaves exactly like
`std::cin` but draws from the string. The chained extraction returns the stream,
which evaluates to `true` only if no error flag is set (notes 28.5).
</details>

<details><summary>Task 2 — manipulator order matters; setw resets; fixed persists</summary>

```cpp
std::ostringstream out {};

out << std::left << std::setw(10) << name;                       // name field
out << std::right << std::fixed << std::setprecision(1)
    << std::setw(6) << score;                                    // score field

return out.str();
```

`std::setw(N)` applies only to the immediately following insertion and then
resets to 0 (notes 28.3). `std::fixed` and `std::setprecision(1)` stay active
for all subsequent floating-point insertions — harmless here since we output
only one number per call.
</details>

<details><summary>Task 3 — header and separator widths</summary>

```cpp
std::ostringstream out {};

// Header: "Name" left in 10-char field, "Score" right in 6-char field
out << std::left  << std::setw(10) << "Name"
    << std::right << std::setw(6)  << "Score" << '\n';

// Separator: 9-dash name part, space, 6-dash score part
out << std::left  << std::setw(9) << "---------"
    << ' '
    << std::right << std::setw(6) << "------" << '\n';

// Data rows
for (const ScoreRecord& r : records)
    out << formatRow(r.name, r.score) << '\n';

return out.str();
```

The header uses `setw(10)` for name (same as `formatRow`) but only `setw(6)`
for "Score" because the word is 5 chars — one padding space on the left.
The separator uses `setw(9)` for the dashes (the name field is 10, but 9 dashes
look cleaner) then a literal space before the 6-dash score divider.
</details>

<details><summary>Task 4 — ofstream write, ifstream first-line read</summary>

```cpp
void writeReport(const std::string& path, const std::string& report)
{
    std::ofstream file { path };
    if (!file) return;             // open failed — wrong path, permissions, etc.
    file << report;                // same << as std::cout, writes to disk
}

std::string readReportFirstLine(const std::string& path)
{
    std::ifstream file { path };
    if (!file) return "";          // open failed
    std::string line {};
    std::getline(file, line);      // reads up to (not including) the '\n'
    return line;
}
```

The destructor of `ofstream` flushes and closes the file — no explicit
`.close()` needed. `std::getline` on an `ifstream` works identically to
`std::getline(std::cin, line)` — same function, different stream kind.
</details>

<details><summary>Task 5 — the while-extraction idiom</summary>

```cpp
int count { 0 };
int val   {};
while (in >> val)   // extract; if it succeeds, val is updated and condition is true
    ++count;
return count;
```

When `>>` fails (bad token OR EOF), it sets `failbit` (and `eofbit` for EOF).
The stream then evaluates to `false` in a boolean context — `while` exits.
All subsequent `>>` calls on the same failed stream also fail immediately
without changing `val`, so hitting one bad token stops the entire count.
(notes 28.5)
</details>

## Stretch goals (optional — most need later chapters)

- Make `buildReport` sort the rows by score (descending) before printing —
  requires `std::sort` + a lambda comparator (Chapter 18/20).
- Add a `parseReport(std::istream&)` inverse: read a report file back into a
  `std::vector<ScoreRecord>` by parsing each line after the 2-line header —
  this deepens the `ifstream` + `getline` + `istringstream` combination.
- Handle the `>>` / `getline` mixing trap (notes 28.2): write a `readCsvLine`
  function that reads a full CSV line with `std::getline` and then uses an
  `istringstream` to split on commas — a real-world pattern for seed-corpus files.
- Write a `countValidIntsWithRecovery` variant that skips bad tokens and keeps
  counting (using `in.clear()` + `in.ignore()` — notes 28.5). Compare its
  behavior to `countValidInts` to see the state-management difference clearly.
- Replace the file path `std::string` parameters with `std::filesystem::path`
  (C++17 — a preview of the filesystem library, formally beyond Ch 28).
