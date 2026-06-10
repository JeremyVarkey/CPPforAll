# Chapter 6 — Operators · Day-One Triage Console · unit-test grader (Style B).
# Targets follow the drills/CLAUDE.md Makefile contract. TABS, not spaces.
#
# The learner implements ../triage.h in starter/triage.cpp. There is no main() in
# triage.cpp on purpose — the grader (tests/tests.cpp) supplies main() and calls
# every function across many inputs, so the learner never writes a loop.

CXX      := clang++
CXXFLAGS := -std=c++17 -Wall -Wextra

.PHONY: all build run test solution test-solution clean

all: build

# build — compile-check the learner's code (warning-clean object file, no link).
# There is no program to link yet because tests/ provides main(); `make test`
# does the full build+run. This target just proves your triage.cpp compiles.
build:
	$(CXX) $(CXXFLAGS) -c starter/triage.cpp -o starter/triage.o
	@echo "OK ✅  starter/triage.cpp compiles. Now run: make test"

# run — for this lab, "running" the code IS running the grader against it.
run: test

# test — grade the LEARNER's code: link the grader against starter/triage.cpp.
# RED until the TASK blocks are filled in; GREEN once they're correct.
test:
	$(CXX) $(CXXFLAGS) tests/tests.cpp starter/triage.cpp -o tests/run
	@./tests/run

# solution — build+run the grader against the reference implementation.
solution: test-solution

# test-solution — proof the lab is solvable: the reference MUST pass every check.
test-solution:
	$(CXX) $(CXXFLAGS) tests/tests.cpp solution/triage.cpp -o tests/run
	@./tests/run

clean:
	rm -f starter/triage.o tests/run
	rm -rf tests/run.dSYM
