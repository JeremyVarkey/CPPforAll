# Chapter 3 — Coin-Tray Auditor · unit-test grader (Style B).
# Targets follow the drills/CLAUDE.md Makefile contract. Recipes use TABS.
#
# The starter is a library of functions (no main of its own), so `build`
# COMPILES it (-c) to prove it has no syntax / compile-time errors — the whole
# point of the chapter is that a clean compile does NOT mean correct. `run`,
# `test`, and `test-solution` link the grader (tests/tests.cpp) against either
# the starter bodies or the reference bodies.

CXX      := clang++
CXXFLAGS := -std=c++17 -Wall -Wextra

.PHONY: all build run test solution test-solution clean

all: build

# Compile-only: proves the starter builds warning-clean (no linked main needed).
build: starter/coin_tray.cpp coin_tray.h
	$(CXX) $(CXXFLAGS) -c starter/coin_tray.cpp -o starter/coin_tray.o

# "Run" the starter == run the grader against it (a library's natural exercise).
run: test

# Your red->green loop: grade the STARTER bodies.
test: starter/coin_tray.cpp coin_tray.h tests/tests.cpp
	@$(CXX) $(CXXFLAGS) tests/tests.cpp starter/coin_tray.cpp -o tests/run
	@./tests/run

# Build + run the grader against the reference solution (for when you're stuck).
solution: solution/coin_tray.cpp coin_tray.h tests/tests.cpp
	@$(CXX) $(CXXFLAGS) tests/tests.cpp solution/coin_tray.cpp -o tests/run
	@./tests/run

# Proof the exercise is solvable: the reference bodies MUST pass every check.
test-solution: solution/coin_tray.cpp coin_tray.h tests/tests.cpp
	@$(CXX) $(CXXFLAGS) tests/tests.cpp solution/coin_tray.cpp -o tests/run
	@./tests/run

clean:
	rm -f starter/coin_tray.o tests/run
