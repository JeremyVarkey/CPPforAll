# Chapter 18 — Iterators and Algorithms · VecTools · unit-test grader (Style B).
# Targets follow the drills/CLAUDE.md Makefile contract. TABS, not spaces.
#
# The learner implements ../vectools.h in starter/vectools.cpp.
# The grader (tests/tests.cpp) calls every vectools:: function across many
# inputs — including edge cases — and checks hand* vs algo* agree.
#
# -I. puts the chapter root on the include path so every file can write
# #include "vectools.h" by its simple name.

CXX      := clang++
CXXFLAGS := -std=c++17 -Wall -Wextra -I.

.PHONY: all build run test solution test-solution clean

all: build

# ── Starter: compile-check the learner's vectools (warning-clean object) ────
build:
	$(CXX) $(CXXFLAGS) -c starter/vectools.cpp -o starter/vectools.o
	@echo "OK \xe2\x9c\x85  starter/vectools.cpp compiles. Now run: make test"

# run — no interactive program for this lab; print a helpful note instead.
run: build
	@echo "This lab has no interactive driver. Use 'make test' to grade, or 'make solution' to run the reference."

# ── Grader: link tests against the LEARNER's starter/vectools.cpp ───────────
# RED until the TASK blocks are filled in; GREEN once they are.
test:
	$(CXX) $(CXXFLAGS) tests/tests.cpp starter/vectools.cpp -o tests/run
	@./tests/run || echo "FAIL \xe2\x9d\x8c  fill in the TASK blocks in starter/vectools.cpp until every check passes."

# ── Reference solution ───────────────────────────────────────────────────────
solution:
	$(CXX) $(CXXFLAGS) tests/tests.cpp solution/vectools.cpp -o tests/run_sol
	@./tests/run_sol

# ── Proof the exercise is solvable: tests + solution MUST be green ───────────
test-solution:
	$(CXX) $(CXXFLAGS) tests/tests.cpp solution/vectools.cpp -o tests/run_sol
	@./tests/run_sol

clean:
	rm -f starter/vectools.o tests/run tests/run_sol
	rm -rf tests/run.dSYM tests/run_sol.dSYM
