# Chapter 24 — Inheritance · Report-Logger Family · unit-test grader (Style B).
# Targets follow the drills/CLAUDE.md Makefile contract. TABS, not spaces.
#
# The learner edits starter/logger.cpp (three class bodies + processLog).
# The header logger.h is the CONTRACT: complete and provided.
# tests/tests.cpp includes ../logger.h and calls the API through BOTH
# the starter and the solution. The grader is linked against the
# appropriate .cpp with $(CXX) in one command.
#
# -I. puts the chapter root on the include path so every file can write
#   #include "logger.h"   (no path prefix needed)

CXX      := clang++
CXXFLAGS := -std=c++17 -Wall -Wextra -I.

.PHONY: all build run test solution test-solution clean

all: build

# ── Starter: compile-check the learner's logger.cpp (warning-clean object) ──
build:
	$(CXX) $(CXXFLAGS) -c starter/logger.cpp -o starter/logger.o
	@echo "OK ✅  starter/logger.cpp compiles. Now run: make test"

# ── run — build and run the starter's demo main ─────────────────────────────
run: build
	$(CXX) $(CXXFLAGS) starter/main.cpp starter/logger.cpp -o starter/demo
	./starter/demo

# ── test — grade the LEARNER's code ─────────────────────────────────────────
# RED until the TASK blocks are filled in; GREEN once they are correct.
test:
	$(CXX) $(CXXFLAGS) tests/tests.cpp starter/logger.cpp -o tests/run
	@./tests/run || echo "FAIL ❌  fill in the TASK blocks in starter/logger.cpp until every check passes."

# ── solution — build + run the reference demo ────────────────────────────────
solution: solution/demo
	./solution/demo
solution/demo: solution/main.cpp solution/logger.cpp
	$(CXX) $(CXXFLAGS) solution/main.cpp solution/logger.cpp -o solution/demo

# ── test-solution — proof the lab is solvable ────────────────────────────────
# The reference solution MUST pass every check.
test-solution:
	$(CXX) $(CXXFLAGS) tests/tests.cpp solution/logger.cpp -o tests/run
	@./tests/run

clean:
	rm -f starter/logger.o starter/demo solution/demo tests/run
	rm -rf starter/demo.dSYM solution/demo.dSYM tests/run.dSYM
