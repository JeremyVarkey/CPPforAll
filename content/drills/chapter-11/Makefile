# Chapter 11 — Function Overloading & Function Templates
#              Project: the generic toolkit mini-library
#              Style B2 grader (header-only). TABS, not spaces.
#
# Targets follow the drills/CLAUDE.md Makefile contract.
#
# Style B2: the learner edits starter/toolkit.h — all function bodies are
# inline in the header (necessary for templates, convenient for non-templates
# in the same file). The grader (tests/tests.cpp) does #include "toolkit.h"
# and picks up the right version via -Istarter or -Isolution. No separate
# .cpp files — same pattern as Chapter 14.

CXX      := clang++
CXXFLAGS := -std=c++17 -Wall -Wextra

.PHONY: all build run test solution test-solution clean

all: build

# build — compile-check the LEARNER's header (starter/toolkit.h).
# This compiles the tests against the starter so that any syntax error in the
# header is surfaced immediately. Tests will FAIL at runtime (RED) until
# TASK blocks are filled in — that's expected and correct.
build:
	$(CXX) $(CXXFLAGS) -Istarter tests/tests.cpp -o tests/run_starter
	@echo "OK \xe2\x9c\x85  starter/toolkit.h compiles. Now run: make test"

# run — show the learner what RED looks like.
run: build
	@./tests/run_starter || true

# test — grade the LEARNER's header (RED until TASK blocks are filled in).
test:
	$(CXX) $(CXXFLAGS) -Istarter tests/tests.cpp -o tests/run_starter
	@./tests/run_starter || echo "FAIL \xe2\x9d\x8c  fill in the TASK blocks in starter/toolkit.h until every check passes."

# solution — compile and run the REFERENCE header as a quick sanity demo.
solution:
	$(CXX) $(CXXFLAGS) -Isolution tests/tests.cpp -o tests/run_solution
	@./tests/run_solution

# test-solution — proof the lab is solvable: the reference MUST pass every check.
test-solution:
	$(CXX) $(CXXFLAGS) -Isolution tests/tests.cpp -o tests/run_solution
	@./tests/run_solution

clean:
	rm -f tests/run_starter tests/run_solution tests/run a.out
	rm -rf tests/run_starter.dSYM tests/run_solution.dSYM
