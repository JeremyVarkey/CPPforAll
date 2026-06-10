# Chapter 5 — Name-Badge / Greeting Formatter · unit-test grader (Style B).
# Targets follow the drills/CLAUDE.md Makefile contract. TABS, not spaces.
#
# The learner implements function bodies in starter/badge.cpp against the API in
# badge.h. tests/tests.cpp links against EITHER starter or solution:
#   make / make build    compile the starter library (proves it builds)
#   make test            grade the STARTER   -> RED until the TODOs are filled in
#   make test-solution   grade the SOLUTION  -> MUST be GREEN (proof it's solvable)

CXX      := clang++
CXXFLAGS := -std=c++17 -Wall -Wextra

.PHONY: all build run test solution test-solution clean

all: build

# `make build` — compile the starter library on its own (syntax/warning check).
# There is no main() here (this is a function library), so we compile to an
# object file rather than an executable.
build: starter/badge.o
starter/badge.o: starter/badge.cpp badge.h
	$(CXX) $(CXXFLAGS) -c starter/badge.cpp -o starter/badge.o

# `make run` — exercise the starter's functions by running the grader against
# them (a library has no program of its own to run).
run: test

# `make test` — grade the LEARNER's code: link the tests against starter/badge.cpp.
test: tests/tests.cpp starter/badge.cpp badge.h
	@$(CXX) $(CXXFLAGS) tests/tests.cpp starter/badge.cpp -o tests/run && ./tests/run

# `make solution` — build + run the grader against the reference solution.
solution: test-solution

# `make test-solution` — grade the SOLUTION: must be green (our proof it solves).
test-solution: tests/tests.cpp solution/badge.cpp badge.h
	@$(CXX) $(CXXFLAGS) tests/tests.cpp solution/badge.cpp -o tests/run && ./tests/run

clean:
	rm -f starter/badge.o solution/badge.o tests/run
