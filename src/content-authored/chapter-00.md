# Chapter 0 — Introduction / Getting Started

> Source: <https://www.learncpp.com/> (Chapter 0)

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

Welcome. You are about to learn one of the most powerful — and most demanding — programming languages in wide use. Before we touch a line of code, it is worth a few minutes to talk about *how* to learn C++, because the strategy you bring to it matters as much as the hours you put in.

### Why a roadmap helps

C++ is large. If you tried to hold the whole language in your head at once, you would never start. So this course is built the way a good lecture course is: it begins assuming you know nothing about C++, and it builds up one layer at a time — from the basic structure of a program, to data types, to functions, to control flow, and onward to the bigger ideas like classes, templates, dynamic memory, and inheritance.

The crucial thing to understand is that **these layers stack**, and they stack tightly. Each new idea is described using vocabulary from the ideas before it. Read the chain below from left to right:

```text
object -> variable -> type -> expression -> function -> scope -> reference
```

If "type" is fuzzy for you, then "expression" will be harder, and "function" harder still. None of these concepts is difficult on its own, but a wobbly foundation makes the upper floors feel impossible. This is why the course is meant to be read **in order**. Skipping ahead to "the interesting part" usually backfires, because the interesting part is written in a vocabulary the earlier chapters taught you.

> **Tip:** When a later lesson feels like punctuation soup, the fix is almost always to go back one or two concepts, not to push forward harder.

### The early goal is fluency, not mastery

You do not need to memorize the entire language to be productive in it — and you certainly will not do so in the first few chapters. The honest early goal is **fluency**: getting to the point where C++ code stops looking like a wall of random symbols and starts reading like sentences.

Consider a line you might meet later:

```cpp
std::vector<std::string> seeds {};
```

To a newcomer this is intimidating. But it is really just several small ideas standing next to each other: a standard-library container (`std::vector`), the type it holds (`std::string`), a name (`seeds`), and an initializer (`{}`). Once each of those pieces is familiar on its own, the whole line reads at a glance. Fluency is exactly that accumulation of small recognitions.

### Type the examples yourself

Reading code and writing code are different skills, the way reading music and playing an instrument are different skills.

```text
reading:  "I understand that line when I see it."
typing:   "I can reproduce it from scratch and fix the errors when I get it wrong."
```

C++ is unforgiving about detail. A single missing `;`, a stray `&` or `*`, a dropped `const`, or a forgotten `#include` can change what your program means — or stop it from compiling at all. The only reliable way to internalize that precision is to type real examples, break them, and fix them. So whenever you reach a code sample, retype it into your own editor and run it. The friction is the point.

### Treat compiler errors as feedback, not failure

Beginners often read a compiler error as a scolding. It is better to think of it as a lab instrument: noisy, sometimes cryptic, but giving you real evidence about what went wrong and where.

A reliable habit when the compiler complains:

```text
read the FIRST error first
  -> note the file and line number
  -> look at that line and the few lines above it
  -> fix one thing
  -> recompile
```

The reason to fix only the first error and immediately recompile is that **one mistake often cascades**. A single missing semicolon can produce a screen full of errors, most of which are just the compiler losing its place. Fix the first genuine problem and a large fraction of the rest frequently vanish on the next compile.

> **Key insight:** Errors are not a sign you are bad at this. They are the normal medium of programming. Learning to read them calmly is one of the most valuable skills you will build in this course.

### A note on versions

C++ is a living language. It is revised on a regular cadence — you will see names like C++11, C++14, C++17, C++20, and C++23 throughout these notes. Most of what you learn here is stable across all of them, but occasionally the exact behavior of a feature depends on which standard your compiler is set to use. When that matters, the relevant lesson will say so, and the configuration chapters (0.9 through 0.13) will show you how to check and control which standard you are compiling against.

---

## 0.2 — Introduction to Programs and Programming Languages

Before we write C++, let's be clear about what a program actually *is* and why a language like C++ needs to exist at all.

A **program** is simply a sequence of instructions that tells a computer what to do. That definition is deceptively small, so the rest of this lesson unpacks the gap between the instructions *you* write and the instructions the *machine* runs.

### The machine only speaks one language

At the very bottom, a CPU executes **machine instructions** — numbers, encoded in binary, that the hardware is physically wired to interpret. This is the only language a processor truly understands. And critically, it is **CPU-specific**: the instruction set for one family of processors is not the same as another's.

```text
the idea:                 add two numbers
how you'd write it:       int sum { a + b };
what the CPU runs:        CPU-specific binary instructions
```

Because each CPU family has its own instruction set, a program translated into machine code for one kind of processor may not run at all on a different kind. Hold onto that fact — it is the reason portability becomes such a recurring theme.

### Programming languages bridge the gap

Writing raw machine code by hand is brutal, error-prone work. **Programming languages** exist to let humans express instructions at a far higher level — using names, functions, types, control flow, and other abstractions that match how we think about problems rather than how the hardware happens to be built. A piece of software called the **toolchain** then translates what you wrote into something the machine can actually execute.

```text
human intent
   -> C++ source code
   -> compiler + linker
   -> machine executable
   -> CPU runs the instructions
```

You write in the top language; the toolchain does the descent to the bottom one.

### Low-level versus high-level

Languages are often placed on a spectrum from **low-level** (close to the hardware) to **high-level** (close to the way a human describes a problem):

```text
lower level                                              higher level
machine code  ->  assembly  ->  C  ->  C++  ->  scripting languages
```

C++ occupies a genuinely unusual spot in the middle. It gives you **high-level abstractions** — classes, templates, a large standard library — while still handing you **low-level control** over memory layout, pointers, object lifetime, and performance. Most languages make you choose one or the other; C++ lets you reach for either as the situation demands.

That dual nature is why C++ shows up wherever both expressiveness *and* control matter: operating systems, compilers, game engines, finance, scientific simulation, embedded devices, and performance-sensitive tools of every kind.

### Compiled versus interpreted

There are two broad models for getting from source code to running behavior.

C++ is normally **compiled**. The whole program is translated ahead of time into a standalone executable, which the OS then runs directly:

```text
C++ source files
   -> compiler
   -> object files
   -> linker
   -> executable
```

By contrast, an **interpreted** language is typically executed by another program — an interpreter — that reads the source and performs its behavior at runtime:

```text
script source
   -> interpreter / runtime
   -> program behavior
```

Real-world language implementations blur this line in clever ways, but for this course the mental model to carry around is the compiled one. Memorize this loop — you will live inside it for the rest of the book:

```text
edit  ->  compile  ->  link  ->  run
```

### Portability

Here is a payoff of working in a high-level language. Because your **source code** describes intent rather than hardware-specific instructions, the same source can usually be compiled into a working program on different operating systems and CPUs:

```text
                 +-- compiled on macOS   -> macOS executable
C++ source ------+-- compiled on Linux   -> Linux executable
                 +-- compiled on Windows -> Windows executable
```

The executables differ; the source can be one and the same. This is a real advantage over distributing machine code, which is locked to a single kind of machine.

> **Note:** Portability is not automatic. Source that relies on platform-specific APIs, compiler extensions, hard-coded file paths, particular integer sizes, or — worst of all — undefined behavior may still need changes to build and run correctly elsewhere. Writing standard-compliant C++ is what keeps the portability promise intact, and it is a habit this course will encourage from the start.

---

## 0.3 — Introduction to C/C++

You cannot really understand C++ without knowing a little of where it came from, because almost every design choice in the language is an echo of its history. The short version: C++ grew out of C, and C grew out of a need for a portable systems language.

### Before C++, there was C

C was created by **Dennis Ritchie** at Bell Telephone Laboratories around **1972**, as a language for **systems programming** — the kind of low-level work, like writing operating systems, where you need direct and efficient access to the machine.

Ritchie designed C around a handful of objectives that still describe its character today:

1. **Minimalistic** — a small language with few built-in features.
2. **Easy to compile** — simple enough that writing a compiler for it is tractable.
3. **Efficient memory access** — direct, low-level control over memory.
4. **Efficient code generation** — it produces fast machine code.
5. **Self-contained** — it does not depend on other software to run.

The pivotal moment came in **1973**, when Ritchie and **Ken Thompson** rewrote most of the **Unix** operating system in C. Until then, system software was typically written in assembly, which is tied to one specific CPU and must be rewritten by hand for every new machine. C changed the economics entirely:

```text
Assembly:  one program  --tied to-->     one specific CPU   (rewrite by hand per machine)
C:         one program  --recompile-->   CPU A / CPU B / CPU C   (same source code)
```

Because C source could simply be **recompiled** for a different processor, both Unix and C spread rapidly. Portability, as promised in the last lesson, turned out to be a superpower.

C was eventually standardized so that everyone could agree on what the language meant:

| Year | Name | What happened |
|---|---|---|
| 1978 | **K&R** | Kernighan & Ritchie publish *The C Programming Language*; it became the de facto standard. |
| 1989 | **C89 / ANSI C** | ANSI sets the first formal standard. |
| 1990 | **C90** | ISO adopts the ANSI standard. |
| 1999 | **C99** | An ISO update that folded in popular compiler extensions and some C++ features. |

### Then came C++

**Bjarne Stroustrup**, also at Bell Labs, began work on C++ in **1979**. It started life literally as an **extension to C** — its early name was "C with Classes" — and its headline addition was support for **object-oriented programming**, a way of organizing code around the data it operates on. C++ was first standardized by the **ISO committee in 1998**.

Like C, C++ is revised on a regular schedule — roughly **every three years** — and each version is named by the last two digits of its release year:

```text
C++98 - C++03 - C++11 - C++14 - C++17 - C++20 - C++23 - ...
                  ^ a large modernization of the language
```

C++11 is worth singling out: it was a sweeping update that modernized the language so substantially that "modern C++" usually means C++11 and later.

> **Note:** The formal names are a mouthful — C++20 is officially *ISO/IEC 14882:2020* — so everyone uses the informal "C++YY" shorthand instead. New revisions are expected to keep arriving on roughly the same three-year cadence.

### The guiding philosophy: "trust the programmer"

If C++ had a motto, it would be *trust the programmer*. The language gives you enormous freedom and control, especially over memory, and in exchange it generally **will not stop you** from doing something that makes no sense. There are very few guardrails.

This is a double-edged sword, and it is important to internalize early. The same freedom that makes C++ fast and flexible also makes it easy to write programs that are subtly — or catastrophically — wrong. The responsibility for correctness sits with **you**, the programmer, not with the language.

> **Warning:** C++ will happily let you read memory you do not own, use a variable before giving it a value, or index past the end of an array — and it may not warn you at all. Much of this course is about building the habits that keep you on the safe side of that freedom.

### A few common questions

**What is C++ good at?** Anything that demands high performance together with precise control over memory and other resources: games and real-time systems, finance, graphics and simulation, embedded software, audio and video processing, and the performance-critical cores of larger applications. It also has a large ecosystem of third-party libraries.

**Is C++ dying?** No. It remains one of the most widely used languages in the world and consistently ranks among the top few compiled languages. Demand for it has been stable for decades.

**Do I need to learn C first?** No. You can learn C++ directly — this course assumes no C background — and having learned C++ makes picking up C straightforward later, since C is essentially a subset of what you will know.

---

## 0.4 — Introduction to C++ Development

Writing software is not just typing code. It is a process, and understanding the whole process keeps you from the very common beginner trap of diving straight into syntax before you know what you are even trying to build.

The path from an idea to a finished program can be described as seven steps:

```text
1. Define the problem         (the "what")
2. Determine how to solve it   (the "how")
3. Write the program
4. Compile           --+
5. Link                |  covered in lesson 0.5
6. Test                |
7. Debug --(loop back)-+
```

This lesson covers the human-centered first three steps — defining, planning, and writing. The mechanical steps (compiling, linking, testing, debugging) belong to the toolchain and are the subject of lesson 0.5. Note the loop: debugging sends you back into the process, because finding one defect often means revisiting your code, your plan, or even your understanding of the problem.

### Step 1 — Define the problem (the "what")

Before solving anything, you need a **well-defined problem** to solve — a clear statement of what the program should accomplish. The clearer this is, the easier everything downstream becomes. Examples of well-defined problems:

- A program that averages a list of numbers.
- A navigable 2D maze game.
- A tool that predicts stock prices from historical data.

### Step 2 — Determine how to solve it (the "how")

This is the planning step, and it is the **most neglected one**. The temptation is overwhelming to skip straight to writing code, but a few minutes of thought here saves hours later. A good solution has several qualities worth aiming for: it is **straightforward** rather than needlessly complicated; it is **well documented**, especially around its assumptions and limitations; it is **built modularly**, so that pieces can be reused or changed later without breaking everything else; and it **handles the unexpected gracefully**, recovering or giving a useful error message rather than simply collapsing.

This is also a natural place to introduce a word you will use constantly. A **bug** is any kind of programming error that prevents a program from operating correctly.

> **Note:** The term "bug" is older than computing — it appears in Edison's notes in the 1870s — but it gained its modern fame in the 1940s when operators of an early computer traced a fault to an actual moth caught in the hardware. The moth was taped into the logbook and is now in the Smithsonian.

There is a sobering reality behind all this emphasis on planning. Studies of professional software work consistently find that programmers spend only a minority of their time — often cited as 10 to 40 percent — writing a program's *initial* code. The majority, frequently 60 to 90 percent, goes to **maintenance**: fixing bugs, adapting to changed environments, adding features, and keeping things reliable. Code is read and modified far more often than it is first written, which is exactly why clarity and modularity pay off so handsomely.

### Step 3 — Write the program

To write a program you need two things: knowledge of a programming language (that is what this course gives you) and a place to type and save your **source code** — the human-readable text of your program.

That place should be a real **code editor**, not a plain text editor like Notepad. A proper code editor earns its keep with three features in particular:

1. **Line numbering** — so that when the compiler reports an error "on line 42," you can actually find line 42.
2. **Syntax highlighting** — coloring different parts of your code so structure is visible at a glance.
3. **A monospace (fixed-width) font** — so that easily confused characters like lowercase `l`, the digit `1`, and capital `I`, or the letter `O` and the digit `0`, are visually distinct.

> **Best practice:** Name your program's first or primary source file `main.cpp`. The name makes the program's entry point obvious to anyone reading the project — and real programs can grow to hundreds or thousands of source files, so a clear convention matters from day one.

---

## 0.5 — Introduction to the Compiler, Linker, and Libraries

Lesson 0.4 left off after you have written your source code. Now the toolchain takes over. This lesson covers steps 4 through 7 — what actually happens when you turn your `.cpp` files into a running program — because understanding this pipeline is what turns mysterious error messages into solvable problems.

### Step 4 — Compile your source code

The **compiler** is the program that reads each of your `.cpp` source files and does two distinct jobs.

First, it performs a **syntax check**, verifying that your code obeys the rules of C++. If it finds a violation, it stops and emits an **error** — usually with a line number pointing you at the problem — and refuses to go further until you fix it. (This is the syntax-error stream you met in lesson 0.1.)

Second, assuming the syntax is valid, it performs **translation**: it converts your C++ into the machine-language instructions the CPU can run, and writes them into an **object file** (named something like `main.o` on most systems, or `main.obj` on Windows). Object files also carry extra information needed for the next step and for debugging.

```text
 main.cpp --+
 add.cpp  --+-- [ COMPILER ] --> main.o
 io.cpp   --+                    add.o     (one object file per source file)
                                 io.o
```

The key idea: the compiler works on **one file at a time**, producing one object file per source file. None of these object files is a program yet — each is just a translated fragment.

### Step 5 — Link object files and libraries

The **linker** assembles those fragments into a finished product, usually an executable. Linking does four things:

1. **Reads and validates** each object file the compiler produced.
2. **Resolves cross-file references** — when code in one file *uses* something *defined* in another file, the linker matches the use to its definition. If it cannot find a definition for something that was used, you get a **linker error**.
3. **Links in libraries** — it pulls in precompiled, reusable code from **library files**.
4. **Produces the output** — the final executable (or, sometimes, a library).

```text
 main.o --+
 add.o  --+-- [ LINKER ] --> executable
 io.o   --+        ^
                   |
   libraries (standard + third-party)
```

A word on libraries, because you will use them immediately. The **C++ Standard Library** is a large bundle of ready-made functionality that ships with every C++ implementation. The part that handles console input and output is called **iostream**, and you will meet it in your very first program. Conveniently, most linkers include the standard library **automatically**, so you do not have to do anything special to use it.

**Third-party libraries** are written by other people to add capabilities the standard library does not provide — sound, graphics, networking, and so on. These are optional, and you link them in yourself when you need them.

> **Note:** The whole journey from source code to executable is called **building**, and the resulting executable is called a **build**. You will hear both words constantly.

### Steps 6 and 7 — Test and debug

Once you have an executable, two final steps close the loop. **Testing** means running your program with a variety of inputs to confirm it behaves as expected. **Debugging** means finding and fixing the defects when it does not. As noted earlier, debugging loops back into the development process, and you will return to it in depth in a later chapter.

### What an IDE is for

Look back at the process and count the separate tools it requires: an **editor** to write code (step 3), a **compiler** to translate it (step 4), a **linker** to assemble it (step 5), and a **debugger** to chase down problems (step 7). Juggling four programs is tedious. An **IDE — integrated development environment** — bundles all of them into a single application, so you can edit, build, run, and debug without leaving one window. The next lesson is about choosing and installing one.

---

## 0.6 — Installing an Integrated Development Environment (IDE)

An **IDE** is software designed to make it easy to develop, build, and debug your programs. As we saw in lesson 0.5, it bundles the separate tools of the development process into one place. A modern IDE gives you a **code editor** (with syntax highlighting, line numbering, and name completion), a **build system** that compiles and links for you, an **integrated debugger** for tracking down defects, and usually some form of **plugin support** to extend it.

To follow this course you only need to do two things: install an IDE, and make sure it comes with a reasonably modern C++ compiler. Here is the essence, kept deliberately brief — pick the option that matches your platform, install it with its default C++ workload, and move on.

- **Windows:** Visual Studio Community is the standard choice. During installation, select the "Desktop development with C++" workload so the compiler is included.
- **Linux or cross-platform:** Code::Blocks is a free, open-source option. On Windows, get the build that bundles the MinGW compiler; on Linux you may first need to install your distribution's basic build tools (often a `build-essential` package).
- **macOS:** Xcode (with its command-line tools) provides a compiler, and other cross-platform IDEs work as well.
- **Experienced users, any OS:** VS Code is popular but needs meaningful configuration to compile C++. It is excellent once set up, but it is not the gentlest starting point.

The one requirement that genuinely matters for this course is the **compiler version**. This course assumes at least **C++17**, so install a compiler that supports it — and prefer the newest version available, since newer compilers fix bugs and support later standards. As a rough guide, GCC/G++ 7, Clang 8, or Visual Studio 2017 (version 15.7) or later all support C++17. Avoid anything that tops out at C++11 or earlier.

> **Tip:** The simplest path is to pick an IDE that ships with a C++17-capable compiler already bundled, so you do not have to install and wire up a compiler separately.

> **Note:** If installation gives you trouble, that is normal — getting the toolchain working is genuinely the hardest part of starting out, and it gets much easier from here. Lesson 0.8 collects the most common stumbling blocks and their fixes.

---

## 0.7 — Compiling Your First Program

With an IDE installed, you are ready to build something. This lesson walks through the vocabulary your IDE uses and then your first real program.

### Projects, workspaces, and console applications

Your IDE organizes your work into **projects**. A project is a container that holds everything needed to build one program: your `.cpp` source files, plus any images or data, *plus the IDE and compiler settings* for that program. The project remembers its state between sessions, so you can close it and pick up where you left off.

> **Best practice:** Create a new project for each new program you write. This keeps each program's files and settings cleanly separated.

A **workspace** (Visual Studio calls it a *solution*) is a larger container that can hold one or more related projects. While you are learning, the simplest approach is one workspace per program — fewer moving parts means fewer ways to get confused.

The programs in this course are **console applications**. A console application runs in a text terminal with no graphical interface: it prints text and reads keyboard input, and it compiles to a standalone executable. This makes it ideal for learning, because it strips away the considerable extra machinery that graphical (GUI) applications require, letting you focus entirely on the language.

### The Hello World program

Here is the traditional first program in any language. Type it into your project's `main.cpp` and build it.

```cpp
#include <iostream>

int main()
{
    std::cout << "Hello, world!";
    return 0;
}
```

Three lines do the real work:

- `#include <iostream>` pulls in the standard library's input/output facilities — the `iostream` part you met in lesson 0.5.
- `std::cout << "Hello, world!";` sends the text to the console. (`cout` stands for "character output.")
- `return 0;` reports to the operating system that the program finished successfully.

Do not worry about understanding every symbol yet; the next chapter dissects this program line by line. For now, the goal is simply to get it to build and run.

### The build menu

Your IDE's build menu offers several commands. Knowing what each one does saves confusion later:

| Command | What it does |
|---|---|
| **Build** | Compiles every *modified* source file, then links everything into the executable. Unchanged files are skipped, so it is fast. |
| **Clean** | Deletes the cached object files and the executable, forcing the next build to start from scratch. |
| **Rebuild** | A Clean followed by a Build. |
| **Compile** | Recompiles a *single* file only — no linking, and no executable is produced. |
| **Run / Start** | Executes the program. Most IDEs build first automatically if anything has changed. |

### Sensible setup for learning

A few settings make the learning experience smoother. Each is covered in detail in a later lesson, but in brief: create a fresh project per program and use the **console application** type; set the compiler to a recent **C++ standard** (lesson 0.12); turn **compiler warnings** up (lesson 0.11); and for the small projects in this course, disable "precompiled headers" if your IDE offers that option, which avoids a class of confusing first-time errors. If your programs fail to run for no obvious reason, an over-eager antivirus tool is a common culprit — excluding your project directory from scanning often resolves it.

> **Note:** Lesson 0.8 expands on the most frequent first-build problems — a console window that flashes and vanishes, a precompiled-header error, or a program that builds but won't run — with concrete fixes for each.

---

## 0.8 — A Few Common C++ Problems

Almost everyone hits a few snags when they first set up a C++ environment. None of them mean you are doing something wrong; they are just the well-worn potholes of getting started. This lesson is a reference — skim it now, and return to it when something breaks.

### Runtime problems

**The console window flashes open and closes immediately.** This happens because a console program, once it reaches the end of `main`, has nothing left to do, so the window closes before you can read the output. The portable fix is to make the program wait for a keypress before it ends. Add these includes at the top of your file:

```cpp
#include <iostream>
#include <limits>
```

and place this just before `return` in `main`:

```cpp
std::cin.clear();                                                   // reset any error flags on the input stream
std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // discard any leftover input
std::cin.get();                                                     // wait for the user to press Enter
```

> **Warning:** Avoid the popular shortcut `system("pause")`. It only works on Windows and relies on an OS-specific command, so it is not portable. In Visual Studio specifically, running with **Start Without Debugging** (Ctrl+F5) also keeps the window open without any code changes.

**The program builds and runs but produces no output.** A virus scanner or anti-malware tool is occasionally to blame, blocking your freshly built executable. Temporarily disabling it, or excluding your project directory from scanning, usually fixes it.

**The program builds but behaves incorrectly.** This is a *logic error* — the code is valid C++ but does the wrong thing. There is no quick fix here; it is the normal work of programming, and you will learn systematic debugging techniques in a later chapter.

### Compile-time and link-time problems

A few error messages are common enough to recognize on sight:

- **`unresolved external symbol _main`** (or `_WinMain@16`): the linker cannot find your `main` function. Usually `main` is missing, misspelled, or its file is not actually part of the project. Check that `main` exists and is spelled correctly, and that its file belongs to the project; recreating the project as a console application often resolves it.
- **`main` is already defined / multiple definitions of `main`:** a program may have exactly **one** `main` function. Find and remove the extra one.
- **A modern C++ feature won't compile:** your compiler is too old, or it is set to too old a language standard. Upgrade the compiler or set the standard explicitly (lesson 0.12).
- **`cin`, `cout`, or `endl` is an "undeclared identifier":** you forgot `#include <iostream>`, or you forgot the `std::` prefix (it is `std::cout`, not bare `cout`).
- **`end1` is undeclared:** look closely — you typed the digit `1` instead of a lowercase letter `l` in `endl`. This is exactly why a monospace font matters, so that `l`, `1`, and `I` look different.

### A note on IDE-specific quirks

Some IDEs have their own idiosyncratic first-time errors. In Visual Studio, for example, a precompiled-header error (`C1010`, "unexpected end of file while looking for precompiled header directive") is resolved either by disabling precompiled headers or by adding `#include "pch.h"` as the very first line of each source file; and a `_WinMain@16` link error usually means the project was created as a GUI application rather than a console application. The specific messages vary by tool, but the pattern is the same: the wording is searchable, and the fix is usually a one-time project setting.

### When you are still stuck

If none of the above helps, the single most effective move is to **search for the exact error message, in quotes**, online — someone has almost certainly hit it before. Failing that, ask an AI assistant or post on a question-and-answer site such as Stack Overflow. Whatever you do, include the specifics: your operating system, your IDE, and the full error text. The more precisely you describe the problem, the faster you will get a useful answer.

---

## 0.9 — Configuring Your Compiler: Build Configurations

Your compiler is not a single fixed machine — it has dials. A **build configuration** (sometimes called a *build target*) is a named bundle of compiler and IDE settings that controls *how* your project is built: what the executable is called, where it looks for libraries, whether it includes debugging information, and how aggressively it optimizes the generated code. Every new project arrives with two of these configurations.

| | **Debug** | **Release** |
|---|---|---|
| Intended for | Active development and debugging | Shipping to users, or testing performance |
| Optimizations | **Off** | **On** — tuned for size and speed |
| Debug symbols | **Included** | **Stripped out** |
| The trade-off | Larger and slower, but much easier to debug | Smaller and fast, but hard to step through |

The size difference is striking. A trivial Hello World program might produce a roughly 65 KB executable under the Debug configuration but only about 12 KB under Release — several times smaller — because Release discards the debugging information and lets the optimizer compact the code.

The reason both configurations exist is that they serve opposite goals. The **Debug** build keeps your program faithful to the source so that a debugger can map machine instructions back to your lines and let you step through them. The **Release** build hands the optimizer freedom to rearrange and shrink your code for performance, at the cost of that easy correspondence.

> **Best practice:** Use the Debug configuration while you are developing. Switch to the Release configuration only when you are ready to share your program with others or want to measure real performance.

Switching between them is a quick menu or toolbar choice in any IDE (Visual Studio also offers a separate choice between 32-bit and 64-bit output). On the command line with GCC or Clang, the equivalents are roughly `-ggdb` for a debug build and `-O2 -DNDEBUG` for an optimized release build — `-O2` being the optimization level and `-DNDEBUG` switching off debug-only assertions.

> **Tip:** When you change a project setting in later lessons, change it for *all* of your build configurations, not just the one you happen to have selected. Otherwise the setting silently applies in Debug but not Release, or vice versa, which leads to baffling "it works one way but not the other" bugs.

---

## 0.10 — Configuring Your Compiler: Compiler Extensions

Here is a subtle trap that catches many beginners. The C++ standard defines what the language *is* — but individual compilers sometimes add their own **extensions**: non-standard features that go beyond what the standard specifies. These are often added to ease compatibility with other languages or older code (for instance, features borrowed from C99), or simply for historical reasons.

Extensions sound helpful, and occasionally they are, but for a learner they cause real harm:

- **They are not portable.** Code that relies on a compiler extension generally will not compile on a different compiler — or, worse, it may compile but behave differently. The portability you read about in lesson 0.2 quietly evaporates.
- **They are misleading.** Because an extension looks just like ordinary code, a beginner can easily mistake non-standard behavior for genuine C++, and walk away having "learned" something that is not actually part of the language.
- **They are often on by default,** which means you can be relying on them without ever having chosen to.

The remedy is to tell your compiler to refuse non-standard code, so that anything you write is portable, standard C++.

> **Best practice:** Disable compiler extensions. This keeps your programs — and the habits you build — compliant with the C++ standard, so your code will work on any compiler and any system.

Doing this is one setting. In Visual Studio it is the "Conformance mode" option (which corresponds to the `/permissive-` flag). With GCC, Clang, and Code::Blocks, you add the `-pedantic-errors` flag, which turns the use of non-standard extensions into hard errors.

> **Note:** This is a per-project setting in most IDEs, so you will need to set it again for each new project you create. The configuration lessons that follow are all in the same boat — which is exactly why IDEs let you save a project template once you have everything configured the way you like.

---

## 0.11 — Configuring Your Compiler: Warning and Error Levels

Your compiler can tell you about problems in your code at two levels of severity, and understanding the difference between them is important.

An **error** means the compiler has found something it cannot or will not translate — the code is invalid, or wrong enough to halt compilation. The compiler stops and you must fix it.

A **warning** means the compiler found something that is *technically legal* C++ but looks suspicious — the kind of thing that is usually a mistake even though the rules permit it. Crucially, the compiler **does not stop** for a warning; it flags the issue and keeps going. That is exactly why warnings are dangerous to ignore: your program still builds, so it is tempting to scroll past them, and a real bug can hide in the pile.

> **Key insight:** Whether a given issue is an error or merely a warning is partly up to the compiler — the same questionable code might be a warning on one compiler and an error on another. This is another reason to write clean, standard code rather than relying on how leniently any one compiler treats it.

> **Best practice:** Don't let warnings pile up. Resolve each one as you encounter it, as though it were an error. And especially while you are learning, turn your warning levels up so the compiler tells you about more potential problems, not fewer.

Turning warnings up is, again, just a flag or a setting. With GCC, Clang, or Code::Blocks, a strong set is:

```text
-Wall -Weffc++ -Wextra -Wconversion -Wsign-conversion
```

Here `-Wall` enables a broad set of common warnings, `-Wextra` adds more, `-Weffc++` flags style issues drawn from the *Effective C++* guidelines, and `-Wconversion` and `-Wsign-conversion` catch implicit numeric conversions (including between signed and unsigned types) that often hide bugs. In Visual Studio, the equivalent is setting the warning level to `/W4`.

You can go one step further and make the compiler **treat warnings as errors**, which forces you to address every single one before your program will build — an excellent discipline while learning. With GCC, Clang, or Code::Blocks this is the `-Werror` flag; in Visual Studio it is the "Treat Warnings As Errors" option (`/WX`).

> **Tip:** Think of high warning levels as free bug-finding. The compiler is already reading your entire program; turning warnings on, and then turning them into errors, is the cheapest code review you will ever get.

---

## 0.12 — Configuring Your Compiler: Choosing a Language Standard

As you learned in lesson 0.3, C++ is revised every few years, and each revision adds features. But there is a catch that surprises many newcomers: compilers usually **default to an older standard** — frequently C++14 — rather than the newest one they support. So if you want the modern features this course uses, you generally have to select the standard explicitly.

Before you do, it helps to know that each C++ version has had two names during its life: a **development codename** while it was being designed, and a **final name** once it was published. The codename ends in a letter; the final name ends in the release year.

| Codename | Final name | Published |
|---|---|---|
| C++0x | **C++11** | 2011 |
| C++1y | **C++14** | 2014 |
| C++1z | **C++17** | 2017 |
| C++2a | **C++20** | 2020 |
| C++2b | **C++23** | 2023 |
| C++2c | **C++26** | in progress |

You will occasionally see the codename in a compiler's settings (for example, an option labeled `c++2c` for the not-yet-final C++26), so it is worth recognizing both forms.

Why do compilers lag behind the newest standard by default? Partly to give compiler authors time to fix defects in their implementation of new features, and partly so that the best practices for using those features become well understood before they are pushed at everyone. Conservatism here also helps cross-platform compatibility, since not every system has the latest compiler.

> **Best practice:** To take full advantage of everything this course covers, set your compiler to use the latest language standard it supports. For this course you need at least C++17.

Selecting the standard is one setting per IDE. In Visual Studio it is the "C++ Language Standard" option (for example, `/std:c++17` or "ISO C++ Latest"). With GCC, Clang, or G++ you pass a flag such as `-std=c++17` (or `c++20`, `c++23`, and so on). Code::Blocks exposes the same flag through its compiler settings.

> **Note:** Make this change for *all* of your build configurations, as lesson 0.9 warned. And be aware that even after a standard is published, compilers may have **incomplete support** for some of its newer features for a while — so a feature being "part of C++20" does not guarantee your particular compiler implements it yet. Reference sites track which features each compiler supports.

---

## 0.13 — What Language Standard Is My Compiler Using?

It is genuinely useful to be able to ask your compiler, "which C++ standard are you actually using right now?" — if only to confirm that the setting you chose in lesson 0.12 took effect. There is a clean way to do this from inside a program.

C++ defines a special preprocessor macro called **`__cplusplus`**, and the compiler sets its value according to the standard it is compiling under. The value is a number encoding the year and month the standard was finalized, and you can map it back to a familiar name:

| `__cplusplus` value | Standard |
|---|---|
| `199711L` | Pre-C++11 |
| `201103L` | C++11 |
| `201402L` | C++14 |
| `201703L` | **C++17** |
| `202002L` | C++20 |
| `202302L` | C++23 |

The following diagnostic program reads that macro and prints the matching standard. Read past the small `#if` block — it exists only to work around one compiler quirk, explained below the code.

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

You are not expected to follow every line yet — arrays, `const`, pointers, and the preprocessor are all topics for later chapters. But the logic is worth a plain-English walkthrough, because it shows real code solving a real problem.

The program keeps two parallel lists: `stdCode` holds the numeric value of each standard, and `stdName` holds the matching human-readable name. The function `getCPPStandard` returns the value the compiler reported. Then `main` walks through the list looking for a home for that value, and there are two cases worth noticing:

- If the reported value **exactly matches** one of the known codes, the compiler is using precisely that standard.
- If the value falls **between** two known codes — smaller than the next entry in the list — then the compiler is using a *preview or pre-release* of that next standard, finalized after the previous one but before the next official release. The program reports it as such rather than pretending it is an exact match.

> **Note:** Visual Studio has a long-standing quirk: by default it does *not* report `__cplusplus` correctly, leaving it stuck at the "Pre-C++11" value unless you enable a specific flag. The program above sidesteps the problem by reading Microsoft's own `_MSVC_LANG` macro instead when it detects a Microsoft compiler, and it returns `-1` (handled with a polite error message) on the rare older versions where even that is unavailable. If you do get `-1`, revisit your standard setting from lesson 0.12.

With your compiler installed, configured, and verified, you are ready to start writing real C++. The next chapter takes the Hello World program apart and explains every piece of it.
