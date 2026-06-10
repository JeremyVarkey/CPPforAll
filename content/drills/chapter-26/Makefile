# Chapter 26 — Templates and Classes · Stack<T> · header-only unit-test grader (Style B2).
# Targets follow the drills/CLAUDE.md Makefile contract.  TABS, not spaces.
#
# Style B2 explanation (CLAUDE.md):
#   The learner edits starter/stack.h (the FULL class template, bodies inline).
#   tests/tests.cpp does  #include "stack.h"  and the Makefile injects the
#   right directory via -I so the correct stack.h is found.
#
#   test:          ; $(CXX) $(CXXFLAGS) -Istarter  tests/tests.cpp -o tests/run && ./tests/run
#   test-solution: ; $(CXX) $(CXXFLAGS) -Isolution tests/tests.cpp -o tests/run && ./tests/run
#
# Why no starter/main.cpp?  The test harness supplies its own main().  A stack
# template doesn't need a "playable driver" the way the engine did — the grader
# IS the demo.  (See chapter-08 for the Style B variant with a separate demo.)

CXX      := clang++
CXXFLAGS := -std=c++17 -Wall -Wextra

.PHONY: all build run test solution test-solution clean

all: build

# build — compile-check the learner's starter/stack.h (warning-clean).
# Because everything is header-only, we compile tests.cpp against the starter
# for the build check so any syntax error in the header is caught immediately.
build:
	@$(CXX) $(CXXFLAGS) -Istarter tests/tests.cpp -o tests/build_check 2>&1 || true
	@$(CXX) $(CXXFLAGS) -Istarter tests/tests.cpp -o tests/build_check
	@echo "OK \xE2\x9C\x85  starter/stack.h compiles.  Now run: make test"

# run — just re-runs the build check (there is no interactive driver here).
run: build
	@echo "(no interactive driver for this exercise — use 'make test' to grade)"

# test — grade the LEARNER's code: compile + run against starter/stack.h.
# RED until the TASK blocks are filled in; GREEN once they are.
test:
	$(CXX) $(CXXFLAGS) -Istarter tests/tests.cpp -o tests/run
	@./tests/run || echo "FAIL \xE2\x9D\x8C  fill in the TASK blocks in starter/stack.h until every check passes."

# solution — show the reference solution output (acts as a demo).
solution:
	$(CXX) $(CXXFLAGS) -Isolution tests/tests.cpp -o tests/run_sol
	./tests/run_sol

# test-solution — proof the lab is solvable: solution MUST pass every check.
test-solution:
	$(CXX) $(CXXFLAGS) -Isolution tests/tests.cpp -o tests/run
	@./tests/run

clean:
	rm -f tests/run tests/run_sol tests/build_check
	rm -rf tests/run.dSYM tests/run_sol.dSYM tests/build_check.dSYM
