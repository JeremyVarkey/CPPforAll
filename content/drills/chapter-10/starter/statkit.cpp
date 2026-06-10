// Chapter 10 — Type Conversion, Aliases, and Deduction · statkit   (STARTER)
// ─────────────────────────────────────────────────────────────────────────────
// Fill in the eight TASK blocks below. Each maps 1:1 to a task in the README and
// to a declaration in ../statkit.h. The bodies currently return PLACEHOLDERS so
// the file compiles immediately — that is why `make test` is RED right now. Many
// of the placeholders even reproduce the CLASSIC BUG on purpose (e.g. mean does
// integer division and returns 3 for 7/2), so the failing checks point straight
// at the conversion you need to fix. Your job is to turn the wall of red GREEN.
//
//     make build      compile-check your code (should already work)
//     make test       grade it          (RED until you fill these in)
//     make solution   run the reference if you get stuck
//
// No loops, no I/O, no globals — just get the CONVERSIONS in the right places.

#include "../statkit.h"
// (No extra headers are needed. Rounding here is done with + 0.5 and a cast, not
//  std::round — the point of the chapter is to see the conversion happen.)

namespace statkit
{
    // ─── TASK 1: mean — fix the integer-division truncation ───────────────────
    // RIGHT NOW this does `total / count` with two ints, so mean(7, 2) returns 3.0
    // (the .5 was thrown away by integer division before it ever became a double).
    // Convert ONE operand to double BEFORE dividing so the division is real-valued:
    //     static_cast<double>(total) / count
    // (Once one side is double, `count` is promoted and the result is double.)
    //
    //   >>> YOUR CODE HERE <<<
    //
    Average mean(int total, int count)
    {
        return total / count;   // placeholder — INTEGER division, so 7/2 == 3, not 3.5
    }

    // ─── TASK 2: percentage — real division, on a 0–100 scale ─────────────────
    // percentage(1, 4) must be 25.0. Integer `part / whole` is 0, and 0 * 100 is
    // still 0, so converting AFTER the divide is too late. Do the division in
    // floating point, e.g.:  static_cast<double>(part) / whole * 100.0
    //
    //   >>> YOUR CODE HERE <<<
    //
    Percent percentage(int /*part*/, int /*whole*/)
    {
        return 0.0;   // placeholder
    }

    // ─── TASK 3: roundToInt — round half away from zero, then cast ────────────
    // static_cast<int>(value) TRUNCATES toward zero (2.9 -> 2, -2.9 -> -2); it does
    // not round. To round to nearest, nudge by 0.5 in the direction of the value's
    // sign, THEN cast. One clean way that also handles negatives:
    //     value >= 0 ? static_cast<int>(value + 0.5)
    //                : static_cast<int>(value - 0.5)
    // The cast is intentional and documents the float→int narrowing.
    //
    //   >>> YOUR CODE HERE <<<
    //
    int roundToInt(double /*value*/)
    {
        return 0;   // placeholder
    }

    // ─── TASK 4: roundTo — scale, round, unscale (use `auto` for the factor) ──
    // Round to `places` decimals. Compute 10^places as a double, scale the value
    // up, round to nearest with roundToInt (Task 3), then scale back down:
    //     auto factor { ... };                 // a double like 100.0 for places==2
    //     return roundToInt(value * factor) / factor;
    // `auto` is appropriate here — the type is obvious from the initializer (10.8).
    // Build factor without a loop: for places 0..6 you may multiply 10.0 the right
    // number of times, or simply switch on the small set of `places` values.
    //
    //   >>> YOUR CODE HERE <<<
    //
    double roundTo(double value, int /*places*/)
    {
        return value;   // placeholder — returns value unrounded
    }

    // ─── TASK 5: clampScore — clamp into [0, 100], return as the Score alias ──
    // Below 0 -> 0.0; above 100 -> 100.0; otherwise unchanged. The `Score` return
    // type is just `double`, so returning the (possibly fractional) value loses
    // nothing. A nested ?: reads cleanly:
    //     raw < 0.0 ? 0.0 : raw > 100.0 ? 100.0 : raw
    //
    //   >>> YOUR CODE HERE <<<
    //
    Score clampScore(double /*raw*/)
    {
        return 0.0;   // placeholder
    }

    // ─── TASK 6: letterGrade — return the right `char` for the band ───────────
    // >= 90 -> 'A', >= 80 -> 'B', >= 70 -> 'C', >= 60 -> 'D', else 'F'. Boundaries
    // go to the higher grade (exactly 90 is 'A'). Return a char LITERAL directly —
    // a nested ?: or an if/return ladder both work. (Note: 'A' is a char, but if
    // you did 'A' + 1 it would promote to int; you don't need arithmetic here.)
    //
    //   >>> YOUR CODE HERE <<<
    //
    char letterGrade(Percent /*score*/)
    {
        return '?';   // placeholder
    }

    // ─── TASK 7: safeLength — unsigned size_t length -> signed int, documented ─
    // text.length() returns an UNSIGNED size type. Return it as a plain `int` with
    // a static_cast that documents the intentional signed/unsigned conversion:
    //     static_cast<int>(text.length())
    // (Returning .length() straight into an int would trip a signedness warning;
    //  the cast is your promise that the range is small enough to be safe.)
    //
    //   >>> YOUR CODE HERE <<<
    //
    int safeLength(std::string_view /*text*/)
    {
        return 0;   // placeholder
    }

    // ─── TASK 8: weightedMean — let the usual arithmetic conversions work ─────
    // (p0*w0 + p1*w1 + p2*w2) / (w0 + w1 + w2). Each `int * double` product
    // converts the int to double automatically (usual arithmetic conversions,
    // 10.5), so the whole numerator and denominator are doubles and the result is
    // a double Average. No casts are needed BECAUSE a double is already present in
    // every product — write the formula directly.
    //
    //   >>> YOUR CODE HERE <<<
    //
    Average weightedMean(int /*p0*/, double /*w0*/,
                         int /*p1*/, double /*w1*/,
                         int /*p2*/, double /*w2*/)
    {
        return 0.0;   // placeholder
    }
}
