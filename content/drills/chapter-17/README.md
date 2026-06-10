# Chapter 17 — Fixed-Size Arrays: Tic-Tac-Toe Referee

> Reinforces [`../../notes/chapter-17.md`](../../notes/chapter-17.md) · LearnCpp [Chapter 17](https://www.learncpp.com/)

## The project

You will build a **stateless Tic-Tac-Toe referee** — a small library of pure
functions that inspect a game board and answer questions like "who won?" and "is
this a draw?" — plus two utility functions that demonstrate the two most
important C-style-array lessons.

The board is represented as a **2D `std::array`**:

```cpp
using Board = std::array<std::array<char, 3>, 3>;
```

Each cell holds `'X'`, `'O'`, or `' '` (empty). All board-inspecting functions
receive the board by **const reference** (notes 17.3) — a cornerstone rule for
passing fixed-size arrays without unnecessary copies.

The exercise is split into two independent parts:

**Part A — `std::array` 2D (tasks 1–5).** You write helper functions that detect
wins in rows, columns, and diagonals, then combine them into `winner()` and
`isDraw()`. The test suite feeds hand-built boards for every win type — row, column,
main diagonal, anti-diagonal — plus the draw and in-progress cases, so there is no
place for a lucky coincidence to pass the grader.

**Part B — C-style arrays + decay + pointer arithmetic (tasks 6–7).** Two focused
utility functions. `sumScores` demonstrates **array decay** (notes 17.8): the
`const int scores[]` parameter silently becomes `const int*`, so `sizeof(scores)`
inside the function yields the *pointer* size, not the array size. `firstNegative`
demonstrates **pointer arithmetic** with a **half-open range** `[begin, end)` (notes
17.9) — the same conceptual model that powers `std::vector::begin()/end()` and every
standard-library algorithm you will meet in Chapter 18.

Why Tic-Tac-Toe? Because `std::array<std::array<char,3>,3>` is small enough to be
readable yet large enough to practice every 2D traversal pattern — and the game
logic is simple enough that you can verify the output mentally. The real lesson is
in the *shapes*: how to pass a 2D array by const ref, how to index it, and what
happens when you cross the boundary from `std::array` into C-style territory.

---

## Concepts practiced

- **`std::array<T,N>` basics** — construction, indexing, `size()` (notes 17.1, 17.2)
- **Passing `std::array` by const reference** — the right default for read-only array parameters (notes 17.3)
- **Nested `std::array`** — `std::array<std::array<char,3>,3>` as a 2D board; row/column indexing (notes 17.13)
- **Aggregate initialization with nested braces** — `Board {{ {row}, {row}, {row} }}` (notes 17.4)
- **`using` type aliases** — `Board` makes the 2D type readable (ch 10)
- **C-style array decay** — `const int scores[]` parameter is really `const int*`; sizeof gives pointer size (notes 17.8)
- **Pointer arithmetic** — `++p`, `*p`, half-open `[begin, end)` range convention (notes 17.9)
- **Nested range-for loops** for 2D traversal (reused from ch 16 range-for)
- **Index-based `for` loops** for row/column wins (reused from ch 8)
- **Const references** — `const Board&`, `const int*` (reused from ch 12)
- **Type aliases** — `using Board = ...` (reused from ch 10)
- CS6340 bridge: LLVM's `BasicBlock::begin()/end()` follows the same half-open pointer/iterator convention as `firstNegative`.

---

## Your tasks

The starter compiles immediately but every function returns a **wrong placeholder**,
so `make test` starts **RED**. Fill in the seven `>>> YOUR CODE HERE <<<` blocks in
[`starter/tictactoe.cpp`](starter/tictactoe.cpp).

### Part A — `std::array` 2D (tasks 1–5)

1. **Count a mark (`countMark`).** Walk every cell of `board` with nested range-for
   loops and count how many cells equal `ch`. Constraints: pass `board` by
   `const Board&`; use `const auto& row` for the outer loop variable so you do not
   copy each inner array.

2. **Check if full (`isFull`).** Return `true` if no `' '` cell remains. Delegate
   to `countMark` — one call, one comparison. No loops needed here.

3. **Detect a winning row or column (`rowWinner`, `colWinner`).** For each row `r`,
   check whether `board[r][0] == board[r][1] == board[r][2]` and that the cell is
   not `' '`. Similarly for each column `c`. Return the winning character or `' '`.
   Use index-based `for` loops (r = 0..2 or c = 0..2).

4. **Detect a winning diagonal (`diagWinner`).** Check the main diagonal
   `(0,0)→(1,1)→(2,2)` and the anti-diagonal `(0,2)→(1,1)→(2,0)`. Return the
   winner or `' '`. No loops needed — just three-cell comparisons.

5. **Combine the helpers (`winner`, `isDraw`).** `winner()` calls `rowWinner`,
   `colWinner`, and `diagWinner` in sequence, returning the first non-`' '` result.
   `isDraw()` returns `true` when `winner()` is `' '` **and** `isFull()` is `true`.

### Part B — C-style arrays (tasks 6–7)

6. **Sum a C-style array (`sumScores`).** Sum `scores[0..count-1]` with a plain
   index loop. The key lesson is in the **decay**: `const int scores[]` in the
   parameter list is identical to `const int* scores` after the compiler processes
   it — the length is completely lost. That is why the caller passes `count`
   separately. Do not use `sizeof(scores)` inside the body.

7. **Walk a half-open range (`firstNegative`).** Walk `[begin, end)` with
   `for (const int* p { begin }; p != end; ++p)` and return the first pointer
   where `*p < 0`, or `end` if none. Constraint: advance with `++p`, not `p[i]`
   or `p + i` — the goal is to *feel* pointer arithmetic directly.

---

## The decay lesson (why `sizeof` lies inside `sumScores`)

Consider this in the **caller**:

```cpp
int data[] { 10, 20, 30 };                    // type is int[3]
// sizeof(data) == 3 * sizeof(int) == 12       ← the caller can see this
sumScores(data, 3);
```

And this inside `sumScores`:

```cpp
int sumScores(const int scores[], int count)
{
    // scores is now a POINTER (the array decayed at the call site)
    // sizeof(scores) == sizeof(const int*) == 8  on 64-bit
    // That is the POINTER size, not the array size!
}
```

The `[]` in the parameter is cosmetic — the compiler silently rewrites it to `*`.
This is **array decay** (notes 17.8): the array object degrades to a pointer to its
first element, and length information is permanently lost. That is why every C-style
array function also takes a separate `count` parameter.

`std::array` (Part A) does **not** have this problem — passing `const Board&` keeps
the full type (`std::array<std::array<char,3>,3>`) intact across the function
boundary.

---

## Constraints

- **Allowed:** `std::array`, C-style arrays, range-for loops, index-based for loops,
  `const` references, type aliases (`using`), `if`/`else`, `return`, and all
  features from chapters ≤ 17.
- **Forbidden:** `<algorithm>` or any `std::` algorithm/iterator object (Chapter 18
  — not taught yet), `new[]` / `delete[]` (Chapter 19), lambdas (Chapter 20).
- **Required idioms:**
  - All board parameters **must** be passed by `const Board&` (never by value).
  - `firstNegative` **must** advance with `++p`, not index arithmetic.
  - `sumScores` must use the `count` parameter (not sizeof) for the loop bound.

---

## Build & run

```sh
make            # compile-check starter/tictactoe.cpp (warning-clean)
make test       # grade your code -> RED until the TASK blocks are filled in
make solution   # run the grader against the reference (to see what passing looks like)
make clean      # remove build artifacts
```

`make test` is the grader — it supplies its own `main` and calls every function
across many fixed inputs.

---

## Success criteria

`make test` prints **PASS ✅ all referee checks passed.** and exits 0. Until then
it prints one `FAIL: …  @line N` per broken check and ends with
`FAIL ❌  N check(s) failed`. The grader exercises every function across many
inputs, including the edge cases that separate correct logic from a near-miss:

- `countMark(kEmpty, 'X') == 0` — empty board has no X marks
- `countMark(kDraw, 'X') + countMark(kDraw, 'O') + countMark(kDraw, ' ') == 9` — counts must cover every cell
- `isFull(kDraw) == true` — all 9 cells occupied
- `rowWinner(kEmpty) == ' '` — all-empty rows do not count as wins
- `rowWinner(kXRow2) == 'X'` — detects the bottom row, not just the top
- `diagWinner(kXMainDiag) == 'X'` and `diagWinner(kOAntiDiag) == 'O'` — both diagonals detected
- `winner(kDraw) == ' '` — a draw board has no winner
- `isDraw(kXRow0) == false` — a won board is not a draw even when full
- `sumScores(negs, 4) == 10` — negative values in the array must be summed correctly
- `firstNegative(data, data) == data` — empty range must return end immediately (no crash)
- `firstNegative(allPositive, end) == end` — "not found" must return the end sentinel

Turning that wall of red into a single green line is the whole exercise.

---

## Hints

<details><summary>Task 1 — nested range-for and const auto&</summary>

```cpp
int count { 0 };
for (const auto& row : board)   // const auto& avoids copying the inner array
    for (char cell : row)
        if (cell == ch)
            ++count;
return count;
```

The outer `const auto&` is important: without it, each `row` would be a copy of
`std::array<char,3>`, a pointless allocation. Always bind the outer loop variable
by reference when looping over an `std::array` of arrays (notes 17.3 / 17.13).
</details>

<details><summary>Task 3 — index loop for row/column wins</summary>

For rows:
```cpp
for (int r { 0 }; r < 3; ++r) {
    char first { board[static_cast<std::size_t>(r)][0] };
    if (first != ' '
        && first == board[static_cast<std::size_t>(r)][1]
        && first == board[static_cast<std::size_t>(r)][2])
        return first;
}
return ' ';
```
Checking `first != ' '` prevents an all-empty row from being mistakenly called
a winner. The same pattern applies to columns — just transpose the indices.
</details>

<details><summary>Task 4 — diagonal comparisons (no loops needed)</summary>

```cpp
char mid { board[1][1] };           // centre cell shared by both diagonals

// main diagonal: (0,0) – (1,1) – (2,2)
if (mid != ' ' && board[0][0] == mid && board[2][2] == mid)
    return mid;

// anti-diagonal: (0,2) – (1,1) – (2,0)
if (mid != ' ' && board[0][2] == mid && board[2][0] == mid)
    return mid;

return ' ';
```
Checking `mid != ' '` short-circuits immediately if the centre is empty.
</details>

<details><summary>Task 5 — winner/isDraw orchestration</summary>

```cpp
char winner(const Board& board) {
    char w { rowWinner(board) };   if (w != ' ') return w;
    w = colWinner(board);          if (w != ' ') return w;
    w = diagWinner(board);         if (w != ' ') return w;
    return ' ';
}

bool isDraw(const Board& board) {
    return winner(board) == ' ' && isFull(board);
}
```
</details>

<details><summary>Task 6 — sumScores: plain index loop on a decayed pointer</summary>

```cpp
int total { 0 };
for (int i { 0 }; i < count; ++i)
    total += scores[i];
return total;
```
`scores[i]` is valid even though `scores` is a pointer — subscripting a pointer is
identical to `*(scores + i)` (notes 17.9). Remember: `count` is the ONLY
reliable source of length here; sizeof(scores) yields the pointer size, not the
array length.
</details>

<details><summary>Task 7 — firstNegative: pointer arithmetic with ++p</summary>

```cpp
for (const int* p { begin }; p != end; ++p) {
    if (*p < 0)
        return p;
}
return end;
```
`++p` advances by one `int` (the compiler scales by `sizeof(int)` for you — notes
17.9). `*p` dereferences to read the element. Returning `end` when no negative is
found is the standard "not found" convention for half-open ranges — the caller
checks `result != end`.
</details>

---

## Stretch goals (optional — some use later chapters)

- Add `printBoard(const Board&)` that renders the board to `std::cout` with ASCII
  borders (uses only ch ≤ 17 features).
- Make `winner()` return `std::optional<char>` so the absence of a winner is
  explicit rather than encoded as `' '` (notes ch 12 `std::optional`).
- Replace the `'X'`/`'O'`/`' '` `char` with a scoped `enum class Cell { X, O, Empty }`
  and update all functions — prevents the caller from passing arbitrary characters
  (Chapter 13).
- Write a flat-board variant `using FlatBoard = std::array<char, 9>` with an
  index mapping `idx(r, c) = r*3 + c`, as described in notes 17.13 ("Flattening"),
  and add a second set of helper functions for it.
- In a separate `main.cpp`, write a simple two-player console loop that lets two
  humans play; use `winner()` and `isDraw()` from this library to decide when to
  stop (uses only ch ≤ 17 features).
