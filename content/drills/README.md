# Drills

Small standalone exercises to make concepts physical. One subfolder per drill — keep them self-contained with a `README.md` describing the goal and a `Makefile` or build command.

## Suggested Drills

Order roughly by usefulness for upcoming labs.

1. **`hello-cpp/`** — bare-bones `main.cpp` + Makefile. Compile from the command line with `g++` (or `clang++`), run, observe. Proves the host C++ toolchain works.
2. **`linked-list/`** — pointer-heavy linked list with `new`/`delete` and a destructor that frees nodes. Internalizes pointer ownership.
3. **`shape-hierarchy/`** — abstract `Shape` with virtual `area()`, concrete `Circle`, `Rectangle`. Mirrors the LLVM Instruction class hierarchy.
4. **`llvm-count-uniq-ops/`** — modify Lab 0's counter to also count unique opcodes per function. New code, new questions, same skeleton. Run inside the course Docker container.
5. **`llvm-walk-cfg/`** — for each function, print the control-flow graph as adjacency text: each BasicBlock's successors. Forces familiarity with `BB.getTerminator()` and `succ_begin/succ_end`.
6. **`fuzzer-toy/`** — pure C, no LLVM: a tiny "fuzzer" that mutates an input string and tries to crash a target. Conceptual prep for Lab 1 (Fuzzing).

## Rules

- No solutions to course labs go in here. Drills must produce different outputs or solve different problems.
- Each drill ends with a one-line takeaway logged in `../learning-log.md`.
