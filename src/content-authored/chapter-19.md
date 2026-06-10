# Chapter 19 — Dynamic Allocation

> Source: <https://www.learncpp.com/> (Chapter 19)

Until now, every object you have created has had its lifetime decided for you. A local variable is born when control enters its block and dies when control leaves; a global lives for the whole program. You never had to ask for memory, and you never had to give it back — the compiler wove the bookkeeping into the structure of your code. That arrangement is comfortable, and for most of what you write it is exactly right.

But it has a hard edge. The compiler can only manage what it can see at compile time. If you do not know until the program runs how many sensor readings will arrive, how long a user's input will be, or how many nodes a tree will grow, then a fixed-size local variable cannot hold the answer — its size was decided before the question was asked. And if an object needs to outlive the function that creates it, scope-bound lifetime is precisely the wrong tool: the object you want to keep gets destroyed the moment you return.

This chapter introduces the third way objects can live: **dynamic allocation**. You ask the runtime for storage with `new`, you use it for as long as you like, and you hand it back with `delete`. The size can be decided at run time, and the lifetime is whatever you choose — which is the power, and also the danger. Now *you* are responsible for the cleanup the compiler used to handle, and the language will not stop you from forgetting. Most modern C++ hides raw `new` and `delete` inside safer types like `std::vector` and smart pointers, and you should reach for those first. But to understand why those types exist — and to read the older code and low-level APIs that still use the raw machinery — you need to spend a chapter doing it by hand. By the end you will feel exactly how fragile manual memory management is, which is the best possible motivation for the abstractions that replace it.

## Contents
- [19.1 — Dynamic memory allocation with new and delete](#191--dynamic-memory-allocation-with-new-and-delete)
- [19.2 — Dynamically allocating arrays](#192--dynamically-allocating-arrays)
- [19.3 — Destructors](#193--destructors)
- [19.4 — Pointers to pointers and dynamic multidimensional arrays](#194--pointers-to-pointers-and-dynamic-multidimensional-arrays)
- [19.5 — Void pointers](#195--void-pointers)
- [Chapter 19 study summary](#chapter-19-study-summary)

---

## 19.1 — Dynamic memory allocation with new and delete

### Three places an object can live

Every object in a C++ program is allocated in one of three ways, and the difference is entirely about *when the object is born and when it dies*.

| Allocation kind | Typical examples | Lifetime |
|---|---|---|
| **Static** | globals, `static` locals | the whole program (or from first use until program end) |
| **Automatic** | function parameters, ordinary local variables | from block entry to block exit |
| **Dynamic** | objects created with `new` | from `new` until you explicitly `delete` it |

The first two are managed for you. A static object is set up before `main` begins; an automatic object appears when its declaration is reached and vanishes when its scope ends. You write the declaration, and the lifetime follows mechanically.

Dynamic allocation breaks that link. You are asking for an object whose lifetime is *not* tied to the scope where it was created — and so the language hands you both halves of the job:

```
automatic:
    enter function  ->  create local object  ->  leave function  ->  destroy object

dynamic:
    call new  ->  object exists on the heap  ->  ... (as long as you like) ...  ->  call delete  ->  object ends
```

Notice what is missing from the dynamic line: there is no automatic "destroy" step at the end of a scope. The object sits there until *you* say otherwise. That is the freedom you came for, and the obligation you signed up for.

> **Key insight:** Static and automatic lifetimes are decided by where you write the declaration. A dynamic lifetime is decided by when you call `new` and when you call `delete` — the scope of the code has nothing to do with it.

### The heap

Where do these objects live? Automatic variables are stored in a function's **stack frame** — a tidy region that grows and shrinks as functions are called and return. Dynamically allocated objects live somewhere else entirely: a separate region usually called the **heap** (or the **free store**).

```
stack frame for parse():
    local int   count
    local Token* current      <-- a pointer, living on the stack

heap:
    Token object created with new   <-- the actual object, living on the heap
```

The stack is fast and strictly scope-bound: allocation is essentially free, but everything in a frame disappears when the function returns. The heap is the opposite trade — far more flexible about size and lifetime, but you reach its objects through an address, and cleanup is your job unless a library type is doing it for you.

### Allocating one object

The scalar form of `new` creates a single object on the heap and returns its **address**:

```cpp
int* score { new int };   // create an int on the heap; store its address in score
*score = 42;

std::cout << *score << '\n';   // 42

delete score;   // hand the storage back
score = nullptr;
```

Two distinct things are happening on that first line, and conflating them is the root of most beginner confusion:

1. `new int` creates an `int` object in dynamic storage.
2. `score` is an ordinary pointer variable — living on the stack — that *holds the address* of that object.

The pointer is not the dynamic object. It is the **handle** you use to reach the object — the only way you have to find it again.

```
score (on the stack):
    score --------+
                  |
heap object:      v
                int { 42 }
```

This separation has a sharp consequence: if you lose the pointer — overwrite it, let it go out of scope — you have lost the *only* path back to the object. The object is still sitting on the heap, occupying memory, but no line of code can ever reach it to free it. Hold onto that thought; it has a name we will meet shortly.

### Initialize at the point of allocation

`new int` gives you an `int` whose value is indeterminate — exactly like an uninitialized local. Prefer to create the object in a known state in the same breath:

```cpp
int*    exact   { new int { 7 } };
double* average { new double { 0.0 } };

delete exact;
delete average;
```

This is the same habit you have been practising since Chapter 1: don't make an object and *then* assign it: make it with the value it should have. The only new wrinkle is that the initializer now lives inside the `new` expression.

> **Best practice:** Initialize dynamically allocated objects at the point of allocation: `new int { 7 }`, not `new int` followed by an assignment.

### Deleting one object

`delete` is the counterpart to `new`. It ends the lifetime of the object the pointer refers to and returns that storage to the heap so it can be reused:

```cpp
int* value { new int { 5 } };

delete value;     // the dynamic int is destroyed; its storage is reclaimed
value = nullptr;  // value no longer points at storage we have given up
```

Read the second line carefully, because it reveals something subtle. `delete` does **not** change the pointer. After `delete value;`, the variable `value` still exists and still holds the same address it always did — but the object at that address is gone. The pointer now points at storage you no longer own. Setting it to `nullptr` is how you mark the handle as "no longer valid," so that a later mistake (dereferencing it, deleting it again) fails loudly instead of silently corrupting memory.

One small mercy: **deleting a null pointer is guaranteed to do nothing.**

```cpp
int* value {};   // value-initialized to nullptr
delete value;    // ok: defined to be a no-op
```

So you never need to write `if (value) delete value;`. A bare `delete` already handles the null case safely.

> **Note:** `delete` reclaims the *object's storage*; it does not touch the *pointer*. The pointer keeps its old address until you overwrite it — which is why nulling it out afterward is a useful habit.

### Dangling pointers

A **dangling pointer** is a pointer that holds the address of an object whose lifetime has already ended. Reading or writing through it is **undefined behavior** — the storage may have been handed to something else entirely, and the program may crash, corrupt data, or appear to work and fail later.

The trap is that nulling out *one* pointer does not rescue the others that copied the same address:

```cpp
int* first { new int { 10 } };
int* alias { first };   // alias now holds the same address

delete first;
first = nullptr;        // first is safe...

// ...but alias still holds the old address.
// *alias is undefined behavior — the object it named is gone.
```

`first` and `alias` were two handles to one object. Deleting through `first` ended the object; nulling `first` cleaned up that one handle — but `alias` was never told, and it is now dangling. This is the central difficulty of raw pointers: **the language does not track how many pointers refer to an object**, so it cannot warn you that one of them just became invalid.

That difficulty is really a question of *ownership*. Among all the pointers to an object, which one is responsible for deleting it?

| Pointer role | Meaning |
|---|---|
| **Owning** pointer | responsible for `delete`-ing the object exactly once |
| **Observing** pointer | may read or write the object, but must *not* delete it |

A raw `int*` does not encode which kind it is — they look identical. A future reader (often you, months later) has to infer the role from naming, comments, or convention. Getting it wrong gives you either a leak (nobody deleted it) or a double-delete (two owners both deleted it). Keeping this distinction straight by hand is the discipline this chapter is really teaching.

> **Warning:** After you `delete` a pointer, every *other* pointer holding that same address is now dangling. Nulling the one you deleted does not fix the copies. This is why "who owns this object?" must have exactly one answer.

### When allocation fails

Memory is finite. If the heap cannot satisfy a request, ordinary `new` throws a `std::bad_alloc` exception. If you would rather get a null pointer back than an exception, there is a `std::nothrow` form:

```cpp
#include <new>

int* buffer { new (std::nothrow) int { 0 } };

if (!buffer)
{
    std::cerr << "allocation failed\n";
}

delete buffer;   // safe even if buffer is nullptr
```

Most introductory programs do not handle heap exhaustion gracefully, and that is fine while you are learning. Production code that runs in tight-memory environments needs an actual policy — throw, return an error, use a bounded allocator, or design the path so it never allocates — but that is a topic for later.

### Memory leaks

A **memory leak** is the other failure mode, and it is the more common one. It happens when allocated storage is never released *and* the program has lost the ability to release it. The memory is occupied but unreachable — gone until the process exits.

The simplest leak is forgetting to delete:

```cpp
void leakOne()
{
    int* count { new int { 1 } };
}   // count (the pointer) is destroyed here — but the dynamic int was never deleted
```

When `leakOne` returns, the pointer `count` — an automatic variable — is destroyed along with the rest of the stack frame. But the `int` it pointed to lives on the heap, untouched by that cleanup. There was exactly one handle to it, and it just vanished. The `int` is now stranded.

A subtler leak comes from overwriting the only owning pointer:

```cpp
int* p { new int { 1 } };
p = new int { 2 };   // p now points at the second int; the first is stranded

delete p;            // releases only the second allocation; the first leaks
```

The fix in raw-pointer code is to keep each owner alive until you have deleted it:

```cpp
int* p { new int { 1 } };
delete p;            // release the first before reusing p
p = new int { 2 };
delete p;            // release the second
```

This is tedious and easy to get wrong, which is the whole point of the chapter. Modern C++ would rather you not write it at all:

```cpp
#include <memory>

auto count { std::make_unique<int>(1) };
// count deletes its int automatically when it goes out of scope
```

`std::unique_ptr` (which you will study properly in Chapter 22) is an *owning* pointer that runs `delete` for you when it is destroyed. It does not make the lifetime problem disappear — something still has to own the object — but it makes ownership **explicit in the type** and **scope-bound** again, which is most of the battle.

> **Best practice:** In new code, reach for `std::vector`, `std::string`, and smart pointers before raw `new`/`delete`. Learn the raw machinery so you can read it and understand *why* those types exist — then let them do the bookkeeping.

---

## 19.2 — Dynamically allocating arrays

### When one object is not enough

The scalar `new` gives you exactly one object. But the motivating problem for dynamic allocation was *not knowing the count until run time* — and one object rarely answers that. What you usually want is an array whose length is computed while the program runs. That is what the **array form** of `new` is for.

```cpp
int length {};
std::cin >> length;            // the size is a runtime value

int* values { new int[length]{} };   // allocate that many ints, all zeroed

values[0] = 10;

delete[] values;               // note the brackets
values = nullptr;
```

This is something a plain fixed-size array cannot do: `int values[length];` is not legal standard C++ when `length` is a runtime variable, because the size of an automatic array must be known at compile time. `new int[length]` has no such restriction — the count is just a number, evaluated when the line runs.

### The brackets are part of the contract

Look closely at the cleanup line: it is `delete[]`, not `delete`. The brackets are not decoration. The runtime tracks array allocations differently from single-object allocations, and you must release each with its matching form:

| You allocated with | You must release with |
|---|---|
| `new int` | `delete ptr` |
| `new int[n]` | `delete[] ptr` |

Mixing them — scalar `delete` on an array, or `delete[]` on a single object — is **undefined behavior**. Nothing in the type system stops you (both are just `int*`), so this is a rule you carry in your head.

> **Warning:** `new` pairs with `delete`; `new[]` pairs with `delete[]`. The pointer type is the same `int*` either way, so the compiler cannot catch a mismatch — getting it right is on you.

### A dynamic array behaves like an array

Once allocated, you index a dynamic array exactly as you would any other:

```cpp
double* samples { new double[3] { 1.0, 2.0, 4.0 } };

for (int i { 0 }; i < 3; ++i)
    std::cout << samples[i] << '\n';

delete[] samples;
```

But there is one thing it cannot do, and it is a big one: **the pointer does not know its own length.**

```cpp
double* samples { new double[3]{} };

// std::size(samples) does not compile — samples is a pointer, not an array.
// sizeof(samples) gives the size of the pointer (e.g. 8), not 3 * sizeof(double).

delete[] samples;
```

A fixed-size `std::array` or a built-in array carries its length in its type, so the library can recover it. A raw pointer has lost that information entirely — `samples` is just an address. Any code that receives only a `double*` therefore needs the length delivered *separately*: as another parameter, or by a sentinel convention (like the null terminator on a C string).

This is the classic C-style signature, and you will see it constantly in older and lower-level code:

```cpp
void processCounters(unsigned* counters, int counterCount);
```

One parameter points at the first element; the other says how many elements are valid. The function has to honor both — read past `counterCount` and you are reading memory you do not own.

> **Key insight:** A pointer to a dynamic array carries the address but *not* the length. Length must travel alongside the pointer — as a separate parameter or a sentinel. This is exactly the contract the chapter lab asks you to implement.

### Initializing a dynamic array

You can value-initialize a dynamic array to a clean state with `{}`:

```cpp
int* counts { new int[5]{} };   // all five elements become 0

delete[] counts;
```

That empty-brace `{}` is doing real work — without it, `new int[5]` leaves the elements with indeterminate values. With it, every element is zero-initialized. You can also supply explicit starting values with an initializer list:

```cpp
int* priorities { new int[4] { 3, 1, 4, 1 } };

delete[] priorities;
```

> **Tip:** Write `new int[n]{}` (with braces) to get a zero-initialized array. The lab relies on this: `new int[n]{}` gives you `n` zeros, and you overwrite them with the fill value in a loop.

### "Resizing" means allocating a new array

A raw dynamic array has a fixed length once it is allocated — there is no `new` operation that grows an existing block in place. To "resize," you perform a four-step dance: allocate fresh storage of the new size, copy across the elements you want to keep, delete the old storage, and redirect your pointer at the new block.

```cpp
int  oldLength { 3 };
int* data { new int[oldLength] { 2, 4, 6 } };

int  newLength { 5 };
int* larger { new int[newLength]{} };          // 1. new, larger array (zeroed)

for (int i { 0 }; i < oldLength; ++i)          // 2. copy the elements to keep
    larger[i] = data[i];

delete[] data;                                 // 3. release the old array
data = larger;                                 // 4. redirect the owning pointer
larger = nullptr;

delete[] data;                                 // cleanup at end of life
```

The **order is not negotiable**. Step 2 reads from the old array, so it must happen *before* step 3 frees it. Reverse those two and you are copying out of memory you have already handed back — undefined behavior, and exactly the kind of bug that "works on my machine" right up until it doesn't. The chapter lab's `resizeBuffer` is precisely this algorithm; getting the order right is the heart of that task.

This bookkeeping is fiddly and entirely mechanical — which is why the standard library does it for you:

```cpp
#include <vector>

std::vector<int> data { 2, 4, 6 };
data.resize(5);   // grows the storage, copies the old elements, frees the old block
```

`std::vector::resize` *is* the four-step dance, written once, tested thoroughly, and impossible to get the order wrong on.

### Why raw dynamic arrays are fragile

Step back and count the ways a raw dynamic array can hurt you:

- The pointer does not carry a visible length — callers must be told separately.
- Cleanup must use `delete[]`, and mismatching it is undefined behavior.
- Resizing is a manual, order-sensitive copy.
- Copying the pointer copies the *address*, not the elements — two handles, one block, a double-delete waiting to happen.
- Ownership is invisible in the type — `int*` does not say who frees it.

Every one of these is a place to make a mistake the compiler cannot catch. `std::vector<T>` answers all of them at once: it owns its allocation, stores the size and capacity right next to the data, copies deeply when you copy it, and frees itself when it dies.

> **Best practice:** Prefer `std::vector<T>` for any runtime-sized sequence. Use raw `new[]`/`delete[]` only when a lab, an API, or a measured performance constraint specifically requires it — and the lab in this chapter requires it precisely so you feel what `vector` is saving you from.

---

## 19.3 — Destructors

### Cleanup that runs by itself

You have now felt the central pain of raw dynamic memory: every `new` needs a matching `delete`, and a forgotten one leaks. The destructor is the language feature that lets you guarantee the matching `delete` happens — not by remembering to write it at every call site, but by attaching it to the object's death.

A **destructor** is a special member function that runs automatically when an object is destroyed. If a constructor is the class's setup hook, the destructor is its **cleanup hook**.

```cpp
class Trace
{
public:
    Trace()  { std::cout << "construct\n"; }
    ~Trace() { std::cout << "destroy\n"; }
};

int main()
{
    Trace t{};
}   // t's destructor runs here, automatically — prints "destroy"
```

The name of a destructor is the class name preceded by a tilde, `~`:

```cpp
class Buffer
{
public:
    ~Buffer();   // the destructor for Buffer
};
```

A destructor takes **no parameters** and has **no return type** (not even `void`), and a class may have **exactly one**. There is nothing to overload — an object is destroyed only one way.

### Destructors and ownership: RAII

Here is where it all comes together. If a class acquires a resource when it is constructed and releases that resource in its destructor, then the resource's lifetime becomes *identical to the object's lifetime* — and the object's lifetime is managed for you by the rules of scope. This pattern has an unwieldy name and a profound payoff: **RAII**, for *Resource Acquisition Is Initialization*.

```cpp
class IntBuffer
{
private:
    int* m_data {};
    int  m_length {};

public:
    explicit IntBuffer(int length)
        : m_data { new int[length]{} }   // acquire the resource on construction
        , m_length { length }
    {
    }

    ~IntBuffer()
    {
        delete[] m_data;                 // release it on destruction
    }

    int length() const { return m_length; }
    int& operator[](int index) { return m_data[index]; }
};
```

Now look at how a *caller* uses it:

```cpp
void run()
{
    IntBuffer counters { 10 };
    counters[0] = 1;
}   // delete[] runs automatically, via IntBuffer::~IntBuffer()
```

There is no `delete[]` in `run`. There cannot be a forgotten one, because there is no manual one to forget. The `new[]` lives inside the constructor, the `delete[]` lives inside the destructor, and the two are bound together by the object — allocate-on-birth, free-on-death. The caller just declares an `IntBuffer` like any ordinary local, and the cleanup rides along with the scope.

> **Key insight:** RAII converts "remember to free this resource" — a discipline you can fail at — into "the object owns the resource," a guarantee the language enforces. The destructor is the mechanism that makes the guarantee.

This is, almost exactly, what `std::vector` does internally. The lab's stretch goal asks you to wrap the five raw functions into a class with a destructor — and when you do, you will have built a miniature `vector` and understood it from the inside.

### Construction and destruction order

When several automatic objects live in the same scope, they are destroyed in the **reverse** of the order they were constructed:

```cpp
Trace first{};
Trace second{};
Trace third{};

// construction order:  first  -> second -> third
// destruction order:   third  -> second -> first
```

```
construct:  first  ->  second  ->  third
destroy:    third  ->  second  ->  first
```

This reverse ordering is not arbitrary — it is exactly what you want when one object depends on another. If `second` was built using something `first` set up, you want `second` torn down *before* `first` disappears out from under it. Because you naturally construct a dependency before the thing that depends on it, reverse-order destruction tears them down in the safe sequence for free.

### `delete` runs the destructor

For a *dynamically* allocated class object, `delete` does two things in order: it runs the destructor, then it releases the storage.

```cpp
Trace* trace { new Trace{} };
delete trace;   // calls ~Trace() first, then frees the memory
```

And for a dynamic *array* of class objects, `delete[]` runs the destructor for **every** element:

```cpp
Trace* traces { new Trace[3]{} };
delete[] traces;   // ~Trace() runs three times, then the block is freed
```

This is another reason the `delete` vs `delete[]` distinction matters so much: calling scalar `delete` on an array would destroy only the first element's worth and leave the rest's destructors un-run — resources leaked, exactly the thing destructors exist to prevent.

### A warning about `std::exit()`

Returning from `main` (or from any function) unwinds the automatic objects in scope, running their destructors along the way — that is the normal, clean exit path. Calling `std::exit()` ends the program more abruptly and does **not** run the destructors of ordinary local automatic objects the way a normal scope exit does.

```cpp
void stopNow()
{
    Trace t{};
    std::exit(1);   // t's destructor is NOT run via the normal scope-exit path
}
```

In code that owns files, locks, temporary directories, or heap memory, jumping out with `std::exit` can skip cleanup that really mattered. Prefer to leave through normal control flow — return an error code up the call stack, or throw an exception (Chapter 27) — so destructors get their chance to run.

> **Warning:** `std::exit()` bypasses the destructors of local automatic objects. If those destructors release resources, that cleanup is skipped. Prefer returning or throwing over abruptly exiting.

### RAII is the lesson to take away

Raw `new`/`delete` are easy to misuse — that has been the running theme. RAII is the idiomatic C++ answer: move the cleanup into a destructor so the resource follows the object's lifetime, and let scope do the rest.

| Resource | Its RAII owner |
|---|---|
| dynamic array | `std::vector<T>` (or a custom owner class) |
| single dynamic object | `std::unique_ptr<T>` |
| file handle | `std::fstream` |
| lock | `std::lock_guard` |

In modern C++, "I need to write a destructor" almost always means "I am building a type that owns a resource." If a class only stores ordinary values — `int`s, `std::string`s, `std::vector`s that already clean up after themselves — you do not need to write a destructor at all; the compiler-generated one is correct. You write a destructor precisely when you are the one holding a raw resource that something must release.

---

## 19.4 — Pointers to pointers and dynamic multidimensional arrays

### A pointer can point at a pointer

A pointer holds the address of an object. A pointer *can itself be an object* with an address — so a pointer can hold the address of another pointer. That is a **pointer to pointer**, written with two stars:

```cpp
int   value     { 7 };
int*  ptr       { &value };   // ptr holds the address of value
int** ptrToPtr  { &ptr };     // ptrToPtr holds the address of ptr

std::cout << **ptrToPtr << '\n';   // 7 — dereference twice to reach value
```

```
ptrToPtr  -->  ptr  -->  value
               |          |
               |          +-- the int object (7)
               +------------- the pointer object (an address)
```

Read the type from the inside out: `int**` is "a pointer to (a pointer to (an `int`))." Each `*` you apply in `**ptrToPtr` peels off one level — the first dereference gets you `ptr`, the second gets you the `int`.

### Where pointers to pointers show up

You will not reach for `int**` often in modern C++, but it appears in a few recognizable situations:

- modifying a *pointer variable* through another pointer (changing where someone else's pointer points),
- representing an **array of pointers**,
- interfacing with **C libraries**, whose APIs use this shape heavily,
- building old-style dynamic **multidimensional arrays**.

For ordinary modern code, references, containers, and smart pointers cover these needs more safely. But the pattern is worth recognizing when you meet it.

### A dynamic 2D array, the old way

One classic technique builds a two-dimensional array as an array of **row pointers**: an outer `int**` block holding pointers, each of which points at its own separately allocated row.

```cpp
int rows { 3 };
int cols { 4 };

int** grid { new int*[rows]{} };          // outer array: 3 int-pointers

for (int r { 0 }; r < rows; ++r)
    grid[r] = new int[cols]{};            // each row: its own array of 4 ints

grid[1][2] = 9;

for (int r { 0 }; r < rows; ++r)
    delete[] grid[r];                     // free each row first

delete[] grid;                            // then free the outer array
```

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

This shape is flexible — rows could even be different lengths — but it pays for that flexibility with *many* separate allocations and a correspondingly large number of cleanup paths.

### Cleanup must mirror allocation exactly

Every successful row `new[]` needs a matching `delete[]`, *and* the outer array needs its own. Miss the outer one and you leak the row pointers; miss a row and you leak that row's data; free the outer block before the rows and you can no longer reach the rows to free them. The order — rows first, then the outer array — matters as much as it did in the resize algorithm.

The fragile part is what happens when something goes wrong *between* the allocations:

```cpp
int** grid { new int*[rows]{} };

for (int r { 0 }; r < rows; ++r)
    grid[r] = new int[cols]{};

// If an early return or an exception happens here, every row leaks.
```

This is a lot of correctness to keep in your head, and RAII makes most of it disappear:

```cpp
#include <vector>

std::vector<std::vector<int>> grid(rows, std::vector<int>(cols));
grid[1][2] = 9;
// every inner and outer allocation frees itself — no delete[] anywhere
```

For numeric and dataflow work, there is an even better option: store the grid as **one flat array** and compute the index yourself.

```cpp
#include <vector>

std::vector<int> grid(rows * cols);

auto index = [cols](int row, int col)
{
    return row * cols + col;
};

grid[index(1, 2)] = 9;
```

One allocation instead of `rows + 1` of them, and the elements sit contiguously in memory — which is usually noticeably faster, because the CPU's cache loves contiguous data. The row-pointer technique is worth understanding because you will read it in older code, but it is rarely the right thing to write today.

> **Best practice:** For a dynamic 2D array, prefer a `std::vector` of `std::vector`s, or — better for numeric work — a single flat `std::vector` with a computed index. Both free themselves; the flat version is also more cache-friendly than separately allocated rows.

### Reference-to-pointer is often clearer than pointer-to-pointer

Recall from Chapter 12 that a function which needs to change a caller's *pointer* can take a **reference to a pointer**. When the goal is "write a new pointer value back into the caller's variable," that is usually clearer than the `int**` form:

```cpp
void allocateOne(int*& out)   // reference to an int-pointer
{
    out = new int { 5 };      // writes back into the caller's pointer
}

int* value {};
allocateOne(value);           // value now points at a fresh int
delete value;
```

A C API doing the same job would typically take `int**` and require the caller to pass `&value`. The reference version expresses the same intent without the extra layer of explicit address-taking — and it is exactly the mechanism the chapter lab uses: `destroyBuffer` and `resizeBuffer` take `int*&` so they can write `nullptr` or a new address back into your pointer. Take those parameters by value and the write-back silently fails — the caller's pointer never changes.

> **Tip:** When a function must update a caller's pointer, prefer a reference-to-pointer parameter (`int*&`) over a pointer-to-pointer (`int**`). The lab leans on this directly.

---

## 19.5 — Void pointers

### A pointer that has forgotten its type

Every pointer you have used so far has known what it points at: an `int*` points at an `int`, a `Trace*` at a `Trace`. A `void*` is the exception — it can hold the address of an object *without recording what type of object lives there*.

```cpp
int   count   { 3 };
void* erased  { &count };   // erased holds the address, but "forgets" it's an int
```

The type has been **erased** from the pointer. The compiler still knows `erased` is an address, but it no longer knows whether the thing at that address is an `int`, a `double`, a `Trace`, or anything else. You have traded away the one thing the type system was protecting.

### You must cast before you can dereference

Because a `void*` has no pointed-to type, the compiler does not know how many bytes the object occupies or how to interpret them — so it will not let you dereference a `void*` directly. You have to convert it back to a real pointer type first:

```cpp
int   count    { 3 };
void* erased   { &count };

int* restored { static_cast<int*>(erased) };   // promise: this is really an int
std::cout << *restored << '\n';                 // 3
```

That `static_cast` is a promise *you* are making to the compiler, and the compiler takes you at your word. If the promise is wrong — if the object is not actually the type you cast to — reading through the result is **undefined behavior**:

```cpp
double* wrong { static_cast<double*>(erased) };
// *wrong is undefined behavior — the object is really an int, not a double
```

Nothing checks this for you. The whole safety net of the type system has been removed, and it is now entirely on you to remember what the real type was.

### What `void*` cannot do

The limitations follow directly from "no pointed-to type":

- You **cannot dereference** a `void*` without casting it to a typed pointer first.
- You **cannot do meaningful pointer arithmetic** on a standard `void*` — there is no element size to step by.
- A `void*` carries **no length, no ownership, and no const-ness** beyond the pointer itself.
- The compiler **cannot type-check** any use of the object once the type is erased.

If the object should not be modified, at least preserve const-ness by using `const void*`:

```cpp
const int   count   { 3 };
const void* erased  { &count };   // keeps the "do not modify" promise
```

Casting that const away to write through it is a separate, dangerous operation — don't, unless you genuinely own the object and know it is non-const underneath.

### Why `void*` exists at all

`void*` is the tool of last resort for low-level, C-style interfaces that need to pass "some data" through a generic slot — most commonly a callback that receives a context pointer:

```cpp
using Callback = void (*)(void*);

void printInt(void* data)
{
    int* value { static_cast<int*>(data) };
    std::cout << *value << '\n';
}
```

This works, but notice where the type information has gone: into a *convention* that lives outside the type system. The caller and the callee have to agree, by documentation and discipline, that the `void*` really points at an `int`. The compiler cannot help; if either side gets the agreement wrong, you are back in undefined-behavior territory.

Modern C++ has better tools for almost every job `void*` used to do:

| What you need | Reach for |
|---|---|
| one generic function over many types | a **template** |
| one of several known types | `std::variant` |
| an owning dynamic object | a **smart pointer** |
| a byte-level view of memory | `std::byte*` or `std::span<std::byte>` |
| polymorphic behavior | a base-class pointer or reference |

### Reading `void*` in real code

When you meet a `void*` in someone else's code, treat it as a sign that you have stepped outside the friendly part of the language, and ask:

1. What concrete type is *actually* stored at this address?
2. Who **owns** the pointed-to object?
3. Who is allowed to cast it back, and to what?
4. How is the object's **lifetime** guaranteed for as long as the `void*` is used?
5. What happens if the wrong callback runs, or the wrong cast is applied?

If you cannot answer those, you cannot use the pointer safely — and that is the point. A `void*` is low-level glue, not a general-purpose data model. Recognize it, handle it carefully where you must, and prefer a typed alternative everywhere you can.

> **Warning:** A `void*` removes the type system's protection. The cast back to a typed pointer must match the real object exactly, or you have undefined behavior — and nothing in the compiler will warn you. Prefer templates, `std::variant`, smart pointers, or `std::span` whenever you can.

---

## Chapter 19 study summary

### Core takeaways

- **Dynamic allocation** creates objects whose lifetime *you* control explicitly, from `new` to `delete` — independent of any scope.
- `new` must be paired with `delete`; `new[]` must be paired with `delete[]`. The pointer type cannot tell them apart, so the discipline is yours.
- A pointer can **outlive** the object it points at. Using such a **dangling pointer** is undefined behavior, and nulling one copy does not save the others.
- Losing the only owning pointer before cleanup is a **memory leak** — storage occupied but unreachable.
- A **destructor** makes cleanup automatic at the moment an object is destroyed.
- **RAII** — acquire in the constructor, release in the destructor — is the idiomatic C++ answer to raw resource management, and the reason `std::vector` and smart pointers are safe.
- `int**` means "pointer to pointer to `int`"; it is often a sign of low-level or C-style code, and `int*&` is usually clearer when you just need to write a pointer back.
- `void*` erases type information; replace it with a typed alternative wherever practical.

### A modern preference ladder

When you need to represent ownership or a runtime-sized collection, climb to the highest-level tool that fits before dropping to raw pointers:

| Use case | Prefer |
|---|---|
| runtime-sized sequence | `std::vector<T>` |
| text | `std::string` |
| sole ownership of one object | `std::unique_ptr<T>` |
| genuinely shared ownership | `std::shared_ptr<T>` (only when ownership truly is shared) |
| non-owning access | a reference, a raw pointer, or `std::span<T>` |
| raw `new`/`delete` | only for learning, low-level libraries, or APIs that require it |

### Before the lab

The DynBuffer Workbench is this chapter made physical. You will build five free functions that manage a raw `int` array on the heap with nothing but `new[]`, `delete[]`, and a pointer — no `std::vector`, no smart pointers. You will write the fill loop, the deep copy that allocates *separate* storage (not a second pointer to the same block), and the allocate-copy-delete-redirect resize whose step order you now know cold. Two of the functions take `int*&` so they can null out or redirect your pointer through the reference — the Chapter 12 mechanism that finally pays off visibly here. When `make test` goes green, you will have done by hand everything `std::vector` does for you — which is the surest way to understand why it exists.
