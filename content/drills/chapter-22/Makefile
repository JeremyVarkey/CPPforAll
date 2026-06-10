# Chapter 22 — Move Semantics & Smart Pointers · TrackedBuffer · unit-test grader (Style B).
# Targets follow the drills/CLAUDE.md Makefile contract. TABS, not spaces.
#
# Layout:
#   tracked_buffer.h          — declarations only (provided, complete; do not edit)
#   starter/tracked_buffer.cpp — bodies with TASK blocks (learner fills in)
#   solution/tracked_buffer.cpp — reference bodies
#   tests/tests.cpp            — includes ../tracked_buffer.h, calls the API, CHECKs results
#
# The grader links tests/tests.cpp against whichever implementation is under test.

CXX      := clang++
CXXFLAGS := -std=c++17 -Wall -Wextra

.PHONY: all build run test solution test-solution clean

all: build

# ── build — compile-check the learner's implementation (warning-clean) ────────
build:
	$(CXX) $(CXXFLAGS) -c starter/tracked_buffer.cpp -o starter/tracked_buffer.o
	@echo "OK \xE2\x9C\x85  starter/tracked_buffer.cpp compiles. Now run: make test"

# ── run — build and run a minimal smoke test on the starter ───────────────────
run: build
	$(CXX) $(CXXFLAGS) tests/tests.cpp starter/tracked_buffer.cpp -o starter/run
	@./starter/run; true

# ── test — grade the LEARNER's code (RED until tasks are filled in) ───────────
test:
	$(CXX) $(CXXFLAGS) tests/tests.cpp starter/tracked_buffer.cpp -o tests/run
	@./tests/run || echo "FAIL \xE2\x9D\x8C  fill in the TASK blocks in starter/tracked_buffer.cpp until every check passes."

# ── solution — build + run the reference solution ─────────────────────────────
solution:
	$(CXX) $(CXXFLAGS) tests/tests.cpp solution/tracked_buffer.cpp -o solution/run
	@./solution/run

# ── test-solution — proof the lab is solvable: MUST be green ─────────────────
test-solution:
	$(CXX) $(CXXFLAGS) tests/tests.cpp solution/tracked_buffer.cpp -o tests/run
	@./tests/run

# ── clean — remove all build artifacts ────────────────────────────────────────
clean:
	rm -f starter/tracked_buffer.o starter/run solution/run tests/run
	rm -rf starter/run.dSYM solution/run.dSYM tests/run.dSYM
