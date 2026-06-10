# Chapter 19 — Dynamic Allocation

> Source: <https://www.learncpp.com/> (Chapter 19)
> One file per chapter. Each lesson is a section below.

## Contents
- [19.1 — Dynamic memory allocation with new and delete](#191--dynamic-memory-allocation-with-new-and-delete)
- [19.2 — Dynamically allocating arrays](#192--dynamically-allocating-arrays)
- [19.3 — Destructors](#193--destructors)
- [19.4 — Pointers to pointers and dynamic multidimensional arrays](#194--pointers-to-pointers-and-dynamic-multidimensional-arrays)
- [19.5 — Void pointers](#195--void-pointers)
- [Chapter 19 study summary](#chapter-19-study-summary)

---

## 19.1 — Dynamic memory allocation with new and delete

### Three places objects can live

C++ programs commonly use three allocation patterns:

| Allocation kind | Typical examples | Lifetime |
|---|---|---|
| Static | globals, static locals | whole program or from first use until program end |
| Automatic | function parameters, ordinary local variables | block entry to block exit |
| Dynamic | objects created with `new` | until explicitly released with `delete` |

Static and automatic allocation are managed for you. Dynamic allocation asks the
programmer to manage an object whose lifetime is not tied to the scope where it
was created.

```
automatic:
    enter function -> create local object -> leave function -> destroy object

dynamic:
    call new -> object exists somewhere on heap -> call delete -> object ends
```

Dynamic allocation is useful when the amount of data is not known at compile
time or when an object must outlive the function that creates it. Modern C++
usually wraps dynamic allocation inside standard library types (`std::vector`,
`std::string`, smart pointers), but reading raw `new` / `delete` code is still
important for understanding ownership and older code.

CS6340 tie-in: LLVM APIs expose many pointers and references. Most starter-code
work will not manually `new` LLVM objects, but the ownership question is always:
"who created this object, and who is responsible for destroying it?"

### The heap

Automatic variables are normally stored in a function's stack frame. Dynamically
allocated objects are stored in a separate memory region often called the heap
or free store.

```
stack frame for parse():
    local int count
    local Token* current

heap:
    Token object created with new
    vector storage owned internally by std::vector
```

The stack is fast and scope-bound. The heap is more flexible, but access usually
goes through an address and cleanup is manual unless a library type owns the
allocation for you.

### Allocating one object

The scalar form of `new` creates one object and returns its address:

```cpp
int* score { new int };
*score = 42;

std::cout << *score << '\n';

delete score;
score = nullptr;
```

Two separate things are happening:

1. `new int` creates an `int` object in dynamic storage.
2. `score` stores the address of that object.

The pointer is not the dynamic object. It is only the handle you use to reach the
object.

```
score variable on stack:
    score --------+
                  |
heap object:      v
                int { 42 }
```

If the only pointer to a dynamic object is lost, the program has no way to
release that object.

### Initializing dynamic objects

Prefer initializing the object at the point of allocation:

```cpp
int* exact { new int { 7 } };
double* average { new double { 0.0 } };

delete exact;
delete average;
```

This follows the same habit as normal variables: create an object in a known
state instead of creating it and assigning later.

### Deleting one object

`delete` ends the lifetime of the object pointed to by a pointer and releases
the dynamic storage:

```cpp
int* value { new int { 5 } };

delete value;     // dynamic int is destroyed
value = nullptr;  // value no longer points at released storage
```

After `delete`, the pointer variable still exists, but the object it pointed to
does not. That is why setting the pointer to `nullptr` is useful when the pointer
will continue to exist.

Deleting a null pointer is harmless:

```cpp
int* value {};
delete value; // ok: no effect
```

There is no need to write an `if (value)` guard before every `delete`.

### Dangling pointers

A dangling pointer stores an address for an object whose lifetime has ended.
Using it is undefined behavior.

```cpp
int* first { new int { 10 } };
int* alias { first };

delete first;
first = nullptr;

// alias still holds the old address.
// *alias would be undefined behavior.
```

The hard part is that nulling one pointer does not fix every other pointer that
copied the same address.

Ownership mental model:

| Pointer role | Meaning |
|---|---|
| Owning pointer | responsible for deleting the dynamic object |
| Observing pointer | may inspect the object but must not delete it |

Raw pointers do not encode this distinction. A future reader has to infer it
from naming, comments, or surrounding convention.

### Allocation failure

Ordinary `new` throws `std::bad_alloc` if allocation fails. There is also a
`std::nothrow` form that returns `nullptr` instead:

```cpp
#include <new>

int* buffer { new (std::nothrow) int { 0 } };

if (!buffer)
{
    std::cerr << "allocation failed\n";
}

delete buffer;
```

Most beginner code does not handle heap exhaustion carefully, but production
code needs a policy: throw, return an error, use a bounded allocator, or avoid
manual allocation in that path.

### Memory leaks

A memory leak happens when allocated dynamic storage is not released and the
program loses the ability to release it.

```cpp
void leakOne()
{
    int* count { new int { 1 } };
} // count is destroyed, but the dynamic int was never deleted
```

Another leak pattern is overwriting the only owning pointer:

```cpp
int* p { new int { 1 } };
p = new int { 2 }; // first allocation is now unreachable

delete p;          // releases only the second allocation
```

Correct raw-pointer code must keep the owner alive until cleanup:

```cpp
int* p { new int { 1 } };
delete p;
p = new int { 2 };
delete p;
```

In modern C++, prefer not writing this manually:

```cpp
#include <memory>

auto count { std::make_unique<int>(1) };
```

`std::unique_ptr` will delete the object when the smart pointer is destroyed.
That does not remove the lifetime problem, but it makes ownership explicit and
scope-bound.

---

## 19.2 — Dynamically allocating arrays

### Array new and array delete

The array form of `new` creates multiple elements:

```cpp
int length {};
std::cin >> length;

int* values { new int[length]{} };

values[0] = 10;

delete[] values;
values = nullptr;
```

The brackets matter:

| Allocation | Correct release |
|---|---|
| `new int` | `delete ptr` |
| `new int[n]` | `delete[] ptr` |

Mixing scalar `delete` with array `new[]`, or array `delete[]` with scalar
`new`, is undefined behavior.

### Dynamic arrays are still array-like

A dynamically allocated array can be subscripted:

```cpp
double* samples { new double[3] { 1.0, 2.0, 4.0 } };

for (int i { 0 }; i < 3; ++i)
    std::cout << samples[i] << '\n';

delete[] samples;
```

But the pointer does not know the length in a way ordinary code can query:

```cpp
double* samples { new double[3]{} };

// std::size(samples) does not work here.
// sizeof(samples) is the size of the pointer, not the array.

delete[] samples;
```

If code receives only `double*`, it also needs a separate length parameter or a
sentinel convention.

CS6340 tie-in:

```cpp
void processCounters(unsigned* counters, int counterCount);
```

That signature has the classic C-style split: one parameter points to the first
element, and another explains how many elements are valid. The function must
respect both.

### Dynamic arrays can be initialized

Value-initialize dynamic arrays with `{}` when you want a clean starting state:

```cpp
int* counts { new int[5]{} }; // all elements become 0

delete[] counts;
```

Initializer lists can provide explicit starting values:

```cpp
int* priorities { new int[4] { 3, 1, 4, 1 } };

delete[] priorities;
```

### Resizing means allocating a new array

A raw dynamic array has a fixed length after allocation. To "resize" it, allocate
new storage, copy the old elements you want to keep, delete the old storage, then
redirect the pointer.

```cpp
int oldLength { 3 };
int* data { new int[oldLength] { 2, 4, 6 } };

int newLength { 5 };
int* larger { new int[newLength]{} };

for (int i { 0 }; i < oldLength; ++i)
    larger[i] = data[i];

delete[] data;
data = larger;
larger = nullptr;

delete[] data;
```

This is exactly the sort of bookkeeping `std::vector` exists to handle:

```cpp
#include <vector>

std::vector<int> data { 2, 4, 6 };
data.resize(5);
```

Prefer `std::vector` unless a lab, API, or performance constraint specifically
requires raw arrays.

### Why raw dynamic arrays are fragile

Raw dynamic arrays combine several risks:

- The pointer does not carry a visible length.
- Cleanup must use `delete[]`.
- Resizing is manual.
- Copying the pointer does not copy the elements.
- Ownership is unclear from the type alone.

`std::vector<T>` is normally the right dynamic array abstraction because it owns
the allocation and keeps the size and capacity with the data.

---

## 19.3 — Destructors

### What a destructor is

A destructor is a special member function that runs when an object is destroyed.
It is the class's cleanup hook.

```cpp
class Trace
{
public:
    Trace() { std::cout << "construct\n"; }
    ~Trace() { std::cout << "destroy\n"; }
};

int main()
{
    Trace t{};
} // t's destructor runs here
```

Destructor naming:

```cpp
class Buffer
{
public:
    ~Buffer(); // class name preceded by ~
};
```

A destructor has no return type and no parameters. A class can have only one
destructor.

### Destructors and dynamic ownership

Destructors are the foundation of RAII: Resource Acquisition Is Initialization.
The object acquires a resource in construction, owns it during its lifetime, and
releases it in destruction.

```cpp
class IntBuffer
{
private:
    int* m_data {};
    int m_length {};

public:
    explicit IntBuffer(int length)
        : m_data { new int[length]{} }
        , m_length { length }
    {
    }

    ~IntBuffer()
    {
        delete[] m_data;
    }

    int length() const { return m_length; }
    int& operator[](int index) { return m_data[index]; }
};
```

Usage:

```cpp
void run()
{
    IntBuffer counters { 10 };
    counters[0] = 1;
} // delete[] happens automatically through IntBuffer::~IntBuffer()
```

The caller does not need to remember `delete[]`. The type owns the cleanup.

### Construction and destruction order

For automatic objects in the same scope, destruction happens in reverse order of
construction.

```cpp
Trace first{};
Trace second{};
Trace third{};

// destruction order: third, second, first
```

Mental model:

```
construct: first -> second -> third
destroy:   third -> second -> first
```

This reverse order matters when one object depends on another. The dependent
object should usually be created after the object it depends on, so it is
destroyed before that dependency disappears.

### `delete` calls destructors

When a class object is dynamically allocated, `delete` runs the destructor before
releasing storage:

```cpp
Trace* trace { new Trace{} };
delete trace; // calls ~Trace(), then releases memory
```

For dynamic arrays, `delete[]` runs the destructor for each element:

```cpp
Trace* traces { new Trace[3]{} };
delete[] traces;
```

This is another reason the correct `delete` form matters.

### `std::exit()` warning

Returning from `main` unwinds ordinary automatic objects in scope. Calling
`std::exit()` terminates the program more abruptly and does not clean up normal
local automatic objects in the same way.

```cpp
void stopNow()
{
    Trace t{};
    std::exit(1); // t's destructor is not the ordinary scope-exit path
}
```

In code that owns files, locks, temporary directories, or heap memory, bypassing
destructors can skip important cleanup. Prefer returning errors or throwing
exceptions through normal cleanup paths when possible.

### RAII is the main lesson

Raw `new` and `delete` are easy to misuse. RAII moves the cleanup into a
destructor so the resource follows object lifetime:

| Resource | RAII owner |
|---|---|
| dynamic array | `std::vector<T>` or custom owner class |
| single dynamic object | `std::unique_ptr<T>` |
| file handle | `std::fstream` |
| lock | `std::lock_guard` |

In modern C++, "write a destructor" usually means you are building a resource
owning type. If the class only stores ordinary values, the compiler-generated
destructor is normally enough.

---

## 19.4 — Pointers to pointers and dynamic multidimensional arrays

### Pointer to pointer syntax

A pointer stores an address. A pointer to pointer stores the address of a
pointer.

```cpp
int value { 7 };
int* ptr { &value };
int** ptrToPtr { &ptr };

std::cout << **ptrToPtr << '\n'; // prints 7
```

Diagram:

```
ptrToPtr --> ptr --> value
              |       |
              |       +-- int object
              +---------- pointer object
```

Read from the inside out:

```cpp
int** ptrToPtr;
// ptrToPtr is a pointer to a pointer to int
```

### Why this appears

Pointers to pointers show up when code needs to:

- modify a pointer variable through another pointer,
- represent an array of pointers,
- interface with C libraries,
- build old-style dynamic multidimensional arrays.

For normal modern C++ code, prefer references, containers, and smart pointers.

### Dynamic two-dimensional arrays with row pointers

One old C++ technique is to allocate an array of row pointers, then allocate
each row separately:

```cpp
int rows { 3 };
int cols { 4 };

int** grid { new int*[rows]{} };

for (int r { 0 }; r < rows; ++r)
    grid[r] = new int[cols]{};

grid[1][2] = 9;

for (int r { 0 }; r < rows; ++r)
    delete[] grid[r];

delete[] grid;
```

Memory shape:

```
grid
 |
 v
+---------+     +---+---+---+---+
| row 0 --+---> | 0 | 0 | 0 | 0 |
+---------+     +---+---+---+---+
| row 1 --+---> | 0 | 0 | 9 | 0 |
+---------+     +---+---+---+---+
| row 2 --+---> | 0 | 0 | 0 | 0 |
+---------+     +---+---+---+---+
```

This shape allows rows to have different lengths, but it creates many
allocations and a lot of cleanup paths.

### Cleanup must mirror allocation

Every successful row allocation needs a matching `delete[]`, and the outer array
of pointers also needs a matching `delete[]`.

Failure-prone pattern:

```cpp
int** grid { new int*[rows]{} };

for (int r { 0 }; r < rows; ++r)
    grid[r] = new int[cols]{};

// If an early return happens here, cleanup is skipped.
```

RAII is safer:

```cpp
#include <vector>

std::vector<std::vector<int>> grid(rows, std::vector<int>(cols));
grid[1][2] = 9;
```

Even better for many numeric/dataflow cases, use one flat vector and compute the
index:

```cpp
std::vector<int> grid(rows * cols);

auto index = [cols](int row, int col)
{
    return row * cols + col;
};

grid[index(1, 2)] = 9;
```

Flat storage is usually more cache-friendly than separately allocated rows.

### Pointers to pointers vs references to pointers

If C++ code needs to change a caller's pointer, a reference to pointer is often
clearer than a pointer to pointer:

```cpp
void allocateOne(int*& out)
{
    out = new int { 5 };
}

int* value {};
allocateOne(value);
delete value;
```

In C APIs, the equivalent shape often uses `int**`.

CS6340 tie-in: LLVM APIs often pass objects by pointer or reference, but most
lab code should not manufacture nested raw pointer ownership unless the starter
code requires it. Prefer local containers and let LLVM own its IR objects.

---

## 19.5 — Void pointers

### What `void*` means

A `void*` can hold the address of an object without saying what type of object
lives there.

```cpp
int count { 3 };
void* erased { &count };
```

The type has been erased from the pointer. The compiler knows `erased` is an
address, but it does not know whether the pointed-to object is an `int`, a
`double`, a `Token`, or something else.

### You must cast before dereferencing

Because `void*` has no pointed-to type, it cannot be dereferenced directly:

```cpp
int count { 3 };
void* erased { &count };

int* restored { static_cast<int*>(erased) };
std::cout << *restored << '\n';
```

The cast must match the real object type. Casting to the wrong type and reading
through that pointer produces undefined behavior.

```cpp
double* wrong { static_cast<double*>(erased) };
// *wrong is undefined behavior: the object is actually an int
```

### Void pointer limitations

Important constraints:

- You cannot dereference a `void*` without converting it first.
- You cannot do meaningful pointer arithmetic on standard `void*`.
- A `void*` does not know array length, ownership, or constness beyond its own
  pointer type.
- The compiler cannot type-check the use after the type is erased.

If the object should not be modified, preserve const:

```cpp
const int count { 3 };
const void* erased { &count };
```

Dropping constness is a separate and dangerous operation.

### Why `void*` exists

`void*` is mostly useful for low-level C-style interfaces where data is passed
through a generic callback or storage slot.

```cpp
using Callback = void (*)(void*);

void printInt(void* data)
{
    auto* value { static_cast<int*>(data) };
    std::cout << *value << '\n';
}
```

This works, but the type contract is outside the type system. The caller and
callee must agree by convention.

Modern C++ alternatives are usually better:

| Need | Prefer |
|---|---|
| generic function over many types | templates |
| one of several known types | `std::variant` |
| owning dynamic object | smart pointer |
| byte-level memory view | `std::byte*` or `std::span<std::byte>` |
| polymorphic behavior | base-class pointer/reference |

### How to read `void*` in real code

When you see `void*`, ask:

1. What concrete type is actually stored there?
2. Who owns the pointed-to object?
3. Who is allowed to cast it back?
4. How is lifetime guaranteed?
5. What happens if the wrong callback or wrong cast is used?

For CS6340 lab code, a `void*` is a warning sign that you are outside the
friendly part of the C++ type system. Treat it as low-level glue, not as an
ordinary data model.

---

## Chapter 19 study summary

### Core takeaways

- Dynamic allocation creates objects whose lifetime is controlled explicitly.
- `new` must be paired with `delete`; `new[]` must be paired with `delete[]`.
- A pointer can outlive the object it points at; that creates dangling-pointer
  risk.
- Losing the only owning pointer before cleanup creates a memory leak.
- Destructors make cleanup automatic at object destruction time.
- RAII is the idiomatic C++ solution to raw resource management.
- `int**` means "pointer to pointer to int"; it is often a sign of low-level or
  C-style code.
- `void*` erases type information and should be replaced with typed alternatives
  whenever practical.

### Modern preference ladder

When choosing an ownership representation, prefer higher-level tools first:

| Use case | Prefer |
|---|---|
| runtime-sized sequence | `std::vector<T>` |
| text | `std::string` |
| optional ownership of one object | `std::unique_ptr<T>` |
| shared ownership | `std::shared_ptr<T>` only when ownership is truly shared |
| non-owning access | reference, pointer, `std::span<T>` |
| raw allocation | only for learning, low-level libraries, or required APIs |

### CS6340 lens

LLVM code uses many pointers, references, and ownership conventions. The main
lesson is not "use `new` everywhere." The main lesson is to read signatures
accurately:

```cpp
Function& F        // existing function object, not nullable
Module* M          // address of module object, maybe check convention/nullness
Instruction* I     // pointer to an instruction, lifetime owned elsewhere
std::vector<T> xs  // container owns dynamic storage internally
```

Before mutating or storing any pointer, identify whether you own the object or
are merely observing an object owned by the framework.
