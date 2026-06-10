# Chapter 17 — Fixed-Size Arrays · Tic-Tac-Toe Referee · unit-test grader (Style B).
# Targets follow the drills/CLAUDE.md Makefile contract.  TABS, not spaces.
#
# Layout:
#   tictactoe.h         — declarations (provided, complete, do NOT edit)
#   starter/tictactoe.cpp — learner's implementations (7 TASK blocks to fill in)
#   solution/tictactoe.cpp — reference implementation
#   tests/tests.cpp       — grader (includes ../tictactoe.h; links against one .cpp)

CXX      := clang++
CXXFLAGS := -std=c++17 -Wall -Wextra

.PHONY: all build run test solution test-solution clean

all: build

# build — compile-check the learner's starter (warning-clean .o, no link)
# Also builds a trivial demo so `make run` works without a grader.
build:
	$(CXX) $(CXXFLAGS) -c starter/tictactoe.cpp -o starter/tictactoe.o
	@echo "OK \xe2\x9c\x85  starter/tictactoe.cpp compiles.  Now run: make test"

# run — smoke-test: just confirm the starter .o rebuilt cleanly (no separate main).
run: build
	@echo "(No interactive driver for this exercise — run: make test)"

# test — grade the LEARNER's code: link the grader against starter/tictactoe.cpp.
# RED until the TASK blocks are filled in; GREEN once they are.
test:
	$(CXX) $(CXXFLAGS) tests/tests.cpp starter/tictactoe.cpp -o tests/run
	@./tests/run || echo "FAIL \xe2\x9d\x8c  fill in the TASK blocks in starter/tictactoe.cpp until every check passes."

# solution — run the grader against the REFERENCE solution (shows what passing looks like).
solution: tests/tests.cpp solution/tictactoe.cpp
	$(CXX) $(CXXFLAGS) tests/tests.cpp solution/tictactoe.cpp -o solution/run
	./solution/run

# test-solution — proof the lab is solvable: the reference MUST pass every check.
test-solution:
	$(CXX) $(CXXFLAGS) tests/tests.cpp solution/tictactoe.cpp -o tests/run
	@./tests/run

clean:
	rm -f starter/tictactoe.o tests/run solution/run
	rm -rf tests/run.dSYM solution/run.dSYM
