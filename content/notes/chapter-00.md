# Chapter 0 — Introduction / Getting Started

> Source: <https://www.learncpp.com/> (Chapter 0)
> One file per chapter. Each lesson is a section below.

## Contents
- [0.1 — Introduction to These Tutorials](#01--introduction-to-these-tutorials)
- [0.2 — Introduction to Programs and Programming Languages](#02--introduction-to-programs-and-programming-languages)
- [0.3 — Introduction to C/C++](#03--introduction-to-cc)
- [0.4 — Introduction to C++ Development](#04--introduction-to-c-development)
- [0.5 — Introduction to the Compiler, Linker, and Libraries](#05--introduction-to-the-compiler-linker-and-libraries)
- [0.6 — Installing an IDE](#06--installing-an-integrated-development-environment-ide)
- [0.7 — Compiling Your First Program](#07--compiling-your-first-program)
- [0.8 — A Few Common C++ Problems](#08--a-few-common-c-problems)
- [0.9 — Configuring Your Compiler: Build Configurations](#09--configuring-your-compiler-build-configurations)
- [0.10 — Configuring Your Compiler: Compiler Extensions](#010--configuring-your-compiler-compiler-extensions)
- [0.11 — Configuring Your Compiler: Warning and Error Levels](#011--configuring-your-compiler-warning-and-error-levels)
- [0.12 — Configuring Your Compiler: Choosing a Language Standard](#012--configuring-your-compiler-choosing-a-language-standard)
- [0.13 — What Language Standard Is My Compiler Using?](#013--what-language-standard-is-my-compiler-using)

---

## 0.1 — Introduction to These Tutorials

LearnCpp is structured as a progressive C++ course. The early chapters assume no
C++ background and gradually build from program structure to data types,
functions, files, control flow, compound types, classes, templates, dynamic
allocation, inheritance, exceptions, and I/O.

### How to use the tutorial series

The intended use is sequential:

```text
read lesson
  |
  v
understand the examples
  |
  v
try small code locally
  |
  v
answer/reason through checks
  |
  v
move to next lesson
```

C++ concepts stack heavily. Skipping early vocabulary makes later syntax feel
like noise:

```text
object -> variable -> type -> expression -> function -> scope -> reference
```

If one layer is fuzzy, the next layer becomes harder to parse.

### C++ is large, but the early goal is fluency

C++ is not learned all at once. The useful early goal is not "memorize the
entire language." The goal is to build enough fluency that code stops looking
like punctuation soup.

For CS6340, that means recognizing patterns like:

```cpp
void runOnFunction(Function& F);
std::vector<std::string> seeds {};
auto* call { dyn_cast<CallInst>(&instruction) };
```

These lines combine many fundamentals:

- function declarations
- references
- templates
- standard library types
- initialization
- pointers
- type deduction

### Type examples yourself when possible

Reading code and writing code are different activities.

```text
reading:
  "I understand that line when I see it."

typing:
  "I can recreate the syntax and fix the compiler errors."
```

Typing short examples helps because C++ is precise. A missing `;`, `&`, `*`,
`const`, or header include can change the program completely.

### Treat compiler errors as feedback

Compiler messages are part of the learning loop. They can be noisy, but they are
evidence.

Useful approach:

```text
first error first
  |
  v
read file and line
  |
  v
inspect nearby code
  |
  v
fix one issue
  |
  v
recompile
```

One syntax mistake can trigger many downstream errors. Fixing the first real
error often removes the rest.

### Version note

LearnCpp is a living site. C++ also evolves through language standards such as
C++11, C++14, C++17, C++20, and C++23. These notes should preserve the concepts,
but when exact compiler behavior matters, check:

- the current LearnCpp lesson
- the compiler version
- the selected language standard
- the course environment

---

## 0.2 — Introduction to Programs and Programming Languages

A program is a sequence of instructions that tells a computer what to do.

At the hardware level, computers execute machine instructions. Those
instructions are represented in binary, but humans usually write programs in
higher-level programming languages.

### Hardware only understands machine language directly

Machine language is CPU-specific.

```text
source idea:
  add two numbers

human-friendly language:
  int sum { a + b };

machine-level reality:
  CPU-specific binary instructions
```

Different CPU families have different instruction sets. A program compiled for
one kind of CPU may not run directly on another.

### Programming languages bridge humans and hardware

Programming languages let humans express instructions at a higher level.

```text
human intent
  |
  v
C++ source code
  |
  v
compiler + linker
  |
  v
machine executable
  |
  v
CPU runs instructions
```

The higher-level language gives you names, functions, types, control flow, and
abstractions. The toolchain translates those into something the machine can run.

### Low-level vs high-level languages

Low-level languages are closer to hardware.

High-level languages are closer to human problem descriptions.

```text
lower level                         higher level
machine code -> assembly -> C -> C++ -> Python-style scripting
```

C++ sits in an interesting middle zone:

- high-level abstractions: classes, templates, standard library
- low-level control: memory layout, pointers, object lifetime, performance

That mix is why C++ is used for systems, compilers, game engines, finance,
embedded software, and performance-sensitive tools.

### Compiled vs interpreted language model

C++ is normally compiled.

```text
C++ source files
  |
  v
compiler
  |
  v
object files
  |
  v
linker
  |
  v
executable
```

An interpreted language is often executed by another program at runtime:

```text
script source
  |
  v
interpreter/runtime
  |
  v
program behavior
```

Modern language implementations can mix these ideas, but for this course the
main C++ mental model is:

```text
edit -> compile -> link -> run
```

### Portability

Source code is more portable than machine code.

```text
C++ source
   |
   +-- compile on macOS  -> macOS executable
   |
   +-- compile on Linux  -> Linux executable
   |
   +-- compile on Windows -> Windows executable
```

The same source may still need changes if it depends on platform-specific APIs,
compiler extensions, file paths, integer sizes, or undefined behavior.

### Why this matters for CS6340

CS6340 sits directly on this stack.

```text
C++ source
  |
  v
compiler frontend
  |
  v
intermediate representation (LLVM IR)
  |
  v
analysis / instrumentation / optimization
  |
  v
object code or executable behavior
```

When Lab 1 asks you to instrument or fuzz programs, you are working with the
consequences of this translation pipeline.

---

## 0.3 — Introduction to C/C++

### Before C++, there was C

| Fact | Detail |
|---|---|
| Author | **Dennis Ritchie** |
| Where / when | Bell Telephone Laboratories, **1972** |
| Purpose | A **systems programming language** |

Ritchie's design objectives for C:

1. **Minimalistic** — small language, few built-in features.
2. **Easy to compile** — simple enough to write compilers for.
3. **Efficient memory access** — direct, low-level access to memory.
4. **Efficient code generation** — produces fast machine code.
5. **Self-contained** — not dependent on other software to run.

**C + Unix: the portability breakthrough**

- **1973**: Ritchie and **Ken Thompson** rewrote most of **Unix** in C.
- Unlike **assembly** (tied to one CPU type), C was **portable** — the same
  source could be **recompiled** for a different CPU.
- This accelerated adoption of both Unix and C.

```
Assembly: one program  ──tied to──>  one specific CPU  (rewrite by hand per machine)
C:        one program  ──recompile──> CPU A / CPU B / CPU C  (same source code)
```

**Standardization timeline of C**

| Year | Name | Who / what |
|---|---|---|
| 1978 | **K&R** | Kernighan & Ritchie publish *The C Programming Language*; de facto standard. |
| 1989 | **C89 / ANSI C** | ANSI sets first formal standard. |
| 1990 | **C90** | ISO adopts the ANSI standard. |
| 1999 | **C99** | ISO update; pulled in compiler extensions and C++ features. |

### C++

| Fact | Detail |
|---|---|
| Author | **Bjarne Stroustrup** |
| Where / when | Bell Labs, beginning **1979** |
| Origin | Started as an **extension to C** |
| Big addition | **Object-oriented programming** |
| Standardized | by the **ISO committee in 1998** |

C++ versions, released **every ~3 years**, named by the last two digits of the year:

```
C++98 ─ C++03 ─ C++11 ─ C++14 ─ C++17 ─ C++20 ─ C++23 ─ ...
                 ↑ big modernization release
```

> Formal names are unwieldy — C++20 is officially **ISO/IEC 14882:2020** — so the
> informal "C++YY" naming is used. Future upgrades expected **every ~3 years**.

### Philosophy: *"trust the programmer"*

- High **freedom and control** (especially over memory).
- The language **will not stop you** from doing something nonsensical.
- Responsibility is on **you** — this is why C++ is fast *and* full of pitfalls.
  (This is the core reason program-analysis/testing fields like CS 6340 exist.)

### Q&A

- **Good at?** *"high performance and precise control over memory and other
  resources."* → games, real-time, finance, graphics/simulation, productivity
  apps, embedded, audio/video, AI/neural nets. Large third-party ecosystem.
- **Dying?** *"Nope."* 2nd–3rd most popular *compiled* language, 5th–6th overall.
- **Need C first?** *"Nope!"* Learning C++ first makes C easy later.

---

## 0.4 — Introduction to C++ Development

The development process is a **7-step flow** from idea → executable. (Steps 4–7
are detailed in 0.5; this lesson focuses on 1–3.)

```
1. Define the problem        ─┐
2. Determine how to solve it  │  (planning / "what" + "how")
3. Write the program          │
4. Compile                    │  ── 0.5
5. Link                       │  ── 0.5
6. Test                       │  ── 0.5
7. Debug ──(loop back)────────┘  ── 0.5
```

### Step 1 — Define the problem (the "what")

Identify a **well-defined problem** to solve. Examples:
- A number-averaging program.
- A navigable 2D maze game.
- A stock-price prediction tool.

### Step 2 — Determine how to solve it (the "how")

The **most-neglected step.** Don't jump straight to code. Good solutions are:

> *"straightforward (not overly complicated or confusing) … well documented
> (especially around any assumptions … or limitations) … built modularly, so
> parts can be reused or changed later without impacting other parts … [and]
> can recover gracefully or give useful error messages when something
> unexpected happens."*

- **Bug** = *"any kind of programming error that prevents the program from
  operating correctly."*
- **Historical note:** "bug" dates to Edison (1870s); popularized in the 1940s
  when an actual **moth** was found in early computer hardware (now in the
  Smithsonian).
- **Maintenance reality:** programmers spend only **10–40%** of time writing
  initial code; **60–90%** on **maintenance** — debugging, environment updates,
  enhancements, reliability.

### Step 3 — Write the program

Needs: knowledge of a programming language + a text editor to save **source
code**. Use a real **code editor** (not Notepad). Good editors give:

1. **Line numbering** — to locate the line an error refers to.
2. **Syntax highlighting / coloring** — visual differentiation of code parts.
3. **Monospace (fixed-width) font** — so characters like `l`/`1`/`I` and `O`/`0`
   are unambiguous.

> **Best practice:** Name the first/primary source file `main.cpp`. Makes the
> entry point obvious. Big programs can have hundreds/thousands of source files.

---

## 0.5 — Introduction to the Compiler, Linker, and Libraries

### Step 4 — Compile your source code

The **compiler** processes each `.cpp` file and does two things:

1. **Syntax check** — verifies the code obeys C++ rules. On violation it emits
   an **error with a line number** and stops until you fix it.
2. **Translation** — converts C++ into machine-language instructions, written to
   an **object file** (`name.o` or `name.obj`). Object files also hold data
   needed for linking and debugging.

```
 main.cpp ─┐
 add.cpp  ─┤── [ COMPILER ] ──> main.o
 io.cpp   ─┘                    add.o      (one object file per source file)
                                io.o
```

### Step 5 — Link object files and libraries

The **linker** combines object files into the final output (usually an
executable). **Linking** has 4 jobs:

1. **Read & validate** each object file.
2. **Resolve cross-file references** — match each *use* of something to its
   *definition* in another file. A missing match = **linker error**.
3. **Link in libraries** — pull in precompiled reusable code (**library files**).
4. **Produce output** — the executable (or a library).

```
 main.o ─┐
 add.o  ─┤── [ LINKER ] ──> executable
 io.o   ─┘        ↑
   libraries (std lib + 3rd-party) ──┘
```

- **C++ Standard Library** — large bundle of functionality shipped with C++. Its
  I/O part is **iostream** (console input/output). Most linkers include the
  standard library **automatically**.
- **Third-party libraries** — written by others, add functionality beyond the
  standard library (e.g. sound). Optional; you link them in yourself.

> **Building** = the whole source→executable process. The result is a **build**.

### Steps 6 & 7 — Test and debug

- **Testing** — run the program with varied inputs to check it behaves as
  expected.
- **Debugging** — find and fix programming errors when behavior is wrong (loops
  back into the process).

### IDEs

Steps 3, 4, 5, and 7 each need a separate tool (editor, compiler, linker,
debugger). An **IDE (integrated development environment)** bundles them all into
one package.

---

## 0.6 — Installing an Integrated Development Environment (IDE)

An **IDE** = *"software designed to make it easy to develop, build, and debug
your programs."* A modern IDE bundles:

- **Code editor** — syntax highlighting, line numbering, name completion.
- **Build system** — compile + link.
- **Integrated debugger** — find defects.
- **Plugin support** — extend/customize.

### Recommended IDEs by platform

| Platform | Recommendation | Install notes |
|---|---|---|
| **Windows** | **Visual Studio 2022 Community** | In installer pick **"Desktop development with C++"**; ensure a Windows 10/11 SDK is selected. |
| **Linux / cross-platform** | **Code::Blocks** (free, open source) | Windows users: get the **MinGW-bundled** version. Linux users may need `build-essential`. |
| **macOS** | **Xcode** (if available), or Eclipse with C++ components | — |
| **Experienced users (any OS)** | **VS Code** | *Not recommended for beginners* — needs significant config; many beginner challenges reported. |

### Compiler / C++ standard requirement

- Install a compiler supporting **at least C++17**; prefer the **newest** version.
- Minimum versions for C++17:
  - **GCC/G++ 7**
  - **Clang++ 8**
  - **Visual Studio 2017 15.7**
- **Avoid** compilers that only support C++11 or earlier.

> **Tip:** Pick an IDE bundled with a C++17-compatible compiler.
> **Warning:** VS Code needs config experience and isn't fully supported for
> beginners in this tutorial.

(CS 6340 note: the course Docker image targets **LLVM/Clang 14**, well past the
C++17 minimum — so everything here is satisfied.)

---

## 0.7 — Compiling Your First Program

### Projects, workspaces, console apps

- **Project** = a container holding everything to build one program: `.cpp`
  source files, images, data, **plus IDE/compiler settings**. It preserves state
  between sessions.
  > **Best practice:** *"Create a new project for each new program you write."*
- **Workspace / Solution** = a container holding one or more related **projects**.
  While learning, use **a separate workspace per program** — simpler, fewer errors.
- **Console application** = runs in a terminal/console, no GUI; prints text and
  reads keyboard input; compiles to a standalone executable. Ideal for learning
  (minimal complexity). Contrast: **GUI applications** need extra frameworks.

### The Hello World program

```cpp
#include <iostream>

int main()
{
    std::cout << "Hello, world!";
    return 0;
}
```

- `#include <iostream>` — pulls in the standard I/O library.
- `std::cout << ...` — sends text to the console.
- `return 0;` — signals successful execution to the OS.

### Build-menu operations

| Command | What it does |
|---|---|
| **Build** | Compile all **modified** files, then link into the executable (skips unchanged files). |
| **Clean** | Delete cached object files and the executable (forces a fresh build). |
| **Rebuild** | **Clean**, then **Build**. |
| **Compile** | Recompile **one** file only — no linking, no executable produced. |
| **Run / Start** | Execute the program (many IDEs auto-build first if needed). |

### Best practices & setup

- New project **per program**; use **Console Application (.exe)** type.
- Configure the compiler to use a recent **C++ standard** (covered in 0.12).
- Enable **compiler warnings**.
- For small projects, **disable precompiled headers** initially.
- Consider **excluding project directories from antivirus** to avoid false positives.

### Common issues (preview of 0.8)

- **C1010 precompiled-header error** → recreate project with PCH disabled (or add
  the `#include "pch.h"` line).
- **Console window flashes closed** → add a pause (`std::cin.get()`) before
  `return`; or use **Start Without Debugging (Ctrl+F5)** in Visual Studio.
- **Won't run / no output** → antivirus may be blocking; disable or exclude dir.

> Author's note in spirit: installation + compilation are the **hardest** part;
> understanding the code comes next.

---

## 0.8 — A Few Common C++ Problems

### A. General run-time issues

**Console window closes immediately (output flashes by).**
Add these includes at the top:
```cpp
#include <iostream>
#include <limits>
```
Then put this **before** `return` in `main()` to pause for a keypress:
```cpp
std::cin.clear();                                                   // reset any error flags
std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // discard leftover input
std::cin.get();                                                     // wait for Enter
```
- **Avoid** `system("pause")` — it's OS-dependent (Windows-only).
- In Visual Studio, prefer **Start Without Debugging (Ctrl+F5)** over F5.

**Program runs but produces no output** → a virus scanner / anti-malware may be
blocking it. Temporarily disable, or exclude the project directory.

**Compiles but behaves incorrectly (logic errors)** → use the debugging
techniques in **chapter 3**.

### B. General compile-time issues

| Symptom | Cause / fix |
|---|---|
| `unresolved external symbol _main` (or `_WinMain@16`) | `main()` missing/misspelled, or its file isn't compiled into the project. Verify `main()` exists, check spelling, ensure the file is part of the project; recreate as a console project if needed. |
| `main is already defined` / multiple `main()` | Only **one** `main()` allowed per program. Find and remove the extras. |
| C++11/14/17 features won't compile | Compiler too old, or wrong language-standard setting. Upgrade compiler / set the standard (lesson 0.12). |
| `cannot open .exe for writing` (LNK1168) | The exe is still running, locked by another process, or blocked by antivirus. Close the running instance, disable AV, or reboot. |
| `cin` / `cout` / `endl` "undeclared identifier" | Add `#include <iostream>` and prefix with `std::` (e.g. `std::cout`). If still broken, reinstall/upgrade compiler. |
| `end1` undeclared | You typed number **1** instead of lowercase **L** in `endl`. Use a monospace font so `l`/`1`/`I` and `O`/`0` are distinguishable. |

### C. Visual Studio–specific issues

| Symptom | Cause / fix |
|---|---|
| **C1010** "unexpected end of file while looking for precompiled header directive" | Disable precompiled headers (per 0.7), **or** add `#include "pch.h"` (older: `"stdafx.h"`) as the **first line** of every `.cpp`. |
| **LNK2022 / `_WinMain@16`** unresolved | Project created as a GUI app — recreate it as a **Windows/Win32 Console application**. |
| "Cannot find or open the PDB file" warning | Non-fatal. To silence: **Debug → Options → Symbols** → enable **Microsoft Symbol Server**. |

### D. When still stuck

Search the **exact error message in quotes**, ask an AI assistant, or post on a
Q&A board (e.g. Stack Overflow) — include your **OS, IDE, and full details**.

---

## 0.9 — Configuring Your Compiler: Build Configurations

A **build configuration** (a.k.a. *build target*) is a bundle of IDE/compiler
settings controlling how a project compiles — executable name, library search
paths, whether debug info is included, and optimization level. New projects ship
with two:

| | **Debug** | **Release** |
|---|---|---|
| Purpose | Active development & debugging | Shipping to users / perf testing |
| Optimizations | **Off** | **On** (optimized for size & speed) |
| Debug symbols | **Included** | **Stripped** |
| Result | *"larger and slower, but much easier to debug"* | smaller & fast |
| Example size | Hello World ≈ **65 KB** | same program ≈ **12 KB** (~5×) |

> **Best practice:** *"Use the debug build configuration when developing… When
> you're ready to release your executable to others, or want to test
> performance, use the release build configuration."*

**Switching configs:**

- **Visual Studio** — *Solution Configurations* dropdown in the Standard
  toolbar (Debug ↔ Release). A separate *Solution Platforms* dropdown picks
  **x86 (32-bit)** vs **x64 (64-bit)**.
- **Code::Blocks** — *Build Target* dropdown in the Compiler toolbar.
- **GCC/Clang command line** — debug: `-ggdb`; release: `-O2 -DNDEBUG`.

> **Tip:** When you change a project setting, change it for **all build
> configurations** so it doesn't apply to only one mode.

(CS 6340 tie-in: `-O0 -g` ≈ debug, `-O2`/`-O3` ≈ release. Optimization level
*changes the generated IR*, which is exactly why analysis passes care which one
you built with.)

---

## 0.10 — Configuring Your Compiler: Compiler Extensions

**Compiler extensions** = non-standard features a compiler adds beyond what the
C++ standard specifies — *"often to enhance compatibility with other versions of
the language (e.g. C99), or for historical reasons."*

**Why they're a problem:**
- **Not portable** — code using them *"generally will not compile on other
  compilers… or if they do, they may not run correctly."*
- **Misleading** — beginners can mistake non-standard behavior for real C++.
- **Often enabled by default**, which makes the trap worse.

> **Best practice:** *"Disable compiler extensions to ensure your programs (and
> coding practices) remain compliant with C++ standards and will work on any
> system."*

**How to disable:**

- **Visual Studio** — Project Properties → C/C++ → Language → **Conformance mode
  = Yes (`/permissive-`)**.
- **GCC / Clang / Code::Blocks** — add **`-pedantic-errors`** (Code::Blocks:
  Settings → Compiler → Compiler flags).

> **Note:** This is a **per-project** setting — reconfigure for each new project.

---

## 0.11 — Configuring Your Compiler: Warning and Error Levels

Two kinds of compiler diagnostics:

- **Error** — compiler **stops**; the code is wrong or serious enough to halt.
- **Warning** — compiler **continues** but flags code that's *legal* but looks
  suspicious.

> **Key insight:** different compilers may classify the same issue differently —
> one warns, another errors.

> **Best practice:** *"Don't let warnings pile up. Resolve them as you encounter
> them (as if they were errors)."* and *"Turn your warning levels up, especially
> while you are learning."*

**Turn warnings up:**

- **GCC / Clang / Code::Blocks:**
  `-Wall -Weffc++ -Wextra -Wconversion -Wsign-conversion`
  - `-Wall` basic, `-Wextra` more, `-Weffc++` *Effective C++* style,
    `-Wconversion` implicit conversions, `-Wsign-conversion` signed/unsigned.
- **Visual Studio:** set **Warning Level = /W4** (C/C++ → General). Add
  `/w44365` (signed/unsigned) in Command Line; set External Header Warning Level
  `/external:W3`.

**Treat warnings as errors** (forces you to fix every one):

- **GCC / Clang / Code::Blocks:** `-Werror`
- **Visual Studio:** Treat Warnings As Errors = **Yes (`/WX`)**

(CS 6340 tie-in: `-Werror` discipline is the cheapest static analysis there is —
the compiler is already a bug finder; turning warnings into errors makes you use
it.)

---

## 0.12 — Configuring Your Compiler: Choosing a Language Standard

Compilers usually **default to an older standard** (often C++14), so you set it
explicitly.

**Development codenames vs final names** (the codename has a letter; the final
name has the year):

| Codename | Final | Published |
|---|---|---|
| C++0x | **C++11** | 2011 |
| C++1y | **C++14** | 2014 |
| C++1z | **C++17** | 2017 |
| C++2a | **C++20** | 2020 |
| C++2b | **C++23** | 2023 |
| C++2c | **C++26** | in progress |

> Why defaults lag: new standards need time *"for compiler makers to resolve
> defects, and so that best practices for new features are well understood."*
> Cross-platform compatibility also favors conservatism.

> **Author's note / recommendation:** *"To take full advantage of all lesson
> content, we recommend using the latest language standard your compiler
> supports."*

**Setting the standard:**

- **Visual Studio** — Project Properties → C/C++ → Language → **C++ Language
  Standard** → e.g. `/std:c++17` or **ISO C++ Latest (`/std:c++latest`)**.
  *Caveat: must reselect for every new project.*
- **Code::Blocks** — Settings → Compiler → check *"Have g++ follow the C++XX ISO
  standard `[-std=c++XX]`"*, or add `-std=c++17` in Other compiler options.
- **GCC/G++/Clang** — `-std=c++11 / c++14 / c++17 / c++20 / c++23`; experimental:
  `-std=c++2c`.
- **VS Code** — flag in `tasks.json` `"args"`; IntelliSense in `settings.json`:
  `"C_Cpp.default.cppStandard": "c++20"`.

> Make the change for **all build configurations**. Both VS and Code::Blocks let
> you **export project templates** so you don't reconfigure every time.
> Compilers often have **incomplete support** for new features even after
> standardization — CPPReference tracks per-compiler support.

---

## 0.13 — What Language Standard Is My Compiler Using?

A diagnostic program: print which C++ standard the compiler is actually using by
reading the **`__cplusplus`** preprocessor macro and mapping its value to a name.

**Value → standard map:**

| `__cplusplus` value | Standard |
|---|---|
| `199711L` | Pre-C++11 |
| `201103L` | C++11 |
| `201402L` | C++14 |
| `201703L` | **C++17** |
| `202002L` | C++20 |
| `202302L` | C++23 |
| `202612L` | C++26 (placeholder) |

```cpp
#include <iostream>

const int numStandards = 7;
const long stdCode[numStandards] = { 199711L, 201103L, 201402L, 201703L, 202002L, 202302L, 202612L };
const char* stdName[numStandards] = { "Pre-C++11", "C++11", "C++14", "C++17", "C++20", "C++23", "C++26" };

long getCPPStandard()
{
#if defined (_MSVC_LANG)
    return _MSVC_LANG;
#elif defined (_MSC_VER)
    return -1;
#else
    return __cplusplus;
#endif
}

int main()
{
    long standard = getCPPStandard();

    if (standard == -1)
    {
        std::cout << "Error: Unable to determine your language standard.  Sorry.\n";
        return 0;
    }

    for (int i = 0; i < numStandards; ++i)
    {
        if (standard == stdCode[i])
        {
            std::cout << "Your compiler is using " << stdName[i]
                << " (language standard code " << standard << "L)\n";
            break;
        }

        if (standard < stdCode[i])
        {
            std::cout << "Your compiler is using a preview/pre-release of " << stdName[i]
                << " (language standard code " << standard << "L)\n";
            break;
        }
    }

    return 0;
}
```

**Notes:**
- **Visual Studio** is non-conforming for `__cplusplus` — it reports `199711L`
  unless you add **`/Zc:__cplusplus`**; the program above sidesteps this by
  reading **`_MSVC_LANG`** instead (VS 2015+). Older VS returns `-1` (unknown).
- **Exact match** → that standard; value **between** two known codes → a
  *preview/pre-release* of the next one.
- If it returns `-1`, recheck settings (lesson 0.12).
