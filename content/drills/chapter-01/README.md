# Chapter 1 — C++ Basics · Project: Cash Register

> Reinforces [`../../notes/chapter-01.md`](../../notes/chapter-01.md) · LearnCpp [Chapter 1](https://www.learncpp.com/)

## The project
You're writing the brain of a supermarket cash register. The program reads three
whole numbers — a **unit price**, a **quantity**, and the **cash** the customer
hands over — then prints a tidy receipt showing the line item, the subtotal, and
the change due.

It's deliberately tiny: no functions, no `if`, no loops (those come in later
chapters). The whole point is to get fluent with the four things Chapter 1 is
about — **defining and initializing variables**, **reading input with `std::cin`**,
**computing with operators and expressions**, and **printing with `std::cout`**.

## Concepts practiced
- Defining variables and **list/value-initializing** them with `{}` (1.4)
- Reading input with `std::cin >>`, chained (1.5)
- Arithmetic **operators** and **expressions** (1.9, 1.10)
- Output with `std::cout <<` and `'\n'` (1.5)
- `main` returning `0` (1.1)

## Your tasks
The starter compiles and runs immediately — but it prints all zeros. Fill in the
three `>>> YOUR CODE HERE <<<` blocks in [`starter/main.cpp`](starter/main.cpp):

1. **Read the inputs.** Read three values from `std::cin` in this order:
   `unitPrice`, `quantity`, `amountPaid`. One chained `std::cin >> a >> b >> c;` is ideal.
2. **Compute the subtotal:** `unitPrice * quantity`.
3. **Compute the change:** `amountPaid - subtotal`.

The receipt-printing lines are written for you — leave them alone so your output
matches the expected format exactly.

## Constraints
- Use `int` for every variable. No other types.
- Only `#include <iostream>`. No functions of your own, no `if`, no loops (not taught yet).
- **Initialize every variable when you define it** (Chapter 1's golden rule).
- Don't change the three `std::cout` lines or the output format.

## Build & run
```sh
make          # compile starter/  ->  starter/app
make run      # run it; then type:  5 3 20  <Enter>
make test     # grade your code against the expected output
make solution # run the reference solution if you get stuck
make clean
```

## Success criteria
`make test` prints **PASS ✅**. It feeds the program the input in
[`tests/input.txt`](tests/input.txt) (`5 3 20` → price 5, qty 3, paid 20) and checks
that the output matches [`tests/expected.txt`](tests/expected.txt) exactly:
```
Items: 3 x $5
Subtotal: $15
Change: $5
```
Until you fill in the tasks, the program prints zeros and `make test` shows **FAIL ❌**
with a diff. Turning that red into green is the whole exercise.

## Hints
<details><summary>Task 1 — reading input</summary>

`std::cin >> unitPrice >> quantity >> amountPaid;` reads three whitespace-separated
integers in one statement (the extraction operator skips the spaces between them).
</details>
<details><summary>Tasks 2 & 3 — the math</summary>

Replace the empty `{}` initializer with the real expression, e.g.
`int subtotal { unitPrice * quantity };`. An expression can go right inside the braces.
</details>

## Stretch goals (optional — need later chapters)
- Print `"Insufficient payment"` when `amountPaid < subtotal` (needs `if`, Chapter 4/8).
- Support prices with cents using `double` and 2-decimal formatting (Chapters 4 & 28).
