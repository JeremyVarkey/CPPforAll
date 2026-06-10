// Chapter 17 — Fixed-Size Arrays · Project: Tic-Tac-Toe Referee   (GRADER)
// ─────────────────────────────────────────────────────────────────────────────
// A tiny no-framework unit-test harness (same style as drills/CLAUDE.md spec).
// Includes ../tictactoe.h and calls every function across MANY inputs — fully
// deterministic because none of the graded functions do I/O.  Each failing CHECK
// prints its expression and line number.  Any failure → non-zero exit → `make test`
// is RED.
//
// The Makefile links this file against starter/tictactoe.cpp (your code) for
// `make test`, and against solution/tictactoe.cpp for `make test-solution`.
//
// Boards are built with nested aggregate init:  Board { { row0 }, { row1 }, ... }
// The outer braces are for the Board (std::array<...,3>); the inner braces for
// each std::array<char,3> row.  See notes 17.4 and 17.13 for why.

#include <iostream>
#include "../tictactoe.h"

static int fails { 0 };

// CHECK: assert a boolean condition; on failure, report what and where.
#define CHECK(cond) \
    do { if(!(cond)){ std::cerr << "FAIL: " #cond "  @line " << __LINE__ << "\n"; ++fails; } } while(0)

// ─────────────────────────────────────────────────────────────────────────────
// Helper boards — hand-built for exact test cases
// ─────────────────────────────────────────────────────────────────────────────

// Empty board — all spaces
static const Board kEmpty {{
    {{ ' ', ' ', ' ' }},
    {{ ' ', ' ', ' ' }},
    {{ ' ', ' ', ' ' }},
}};

// Full board, no winner (a draw).  X goes first so there are 5 X and 4 O.
//   X O X
//   X X O
//   O X O
// No row, column, or diagonal is complete — verified exhaustively.
static const Board kDraw {{
    {{ 'X', 'O', 'X' }},
    {{ 'X', 'X', 'O' }},
    {{ 'O', 'X', 'O' }},
}};

// X wins row 0 (top row: X X X).  '.' marks an empty cell below.
//   X X X
//   O O .
//   . . .
static const Board kXRow0 {{
    {{ 'X', 'X', 'X' }},
    {{ 'O', 'O', ' ' }},
    {{ ' ', ' ', ' ' }},
}};

// X wins row 2 (bottom row).  '.' marks an empty cell below.
//   O O .
//   . O .
//   X X X
static const Board kXRow2 {{
    {{ 'O', 'O', ' ' }},
    {{ ' ', 'O', ' ' }},
    {{ 'X', 'X', 'X' }},
}};

// O wins column 1 (middle column).  '.' marks an empty cell below.
//   X O X
//   X O .
//   . O .
static const Board kOCol1 {{
    {{ 'X', 'O', 'X' }},
    {{ 'X', 'O', ' ' }},
    {{ ' ', 'O', ' ' }},
}};

// O wins column 2 (right column).  '.' marks an empty cell below.
//   X X O
//   X . O
//   . X O
static const Board kOCol2 {{
    {{ 'X', 'X', 'O' }},
    {{ 'X', ' ', 'O' }},
    {{ ' ', 'X', 'O' }},
}};

// X wins main diagonal (0,0)→(1,1)→(2,2).  '.' marks an empty cell below.
//   X O .
//   O X .
//   . . X
static const Board kXMainDiag {{
    {{ 'X', 'O', ' ' }},
    {{ 'O', 'X', ' ' }},
    {{ ' ', ' ', 'X' }},
}};

// O wins anti-diagonal (0,2)→(1,1)→(2,0).  '.' marks an empty cell below.
//   X X O
//   . O .
//   O . X
static const Board kOAntiDiag {{
    {{ 'X', 'X', 'O' }},
    {{ ' ', 'O', ' ' }},
    {{ 'O', ' ', 'X' }},
}};

// In-progress — no winner, not full.  '.' marks an empty cell below.
//   X O .
//   . X .
//   O . .
static const Board kInProgress {{
    {{ 'X', 'O', ' ' }},
    {{ ' ', 'X', ' ' }},
    {{ 'O', ' ', ' ' }},
}};

int main()
{
    // ── Task 1: countMark ────────────────────────────────────────────────────
    // Empty board: all 9 cells are ' ', none are X or O.
    CHECK(countMark(kEmpty, ' ') == 9);
    CHECK(countMark(kEmpty, 'X') == 0);
    CHECK(countMark(kEmpty, 'O') == 0);

    // Draw board: 5 X (X goes first), 4 O, 0 empty — board is completely full.
    CHECK(countMark(kDraw, 'X') == 5);
    CHECK(countMark(kDraw, 'O') == 4);
    CHECK(countMark(kDraw, ' ') == 0);

    // In-progress board: 2 X, 2 O, 5 empty.
    CHECK(countMark(kInProgress, 'X') == 2);
    CHECK(countMark(kInProgress, 'O') == 2);
    CHECK(countMark(kInProgress, ' ') == 5);

    // Edge: counts should sum to 9 for any valid board.
    CHECK(countMark(kXRow0, 'X') + countMark(kXRow0, 'O') + countMark(kXRow0, ' ') == 9);

    // ── Task 2: isFull ───────────────────────────────────────────────────────
    CHECK(!isFull(kEmpty));           // empty board is not full
    CHECK( isFull(kDraw));            // draw board is completely full
    CHECK(!isFull(kXRow0));           // has empty cells
    CHECK(!isFull(kInProgress));      // has empty cells

    // ── Task 3: rowWinner / colWinner ────────────────────────────────────────
    // Row winners
    CHECK(rowWinner(kXRow0)       == 'X');  // top row is all X
    CHECK(rowWinner(kXRow2)       == 'X');  // bottom row is all X
    CHECK(rowWinner(kOCol1)       == ' ');  // no row winner on this board
    CHECK(rowWinner(kEmpty)       == ' ');  // empty rows never win
    CHECK(rowWinner(kDraw)        == ' ');  // no complete row in the draw board
    CHECK(rowWinner(kInProgress)  == ' ');  // in-progress has no row winner

    // Column winners
    CHECK(colWinner(kOCol1)       == 'O');  // middle column is all O
    CHECK(colWinner(kOCol2)       == 'O');  // right column is all O
    CHECK(colWinner(kXRow0)       == ' ');  // no column winner here
    CHECK(colWinner(kEmpty)       == ' ');  // empty columns never win
    CHECK(colWinner(kDraw)        == ' ');  // no complete column in draw board

    // ── Task 4: diagWinner ───────────────────────────────────────────────────
    CHECK(diagWinner(kXMainDiag)  == 'X');  // main diagonal X wins
    CHECK(diagWinner(kOAntiDiag)  == 'O');  // anti-diagonal O wins
    CHECK(diagWinner(kXRow0)      == ' ');  // row win, not diagonal
    CHECK(diagWinner(kOCol1)      == ' ');  // column win, not diagonal
    CHECK(diagWinner(kEmpty)      == ' ');  // empty diagonals don't win
    CHECK(diagWinner(kDraw)       == ' ');  // draw board has no diagonal winner

    // ── Task 5: winner / isDraw ──────────────────────────────────────────────
    // Each win type detected correctly by winner()
    CHECK(winner(kXRow0)      == 'X');  // row win
    CHECK(winner(kXRow2)      == 'X');  // row win (different row)
    CHECK(winner(kOCol1)      == 'O');  // column win
    CHECK(winner(kOCol2)      == 'O');  // column win (different column)
    CHECK(winner(kXMainDiag)  == 'X');  // main-diagonal win
    CHECK(winner(kOAntiDiag)  == 'O');  // anti-diagonal win
    CHECK(winner(kDraw)       == ' ');  // no winner on draw board
    CHECK(winner(kEmpty)      == ' ');  // empty board has no winner
    CHECK(winner(kInProgress) == ' ');  // in-progress has no winner yet

    // isDraw: only true when board is full AND no one has won.
    CHECK( isDraw(kDraw));            // the classic draw scenario
    CHECK(!isDraw(kEmpty));           // empty: not full, not a draw
    CHECK(!isDraw(kXRow0));           // X wins -> not a draw
    CHECK(!isDraw(kOCol1));           // O wins -> not a draw
    CHECK(!isDraw(kInProgress));      // not full yet -> not a draw

    // ── Task 6: sumScores (C-style array + decay lesson) ─────────────────────
    // Standard cases
    {
        int scores5[] { 10, 20, 30, 40, 50 };
        CHECK(sumScores(scores5, 5) == 150);

        int scores3[] { 1, 2, 3 };
        CHECK(sumScores(scores3, 3) == 6);
    }
    // Edge: zero-element slice of an array (count = 0)
    {
        int arr[] { 99, 100, 101 };
        CHECK(sumScores(arr, 0) == 0);     // sum of zero elements = 0
    }
    // Edge: single element
    {
        int single[] { 42 };
        CHECK(sumScores(single, 1) == 42);
    }
    // Edge: negative values in the array
    {
        int negs[] { -5, 10, -3, 8 };
        CHECK(sumScores(negs, 4) == 10);   // -5 + 10 + -3 + 8 = 10
    }
    // Edge: all zeros
    {
        int zeros[] { 0, 0, 0 };
        CHECK(sumScores(zeros, 3) == 0);
    }

    // ── Task 7: firstNegative (pointer arithmetic + begin/end) ───────────────
    // Basic case: negative in the middle
    {
        int data[] { 3, 7, -2, 5 };
        const int* begin { data };
        const int* end   { data + 4 };
        const int* result { firstNegative(begin, end) };
        CHECK(result != end);          // found something
        CHECK(*result == -2);          // found the right one
        CHECK(result == &data[2]);     // at the right address
    }
    // First element is negative
    {
        int data[] { -1, 2, 3 };
        const int* result { firstNegative(data, data + 3) };
        CHECK(result == data);         // the very first element
        CHECK(*result == -1);
    }
    // Last element is negative
    {
        int data[] { 1, 2, 3, -9 };
        const int* result { firstNegative(data, data + 4) };
        CHECK(result == &data[3]);     // the last element
        CHECK(*result == -9);
    }
    // No negatives: should return end
    {
        int data[] { 1, 2, 3, 4 };
        const int* result { firstNegative(data, data + 4) };
        CHECK(result == data + 4);     // returned the end sentinel
    }
    // Edge: empty range (begin == end) — should return end immediately
    {
        int data[] { -5, -10 };
        const int* result { firstNegative(data, data) };   // empty range
        CHECK(result == data);         // end of empty range is begin
    }
    // Edge: all negative
    {
        int data[] { -3, -1, -7 };
        const int* result { firstNegative(data, data + 3) };
        CHECK(result == data);         // first element, which is the first negative
        CHECK(*result == -3);
    }
    // Edge: only one element, positive
    {
        int data[] { 5 };
        CHECK(firstNegative(data, data + 1) == data + 1);  // not found -> end
    }
    // Edge: only one element, negative
    {
        int data[] { -5 };
        CHECK(firstNegative(data, data + 1) == data);      // found at [0]
    }

    // ── Final report ─────────────────────────────────────────────────────────
    if (!fails)
        std::cout << "PASS ✅  all referee checks passed.\n";
    else
        std::cerr << "\nFAIL ❌  " << fails << " check(s) failed"
                     " \xe2\x80\x94 fix the TASK blocks in tictactoe.cpp.\n";

    return fails ? 1 : 0;
}
