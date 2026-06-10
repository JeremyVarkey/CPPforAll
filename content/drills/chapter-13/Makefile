# Chapter 13 — Compound Types: Enums & Structs · Inventory Ledger · unit-test grader (Style B).
# Targets follow the drills/CLAUDE.md Makefile contract. TABS, not spaces.
#
# Layout:
#   ledger.h            — DECLARATIONS only (provided, complete; do NOT edit)
#   starter/ledger.cpp  — LEARNER fills in the 6 TASK blocks
#   solution/ledger.cpp — reference implementation (peek if stuck)
#   tests/tests.cpp     — grader: includes ../ledger.h, calls the API, CHECKs results
#
# The Makefile switches which .cpp the tests link against:
#   make test          -> tests/tests.cpp + starter/ledger.cpp  (RED until fixed)
#   make test-solution -> tests/tests.cpp + solution/ledger.cpp (MUST be green)

CXX      := clang++
CXXFLAGS := -std=c++17 -Wall -Wextra

.PHONY: all build run test solution test-solution clean

all: build

# ── build — compile-check the LEARNER's starter (warning-clean object) ────────
build:
	$(CXX) $(CXXFLAGS) -c starter/ledger.cpp -o starter/ledger.o
	@echo "OK \xE2\x9C\x85  starter/ledger.cpp compiles. Now run: make test"

# ── run — (no interactive driver for this lab; show a helpful message) ────────
run: build
	@echo "This lab has no interactive driver. Run  make test  to grade your code,"
	@echo "or  make solution  to build and run the reference solution demo."

# ── test — grade the LEARNER's code (RED until TASK blocks are filled in) ─────
test:
	$(CXX) $(CXXFLAGS) tests/tests.cpp starter/ledger.cpp -o tests/run
	@./tests/run || echo "FAIL \xE2\x9D\x8C  fill in the TASK blocks in starter/ledger.cpp until every check passes."

# ── solution — build and run the REFERENCE solution ──────────────────────────
solution: solution/demo
	./solution/demo
solution/demo: solution/ledger.cpp ledger.h
	$(CXX) $(CXXFLAGS) tests/tests.cpp solution/ledger.cpp -o solution/demo

# ── test-solution — proof the exercise is solvable: reference MUST be green ───
test-solution:
	$(CXX) $(CXXFLAGS) tests/tests.cpp solution/ledger.cpp -o tests/run
	@./tests/run

# ── clean — remove all build artifacts ───────────────────────────────────────
clean:
	rm -f starter/ledger.o tests/run solution/demo
	rm -rf tests/run.dSYM solution/demo.dSYM
