# Chapter 28 — Input and Output · The Report Engine · unit-test grader (Style B).
# Targets follow the drills/CLAUDE.md Makefile contract. TABS, not spaces.
#
# Layout: report.h (provided, declarations only) lives in the chapter root;
# the learner implements starter/report.cpp; the grader is tests/tests.cpp.
#
# File I/O note: Task 4 writes tests/tmp_report.txt (relative to this directory,
# which is the working directory when make is invoked). `make clean` removes it.

CXX      := clang++
CXXFLAGS := -std=c++17 -Wall -Wextra

.PHONY: all build run test solution test-solution clean

all: build

# ── build — compile-check the learner's starter/report.cpp ───────────────────
# Produces a warning-clean .o (object file only — no main, so no link).
# This is the "make" step in the learner's red→green loop.
build:
	$(CXX) $(CXXFLAGS) -c starter/report.cpp -o starter/report.o
	@echo "OK starter/report.cpp compiles. Now run: make test"

# run — build and run a minimal demo using the starter implementation
run: build
	@echo "(No interactive driver for this lab — use 'make test' to grade.)"

# ── test — grade the LEARNER's code ──────────────────────────────────────────
# Compile the grader against the LEARNER's starter/report.cpp.
# RED until the TASK blocks are filled in; GREEN once they're correct.
test:
	$(CXX) $(CXXFLAGS) tests/tests.cpp starter/report.cpp -o tests/run
	@./tests/run || echo "FAIL fill in the TASK blocks in starter/report.cpp until every check passes."

# ── solution — build and run the reference (for comparison / if stuck) ────────
solution:
	$(CXX) $(CXXFLAGS) tests/tests.cpp solution/report.cpp -o tests/run_sol
	@./tests/run_sol

# ── test-solution — proof the lab is solvable ─────────────────────────────────
# The reference MUST pass every check. This is our verification gate.
test-solution:
	$(CXX) $(CXXFLAGS) tests/tests.cpp solution/report.cpp -o tests/run_sol
	@./tests/run_sol

# ── clean — remove all build artifacts ────────────────────────────────────────
clean:
	rm -f starter/report.o tests/run tests/run_sol
	rm -f tests/tmp_report.txt
	rm -rf tests/run.dSYM tests/run_sol.dSYM
