# Chapter 4 — Numeric Types Lab · unit-test grader (Style B).
# Targets follow the drills/CLAUDE.md Makefile contract. Recipe lines use TABS.

CXX      := clang++
CXXFLAGS := -std=c++17 -Wall -Wextra

HEADER   := numeric_lab.h

.PHONY: all build run test solution test-solution clean

all: build

# `make` / `make build` — compile the STARTER implementation (proves it builds).
build: starter/app
starter/app: starter/numeric_lab.cpp $(HEADER) tests/demo.cpp
	$(CXX) $(CXXFLAGS) tests/demo.cpp starter/numeric_lab.cpp -o starter/app

# `make run` — run the starter demo so you can see your functions' behavior.
run: build
	./starter/app

# `make test` — grade the LEARNER's starter code. RED until the TASK blocks are done.
test: $(HEADER) tests/tests.cpp starter/numeric_lab.cpp
	@$(CXX) $(CXXFLAGS) tests/tests.cpp starter/numeric_lab.cpp -o tests/run
	@./tests/run && echo "PASS ✅  starter passes — nice work." || echo "FAIL ❌  fill in the TASK blocks in starter/numeric_lab.cpp until every CHECK passes."

# `make solution` — build + run the reference demo.
solution: solution/app
	./solution/app
solution/app: solution/numeric_lab.cpp $(HEADER) tests/demo.cpp
	$(CXX) $(CXXFLAGS) tests/demo.cpp solution/numeric_lab.cpp -o solution/app

# `make test-solution` — grade the SOLUTION. MUST be green (our proof it's solvable).
test-solution: $(HEADER) tests/tests.cpp solution/numeric_lab.cpp
	@$(CXX) $(CXXFLAGS) tests/tests.cpp solution/numeric_lab.cpp -o tests/run
	@./tests/run && echo "PASS ✅  solution passes all checks." || echo "FAIL ❌  solution does NOT pass — this is a bug in the exercise."

clean:
	rm -f starter/app solution/app tests/run
