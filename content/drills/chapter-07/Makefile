# Chapter 7 — Pass Telemetry · unit-test grader (Style B).
# Multi-file: telemetry.h (interface) + telemetry.cpp (impl) + demo.cpp (consumer).
# Targets follow the drills/CLAUDE.md Makefile contract. Recipes use TABS.

CXX      := clang++
CXXFLAGS := -std=c++17 -Wall -Wextra

.PHONY: all build run test solution test-solution clean

all: build

# Build the STARTER library together with the demo consumer (two TUs, then linked).
build: starter/app
starter/app: starter/telemetry.cpp demo.cpp telemetry.h
	$(CXX) $(CXXFLAGS) demo.cpp starter/telemetry.cpp -o starter/app

run: build
	./starter/app

# Grade the LEARNER's code: link the tests against the starter library.
test: tests/run-starter
	@./tests/run-starter || echo "FAIL ❌  fill in the TASK blocks until every CHECK passes."
tests/run-starter: tests/tests.cpp starter/telemetry.cpp telemetry.h
	$(CXX) $(CXXFLAGS) tests/tests.cpp starter/telemetry.cpp -o tests/run-starter

# Build + run the REFERENCE solution against the same demo.
solution: solution/app
	./solution/app
solution/app: solution/telemetry.cpp demo.cpp telemetry.h
	$(CXX) $(CXXFLAGS) demo.cpp solution/telemetry.cpp -o solution/app

# Proof the exercise is solvable: the reference must pass every check.
test-solution: tests/run-solution
	@./tests/run-solution
tests/run-solution: tests/tests.cpp solution/telemetry.cpp telemetry.h
	$(CXX) $(CXXFLAGS) tests/tests.cpp solution/telemetry.cpp -o tests/run-solution

clean:
	rm -f starter/app solution/app tests/run-starter tests/run-solution
