# Chapter 25 — Virtual Functions: MINI-LLVM

> Reinforces [`../../notes/chapter-25.md`](../../notes/chapter-25.md) · LearnCpp [Chapter 25](https://www.learncpp.com/)

## The project

You're building **MINI-LLVM**: a tiny instruction hierarchy that mirrors the
real `llvm::Instruction` class family you'll work with in CS6340 labs.  By the
end of the exercise you'll have an abstract base class `Inst` with three
concrete derived classes (`AddInst`, `LoadInst`, `BranchInst`), a polymorphic
free function `describe()`, a safe downcast helper `asBranch()`, and a
`countOpcode()` function that walks a vector of owned instructions — all
without importing a single LLVM header.

This exercise resolves the **Chapter 24 cliffhanger**: in Ch 24 you saw that
calling a base-class function through a `Base&` always invokes the base
version, even when the object is actually a derived type (static binding).  The
`describe()` function here is the exact same pattern — one function that accepts
`const Inst&` — but now `opcodeName()` is `virtual`, so the call dispatches to
whichever concrete class is actually there.  That is **runtime polymorphism**,
and it is the mechanism LLVM relies on for every pass that iterates over
instructions.

The LLVM vocabulary map for this exercise (see also `../../llvm-idioms.md`):

| This exercise | Real LLVM API |
|---|---|
| `Inst` | `llvm::Instruction` |
| `Inst::opcodeName()` | `Instruction::getOpcodeName()` |
| `AddInst` | `llvm::BinaryOperator` (Add opcode) |
| `LoadInst` | `llvm::LoadInst` |
| `BranchInst` | `llvm::BranchInst` |
| `asBranch(p)` | `dyn_cast<BranchInst>(&I)` |
| `asBranch(p) != nullptr` | `isa<BranchInst>(&I)` (the boolean-only cousin) |
| `countOpcode(module, "add")` | a Module → BasicBlock → Instruction walk |

`dynamic_cast` here is standard C++; LLVM's `isa<>` / `cast<>` / `dyn_cast<>`
are its own faster, RTTI-free helpers (see `../../llvm-idioms.md`), but the
reasoning is identical: *ask whether the runtime object is a given derived type,
then either branch on a bool (`isa`) or use the downcast pointer (`dyn_cast`).*

## Concepts practiced

- **`virtual` functions** — enable dynamic dispatch through a base pointer or reference (notes 25.2)
- **`override`** — compile-time verification that a derived function matches a base virtual (notes 25.3)
- **`final`** — locks `BranchInst::opcodeName()` against further overriding (notes 25.3)
- **Pure virtual / abstract base classes** — `= 0` on `opcodeName()` makes `Inst` non-instantiable (notes 25.7)
- **Virtual destructors** — required whenever a class is deleted through a base pointer; the destroyed-counter *proves* it fired (notes 25.4)
- **Object slicing** — the README demonstrates value-copy slicing (with a *concrete* base, since abstract `Inst` cannot be stored by value at all) and why `std::vector<std::unique_ptr<Inst>>` is the fix for a polymorphic collection (notes 25.9)
- **`dynamic_cast` (pointer form)** — runtime-checked downcast returning `nullptr` on mismatch, the safe pattern used by LLVM's own `dyn_cast<>` (notes 25.10)
- Reused from earlier chapters: `std::vector` (Ch 16), `std::unique_ptr` (Ch 22 — provided scaffolding), `std::string_view` (Ch 5), header/source split (Ch 2)

## Your tasks

The starter compiles immediately but every function body returns a wrong
placeholder, so `make test` starts **RED**.  Fill in the six
`>>> YOUR CODE HERE <<<` blocks in [`starter/inst.cpp`](starter/inst.cpp).
They ramp from easy to a small stretch:

1. **Virtual base destructor (`~Inst()`).**  Define the body of `Inst::~Inst()`.
   It may be empty — the important thing is that it exists and is declared
   `virtual` in the header.  Without a virtual destructor, deleting a derived
   object through an `Inst*` skips the derived destructor entirely.

2. **`opcodeName()` overrides.**  Implement all three `opcodeName()` bodies:
   `AddInst` returns `"add"`, `LoadInst` returns `"load"`, `BranchInst` returns
   `"br"`.  These three strings make virtual dispatch observable: the same call
   `inst->opcodeName()` through an `Inst*` produces a different string depending
   on which concrete class lives at the other end of the pointer.

3. **Derived destructors — the vtable proof.**  Each body does one thing:
   `++Inst::s_destroyed`.  When the grader deletes an `AddInst` through a
   `std::unique_ptr<Inst>`, it checks that `s_destroyed` ticked up — direct
   proof that the virtual destructor chain ran the *derived* destructor before
   the base one.

4. **`describe(const Inst& inst)`.**  Return `"inst: " + inst.opcodeName()`.
   This is the **Chapter 24 cliffhanger resolved**: the static type of `inst`
   is `const Inst&`, but `opcodeName()` is `virtual`, so the call dispatches
   at runtime to whichever concrete class `inst` actually is.  You write this
   function once; it works for any present or future `Inst` subclass.

5. **`asBranch(Inst* inst)`.**  Use `dynamic_cast<BranchInst*>(inst)` and
   return the result directly.  Returns a valid `BranchInst*` when the pointed-at
   object is a `BranchInst`; returns `nullptr` otherwise (including for
   `nullptr` input).  This is LLVM's `dyn_cast<BranchInst>(&I)` in miniature.
   Always use the **pointer form** — the reference form throws `std::bad_cast`
   on failure instead.

6. **`countOpcode(...)`.**  Iterate over the `std::vector<std::unique_ptr<Inst>>`
   with a range-`for`.  For each element, call `opcodeName()` on the dereferenced
   pointer and count how many match the requested `opcode` string.  Return the
   count.

You may **not** edit `inst.h` or anything in `tests/` — those are the contract
and the grader.

---

### Object slicing — the anti-pattern this exercise avoids

`Inst` is abstract, so you *cannot* write `std::vector<Inst>` and push derived
objects into it; the compiler will reject it immediately (you cannot copy-construct
an abstract class).  But consider what would happen with a **concrete** pair:

```cpp
// Slicing in action — using a concrete base to show the effect.
// (Abstract Inst cannot be stored by value at all, which is even stricter.)
class Concrete { public: int base{1}; virtual std::string name() const { return "base"; } };
class Derived   : public Concrete { public: int extra{2}; std::string name() const override { return "derived"; } };

Concrete sliced = Derived{};    // slicing: only the Concrete part is copied
std::cout << sliced.name();     // prints "base", not "derived" — Derived::name is gone
// sliced.extra does not exist — the derived data was silently dropped
```

This is why `countOpcode` uses `std::vector<std::unique_ptr<Inst>>`: the full
derived object lives on the heap; the smart pointer holds just a typed reference
to it; virtual dispatch still works through that reference.  (notes 25.9)

---

## Constraints

- **Allowed (Ch 25):** `virtual`, `override`, `final`, `= 0` (pure virtual),
  virtual destructors, `dynamic_cast` (pointer form only).
- **Allowed (earlier chapters):** `std::string`, `std::string_view`, `std::vector`,
  `std::unique_ptr`, `std::make_unique`, range-`for`, `const` references.
- **Forbidden:** `typeid`/`type_info` RTTI, `reinterpret_cast`, raw `new`/`delete`
  in learner code (use the provided `unique_ptr` scaffolding), `static_cast`
  down the hierarchy (not safe — use `dynamic_cast`), virtual inheritance (Ch 25.8 —
  out of scope for this exercise).
- **Required idioms:** `override` on every derived virtual override; virtual
  destructor on `Inst`; pointer form of `dynamic_cast` in `asBranch()`;
  `std::vector<std::unique_ptr<Inst>>` for polymorphic ownership.

## Build & run

```sh
make              # compile starter/inst.cpp warning-clean (object file only)
make test         # grade your code  ->  RED until the TASK blocks are filled in
make solution     # run the grader against the reference solution
make test-solution  # proof the exercise is solvable: reference MUST be green
make clean        # remove build artifacts
```

The grader compiles `tests/tests.cpp` together with your `starter/inst.cpp` and
runs all checks.  Turning 32 red lines green is the exercise.

## Success criteria

`make test` prints **PASS ✅  all MINI-LLVM checks passed.** and exits 0.  Until
then it prints one `FAIL: … @line N` per broken check.  The grader covers:

- `opcodeName()` called **directly** on concrete objects (baseline sanity)
- `opcodeName()` called through an **`Inst*`** (pure virtual dispatch)
- **Virtual destructor proof**: `s_destroyed` increments when a derived object
  is deleted through a `unique_ptr<Inst>` base pointer
- `describe()` through **concrete objects** and through **base references**
- `asBranch()` succeeding on a `BranchInst`, failing (`nullptr`) on others, and
  handling `nullptr` input
- `countOpcode()` on empty, single, mixed, and all-same vectors
- An **integrated round-trip**: build a 4-instruction basic block, call
  `describe()`, `countOpcode()`, and `asBranch()` together

## Hints

<details><summary>Task 1 — why the virtual destructor body can be empty</summary>

`Inst` carries no raw resources (no `new`-allocated memory, no file handles).
The body of `~Inst()` genuinely does not need to do anything.  What matters is:

1. The declaration in `inst.h` says `virtual ~Inst();` — making it virtual.
2. There is a definition somewhere (your `inst.cpp`) — even `Inst::~Inst() = default;`
   (which lets the compiler generate an empty body) is fine.

The *derived* destructors (`~AddInst`, etc.) do the interesting work in TASK 3.
</details>

<details><summary>Task 2 — what "override" actually checks</summary>

`override` tells the compiler:

```
This function is intended to override a virtual base function.
If it does not match one exactly (return type, name, parameters, const), make it a compile error.
```

Without `override`, a typo like `std::string opcodeNAME() const` silently
creates a brand-new function instead of overriding the base virtual.  Because
`Inst::opcodeName()` here is **pure** (`= 0`, no body to fall back on), the
derived class would then leave that pure virtual *un*-overridden — so it stays
**abstract** and the test's `AddInst add {}` fails to compile outright ("variable
type 'AddInst' is an abstract class").  (For a non-pure base virtual, the typo
would instead silently *hide* the base version and dispatch would keep hitting
the base body.)  With `override`, the compiler catches the mismatch at the
declaration, before any of that.  Always use it on every derived override.
(notes 25.3)
</details>

<details><summary>Task 3 — the virtual destructor chain in detail</summary>

When `std::unique_ptr<Inst>` goes out of scope holding an `AddInst*`, this is
what happens:

```
unique_ptr calls: delete (Inst*)(add_ptr)
    |
    v
Virtual dispatch: vtable for AddInst -> slot for ~AddInst
    |
    v
AddInst::~AddInst() runs -> ++Inst::s_destroyed
    |
    v (automatic base-chain)
Inst::~Inst() runs -> (empty body)
```

If `Inst::~Inst()` were **not virtual**, the vtable lookup never happens and the
compiler calls `Inst::~Inst()` directly — `AddInst::~AddInst()` is skipped
entirely, `s_destroyed` stays 0, and any AddInst-specific resources would be
leaked.  That is the bug the virtual destructor rule prevents.  (notes 25.4)
</details>

<details><summary>Task 4 — seeing the dispatch in action</summary>

```cpp
std::string describe(const Inst& inst)
{
    return "inst: " + inst.opcodeName();
    //                ^^^^^^^^^^^^^^^^^
    //  static type:  const Inst&
    //  dynamic type: AddInst, LoadInst, or BranchInst (at runtime)
    //
    //  Because opcodeName() is virtual, the call goes through the vtable
    //  of the ACTUAL object, not the static type. This is late binding.
}
```

Chapter 24 had this same pattern with a NON-virtual function: every call
through `const Base&` hit `Base::identify()` regardless of the actual object.
Adding `virtual` is the one-word fix.
</details>

<details><summary>Task 5 — dynamic_cast pointer vs reference form</summary>

**Pointer form** (use this):
```cpp
BranchInst* b = dynamic_cast<BranchInst*>(inst);
// b == nullptr if inst does not point at a BranchInst
// b is valid   if inst does
```

**Reference form** (avoid in asBranch):
```cpp
BranchInst& b = dynamic_cast<BranchInst&>(*inst);
// throws std::bad_cast if *inst is not a BranchInst
// need a try/catch to handle failure — more code, harder to use
```

LLVM's own `dyn_cast<T>(ptr)` uses the pointer form and returns nullptr on
failure — exactly the pattern you're implementing here.  (notes 25.10,
llvm-idioms.md)
</details>

<details><summary>Task 6 — the range-for loop shape</summary>

```cpp
int count { 0 };
for (const auto& ptr : module)   // ptr is const unique_ptr<Inst>&
{
    // ptr->opcodeName() dereferences unique_ptr and calls virtual opcodeName()
    if (ptr->opcodeName() == opcode)
        ++count;
}
return count;
```

`ptr->opcodeName()` is the same as `(*ptr).opcodeName()`.  The `->` operator on
`unique_ptr` forwards to the underlying raw pointer.  Virtual dispatch still
works — the vtable pointer lives in the concrete object on the heap, not in the
`unique_ptr` wrapper.
</details>

<details><summary>Stuck on a build error instead of a test failure?</summary>

- *"allocating an object of abstract class type 'Inst'"* — you cannot create an
  `Inst` object directly; only derived concrete classes.  If a test tries `Inst
  x {}`, that is the bug — but the provided tests never do this.
- *"override" compile error* — the signature in your derived definition does not
  exactly match the base virtual declaration.  Check `const`, parameter types,
  and return type carefully.  The `override` keyword is catching a real mismatch.
- *"undefined reference to Inst::s_destroyed"* — the static member needs a
  definition in exactly one `.cpp`.  It is already provided in the starter; do
  not remove or duplicate it.
- *dynamic_cast returns garbage instead of nullptr* — `dynamic_cast` requires
  the class to be polymorphic (have at least one virtual function).  `Inst`
  qualifies; just make sure you are using the pointer form, not a C-style cast.
</details>

## Stretch goals (optional — some use later chapters)

- Add a `RetInst` class that overrides `opcodeName()` to return `"ret"` and
  marks both the override and the class itself as `final` — experiment with what
  `final` allows and forbids.
- Add a `virtual void print(std::ostream&) const` to `Inst` and a non-member
  `operator<<` that calls it — the polymorphic printing pattern from notes 25.11.
- Make `describe()` return the result of a second virtual method, say
  `virtual std::string operands() const`, that each derived class implements
  differently — explore how deep the dispatch chain can go.
- Replace `countOpcode` with a `findFirst(module, opcode) -> Inst*` that returns
  a pointer to the first matching instruction, or `nullptr` if none — combine
  range-`for` with `dynamic_cast` to also downcast the result.
- Add exception handling (Ch 27 preview): wrap the reference form of
  `dynamic_cast` in a `try`/`catch(std::bad_cast&)` and compare the error path
  against the nullable pointer form.
