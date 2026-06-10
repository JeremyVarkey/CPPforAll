// Chapter 22 — Move Semantics & Smart Pointers · Project: TrackedBuffer (SOLUTION)
// ─────────────────────────────────────────────────────────────────────────────
// Reference implementation — complete, correct, richly commented.
// Every design decision is explained so you can compare with your own solution.
//
// RULE OF FIVE (notes 22.3)
// TrackedBuffer owns a raw pointer (m_data). Any class that owns a raw pointer
// must define ALL FIVE special members explicitly, because the compiler-generated
// versions do shallow copies — they copy the pointer value, not the data:
//
//   Two objects would point at the same array.
//   Whichever object is destroyed first frees the array.
//   The second object then holds a DANGLING POINTER → UB on destruction.
//
// The five members we define: destructor, copy ctor, copy assignment,
//                              move ctor, move assignment.

#include "../tracked_buffer.h"

#include <algorithm>   // std::copy_n
#include <cstddef>     // std::size_t
#include <memory>      // std::unique_ptr, std::make_unique

// ── Static counter definitions ────────────────────────────────────────────────
// Static class members are DECLARED in the header; they need exactly ONE
// DEFINITION in a .cpp file. (Chapter 15: static class members)
int TrackedBuffer::s_copies { 0 };
int TrackedBuffer::s_moves  { 0 };

void TrackedBuffer::resetCounters()
{
    s_copies = 0;
    s_moves  = 0;
}

// ── Default / size constructor (provided, not a task) ─────────────────────────
// Value-initialises the array with `new int[size]{}` so all elements start at 0.
// A size of 0 keeps m_data as nullptr — that is the "empty" state we use for
// moved-from buffers too. (Chapter 19: dynamic allocation with new[]/delete[])
TrackedBuffer::TrackedBuffer(std::size_t size)
    : m_size { size }
    , m_data { size > 0 ? new int[size]{} : nullptr }
{
}

// ── Destructor (provided, not a task) ────────────────────────────────────────
// delete[] nullptr is explicitly defined to be a no-op (C++ standard), so this
// destructor works safely for empty buffers, default-constructed buffers, and
// moved-from buffers (which had their pointer zeroed to nullptr).
TrackedBuffer::~TrackedBuffer()
{
    delete[] m_data;
}

// ── TASK 1: Copy constructor (deep copy) ─────────────────────────────────────
// WHY deep copy: two objects must own INDEPENDENT arrays. If we just copied
// m_data (shallow), both destructors would delete the same memory — UB.
//
// DESIGN: We allocate a fresh array of the SAME size and then copy each element
// with std::copy_n (from <algorithm>). An empty `other` (size 0) is handled by
// keeping m_data = nullptr and skipping copy_n.
//
// s_copies lets tests observe that this path was taken (not the move path).
TrackedBuffer::TrackedBuffer(const TrackedBuffer& other)
    : m_size { other.m_size }
    , m_data { other.m_size > 0 ? new int[other.m_size]{} : nullptr }
{
    if (m_size > 0)
        std::copy_n(other.m_data, m_size, m_data);

    ++s_copies;
}

// ── TASK 2: Move constructor (steal pointer, leave source empty) ───────────────
// WHY move: if `other` is about to be destroyed (a temporary or an explicit
// std::move), allocating a fresh array and copying every element is wasteful.
// Instead we STEAL the pointer — O(1) regardless of array size.
//
// AFTER THE STEAL: we MUST null out `other.m_data` and zero `other.m_size`.
// The source object still exists (until its own destructor fires) and its
// destructor calls `delete[] m_data`. If we left the original pointer in `other`,
// the same array would be deleted twice — UB.
//
// noexcept: pointer steal and integer copy cannot throw. Marking noexcept tells
// std::vector to prefer this move during reallocation (notes 22.3).
//
// C++17 NOTE: prvalue return values are subject to MANDATORY COPY ELISION —
// the compiler constructs the result directly; neither the copy nor the move
// constructor fires. That is why we do NOT test s_moves on the return value of
// makeTracked — and why you should NEVER std::move a local being returned by
// value (it interferes with elision). (notes 22.4)
TrackedBuffer::TrackedBuffer(TrackedBuffer&& other) noexcept
    : m_size { other.m_size }
    , m_data { other.m_data }
{
    other.m_size = 0;
    other.m_data = nullptr;

    ++s_moves;
}

// ── TASK 3: Copy assignment ───────────────────────────────────────────────────
// Unlike the copy constructor, the destination *already* owns resources —
// we must RELEASE them before taking on the new ones.
//
// SELF-ASSIGNMENT: `b = b` or `b = someRef` where someRef refers to b. If we
// deleted m_data first and then tried to copy from *this, we'd be reading freed
// memory. Always detect and bail early.
//
// STEPS:
//   1. Guard: if (this == &other) return *this;
//   2. Release: delete[] m_data;
//   3. Deep-copy: allocate + copy_n (same as the copy constructor).
//   4. Increment s_copies; return *this.
TrackedBuffer& TrackedBuffer::operator=(const TrackedBuffer& other)
{
    if (this == &other)   // self-assignment guard
        return *this;

    delete[] m_data;      // release current resource (Chapter 19 discipline)

    m_size = other.m_size;
    m_data = (m_size > 0) ? new int[m_size]{} : nullptr;

    if (m_size > 0)
        std::copy_n(other.m_data, m_size, m_data);

    ++s_copies;
    return *this;
}

// ── TASK 4: Move assignment ────────────────────────────────────────────────────
// Combines the ideas of copy assignment (must release current resources) and
// the move constructor (steal instead of copy).
//
// ORDER MATTERS:
//   1. Self-move guard: if (this == &other) return *this;
//      `b = std::move(b)` must leave b valid (not crashed). It is unusual but
//      defined to be legal and must be safe. (notes 22.3)
//   2. Release: delete[] m_data — free the resource we currently own.
//   3. Steal: copy pointer + size from other.
//   4. Zero source: null out other.m_data, zero other.m_size.
//   5. Increment s_moves; return *this.
//
// noexcept: same reason as the move constructor.
TrackedBuffer& TrackedBuffer::operator=(TrackedBuffer&& other) noexcept
{
    if (this == &other)   // self-move guard
        return *this;

    delete[] m_data;      // release current resource

    m_size = other.m_size;  // steal
    m_data = other.m_data;

    other.m_size = 0;    // zero source so its dtor is safe
    other.m_data = nullptr;

    ++s_moves;
    return *this;
}

// ── TASK 5: makeTracked factory ───────────────────────────────────────────────
// Factory pattern: the caller receives a fully-initialised TrackedBuffer ALREADY
// wrapped in a unique_ptr. They never touch `new` or `delete` directly.
//
// std::make_unique<TrackedBuffer>(size):
//   - allocates sizeof(TrackedBuffer) bytes on the heap,
//   - constructs a TrackedBuffer(size) in that memory,
//   - wraps the result in a std::unique_ptr<TrackedBuffer>,
//   - returns it (by value — eligible for copy elision, no overhead).
//
// After this function returns, the caller holds EXCLUSIVE OWNERSHIP.
// When their unique_ptr goes out of scope (or is reset), the TrackedBuffer is
// automatically deleted — no leak possible. (notes 22.5)
std::unique_ptr<TrackedBuffer> makeTracked(std::size_t size)
{
    return std::make_unique<TrackedBuffer>(size);
}

// ── TASK 6: takeOwnership ─────────────────────────────────────────────────────
// Accepting a unique_ptr by VALUE is the idiom for "this function takes ownership."
// The caller MUST write:  takeOwnership(std::move(p));
// If they forget std::move, the copy constructor fires — but unique_ptr DELETES
// its copy constructor (unique ownership), so it is a COMPILE ERROR.
// std::move is simply a cast; it does not move by itself. (notes 22.4)
//
// After the function returns, `p` goes out of scope and ~unique_ptr() fires,
// which calls delete on the TrackedBuffer. No manual delete needed.
void takeOwnership(std::unique_ptr<TrackedBuffer> p)
{
    if (p && !p->empty())
        (*p)[0] = 42;
    // p destructs here — TrackedBuffer is deleted automatically.
}
