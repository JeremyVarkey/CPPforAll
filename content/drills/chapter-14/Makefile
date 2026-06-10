# Chapter 14 — Introduction to Classes · The Fraction Class · Style B2 grader.
# Targets follow the drills/CLAUDE.md Makefile contract. TABS, not spaces.
#
# Style B2 (header-only): the learner edits starter/fraction.h — the full class
# with all member bodies INLINE (the ch14 idiom; .h/.cpp split comes in ch15).
# The grader switches include paths (-Istarter vs -Isolution) instead of linking
# different .cpp files. tests/tests.cpp does #include "fraction.h" and picks up
# whichever directory is on -I. No demo main is separately built for this style.

CXX      := clang++
CXXFLAGS := -std=c++17 -Wall -Wextra

.PHONY: all build run test solution test-solution clean

all: build

# build — compile-check the LEARNER's header (starter/fraction.h).
# We compile the tests against the starter so that any error is surfaced early.
# This target deliberately succeeds even though the tests will FAIL at runtime;
# the "RED" behavior is a *runtime* fail, not a compile error.
build:
	$(CXX) $(CXXFLAGS) -Istarter tests/tests.cpp -o tests/run_starter
	@echo "OK ✅  starter/fraction.h compiles. Now run:  make test"

# run — run the starter grader so the learner sees the red output interactively.
run: build
	@./tests/run_starter || true

# test — grade the LEARNER's header: RED until the TASK blocks are filled in.
test:
	$(CXX) $(CXXFLAGS) -Istarter tests/tests.cpp -o tests/run_starter
	@./tests/run_starter || echo "FAIL ❌  fill in the TASK blocks in starter/fraction.h until every check passes."

# solution — compile and run the REFERENCE solution as a quick sanity demo.
solution:
	$(CXX) $(CXXFLAGS) -Isolution tests/tests.cpp -o tests/run_solution
	@./tests/run_solution

# test-solution — proof the lab is solvable: solution MUST pass every check.
test-solution:
	$(CXX) $(CXXFLAGS) -Isolution tests/tests.cpp -o tests/run_solution
	@./tests/run_solution

clean:
	rm -f tests/run_starter tests/run_solution
	rm -rf tests/run_starter.dSYM tests/run_solution.dSYM
