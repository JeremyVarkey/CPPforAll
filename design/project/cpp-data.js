/* CPPforAll — chapter data.
   Notes are structured blocks rendered by the app:
   {t:'h2'|'p'|'lead'|'list'|'code'|'tip'|'warn'|'xref'}  */

window.CPP_CHAPTERS = [
  { n: 1,  title: "Hello, C++",                tag: "basics" },
  { n: 2,  title: "Variables & Types",         tag: "types" },
  { n: 3,  title: "Operators & Expressions",   tag: "operators" },
  { n: 4,  title: "Console Input & Output",    tag: "i/o" },
  { n: 5,  title: "Conditionals",              tag: "control flow" },
  { n: 6,  title: "Loops",                     tag: "control flow" },
  { n: 7,  title: "Functions",                 tag: "functions" },
  { n: 8,  title: "Arrays & Vectors",          tag: "containers" },
  { n: 9,  title: "Strings",                   tag: "text" },
  { n: 10, title: "Pointers & References",     tag: "memory" },   // HERO
  { n: 11, title: "Dynamic Memory",            tag: "memory" },
  { n: 12, title: "Structs",                   tag: "data" },
  { n: 13, title: "Enumerations",              tag: "data" },
  { n: 14, title: "Classes & Objects",         tag: "oop" },
  { n: 15, title: "Constructors & Destructors",tag: "oop" },
  { n: 16, title: "Encapsulation",             tag: "oop" },
  { n: 17, title: "Operator Overloading",      tag: "oop" },
  { n: 18, title: "Inheritance",               tag: "oop" },
  { n: 19, title: "Polymorphism",              tag: "oop" },
  { n: 20, title: "Templates",                 tag: "generics" },
  { n: 21, title: "The Standard Library",      tag: "stl" },
  { n: 22, title: "Iterators & Algorithms",    tag: "stl" },
  { n: 23, title: "Lambdas",                   tag: "functional" },
  { n: 24, title: "Smart Pointers",            tag: "memory" },
  { n: 25, title: "Move Semantics",            tag: "performance" },
  { n: 26, title: "Exceptions",                tag: "errors" },
  { n: 27, title: "File I/O",                  tag: "i/o" },
  { n: 28, title: "Concurrency Basics",        tag: "threads" },
];

/* ---- Fleshed-out hero chapter: Pointers & References ---- */
window.CPP_HERO = {
  n: 10,
  title: "Pointers & References",
  tag: "memory",
  kicker: "Chapter 10 · Memory",
  readingTime: "11 min read",
  notes: [
    { t: "lead", html: "Every value your program touches lives somewhere in memory. A <strong>pointer</strong> is a variable that holds the address of that somewhere — and once you can name an address, you can share data without copying it, build structures that grow, and talk directly to the machine." },

    { t: "h2", text: "Memory is just a long street of boxes" },
    { t: "p", html: "Picture memory as a row of numbered mailboxes. Each <code>int</code> you declare reserves a box and writes a value inside it. The <em>number on the box</em> is its address. Normally you never see that number — you use the variable name and the compiler does the bookkeeping." },
    { t: "p", html: "A pointer lets you hold the number itself. The <code>&amp;</code> operator reads an address (\u201caddress&nbsp;of\u201d), and the <code>*</code> operator follows an address back to its value (\u201cdereference\u201d)." },
    { t: "code", lang: "cpp", file: "addresses.cpp", code:
`int score = 42;        // a box holding 42
int* p = &score;       // p holds the ADDRESS of score

std::cout << score;    // 42  — the value
std::cout << &score;   // 0x7ffe… — where it lives
std::cout << p;        // 0x7ffe… — same address
std::cout << *p;       // 42  — follow p back to the value

*p = 99;               // write THROUGH the pointer
std::cout << score;    // 99  — score changed!` },

    { t: "tip", title: "Read the type right-to-left", html: "<code>int* p</code> reads as \u201cp is a pointer to int.\u201d The <code>*</code> binds to the type, not the name. Declaring two on one line bites everyone once: <code>int* a, b;</code> makes <code>a</code> a pointer but <code>b</code> a plain int." },

    { t: "h2", text: "References: a second name for the same box" },
    { t: "p", html: "A <strong>reference</strong> is an alias. It is bound to an existing variable at birth and can never be re-pointed. There is no separate address to manage and nothing to dereference — you just use it like the original." },
    { t: "code", lang: "cpp", file: "references.cpp", code:
`int score = 42;
int& alias = score;    // alias IS score, by another name

alias = 99;
std::cout << score;    // 99

// A reference must be initialised and can't be reseated:
int other = 7;
alias = other;         // copies 7 INTO score — does NOT rebind` },

    { t: "p", html: "References shine in function parameters. Pass by reference and the function works on your variable directly — no copy, and changes stick." },
    { t: "code", lang: "cpp", file: "swap.cpp", code:
`void swap(int& a, int& b) {
  int tmp = a;
  a = b;
  b = tmp;
}

int x = 1, y = 2;
swap(x, y);            // x == 2, y == 1` },

    { t: "xref", to: 8, html: "This builds on <strong>Chapter 8 · Arrays &amp; Vectors</strong> — an array name already decays to a pointer to its first element." },

    { t: "h2", text: "Pointer or reference — which?" },
    { t: "list", items: [
      "Use a <strong>reference</strong> when the thing always exists and you never need to repoint it — the common case for function parameters.",
      "Use a <strong>pointer</strong> when it may be absent (use <code>nullptr</code>), when you need to walk through memory, or when ownership transfers.",
      "Prefer references for readability; reach for pointers when you genuinely need their extra powers.",
    ]},

    { t: "warn", title: "The null and the dangling", html: "Dereferencing a <code>nullptr</code> or a pointer to freed memory is undefined behaviour — often a crash, sometimes worse. Always check a pointer before following it, and never keep a pointer to something that has gone out of scope." },

    { t: "p", html: "In modern C++ you rarely manage raw pointers by hand — <strong>smart pointers</strong> (Chapter 24) own memory for you. But to understand them, you first have to understand the raw machinery on this page." },
  ],

  exercise: {
    title: "Build a tiny pointer-based linked list",
    summary: "Implement a minimal singly-linked list of integers using raw pointers, then reverse it in place. This exercises allocation, the <code>-&gt;</code> operator, null checks, and pointer rewiring — the muscles every later memory chapter relies on.",
    tasks: [
      "Define a <code>Node</code> struct holding an <code>int value</code> and a <code>Node* next</code>.",
      "Write <code>push_front(Node*&amp; head, int v)</code> that allocates a node and links it at the front.",
      "Write <code>reverse(Node*&amp; head)</code> that reverses the list by rewiring <code>next</code> pointers — no new allocations.",
      "Write <code>print(Node* head)</code> and <code>free_all(Node* head)</code>; make sure every <code>new</code> has a matching <code>delete</code>.",
    ],
    criteria: [
      "Reversing [1,2,3] prints 3 2 1.",
      "Reversing an empty list and a single-element list both work.",
      "No memory leaks — the tests run under a leak check.",
    ],
    files: [
      { name: "list.starter.cpp", lang: "C++", kind: "starter", code:
`#include <iostream>

struct Node {
  int value;
  Node* next;
};

// TODO 1: allocate a node and link it at the front.
void push_front(Node*& head, int v) {
  // your code here
}

// TODO 2: reverse the list in place by rewiring next pointers.
void reverse(Node*& head) {
  // your code here
}

void print(Node* head) {
  for (Node* p = head; p != nullptr; p = p->next)
    std::cout << p->value << ' ';
  std::cout << '\\n';
}

// TODO 3: delete every node so there are no leaks.
void free_all(Node* head) {
  // your code here
}

int main() {
  Node* head = nullptr;
  push_front(head, 3);
  push_front(head, 2);
  push_front(head, 1);
  print(head);      // expect: 1 2 3
  reverse(head);
  print(head);      // expect: 3 2 1
  free_all(head);
}` },
      { name: "list.solution.cpp", lang: "C++", kind: "solution", code:
`#include <iostream>

struct Node {
  int value;
  Node* next;
};

void push_front(Node*& head, int v) {
  Node* node = new Node{v, head};
  head = node;
}

void reverse(Node*& head) {
  Node* prev = nullptr;
  Node* curr = head;
  while (curr != nullptr) {
    Node* next = curr->next;  // remember the rest
    curr->next = prev;        // flip the link
    prev = curr;              // step forward
    curr = next;
  }
  head = prev;
}

void print(Node* head) {
  for (Node* p = head; p != nullptr; p = p->next)
    std::cout << p->value << ' ';
  std::cout << '\\n';
}

void free_all(Node* head) {
  while (head != nullptr) {
    Node* next = head->next;
    delete head;
    head = next;
  }
}

int main() {
  Node* head = nullptr;
  push_front(head, 3);
  push_front(head, 2);
  push_front(head, 1);
  print(head);      // 1 2 3
  reverse(head);
  print(head);      // 3 2 1
  free_all(head);
}` },
      { name: "list.test.cpp", lang: "C++", kind: "tests", code:
`#include <cassert>
#include <sstream>
#include "list.cpp"

static std::string render(Node* head) {
  std::ostringstream out;
  for (Node* p = head; p; p = p->next) out << p->value << ' ';
  return out.str();
}

int main() {
  Node* head = nullptr;
  push_front(head, 3);
  push_front(head, 2);
  push_front(head, 1);
  assert(render(head) == "1 2 3 ");

  reverse(head);
  assert(render(head) == "3 2 1 ");
  free_all(head);

  // empty list reverses cleanly
  Node* empty = nullptr;
  reverse(empty);
  assert(empty == nullptr);

  // single element
  Node* one = nullptr;
  push_front(one, 7);
  reverse(one);
  assert(render(one) == "7 ");
  free_all(one);

  std::cout << "All tests passed.\\n";
}` },
    ],
  },
};

/* Short, realistic intro blocks for non-hero chapters (stubs that still read real). */
window.CPP_STUB = (ch) => ({
  n: ch.n,
  title: ch.title,
  tag: ch.tag,
  kicker: `Chapter ${ch.n} · ${ch.tag.replace(/\b\w/g, c => c.toUpperCase())}`,
  readingTime: `${6 + (ch.n % 6)} min read`,
  notes: [
    { t: "lead", html: `Notes for <strong>${ch.title}</strong> are being written. This chapter is part of the <em>${ch.tag}</em> track and slots in after the foundations you have already built.` },
    { t: "h2", text: "What you'll learn" },
    { t: "p", html: `By the end you'll be comfortable reading and writing the core ${ch.title.toLowerCase()} patterns, and you'll have worked a hands-on exercise that puts them to use.` },
    { t: "p", html: "Pick Chapter 10 \u2014 Pointers &amp; References from the rail to see a fully fleshed-out chapter." },
  ],
  exercise: null,
});
