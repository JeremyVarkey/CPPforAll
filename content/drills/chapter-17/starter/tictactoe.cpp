// Chapter 17 — Fixed-Size Arrays · Project: Tic-Tac-Toe Referee   (STARTER)
// ─────────────────────────────────────────────────────────────────────────────
// Fill in the seven TASK blocks below.  Each maps 1:1 to a task in the README
// and to a declaration in ../tictactoe.h.  The bodies currently return WRONG
// placeholders so the file compiles immediately — that is why `make test` is
// RED right now.  Your job is to turn it GREEN by implementing the real logic.
//
//     make build         compile your code      (should already work)
//     make test          grade it               (RED until you fill these in)
//     make solution      run the grader against the reference if you get stuck
//
// ── Scope reminder ────────────────────────────────────────────────────────────
// Only chapter ≤ 17 features are needed:
//   std::array, C-style arrays, range-for loops (ch 16), index loops (ch 8),
//   const references (ch 12), function templates are NOT required here —
//   the Board typedef hides the type complexity.
//   FORBIDDEN: <algorithm>, new[], any C++20 feature.

#include "../tictactoe.h"

// ─────────────────────────────────────────────────────────────────────────────
// PART A — std::array 2D (tasks 1–5)
// ─────────────────────────────────────────────────────────────────────────────

// ─── TASK 1: count a mark across the whole board ─────────────────────────────
// countMark — walk every cell of `board` and count how many equal `ch`.
//
// KEY TERM: std::array<std::array<char,3>,3> is a 2D "array of arrays" (notes
// 17.13). board[r] is the r-th row (an std::array<char,3>); board[r][c] is one
// cell. Walk rows with an outer range-for, then columns with an inner range-for.
//
// Constraint: pass board by const reference — never copy a 2D array (notes 17.3).
// The signature already specifies `const Board&`; keep it.
//
//   >>> YOUR CODE HERE <<<
//
int countMark(const Board& board, char ch)
{
    (void)board; (void)ch;
    return 0;   // placeholder — always reports 0 marks (wrong for non-empty boards)
}
// ─────────────────────────────────────────────────────────────────────────────

// ─── TASK 2: check whether the board is full ─────────────────────────────────
// isFull — return true when no ' ' (empty) cell remains.
//
// Hint: delegate to countMark. One call, one comparison — no loops needed here.
//
//   >>> YOUR CODE HERE <<<
//
bool isFull(const Board& board)
{
    (void)board;
    return false;   // placeholder — always reports not full (wrong for full boards)
}
// ─────────────────────────────────────────────────────────────────────────────

// ─── TASK 3: scan for a winning row or column ────────────────────────────────
// rowWinner / colWinner — detect a complete row or column.
//
// KEY TERM: a *row* win means board[r][0] == board[r][1] == board[r][2] AND that
// character is not ' ' (an all-empty row does NOT count as a win). Similarly for
// columns: board[0][c] == board[1][c] == board[2][c].
//
// Use an outer loop over r (or c) and compare the three cells directly.
// Return the winning character, or ' ' if no row (or column) is complete.
//
//   >>> YOUR CODE HERE <<<
//
char rowWinner(const Board& board)
{
    (void)board;
    return ' ';   // placeholder — reports no row winner (wrong when a row is won)
}

//   >>> YOUR CODE HERE <<<
//
char colWinner(const Board& board)
{
    (void)board;
    return ' ';   // placeholder — reports no column winner
}
// ─────────────────────────────────────────────────────────────────────────────

// ─── TASK 4: check the two diagonals ─────────────────────────────────────────
// diagWinner — return the winning character if the main diagonal or the
// anti-diagonal is complete, or ' ' if neither is.
//
// KEY TERM: main diagonal — (0,0),(1,1),(2,2). Anti-diagonal — (0,2),(1,1),(2,0).
// No loops needed; just compare three cells each time.
//
//   >>> YOUR CODE HERE <<<
//
char diagWinner(const Board& board)
{
    (void)board;
    return ' ';   // placeholder — always reports no diagonal winner
}
// ─────────────────────────────────────────────────────────────────────────────

// ─── TASK 5: combine the helpers ─────────────────────────────────────────────
// winner — ask each helper in turn; return the first non-' ' result, else ' '.
// isDraw  — no winner AND board is full.
//
// Keep the logic in the helpers — winner() and isDraw() are just orchestrators.
//
//   >>> YOUR CODE HERE <<<
//
char winner(const Board& board)
{
    (void)board;
    return ' ';   // placeholder — always reports no winner
}

bool isDraw(const Board& board)
{
    (void)board;
    return false;   // placeholder — always reports not a draw
}
// ─────────────────────────────────────────────────────────────────────────────

// ─────────────────────────────────────────────────────────────────────────────
// PART B — C-style arrays + decay + pointer arithmetic (tasks 6–7)
// ─────────────────────────────────────────────────────────────────────────────

// ─── TASK 6: sum a C-style array (and observe decay) ─────────────────────────
// sumScores — sum scores[0..count-1].
//
// KEY TERM: ARRAY DECAY (notes 17.8).  The parameter `const int scores[]` is
// SYNTACTIC SUGAR for `const int* scores` — the compiler converts it for you.
// Inside this function body, `scores` is a POINTER; sizeof(scores) yields the
// pointer size (8 bytes on most 64-bit platforms), NOT the array size.  The
// README shows this in the "decay lesson" section.
//
// Use a plain index loop `for (int i = 0; i < count; ++i)`.
// Do NOT call sizeof(scores) — just use count.
//
//   >>> YOUR CODE HERE <<<
//
int sumScores(const int scores[], int count)
{
    (void)scores; (void)count;
    return 0;   // placeholder — always returns 0 (wrong for non-empty arrays)
}
// ─────────────────────────────────────────────────────────────────────────────

// ─── TASK 7: walk a half-open range with pointer arithmetic ──────────────────
// firstNegative — return a pointer to the first negative element in [begin,end),
// or `end` if none.
//
// KEY TERM: POINTER ARITHMETIC (notes 17.9).  The half-open range [begin,end)
// convention: `begin` points at the first element, `end` points ONE PAST the last.
// Walk with `for (const int* p { begin }; p != end; ++p)` and test `*p < 0`.
//
// Return the pointer where the first negative lives, or `end` if the range has
// no negative element.  The caller checks `result != end` to know if one was found.
//
// Constraint: use `++p` (not p[i] or p+i) to advance — the 17.9 lesson.
//
//   >>> YOUR CODE HERE <<<
//
const int* firstNegative(const int* begin, const int* end)
{
    (void)begin; (void)end;
    return end;   // placeholder — pretends no negative was found (wrong when one exists)
}
// ─────────────────────────────────────────────────────────────────────────────
