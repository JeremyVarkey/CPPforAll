# Chapter 19 — Dynamic Allocation · DynBuffer Workbench · unit-test grader (Style B).
# Targets follow the drills/CLAUDE.md Makefile contract. TABS, not spaces.
#
# Layout:
#   dynbuf.h            — DECLARATIONS only (provided, do not edit)
#   starter/dynbuf.cpp  — bodies with TASK blocks (learner fills in)
#   solution/dynbuf.cpp — reference bodies (peek when stuck)
#   tests/tests.cpp     — grader: includes ../dynbuf.h, calls the API, CHECKs results
#
# The Makefile compiles tests/tests.cpp against EITHER starter or solution .cpp
# by swapping the second source file — same -I. brings dynbuf.h in by its bare name.

CXX      := clang++
# -I. puts the chapter root on the include path so #include "dynbuf.h" resolves.
CXXFLAGS := -std=c++17 -Wall -Wextra -I.

.PHONY: all build run test solution test-solution clean

all: build

# ── build: compile-check the learner's starter (warning-clean object file) ──
build:
	$(CXX) $(CXXFLAGS) -c starter/dynbuf.cpp -o starter/dynbuf.o
	@echo "OK \xE2\x9C\x85  starter/dynbuf.cpp compiles.  Now run: make test"

# run: not meaningful for a pure library — remind the learner to use test.
run: build
	@echo "(No standalone driver for this chapter. Use: make test)"

# ── test: grade the LEARNER's starter — RED until TASK blocks are filled ────
test:
	$(CXX) $(CXXFLAGS) tests/tests.cpp starter/dynbuf.cpp -o tests/run
	@./tests/run || echo "FAIL \xE2\x9D\x8C  fill in the TASK blocks in starter/dynbuf.cpp until every check passes."

# ── solution: build + silently run the reference against its own tests ───────
solution:
	$(CXX) $(CXXFLAGS) tests/tests.cpp solution/dynbuf.cpp -o tests/run_sol
	@./tests/run_sol

# ── test-solution: proof the lab is solvable — MUST be green ─────────────────
test-solution:
	$(CXX) $(CXXFLAGS) tests/tests.cpp solution/dynbuf.cpp -o tests/run
	@./tests/run

clean:
	rm -f starter/dynbuf.o tests/run tests/run_sol
	rm -rf tests/run.dSYM tests/run_sol.dSYM
