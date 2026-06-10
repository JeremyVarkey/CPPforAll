# Chapter 20 — Functions II (Recursion, Function Pointers, Lambdas)
# Project: calc-core · unit-test grader (Style B).
# Targets follow the drills/CLAUDE.md Makefile contract. Recipes use TABS.
#
# Layout:
#   calccore.h          — API declarations (provided, complete)
#   starter/calccore.cpp — LEARNER fills in the TODO blocks
#   solution/calccore.cpp — reference implementation
#   demo.cpp            — ungraded CLI demo (shows recursion, fn ptrs, lambdas live)
#   tests/tests.cpp     — automated grader; links against starter or solution
#
# -I. puts the chapter root on the include path so every file can write
# #include "calccore.h" by its simple name.

CXX      := clang++
CXXFLAGS := -std=c++17 -Wall -Wextra -I.

.PHONY: all build run test solution test-solution clean

all: build

# ── Starter build: compile-check the learner's calccore.cpp and build demo ───
# Two targets: an object-file compile-check (catches warnings fast) and the
# full demo executable so `make run` works.
build: starter/demo
starter/demo: demo.cpp calccore.h starter/calccore.cpp
	$(CXX) $(CXXFLAGS) demo.cpp starter/calccore.cpp -o starter/demo
	@echo "OK \xE2\x9C\x85  starter/calccore.cpp compiles.  Run: make test   (or: make run to see the demo)"

# ── Run: demo with the learner's implementation ───────────────────────────────
run: build
	./starter/demo

# ── Test: grade the LEARNER's starter code ───────────────────────────────────
# RED until the TASK blocks are filled in; GREEN once they are.
test: tests/tests.cpp calccore.h starter/calccore.cpp
	@$(CXX) $(CXXFLAGS) tests/tests.cpp starter/calccore.cpp -o tests/run
	@./tests/run || echo "FAIL \xE2\x9D\x8C  fill in the TASK blocks in starter/calccore.cpp until every check passes."

# ── Solution: build + run the reference demo ─────────────────────────────────
solution: solution/demo
	./solution/demo
solution/demo: demo.cpp calccore.h solution/calccore.cpp
	$(CXX) $(CXXFLAGS) demo.cpp solution/calccore.cpp -o solution/demo

# ── test-solution: proof the exercise is solvable (MUST be green) ────────────
test-solution: tests/tests.cpp calccore.h solution/calccore.cpp
	@$(CXX) $(CXXFLAGS) tests/tests.cpp solution/calccore.cpp -o tests/run
	@./tests/run

clean:
	rm -f starter/demo solution/demo tests/run
	rm -rf starter/demo.dSYM solution/demo.dSYM tests/run.dSYM
