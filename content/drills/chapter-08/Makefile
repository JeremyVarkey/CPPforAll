# Chapter 8 — Control Flow · The Number-Guessing Engine · unit-test grader (Style B).
# Targets follow the drills/CLAUDE.md Makefile contract. TABS, not spaces.
#
# The learner implements ../engine.h in starter/engine.cpp (the PURE, graded
# logic). The grader (tests/tests.cpp) supplies main() and calls every engine
# function across many deterministic inputs — so the learner never writes a test
# loop. A second main, starter/main.cpp, is the PLAYABLE driver that wraps the
# engine with <random> + std::cin/cout; it is NOT graded (`make run` plays it).

CXX      := clang++
CXXFLAGS := -std=c++17 -Wall -Wextra

.PHONY: all build run test solution test-solution clean

all: build

# build — compile-check the learner's engine (warning-clean object file, no link)
# AND build the playable driver. `make test` does the full grader build+run.
build:
	$(CXX) $(CXXFLAGS) -c starter/engine.cpp -o starter/engine.o
	$(CXX) $(CXXFLAGS) starter/main.cpp starter/engine.cpp -o starter/play
	@echo "OK ✅  starter/engine.cpp compiles. Now run: make test   (or: make run to play)"

# run — play the game against YOUR engine (the driver supplies main()).
run: build
	./starter/play

# test — grade the LEARNER's code: link the grader against starter/engine.cpp.
# RED until the TASK blocks are filled in; GREEN once they're correct.
test:
	$(CXX) $(CXXFLAGS) tests/tests.cpp starter/engine.cpp -o tests/run
	@./tests/run

# solution — play the game against the REFERENCE engine.
solution: solution/play
	./solution/play
solution/play: solution/main.cpp solution/engine.cpp
	$(CXX) $(CXXFLAGS) solution/main.cpp solution/engine.cpp -o solution/play

# test-solution — proof the lab is solvable: the reference MUST pass every check.
test-solution:
	$(CXX) $(CXXFLAGS) tests/tests.cpp solution/engine.cpp -o tests/run
	@./tests/run

clean:
	rm -f starter/engine.o starter/play solution/play tests/run
	rm -rf tests/run.dSYM starter/play.dSYM solution/play.dSYM
