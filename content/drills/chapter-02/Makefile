# Chapter 2 — geo mini-library · unit-test grader.
# Targets follow the drills/CLAUDE.md Makefile contract. (Recipes use TABS.)
#
# Multi-file build (the point of the chapter): the program/grader is compiled
# from TWO .cpp files at once — a consumer (demo.cpp or tests/tests.cpp) plus an
# implementation (starter/geo.cpp or solution/geo.cpp). geo.h is included by all
# of them; it is never compiled on its own.

CXX      := clang++
# -I. puts the chapter root on the include path, so every file can write the
# paired-header include by its simple name: #include "geo.h" (matches the 2.11
# add.h/add.cpp convention, and how real projects — and LLVM — find headers).
CXXFLAGS := -std=c++17 -Wall -Wextra -I.

.PHONY: all build run test solution test-solution clean

all: build

# ── Starter: link demo.cpp against the LEARNER's starter/geo.cpp ────────────
build: starter/app
starter/app: demo.cpp geo.h starter/geo.cpp
	$(CXX) $(CXXFLAGS) demo.cpp starter/geo.cpp -o starter/app

run: build
	./starter/app

# ── The grader: compile the tests with the LEARNER's starter/geo.cpp ───────
# RED until the TASK blocks are filled in; GREEN once they are.
test: tests/tests.cpp geo.h starter/geo.cpp
	@$(CXX) $(CXXFLAGS) tests/tests.cpp starter/geo.cpp -o tests/run
	@./tests/run || echo "FAIL ❌  fill in the TASK blocks in starter/geo.cpp until every check passes."

# ── The reference solution, built + run as a program ───────────────────────
solution: solution/app
	./solution/app
solution/app: demo.cpp geo.h solution/geo.cpp
	$(CXX) $(CXXFLAGS) demo.cpp solution/geo.cpp -o solution/app

# ── Proof the exercise is solvable: tests + solution/geo.cpp MUST be green ──
test-solution: tests/tests.cpp geo.h solution/geo.cpp
	@$(CXX) $(CXXFLAGS) tests/tests.cpp solution/geo.cpp -o tests/run
	@./tests/run

clean:
	rm -f starter/app solution/app tests/run
