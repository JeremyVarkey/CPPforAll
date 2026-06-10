# Chapter 21 — Operator Overloading · Fraction v2 · unit-test grader (Style B).
# Targets follow the drills/CLAUDE.md Makefile contract. TABS, not spaces.
#
# Layout:
#   fraction.h           — the complete class declaration (declarations only)
#   starter/fraction.cpp — the learner fills in the TASK blocks
#   solution/fraction.cpp — reference implementation
#   tests/tests.cpp      — the grader; includes ../fraction.h
#
# The grader is compiled by linking tests/tests.cpp with EITHER starter or
# solution — the only difference is which -I path is searched for fraction.h.
# Both paths find the SAME header at the chapter root (not inside starter/ or
# solution/), so the header truly is the shared contract.

CXX      := clang++
# -I. puts the chapter root on the include path so #include "../fraction.h"
# resolves correctly from both starter/ and tests/ subdirectories.
CXXFLAGS := -std=c++17 -Wall -Wextra -I.

.PHONY: all build run test solution test-solution clean

all: build

# ── build — compile-check the learner's starter/fraction.cpp (warning-clean) ──
# The starter must compile out of the box so the learner can run make test
# immediately and see the RED failures.
build:
	$(CXX) $(CXXFLAGS) -c starter/fraction.cpp -o starter/fraction.o
	@echo "OK \xE2\x9C\x85  starter/fraction.cpp compiles. Run: make test"

# ── run — (no interactive program for this exercise; use make test) ────────────
run: build
	@echo "No interactive driver — use: make test  or  make test-solution"

# ── test — grade the LEARNER's starter/fraction.cpp ───────────────────────────
# RED on the starter (stubs return wrong values); GREEN after all TASK blocks
# are correctly filled in.
test: tests/tests.cpp fraction.h starter/fraction.cpp
	@$(CXX) $(CXXFLAGS) tests/tests.cpp starter/fraction.cpp -o tests/run
	@./tests/run || echo "FAIL \xE2\x9D\x8C  fill in the TASK blocks in starter/fraction.cpp until every check passes."

# ── solution — build and show what the reference solution does ─────────────────
solution: solution/fraction.cpp fraction.h
	$(CXX) $(CXXFLAGS) tests/tests.cpp solution/fraction.cpp -o solution/run
	./solution/run

# ── test-solution — proof the lab is solvable; MUST be green ──────────────────
test-solution: tests/tests.cpp fraction.h solution/fraction.cpp
	@$(CXX) $(CXXFLAGS) tests/tests.cpp solution/fraction.cpp -o tests/run
	@./tests/run

clean:
	rm -f starter/fraction.o tests/run solution/run
	rm -rf tests/run.dSYM solution/run.dSYM
