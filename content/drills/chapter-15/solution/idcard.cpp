// Chapter 15 — More on Classes · Project: IdCard Badge Printer   (REFERENCE SOLUTION)
// ─────────────────────────────────────────────────────────────────────────────
// One complete, correct, warning-clean implementation of ../idcard.h.
// Peek only after you've taken a real swing at starter/idcard.cpp — the
// learning is in writing the out-of-class bodies yourself.
//
// KEY PATTERNS this solution demonstrates (map to the notes' sections):
//
//   notes 15.1  — explicit `this->member = param;` idiom (clarity; not required
//                 here because members are `m_`-prefixed — see setOwner below)
//   notes 15.1  — return *this by reference for method chaining
//   notes 15.2  — ClassName::method(...) syntax for out-of-class definitions
//   notes 15.4  — destructor decrements a class-wide counter (RAII preview)
//   notes 15.6  — static member variables s_liveCount, s_nextSerial
//   notes 15.7  — static member functions liveCount(), nextSerial() — no `this`
//
// CS6340 note: LLVM's Pass infrastructure uses a near-identical pattern for
// counting registered passes and issuing unique IDs — static counters + inline
// static class-level storage. See llvm/IR/PassInstrumentation.h for the spirit.
// ─────────────────────────────────────────────────────────────────────────────

#include "../idcard.h"
#include <string>
#include <string_view>

// ─── TASK 1: Constructor (notes 15.2 — out-of-class definition) ──────────────
// We initialise m_owner and m_title from the parameters in the member-initialiser
// list (the `: m_owner{owner}, m_title{title}` part). The body then:
//   • Claims the next serial number (read THEN increment s_nextSerial).
//   • Increments s_liveCount so the object is counted as alive immediately.
//
// NOTE: default arguments ("Unnamed", "Untitled") are declared ONCE in the
// header — they MUST NOT appear again here. This is notes 15.2's rule.
IdCard::IdCard(std::string_view owner, std::string_view title)
    : m_owner { owner }    // initialise member from parameter (Chapter 14)
    , m_title { title }
{
    // Claim this card's unique serial: read the class-wide counter, then advance.
    // s_nextSerial is a static member — it belongs to the class, not this object.
    m_serial = s_nextSerial;   // the value THIS card gets
    ++s_nextSerial;            // the NEXT card will get a different number

    // Register that one more IdCard is alive. The destructor undoes this.
    ++s_liveCount;
}

// ─── TASK 2: Destructor (notes 15.4 — RAII / deterministic cleanup) ──────────
// The destructor name is ~ClassName. No return type, no parameters.
//
// When an IdCard goes out of scope (or is otherwise destroyed), C++ calls this
// automatically — deterministically, at a known program point, not "eventually".
// That determinism is what makes the tests' nested-scope trick reliable: create
// cards inside a { } block, leave the block, and s_liveCount falls predictably.
//
// In a real class, the destructor might close a file handle or release a lock.
// Here it simply bookkeeps — but the RAII lesson is the same.
IdCard::~IdCard()
{
    --s_liveCount;   // one fewer live IdCard in the program
}

// ─── TASK 3a: setOwner (notes 15.1 — explicit `this`, return *this) ──────────
// The parameter `owner` pairs with the private member m_owner. Because of the
// `m_` prefix they are DIFFERENT names, so `m_owner = owner` is already
// unambiguous — the compiler resolves m_owner as the member with no help needed.
// We still write `this->m_owner = owner` to make the intent explicit: "I mean the
// member of THIS object." `this->` would be strictly REQUIRED only if a parameter
// shared a member's exact name. The notes' Person::setName shows this idiom (15.1).
//
// Returning `*this` by reference (IdCard&) means the caller gets back a reference
// to the same object, not a copy — so the next chained call operates on the same
// card.
IdCard& IdCard::setOwner(std::string_view owner)
{
    this->m_owner = owner;   // explicit-`this` idiom (optional here: m_owner != owner)
    return *this;            // return the object itself, by reference
}

// ─── TASK 3b: setTitle (same pattern as setOwner) ────────────────────────────
IdCard& IdCard::setTitle(std::string_view title)
{
    this->m_title = title;
    return *this;
}

// ─── TASK 3c: setRole (same chaining idiom) ──────────────────────────────────
// Note the fully-qualified parameter type: IdCard::Role. Outside the class body,
// `Role` alone is not in scope — we must write `IdCard::Role`.
IdCard& IdCard::setRole(IdCard::Role role)
{
    this->m_role = role;
    return *this;
}

// ─── TASK 4: Const accessors ─────────────────────────────────────────────────
// Plain read-only getters. The `const` qualifier on the function (after the `()`)
// matches the declaration in the header and tells the compiler this function does
// not modify the object — so it can be called on const IdCard objects.
//
// std::string_view is non-owning: it points into the underlying std::string's
// buffer. That is safe as long as the IdCard outlives the caller's use of the view
// — fine here, since tests read the view before modifying the card again.
std::string_view IdCard::owner() const { return m_owner; }
std::string_view IdCard::title() const { return m_title; }
IdCard::Role     IdCard::role()  const { return m_role;  }
int              IdCard::serial() const { return m_serial; }

// ─── TASK 5a: liveCount (notes 15.7 — static member function) ────────────────
// Static member functions have NO implicit object — no `this` pointer. They can
// only touch static members (s_liveCount, s_nextSerial) or things passed in.
//
// IMPORTANT: `static` appears in the DECLARATION (in .h), NOT in the definition
// here. Writing `static int IdCard::liveCount()` is a compiler error. (The same
// rule will apply to `virtual` — a preview, formally Chapters 24–25 — which also
// goes only on the in-class declaration, never on the out-of-class definition.)
int IdCard::liveCount()
{
    return s_liveCount;   // class-wide; not tied to any one object
}

// ─── TASK 5b: nextSerial ─────────────────────────────────────────────────────
// Lets tests predict what serial the next-constructed card will get, enabling
// precise assertions without hard-coding absolute serial values.
int IdCard::nextSerial()
{
    return s_nextSerial;
}
