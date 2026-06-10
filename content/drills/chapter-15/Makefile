# Chapter 15 — More on Classes · IdCard Badge Printer · unit-test grader (Style B).
# Targets follow the drills/CLAUDE.md Makefile contract. TABS, not spaces.
#
# Layout:
#   idcard.h              — the class declaration + nested types (DO NOT EDIT)
#   starter/idcard.cpp    — learner fills in the 5 TASK blocks
#   solution/idcard.cpp   — reference implementation
#   tests/tests.cpp       — the grader (includes ../idcard.h; links the .cpp)
#
# -I. puts the chapter root on the include path so every file can write
# #include "idcard.h" (or "../idcard.h" from subdirs — both work).

CXX      := clang++
CXXFLAGS := -std=c++17 -Wall -Wextra

.PHONY: all build run test solution test-solution clean

all: build

# build — compile-check the LEARNER's starter code (warning-clean object, then
# build a tiny demo so the learner can run their code without waiting for tests).
build:
	$(CXX) $(CXXFLAGS) -c starter/idcard.cpp -o starter/idcard.o
	@echo "OK ✅  starter/idcard.cpp compiles. Now run: make test"

# run — not a graded target; build passes so there is nothing interesting to run
# without main(). Use `make test` to exercise the code.
run: build
	@echo "(No standalone driver for this exercise — try: make test)"

# test — grade the LEARNER's code: link the grader against starter/idcard.cpp.
# RED until the TASK blocks are filled in; GREEN once they're correct.
test:
	$(CXX) $(CXXFLAGS) tests/tests.cpp starter/idcard.cpp -o tests/run
	@./tests/run

# solution — build and run a quick demo using the REFERENCE implementation.
# (Since there is no main.cpp driver, we run the test suite against the solution.)
solution:
	$(CXX) $(CXXFLAGS) tests/tests.cpp solution/idcard.cpp -o solution/run
	@./solution/run

# test-solution — proof the lab is solvable: the reference MUST pass every check.
test-solution:
	$(CXX) $(CXXFLAGS) tests/tests.cpp solution/idcard.cpp -o tests/run
	@./tests/run

clean:
	rm -f starter/idcard.o tests/run solution/run
	rm -rf tests/run.dSYM solution/run.dSYM
