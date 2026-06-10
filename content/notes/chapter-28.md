# Chapter 28 — Input and Output

> Source: <https://www.learncpp.com/> (Chapter 28)
>
> Goal for CS6340: read and write C++ stream code confidently enough to load
> seeds, print diagnostics, validate input, use string streams, and work with
> files while building Lab 1-style tooling.

## Contents

- [28.1 — Input and output streams](#281--input-and-output-streams)
- [28.2 — Input with `istream`](#282--input-with-istream)
- [28.3 — Output with `ostream` and `ios`](#283--output-with-ostream-and-ios)
- [28.4 — Stream classes for strings](#284--stream-classes-for-strings)
- [28.5 — Stream states and input validation](#285--stream-states-and-input-validation)
- [28.6 — Basic file I/O](#286--basic-file-io)
- [28.7 — Random file I/O](#287--random-file-io)
- [Chapter 28 summary](#chapter-28-summary)
- [CS6340 patterns](#cs6340-patterns)
- [Mini drill](#mini-drill)

## 28.1 — Input and output streams

C++ uses streams for input and output.

A stream is an abstraction for a sequence of characters flowing from a source or
to a destination.

```text
input stream:

keyboard/file/string/network-ish source
        |
        v
     program

output stream:

     program
        |
        v
terminal/file/string/log-ish destination
```

You already know the most common standard streams:

```cpp
std::cout // standard output
std::cin  // standard input
std::cerr // standard error, unbuffered or less buffered
std::clog // standard logging output, buffered
```

Include:

```cpp
#include <iostream>
```

### Output stream example

```cpp
#include <iostream>

int main()
{
    std::cout << "Hello\n";
    std::cerr << "Error message\n";

    return 0;
}
```

`operator<<` inserts data into an output stream.

Read:

```cpp
std::cout << value;
```

as:

```text
send value into std::cout
```

### Input stream example

```cpp
#include <iostream>

int main()
{
    int x {};

    std::cin >> x;

    std::cout << "You entered " << x << '\n';

    return 0;
}
```

`operator>>` extracts data from an input stream.

Read:

```cpp
std::cin >> x;
```

as:

```text
read from std::cin into x
```

### Stream class family

Common stream classes:

| Kind | Header | Use |
| --- | --- | --- |
| `std::istream` | `<istream>` / `<iostream>` | input |
| `std::ostream` | `<ostream>` / `<iostream>` | output |
| `std::iostream` | `<iostream>` | both input and output |
| `std::ifstream` | `<fstream>` | input file stream |
| `std::ofstream` | `<fstream>` | output file stream |
| `std::fstream` | `<fstream>` | input/output file stream |
| `std::istringstream` | `<sstream>` | input from string |
| `std::ostringstream` | `<sstream>` | output to string |
| `std::stringstream` | `<sstream>` | input/output string stream |

The inheritance relationship is roughly:

```text
ios_base
   |
 basic_ios
   |
   +-- basic_istream  -> istream, ifstream, istringstream
   |
   +-- basic_ostream  -> ostream, ofstream, ostringstream
```

You do not need to memorize the full hierarchy. The practical lesson is that
many stream operations are shared across terminal, file, and string streams.

### Why streams are useful

The same output function can write to console, file, or string:

```cpp
#include <ostream>
#include <string>

void writeResult(std::ostream& out, const std::string& seed, bool crashed)
{
    out << seed << ',' << crashed << '\n';
}
```

Then:

```cpp
writeResult(std::cout, "abc", false);
```

or:

```cpp
std::ofstream file { "results.csv" };
writeResult(file, "abc", false);
```

or:

```cpp
std::ostringstream text {};
writeResult(text, "abc", false);
std::string line { text.str() };
```

This is a clean design trick: depend on `std::ostream&` when you only need to
write somewhere.

## 28.2 — Input with `istream`

`std::istream` is the base input stream type behind `std::cin`,
`std::ifstream`, and `std::istringstream`.

The extraction operator `>>` reads formatted input.

### Formatted extraction skips leading whitespace

```cpp
int x {};
std::cin >> x;
```

If the user enters:

```text
   42
```

the stream skips leading whitespace and extracts `42`.

For strings:

```cpp
std::string word {};
std::cin >> word;
```

Input:

```text
hello world
```

extracts only:

```text
hello
```

because formatted string extraction stops at whitespace.

### Reading a full line with `std::getline`

Use `std::getline` when spaces matter:

```cpp
#include <iostream>
#include <string>

int main()
{
    std::string line {};

    std::getline(std::cin, line);

    std::cout << "line: " << line << '\n';

    return 0;
}
```

Input:

```text
hello world
```

stores:

```text
hello world
```

### Mixing `>>` and `getline`

This is a classic trap.

```cpp
int count {};
std::string name {};

std::cin >> count;
std::getline(std::cin, name);
```

If input is:

```text
3
Jeremy
```

`std::cin >> count` reads `3` but leaves the newline in the stream. Then
`getline` sees that newline immediately and returns an empty string.

ASCII:

```text
input buffer before extraction:

[3][\n][J][e][r][e][m][y][\n]

after std::cin >> count:

[\n][J][e][r][e][m][y][\n]
 ^
 getline stops here
```

Fix with `std::ws`:

```cpp
std::cin >> count;
std::getline(std::cin >> std::ws, name);
```

`std::ws` consumes leading whitespace before `getline`.

### `get`, `peek`, and `ignore`

`get()` reads one character.

```cpp
char ch {};
std::cin.get(ch);
```

`peek()` looks at the next character without extracting it.

```cpp
char next { static_cast<char>(std::cin.peek()) };
```

`ignore()` discards characters.

```cpp
std::cin.ignore(1000, '\n'); // discard up to newline
```

For a safer maximum:

```cpp
#include <limits>

std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
```

This commonly appears after bad input:

```cpp
std::cin.clear();
std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
```

`clear()` resets error flags; `ignore()` removes leftover bad characters.

### Input function pattern

For reusable input code:

```cpp
#include <istream>
#include <string>

bool readSeed(std::istream& in, std::string& seed)
{
    return static_cast<bool>(std::getline(in, seed));
}
```

This works with:

```cpp
readSeed(std::cin, seed);
```

and:

```cpp
std::ifstream file { "seeds.txt" };
readSeed(file, seed);
```

and:

```cpp
std::istringstream text { "abc\nxyz\n" };
readSeed(text, seed);
```

That is the same stream-polymorphism idea from 28.1.

## 28.3 — Output with `ostream` and `ios`

`std::ostream` is the output stream type behind `std::cout`,
`std::ofstream`, and `std::ostringstream`.

Use `operator<<` to insert values into the stream.

```cpp
std::cout << "seed=" << seed << " crashed=" << crashed << '\n';
```

### Chaining works left to right

This:

```cpp
std::cout << "x=" << x << '\n';
```

works because each insertion returns the stream:

```text
((std::cout << "x=") << x) << '\n'
```

### `std::endl` vs `'\n'`

Prefer `'\n'` for normal line breaks.

```cpp
std::cout << "done\n";
```

`std::endl` writes a newline and flushes the stream:

```cpp
std::cout << "done" << std::endl;
```

Flushing can be useful before a crash or when interactive output must appear
immediately, but it can hurt performance in loops.

For fuzzing-style loops:

```cpp
for (const std::string& seed : seeds)
{
    std::cout << seed << '\n'; // better than std::endl in a hot loop
}
```

### Formatting numbers

Include:

```cpp
#include <iomanip>
```

Width:

```cpp
std::cout << std::setw(8) << 42 << '\n';
```

Precision:

```cpp
double ratio { 1.0 / 3.0 };

std::cout << std::setprecision(3) << ratio << '\n';
```

Fixed decimals:

```cpp
std::cout << std::fixed << std::setprecision(2) << 3.14159 << '\n';
```

Boolean names:

```cpp
std::cout << std::boolalpha << true << '\n';  // true
std::cout << std::noboolalpha << true << '\n'; // 1
```

Base formatting:

```cpp
std::cout << std::dec << 31 << '\n'; // 31
std::cout << std::hex << 31 << '\n'; // 1f
std::cout << std::oct << 31 << '\n'; // 37
```

### Some manipulators persist

Some formatting settings remain on the stream.

```cpp
std::cout << std::hex << 31 << '\n';
std::cout << 31 << '\n'; // still hex unless changed back
std::cout << std::dec;
```

`std::setw`, however, usually applies only to the next formatted output item.

Practical habit:

```cpp
std::cout << std::dec; // restore after hex diagnostics
```

### Output function pattern

Design output functions to accept `std::ostream&`:

```cpp
#include <ostream>
#include <string>

struct Result
{
    std::string seed;
    bool crashed {};
    int coverage {};
};

void writeResult(std::ostream& out, const Result& result)
{
    out << result.seed << ','
        << std::boolalpha << result.crashed << ','
        << result.coverage << '\n';
}
```

Then test it with a string stream before using files:

```cpp
std::ostringstream out {};
writeResult(out, Result { "abc", false, 12 });

std::cout << out.str();
```

## 28.4 — Stream classes for strings

String streams let you treat a `std::string` as a stream.

Include:

```cpp
#include <sstream>
```

Types:

```cpp
std::istringstream  // read from a string
std::ostringstream  // write to a string
std::stringstream   // read and write a string
```

### `std::istringstream`

Use it to parse text.

```cpp
#include <iostream>
#include <sstream>
#include <string>

int main()
{
    std::string line { "abc 12 1" };
    std::istringstream in { line };

    std::string seed {};
    int coverage {};
    bool crashed {};

    in >> seed >> coverage >> crashed;

    std::cout << seed << ' ' << coverage << ' ' << crashed << '\n';

    return 0;
}
```

This is useful when each line has structured fields.

ASCII:

```text
string line:

"abc 12 1"
   |
   v
istringstream
   |
   +--> seed = "abc"
   +--> coverage = 12
   +--> crashed = true
```

### `std::ostringstream`

Use it to build text.

```cpp
#include <sstream>
#include <string>

std::string makeReportLine(const std::string& seed, bool crashed, int coverage)
{
    std::ostringstream out {};

    out << seed << ',' << std::boolalpha << crashed << ',' << coverage;

    return out.str();
}
```

This avoids manual string concatenation with many conversions.

### `std::stringstream`

Use it when you need both input and output on a string buffer.

```cpp
std::stringstream stream {};

stream << "42";

int value {};
stream >> value;
```

For simple code, prefer the more specific type:

- `istringstream` for reading
- `ostringstream` for writing

Specific types communicate intent.

### Clearing and reusing string streams

If you reuse a string stream, reset both state and contents.

```cpp
std::stringstream stream {};

stream << "bad";

int value {};
stream >> value; // fails

stream.clear();  // reset error flags
stream.str("");  // reset stored string

stream << "123";
stream >> value;
```

For beginner code, it is often simpler to create a new stream when parsing a new
line:

```cpp
for (const std::string& line : lines)
{
    std::istringstream in { line };
    // parse line
}
```

## 28.5 — Stream states and input validation

Streams track state flags.

Important states:

| State | Meaning |
| --- | --- |
| `goodbit` | no error |
| `eofbit` | end of input reached |
| `failbit` | formatted extraction failed |
| `badbit` | serious stream error |

You usually inspect a stream through boolean context:

```cpp
if (std::cin)
{
    // stream is okay
}
```

or extraction result:

```cpp
int value {};

if (std::cin >> value)
{
    // read succeeded
}
else
{
    // read failed
}
```

### Failed extraction leaves input behind

If the program expects an integer:

```cpp
int value {};
std::cin >> value;
```

and the user enters:

```text
abc
```

then:

- extraction fails
- `failbit` is set
- `value` is not assigned the intended numeric input
- the invalid characters may remain in the input buffer
- future extractions will keep failing until the stream is cleared

Recovery pattern:

```cpp
#include <iostream>
#include <limits>

std::cin.clear();
std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
```

`clear()` resets flags.

`ignore()` discards the bad line.

### Numeric validation loop

```cpp
#include <iostream>
#include <limits>

int readInt()
{
    while (true)
    {
        std::cout << "Enter an integer: ";

        int value {};
        if (std::cin >> value)
        {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return value;
        }

        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        std::cout << "Invalid input\n";
    }
}
```

This pattern:

```text
try extraction
if success:
    discard rest of line
    return value
if failure:
    clear stream state
    discard bad input
    retry
```

### Range validation

```cpp
int readIntBetween(int min, int max)
{
    while (true)
    {
        int value { readInt() };

        if (value >= min && value <= max)
            return value;

        std::cout << "Enter a value from " << min << " to " << max << '\n';
    }
}
```

Type validation and range validation are separate:

```text
type validation:
  did the input parse as an int?

range validation:
  is the int acceptable for this program?
```

### String validation

For strings, parsing usually succeeds as long as input exists, so validate the
contents.

```cpp
bool isValidSeed(const std::string& seed)
{
    return !seed.empty() && seed.size() <= 4096;
}
```

Read lines and validate:

```cpp
std::string seed {};

while (std::getline(std::cin, seed))
{
    if (!isValidSeed(seed))
    {
        std::cerr << "invalid seed\n";
        continue;
    }

    runTarget(seed);
}
```

### Numeric validation as a string

Sometimes you want stricter validation than `>> int` gives.

For example, formatted extraction may parse a prefix:

```text
123abc
```

as `123`, leaving `abc` behind depending on how you read.

A stricter pattern:

```cpp
#include <charconv>
#include <string>

bool parseIntStrict(const std::string& text, int& value)
{
    const char* begin { text.data() };
    const char* end { text.data() + text.size() };

    auto result { std::from_chars(begin, end, value) };

    return result.ec == std::errc{} && result.ptr == end;
}
```

This says:

- parse must succeed
- parsing must consume the entire string

For Lab 1, you may not need `std::from_chars`, but the idea is useful: sometimes
line-oriented input plus explicit validation is cleaner than interactive
`std::cin >> value` loops.

## 28.6 — Basic file I/O

File streams live in:

```cpp
#include <fstream>
```

Types:

```cpp
std::ifstream // input file stream
std::ofstream // output file stream
std::fstream  // input/output file stream
```

### Reading a file line by line

```cpp
#include <fstream>
#include <iostream>
#include <string>

int main()
{
    std::ifstream file { "seeds.txt" };

    if (!file)
    {
        std::cerr << "could not open seeds.txt\n";
        return 1;
    }

    std::string line {};
    while (std::getline(file, line))
    {
        std::cout << "seed: " << line << '\n';
    }

    return 0;
}
```

Important:

```cpp
if (!file)
```

checks whether opening succeeded.

### Writing a file

```cpp
#include <fstream>
#include <iostream>

int main()
{
    std::ofstream file { "results.txt" };

    if (!file)
    {
        std::cerr << "could not open results.txt\n";
        return 1;
    }

    file << "abc,false,12\n";
    file << "xyz,true,4\n";

    return 0;
}
```

By default, opening an `ofstream` truncates the file.

That means existing contents are replaced.

### Append mode

To append:

```cpp
std::ofstream file { "results.txt", std::ios::app };
```

Now writes go to the end of the file.

Common open modes:

| Mode | Meaning |
| --- | --- |
| `std::ios::in` | open for reading |
| `std::ios::out` | open for writing |
| `std::ios::app` | append writes at end |
| `std::ios::ate` | open and seek to end initially |
| `std::ios::trunc` | truncate existing file |
| `std::ios::binary` | binary mode |

Modes can be combined:

```cpp
std::ofstream file { "out.bin", std::ios::binary | std::ios::trunc };
```

### File paths

Relative paths are relative to the program's working directory, not necessarily
the source file's directory.

```text
source file:
  project/src/main.cpp

program run from:
  project/build/

relative open:
  "seeds.txt"

actual path:
  project/build/seeds.txt
```

If a file "does not exist" even though you see it in the project, check the
working directory.

For CS6340, this matters when code runs inside a build directory, Docker
container, or autograder harness.

### Reading all lines into a vector

```cpp
#include <fstream>
#include <string>
#include <vector>

std::vector<std::string> readLines(const std::string& path)
{
    std::ifstream file { path };

    std::vector<std::string> lines {};
    std::string line {};

    while (std::getline(file, line))
        lines.push_back(line);

    return lines;
}
```

Stronger version with open check:

```cpp
std::vector<std::string> readLines(const std::string& path)
{
    std::ifstream file { path };

    if (!file)
        return {};

    std::vector<std::string> lines {};
    std::string line {};

    while (std::getline(file, line))
        lines.push_back(line);

    return lines;
}
```

Depending on the program, returning `{}` on failure may be too quiet. You might
instead print an error, throw an exception, or return a status object.

### Write structured results

```cpp
struct Result
{
    std::string seed;
    bool crashed {};
    int coverage {};
};

void writeResults(const std::string& path, const std::vector<Result>& results)
{
    std::ofstream file { path };

    if (!file)
    {
        std::cerr << "could not open " << path << '\n';
        return;
    }

    for (const Result& result : results)
    {
        file << result.seed << ','
             << std::boolalpha << result.crashed << ','
             << result.coverage << '\n';
    }
}
```

This combines:

- output streams
- file streams
- range-based loops
- formatting
- structs

Very Lab 1-adjacent.

## 28.7 — Random file I/O

Random file I/O means moving to a specific position in a file and reading or
writing there, instead of only reading/writing sequentially from beginning to
end.

Sequential:

```text
read byte 0
read byte 1
read byte 2
...
```

Random access:

```text
jump to byte 1000
read 20 bytes
jump to byte 12
write 4 bytes
```

### Seek and tell

Input streams:

```cpp
file.seekg(position); // set get/read position
auto pos { file.tellg() }; // get current read position
```

Output streams:

```cpp
file.seekp(position); // set put/write position
auto pos { file.tellp() }; // get current write position
```

The names:

```text
g = get  = reading
p = put  = writing
```

### Read file size

```cpp
#include <fstream>
#include <iostream>

int main()
{
    std::ifstream file { "input.txt", std::ios::binary };

    if (!file)
    {
        std::cerr << "open failed\n";
        return 1;
    }

    file.seekg(0, std::ios::end);
    std::streampos size { file.tellg() };
    file.seekg(0, std::ios::beg);

    std::cout << "size: " << size << " bytes\n";

    return 0;
}
```

Position origins:

| Origin | Meaning |
| --- | --- |
| `std::ios::beg` | beginning |
| `std::ios::cur` | current position |
| `std::ios::end` | end |

### Binary read

Formatted extraction works with text. For raw bytes, use `read`.

```cpp
#include <fstream>
#include <vector>

std::vector<char> readBytes(const std::string& path)
{
    std::ifstream file { path, std::ios::binary };

    if (!file)
        return {};

    file.seekg(0, std::ios::end);
    std::streamsize size { file.tellg() };
    file.seekg(0, std::ios::beg);

    std::vector<char> bytes(static_cast<std::size_t>(size));

    if (size > 0)
        file.read(bytes.data(), size);

    return bytes;
}
```

Notes:

- binary mode avoids platform text translation
- `read` works with raw character buffers
- `bytes.data()` points to the vector's contiguous storage
- cast carefully when moving between stream sizes and container sizes

### Binary write

```cpp
#include <fstream>
#include <vector>

void writeBytes(const std::string& path, const std::vector<char>& bytes)
{
    std::ofstream file { path, std::ios::binary };

    if (!file)
        return;

    file.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
}
```

### Random mutation example

For fuzzing, you may read bytes, mutate one position, then write a new file:

```cpp
std::vector<char> bytes { readBytes("seed.bin") };

if (!bytes.empty())
{
    std::size_t index { bytes.size() / 2 };
    bytes[index] ^= 0x01;
}

writeBytes("mutant.bin", bytes);
```

ASCII:

```text
seed.bin bytes:

index: 0    1    2    3
byte:  0x41 0x42 0x43 0x44

flip one bit at index 2:

index: 0    1    2    3
byte:  0x41 0x42 0x42 0x44
```

### Warnings for random file I/O

Random file I/O is more fragile than line-based text I/O.

Watch for:

- seeking past valid positions
- assuming text file bytes map cleanly to characters on all platforms
- mixing formatted extraction with raw reads without care
- ignoring failed reads/writes
- converting negative or failed `tellg()` results to unsigned sizes

Safer habit:

```cpp
auto pos { file.tellg() };

if (pos == std::streampos { -1 })
{
    // tellg failed
}
```

For Lab 1, you may mostly need ordinary text file I/O. But binary/random access
is useful background for understanding byte-level mutation.

## Chapter 28 summary

- Streams model input and output as flows of characters.
- `std::cin`, `std::cout`, `std::cerr`, and `std::clog` are standard streams.
- `operator<<` inserts into output streams.
- `operator>>` extracts from input streams.
- Formatted extraction skips leading whitespace and stops according to the type.
- `std::getline` reads a whole line.
- Mixing `>>` and `getline` can leave a newline behind; use `std::ws` or
  `ignore`.
- `get`, `peek`, and `ignore` operate closer to the character-buffer level.
- Prefer `'\n'` over `std::endl` unless you need a flush.
- `<iomanip>` provides formatting manipulators like `setw`, `setprecision`,
  `fixed`, and `boolalpha`.
- String streams let strings behave like streams.
- `istringstream` parses from a string.
- `ostringstream` builds a string.
- Streams track states such as `eofbit`, `failbit`, and `badbit`.
- After failed input, use `clear()` and `ignore()` before retrying.
- Validate both parse success and program-specific constraints.
- File streams are in `<fstream>`.
- `ifstream` reads files; `ofstream` writes files; `fstream` can do both.
- Always check whether a file opened successfully.
- Relative paths are relative to the program's working directory.
- Random file I/O uses seek/tell operations.
- Use binary mode and raw `read`/`write` for byte-oriented file work.

## CS6340 patterns

### Load seed lines

```cpp
std::vector<std::string> loadSeeds(const std::string& path)
{
    std::ifstream file { path };

    if (!file)
    {
        std::cerr << "could not open seed file: " << path << '\n';
        return {};
    }

    std::vector<std::string> seeds {};
    std::string line {};

    while (std::getline(file, line))
    {
        if (!line.empty())
            seeds.push_back(line);
    }

    return seeds;
}
```

### Write results to any stream

```cpp
void writeCsvHeader(std::ostream& out)
{
    out << "seed,crashed,coverage\n";
}

void writeCsvRow(std::ostream& out, const Result& result)
{
    out << result.seed << ','
        << std::boolalpha << result.crashed << ','
        << result.coverage << '\n';
}
```

Then:

```cpp
std::ofstream file { "results.csv" };

writeCsvHeader(file);

for (const Result& result : results)
    writeCsvRow(file, result);
```

### Parse a config line

```cpp
struct Config
{
    int trials {};
    int timeoutMs {};
};

bool parseConfigLine(const std::string& line, Config& config)
{
    std::istringstream in { line };

    return static_cast<bool>(in >> config.trials >> config.timeoutMs);
}
```

For stricter parsing, also check that no extra junk remains after the expected
fields.

### Debug with `std::cerr`

```cpp
std::cerr << "running seed: " << seed << '\n';
```

Use `std::cerr` for diagnostics and errors so normal program output can stay
separate from debug/status output.

## Mini drill

Write a function that reads a file containing one seed per line and returns only
seeds whose length is at most `maxLength`.

One possible solution:

```cpp
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

std::vector<std::string> loadBoundedSeeds(const std::string& path,
                                          std::size_t maxLength)
{
    std::ifstream file { path };

    if (!file)
    {
        std::cerr << "could not open " << path << '\n';
        return {};
    }

    std::vector<std::string> seeds {};
    std::string line {};

    while (std::getline(file, line))
    {
        if (line.size() <= maxLength)
            seeds.push_back(line);
    }

    return seeds;
}
```

What this uses:

- `std::ifstream` for file input
- `if (!file)` for open failure
- `std::getline` for full-line seed reading
- `std::vector<std::string>` for dynamic storage
- `std::size_t` for string length comparison

