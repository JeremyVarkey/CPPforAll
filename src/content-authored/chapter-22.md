# Chapter 22 - Move Semantics and Smart Pointers

> Source: <https://www.learncpp.com/> (Chapter 22)
> One file per chapter. Each lesson is a section below.

## Contents
- [22.1 - Introduction to smart pointers and move semantics](#221---introduction-to-smart-pointers-and-move-semantics)
- [22.2 - R-value references](#222---r-value-references)
- [22.3 - Move constructors and move assignment](#223---move-constructors-and-move-assignment)
- [22.4 - std::move](#224---stdmove)
- [22.5 - std::unique_ptr](#225---stdunique_ptr)
- [22.6 - std::shared_ptr](#226---stdshared_ptr)
- [22.7 - Circular dependency issues with std::shared_ptr, and std::weak_ptr](#227---circular-dependency-issues-with-stdshared_ptr-and-stdweak_ptr)
- [22.x - Chapter 22 summary and quiz](#22x---chapter-22-summary-and-quiz)

---

## 22.1 - Introduction to smart pointers and move semantics

Back in Chapter 19 you learned to allocate memory with `new` and hand it back with `delete`. That gave you power — objects whose lifetime you control by hand, arrays whose size is decided at runtime — but it also handed you a job that is surprisingly easy to get wrong. Every `new` you write becomes a promise: *somewhere, on every path out of this code, I will call the matching `delete`.* This chapter is about handing that promise to the compiler instead of keeping it yourself.

We will get there in two steps. First we will understand *why* a class can clean up after itself automatically (RAII), which leads to **smart pointers** — objects that own a heap allocation and free it for you. Then we will discover that owning a resource raises a new question — what should happen when you copy an owner? — whose answer is **move semantics**. The two ideas are tightly braided, which is why they share a chapter.

### The raw pointer cleanup problem

Consider an ordinary function that allocates a buffer, does some work, and frees it:

```cpp
void writeReport()
{
    ReportBuffer* buffer { new ReportBuffer{} };

    if (!buffer->load())
        return;                 // leak: delete is skipped

    buffer->flush();
    delete buffer;
}
```

Read it carefully. There are two exits from this function, and only one of them runs `delete`. If `load()` fails, we return early and the buffer leaks. The memory stays allocated until the program ends, and if `writeReport` is called in a loop, the leak grows without bound.

The deeper point is that `new` is not the villain here. The villain is that **ownership and cleanup live in two different places**:

```
new creates resource        delete releases resource
        |                           |
        v                           v
    ReportBuffer* buffer ---- many exits can bypass this path
```

A raw pointer is just an address. It has no attached behavior, no sense that it is responsible for anything. So nothing forces the `delete` to happen. Early returns, thrown exceptions, complicated branches, and innocent future edits can all introduce a path where cleanup never runs. You can patch each leak by hand, but you are fighting the language's defaults instead of using them.

### RAII is the underlying idea

The fix is a C++ design pattern with an awkward name and a beautiful payoff: **RAII**, "resource acquisition is initialization." The idea in plain terms:

- acquire the resource in a constructor (or a factory function),
- store the resulting ownership inside an object,
- release the resource in that object's destructor,
- then let the language's automatic destructor calls do the rest.

That last point is the engine. You already know that when a local object goes out of scope, its destructor runs — *automatically, on every path*, including early returns and stack unwinding from an exception. RAII piggybacks on that guarantee. If cleanup lives in a destructor, the language runs it for you everywhere the object dies.

Here is the smallest possible RAII type — an object that owns a single heap `int`:

```cpp
class OwnedInt
{
private:
    int* m_ptr {};

public:
    explicit OwnedInt(int value)
        : m_ptr { new int { value } }   // acquire in constructor
    {
    }

    ~OwnedInt()
    {
        delete m_ptr;                   // release in destructor
    }

    int value() const { return *m_ptr; }
};

void printMaybe(bool shouldPrint)
{
    OwnedInt answer { 42 };

    if (!shouldPrint)
        return;                 // answer still destructs

    std::cout << answer.value() << '\n';
}
```

Look at `printMaybe`. It has the same early-return shape that leaked in `writeReport` — but it cannot leak. When `answer` goes out of scope on *either* path, `~OwnedInt()` runs and the heap `int` is freed. We moved the cleanup from "wherever the programmer remembers to put it" to "wherever the object happens to die," and the language guarantees the second is everywhere.

> **Key insight:** A destructor runs automatically when an object is destroyed, on every exit path. RAII routes resource cleanup through that guarantee so you never have to remember to free anything.

### What a smart pointer is

`OwnedInt` is the seed of the idea. A **smart pointer** is a class built on exactly this principle: it behaves like a pointer while it owns and manages a dynamically allocated object. "Behaves like a pointer" means it overloads the operators you expect a pointer to support:

```cpp
*ptr       // access the owned object
ptr->name  // access a member of the owned object
if (ptr)   // test whether it currently owns or observes something usable
```

And "manages" means the ownership operation you cannot see in the code but always get:

```cpp
// when ptr is destroyed, the managed object is cleaned up
```

One practical caution follows from RAII. A smart pointer only does its job when *its own* destructor runs. So a smart pointer should almost always be a local (stack) variable or a class member — not itself something you allocate with `new`. If you write `new std::unique_ptr<T>{...}` and then forget to delete *that*, the smart pointer's destructor never runs, and the object it was supposed to protect leaks anyway. You would have wrapped a leak inside a leak.

### The shallow-copy trap

Now we hit the problem that makes ownership genuinely hard, and that motivates the second half of this chapter. Suppose you try to write your own smart pointer. A natural first attempt:

```cpp
template <typename T>
class BadOwner
{
private:
    T* m_ptr {};

public:
    explicit BadOwner(T* ptr = nullptr)
        : m_ptr { ptr }
    {
    }

    ~BadOwner()
    {
        delete m_ptr;
    }

    T& operator*() const { return *m_ptr; }
    T* operator->() const { return m_ptr; }
};
```

The destructor is exactly right. The danger is copying. Recall from earlier chapters that if you do not write a copy constructor, the compiler generates one that copies each member. For `BadOwner`, the only member is a pointer — so the generated copy copies the *address*, not the thing it points at:

```cpp
BadOwner<Token> a { new Token { "if" } };
BadOwner<Token> b { a };        // compiler-generated shallow copy
```

Now `a` and `b` hold the same address. Both believe they own that `Token`. At the end of the scope, *both* destructors run:

```
a.m_ptr ----+
            v
          Token
            ^
b.m_ptr ----+

end of scope:
    b deletes Token
    a tries to delete same Token again -> undefined behavior
```

This is the **shallow-copy** problem you met when you first wrote resource-owning classes — but ownership raises the stakes. A shallow copy of plain data is merely wrong; a shallow copy of an *owner* is a double-delete, which is undefined behavior and a common source of crashes and security bugs.

> **Warning:** Copying a raw owning pointer with the compiler-generated copy gives two owners of one resource. When both are destroyed, the resource is deleted twice. That is undefined behavior.

### Copy semantics versus move semantics

So copying an owner is dangerous. What *should* happen when you write `b = a` for an owning type? There are two reasonable answers, and modern C++ lets a class choose between them.

**Copy semantics** means the destination gets its own independent copy of the resource. After the copy, both objects are valid and fully usable, and neither affects the other. This is what `std::string` does:

```cpp
std::string a { "abc" };
std::string b { a };        // b has its own copy of the characters
```

**Move semantics** means the destination *takes over* the resource from the source. No new resource is allocated; ownership simply transfers. The source is left valid but empty — it no longer owns what it used to:

```cpp
std::unique_ptr<Token> a { std::make_unique<Token>("if") };
std::unique_ptr<Token> b { std::move(a) }; // b owns; a is now empty
```

For a resource that should have exactly one owner, transfer is often *exactly* the right operation — and it is cheap, because it shuffles a pointer rather than duplicating the whole resource:

```
before move:
    a ---> Token("if")
    b ---> null

after move:
    a ---> null
    b ---> Token("if")
```

The rest of this chapter builds out both halves: the machinery that makes move semantics possible (rvalue references, move constructors, `std::move`), and the standard smart pointers that put it all to work (`std::unique_ptr`, `std::shared_ptr`, `std::weak_ptr`).

### Why the old std::auto_ptr was a cautionary tale

You may run across `std::auto_ptr` in old code or old books. It was an early attempt at a smart pointer, and it got one thing badly wrong: it tried to implement *ownership transfer* by hijacking the *copy* operation. So writing what looked like an innocent copy would silently empty the source. That broke every reasonable assumption — functions, containers, and algorithms all assume that copying an object leaves the original untouched. `std::auto_ptr` is removed from modern C++ for exactly this reason.

The lesson the language learned is the lesson of this chapter: copying and moving are *different operations* and deserve *different functions*.

| Operation | Meaning |
|---|---|
| Copy constructor / copy assignment | create an independent copy; source unchanged |
| Move constructor / move assignment | transfer resources; source left empty but valid |

The standard smart pointers you will learn here are move-aware from the ground up, so they get this right.

> **Note (CS6340):** LLVM pass code passes IR objects around by pointer and reference all the time — `bool runOnFunction(llvm::Function& function)`, `void visitInstruction(llvm::Instruction* instruction)`. Those parameters almost never mean "this function owns the object." The pass infrastructure owns the module, functions, blocks, and instructions; your pass merely observes or mutates them through non-owning handles. Reach for a smart pointer only when *your* code owns a heap object. Do not wrap a borrowed LLVM pointer in `std::unique_ptr` or `std::shared_ptr` — you would be claiming a responsibility to destroy something you do not own.

---

## 22.2 - R-value references

To make move semantics work, the language needs a way to tell two situations apart: "copy from this object, which I still need" versus "steal from this object, which is about to disappear." That distinction is built on a piece of theory you met earlier — value categories — plus one new tool, the **rvalue reference**. This lesson is about that tool. It is a little abstract on its own; hold on, because the next lesson is where it pays off.

### A quick lvalue / rvalue recap

Every expression in C++ has two independent properties: a *type* (like `int`) and a *value category*. The two value categories that matter here are **lvalues** and **rvalues**.

```cpp
int x { 5 };

x       // lvalue: names an object that has identity and a place in memory
5       // rvalue: a temporary value with no lasting identity
x + 1   // rvalue: the result of a computation, a temporary
```

A useful mental shortcut: an lvalue is something you could take the address of and that persists; an rvalue is a fleeting value — a literal, or the temporary result of an expression — that is about to evaporate. That "about to evaporate" quality is the key. If an expression is an rvalue, the value it holds is not needed after this statement, so it is safe to pillage.

You already know the lvalue reference, written with a single `&`:

```cpp
int& ref { x };          // binds to a modifiable lvalue
const int& cref { 5 };   // a const lvalue reference can bind to an rvalue
```

That second line is worth pausing on. A `const` lvalue reference can bind to an rvalue — which is why you can pass a temporary to a `const T&` parameter without copying. But because it is `const`, you cannot modify what it refers to. It can *look* at the temporary; it cannot *consume* it.

### Rvalue references use &&

C++11 introduced a new kind of reference, written with two ampersands, the **rvalue reference**:

```cpp
int&& tempRef { 5 };
```

An rvalue reference is designed to bind to an rvalue — and, unlike a `const` lvalue reference, it gives you *non-const* access to that rvalue. That is the whole trick. Here is the full picture of what binds to what:

| Reference kind | Binds to modifiable lvalue? | Binds to rvalue? | Can modify through it? |
|---|---:|---:|---:|
| `int&` | Yes | No | Yes |
| `const int&` | Yes | Yes | No |
| `int&&` | No | Yes | Yes |
| `const int&&` | No | Yes | No |

Read the `int&&` row against the `const int&` row. Both can bind to a temporary, but only the rvalue reference lets you *modify* it. That is precisely what a move operation needs: a handle to an expiring object that it is allowed to reach into and gut.

> **Key insight:** A non-const rvalue reference binds to a temporary (or an explicitly-moved-from object) and grants modifiable access to it. That combination — "this is expiring, and you may take its insides" — is the foundation of move semantics.

### Rvalue references can extend a temporary's lifetime

One curiosity worth seeing once: binding an rvalue reference to a temporary extends that temporary's life to match the reference.

```cpp
struct Pair
{
    int first {};
    int second {};
};

Pair&& pair { Pair { 3, 4 } };
pair.first = 9;

std::cout << pair.first << ", " << pair.second << '\n';   // 9, 4
```

The temporary `Pair { 3, 4 }` would normally die at the end of its statement, but bound to `pair` it lives as long as `pair` does — and because the reference is non-const, we can even modify it. This is legal but unusual; you rarely store rvalue references in local variables like this. Their real home is **function parameters**, which is where we head next.

### Rvalue reference parameters enable overloads

Here is the payoff that motivates the whole feature. Because `T&` and `T&&` are different parameter types, you can overload a function on them — and the compiler will route an *lvalue argument* to one and an *rvalue argument* to the other:

```cpp
void logValue(const std::string& text)
{
    std::cout << "observing: " << text << '\n';
}

void logValue(std::string&& text)
{
    std::cout << "can consume temporary: " << text << '\n';
}

std::string name { "branch" };

logValue(name);                   // lvalue argument -> const& overload
logValue(std::string { "edge" }); // rvalue argument -> && overload
```

When you pass `name` (an lvalue, something you still need), overload resolution prefers the `const std::string&` version, which only observes. When you pass a freshly-built temporary (an rvalue, about to vanish), it prefers the `std::string&&` version, which is allowed to treat `text` as consumable. This single mechanism is how a class will distinguish "copy this existing object" from "steal from this expiring one." A move constructor is just an overload that takes `T&&`.

### A named rvalue reference variable is itself an lvalue

This is the rule that trips up everyone the first time, so let us make it crisp. Look:

```cpp
void consume(std::string&& value);

void wrapper(std::string&& value)
{
    consume(value);              // error: value is a named variable -> it is an lvalue
    consume(std::move(value));   // ok: explicitly treat it as movable again
}
```

How can `value` be an lvalue when its *type* is `std::string&&`? Because *type* and *value category* are two separate properties. The declared type of `value` is "rvalue reference to string," but the *expression* `value` — the act of naming it — refers to an object with a name and an address that persists for the rest of the function. Anything you can name and that sticks around is an lvalue.

```
std::string&& value
        type: rvalue reference to string
  expression: value names a persistent object, therefore lvalue
```

The reasoning behind the rule is actually sensible: inside `wrapper`, you can use `value` more than once, so the language refuses to silently let the first use gut it. If you genuinely mean "I am done with this, you may move from it," you say so explicitly with `std::move` — the subject of lesson 22.4.

> **Warning:** A variable of type `T&&` is still an lvalue when you use it by name. To pass it on so that a move can occur, wrap it in `std::move`. Forgetting this is the most common reason a "move" silently turns into a copy.

### Do not return an rvalue reference casually

A final caution that the chapter lab calls out explicitly. Returning `T&&` from an ordinary function is almost always a bug:

```cpp
std::string&& bad()
{
    std::string local { "gone" };
    return std::move(local);     // dangling reference: local dies at return
}
```

`local` is destroyed when the function exits, so the returned reference dangles — it refers to memory that no longer holds a valid object. Return *by value* instead and let the language's return optimizations handle efficiency (more on that in 22.4). The rule of thumb: return values, not references to locals.

---

## 22.3 - Move constructors and move assignment

This is the lesson the chapter lab is built around. By the end of it you will be able to write a class that, when copied, deep-copies its resource, but when *moved*, cheaply steals it. The `TrackedBuffer` you complete in the lab does exactly this, with counters that let you *watch* which path fired.

### First, the copy operations recap

Recall the resource-owning class from Chapter 19: it holds a size and an owning pointer to a heap array.

```cpp
class Buffer
{
private:
    std::size_t m_size {};
    int* m_data {};

public:
    explicit Buffer(std::size_t size)
        : m_size { size },
          m_data { new int[size]{} }
    {
    }

    ~Buffer()
    {
        delete[] m_data;
    }
};
```

If you let the compiler generate the copy constructor, it copies `m_data` — the *pointer* — so two `Buffer` objects end up owning the same array. That is the shallow-copy double-delete from 22.1, all over again. The fix is a **deep copy**: allocate a fresh array and copy the elements across.

```cpp
Buffer(const Buffer& other)
    : m_size { other.m_size },
      m_data { new int[other.m_size]{} }
{
    std::copy_n(other.m_data, m_size, m_data);
}
```

This is correct. Each `Buffer` now owns its own array. But notice what it costs: an allocation plus an element-by-element copy. When the source object is about to be destroyed anyway — say it is a temporary returned from a function — that work is pure waste. We allocate a duplicate only to throw the original away moments later. Move semantics exists to eliminate exactly this waste.

### Move construction

A **move constructor** builds a new object by *taking* the resources of another object rather than duplicating them. Its signature takes an rvalue reference — `Buffer&&` — so (per 22.2) overload resolution selects it precisely when the source is an expiring value the language is willing to let us gut.

```cpp
class Buffer
{
private:
    std::size_t m_size {};
    int* m_data {};

public:
    explicit Buffer(std::size_t size = 0)
        : m_size { size },
          m_data { size == 0 ? nullptr : new int[size]{} }
    {
    }

    ~Buffer()
    {
        delete[] m_data;
    }

    Buffer(const Buffer& other)            // copy: deep
        : Buffer { other.m_size }
    {
        std::copy_n(other.m_data, m_size, m_data);
    }

    Buffer(Buffer&& other) noexcept        // move: steal
        : m_size { other.m_size },
          m_data { other.m_data }
    {
        other.m_size = 0;
        other.m_data = nullptr;
    }
};
```

Study the move constructor. It does two things, in two stages:

1. **Steal.** Copy the source's `m_size` and `m_data` *pointer* into the new object. No allocation, no element copy — the new object now points at the array the source used to own.
2. **Neutralize the source.** Set `other.m_data = nullptr` and `other.m_size = 0`. This is not optional housekeeping; it is the whole correctness argument. If we left `other.m_data` pointing at the array, then when `other`'s destructor ran it would `delete[]` the array we just stole — a double-delete.

```
before move construction:
    other.m_data ---> [ 0 0 0 0 ]
    new object   ---> not built yet

after move construction:
    other.m_data ---> nullptr
    new.m_data   ---> [ 0 0 0 0 ]
```

The moved-from object must be left in a state that is **safe to destruct**. Setting pointers to `nullptr` is the standard way, because both `delete nullptr` and `delete[] nullptr` are guaranteed no-ops. The source object is now empty — valid, destructible, but no longer owning anything.

> **Key insight:** A move constructor copies a handle (a pointer) instead of duplicating a resource, then nulls the source so the resource is owned by exactly one object. That is why a move is cheap and a deep copy is not.

### Move assignment

The move *constructor* builds a brand-new object, so it starts from nothing. Move *assignment* overwrites an object that already exists — which means it has an extra job the constructor does not: it must release whatever the destination currently owns before stealing the source's resource. Otherwise the old array leaks.

```cpp
Buffer& operator=(Buffer&& other) noexcept
{
    if (this == &other)              // 1. guard against self-move
        return *this;

    delete[] m_data;                 // 2. release our current resource

    m_size = other.m_size;           // 3. steal the source's resource
    m_data = other.m_data;

    other.m_size = 0;                // 4. neutralize the source
    other.m_data = nullptr;

    return *this;                    // 5. return *this for chaining
}
```

The order of those five steps matters, and the lab grades you on getting it right:

```
1. detect self-assignment  (b = std::move(b) must stay valid)
2. release destination's old resource  (or it leaks)
3. copy pointer/handle values from source
4. neutralize source  (or it double-deletes)
5. return *this
```

Step 1 — the self-assignment guard — is the subtle one. The expression `b = std::move(b)` is unusual, but it is legal, and your operator must survive it. Without the guard, `this` and `&other` are the same object: you would `delete[] m_data`, and then on the very next line read `other.m_data` (the same, now-freed pointer) — reading freed memory, undefined behavior. The single `if (this == &other) return *this;` makes that case a harmless no-op.

> **Warning:** Both assignment operators (copy and move) need a self-assignment guard, and move assignment must release the destination's existing resource *before* stealing the source's. Skip either and you get a leak, a double-delete, or a read of freed memory.

### Moves are selected automatically for rvalues

You almost never call these operators by their real names. The compiler invokes them through ordinary-looking syntax, choosing copy versus move based on the value category of the right-hand side:

```cpp
Buffer makeBuffer()
{
    Buffer temp { 1024 };
    return temp;                 // may be elided; otherwise moved
}

Buffer a { 16 };
Buffer b { makeBuffer() };       // move construction (if not elided)

Buffer c { 32 };
c = makeBuffer();                // move assignment (if not elided)
```

`makeBuffer()` returns a temporary — an rvalue — so initializing `b` from it selects the move constructor, and assigning it to `c` selects move assignment. Because the source is a temporary nobody else can see, stealing from it is perfectly safe.

The phrase "if not elided" matters and the lab makes a point of it. The compiler is often allowed to skip the move entirely, constructing the object directly in its destination — an optimization called **copy elision**. Two cases are worth keeping straight. Returning a *named* local like `return temp;` is **named return value optimization (NRVO)**: a compiler *may* elide it, but C++17 does not *require* it, so a conforming compiler is free to fall back to a move (which is why `temp` still needs a move constructor). The case C++17 makes **mandatory** is the elision of a genuine *prvalue* — for example `return Buffer{1024};`, or the temporary produced by `makeBuffer()` when it is used to initialize `b` in `Buffer b { makeBuffer() };`. In that mandatory case, *no* constructor — not copy, not move — fires for that step. The practical upshot is the same one the lab flags: never assert your move counter on the result of a factory like `makeBuffer()`, because elision may have left nothing to count. You test moves on *named* objects you explicitly move, where no elision applies. We will return to this in 22.4.

### Why noexcept matters on move operations

You will notice every move operation above is marked `noexcept`. That keyword promises the function will not throw an exception, and for move operations it has a concrete performance consequence.

Consider `std::vector`. When it grows past its capacity, it allocates a bigger block and relocates its existing elements into it. It would love to *move* them (cheap) rather than copy them (expensive). But the vector also promises a strong exception guarantee: if relocation fails partway, the vector must look untouched. A move operation that could throw would make that promise impossible to keep — half the elements moved, half not, and no clean way back. So the vector plays it safe: **it will only move during reallocation if your move operations are `noexcept`.** Otherwise it copies, and you silently lose the performance you wrote a move constructor to get.

> **Best practice:** Mark move constructors and move assignment operators `noexcept`. Standard containers like `std::vector` only use your moves during reallocation when they are known not to throw; without `noexcept`, they fall back to copying.

### The rule of five

This brings us to a guideline that ties the chapter together. There are five "special member functions" that deal with an object's lifetime and resource handling:

| Special member | What it does |
|---|---|
| Destructor | releases the owned resource |
| Copy constructor | builds a new object from an existing lvalue |
| Copy assignment | overwrites an existing object from an lvalue |
| Move constructor | builds a new object from an expiring value |
| Move assignment | overwrites an existing object from an expiring value |

The **rule of five** says: if your class needs to define (or explicitly delete) *any* of these — which usually means it manages a raw resource — then you should think carefully about *all five*, because the compiler-generated defaults for the others are probably wrong for a resource-owning type. They form a set; handle them as one.

There is a happy corollary. The reason you rarely write these by hand in real code is that you usually do not manage a raw resource directly. If you store your resource in a `std::unique_ptr`, `std::vector`, or `std::string`, *those* types already implement the rule of five correctly, and the compiler-generated defaults for your class just forward to them. The best way to obey the rule of five is often to let a member type obey it for you.

### Move-only types

Sometimes a resource simply cannot be copied in any meaningful way — an open file, a network socket, a lock. For such a type, copying makes no sense, but *transferring* ownership does. You express that by deleting the copy operations and defaulting the moves:

```cpp
class OutputFile
{
private:
    std::ofstream m_stream;

public:
    explicit OutputFile(const std::string& path)
        : m_stream { path }
    {
    }

    OutputFile(const OutputFile&) = delete;             // no copying
    OutputFile& operator=(const OutputFile&) = delete;

    OutputFile(OutputFile&&) noexcept = default;        // moving is fine
    OutputFile& operator=(OutputFile&&) noexcept = default;
};
```

This says, at compile time, "there is exactly one owner of this file, but ownership may be transferred." Trying to copy an `OutputFile` is now a compile error rather than a runtime surprise. `std::unique_ptr`, which you will meet in 22.5, is the most important move-only type in the standard library — and it is move-only for exactly this reason.

---

## 22.4 - std::move

You have now seen `std::move` used several times, always at the call site where you wanted a move to happen. It is time to say precisely what it does — because its name is genuinely misleading, and the misunderstanding it causes is one of the most common sources of confusion in modern C++.

### std::move does not move anything

Here is the surprising truth: **`std::move` does not move.** It performs no copying, no stealing, no work on the object at all. All it does is *cast* its argument to an rvalue reference. In other words, it changes the value category of an expression from lvalue to rvalue, and nothing else.

```cpp
#include <utility>           // std::move lives here

std::string name { "coverage" };
std::string saved { std::move(name) };
```

Recall from 22.2 that overload resolution picks the move constructor when the source is an rvalue, and the copy constructor when it is an lvalue. `name` is a named variable, so it is an lvalue — left to itself it would be *copied*. `std::move(name)` casts it to an rvalue, which flips overload resolution to select the move constructor. *That* constructor is what actually performs the move.

So the right way to read `std::move(x)` is: "I promise I am done with `x`; you may treat it as something to move from." It is a request, an opt-in, a green light — not the move itself. Whether a move actually happens depends entirely on whether the selected constructor or assignment operator implements one.

> **Key insight:** `std::move(x)` is a cast that says "you may move from `x`." The actual move (if any) is performed by the move constructor or move assignment operator that the cast causes overload resolution to select. If the type has no move operations, you silently get a copy.

### Why swaps love moves

A clean illustration of the payoff. Here is the classic swap, written with copies:

```cpp
template <typename T>
void swapByCopy(T& a, T& b)
{
    T temp { a };   // copy a
    a = b;          // copy b
    b = temp;       // copy temp
}
```

Three copies. For a `T` like `std::string` or `std::vector`, that is three allocations and three element-by-element duplications — to accomplish a swap that fundamentally just shuffles ownership around. Now the same swap with moves:

```cpp
#include <utility>

template <typename T>
void swapByMove(T& a, T& b)
{
    T temp { std::move(a) };   // steal a into temp
    a = std::move(b);          // steal b into a
    b = std::move(temp);       // steal temp into b
}
```

No allocations, no element copies — just three ownership transfers. For an `int`, the two versions are identical (there is nothing to steal). For resource-owning types, the move version can be dramatically faster. This is exactly why `std::swap` in the standard library is move-based, and why a move-based swap is one of the stretch goals in the chapter lab.

### Moving into containers

A very common, very practical use: putting a local object into a container you no longer need the local for.

```cpp
std::vector<std::string> names {};

std::string generated { "mutation-operator" };
names.push_back(generated);             // copy: generated is untouched

std::string temporary { "branch-id" };
names.push_back(std::move(temporary));  // move: temporary is now empty
```

The first `push_back` copies — sensible, because we pass `generated` by name (an lvalue) and might use it again. The second tells `push_back` to *move* its argument in, avoiding a string copy. The cost is that `temporary` is afterward in a moved-from state, which leads to the single most important rule about `std::move`.

### Moved-from objects are valid but unspecified

After you move from a standard library object, what is left behind? The standard guarantees the object is **valid but unspecified**. "Valid" means it is still a real, well-formed object — you can assign to it, ask its size, or let it destruct safely. "Unspecified" means you may *not* assume anything about its *contents*.

So these are all fine — they either overwrite the object or query its (legitimate, if unknown) state:

```cpp
temporary.clear();
temporary = "new-name";
if (temporary.empty()) { /* ok */ }
```

But this is a bug:

```cpp
char first { temporary[0] };   // unsafe: temporary might be empty after the move
```

After the move, `temporary` might be empty, or it might hold some other valid-but-arbitrary value. Reading `[0]` assumes there is a character there, and that assumption is not guaranteed. The discipline is simple: **treat a moved-from object as consumed.** Either give it a new value or stop using it (other than letting it destruct).

> **Warning:** A moved-from standard library object is valid but its value is unspecified. You may assign to it or destroy it, but do not read its old contents or assume anything about its state. The lab calls this the "moved-from state" and tests for it directly.

### When to use std::move — and when not to

`std::move` is a sharp tool. Use it when:

- transferring ownership from one smart pointer to another,
- moving a large local object into a container,
- implementing your own move constructor or move assignment,
- writing generic code that swaps or relocates values,
- returning an object in the rare cases where the normal return optimization does not already do the right thing.

Avoid `std::move` when:

- you still need the object's current value afterward,
- the object is `const` (you cannot move *from* a const object — there is nothing to steal without modifying it, so it silently copies),
- you are returning a local by value in ordinary C++17 code.

That last one deserves emphasis, because beginners reach for `std::move` on return statements thinking it helps. It does not — it *hurts*:

```cpp
std::string makeName()
{
    std::string name { "instrumented" };
    return name;                // best: let copy elision / implicit move apply
}
```

When you return a local by value, the compiler already does the optimal thing on its own: it elides the copy entirely where it can, or performs an implicit move where it cannot. Writing `return std::move(name);` actively *prevents* copy elision (the compiler can no longer treat the return as a plain prvalue), so you end up forcing a move where you could have had nothing at all. Trust the language here.

> **Best practice:** Do not write `std::move` on a return statement that returns a local variable by value. Copy elision and the implicit move already make it optimal, and `std::move` only gets in the way.

This is the same elision rule the lab flags as the "C++17 elision trap": never assert a move counter on the result of a factory function, because no move constructor fires on a prvalue return. You verify moves on *named* objects you wrap in `std::move`, where the move genuinely happens.

> **Note (CS6340):** Even in LLVM pass code, which is mostly pointers and references, move semantics shows up in your own helper data. Building a list of coverage labels, you might write `labels.push_back(std::move(label));` — fine, *as long as you never read `label` again*. The rule is the same everywhere: once you have moved from a value, do not log it, compare it, or build a later identifier from it unless you first give it a fresh value.

---

## 22.5 - std::unique_ptr

Everything so far has been preparation. Now you get to use the payoff: the standard library's smart pointers, which package RAII and move semantics into types you can use immediately. The first and most important is `std::unique_ptr`.

### Unique ownership

`std::unique_ptr<T>` owns exactly one dynamically allocated `T`, and it is your default choice whenever a heap object should have a single, clear owner. When the `unique_ptr` is destroyed, it deletes what it owns. No manual `delete`, ever.

```cpp
#include <memory>

auto node { std::make_unique<Node>("entry") };
// node owns a heap-allocated Node; it will be deleted when node goes out of scope
```

The "unique" in the name is enforced by the type itself: a `std::unique_ptr` cannot be copied. It is a move-only type, exactly like the `OutputFile` from 22.3. This is what makes its ownership guarantee airtight — there is no way to accidentally end up with two `unique_ptr`s owning the same object, because copying is a compile error.

```cpp
auto a { std::make_unique<Node>("entry") };

// auto b { a };                // error: unique_ptr's copy constructor is deleted
auto b { std::move(a) };        // ok: ownership transfers from a to b
```

After the move, `b` owns the `Node` and `a` is empty (it holds `nullptr`). Ownership moved; it was never shared. This is move semantics doing precisely the job it was designed for.

### Prefer std::make_unique

There are two ways to create a `unique_ptr`. Prefer this:

```cpp
auto token { std::make_unique<Token>("identifier", 12) };
```

over this:

```cpp
std::unique_ptr<Token> token { new Token { "identifier", 12 } };
```

`std::make_unique` is shorter, it spares you from writing the type twice, and it never leaves a raw `new` exposed in your code. It also closes a subtle gap where, if a `new` and the construction of the owning `unique_ptr` were separate steps in a larger expression, an exception in between could leak. `make_unique` performs the allocation and establishes ownership as one atomic step.

> **Best practice:** Create `unique_ptr`s with `std::make_unique` rather than a raw `new`. It is clearer, avoids repeating the type, and never leaves an unmanaged `new` that could leak.

### Accessing the owned object

A `std::unique_ptr` behaves like a pointer for the operations you would expect, and it can be empty, so you test it before dereferencing when emptiness is possible:

```cpp
auto block { std::make_unique<BlockInfo>(7) };

if (block)                              // true if it owns something
{
    std::cout << block->id() << '\n';   // arrow access, like a raw pointer
    std::cout << (*block).id() << '\n'; // dereference, also like a raw pointer
}
```

### Returning unique_ptr by value

Returning a `std::unique_ptr` by value is not only allowed, it is the idiomatic way to write a factory function that hands ownership to the caller:

```cpp
std::unique_ptr<TraceFile> openTraceFile(const std::string& path)
{
    auto file { std::make_unique<TraceFile>(path) };

    if (!file->isOpen())
        return nullptr;     // a unique_ptr can return null to signal failure

    return file;            // ownership transfers out to the caller
}
```

The caller receives a `unique_ptr` and, with it, ownership:

```cpp
auto trace { openTraceFile("coverage.log") };
if (trace)
    trace->write("start");
// trace (and the TraceFile) is cleaned up automatically at end of scope
```

This is exactly the shape of the `makeTracked` factory in the chapter lab. And note — per 22.4 — you write `return file;`, *not* `return std::move(file);`. The compiler handles the transfer optimally on its own; adding `std::move` would only interfere with elision.

> **Warning:** Never return a `std::unique_ptr` by raw pointer or reference (`TraceFile*`, `std::unique_ptr<T>&`). That hides who owns the object and can leave the caller with a dangling pointer once the `unique_ptr` is destroyed. Return it *by value* — that is what transfers ownership cleanly.

### Passing unique_ptr to functions

The parameter type you choose communicates your intent about ownership. There are three common cases, and the lab exercises two of them:

```cpp
void takeOwnership(std::unique_ptr<TraceFile> file)   // by value: I take ownership
{
    file->write("owned here");
}   // file destructs here, deleting the TraceFile

void observeFile(const TraceFile& file)               // by ref: I just look
{
    std::cout << file.path() << '\n';
}

void maybeObserveFile(const TraceFile* file)          // by ptr: I look, maybe null
{
    if (file)
        std::cout << file->path() << '\n';
}
```

At the call site, the syntax itself signals whether ownership moves:

```cpp
auto file { std::make_unique<TraceFile>("out.txt") };

observeFile(*file);             // no transfer: pass the pointed-to object by ref
maybeObserveFile(file.get());   // no transfer: pass a raw observing pointer

takeOwnership(std::move(file)); // transfer: file is empty after this line
```

That last line is the lab's `takeOwnership`, and it is worth dwelling on. Because `takeOwnership` takes its `unique_ptr` *by value*, and `unique_ptr` cannot be copied, you *must* write `std::move(file)` — passing `file` directly is a compile error. The language refuses to let ownership transfer happen by accident; you have to ask for it explicitly. After the call, your local `file` is null, which is correct: the ownership genuinely left.

Here is the rule of thumb in one table:

| Function intent | Parameter style |
|---|---|
| Take ownership | `std::unique_ptr<T>` by value |
| Observe a required object | `T&` or `const T&` |
| Observe an optional object | `T*` or `const T*` |

### unique_ptr and arrays

`std::unique_ptr<T[]>` (note the `[]`) can manage a `new[]`-allocated array, calling `delete[]` for you. But in most code you should prefer a standard container, which gives you the same ownership safety plus a size, bounds-checked access, and the ability to grow:

```cpp
std::vector<int> counters(100);   // usually better than unique_ptr<int[]>
std::string text;                 // better than unique_ptr<char[]>
```

Reach for `std::unique_ptr<T[]>` only when a genuine ownership boundary forces a raw dynamic-array shape on you. (The chapter lab's `TrackedBuffer` uses a raw `int*` on purpose, to make you implement the rule of five by hand — but its final stretch goal invites you to swap in a `unique_ptr<int[]>` and watch the boilerplate evaporate.)

### get, reset, and release

Three member functions you will occasionally need:

```cpp
auto ptr { std::make_unique<Node>("root") };

Node* raw { ptr.get() };        // observe only: ptr still owns the Node

ptr.reset();                    // delete the owned object; ptr becomes empty

Node* leaked { ptr.release() }; // ptr gives up ownership WITHOUT deleting;
delete leaked;                  // now YOU must delete it manually
```

- `get()` hands you a raw *observing* pointer. Ownership stays with the `unique_ptr`. Use it to pass a non-owning pointer to a function (as in `maybeObserveFile` above). Do not delete what `get()` returns.
- `reset()` deletes the current object and empties the `unique_ptr`.
- `release()` is the dangerous one: it makes the `unique_ptr` give up ownership *without* deleting, handing you a raw pointer that you are now responsible for. It defeats the entire purpose of the smart pointer and is easy to misuse. Most code never needs it.

### Misuse patterns to avoid

Two mistakes break the unique-ownership invariant and lead to a double-delete. First, never build two `unique_ptr`s from the same raw pointer:

```cpp
Node* raw { new Node("bad") };

std::unique_ptr<Node> a { raw };
std::unique_ptr<Node> b { raw };    // disaster: both will delete the same Node
```

Each `unique_ptr` thinks it is the sole owner, so each deletes the `Node` — the second delete is undefined behavior. Using `std::make_unique` makes this mistake nearly impossible, because there is no loose raw pointer to wrap twice.

Second, never manually delete what a `unique_ptr` owns:

```cpp
auto node { std::make_unique<Node>("bad") };
delete node.get();                  // wrong: node will delete it again later
```

> **Best practice:** Once a resource is owned by a `unique_ptr`, let the `unique_ptr` own it completely. Do not wrap the same raw pointer twice, and do not `delete` what `get()` returns. The whole point is that you stop managing the lifetime by hand.

---

## 22.6 - std::shared_ptr

`std::unique_ptr` answers "what if exactly one thing owns this object?" But sometimes a resource genuinely has *several* legitimate owners, and it should stay alive as long as *any* of them still needs it. That is what `std::shared_ptr` is for.

### Shared ownership

`std::shared_ptr<T>` represents **shared ownership**. Many `shared_ptr` objects can co-own the same resource, and the resource is destroyed only when the *last* of them goes away. Unlike `unique_ptr`, a `shared_ptr` *can* be copied — and copying it is exactly how you create another co-owner.

```cpp
#include <memory>

auto config { std::make_shared<Config>("lab") };

auto a { config };      // a co-owns the same Config
auto b { config };      // b co-owns it too

// the Config stays alive while config, a, OR b still owns it
```

How does it know when the last owner is gone? Every `shared_ptr` to a given object shares a small hidden **control block** that holds a reference count. Copying a `shared_ptr` increments the count; destroying one decrements it. When the count hits zero, the managed object is deleted.

```
shared_ptr ---> resource pointer ---> Config
          \
           -> control block:
                strong owners: 3
                weak observers: 0
```

(The "weak observers" line is for `std::weak_ptr`, which we meet in 22.7.) This counting is automatic, but it is not free — there is a little runtime overhead per copy and destroy, and the count must be kept correct even across threads. That cost is the price of shared lifetime, and it is the reason `shared_ptr` is not the default.

### Prefer std::make_shared

As with `unique_ptr`, prefer the factory:

```cpp
auto table { std::make_shared<LookupTable>() };
```

`std::make_shared` is simpler and usually more efficient, because it can allocate the managed object and its control block together in a single allocation, rather than two.

> **Best practice:** Create shared objects with `std::make_shared`, and create *additional* owners by copying an existing `shared_ptr` — not by wrapping the raw pointer again.

### Copy an existing shared_ptr; never re-wrap a raw pointer

This point is the `shared_ptr` analogue of the unique-ownership trap, and it is critical. To get a second owner, *copy the `shared_ptr` you already have*:

```cpp
auto first { std::make_shared<Node>("root") };
auto second { first };              // correct: shares the SAME control block
```

Both `first` and `second` point at one control block, whose count is now 2. Everything works.

But if you independently construct two `shared_ptr`s from the same raw pointer, each one builds its *own* control block:

```cpp
Node* raw { new Node("root") };

std::shared_ptr<Node> first { raw };
std::shared_ptr<Node> second { raw }; // wrong: two SEPARATE control blocks
```

Now there are two control blocks, each with a count of 1, each believing it is the sole owner. When the first reaches zero it deletes the `Node`; when the second reaches zero it deletes the *already-deleted* `Node` — a double-delete, undefined behavior. The rule is absolute:

> **Warning:** To add an owner, copy an existing `shared_ptr`. Never construct two `shared_ptr`s independently from the same raw pointer — each makes its own control block, and the object gets deleted more than once. Using `std::make_shared` sidesteps this entirely.

### shared_ptr is for *real* shared lifetime

It is tempting to reach for `shared_ptr` whenever ownership feels complicated, as a way to avoid thinking. Resist that. Shared ownership has real costs — runtime overhead, and lifetime that is no longer local and easy to reason about (the object dies "whenever the last owner happens to release it," which can be hard to predict). Use it only when the lifetime truly is shared.

Good cases for `shared_ptr`:

- a resource must outlive several independent owners and die only when all are done,
- callbacks or asynchronous tasks need to keep a shared state object alive,
- graph-like structures where nodes are genuinely shared.

Poor cases:

- there is one clear owner — use `std::unique_ptr`,
- callers only need temporary access — use a reference or raw pointer,
- the object's lifetime belongs to some external framework — borrow it, do not own it.

> **Best practice:** Default to `std::unique_ptr` for ownership and to references or raw pointers for non-owning access. Use `std::shared_ptr` only when an object genuinely needs multiple owners keeping it alive.

### unique_ptr can become shared_ptr (but not the reverse)

Ownership can flow from unique to shared. Because a `unique_ptr` proves it is the sole owner, you can safely move it into a `shared_ptr`:

```cpp
std::unique_ptr<Config> unique { std::make_unique<Config>() };
std::shared_ptr<Config> shared { std::move(unique) };   // ok: unique gives up ownership
```

The reverse is not generally safe. A `shared_ptr` might be one of several owners, and its type alone gives you no way to prove it is the only one. You cannot extract a `unique_ptr` (which demands sole ownership) from something that might be shared.

### shared_ptr and arrays

As with `unique_ptr`, prefer a real container over a smart pointer to a raw array. Older standards handled arrays through `shared_ptr` awkwardly, and even in modern code `std::vector<T>` or `std::array<T, N>` is almost always the clearer abstraction for a sequence of elements.

> **Note (CS6340):** LLVM pass code should not use `std::shared_ptr` for IR objects. The pass manager, module, and `LLVMContext` own the instructions, blocks, and functions; your pass borrows them. If a helper needs to remember an instruction for a while, store a raw pointer — `std::vector<llvm::Instruction*> interesting;` — which honestly says "these are borrowed, I do not own them." Wrapping them in a `shared_ptr` would falsely claim co-ownership of objects whose lifetime LLVM controls.

---

## 22.7 - Circular dependency issues with std::shared_ptr, and std::weak_ptr

`std::shared_ptr` solves shared ownership elegantly — until two objects own *each other*. That single case breaks reference counting in a way it cannot recover from on its own, and the fix is a fourth and final smart pointer: `std::weak_ptr`.

### The shared_ptr cycle problem

Reference counting deletes an object when its count reaches zero. But a count can be held up by the object's own peers. Consider two nodes that each hold a `shared_ptr` to the other:

```cpp
class Node
{
public:
    std::string name {};
    std::shared_ptr<Node> neighbor {};
};

auto left { std::make_shared<Node>() };
auto right { std::make_shared<Node>() };

left->neighbor = right;     // left now co-owns right
right->neighbor = left;     // right now co-owns left
```

Trace the counts at the end of the scope. The local variables `left` and `right` are destroyed, each dropping one reference. But:

```
left variable releases its reference to the left Node
right variable releases its reference to the right Node

but:
    the left Node still owns the right Node through neighbor
    the right Node still owns the left Node through neighbor

so each count stays at 1 -- it never reaches zero
```

Neither node's count ever reaches zero, because each is kept alive by the other. Both objects leak. This is a **reference cycle**, and it is the one situation `shared_ptr` cannot handle by itself: the counting mechanism has no way to notice that a group of objects is only keeping *itself* alive.

> **Key insight:** `std::shared_ptr` reference counting cannot collect cycles. If two (or more) objects own each other through `shared_ptr`s, their counts never reach zero and they leak, even after every external owner is gone.

### std::weak_ptr is a non-owning observer

The cure is to make one direction of the relationship *non-owning*. `std::weak_ptr<T>` observes an object that is managed by a `shared_ptr<T>`, but **does not** contribute to its reference count. A `weak_ptr` says "let me know about this object, but do not keep it alive on my account."

```cpp
class Node
{
public:
    std::string name {};
    std::weak_ptr<Node> neighbor {};    // observes, does not own
};
```

With the back-link (or cross-link) changed to a `weak_ptr`, the cycle is broken: the neighbor link no longer holds the target's count above zero, so when the external owners go away, the strong counts fall to zero and both nodes are properly destroyed.

### A weak_ptr must be locked before use

There is a catch, and it is a sensible one. Because a `weak_ptr` does not keep its target alive, the target may have *already been destroyed* by the time you want to use the link. So a `weak_ptr` deliberately does not give you `operator->` or `operator*` directly — that would let you dereference a possibly-dead object. Instead, you ask it for a temporary `shared_ptr` via `lock()`:

```cpp
void printNeighbor(const std::weak_ptr<Node>& weak)
{
    if (auto node { weak.lock() })          // try to get a shared_ptr
        std::cout << node->name << '\n';    // alive: safe to use through node
    else
        std::cout << "neighbor expired\n";  // already destroyed
}
```

`lock()` is the safe gate. It returns:

- a **non-empty** `shared_ptr` if the object is still alive — and that temporary `shared_ptr` keeps it alive for as long as you hold it, so it cannot vanish mid-use,
- an **empty** `shared_ptr` if the object has already been destroyed.

That is the whole protocol: lock, check, use. You can also ask directly whether the target is gone:

```cpp
if (weak.expired())
    std::cout << "already gone\n";
```

For deciding whether to *use* the object, prefer `lock()` and test the result — between an `expired()` check and the use, the object could expire — whereas the `shared_ptr` from `lock()` guarantees the object stays alive while you hold it.

> **Best practice:** To use the object behind a `weak_ptr`, call `lock()` and check the returned `shared_ptr` rather than calling `expired()` and then dereferencing. The locked `shared_ptr` keeps the object alive for the duration of your use.

### Where weak_ptr fits

The general principle: when objects share ownership, model the *owning* links with `shared_ptr` and the *navigational, non-owning* links with `weak_ptr`. Typical uses:

- a child's pointer back to its parent, when the parent owns the children,
- graph edges that should let you traverse but not extend lifetime,
- observer lists, where observers should not keep the subject alive,
- caches, where a cached entry may legitimately disappear.

```
shared ownership of a Document:
    editor tab ----+
                   v
                Document
                   ^
background task ---+

non-owning observations:
    outline item --weak_ptr--> Document
    search hit ----weak_ptr--> Document
```

The editor tab and the background task *own* the document (it lives while either holds it). The outline item and the search hit merely *observe* it — they can navigate to it if it still exists, but they will not stop it from being freed once the real owners are done.

### shared_ptr vs weak_ptr vs raw pointer

Putting the four non-owning and shared tools side by side:

| Tool | Owns? | Can be empty / null? | Use when |
|---|---:|---:|---|
| `std::shared_ptr<T>` | Yes, shared | Yes | multiple owners must keep the object alive |
| `std::weak_ptr<T>` | No | Yes | the object is shared-owned elsewhere and may expire |
| `T*` | No (by convention) | Yes | lightweight optional borrowed access |
| `T&` | No (by convention) | No | lightweight required borrowed access |

One boundary to keep clear: `std::weak_ptr` only works with objects managed by a `std::shared_ptr`. It is *not* a general-purpose replacement for every raw pointer. If an object is not shared-owned, a plain `T*` is the right tool for non-owning, nullable access.

---

## 22.x - Chapter 22 summary and quiz

### Summary

This chapter replaced manual memory management with two cooperating ideas: lifetime tied to objects (RAII and smart pointers) and resource transfer instead of duplication (move semantics).

- Raw owning pointers are fragile because cleanup is *separate* from ownership — a single missed `delete` leaks.
- **RAII** ties cleanup to an object's destructor, which runs automatically on every exit path. **Smart pointers** are the standard library's RAII wrappers for heap objects.
- Copying an owner shallowly gives two owners of one resource and a double-delete. The answer is **move semantics**: transfer the resource instead of duplicating it.
- **Rvalue references** (`&&`) bind to expiring values and grant modifiable access — the parameter type that makes moves possible. A **named** rvalue reference variable is itself an *lvalue*.
- A **move constructor** steals a handle and nulls the source; **move assignment** also releases the destination's old resource first and guards against self-assignment. Mark both **`noexcept`**.
- The **rule of five**: a resource-owning class should account for all five special members together — or, better, store its resource in a type (like `unique_ptr`) that already does.
- **`std::move`** is a *cast* to rvalue, not a move; it only *permits* a move that some constructor or operator then performs.
- A **moved-from** standard library object is valid but has an *unspecified* value — assign to it or destroy it, but do not read its old contents.
- **`std::unique_ptr`** is the default for exclusive ownership; it is move-only. Create it with `std::make_unique`; pass it *by value* only to transfer ownership.
- **`std::shared_ptr`** is for genuine shared ownership via a reference-counted control block. Create it with `std::make_shared` and copy it to add owners.
- **`std::weak_ptr`** observes a shared-owned object without extending its lifetime, which breaks the reference cycles that `shared_ptr` cannot collect.

### Ownership decision table

When you reach for a pointer-like tool, this is the question to ask — "who owns this, and for how long?" — and the answer:

| Situation | Prefer |
|---|---|
| One owner of a heap object | `std::unique_ptr<T>` |
| Multiple owners must keep the object alive | `std::shared_ptr<T>` |
| Non-owning optional access | `T*`, or `std::weak_ptr<T>` if the object is shared-owned |
| Non-owning required access | `T&` or `const T&` |
| Dynamic array-like data | `std::vector<T>` before a smart-pointer array |
| String data | `std::string` before an owning `char*` |

### What the lab will make you prove

The chapter lab, `TrackedBuffer`, is built so that move semantics is not something you read about but something you *watch happen*. Every special member increments a static counter, so the tests can tell, unambiguously, which path fired. Make sure you can produce each of these behaviors before you call the chapter done:

- a copy deep-copies (`s_copies` up, two *separate* arrays, editing one does not change the other),
- a move steals the pointer (`s_moves` up, the source's `data()` becomes `nullptr` and `size()` becomes `0`, and the new object holds the *original* address),
- both assignment operators survive self-assignment (`b = b` and `self = std::move(self)` leave a valid object),
- the move operations are `noexcept`,
- `makeTracked` returns ownership via `std::make_unique` — and you do *not* assert a move count on that prvalue return (the C++17 elision trap),
- after `takeOwnership(std::move(p))`, your local `p` is null,
- a `shared_ptr`'s `use_count` rises on copy and falls when an owner leaves scope.

> **Note (CS6340):** When reading or writing LLVM code, run every pointer through the same five questions: *Who owns this object? Is this handle only borrowing from LLVM? Can the pointee be null? Can the pointee outlive this helper? Would moving from this value make later code read an unspecified state?* For Lab 1-style pass code the honest types are usually `llvm::Function& F` (borrowed, required, non-null), `llvm::Instruction* I` (borrowed, nullable), and `std::vector<std::string> labels` (owned by your helper). Use smart pointers for *your own* ownership boundaries — never to claim objects whose lifetime belongs to the LLVM framework.
