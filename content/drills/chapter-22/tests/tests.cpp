// Chapter 22 — Move Semantics & Smart Pointers · Project: TrackedBuffer (GRADER)
// ─────────────────────────────────────────────────────────────────────────────
// Tiny no-framework unit-test harness (same style as drills/CLAUDE.md).
// Each CHECK that fails prints its expression and line number.
// Any failure → non-zero exit → `make test` is RED.
//
// The Makefile links this file against starter/tracked_buffer.cpp for `make test`
// and against solution/tracked_buffer.cpp for `make test-solution`.
//
// IMPORTANT DESIGN NOTES FOR TEST AUTHORSHIP
// ─────────────────────────────────────────────
// C++17 MANDATORY COPY ELISION: constructing a TrackedBuffer from a prvalue
// (e.g. the return value of makeTracked) does NOT invoke any copy/move ctor —
// the object is constructed directly in the destination. Therefore we NEVER
// check s_moves after a factory return. We only check moves on NAMED objects
// wrapped in std::move(), or after move assignment. (notes 22.3, 22.4)
//
// MOVED-FROM STATE: after `TrackedBuffer b2 { std::move(b1) }`, b1 must have
// size() == 0 and data() == nullptr. We test this explicitly.

#include <iostream>
#include <memory>
#include "../tracked_buffer.h"

static int fails = 0;

// CHECK: assert a boolean condition; on failure report expression and line.
#define CHECK(cond) \
    do { if(!(cond)){ std::cerr << "FAIL: " #cond "  @line " << __LINE__ << "\n"; ++fails; } } while(0)

int main()
{
    // ── Basic construction / empty state ──────────────────────────────────────
    {
        TrackedBuffer empty{};
        CHECK(empty.size() == 0);
        CHECK(empty.empty());
        CHECK(empty.data() == nullptr);

        TrackedBuffer buf{ 4 };
        CHECK(buf.size() == 4);
        CHECK(!buf.empty());
        CHECK(buf.data() != nullptr);
        // All elements are zero-initialised by new int[size]{}
        CHECK(buf[0] == 0);
        CHECK(buf[3] == 0);
    }

    // ── TASK 1: Copy constructor — deep copy, bumps s_copies not s_moves ─────
    {
        TrackedBuffer::resetCounters();

        TrackedBuffer src{ 3 };
        src[0] = 10; src[1] = 20; src[2] = 30;

        TrackedBuffer dst{ src };   // copy constructor

        // Counter check — copy path was taken, not move
        CHECK(TrackedBuffer::s_copies == 1);
        CHECK(TrackedBuffer::s_moves  == 0);

        // Value check — all elements duplicated
        CHECK(dst.size() == 3);
        // Guard element access: only dereference if the copy produced a valid array.
        if (dst.size() == 3 && dst.data() != nullptr)
        {
            CHECK(dst[0] == 10);
            CHECK(dst[1] == 20);
            CHECK(dst[2] == 30);

            // Independence check — modifying dst must NOT change src
            dst[0] = 99;
            CHECK(src[0] == 10);   // src unchanged: DEEP copy

            // Address check — they must own separate arrays
            CHECK(dst.data() != src.data());
        }
    }

    // ── TASK 1 (edge): copy of empty buffer ───────────────────────────────────
    {
        TrackedBuffer::resetCounters();

        TrackedBuffer empty{};
        TrackedBuffer copyEmpty{ empty };

        CHECK(TrackedBuffer::s_copies == 1);
        CHECK(copyEmpty.empty());
        CHECK(copyEmpty.data() == nullptr);
    }

    // ── TASK 2: Move constructor — steals pointer, bumps s_moves not s_copies
    {
        TrackedBuffer::resetCounters();

        TrackedBuffer src{ 5 };
        src[0] = 7; src[1] = 8; src[2] = 9; src[3] = 4; src[4] = 5;

        const int* originalData = src.data();  // remember the address

        TrackedBuffer dst{ std::move(src) };   // move constructor (named object)

        // Counter check — move path, not copy
        CHECK(TrackedBuffer::s_moves  == 1);
        CHECK(TrackedBuffer::s_copies == 0);

        // Destination received the data
        CHECK(dst.size() == 5);
        if (dst.size() == 5 && dst.data() != nullptr)
        {
            CHECK(dst[0] == 7);
            CHECK(dst[4] == 5);
            // The pointer was STOLEN (same address, not a new allocation)
            CHECK(dst.data() == originalData);
        }

        // Moved-from state: src must be empty (notes 22.3)
        CHECK(src.size() == 0);
        CHECK(src.empty());
        CHECK(src.data() == nullptr);
    }

    // ── TASK 2 (edge): move of empty buffer ───────────────────────────────────
    {
        TrackedBuffer::resetCounters();

        TrackedBuffer empty{};
        TrackedBuffer movedEmpty{ std::move(empty) };

        CHECK(TrackedBuffer::s_moves == 1);
        CHECK(movedEmpty.empty());
        CHECK(empty.empty());   // both empty, no UB
    }

    // ── TASK 3: Copy assignment ────────────────────────────────────────────────
    {
        TrackedBuffer::resetCounters();

        TrackedBuffer src{ 3 };
        src[0] = 1; src[1] = 2; src[2] = 3;

        TrackedBuffer dst{ 6 };   // dst already owns a different array
        dst = src;               // copy assignment

        CHECK(TrackedBuffer::s_copies == 1);
        CHECK(TrackedBuffer::s_moves  == 0);

        CHECK(dst.size() == 3);
        if (dst.size() == 3 && dst.data() != nullptr)
        {
            CHECK(dst[0] == 1);
            CHECK(dst[2] == 3);

            // Independence: modifying dst does not affect src
            dst[0] = 77;
            CHECK(src[0] == 1);

            // Separate arrays
            CHECK(dst.data() != src.data());
        }
    }

    // ── TASK 3 (edge): copy self-assignment ────────────────────────────────────
    {
        TrackedBuffer::resetCounters();

        TrackedBuffer buf{ 2 };
        buf[0] = 5; buf[1] = 6;

        // Self-assignment must be safe and NOT corrupt the data
        // Use a reference to avoid compiler warnings about obvious self-assign
        TrackedBuffer& ref = buf;
        buf = ref;

        CHECK(buf.size() == 2);
        CHECK(buf[0] == 5);
        CHECK(buf[1] == 6);
    }

    // ── TASK 4: Move assignment ────────────────────────────────────────────────
    {
        TrackedBuffer::resetCounters();

        TrackedBuffer src{ 4 };
        src[0] = 10; src[1] = 20; src[2] = 30; src[3] = 40;
        const int* originalData = src.data();

        TrackedBuffer dst{ 7 };   // dst currently owns a 7-element array
        dst = std::move(src);    // move assignment

        CHECK(TrackedBuffer::s_moves  == 1);
        CHECK(TrackedBuffer::s_copies == 0);

        // Destination has the data
        CHECK(dst.size() == 4);
        if (dst.size() == 4 && dst.data() != nullptr)
        {
            CHECK(dst[0] == 10);
            CHECK(dst[3] == 40);
            // Pointer was stolen
            CHECK(dst.data() == originalData);
        }

        // Source is empty (moved-from state)
        CHECK(src.size() == 0);
        CHECK(src.empty());
        CHECK(src.data() == nullptr);
    }

    // ── TASK 4 (edge): self-move-assignment must be safe AND preserve data ────
    // `b = std::move(b)` must leave b in a valid state (notes 22.3). The self-move
    // guard (if (this == &other) return *this;) achieves this by bailing out
    // BEFORE the delete[]/steal logic — so the buffer is left untouched. Without
    // the guard, the buffer would `delete[] m_data` and then read its own freed
    // pointer (this == &other), losing the data (and risking UB). We therefore
    // require the data to survive intact, which is exactly what the guard
    // guarantees and what TASK 4 instructs.
    {
        TrackedBuffer buf{ 3 };
        buf[0] = 1; buf[1] = 2; buf[2] = 3;
        const int* before = buf.data();

        // Use a reference so the self-move is not flagged by -Wself-move.
        TrackedBuffer& ref = buf;
        buf = std::move(ref);

        // Guard preserves the object exactly: same size, same array, same values.
        CHECK(buf.size() == 3);
        CHECK(buf.data() == before);   // pointer not freed/reallocated
        if (buf.size() == 3 && buf.data() != nullptr)
        {
            CHECK(buf[0] == 1);
            CHECK(buf[1] == 2);
            CHECK(buf[2] == 3);
        }
    }

    // ── TASK 5: makeTracked factory ───────────────────────────────────────────
    {
        // Factory returns a non-null unique_ptr owning a fresh TrackedBuffer.
        auto p = makeTracked(8);
        CHECK(p != nullptr);
        if (p)   // guard: only dereference if non-null
        {
            CHECK(p->size() == 8);         // correct size
            CHECK(!p->empty());
            CHECK((*p)[0] == 0);           // zero-initialised
        }

        // Ownership transfer: move the unique_ptr to a second variable.
        auto q = std::move(p);         // p is now null; q owns the buffer
        CHECK(!p);                     // p is empty after move (notes 22.5)
        CHECK(q != nullptr);
        if (q)   // guard
            CHECK(q->size() == 8);

        // makeTracked(0) gives a valid empty buffer
        auto empty = makeTracked(0);
        CHECK(empty != nullptr);
        if (empty)
            CHECK(empty->empty());
    }

    // ── TASK 6: takeOwnership — unique_ptr by value ────────────────────────────
    // takeOwnership accepts a unique_ptr BY VALUE, so the caller MUST std::move.
    // After the call the caller's pointer is null — ownership has transferred.
    // (notes 22.5: "takeOwnership(std::move(file)); // file is now empty")
    //
    // Side-effect note: takeOwnership also writes 42 into element [0] of a
    // non-empty buffer. That write is, by design, NOT observable from the caller:
    // because the unique_ptr is passed by value, takeOwnership becomes the sole
    // owner and frees the buffer when it returns. Reading the buffer afterward
    // would be a use-after-free, so we do not assert == 42 here. The observable,
    // testable contract of a by-value-consuming function is the OWNERSHIP TRANSFER
    // (the caller's pointer is emptied) — that is the lesson, and what we check.
    {
        auto p = makeTracked(4);
        CHECK(p != nullptr);
        if (p)   // guard: only call if non-null (else UB in takeOwnership body)
        {
            CHECK((*p)[0] == 0);         // zero-initialised before the call
            takeOwnership(std::move(p)); // body writes [0]=42, then frees
            CHECK(!p);                   // essential: caller's pointer is emptied
            // Buffer is freed now: reading (*p) or any saved raw pointer = UAF.
        }
    }

    // ── shared_ptr: unique→shared transfer, copy, and use_count ───────────────
    // std::shared_ptr is a Chapter 22 concept (notes 22.6): shared ownership via a
    // reference-counted control block. Copying a shared_ptr adds an owner and
    // raises use_count; destroying or reset()-ing one lowers it. unique→shared
    // ownership transfer is legal and one-directional (notes 22.6).
    {
        // Promote unique ownership to shared ownership.
        auto uniq = std::make_unique<TrackedBuffer>(4);
        std::shared_ptr<TrackedBuffer> keeper{ std::move(uniq) };
        CHECK(!uniq);                       // uniq emptied by the move
        CHECK(keeper.use_count() == 1);
        CHECK((*keeper)[0] == 0);           // shared object is usable

        // Copy the shared_ptr → a second owner shares the same control block.
        std::shared_ptr<TrackedBuffer> observer{ keeper };
        CHECK(keeper.use_count() == 2);
        CHECK(observer.use_count() == 2);
        CHECK(observer->size() == 4);

        observer.reset();                   // drop one owner
        CHECK(keeper.use_count() == 1);     // keeper still alive, object intact
        CHECK(keeper->size() == 4);
    }

    // ── shared_ptr basics: use_count ──────────────────────────────────────────
    // (notes 22.6: shared ownership, control block, use_count)
    // This is a READ-ONLY observation task — no implementation needed.
    // It verifies that TrackedBuffer works correctly inside a shared_ptr.
    {
        TrackedBuffer::resetCounters();

        std::shared_ptr<TrackedBuffer> a = std::make_shared<TrackedBuffer>(3);
        CHECK(a.use_count() == 1);

        {
            std::shared_ptr<TrackedBuffer> b = a;   // copy the shared_ptr (notes 22.6)
            CHECK(a.use_count() == 2);
            CHECK(b.use_count() == 2);
            // b goes out of scope here
        }

        CHECK(a.use_count() == 1);   // b is gone; a still alive
        CHECK(a->size() == 3);
    }

    // ── Summary ───────────────────────────────────────────────────────────────
    if (!fails)
        std::cout << "PASS \xE2\x9C\x85  all TrackedBuffer checks passed.\n";
    else
        std::cerr << "\nFAIL \xE2\x9D\x8C  " << fails
                  << " check(s) failed — fill in the TASK blocks in "
                     "starter/tracked_buffer.cpp until every check passes.\n";

    return fails ? 1 : 0;
}
