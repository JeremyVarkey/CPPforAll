# Chapter 12 — Compound Types: References and Pointers · Alias Workshop
# Targets follow the drills/CLAUDE.md Makefile contract.  TABS, not spaces.
#
# Style B unit-test grader: the learner implements aliases.h in starter/aliases.cpp;
# tests/tests.cpp links against it for grading.  The -I. flag lets every file
# #include "aliases.h" by its simple chapter-root-relative name.

CXX      := clang++
CXXFLAGS := -std=c++17 -Wall -Wextra -I.

.PHONY: all build run test solution test-solution clean

all: build

# ── build: compile the LEARNER's starter (warning-clean object file, no link) ──
build:
	$(CXX) $(CXXFLAGS) -c starter/aliases.cpp -o starter/aliases.o
	@echo "OK \xe2\x9c\x85  starter/aliases.cpp compiles.  Now run: make test"

# ── run: no interactive driver for this lab — point to test instead ─────────
run: build
	@echo "No interactive driver for this lab.  Use 'make test' to grade your code."

# ── test: grade the LEARNER's starter — RED on stub bodies, GREEN when done ──
test:
	$(CXX) $(CXXFLAGS) tests/tests.cpp starter/aliases.cpp -o tests/run
	@./tests/run || echo "FAIL \xe2\x9d\x8c  fill in the TASK blocks in starter/aliases.cpp until every check passes."

# ── solution: build + run the REFERENCE solution ─────────────────────────────
solution:
	$(CXX) $(CXXFLAGS) tests/tests.cpp solution/aliases.cpp -o tests/run
	@./tests/run

# ── test-solution: proof the exercise is solvable — MUST be green ────────────
test-solution:
	$(CXX) $(CXXFLAGS) tests/tests.cpp solution/aliases.cpp -o tests/run
	@./tests/run

clean:
	rm -f starter/aliases.o tests/run
	rm -rf tests/run.dSYM
