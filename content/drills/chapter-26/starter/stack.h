// Chapter 26 — Templates and Classes · Project: Stack<T>   (STARTER)
// ─────────────────────────────────────────────────────────────────────────────
// Fill in the four TASK blocks below. They map 1:1 to the tasks in the README
// and to the CHECKs in tests/tests.cpp. Every body currently compiles but
// returns PLACEHOLDER values so `make test` is RED. Your job: turn it GREEN.
//
//     make build          compile-check this header (should already work)
//     make test           grade your code (RED until the TASKs are filled in)
//     make solution       see the reference if you're truly stuck
//
// ── WHY HEADER-ONLY? ─────────────────────────────────────────────────────────
// This is the CENTRAL lesson of Chapter 26 (notes 26.1).  When a translation
// unit writes  Stack<int>  the compiler must see the FULL class template
// definition RIGHT THERE so it can stamp out a concrete Stack<int>.  If the
// member bodies lived in a .cpp file the linker would say "undefined symbol
// Stack<int>::push" — it never got the chance to instantiate them.
//
// Practical rule (notes 26.1):
//   "For class templates, put the class definition and member definitions in the
//    header, or put member definitions in an included .inl file at the bottom."
//
// This file is that header. Everything — data layout, in-class members, and
// the one out-of-class member (Task 3) — lives here.
// ─────────────────────────────────────────────────────────────────────────────

#ifndef STACK_H
#define STACK_H

#include <array>       // std::array — Ch 17 fixed-size array
#include <cassert>     // assert — Ch 9 precondition enforcement
#include <cstddef>     // std::size_t
#include <string_view> // std::string_view — for TypeLabel (Ch 5)

// ─────────────────────────────────────────────────────────────────────────────
// TASK 1: Write the Stack class template
// ─────────────────────────────────────────────────────────────────────────────
// Design
// ──────
// Stack is a **class template** (notes 26.1) and uses a **non-type template
// parameter** for the capacity (notes 26.2). Its backing storage is a fixed-
// size std::array<T, Capacity> (Ch 17) — zero dynamic allocation, just like
// llvm::SmallVector's inline buffer.
//
// Signature to implement (replace the placeholder class below):
//
//   template <typename T, int Capacity = 8>
//   class Stack { ... };
//
// Required PUBLIC member functions (all defined IN-CLASS except pop — see
// Task 3 for the out-of-class syntax practice):
//
//   bool push(const T& value)
//       Add `value` on top. Return false if already full, true on success.
//
//   T top() const
//       Return the top element WITHOUT removing it.
//       Precondition: stack is not empty — enforce with assert (Ch 9).
//
//   int size() const
//       Return the number of elements currently on the stack.
//
//   bool isEmpty() const
//       Return true when size() == 0.
//
//   bool isFull() const
//       Return true when size() == Capacity.
//
//   void pop()                    <- DECLARE here; DEFINE in Task 3 (below class)
//       Remove the top element.
//       Precondition: stack is not empty — enforce with assert.
//
// Private data:
//   std::array<T, Capacity> m_data {};   // the fixed-size backing store
//   int                     m_size {};   // how many elements are live
//
// Key term: the TEMPLATE PARAMETER LIST  template <typename T, int Capacity = 8>
//   T        — a TYPE parameter (placeholder for int, std::string, …)
//   Capacity — a NON-TYPE parameter (a compile-time int value, default 8)
//
// Constraint: all member function bodies except pop go IN-CLASS (inside the
// class body { }). Only pop is declared in-class and DEFINED below (Task 3).
//
// Hint: inside the class body you may refer to the class simply as Stack;
// outside (Task 3) you must write Stack<T, Capacity>.
//
// ─── TASK 1 ──────────────────────────────────────────────────────────────────
//
//   >>> YOUR CODE HERE <<<
//
// ─── placeholder — correct template signature, wrong bodies ──────────────────
// The class is already a template (so Stack<int> and Stack<std::string> parse)
// but every body returns a WRONG placeholder — that is why make test is RED.
// Replace this entire template with a correct implementation.
template <typename T, int Capacity = 8>
class Stack
{
public:
    bool push(const T& /*value*/) { return false; }  // placeholder: never pushes
    T    top()    const           { return T{};   }  // placeholder: default-constructed
    int  size()   const           { return 0;     }  // placeholder: always 0
    bool isEmpty() const          { return true;  }  // placeholder: always empty
    bool isFull()  const          { return true;  }  // placeholder: always full
    void pop()                    {}                 // placeholder: does nothing
};
// ─────────────────────────────────────────────────────────────────────────────


// ─────────────────────────────────────────────────────────────────────────────
// TASK 2: Add the TypeLabel primary template
// ─────────────────────────────────────────────────────────────────────────────
// A TRAIT-style template tells us the human-readable name of a type at compile
// time — exactly the kind of helper found inside LLVM's type system utilities.
//
// Write this primary template:
//
//   template <typename T>
//   struct TypeLabel
//   {
//       static constexpr std::string_view name { "unknown" };
//   };
//
// "Primary template" (notes 26.4): the catch-all definition the compiler uses
// when no specialization matches. Any type with no explicit specialization will
// have TypeLabel<T>::name == "unknown".
//
// ─── TASK 2 ──────────────────────────────────────────────────────────────────
//
//   >>> YOUR CODE HERE <<<
//
// ─── placeholder — wrong name for every type ─────────────────────────────────
template <typename T>
struct TypeLabel
{
    // placeholder: all types get "unknown" (correct for unspecialized types, but
    // the specialized ones like int, double, string_view also need a name — see Task 4).
    static constexpr std::string_view name { "unknown" };
};
// ─────────────────────────────────────────────────────────────────────────────


// ─────────────────────────────────────────────────────────────────────────────
// TASK 3: Define Stack<T, Capacity>::pop OUT-OF-CLASS
// ─────────────────────────────────────────────────────────────────────────────
// Pop is DECLARED inside the Stack class (Task 1) but DEFINED here, below the
// class body.  This practices the out-of-class member syntax (notes 26.1):
//
//   template <typename T, int Capacity>
//   void Stack<T, Capacity>::pop()
//   {
//       assert(!isEmpty());     // precondition (Ch 9)
//       --m_size;               // "removing" the top is just shrinking the live count
//   }
//
// Two required pieces (notes 26.1 / 26.2):
//   template <typename T, int Capacity>   <- repeat the FULL template parameter list
//   Stack<T, Capacity>::pop               <- qualify with the instantiation pattern
//
// Do NOT write just  Stack::pop  — outside the class body the compiler needs
// the full  Stack<T, Capacity>  qualification to find the right template.
//
// NOTE: Once you implement the real Stack in Task 1 (with m_size / m_data),
// replace the placeholder pop body below with the real one too.
// Currently it is left as an empty body so the file compiles with the stub class.
//
// ─── TASK 3 ──────────────────────────────────────────────────────────────────
//
//   >>> YOUR CODE HERE <<<
//
// ─── placeholder out-of-class definition (empty body — wrong behavior) ────────
// (Once you add m_size in Task 1, replace this with the real implementation.)
// ─────────────────────────────────────────────────────────────────────────────


// ─────────────────────────────────────────────────────────────────────────────
// TASK 4: Full specializations of TypeLabel
// ─────────────────────────────────────────────────────────────────────────────
// Add FULL SPECIALIZATIONS (notes 26.4) so common types report their proper
// names instead of "unknown".  The syntax:
//
//   template <>                          // no remaining template parameters
//   struct TypeLabel<int>                // specialization for T = int exactly
//   {
//       static constexpr std::string_view name { "int" };
//   };
//
// Required specializations (one for each):
//   TypeLabel<int>              -> name == "int"
//   TypeLabel<double>           -> name == "double"
//   TypeLabel<std::string_view> -> name == "string_view"
//
// Key terms (notes 26.4):
//   - "Full specialization" — ALL template parameters are fixed (contrast with
//     partial specialization, which the notes cover in 26.5).
//   - The PRIMARY template (Task 2) must be defined BEFORE any specialization.
//
// Constraint: do NOT change the primary template. Add three separate
//   template <> struct TypeLabel<XYZ> { ... };  blocks below.
//
// ─── TASK 4 ──────────────────────────────────────────────────────────────────
//
//   >>> YOUR CODE HERE <<<
//
// ─────────────────────────────────────────────────────────────────────────────

#endif // STACK_H
