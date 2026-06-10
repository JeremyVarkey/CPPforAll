// Chapter 26 — Templates and Classes · Project: Stack<T>   (REFERENCE SOLUTION)
// ─────────────────────────────────────────────────────────────────────────────
// Complete, correct, warning-clean reference for chapter-26/starter/stack.h.
// Peek only after taking a real swing at the starter — the learning is in
// wrestling with the template syntax yourself, then comparing here.
//
// ── WHY HEADER-ONLY? (notes 26.1) ────────────────────────────────────────────
// When a translation unit writes  Stack<int>  the compiler must see the FULL
// template definition at that point so it can stamp out a concrete Stack<int>.
// If the member bodies lived in a separate .cpp, that .cpp would compile but
// never produce a Stack<int> instantiation — the linker would then fail with
// "undefined symbol Stack<int>::push" because no translation unit ever asked
// for it while the definition was visible.
//
// Both in-class and the out-of-class pop definition therefore live here in the
// header.  That makes this file a worked example of the constraint the chapter
// exists to teach.
// ─────────────────────────────────────────────────────────────────────────────

#ifndef STACK_H
#define STACK_H

#include <array>        // std::array — Ch 17 fixed-size array
#include <cassert>      // assert     — Ch 9 precondition enforcement
#include <cstddef>      // std::size_t
#include <string_view>  // std::string_view — for TypeLabel labels (Ch 5)

// ─── TASK 1 + TASK 3: Stack class template (declaration + out-of-class pop) ──
//
// KEY TERM: CLASS TEMPLATE (notes 26.1)
//   template <typename T, int Capacity = 8>
//   class Stack { … };
//
//   T        is a TYPE parameter — the kind of value the stack holds.
//   Capacity is a NON-TYPE parameter (notes 26.2) — a compile-time integer
//            that fixes the array size.  Default is 8 (an arbitrary small
//            bound — the same idea as the inline-capacity tuning value in
//            llvm::SmallVector<T, N>, explained two lines down).
//
// Why a non-type param here?  Because std::array<T, Capacity> needs Capacity
// as a compile-time constant.  A runtime constructor argument would require
// dynamic allocation (Ch 19), defeating the "zero-allocation bounded stack"
// goal. This is exactly the trade-off llvm::SmallVector makes: the inline-
// buffer size is a non-type param so it can be an actual array on the stack.

template <typename T, int Capacity = 8>
class Stack
{
    // ── private data ────────────────────────────────────────────────────────
    // m_data: the fixed backing store.  We never resize it; m_size tracks how
    // many elements are "live" (i.e. pushed but not yet popped).
    std::array<T, Capacity> m_data {};  // value-initialized (zeros / default ctors)
    int                     m_size {};  // 0 … Capacity

public:
    // ── push ────────────────────────────────────────────────────────────────
    // KEY DESIGN CHOICE: push returns bool instead of throwing (Ch 27 style)
    // or asserting.  The caller decides what "full" means for its use case.
    // Return false when the stack is already full so the caller can react
    // (e.g., a fuzzer queue might flush and retry rather than crash).
    bool push(const T& value)
    {
        if (isFull())
            return false;           // precondition not met — caller must handle

        m_data[static_cast<std::size_t>(m_size)] = value;
        ++m_size;
        return true;
    }

    // ── top ─────────────────────────────────────────────────────────────────
    // Returns the top element WITHOUT removing it.  Calling top() on an empty
    // stack is undefined behaviour in std::stack, so we enforce the precondition
    // with assert (Ch 9) — fast in debug, zero-cost in release.
    T top() const
    {
        assert(!isEmpty() && "Stack::top called on empty stack");
        return m_data[static_cast<std::size_t>(m_size - 1)];
    }

    // ── size / isEmpty / isFull ──────────────────────────────────────────────
    // These are small enough that keeping them in-class is normal and clear.
    int  size()    const { return m_size;               }
    bool isEmpty() const { return m_size == 0;           }
    bool isFull()  const { return m_size == Capacity;    }

    // ── pop (declaration only — DEFINED BELOW the class) ────────────────────
    // TASK 3 teaches the out-of-class member definition syntax.  The body is
    // deliberately defined below so you can see the template<typename T, int
    // Capacity> Stack<T, Capacity>::pop() pattern in isolation, exactly as
    // notes 26.1 and 26.2 describe.
    void pop();
};

// ─── TASK 3: out-of-class member definition ──────────────────────────────────
//
// KEY SYNTAX (notes 26.1 / 26.2):
//
//   template <typename T, int Capacity>      ← REPEAT the full parameter list
//   void Stack<T, Capacity>::pop()           ← qualify with BOTH params
//   { … }
//
// Inside the class body you can just write Stack; outside it the compiler needs
// the full Stack<T, Capacity> to disambiguate (there are infinitely many
// instantiations; the "which Stack" must be spelled out).
//
// Compare to a non-template:
//   int Counter::value() const { … }         ← just "Counter", no template head
//
// Adding "template <typename T, int Capacity>" and changing "Stack" to
// "Stack<T, Capacity>" is the only extra work class templates add to out-of-
// class definitions.  Once you've written it once the pattern sticks.
//
template <typename T, int Capacity>
void Stack<T, Capacity>::pop()
{
    assert(!isEmpty() && "Stack::pop called on empty stack");
    --m_size;   // the element at m_data[m_size] is now "above" the live region;
                // it will be overwritten the next time push() lands there.
}


// ─── TASK 2: TypeLabel primary template ──────────────────────────────────────
//
// KEY TERM: TRAIT (notes 26.4 reading between lines)
// A "trait" is a class template whose sole purpose is to carry compile-time
// information ABOUT a type.  TypeLabel is a minimal trait: given any type T,
// TypeLabel<T>::name is a human-readable string_view known at compile time.
//
// The canonical standard-library example of this exact pattern is
// std::numeric_limits<T> — a class template fully specialized per arithmetic
// type to expose compile-time facts about it (e.g. std::numeric_limits<int>::max()).
// std::iterator_traits<T> is another.  LLVM uses the same shape in real trait
// templates such as llvm::format_provider<T> (specialized per type to teach the
// formatting library how to print that type).  The common thread: a class
// template, specialized per type, that describes a type's properties at compile
// time without altering the type itself.
//
// The PRIMARY template is the catch-all: it fires when NO specialization matches.
// This one says "I don't know what this type is → call it 'unknown'."
//
template <typename T>
struct TypeLabel
{
    static constexpr std::string_view name { "unknown" };
};


// ─── TASK 4: full specializations of TypeLabel ───────────────────────────────
//
// KEY TERM: FULL CLASS SPECIALIZATION (notes 26.4)
//   template <>
//   struct TypeLabel<int> { … };
//
// "template <>" means "all template parameters are now fixed" — no placeholders
// remain.  The angle brackets after the class name carry the specific type(s).
//
// The PRIMARY template MUST be visible before any specialization, just as it is
// here (Task 2 above, Task 4 below).
//
// Why not just an if/switch at runtime?  Because these names are constants used
// in assertions and error messages that the compiler can fold into the binary —
// zero runtime overhead, and they communicate intent clearly to readers.
//
// TypeLabel<int>  ────────────────────────────────────────────────────────────
template <>
struct TypeLabel<int>
{
    static constexpr std::string_view name { "int" };
};

// TypeLabel<double>  ─────────────────────────────────────────────────────────
template <>
struct TypeLabel<double>
{
    static constexpr std::string_view name { "double" };
};

// TypeLabel<std::string_view>  ───────────────────────────────────────────────
template <>
struct TypeLabel<std::string_view>
{
    static constexpr std::string_view name { "string_view" };
};

#endif // STACK_H
