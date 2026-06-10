// Chapter 17 — Fixed-Size Arrays · Project: Tic-Tac-Toe Referee  (REFERENCE SOLUTION)
// ─────────────────────────────────────────────────────────────────────────────
// One complete, correct, warning-clean implementation of ../tictactoe.h.
// Peek only after you have taken a real swing at starter/tictactoe.cpp — the
// learning is in wiring up the array traversals yourself, then comparing.
//
// Everything here uses only chapter ≤ 17 features.  No <algorithm>, no new[].

#include "../tictactoe.h"

// ─────────────────────────────────────────────────────────────────────────────
// PART A — std::array 2D (tasks 1–5)
// ─────────────────────────────────────────────────────────────────────────────

// ─── TASK 1: count a mark across the whole board ─────────────────────────────
// We walk EVERY cell with nested range-for loops (notes 17.13).  The outer loop
// binds each row by const reference (`const auto& row`) — without the `&`, each
// inner std::array<char,3> would be COPIED on every iteration (notes 17.3).
// `const auto&` matches the inner-array type automatically (ch 10 auto).
int countMark(const Board& board, char ch)
{
    int count { 0 };

    for (const auto& row : board)           // outer: each row is std::array<char,3>
        for (char cell : row)               // inner: each char in that row
            if (cell == ch)
                ++count;

    return count;
}

// ─── TASK 2: check whether the board is full ─────────────────────────────────
// Delegate entirely to countMark — if zero empty cells remain, the board is full.
// This keeps all the traversal logic in one place (countMark) and keeps isFull
// as a one-liner that reads like plain English.
bool isFull(const Board& board)
{
    return countMark(board, ' ') == 0;
}

// ─── TASK 3: scan for a winning row or column ────────────────────────────────
// rowWinner: for each row r, check if all three cells are equal AND non-empty.
//   board[r][0] == board[r][1] == board[r][2] != ' '
//
// We use a plain index loop so we can address board[r][0], board[r][1], board[r][2]
// explicitly.  Range-for works equally well; index loops match notes 17.2's
// "index-based for loop" discussion.
char rowWinner(const Board& board)
{
    for (int r { 0 }; r < 3; ++r)
    {
        char first { board[static_cast<std::size_t>(r)][0] };  // left-most cell in row r

        if (first != ' '
            && first == board[static_cast<std::size_t>(r)][1]
            && first == board[static_cast<std::size_t>(r)][2])
        {
            return first;    // this row belongs to `first`
        }
    }

    return ' ';   // no row is complete
}

// colWinner: same idea, but the "line" runs down column c.
//   board[0][c] == board[1][c] == board[2][c] != ' '
char colWinner(const Board& board)
{
    for (int c { 0 }; c < 3; ++c)
    {
        char first { board[0][static_cast<std::size_t>(c)] };  // top cell of column c

        if (first != ' '
            && first == board[1][static_cast<std::size_t>(c)]
            && first == board[2][static_cast<std::size_t>(c)])
        {
            return first;    // this column belongs to `first`
        }
    }

    return ' ';
}

// ─── TASK 4: check the two diagonals ─────────────────────────────────────────
// Main diagonal: (0,0) → (1,1) → (2,2).  The shared middle cell is board[1][1].
// Anti-diagonal: (0,2) → (1,1) → (2,0).
//
// No loops needed — just compare the three cells for each diagonal.  If the
// shared middle is ' ', neither diagonal can be a winning line, so we can short-
// circuit on that check first (optional optimization — correctness doesn't depend
// on it).
char diagWinner(const Board& board)
{
    // Main diagonal ─────────────────────────────────────────────────────────
    char mid { board[1][1] };   // centre cell is shared by both diagonals

    if (mid != ' ' && board[0][0] == mid && board[2][2] == mid)
        return mid;             // main diagonal won by `mid`

    // Anti-diagonal ─────────────────────────────────────────────────────────
    if (mid != ' ' && board[0][2] == mid && board[2][0] == mid)
        return mid;             // anti-diagonal won by `mid`

    return ' ';
}

// ─── TASK 5: combine the helpers ─────────────────────────────────────────────
// winner: call each helper in sequence; return the first non-' ' result.
// All the interesting logic lives in the helper functions above — winner() is
// just the entry point the grader (and a real game loop) would call.
char winner(const Board& board)
{
    char w { ' ' };

    w = rowWinner(board);
    if (w != ' ') return w;

    w = colWinner(board);
    if (w != ' ') return w;

    w = diagWinner(board);
    if (w != ' ') return w;

    return ' ';   // no winner in any direction
}

// isDraw: no winner AND no empty cells (board is full).
// We never call isFull first and winner second — that would traverse the board
// twice unnecessarily.  winner() is cheaper to call first because it can return
// early on the first winning line found.
bool isDraw(const Board& board)
{
    return winner(board) == ' ' && isFull(board);
}

// ─────────────────────────────────────────────────────────────────────────────
// PART B — C-style arrays + decay + pointer arithmetic (tasks 6–7)
// ─────────────────────────────────────────────────────────────────────────────

// ─── TASK 6: sum a C-style array (and observe decay) ─────────────────────────
// KEY TERM: ARRAY DECAY (notes 17.8).
//   `const int scores[]` in a parameter list is EXACTLY the same as
//   `const int* scores` — the compiler rewrites it.  Therefore sizeof(scores)
//   inside this function is sizeof(const int*) ≈ 8, NOT sizeof(the-caller's-array).
//   That is the decay lesson the README explains in full.
//
// We sum with a plain index loop.  The caller passes `count` because the pointer
// alone carries NO length information after decay (notes 17.8 "decay loses length
// information").
int sumScores(const int scores[], int count)
{
    int total { 0 };

    for (int i { 0 }; i < count; ++i)
        total += scores[i];

    return total;
}

// ─── TASK 7: walk a half-open range with pointer arithmetic ──────────────────
// KEY TERM: POINTER ARITHMETIC + half-open range [begin, end) (notes 17.9).
//   `++p` advances p by ONE ELEMENT (not one byte — the compiler scales the
//   addition by sizeof(int) automatically).  We stop when p == end (the one-past-
//   last sentinel), never dereferencing end.  This pattern is EXACTLY how
//   standard-library iterators (ch 18) work under the hood; getting comfortable
//   with it here pays off immediately next chapter.
//
//   CS6340 tie-in: LLVM's instruction iterators and BasicBlock::begin()/end()
//   follow exactly this half-open range protocol.  Knowing it at the pointer level
//   makes the higher-level API feel familiar, not magic.
const int* firstNegative(const int* begin, const int* end)
{
    for (const int* p { begin }; p != end; ++p)    // ++p: move one int forward
    {
        if (*p < 0)     // dereference p to inspect the element
            return p;   // first negative found — return its address
    }

    return end;   // sentinel: no negative in [begin, end)
}
