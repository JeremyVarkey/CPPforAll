# Chapter 9 — Error Handling · The Robust Input Desk · unit-test grader (Style B).
# Targets follow the drills/CLAUDE.md Makefile contract. TABS, not spaces.
#
# The learner implements ../desk.h in starter/desk.cpp. There is no main() in
# desk.cpp on purpose — the graders supply main():
#   • tests/tests.cpp      unit-tests the PURE functions (Tasks 1–5) on good and
#                          malicious inputs.
#   • tests/cin_driver.cpp drives the std::cin recovery routine (Task 6) by
#                          FEEDING it tests/cin_input.txt (letters, out-of-range,
#                          blank line, trailing junk) and checking it recovers.
# `make test` runs BOTH and is GREEN only when every part passes.

CXX      := clang++
CXXFLAGS := -std=c++17 -Wall -Wextra

.PHONY: all build run test test-pure cin-test solution test-solution test-solution-pure cin-test-solution clean

all: build

# build — compile-check the learner's code (warning-clean object file, no link).
# There is no program to link yet because the tests/ files provide main(); the
# test targets do the full build+run. This just proves your desk.cpp compiles.
build:
	$(CXX) $(CXXFLAGS) -c starter/desk.cpp -o starter/desk.o
	@echo "OK \xE2\x9C\x85  starter/desk.cpp compiles. Now run: make test"

# run — for this lab, "running" the code IS running the graders against it.
run: test

# ── LEARNER'S GRADE (red -> green) ───────────────────────────────────────────
# test — run BOTH graders against starter/desk.cpp. RED until the TASK blocks are
# filled in; GREEN once they're all correct.
test: test-pure cin-test
	@echo "PASS \xE2\x9C\x85  all parts passed — the input desk is robust."

# Part 1: unit-test the pure validators/parsers (Tasks 1–5).
test-pure:
	$(CXX) $(CXXFLAGS) tests/tests.cpp starter/desk.cpp -o tests/run
	@./tests/run

# Part 2: feed hostile input to the std::cin recovery routine (Task 6) and verify
# it accepts exactly 7 then 50. `grep ACCEPTED` ignores the retry/error messages
# printed in between; we compare only the accepted values.
cin-test:
	$(CXX) $(CXXFLAGS) tests/cin_driver.cpp starter/desk.cpp -o tests/cin_run
	@./tests/cin_run < tests/cin_input.txt > tests/.cin_out.txt 2>/dev/null; \
	grep '^ACCEPTED=' tests/.cin_out.txt > tests/.cin_got.txt || true; \
	printf 'ACCEPTED=7\nACCEPTED=50\n' > tests/.cin_exp.txt; \
	if diff -q tests/.cin_exp.txt tests/.cin_got.txt >/dev/null 2>&1; then \
		echo "PASS \xE2\x9C\x85  readIntInRange recovered from hostile input (got 7 then 50)."; \
	else \
		echo "FAIL \xE2\x9D\x8C  readIntInRange did not recover correctly."; \
		echo "  expected accepted values:"; sed 's/^/    /' tests/.cin_exp.txt; \
		echo "  got:";                      sed 's/^/    /' tests/.cin_got.txt; \
		exit 1; \
	fi

# ── SOLVABILITY PROOF (must be green) ────────────────────────────────────────
# solution — build+run the graders against the reference implementation.
solution: test-solution

# test-solution — proof the lab is solvable: the reference MUST pass every part.
test-solution: test-solution-pure cin-test-solution
	@echo "PASS \xE2\x9C\x85  solution passes all parts."

test-solution-pure:
	$(CXX) $(CXXFLAGS) tests/tests.cpp solution/desk.cpp -o tests/run
	@./tests/run

cin-test-solution:
	$(CXX) $(CXXFLAGS) tests/cin_driver.cpp solution/desk.cpp -o tests/cin_run
	@./tests/cin_run < tests/cin_input.txt > tests/.cin_out.txt 2>/dev/null; \
	grep '^ACCEPTED=' tests/.cin_out.txt > tests/.cin_got.txt || true; \
	printf 'ACCEPTED=7\nACCEPTED=50\n' > tests/.cin_exp.txt; \
	if diff -q tests/.cin_exp.txt tests/.cin_got.txt >/dev/null 2>&1; then \
		echo "PASS \xE2\x9C\x85  solution's readIntInRange recovered from hostile input."; \
	else \
		echo "FAIL \xE2\x9D\x8C  solution's readIntInRange did not recover correctly."; \
		exit 1; \
	fi

clean:
	rm -f starter/desk.o tests/run tests/cin_run
	rm -f tests/.cin_out.txt tests/.cin_got.txt tests/.cin_exp.txt
	rm -rf tests/run.dSYM tests/cin_run.dSYM
