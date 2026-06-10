// Chapter 22 — Move Semantics & Smart Pointers · Project: TrackedBuffer (STARTER)
// ─────────────────────────────────────────────────────────────────────────────
// Fill in the six TASK blocks below. Each maps 1:1 to a task in the README and
// to a declaration in ../tracked_buffer.h. The bodies currently return
// PLACEHOLDERS so the file compiles immediately — that's why `make test` is RED
// right now. Your job is to turn it GREEN.
//
//     make build           compile your code (should already work)
//     make test            grade it           (RED until you fill these in)
//     make test-solution   run grader against the reference
//
// KEY IDEAS to keep in mind while you work
// ─────────────────────────────────────────
// * DEEP COPY vs SHALLOW COPY (notes 22.1, 22.3)
//   A default compiler-generated copy would copy the POINTER (shallow), so two
//   objects would think they own the same array — double-delete UB. Deep copy
//   allocates a NEW array and copies each element.
//
// * MOVE = STEAL + ZERO (notes 22.3)
//   A move constructor / assignment STEALS m_data and m_size from the source,
//   then ZEROS them out. After a move the source is "empty" (size==0, data==nullptr),
//   so its destructor safely runs `delete[] nullptr` — which is a no-op.
//
// * noexcept on move ops (notes 22.3)
//   std::vector checks whether the move constructor is noexcept before deciding
//   to move vs copy during reallocation. Mark your move ops noexcept!
//
// * self-assignment (notes 22.3)
//   Both assignment operators must check (this == &other) and do nothing if true,
//   otherwise you'd delete your own array before reading from it.
//
// * std::unique_ptr = RAII for a heap object (notes 22.5)
//   make_unique<T>(args) allocates, constructs, and wraps in one step.
//   Passing unique_ptr by VALUE transfers ownership — the caller MUST std::move.

#include "../tracked_buffer.h"

#include <algorithm>   // std::copy_n
#include <cstddef>     // std::size_t
#include <memory>      // std::unique_ptr, std::make_unique

// ── Static counter definitions ────────────────────────────────────────────────
// Static class members need ONE definition in a .cpp file (Chapter 15 rule).
int TrackedBuffer::s_copies { 0 };
int TrackedBuffer::s_moves  { 0 };

void TrackedBuffer::resetCounters()
{
    s_copies = 0;
    s_moves  = 0;
}

// ── Provided: default / size constructor ─────────────────────────────────────
// Allocates `size` ints, zero-initialised, if size > 0. Otherwise m_data stays
// nullptr. This is the only ctor the learner does NOT implement.
TrackedBuffer::TrackedBuffer(std::size_t size)
    : m_size { size }
    , m_data { size > 0 ? new int[size]{} : nullptr }
{
}

// ── Provided: destructor ──────────────────────────────────────────────────────
// delete[] on a null pointer is defined by the C++ standard to have no effect
// ([expr.delete]), so this works for empty buffers and for buffers whose pointer
// was stolen by a move (their m_data was zeroed to nullptr). (notes 22.3)
TrackedBuffer::~TrackedBuffer()
{
    delete[] m_data;
}

// ─── TASK 1: Copy constructor (deep copy) ────────────────────────────────────
// Construct *this as an independent copy of `other`.
//
// Steps:
//   1. Set m_size = other.m_size.
//   2. Allocate a NEW array of m_size ints (use new int[m_size]{}).
//      If other is empty (m_size == 0), set m_data = nullptr instead.
//   3. Copy each element: std::copy_n(other.m_data, m_size, m_data).
//   4. Increment TrackedBuffer::s_copies.
//
// Constraint: the resulting object must own its OWN array — NOT share other's.
// Hint: allocate with `new int[m_size]{}` (value-initialises to 0, then copy).
//
//   >>> YOUR CODE HERE <<<
//
TrackedBuffer::TrackedBuffer(const TrackedBuffer& other)
    : m_size { 0 }
    , m_data { nullptr }
{
    // PLACEHOLDER — always produces an empty buffer (wrong; tests will fail).
    // Replace this body with the real deep-copy logic.
    (void)other;   // suppress unused-parameter warning on the placeholder
}
// ─────────────────────────────────────────────────────────────────────────────

// ─── TASK 2: Move constructor (steal pointer, leave source empty) ─────────────
// Construct *this by STEALING other's resources — no allocation, no copying.
//
// Steps:
//   1. Set m_size = other.m_size.
//   2. Set m_data = other.m_data.
//   3. Zero out the source: other.m_size = 0; other.m_data = nullptr.
//      (This leaves `other` in the valid "empty" state so its dtor is safe.)
//   4. Increment TrackedBuffer::s_moves.
//
// Constraint: mark noexcept — this member function must not throw. (notes 22.3)
// Hint: a move ctor is NEVER called by the compiler for prvalue returns in
// C++17 due to mandatory copy elision — only for NAMED objects wrapped in
// std::move. Never test s_moves on a prvalue return from a factory.
//
//   >>> YOUR CODE HERE <<<
//
TrackedBuffer::TrackedBuffer(TrackedBuffer&& other) noexcept
    : m_size { 0 }
    , m_data { nullptr }
{
    // PLACEHOLDER — always produces an empty buffer (wrong; tests will fail).
    (void)other;
}
// ─────────────────────────────────────────────────────────────────────────────

// ─── TASK 3: Copy assignment ──────────────────────────────────────────────────
// Overwrite *this with an independent copy of `other`.
//
// Steps:
//   1. Self-assignment guard: if (this == &other) return *this;
//   2. Release existing resources: delete[] m_data.
//   3. Deep-copy: m_size = other.m_size; m_data = new int[m_size]{} (or nullptr
//      if empty); std::copy_n(other.m_data, m_size, m_data).
//   4. Increment s_copies.
//   5. return *this;
//
//   >>> YOUR CODE HERE <<<
//
TrackedBuffer& TrackedBuffer::operator=(const TrackedBuffer& other)
{
    // PLACEHOLDER — does nothing (leaves *this unchanged, which is wrong).
    (void)other;
    return *this;
}
// ─────────────────────────────────────────────────────────────────────────────

// ─── TASK 4: Move assignment ──────────────────────────────────────────────────
// Overwrite *this by STEALING other's resources.
//
// Steps:
//   1. Self-move-assignment guard: if (this == &other) return *this;
//      (std::move on self is rare but must be safe — notes 22.3)
//   2. Release existing resources: delete[] m_data.
//   3. Steal: m_size = other.m_size; m_data = other.m_data.
//   4. Zero source: other.m_size = 0; other.m_data = nullptr.
//   5. Increment s_moves.
//   6. return *this;
//
// Constraint: mark noexcept. (notes 22.3)
//
//   >>> YOUR CODE HERE <<<
//
TrackedBuffer& TrackedBuffer::operator=(TrackedBuffer&& other) noexcept
{
    // PLACEHOLDER — does nothing (leaves *this unchanged, which is wrong).
    (void)other;
    return *this;
}
// ─────────────────────────────────────────────────────────────────────────────

// ─── TASK 5: makeTracked factory ─────────────────────────────────────────────
// Allocate a TrackedBuffer of `size` elements on the HEAP and return it wrapped
// in a std::unique_ptr<TrackedBuffer>.
//
// Use:  return std::make_unique<TrackedBuffer>(size);
//
// Why make_unique? It allocates + constructs in one step and avoids the
// "new in one expression, unique_ptr ctor in another" exception-safety hole.
// (notes 22.5: "Prefer std::make_unique")
//
//   >>> YOUR CODE HERE <<<
//
std::unique_ptr<TrackedBuffer> makeTracked(std::size_t /*size*/)
{
    return nullptr;   // PLACEHOLDER — returns an empty (null) unique_ptr
}
// ─────────────────────────────────────────────────────────────────────────────

// ─── TASK 6: takeOwnership via unique_ptr by value ────────────────────────────
// Accept ownership of `p` by value; if the buffer is non-empty, write 42 into
// element [0]. When this function returns, p goes out of scope and the
// unique_ptr destructor deletes the TrackedBuffer automatically.
//
// The call site MUST be:  takeOwnership(std::move(local_ptr));
// Trying to copy a unique_ptr is a COMPILE ERROR — unique_ptr deletes its copy
// constructor. std::move is the cast that says "I'm transferring ownership."
// (notes 22.5: "Passing unique_ptr to functions")
//
//   >>> YOUR CODE HERE <<<
//
void takeOwnership(std::unique_ptr<TrackedBuffer> /*p*/)
{
    // PLACEHOLDER — does nothing (element[0] stays 0; tests will notice).
}
// ─────────────────────────────────────────────────────────────────────────────
