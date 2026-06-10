# Chapter 27 — Exceptions · SafeConfig Parser · unit-test grader (Style B).
# Targets follow the drills/CLAUDE.md Makefile contract. TABS, not spaces.
#
# Layout:
#   config_parser.h         — declarations (the CONTRACT; do not edit)
#   starter/config_parser.cpp — LEARNER's bodies + TASK blocks
#   solution/config_parser.cpp — reference bodies
#   tests/tests.cpp         — grader: includes ../config_parser.h, CHECKs results
#
# The grader is linked against starter/ or solution/ via the Makefile target.

CXX      := clang++
CXXFLAGS := -std=c++17 -Wall -Wextra

.PHONY: all build run test solution test-solution clean

all: build

# ── build: compile-check the starter (warning-clean object file, then link) ──
build:
	$(CXX) $(CXXFLAGS) -c starter/config_parser.cpp -o starter/config_parser.o
	@echo "OK ✅  starter/config_parser.cpp compiles. Now run: make test"

# ── run: no interactive runner for this lab; redirect to test ─────────────────
run: build
	@echo "(No interactive driver for this lab — use 'make test' to grade your code.)"

# ── test: grade the LEARNER's code ───────────────────────────────────────────
# RED until all TASK blocks are filled in; GREEN once correct.
test:
	$(CXX) $(CXXFLAGS) tests/tests.cpp starter/config_parser.cpp -o tests/run
	@./tests/run || echo "FAIL ❌  fill in the TASK blocks in starter/config_parser.cpp."

# ── solution: build + run the reference solution ─────────────────────────────
solution: solution/app
	./solution/app
solution/app: tests/tests.cpp solution/config_parser.cpp
	$(CXX) $(CXXFLAGS) tests/tests.cpp solution/config_parser.cpp -o solution/app

# ── test-solution: proof the lab is solvable (MUST be green) ─────────────────
test-solution:
	$(CXX) $(CXXFLAGS) tests/tests.cpp solution/config_parser.cpp -o tests/run
	@./tests/run

# ── clean ─────────────────────────────────────────────────────────────────────
clean:
	rm -f starter/config_parser.o tests/run solution/app
	rm -rf tests/run.dSYM solution/app.dSYM
