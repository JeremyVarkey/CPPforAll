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

### The raw pointer cleanup problem

Raw owning pointers are easy to leak because the pointer variable has no cleanup
behavior attached to it.

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

The bug is not that `new` is always wrong. The bug is that ownership and cleanup
are separated:

```
new creates resource        delete releases resource
        |                           |
        v                           v
    ReportBuffer* buffer ---- many exits can bypass this path
```

Exceptions, early `return`, complicated conditionals, and future edits can all
create paths where `delete` never runs.

### RAII is the underlying idea

RAII means "resource acquisition is initialization." In practical C++ terms:

- acquire the resource in a constructor or factory,
- store ownership in an object,
- release the resource in the object's destructor,
- let block scope, member lifetime, and stack unwinding call destructors.

```cpp
class OwnedInt
{
private:
    int* m_ptr {};

public:
    explicit OwnedInt(int value)
        : m_ptr { new int { value } }
    {
    }

    ~OwnedInt()
    {
        delete m_ptr;
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

When `answer` goes out of scope, `~OwnedInt()` runs automatically. That is the
same basic idea behind standard smart pointers.

### What a smart pointer is

A **smart pointer** is a class that behaves pointer-like while managing the
lifetime of a dynamically allocated object.

Common pointer-like operations:

```cpp
*ptr       // access the owned object
ptr->name  // access a member of the owned object
if (ptr)   // test whether it owns or observes something usable
```

Common ownership operation:

```cpp
// when ptr is destroyed, the managed object is cleaned up
```

Smart pointers are usually stack objects or class members, not dynamically
allocated objects themselves. If a smart pointer is allocated with `new` and then
forgotten, its destructor does not run, so the object it owns may also leak.

### The shallow-copy trap

A first attempt at a smart pointer often looks like this:

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

The destructor is good, but copying is dangerous:

```cpp
BadOwner<Token> a { new Token { "if" } };
BadOwner<Token> b { a };        // compiler-generated shallow copy
```

Now both objects point at the same `Token`.

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

This is the same "shallow vs deep copy" problem from operator overloading and
dynamic allocation, but ownership makes the failure much more severe.

### Copy semantics vs move semantics

**Copy semantics** means the destination receives its own independent copy.

```cpp
std::string a { "abc" };
std::string b { a };        // b has its own string data
```

**Move semantics** means the destination takes over a resource from the source.
The source remains valid, but no longer owns that resource.

```cpp
std::unique_ptr<Token> a { std::make_unique<Token>("if") };
std::unique_ptr<Token> b { std::move(a) }; // b owns; a is now empty
```

Ownership transfer is often the right operation for resources that should have
only one owner:

```
before move:
    a ---> Token("if")
    b ---> null

after move:
    a ---> null
    b ---> Token("if")
```

### Why old std::auto_ptr was flawed

Older C++ had `std::auto_ptr`, which tried to implement ownership transfer using
copy operations. That meant code that looked like a copy could secretly empty the
source object. This broke normal expectations for functions, containers, and
algorithms that assume copying does not modify the copied-from object.

Modern C++ separates these ideas:

| Operation | Meaning |
|---|---|
| Copy constructor / copy assignment | create an independent copy |
| Move constructor / move assignment | transfer resources |

The standard smart pointers covered in this chapter are move-aware.

### CS6340 tie-in

LLVM code often passes around pointers and references to IR objects:

```cpp
bool runOnFunction(llvm::Function& function);
void visitInstruction(llvm::Instruction* instruction);
```

Those parameters usually do **not** mean "this function owns the LLVM object."
The LLVM pass infrastructure owns the module, functions, basic blocks, and
instructions. A pass typically observes or mutates objects through non-owning
references/pointers.

Use smart pointers when your code owns a heap object. Do not wrap borrowed LLVM
IR pointers in `std::unique_ptr` or `std::shared_ptr` unless you are actually
responsible for destroying that object.

---

## 22.2 - R-value references

### Quick lvalue/rvalue recap

An expression has a type and a value category.

```cpp
int x { 5 };

x       // lvalue: names an object with identity
5       // rvalue: temporary value
x + 1   // rvalue: computed temporary
```

Lvalue references use `&`:

```cpp
int& ref { x };          // binds to modifiable lvalue
const int& cref { 5 };   // const lvalue reference can bind to rvalue
```

The `const int&` case is important because it can bind to both lvalues and
rvalues without copying, but it cannot modify the referred value.

### R-value references use &&

C++11 added rvalue references, written with `&&`.

```cpp
int&& tempRef { 5 };
```

An rvalue reference is designed to bind to an rvalue.

| Reference kind | Can bind to modifiable lvalue? | Can bind to rvalue? | Can modify through it? |
|---|---:|---:|---:|
| `int&` | Yes | No | Yes |
| `const int&` | Yes | Yes | No |
| `int&&` | No | Yes | Yes |
| `const int&&` | No | Yes | No |

Non-const rvalue references are what make move semantics possible: they can bind
to temporary or explicitly moved-from objects and modify them as part of
resource transfer.

### Rvalue references can extend a temporary's lifetime

```cpp
struct Pair
{
    int first {};
    int second {};
};

Pair&& pair { Pair { 3, 4 } };
pair.first = 9;

std::cout << pair.first << ", " << pair.second << '\n';
```

The temporary `Pair { 3, 4 }` lives as long as `pair`. This looks unusual, but it
is legal.

Most everyday code does not store rvalue references in local variables this way.
They matter more as function parameters.

### Rvalue reference parameters support overloads

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

logValue(name);                  // lvalue overload
logValue(std::string { "edge" }); // rvalue overload
```

The rvalue overload is allowed to treat `text` as a consumable value. This is the
mechanism that lets classes distinguish "copy this existing object" from "steal
from this expiring object."

### Named rvalue reference variables are lvalues

This rule is easy to forget:

```cpp
void consume(std::string&& value);

void wrapper(std::string&& value)
{
    consume(value);              // error: value is a named variable, so it is an lvalue
    consume(std::move(value));   // ok: explicitly treat it as movable
}
```

The declared type is `std::string&&`, but the expression `value` names an object,
so the expression is an lvalue. Type and value category are separate ideas.

Mental model:

```
std::string&& value
        type: rvalue reference to string
  expression: value is a named object, therefore lvalue
```

### Do not return rvalue references casually

Returning `T&&` from an ordinary function is usually wrong:

```cpp
std::string&& bad()
{
    std::string local { "gone" };
    return std::move(local);     // dangling reference
}
```

The local object dies when the function exits. Return by value instead.

---

## 22.3 - Move constructors and move assignment

### Copy operations recap

For a resource-owning class, default copying is usually shallow copying.

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

If the compiler generates a copy constructor, it copies the pointer value. Two
`Buffer` objects then think they own the same array.

Deep copying fixes correctness:

```cpp
Buffer(const Buffer& other)
    : m_size { other.m_size },
      m_data { new int[other.m_size]{} }
{
    std::copy_n(other.m_data, m_size, m_data);
}
```

But deep copying can be expensive when the source is about to disappear anyway.

### Move construction

A **move constructor** constructs a new object by taking resources from another
object.

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

    Buffer(const Buffer& other)
        : Buffer { other.m_size }
    {
        std::copy_n(other.m_data, m_size, m_data);
    }

    Buffer(Buffer&& other) noexcept
        : m_size { other.m_size },
          m_data { other.m_data }
    {
        other.m_size = 0;
        other.m_data = nullptr;
    }
};
```

Move construction is cheap because it steals a pointer instead of allocating and
copying every element.

```
before move construction:
    other.m_data ---> [ 0 0 0 0 ]
    new object   ---> not built yet

after move construction:
    other.m_data ---> nullptr
    new.m_data   ---> [ 0 0 0 0 ]
```

The moved-from object must be left in a state that can be safely destructed.
Setting pointer members to `nullptr` is common because `delete nullptr` and
`delete[] nullptr` are safe.

### Move assignment

Move assignment transfers resources into an object that already exists. It must
first release the destination's current resource.

```cpp
Buffer& operator=(Buffer&& other) noexcept
{
    if (this == &other)
        return *this;

    delete[] m_data;             // release current resource

    m_size = other.m_size;       // steal source resource
    m_data = other.m_data;

    other.m_size = 0;            // leave source destructible
    other.m_data = nullptr;

    return *this;
}
```

Move assignment order:

```
1. detect self-assignment
2. release destination's old resource
3. copy pointer/handle values from source
4. neutralize source
5. return *this
```

### Move operations are selected for rvalues

```cpp
Buffer makeBuffer()
{
    Buffer temp { 1024 };
    return temp;                 // may elide; otherwise can move
}

Buffer a { 16 };
Buffer b { makeBuffer() };       // move construction if not elided

Buffer c { 32 };
c = makeBuffer();                // move assignment if not elided
```

Modern compilers often elide the move entirely, but the type still needs
reasonable move behavior when elision is not possible or not mandatory.

### noexcept matters

Move constructors and move assignment operators are often marked `noexcept`.

```cpp
Buffer(Buffer&& other) noexcept;
Buffer& operator=(Buffer&& other) noexcept;
```

Standard containers such as `std::vector` prefer moves during reallocation when
the move operation is known not to throw. If a move could throw, a container may
copy instead to preserve stronger exception guarantees.

### The rule of five

The rule of five says that if a class defines or deletes any one of these, it
should consider defining or deleting all five:

| Special member | Why it matters |
|---|---|
| Destructor | releases owned resource |
| Copy constructor | creates new object from existing lvalue |
| Copy assignment | overwrites existing object from lvalue |
| Move constructor | creates new object from expiring value |
| Move assignment | overwrites existing object from expiring value |

Resource-owning classes should make ownership rules explicit. Many modern
classes avoid writing these manually by storing resources in `std::unique_ptr`,
`std::vector`, `std::string`, or another RAII type.

### Move-only types

Some resources cannot be meaningfully copied. For those, delete copy operations
and allow moves:

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

    OutputFile(const OutputFile&) = delete;
    OutputFile& operator=(const OutputFile&) = delete;

    OutputFile(OutputFile&&) noexcept = default;
    OutputFile& operator=(OutputFile&&) noexcept = default;
};
```

This communicates "there is one owner, but ownership may be transferred."

---

## 22.4 - std::move

### std::move does not move by itself

`std::move` casts an expression to an rvalue reference. It is a request to allow
move semantics. The actual move happens only if the selected constructor or
assignment operator performs one.

```cpp
#include <utility>

std::string name { "coverage" };
std::string saved { std::move(name) };
```

`std::move(name)` says "treat `name` as something that may be moved from."

### Why swaps benefit from moves

A copy-based swap makes three copies:

```cpp
template <typename T>
void swapByCopy(T& a, T& b)
{
    T temp { a };
    a = b;
    b = temp;
}
```

A move-based swap transfers resources instead:

```cpp
#include <utility>

template <typename T>
void swapByMove(T& a, T& b)
{
    T temp { std::move(a) };
    a = std::move(b);
    b = std::move(temp);
}
```

For small `int` values, this does not matter. For `std::string`,
`std::vector`, or resource-owning classes, moving can avoid allocations and
element-by-element copies.

### Moving into containers

```cpp
std::vector<std::string> names {};

std::string generated { "mutation-operator" };
names.push_back(generated);            // copy; generated remains unchanged

std::string temporary { "branch-id" };
names.push_back(std::move(temporary));  // move; temporary is valid but unspecified
```

After the move, `temporary` can be destroyed or assigned a new value. Do not
write logic that depends on its old contents still being present.

### Moved-from objects are valid but unspecified

For standard library types, a moved-from object remains valid. You can:

```cpp
temporary.clear();
temporary = "new-name";
if (temporary.empty()) { /* ok */ }
```

But do not do this:

```cpp
char first { temporary[0] }; // unsafe assumption after move
```

The object might be empty, or it might hold some other valid state.

### When std::move is useful

Use `std::move` when:

- transferring ownership from one smart pointer to another,
- moving a large local object into a container,
- implementing move constructors or move assignment,
- writing generic code that swaps or relocates values,
- returning an object only in cases where normal return value optimization does
  not already express the intent.

Avoid `std::move` when:

- you still need the current value afterward,
- the object is `const` and therefore cannot be meaningfully moved from,
- returning a local by value in normal C++17+ code where copy elision applies.

```cpp
std::string makeName()
{
    std::string name { "instrumented" };
    return name;                // prefer this; let elision/move rules work
}
```

### CS6340 tie-in

Move semantics can show up in Lab 1 helper code even if LLVM itself is mostly
passed by pointer/reference:

```cpp
std::vector<std::string> labels {};

std::string label { makeCoverageLabel(functionName, blockIndex) };
labels.push_back(std::move(label));     // ok if label will not be read again
```

The important discipline is to treat moved-from values as consumed. Do not log,
compare, or build later identifiers from a value after moving from it unless you
first assign it a new value.

---

## 22.5 - std::unique_ptr

### Unique ownership

`std::unique_ptr<T>` owns one dynamically allocated `T`. It is the default smart
pointer for exclusive ownership.

```cpp
#include <memory>

auto node { std::make_unique<Node>("entry") };
```

Ownership is unique:

```cpp
auto a { std::make_unique<Node>("entry") };

// auto b { a };                // error: copying disabled
auto b { std::move(a) };        // ok: ownership transfers
```

After the move, `a` is empty and `b` owns the `Node`.

### Prefer std::make_unique

Prefer:

```cpp
auto token { std::make_unique<Token>("identifier", 12) };
```

over:

```cpp
std::unique_ptr<Token> token { new Token { "identifier", 12 } };
```

`std::make_unique` is shorter, clearer, and avoids edge cases where allocation
and ownership establishment are separated.

### Accessing the owned object

```cpp
auto block { std::make_unique<BlockInfo>(7) };

if (block)
{
    std::cout << block->id() << '\n';
    std::cout << (*block).id() << '\n';
}
```

`std::unique_ptr` can be empty, so test it before dereferencing when emptiness is
possible.

### Returning unique_ptr by value

Returning a `std::unique_ptr` by value is normal.

```cpp
std::unique_ptr<TraceFile> openTraceFile(const std::string& path)
{
    auto file { std::make_unique<TraceFile>(path) };

    if (!file->isOpen())
        return nullptr;

    return file;
}
```

The caller receives ownership:

```cpp
auto trace { openTraceFile("coverage.log") };
if (trace)
    trace->write("start");
```

Do not return `std::unique_ptr` by raw pointer or reference. That obscures
ownership and can produce dangling references.

### Passing unique_ptr to functions

Use the parameter type to communicate ownership.

```cpp
void takeOwnership(std::unique_ptr<TraceFile> file)
{
    file->write("owned here");
} // file closes here

void observeFile(const TraceFile& file)
{
    std::cout << file.path() << '\n';
}

void maybeObserveFile(const TraceFile* file)
{
    if (file)
        std::cout << file->path() << '\n';
}
```

Call sites:

```cpp
auto file { std::make_unique<TraceFile>("out.txt") };

observeFile(*file);             // no ownership transfer
maybeObserveFile(file.get());   // no ownership transfer; nullable

takeOwnership(std::move(file)); // ownership transfer; file is now empty
```

General rule:

| Function intent | Parameter style |
|---|---|
| Take ownership | `std::unique_ptr<T>` by value |
| Observe required object | `T&` or `const T&` |
| Observe optional object | `T*` or `const T*` |

### unique_ptr and arrays

`std::unique_ptr<T[]>` can manage a dynamic array, but prefer standard containers
when possible:

```cpp
std::vector<int> counters(100);     // usually better
std::string text;                   // better than unique_ptr<char[]>
```

Use `std::unique_ptr<T[]>` only when a real ownership boundary requires a raw
dynamic array shape.

### release, reset, and get

Useful member functions:

```cpp
auto ptr { std::make_unique<Node>("root") };

Node* raw { ptr.get() };        // observe only; ptr still owns

ptr.reset();                   // deletes owned object; ptr becomes empty

Node* leaked { ptr.release() }; // ptr stops owning; caller must delete or re-own
delete leaked;
```

`release()` is advanced and easy to misuse. Most code should not need it.

### Misuse patterns

Do not create two `std::unique_ptr` objects from the same raw pointer:

```cpp
Node* raw { new Node("bad") };

std::unique_ptr<Node> a { raw };
std::unique_ptr<Node> b { raw };    // undefined behavior later: double delete
```

Do not manually delete the raw pointer held by a `std::unique_ptr`:

```cpp
auto node { std::make_unique<Node>("bad") };
delete node.get();                  // undefined behavior later
```

If ownership is in a `std::unique_ptr`, let the `std::unique_ptr` own it.

---

## 22.6 - std::shared_ptr

### Shared ownership

`std::shared_ptr<T>` represents shared ownership. Multiple `shared_ptr` objects
can co-own the same resource. The resource is destroyed when the last owner goes
away.

```cpp
#include <memory>

auto config { std::make_shared<Config>("lab") };

auto a { config };
auto b { config };

// Config remains alive while config, a, or b owns it.
```

Mental model:

```
shared_ptr ---> resource pointer ---> Config
          \
           -> control block:
                strong owners: 3
                weak observers: 0
```

The control block tracks ownership counts and knows when to destroy the managed
object.

### Prefer std::make_shared

```cpp
auto table { std::make_shared<LookupTable>() };
```

`std::make_shared` is simpler and often more efficient because the resource and
control block can be allocated together.

### Copy an existing shared_ptr

If you need another shared owner, copy the existing `std::shared_ptr`.

```cpp
auto first { std::make_shared<Node>("root") };
auto second { first };              // correct: shares control block
```

Do not independently create two shared pointers from the same raw pointer:

```cpp
Node* raw { new Node("root") };

std::shared_ptr<Node> first { raw };
std::shared_ptr<Node> second { raw }; // wrong: separate control blocks
```

Each control block would think it is the only owner, leading to double deletion.

### shared_ptr is for real shared lifetime

Do not use `std::shared_ptr` just to avoid thinking about ownership. It has
runtime overhead and makes lifetime less local.

Good cases:

- a resource must live until multiple independent owners are finished with it,
- callbacks or tasks need to keep a shared state object alive,
- graph-like structures genuinely share nodes.

Poor cases:

- one clear owner exists,
- callers only need temporary access,
- the object is owned by some external framework.

For clear ownership, prefer `std::unique_ptr`. For non-owning access, prefer
references or raw pointers.

### unique_ptr can become shared_ptr

Ownership can move from unique to shared:

```cpp
std::unique_ptr<Config> unique { std::make_unique<Config>() };
std::shared_ptr<Config> shared { std::move(unique) };
```

The reverse is not generally safe because a `std::shared_ptr` may have multiple
owners. You cannot prove unique ownership just from a shared pointer's type.

### shared_ptr and arrays

Older C++ standards did not handle arrays through `std::shared_ptr` as cleanly
as scalar objects. Even in modern code, `std::vector<T>` or `std::array<T, N>` is
usually the better abstraction for arrays.

### CS6340 tie-in

LLVM pass code usually should not use `std::shared_ptr` for IR objects. The pass
manager, module, and context own those objects. If a helper needs to remember an
instruction temporarily, store a raw pointer or reference with clear lifetime
assumptions:

```cpp
std::vector<llvm::Instruction*> interestingInstructions;
```

That says "these are borrowed pointers." It does not pretend the helper owns the
instructions.

---

## 22.7 - Circular dependency issues with std::shared_ptr, and std::weak_ptr

### The shared_ptr cycle problem

Reference-counted ownership cannot automatically collect cycles.

```cpp
class Node
{
public:
    std::string name {};
    std::shared_ptr<Node> neighbor {};
};

auto left { std::make_shared<Node>() };
auto right { std::make_shared<Node>() };

left->neighbor = right;
right->neighbor = left;
```

At the end of the scope:

```
left variable releases left Node
right variable releases right Node

but:
    left Node owns right Node through neighbor
    right Node owns left Node through neighbor

strong counts never reach zero
```

Both nodes leak because each keeps the other alive.

### std::weak_ptr is a non-owning observer

`std::weak_ptr<T>` observes an object managed by `std::shared_ptr<T>` without
increasing the strong owner count.

```cpp
class Node
{
public:
    std::string name {};
    std::weak_ptr<Node> neighbor {};    // observes, does not own
};
```

Now the back-link or cross-link does not keep the target alive.

### weak_ptr must be locked before use

A `std::weak_ptr` does not provide `operator->` directly because the target may
already be destroyed. Convert it to a `std::shared_ptr` temporarily with
`lock()`.

```cpp
void printNeighbor(const std::weak_ptr<Node>& weak)
{
    if (auto node { weak.lock() })
        std::cout << node->name << '\n';
    else
        std::cout << "neighbor expired\n";
}
```

`lock()` returns:

- a non-empty `std::shared_ptr` if the object is still alive,
- an empty `std::shared_ptr` if the object has expired.

You can also ask directly:

```cpp
if (weak.expired())
    std::cout << "already gone\n";
```

### Where weak_ptr fits

Use `std::weak_ptr` for:

- parent pointers when children are owned by parents,
- graph edges that should not extend lifetime,
- observer lists,
- caches where cached objects may disappear,
- relationships that should be navigable but not owning.

Example ownership shape:

```
Document shared ownership:
    editor tab ----+
                   v
                Document
                   ^
background task ---+

non-owning observations:
    outline item --weak_ptr--> Document
    search hit ----weak_ptr--> Document
```

### shared_ptr vs weak_ptr vs raw pointer

| Tool | Owns? | Can be empty? | Use when |
|---|---:|---:|---|
| `std::shared_ptr<T>` | Yes, shared | Yes | multiple owners must keep object alive |
| `std::weak_ptr<T>` | No | Yes | object is shared-owned elsewhere and may expire |
| `T*` | No by convention | Yes | lightweight optional borrowed access |
| `T&` | No by convention | No | lightweight required borrowed access |

`std::weak_ptr` only works with objects managed by `std::shared_ptr`. It is not a
general replacement for every raw pointer.

---

## 22.x - Chapter 22 summary and quiz

### Summary

- Raw owning pointers are fragile because cleanup is separate from ownership.
- Smart pointers use RAII to tie cleanup to object lifetime.
- Move semantics transfer resources instead of copying them.
- Rvalue references use `&&` and are the key parameter type for move operations.
- A named rvalue reference variable is an lvalue expression.
- `std::move` is a cast that permits moving; it does not move by itself.
- Moved-from standard library objects are valid but have unspecified values.
- `std::unique_ptr` is the default choice for exclusive dynamic ownership.
- Pass `std::unique_ptr` by value only when transferring ownership.
- `std::shared_ptr` is for real shared ownership and uses a control block.
- Create shared objects with `std::make_shared` and copy existing `shared_ptr`
  objects to share ownership.
- `std::weak_ptr` observes shared-owned objects without extending lifetime and
  breaks cycles.

### Ownership decision table

| Situation | Prefer |
|---|---|
| One owner of a heap object | `std::unique_ptr<T>` |
| Multiple owners keep object alive | `std::shared_ptr<T>` |
| Non-owning optional access | `T*` or `std::weak_ptr<T>` depending on ownership model |
| Non-owning required access | `T&` or `const T&` |
| Dynamic array-like data | `std::vector<T>` before smart pointer arrays |
| String data | `std::string` before owning `char*` |

### CS6340 memory-management checklist

When reading or writing LLVM code, ask:

1. Who owns this object?
2. Is this pointer/reference only borrowing from LLVM?
3. Can the pointee be null?
4. Can the pointee outlive this helper object?
5. Would moving from this value make later code read an unspecified state?

For Lab 1-style pass code, the answer is often:

```cpp
llvm::Function& F       // borrowed, required, non-null
llvm::Instruction* I    // borrowed, optional or nullable
std::vector<std::string> labels // owned by this helper
```

Use smart pointers for your own ownership boundaries, not for objects whose
lifetime belongs to the LLVM framework.
