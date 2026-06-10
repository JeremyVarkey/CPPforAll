# Chapter 23 — Object Relationships · Garage Simulation · unit-test grader (Style B).
# Targets follow drills/CLAUDE.md Makefile contract. TABS, not spaces.
#
# The learner implements ../garage.h in starter/garage.cpp (five TASK blocks).
# The grader (tests/tests.cpp) supplies main() and checks trace-log ORDER,
# aggregation isolation, dependency counts, and RouteList initializer_list.
#
# -I. puts the chapter root on the include path so both garage.cpp files can
# write:  #include "../garage.h"   (same convention as chapter-02 and chapter-08).

CXX      := clang++
CXXFLAGS := -std=c++17 -Wall -Wextra -I.

.PHONY: all build run test solution test-solution clean

all: build

# ── build — compile-check the LEARNER's starter (warning-clean) ───────────────
build:
	$(CXX) $(CXXFLAGS) -c starter/garage.cpp -o starter/garage.o
	@echo "OK \xe2\x9c\x85  starter/garage.cpp compiles. Now run: make test"

# ── run — run the starter's compiled object against a minimal driver ──────────
#    (no separate main.cpp needed — the tests binary doubles as the driver)
run: tests/run
	./tests/run

tests/run: tests/tests.cpp garage.h starter/garage.cpp
	$(CXX) $(CXXFLAGS) tests/tests.cpp starter/garage.cpp -o tests/run

# ── test — grade the LEARNER's code (RED until TASK blocks filled in) ─────────
test:
	$(CXX) $(CXXFLAGS) tests/tests.cpp starter/garage.cpp -o tests/run
	@./tests/run || echo "FAIL \xe2\x9d\x8c  fill in the TASK blocks in starter/garage.cpp until every check passes."

# ── solution — run the REFERENCE solution ─────────────────────────────────────
solution: solution/app
	./solution/app
solution/app: tests/tests.cpp garage.h solution/garage.cpp
	$(CXX) $(CXXFLAGS) tests/tests.cpp solution/garage.cpp -o solution/app

# ── test-solution — proof the exercise is solvable (MUST be green) ────────────
test-solution:
	$(CXX) $(CXXFLAGS) tests/tests.cpp solution/garage.cpp -o tests/run
	@./tests/run

# ── clean — remove all build artifacts ───────────────────────────────────────
clean:
	rm -f starter/garage.o solution/app tests/run
	rm -rf starter/garage.o.dSYM solution/app.dSYM tests/run.dSYM
