// Chapter 10 — Type Conversion, Aliases, and Deduction · statkit   (REFERENCE)
// ─────────────────────────────────────────────────────────────────────────────
// One complete, correct, warning-clean implementation. Peek only after you've
// taken a real swing at starter/statkit.cpp — the learning is in finding where
// each conversion belongs, then comparing.
//
// Theme: every function below places a conversion deliberately. Where a cast is
// needed it is `static_cast` (loud, auditable, intentional); where the usual
// arithmetic conversions already do the right thing (Task 8) there is NO cast,
// because a needless cast is also a lie about what the code is doing.

#include "../statkit.h"

namespace statkit
{
    // TASK 1 — mean.
    // The fix for integer-division truncation: convert ONE operand to double
    // BEFORE the divide. Once the left operand is double, the usual arithmetic
    // conversions promote `count` to double too, so the division is real-valued
    // and 7 / 2 yields 3.5. Casting the *result* (static_cast<double>(total/count))
    // would be too late — the truncation already happened in int.
    Average mean(int total, int count)
    {
        return static_cast<double>(total) / count;
    }

    // TASK 2 — percentage.
    // Same trap, with operand order to watch: the division must be floating point.
    // We cast `part` to double so `part / whole` is real-valued, then scale by
    // 100.0. percentage(1, 4) -> 0.25 -> 25.0.
    Percent percentage(int part, int whole)
    {
        return static_cast<double>(part) / whole * 100.0;
    }

    // TASK 3 — roundToInt.
    // static_cast<int> truncates toward zero, so to round to nearest we nudge by
    // 0.5 in the value's own direction first. This keeps symmetry for negatives:
    // 2.5 -> 3 and -2.5 -> -3 (round half away from zero). The cast documents the
    // intentional float→int narrowing (notes 10.4 / 10.6).
    int roundToInt(double value)
    {
        return (value >= 0.0)
            ? static_cast<int>(value + 0.5)
            : static_cast<int>(value - 0.5);
    }

    // TASK 4 — roundTo.
    // Scale up, round to nearest whole, scale back down. `auto` is used for the
    // scale factor because the initializer makes the type (double) obvious — the
    // idiomatic place for type deduction (notes 10.8). We build 10^places by
    // repeated multiplication over the small, bounded range 0..6 (no loop: a tiny
    // table). roundToInt returns int; dividing it by the double `factor` converts
    // it back to double via the usual arithmetic conversions.
    double roundTo(double value, int places)
    {
        // 10^places for places in 0..6, as a double. (constexpr-friendly lookup.)
        const double powers[] { 1.0, 10.0, 100.0, 1000.0,
                                10000.0, 100000.0, 1000000.0 };
        auto factor { powers[places] };   // auto deduces double from the array element

        return roundToInt(value * factor) / factor;
    }

    // TASK 5 — clampScore.
    // A nested conditional that pins the value into [0, 100]. The `Score` return
    // type is an alias for double, so the fractional part survives untouched —
    // aliases rename, they do not narrow (notes 10.7).
    Score clampScore(double raw)
    {
        return (raw < 0.0)   ? 0.0
             : (raw > 100.0) ? 100.0
             :                 raw;
    }

    // TASK 6 — letterGrade.
    // A relational ladder returning a char literal per band; boundaries resolve to
    // the higher grade because each test is strictly above the next. The return
    // TYPE is char — the right-sized type for a single letter — which is the
    // chapter point: a char literal is a char, and we keep it that way (no
    // arithmetic that would promote it to int).
    char letterGrade(Percent score)
    {
        return (score >= 90.0) ? 'A'
             : (score >= 80.0) ? 'B'
             : (score >= 70.0) ? 'C'
             : (score >= 60.0) ? 'D'
             :                   'F';
    }

    // TASK 7 — safeLength.
    // std::string_view::length() returns an unsigned size_type. We convert it to a
    // signed int with an explicit static_cast: the cast is a documented promise
    // that the text is short enough that the value fits, and it silences the
    // signed/unsigned warning we'd otherwise get from an implicit conversion
    // (notes 10.3 / 10.6 decision table).
    int safeLength(std::string_view text)
    {
        return static_cast<int>(text.length());
    }

    // TASK 8 — weightedMean.
    // The usual arithmetic conversions do the work: in each `int * double` product
    // the int operand is converted to double, so numerator and denominator are
    // both double and the quotient is a double Average. No static_cast is needed —
    // and adding one would falsely imply a narrowing that isn't happening.
    Average weightedMean(int p0, double w0,
                         int p1, double w1,
                         int p2, double w2)
    {
        return (p0 * w0 + p1 * w1 + p2 * w2) / (w0 + w1 + w2);
    }
}
