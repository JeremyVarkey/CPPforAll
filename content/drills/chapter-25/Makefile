# Chapter 25 — Virtual Functions · MINI-LLVM · unit-test grader (Style B).
# Targets follow the drills/CLAUDE.md Makefile contract. TABS, not spaces.
#
# Layout:
#   inst.h              — declarations only (provided, complete)
#   starter/inst.cpp    — bodies with TASK blocks (learner fills in)
#   solution/inst.cpp   — reference bodies
#   tests/tests.cpp     — includes ../inst.h, calls the API, CHECKs results
#
# -I. puts the chapter root on the include path so every file can write:
#   #include "inst.h"
# (mirrors how LLVM passes find their own headers via -I<pass_root>)

CXX      := clang++
CXXFLAGS := -std=c++17 -Wall -Wextra -I.

.PHONY: all build run test solution test-solution clean

all: build

# ── Starter: compile-check the learner's inst.cpp (object only, no link) ─────
# Building an object file verifies the code is warning-clean without needing a
# main(). The test target does the full grader build + run.
build:
	$(CXX) $(CXXFLAGS) -c starter/inst.cpp -o starter/inst.o
	@echo "OK \xE2\x9C\x85  starter/inst.cpp compiles. Now run: make test"

# run — show the learner the solution in action for a quick sanity check.
run: build
	@echo "(no standalone driver; run 'make test' to grade or 'make solution' to see the reference)"

# ── The grader: link the tests against the LEARNER's starter/inst.cpp ─────────
# RED until the TASK blocks are filled in; GREEN once they are.
test:
	$(CXX) $(CXXFLAGS) tests/tests.cpp starter/inst.cpp -o tests/run
	@./tests/run || echo "FAIL \xE2\x9D\x8C  fill in the TASK blocks in starter/inst.cpp until every check passes."

# ── The reference solution: build + run as a proof ───────────────────────────
solution:
	$(CXX) $(CXXFLAGS) tests/tests.cpp solution/inst.cpp -o solution/run
	./solution/run

# ── Proof the exercise is solvable: the reference MUST pass every check ───────
test-solution:
	$(CXX) $(CXXFLAGS) tests/tests.cpp solution/inst.cpp -o tests/run
	@./tests/run

clean:
	rm -f starter/inst.o tests/run solution/run
	rm -rf tests/run.dSYM solution/run.dSYM
