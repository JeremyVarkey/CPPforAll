// ============================================================================
//  tests/tests.cpp  —  automated grader for the DynBuffer Workbench (Ch 19)
// ----------------------------------------------------------------------------
//  This file is the CONSUMER of dynbuf.h.  It includes the contract header
//  and calls every API function through a tiny no-framework harness.  The
//  Makefile compiles it against EITHER starter/dynbuf.cpp  (`make test`) OR
//  solution/dynbuf.cpp  (`make test-solution`).
//
//  IMPORTANT — LEAK-CLEAN BY CONSTRUCTION:
//  Every pointer returned by makeBuffer / cloneBuffer / resizeBuffer is freed
//  (via destroyBuffer or a direct delete[]) before the test exits.  If your
//  implementation is correct, there are zero unmatched allocations.  If the
//  starter placeholders are used, the leaked allocation from the placeholder
//  resizeBuffer is detectable by ASan / Valgrind.
//
//  Tiny harness — no external dependencies:
//    CHECK(cond) — fails with condition text + line number; increments `fails`.
//  The program exits 0 on full pass, 1 on any failure.
// ============================================================================

#include <iostream>
#include "../dynbuf.h"

static int fails { 0 };

#define CHECK(cond) \
    do { \
        if (!(cond)) { \
            std::cerr << "FAIL: " #cond "  @line " << __LINE__ << "\n"; \
            ++fails; \
        } \
    } while (0)

int main()
{
    // ═════════════════════════════════════════════════════════════════════════
    //  TASK 1 — makeBuffer: allocate and fill
    // ─────────────────────────────────────────────────────────────────────────
    // Basic fill — normal case
    {
        int* buf { makeBuffer(5, 7) };
        for (int i { 0 }; i < 5; ++i)
            CHECK(buf[i] == 7);          // every element must equal fill
        delete[] buf;
    }

    // Fill with 0
    {
        int* buf { makeBuffer(3, 0) };
        for (int i { 0 }; i < 3; ++i)
            CHECK(buf[i] == 0);
        delete[] buf;
    }

    // Fill with a negative value
    {
        int* buf { makeBuffer(4, -1) };
        for (int i { 0 }; i < 4; ++i)
            CHECK(buf[i] == -1);
        delete[] buf;
    }

    // Edge case: length 1
    {
        int* buf { makeBuffer(1, 99) };
        CHECK(buf[0] == 99);
        delete[] buf;
    }

    // Edge case: length 0 — new int[0] is legal; we get a non-null pointer we
    //   must still delete[].  No elements to check, but must not crash.
    {
        int* buf { makeBuffer(0, 42) };
        // buf != nullptr is guaranteed by the standard for new int[0]
        delete[] buf;
    }

    // ═════════════════════════════════════════════════════════════════════════
    //  TASK 2 — destroyBuffer: delete[] and null out
    // ─────────────────────────────────────────────────────────────────────────
    // After destroyBuffer the pointer variable must be nullptr.
    {
        int* buf { makeBuffer(3, 5) };
        destroyBuffer(buf);
        CHECK(buf == nullptr);   // the in-out ref must have written nullptr back
    }

    // Calling destroyBuffer on nullptr must be a no-op (not a crash).
    // delete[] nullptr is defined to have no effect; the assignment is also fine.
    {
        int* p { nullptr };
        destroyBuffer(p);
        CHECK(p == nullptr);
    }

    // ═════════════════════════════════════════════════════════════════════════
    //  TASK 3 — cloneBuffer: deep copy
    // ─────────────────────────────────────────────────────────────────────────
    // Cloned array must have the same values as the original.
    {
        int* original { makeBuffer(4, 0) };
        original[0] = 10; original[1] = 20; original[2] = 30; original[3] = 40;

        int* clone { cloneBuffer(original, 4) };

        CHECK(clone[0] == 10);
        CHECK(clone[1] == 20);
        CHECK(clone[2] == 30);
        CHECK(clone[3] == 40);

        // ── DEEP-COPY independence test ───────────────────────────────────
        // Mutating the ORIGINAL must NOT affect the clone (and vice versa).
        // If cloneBuffer just copied the pointer (shallow copy) this would fail.
        original[0] = 999;
        CHECK(clone[0] == 10);   // clone is independent: still 10, not 999

        clone[3] = -1;
        CHECK(original[3] == 40); // original is independent: still 40

        destroyBuffer(original);
        destroyBuffer(clone);
    }

    // Clone of a length-0 buffer — legal edge case
    {
        int* empty { makeBuffer(0, 0) };
        int* cloned { cloneBuffer(empty, 0) };
        // Both are valid (non-null) heap pointers we must free.
        destroyBuffer(empty);
        destroyBuffer(cloned);
    }

    // ═════════════════════════════════════════════════════════════════════════
    //  TASK 4 — resizeBuffer: grow and shrink
    // ─────────────────────────────────────────────────────────────────────────

    // ── Growing: prefix is preserved, new slots are 0 ────────────────────
    {
        int* buf { makeBuffer(3, 0) };
        buf[0] = 1; buf[1] = 2; buf[2] = 3;

        resizeBuffer(buf, 3, 6);   // grow from 3 to 6

        // Prefix: original values intact
        CHECK(buf[0] == 1);
        CHECK(buf[1] == 2);
        CHECK(buf[2] == 3);
        // New slots: value-initialised to 0
        CHECK(buf[3] == 0);
        CHECK(buf[4] == 0);
        CHECK(buf[5] == 0);

        destroyBuffer(buf);
    }

    // ── Shrinking: keeps only the prefix ──────────────────────────────────
    {
        int* buf { makeBuffer(5, 0) };
        buf[0] = 10; buf[1] = 20; buf[2] = 30; buf[3] = 40; buf[4] = 50;

        resizeBuffer(buf, 5, 3);   // shrink from 5 to 3

        CHECK(buf[0] == 10);
        CHECK(buf[1] == 20);
        CHECK(buf[2] == 30);
        // Elements at indices 3 and 4 are gone — we only have 3 elements now.

        destroyBuffer(buf);
    }

    // ── Shrink to newN == 1 (extreme prefix) ──────────────────────────────
    {
        int* buf { makeBuffer(4, 0) };
        buf[0] = 7; buf[1] = 8; buf[2] = 9; buf[3] = 10;

        resizeBuffer(buf, 4, 1);

        CHECK(buf[0] == 7);   // only the first element survives

        destroyBuffer(buf);
    }

    // ── Resize to same size — no-op semantically ──────────────────────────
    {
        int* buf { makeBuffer(3, 5) };
        resizeBuffer(buf, 3, 3);
        for (int i { 0 }; i < 3; ++i)
            CHECK(buf[i] == 5);
        destroyBuffer(buf);
    }

    // ── Grow from 0 ───────────────────────────────────────────────────────
    {
        int* buf { makeBuffer(0, 0) };
        resizeBuffer(buf, 0, 4);
        for (int i { 0 }; i < 4; ++i)
            CHECK(buf[i] == 0);   // all zero (value-init)
        destroyBuffer(buf);
    }

    // ── Shrink to 0 ───────────────────────────────────────────────────────
    {
        int* buf { makeBuffer(3, 9) };
        resizeBuffer(buf, 3, 0);
        // buf now points to a valid (non-null) zero-length allocation
        destroyBuffer(buf);
        CHECK(buf == nullptr);
    }

    // ═════════════════════════════════════════════════════════════════════════
    //  TASK 5 — bufferSum: read-only observer
    // ─────────────────────────────────────────────────────────────────────────
    // Basic sum
    {
        int* buf { makeBuffer(4, 0) };
        buf[0] = 1; buf[1] = 2; buf[2] = 3; buf[3] = 4;
        CHECK(bufferSum(buf, 4) == 10);
        destroyBuffer(buf);
    }

    // All-same fill
    {
        int* buf { makeBuffer(5, 3) };
        CHECK(bufferSum(buf, 5) == 15);
        destroyBuffer(buf);
    }

    // Negative values
    {
        int* buf { makeBuffer(3, 0) };
        buf[0] = -1; buf[1] = -2; buf[2] = -3;
        CHECK(bufferSum(buf, 3) == -6);
        destroyBuffer(buf);
    }

    // Zero-length — sum of nothing is 0
    {
        int* buf { makeBuffer(0, 99) };
        CHECK(bufferSum(buf, 0) == 0);
        destroyBuffer(buf);
    }

    // Mixed sign
    {
        int* buf { makeBuffer(4, 0) };
        buf[0] = 10; buf[1] = -3; buf[2] = 5; buf[3] = -2;
        CHECK(bufferSum(buf, 4) == 10);
        destroyBuffer(buf);
    }

    // ═════════════════════════════════════════════════════════════════════════
    //  INTEGRATION — chain of operations (mirrors real code that uses a buffer)
    // ─────────────────────────────────────────────────────────────────────────
    {
        // Allocate, work, clone (deep copy), resize the original, verify
        // independence throughout.
        int* data { makeBuffer(3, 0) };
        data[0] = 1; data[1] = 2; data[2] = 3;

        int* snapshot { cloneBuffer(data, 3) };   // deep copy before resize

        resizeBuffer(data, 3, 5);   // grow; snapshot must be unaffected
        data[3] = 4; data[4] = 5;

        // snapshot still reflects the pre-resize values
        CHECK(bufferSum(snapshot, 3) == 6);   // 1+2+3
        // data has the grown version
        CHECK(bufferSum(data, 5) == 15);      // 1+2+3+4+5

        destroyBuffer(snapshot);
        destroyBuffer(data);
    }

    // ═════════════════════════════════════════════════════════════════════════
    //  RESULT
    // ─────────────────────────────────────────────────────────────────────────
    if (!fails)
        std::cout << "PASS \xE2\x9C\x85  all dynbuf checks passed.\n";
    else
        std::cout << "FAIL \xE2\x9D\x8C  " << fails
                  << " check(s) failed — see lines above.\n";

    return fails ? 1 : 0;
}
