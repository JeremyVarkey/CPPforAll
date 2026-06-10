# Chapter 1 — Cash Register · output-comparison grader.
# Targets follow the drills/CLAUDE.md Makefile contract.

CXX      := clang++
CXXFLAGS := -std=c++17 -Wall -Wextra

.PHONY: all build run test solution test-solution clean

all: build

build: starter/app
starter/app: starter/main.cpp
	$(CXX) $(CXXFLAGS) starter/main.cpp -o starter/app

run: build
	./starter/app

# Your grader: feed tests/input.txt on stdin, compare output to tests/expected.txt.
test: build
	@./starter/app < tests/input.txt > tests/.out.txt
	@diff tests/expected.txt tests/.out.txt && echo "PASS ✅  output matches expected." || echo "FAIL ❌  fill in the TODO blocks until the diff above is empty."

solution: solution/app
	./solution/app < tests/input.txt
solution/app: solution/main.cpp
	$(CXX) $(CXXFLAGS) solution/main.cpp -o solution/app

# Proof the exercise is solvable: the reference solution must match expected output.
test-solution: solution/app
	@./solution/app < tests/input.txt > tests/.sol.txt
	@diff tests/expected.txt tests/.sol.txt && echo "PASS ✅  solution matches expected." || echo "FAIL ❌  solution does NOT match expected."

clean:
	rm -f starter/app solution/app tests/.out.txt tests/.sol.txt
