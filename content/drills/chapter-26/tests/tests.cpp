// Chapter 26 — Templates and Classes · Project: Stack<T>   (GRADER)
// ─────────────────────────────────────────────────────────────────────────────
// No-framework unit-test harness (drills/CLAUDE.md Style B2).
// The Makefile compiles this file with -Istarter or -Isolution so the correct
// stack.h is found via the plain  #include "stack.h"  below.
//
//   make test           -> grades YOUR starter/stack.h   (RED until done)
//   make test-solution  -> grades solution/stack.h       (must be GREEN)
//
// The tests instantiate BOTH Stack<int> AND Stack<std::string> (the core
// lesson: one template, many element types) plus edge cases for push-when-full,
// top/pop-preconditions, and TypeLabel specializations.
// ─────────────────────────────────────────────────────────────────────────────

#include <iostream>
#include <string>
#include <string_view>
#include "stack.h"     // resolved via -Istarter or -Isolution (Style B2)

static int fails = 0;

// CHECK — print what failed and where; increment the failure counter.
#define CHECK(cond) \
    do { if (!(cond)) { \
        std::cerr << "FAIL: " #cond "  @line " << __LINE__ << "\n"; \
        ++fails; \
    } } while (0)

int main()
{
    // ── Task 1 + 3: Stack<int> — basic operations ─────────────────────────────
    {
        Stack<int> s;

        // fresh stack is empty and not full
        CHECK(s.isEmpty());
        CHECK(!s.isFull());
        CHECK(s.size() == 0);

        // push three values, check size and top after each
        CHECK(s.push(10));
        CHECK(s.size() == 1);
        CHECK(!s.isEmpty());
        CHECK(s.top() == 10);

        CHECK(s.push(20));
        CHECK(s.size() == 2);
        CHECK(s.top() == 20);   // top is LAST pushed

        CHECK(s.push(30));
        CHECK(s.size() == 3);
        CHECK(s.top() == 30);

        // pop (out-of-class definition — Task 3)
        s.pop();
        CHECK(s.size() == 2);
        CHECK(s.top() == 20);   // popping 30 reveals 20

        s.pop();
        CHECK(s.size() == 1);
        CHECK(s.top() == 10);

        s.pop();
        CHECK(s.isEmpty());
        CHECK(s.size() == 0);
    }

    // ── Task 1: push-when-full returns false ──────────────────────────────────
    {
        Stack<int, 3> small;    // non-type param: capacity 3 (notes 26.2)
        CHECK(small.push(1));
        CHECK(small.push(2));
        CHECK(small.push(3));
        CHECK(small.isFull());
        CHECK(small.size() == 3);

        bool ok = small.push(99);   // must return false — stack is full
        CHECK(!ok);
        CHECK(small.size() == 3);   // size unchanged
        CHECK(small.top() == 3);    // top unchanged
    }

    // ── Task 1: Stack<std::string> — the second instantiation ─────────────────
    // The whole point: ONE class template, TWO element types. If your template
    // accidentally hardcodes int assumptions, these checks will catch it.
    {
        Stack<std::string> words;
        CHECK(words.isEmpty());

        CHECK(words.push("hello"));
        CHECK(words.push("world"));
        CHECK(words.size() == 2);
        CHECK(words.top() == "world");

        words.pop();                     // out-of-class pop with std::string
        CHECK(words.size() == 1);
        CHECK(words.top() == "hello");

        words.pop();
        CHECK(words.isEmpty());
    }

    // ── Task 1: Stack<std::string> fill to capacity ───────────────────────────
    {
        Stack<std::string, 2> tiny;
        CHECK(tiny.push("alpha"));
        CHECK(tiny.push("beta"));
        CHECK(tiny.isFull());
        CHECK(!tiny.push("gamma"));    // full: must return false
        CHECK(tiny.size() == 2);
        CHECK(tiny.top() == "beta");
    }

    // ── Task 1 edge: single-element stack ────────────────────────────────────
    {
        Stack<int, 1> one;
        CHECK(one.isEmpty());
        CHECK(!one.isFull());
        CHECK(one.push(42));
        CHECK(one.isFull());
        CHECK(!one.isEmpty());
        CHECK(one.top() == 42);
        CHECK(!one.push(99));      // full after one push
        one.pop();
        CHECK(one.isEmpty());
    }

    // ── Task 1 edge: push-pop-push reuse ─────────────────────────────────────
    // After popping, the capacity slot should be reusable.
    {
        Stack<int, 2> s;
        CHECK(s.push(1));
        CHECK(s.push(2));
        CHECK(s.isFull());
        s.pop();
        CHECK(!s.isFull());
        CHECK(s.push(3));         // slot freed by pop is now reusable
        CHECK(s.isFull());
        CHECK(s.top() == 3);
        CHECK(s.size() == 2);
    }

    // ── Task 2 + 4: TypeLabel primary template and specializations ────────────
    // TypeLabel<T>::name is a compile-time string_view (constexpr).
    // Unknown type: primary template must say "unknown".
    {
        // Use an unspecialized type as a stand-in for "unknown"
        struct MyLocalType {};
        CHECK(TypeLabel<MyLocalType>::name == "unknown");

        // Full specializations (Task 4) must override the primary
        CHECK(TypeLabel<int>::name          == "int");
        CHECK(TypeLabel<double>::name       == "double");
        CHECK(TypeLabel<std::string_view>::name == "string_view");
    }

    // ── Task 2 + 4: TypeLabel with Stack instantiation types ─────────────────
    // A realistic use case: the label follows the element type through the
    // template.  This pattern appears in LLVM diagnostic helpers that print the
    // type name of a container's element.
    {
        CHECK(TypeLabel<int>::name    == "int");     // still holds after Stack use
        CHECK(TypeLabel<double>::name == "double");  // specialization is consistent
    }

    // ── Task 3: out-of-class pop syntax — exhaustive round-trip ─────────────
    // Push the default capacity (8), verify order, pop all, verify empty.
    {
        Stack<int> full;    // default Capacity = 8
        for (int i = 1; i <= 8; ++i)
            CHECK(full.push(i));

        CHECK(full.isFull());
        CHECK(!full.push(99));    // one more push must fail

        // pop in LIFO order: 8, 7, … 1
        for (int expected = 8; expected >= 1; --expected)
        {
            CHECK(full.top() == expected);
            full.pop();
        }
        CHECK(full.isEmpty());
    }

    // ── Final verdict ─────────────────────────────────────────────────────────
    if (!fails)
        std::cout << "PASS \xE2\x9C\x85 all stack checks passed.\n";
    else
        std::cerr << "\nFAIL \xE2\x9D\x8C " << fails
                  << " check(s) failed — fix the TASK blocks in starter/stack.h.\n";

    return fails ? 1 : 0;
}
