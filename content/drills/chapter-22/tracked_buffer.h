// Chapter 22 — Move Semantics & Smart Pointers · Project: TrackedBuffer
// ─────────────────────────────────────────────────────────────────────────────
// This header is the CONTRACT between you and the grader.
// It declares the TrackedBuffer class and two free functions. Do NOT edit it —
// tests/tests.cpp includes it, and so do BOTH starter/tracked_buffer.cpp (yours)
// and solution/tracked_buffer.cpp (the reference). Change a signature and the
// grader breaks.
//
// THE BIG IDEA OF THIS LAB
// A raw new[]-owning class (Chapter 19 payoff) is the perfect host for move
// semantics because moves are so much cheaper than copies: a move steals the
// pointer instead of allocating a fresh array and copying every element.
//
// TrackedBuffer *instruments* its special members with static counters so you
// can observe exactly which operations fire — and prove (via the tests) that
// std::move triggers the move path, NOT the copy path.
//
// This mirrors real C++: owning, resource-holding types in LLVM are routinely
// move-only. `llvm::MemoryBuffer`, for example, deletes its copy constructor and
// is handed around inside a `std::unique_ptr<MemoryBuffer>` — ownership moves,
// never copies, so a large file buffer is never silently deep-copied. The Rule of
// Five (and move-only types) is the idiom behind that discipline. (notes 22.3)
//
// Header guard (Chapter 2): prevents this file being included twice.
#ifndef TRACKED_BUFFER_H
#define TRACKED_BUFFER_H

#include <cstddef>   // std::size_t
#include <memory>    // std::unique_ptr, std::make_unique (notes 22.5)

// ─── TrackedBuffer ───────────────────────────────────────────────────────────
// An RAII class that OWNS a new[]-allocated int array.
// Instruments every special member function with static counters so tests can
// observe exactly how many copies and moves occurred.
//
// Rule of Five (notes 22.3): because we own a raw pointer we MUST provide all
// five special members — destructor, copy ctor, copy assignment, move ctor,
// move assignment — or let the compiler do the wrong thing (shallow copy).
class TrackedBuffer
{
public:
    // ── Static observability counters ────────────────────────────────────────
    // These let the grader ask "how many copies / moves happened?"
    // They are separate from instance state so tests can reset and re-query.
    static int s_copies;   // incremented inside the copy ctor and copy assignment
    static int s_moves;    // incremented inside the move ctor and move assignment

    // Reset both counters to zero.
    static void resetCounters();

    // ── Constructors ─────────────────────────────────────────────────────────

    // Default / size constructor.
    // Allocates an array of `size` default-initialised ints (all zero).
    // A size of 0 is valid: m_data stays nullptr, nothing is allocated.
    // (Provided — not a task.)
    explicit TrackedBuffer(std::size_t size = 0);

    // Copy constructor (TASK 1 — deep copy).
    // Constructs *this as an independent copy of `other`.
    // Must allocate its OWN array and copy every element.
    // Must increment s_copies.
    TrackedBuffer(const TrackedBuffer& other);

    // Move constructor (TASK 2 — steal pointer, leave source empty).
    // Constructs *this by STEALING other.m_data and other.m_size.
    // Must set other.m_data = nullptr and other.m_size = 0 afterward.
    // Must increment s_moves.
    // noexcept: move ops are marked noexcept so std::vector can prefer them
    //           during reallocation (notes 22.3).
    TrackedBuffer(TrackedBuffer&& other) noexcept;

    // ── Assignment operators ──────────────────────────────────────────────────

    // Copy assignment (TASK 3).
    // Overwrites *this with an independent copy of `other`.
    // Must handle self-assignment safely.
    // Must increment s_copies.
    TrackedBuffer& operator=(const TrackedBuffer& other);

    // Move assignment (TASK 4).
    // Overwrites *this by STEALING other's resources.
    // Must release *this's current array first (no leak).
    // Must handle self-move-assignment safely (this == &other).
    // Must increment s_moves.
    // noexcept: same reason as the move constructor.
    TrackedBuffer& operator=(TrackedBuffer&& other) noexcept;

    // ── Destructor ────────────────────────────────────────────────────────────
    // Releases m_data with delete[]. (Provided — not a task.)
    ~TrackedBuffer();

    // ── Observers ────────────────────────────────────────────────────────────
    // (Provided — no tasks here. Read them to understand the moved-from state.)

    std::size_t size() const { return m_size; }

    // True when this buffer owns no array (size == 0, m_data == nullptr).
    bool empty() const { return m_size == 0; }

    // Element access — undefined behaviour if i >= m_size.
    int  operator[](std::size_t i) const { return m_data[i]; }
    int& operator[](std::size_t i)       { return m_data[i]; }

    // Raw pointer — for testing moved-from state (nullptr after a move).
    const int* data() const { return m_data; }

private:
    std::size_t m_size {};   // number of elements owned
    int*        m_data {};   // owned array; nullptr when empty
};

// ─── Free functions ───────────────────────────────────────────────────────────

// TASK 5 — factory returning a unique_ptr<TrackedBuffer>.
// Allocate a TrackedBuffer of `size` elements on the heap, wrap it in a
// std::unique_ptr, and return it. Caller receives exclusive ownership.
// Use std::make_unique (notes 22.5).
std::unique_ptr<TrackedBuffer> makeTracked(std::size_t size);

// TASK 6 — take ownership through a unique_ptr parameter.
// Accept ownership of `p` by value. Inside, write the first element to 42
// if the buffer is non-empty. p is destroyed when this function returns —
// demonstrating that the unique_ptr's destructor cleans up automatically.
// The CALLER must std::move their local unique_ptr into this call; passing
// without std::move is a compile error (copy of unique_ptr is deleted).
void takeOwnership(std::unique_ptr<TrackedBuffer> p);

#endif // TRACKED_BUFFER_H
