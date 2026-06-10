// Chapter 10 — Type Conversion, Aliases, and Deduction · Project: statkit
// ─────────────────────────────────────────────────────────────────────────────
// This header is the CONTRACT between you and the grader, and it is also the
// chapter lesson in one page. DO NOT EDIT IT. The grader (tests/tests.cpp) and
// BOTH starter/statkit.cpp (yours) and solution/statkit.cpp (the reference)
// include it; change a signature here and nothing links.
//
// `statkit` is a tiny grade-book statistics library. Every function it promises
// makes ONE Chapter-10 idea PHYSICAL — something you can watch go wrong and then
// fix:
//
//   • mean(7, 2) must be 3.5, NOT 3   ← the integer-division truncation trap
//   • percentage(1, 4) must be 25.0    ← same trap, plus operand ORDER
//   • roundToInt(2.5) must be 3        ← intentional float→int narrowing, via cast
//   • clampScore / letterGrade         ← `using` aliases + char from int
//   • safeLength(".length()")          ← the unsigned size_t → int conversion
//   • weightedMean(int pts, double w)  ← the usual ARITHMETIC conversions
//
// The headline lesson of the whole chapter lives in this file: a conversion that
// the compiler will happily perform is not always the conversion you MEANT. The
// fix is to put a `static_cast` at exactly the right spot — no more, no fewer.
//
// "Pure" functions: each reads only its arguments and returns a value. No input,
// no output, no globals, no loops (loops are Chapter 8 — and the grader supplies
// the repetition by calling each function across many inputs).

#ifndef STATKIT_H              // ── header guard (Chapter 2) ───────────────────
#define STATKIT_H              // include this file's contents at most once per TU

#include <string_view>         // std::string_view — a cheap read-only text view (Ch 5)

// ─────────────────────────────────────────────────────────────────────────────
//  TYPE ALIASES (notes 10.7) — `using NewName = ExistingType;`
//
//  An alias gives an existing type a second, more meaningful NAME. It reads
//  left-to-right and is the modern replacement for `typedef`. Crucially, an alias
//  does NOT create a new, distinct type: `Score` below is *exactly* `double`, so
//  a `Score` and a plain `double` are freely interchangeable. The payoff is
//  READABILITY and one-place MAINTENANCE — if grades ever needed more precision
//  you'd change `double` here once, not in fifty signatures.
//
//  We use these aliases throughout the API so the signatures document intent:
//  a function returning an `Average` clearly produces a real-valued average, even
//  though the compiler just sees `double`.
// ─────────────────────────────────────────────────────────────────────────────
using Score   = double;   // a single graded value (e.g. a points-out-of-100 score)
using Percent = double;   // a value on the 0–100 percentage scale
using Average = double;   // the real-valued result of an averaging computation

namespace statkit
{
    // ─── TASK 1: mean — the integer-division truncation trap ──────────────────
    // Return the arithmetic mean of `total` divided over `count` items, as an
    // Average (double). Both inputs are int. The trap: `total / count` with two
    // ints does INTEGER division and throws away the remainder, so 7 / 2 is 3,
    // not 3.5. You must convert to double BEFORE the divide (a static_cast on one
    // operand promotes the whole expression). Assume count > 0.
    Average mean(int total, int count);

    // ─── TASK 2: percentage — same trap, and operand ORDER matters ────────────
    // Return part/whole expressed on a 0–100 scale, as a Percent (double).
    // percentage(1, 4) is 25.0. If you compute `(part / whole) * 100` with int
    // division you get 0 * 100 == 0; if you write `part / whole * 100.0` the int
    // division STILL happens first. Convert to floating point at the right spot so
    // the division is real-valued. Assume whole > 0.
    Percent percentage(int part, int whole);

    // ─── TASK 3: roundToInt — INTENTIONAL float→int narrowing, documented ──────
    // Round a double to the nearest int (round half away from zero) and return it
    // as an int. Plain static_cast<int>(2.5) TRUNCATES to 2 — it does not round.
    // Add (or subtract, for negatives) 0.5 before the cast. The static_cast is the
    // POINT here: it announces "I know this discards the fractional part; that is
    // intentional," which a silent `int x = value;` would not. Handle negatives:
    // roundToInt(-2.5) must be -3, not -2.
    int roundToInt(double value);

    // ─── TASK 4: roundTo — build on Task 3; use `auto` for an obvious local ────
    // Round `value` to `places` decimal places and return the double. Strategy:
    // scale up by 10^places, round to the nearest whole number, scale back down.
    // Reuse roundToInt for the middle step. Use `auto` (notes 10.8) for the
    // scale factor local, where the type is obvious from the initializer.
    // roundTo(3.14159, 2) is 3.14. Assume places is 0..6.
    double roundTo(double value, int places);

    // ─── TASK 5: clampScore — narrowing-aware clamp, returns an alias ─────────
    // Clamp a raw score into the valid 0–100 range and return it as a Score.
    // Below 0 → 0.0; above 100 → 100.0; otherwise the value unchanged. The input
    // is a double (it may carry fractional points); the return type is the `Score`
    // alias (which IS double — no information is lost). One ?: or two ifs.
    Score clampScore(double raw);

    // ─── TASK 6: letterGrade — a char ladder; `char` interacts with `int` ─────
    // Map a Percent (0–100) to a single letter grade and return it as a `char`:
    //   >= 90 -> 'A',  >= 80 -> 'B',  >= 70 -> 'C',  >= 60 -> 'D',  else 'F'.
    // Char literals like 'A' have type char, but the moment you do arithmetic on
    // them they PROMOTE to int (notes 10.2) — so returning a char from an int-y
    // expression would narrow. Here you just return the right char literal, but
    // the return type being `char` (not int) is the lesson: pick the type that
    // fits the value. Take the boundary cases (exactly 90, 80, …) as the higher
    // grade.
    char letterGrade(Percent score);

    // ─── TASK 7: safeLength — the unsigned size_t → int conversion (notes 10.3)─
    // std::string_view::length() returns a size_type, which is UNSIGNED. Mixing it
    // with signed ints triggers signed/unsigned warnings and, on subtraction, the
    // wrap-to-huge trap. Return the length as a plain signed `int`, using a
    // static_cast to DOCUMENT that you know the source is unsigned and the range
    // is small enough to be safe (notes 10.6 decision table). safeLength("") is 0.
    int safeLength(std::string_view text);

    // ─── TASK 8: weightedMean — the usual ARITHMETIC conversions (notes 10.5) ──
    // Compute a weighted mean: points are int, weights are double. Return the
    // Average. For three (point, weight) pairs:
    //   (p0*w0 + p1*w1 + p2*w2) / (w0 + w1 + w2)
    // When an int and a double meet under an operator, the int is converted to
    // double and the result is double (the "usual arithmetic conversions"). So
    // here the mixing works in your favor — but only because at least one operand
    // of every product is already a double. Assume the weights sum to > 0.
    Average weightedMean(int p0, double w0,
                         int p1, double w1,
                         int p2, double w2);
}

#endif // STATKIT_H
