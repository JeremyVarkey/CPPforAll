# Chapter 16 — Dynamic Arrays: std::vector · Sensor-Readings Toolkit · unit-test grader (Style B).
# Targets follow the drills/CLAUDE.md Makefile contract. (Recipes use TABS.)
#
# Layout mirrors chapter-02 (the canonical Style B exemplar):
#   sensor.h         — declarations only (provided, complete)
#   starter/sensor.cpp  — TASK blocks for the learner to fill in
#   solution/sensor.cpp — reference implementation
#   tests/tests.cpp  — the grader; includes ../sensor.h (found via -I.)
#
# -I. puts the chapter root on the include path so every file can write
# #include "sensor.h" by its simple name (mirrors ch 2 / LLVM convention).

CXX      := clang++
CXXFLAGS := -std=c++17 -Wall -Wextra -I.

.PHONY: all build run test solution test-solution clean

all: build

# ── Starter: compile-check the learner's sensor.cpp ──────────────────────────
build:
	$(CXX) $(CXXFLAGS) -c starter/sensor.cpp -o starter/sensor.o
	@echo "OK \xE2\x9C\x85  starter/sensor.cpp compiles. Now run: make test"

# ── Run: no interactive main in this lab, just print usage hint ───────────────
run: build
	@echo "No interactive driver for this lab."
	@echo "Use: make test          (grade your solution)"
	@echo "     make test-solution (verify the reference)"

# ── Grade the LEARNER's code (RED until TASK blocks are filled in) ────────────
test:
	$(CXX) $(CXXFLAGS) tests/tests.cpp starter/sensor.cpp -o tests/run
	@./tests/run || echo "FAIL \xE2\x9D\x8C  fill in the TASK blocks in starter/sensor.cpp until every check passes."

# ── Proof the exercise is solvable (MUST be green) ───────────────────────────
test-solution:
	$(CXX) $(CXXFLAGS) tests/tests.cpp solution/sensor.cpp -o tests/run
	@./tests/run

# ── Peek at the reference (builds + prints it was built, no interactive run) ──
solution:
	$(CXX) $(CXXFLAGS) -c solution/sensor.cpp -o solution/sensor.o
	@echo "Reference solution compiled. See solution/sensor.cpp for the annotated implementation."

clean:
	rm -f starter/sensor.o solution/sensor.o tests/run
	rm -rf tests/run.dSYM
