// Chapter 15 — More on Classes · Project: IdCard Badge Printer    (STARTER)
// ─────────────────────────────────────────────────────────────────────────────
// Your job: fill in the five TASK blocks below. Each maps 1:1 to a task in
// README.md and to a declaration in ../idcard.h.
//
// The bodies currently return PLACEHOLDERS so the file compiles out of the box —
// that's why `make test` is RED right now. Turn it GREEN.
//
//   make build        compile-check your code   (should already work)
//   make test         grade it                  (RED until you fill the TASKs)
//   make test-solution  see what green looks like (uses the reference)
//
// SYNTAX reminder — out-of-class member definitions use the scope-resolution
// operator (::) to tell the compiler which class each body belongs to:
//
//     ReturnType ClassName::methodName(params) { … }
//
// That :: prefix is the whole point of notes 15.2. The compiler now links this
// definition to the declaration it found in idcard.h.
//
// STYLE RULE from notes 15.2: default arguments are declared ONCE in the header.
// Do NOT repeat them in any definition below (the compiler will reject it).
// ─────────────────────────────────────────────────────────────────────────────

#include "../idcard.h"    // the class declaration + includes for std::string etc.
#include <string>
#include <string_view>

// ─── TASK 1: Constructor ─────────────────────────────────────────────────────
// Initialise m_owner and m_title from the parameters using a MEMBER-INITIALISER
// LIST (the `:` syntax from Chapter 14). Then, inside the body:
//   • Assign the current value of s_nextSerial to m_serial.
//   • Increment s_nextSerial so the NEXT card gets a different number.
//   • Increment s_liveCount by 1 (this card is now alive).
//
// Note: m_role already has a default (Role::Other) set in the class definition —
// you do NOT need to initialise it here unless you want to change it.
//
// Syntax hint:  IdCard::IdCard(std::string_view owner, std::string_view title)
//                   : m_owner { owner }, m_title { title }
//               { … }
//
//   >>> YOUR CODE HERE <<<
//
IdCard::IdCard(std::string_view /*owner*/, std::string_view /*title*/)
{
    // placeholder — s_liveCount is NOT incremented, serials NOT assigned.
    // Touch m_serial and m_role so the compiler doesn't warn about unused fields.
    m_serial = 0;
    m_role   = Role::Other;
}
// ─────────────────────────────────────────────────────────────────────────────

// ─── TASK 2: Destructor ──────────────────────────────────────────────────────
// The destructor runs AUTOMATICALLY when an IdCard object's lifetime ends —
// either when it goes out of scope, or when execution returns from the block
// that holds it. The tests create cards in nested { } scopes and verify that
// s_liveCount falls exactly as objects are destroyed (notes 15.4).
//
// Your only job: decrement s_liveCount by 1.
//
//   >>> YOUR CODE HERE <<<
//
IdCard::~IdCard()
{
    // placeholder — s_liveCount is NOT decremented
}
// ─────────────────────────────────────────────────────────────────────────────

// ─── TASK 3: setOwner and setTitle — explicit `this` + method chaining ───────
// Each setter:
//   1. Writes `this->m_xxx = xxx;` — the explicit-`this` form (notes 15.1).
//      A clarification, NOT a contradiction: because this class prefixes members
//      with `m_`, the parameter `owner` and the member `m_owner` are DIFFERENT
//      names, so plain `m_owner = owner;` is already unambiguous — `this->` is
//      never strictly required here. We still write `this->m_owner = owner;` to
//      make the intent explicit ("the member of THIS object") and to practice the
//      exact idiom notes 15.1 demonstrates with `Person::setName`. (The one time
//      `this->` becomes mandatory is when a parameter has the SAME name as a
//      member — e.g. a parameter literally named `m_owner`, or a member without
//      the `m_` prefix. We avoid that here on purpose.)
//   2. Returns `*this` (the object itself, by reference) so calls can be chained:
//          card.setOwner("Ada").setTitle("Engineer")
//      `this` is a POINTER; `*this` dereferences it to get the object.
//      The return type IdCard& (reference!) ensures no copy is made.
//
// Please use these EXACT parameter names in your definitions (they are what the
// hints and the notes-15.1 idiom use):
//   setOwner(std::string_view owner)   — pairs with member m_owner
//   setTitle(std::string_view title)   — pairs with member m_title
//   setRole(IdCard::Role role)         — pairs with member m_role
//
//   >>> YOUR CODE HERE <<<
//
IdCard& IdCard::setOwner(std::string_view /*owner*/)
{
    return *this;   // placeholder — does NOT update m_owner
}

IdCard& IdCard::setTitle(std::string_view /*title*/)
{
    return *this;   // placeholder — does NOT update m_title
}

IdCard& IdCard::setRole(IdCard::Role /*role*/)
{
    return *this;   // placeholder — does NOT update m_role
}
// ─────────────────────────────────────────────────────────────────────────────

// ─── TASK 4: Const accessors ─────────────────────────────────────────────────
// Simple read-only getters. Each must be marked `const` (matching the declaration
// in the header) so it can be called on a const IdCard object.
// Return the matching private member value.
//
//   >>> YOUR CODE HERE <<<
//
std::string_view IdCard::owner() const
{
    return {};   // placeholder — returns empty string_view
}

std::string_view IdCard::title() const
{
    return {};   // placeholder — returns empty string_view
}

IdCard::Role IdCard::role() const
{
    return IdCard::Role::Other;   // placeholder — always Other
}

int IdCard::serial() const
{
    return 0;   // placeholder — always returns 0 (wrong; serials are 1-based)
}
// ─────────────────────────────────────────────────────────────────────────────

// ─── TASK 5: Static member functions ─────────────────────────────────────────
// Static member functions belong to the CLASS, not to any particular object.
// They have NO `this` pointer (notes 15.7), so they can only access:
//   • static data members of the class (s_liveCount, s_nextSerial), and
//   • local variables / parameters.
// They CANNOT access non-static members (m_owner, m_title, …) without an object.
//
// liveCount()  — return the current value of s_liveCount.
// nextSerial() — return the current value of s_nextSerial (the value the NEXT
//                constructed card will receive — useful for tests that predict it).
//
// Syntax: the `static` keyword appears in the DECLARATION (in .h) only.
// Do NOT write `static` in the DEFINITION (in .cpp). This is the same rule as
// for virtual functions in later chapters (a preview — formally Chapter 25).
//
//   >>> YOUR CODE HERE <<<
//
int IdCard::liveCount()
{
    return 0;   // placeholder — ignores s_liveCount
}

int IdCard::nextSerial()
{
    return 0;   // placeholder — ignores s_nextSerial
}
// ─────────────────────────────────────────────────────────────────────────────
