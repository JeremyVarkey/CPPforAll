# Chapter 23 - Object Relationships

> Source: <https://www.learncpp.com/> (Chapter 23)
> One file per chapter. Each lesson is a section below.

## Contents
- [23.1 - Object relationships](#231---object-relationships)
- [23.2 - Composition](#232---composition)
- [23.3 - Aggregation](#233---aggregation)
- [23.4 - Association](#234---association)
- [23.5 - Dependencies](#235---dependencies)
- [23.6 - Container classes](#236---container-classes)
- [23.7 - std::initializer_list](#237---stdinitializer_list)
- [23.x - Chapter 23 summary and quiz](#23x---chapter-23-summary-and-quiz)

---

## 23.1 - Object relationships

Up to now, most of your effort has gone into building individual classes — getting one type right, with the correct constructors, the right invariants, clean access control. That skill matters. But real programs are rarely one class. They are dozens of classes that *refer to each other*, and the interesting design decisions live in those references, not inside any single class.

This chapter gives you the vocabulary to reason about those references. It is, in a sense, less about syntax than about intent. Two members can look identical in C++ — both might be a pointer to some other object — and yet mean completely different things about who owns what and who outlives whom. Getting the relationship right is what separates code that quietly leaks or crashes from code that is correct by construction.

### Why relationship vocabulary matters

Once a program grows past toy examples, the questions that keep you up at night stop being "how do I write a loop?" and become questions about how objects relate:

- Is one object literally *part of* another object?
- Does one object control another object's *lifetime* — does destroying the first destroy the second?
- Can several objects refer to the *same* underlying object at once?
- Is this relationship *stored* as part of an object's state, or is it just temporary — alive for the duration of one function call?
- Does one object merely *use* another to get a job done, without remembering it afterward?

Each answer pushes you toward a different C++ representation. Consider three members that look almost the same:

```cpp
class FunctionSummary
{
    std::string m_name;                  // owned value
    std::vector<int> m_branchIds;        // owned container
    const llvm::Function* m_function;    // borrowed association
};
```

Syntactically these are just three data members. But their *ownership stories* differ sharply. The `m_name` string is born and dies with the `FunctionSummary` — it is owned outright. The `m_branchIds` vector likewise owns its integers. The `m_function` pointer owns *nothing*: it points at a function that some other part of the program created and will destroy. If that function dies first, `m_function` becomes a dangling pointer, even though the `FunctionSummary` object is still perfectly intact. The compiler will not warn you about this; the relationship lives in your head and in your comments, not in the type.

> **Key insight:** C++ syntax does not, by itself, tell you who owns whom. A pointer member can mean ownership, borrowing, or a loose association. The relationship is something *you* decide and enforce through the class's invariants and cleanup policy.

### The vocabulary

This chapter covers five relationships. Here they are at a glance, each with its plain-English phrase and the C++ shapes it typically takes:

| Relationship | Plain-English phrase | Typical C++ shape |
|---|---|---|
| Composition | part-of / has-a, with owned lifetime | value member, owning smart pointer |
| Aggregation | part-of / has-a, without owned lifetime | pointer / reference / `reference_wrapper` to an external part |
| Association | related-to / knows-about | pointer, reference, ID, handle |
| Dependency | uses-a, temporarily | function parameter, local helper |
| Container | member-of | a class holding many elements |

There is a sixth relationship — **inheritance**, the "is-a" relationship — but it is large enough to deserve its own chapter, so we save it for next time. Everything in this chapter is about objects relating to *other* objects without one being a kind of the other.

### Ownership is the central axis

If you remember only one idea from this chapter, make it this: **ownership is the axis everything else turns on.** Two designs can both read as "has-a" in English and mean entirely different things in C++.

```cpp
class BasicBlockSummary
{
    std::vector<int> m_lineNumbers;      // owns the line-number values
};

class BasicBlockView
{
    const llvm::BasicBlock& m_block;     // observes an existing block
};
```

A `BasicBlockSummary` *owns* its line numbers — they live inside its vector and die with it. A `BasicBlockView` owns nothing; it merely refers to a block that exists elsewhere. If the underlying `BasicBlock` is destroyed, the view is left pointing at a corpse, even though nobody touched the view itself. Same English word ("has"), opposite ownership semantics.

As you read the rest of the chapter, keep asking the same blunt question of every relationship: *if I destroy this object, should the thing it refers to be destroyed too?* If yes, you are looking at composition. If no, you are looking at aggregation, association, or a dependency.

> **Tip:** When you read an unfamiliar class, do an "ownership pass" before anything else. For each data member, ask: does this class create and destroy that thing, or does it just point at something someone else owns? The answer reshapes how you read everything that follows.

### A note on the running examples

Many examples in this chapter (and in the chapter lab) come from program analysis — the world of compilers and LLVM, which is where this vocabulary earns its keep. LLVM's own data model is a chain of relationships:

```
Module
  |
  +-- Function
        |
        +-- BasicBlock
              |
              +-- Instruction
```

A module contains functions, a function contains basic blocks, a block contains instructions — from the data model's point of view, that nesting is close to composition. But the code you write *against* that structure almost always receives borrowed references and pointers *into* it, never ownership of it. Reading a function signature, then, is largely an exercise in reading the relationship:

```cpp
void analyze(const llvm::Function& F);       // a borrowed input — a dependency
std::vector<llvm::Instruction*> worklist;    // a container of borrowed pointers
std::string reportName;                       // an owned value
```

You do not need to know LLVM to follow this chapter. Just notice, each time, which members own and which only borrow. That habit transfers to every C++ codebase you will ever touch.

---

## 23.2 - Composition

We start with the relationship you already use constantly, perhaps without naming it: **composition**, where one object is literally built out of other objects that it owns.

### Composition means owned parts

Object composition builds a larger object out of smaller ones. The strict form of composition — the form most people mean when they say "composition" — has four properties:

- the part is *part of* the whole,
- the part belongs to *one* whole at a time,
- the whole *manages the part's lifetime* (it creates the part and destroys it),
- the part does *not* need to know about the whole.

A good mental model is a human body and a heart. Your heart is part of you. It is not shared with another person. It came into being with your body and will not outlive it. And the heart does its job without needing a concept of "the body it belongs to." That is composition: a strong, owning, part-of relationship.

In C++, the most natural expression of this is a value data member:

```cpp
class SourceLocation
{
private:
    std::string m_file;
    int m_line {};

public:
    SourceLocation(std::string file, int line)
        : m_file { std::move(file) },
          m_line { line }
    {
    }
};

class Diagnostic
{
private:
    SourceLocation m_location;       // composed part
    std::string m_message;           // composed part

public:
    Diagnostic(SourceLocation location, std::string message)
        : m_location { std::move(location) },
          m_message { std::move(message) }
    {
    }
};
```

A `Diagnostic` is composed of a `SourceLocation` and a `std::string`. When you construct a `Diagnostic`, its location and message are constructed as part of it. When that `Diagnostic` is destroyed, both members are destroyed automatically. Nobody using a `Diagnostic` ever has to remember to clean up its location — the ownership is total and automatic.

### The lifetime story, in order

Composition's great virtue is that lifetimes are handled for you, and they nest predictably. When a `Diagnostic` lives and dies, here is the exact sequence:

```
Diagnostic object lifetime:

construct Diagnostic
    construct m_location          (members built first, in declaration order)
    construct m_message
    [constructor body runs]

use Diagnostic

destroy Diagnostic
    [destructor body runs]
    destroy m_message             (members destroyed last, in REVERSE order)
    destroy m_location
```

Two rules are baked into the language here, and they are worth committing to memory because the chapter lab tests you on exactly them:

1. **Members are constructed in declaration order** — the order they appear in the class body — *before* the constructor's body runs. (The order you write them in the member-initializer list does not change this; declaration order wins.)
2. **Members are destroyed in reverse declaration order**, *after* the destructor's body runs.

> **Key insight:** A composed member's constructor runs before the enclosing object's constructor body; its destructor runs after the enclosing object's destructor body. So for a member `m_engine` declared before `m_make`, you will observe `m_engine` constructed first and destroyed last. This is not a quirk — it is a guarantee, and you can rely on it.

> **Warning:** `-Wall` will warn you (via `-Wreorder`, which Clang spells `-Wreorder-ctor`) if your member-initializer list lists members in a different order than they are declared, precisely because the list order is a lie about what actually happens — the compiler initializes in declaration order regardless. Always write the initializer list in declaration order to keep it honest.

### Composition through direct members

The simplest and most common composition uses plain value members:

```cpp
class TestCase
{
private:
    std::string m_name;
    std::vector<std::string> m_inputs;
    std::vector<std::string> m_expectedOutputs;
};
```

Reach for direct value members — this default — whenever:

- the part is *always present* (there is no "a `TestCase` without a name" state),
- the whole *clearly owns* the part,
- the part should share the whole's lifetime exactly.

This covers the large majority of real classes. When in doubt, start here.

### Composition through owning pointers

Sometimes a value member will not do. The part might be *optional*, *expensive* to create, *polymorphic* (you need a base-class pointer to hold different derived types — a Chapter 24/25 idea), or of a type that is *incomplete* at the point of declaration. In those cases, composition can still apply — you just express it with an owning smart pointer instead of a value:

```cpp
class Report
{
private:
    std::unique_ptr<Formatter> m_formatter;

public:
    explicit Report(std::unique_ptr<Formatter> formatter)
        : m_formatter { std::move(formatter) }
    {
    }
};
```

This is *still composition*. The `Report` owns its `Formatter`: when the report is destroyed, the `unique_ptr`'s destructor deletes the formatter automatically. The pointer is an implementation detail of *how* the ownership is stored; it does not change *that* there is ownership. The test is unchanged — destroy the `Report`, and the `Formatter` goes with it.

> **Tip:** "Owning pointer" and "borrowing pointer" look identical in source. The difference is whether the class's destructor (directly, or via a smart pointer) frees the pointee. A `std::unique_ptr` member is owning; a raw `T*` member that you never `delete` is borrowing. Choose your member type so the ownership intent is as visible as possible.

### Why composition is the design to try first

Composition should usually be your *first* design choice, because it keeps complexity local and self-managing:

- each small class can focus on doing one thing well,
- owned members clean themselves up — no manual bookkeeping,
- implementation details stay hidden behind the whole,
- you can change a part's internal representation without disturbing code that uses the whole.

The alternative — one giant class that knows everything — ages badly:

```cpp
class FuzzerRun
{
    // command-line parsing
    // random-seed management
    // execution counters
    // file writing
    // result formatting
};
```

A class like this is hard to test, hard to reason about, and impossible to reuse a piece of. Composition lets you decompose it into focused parts that the `FuzzerRun` simply owns:

```cpp
class FuzzerRun
{
private:
    RandomSeed m_seed;
    ExecutionCounter m_counter;
    ResultLog m_log;
};
```

Now each responsibility lives in its own testable class, and `FuzzerRun` is mostly a coordinator. Each part manages its own lifetime; the whole gets correct cleanup for free.

### Variations on the theme

Most compositions create and destroy their parts in the obvious way — the part is a value member, born and buried with the whole. But a few valid variations exist:

- a part is created *lazily*, the first time it is actually needed,
- a part is *supplied to the constructor* and then owned by the whole (as in the `Report`/`Formatter` example above),
- cleanup is *delegated* to another RAII object the whole holds.

What unites all of these — what makes them composition — is that the whole's public user never manages the part's lifetime by hand. That responsibility stays inside the whole.

### CS6340 tie-in

For the helper classes you write to summarize program-analysis data, composition is almost always cleaner than juggling many parallel variables:

```cpp
struct BranchCounter
{
    int trueEdges {};
    int falseEdges {};
};

class FunctionCoverage
{
private:
    std::string m_functionName;
    BranchCounter m_branches;
    std::vector<int> m_executedBlockIds;
};
```

`FunctionCoverage` owns everything inside it — the name string, the branch counts, the vector of block IDs. It is a tidy bundle of *summary data extracted from* LLVM. Crucially, it does *not* own the LLVM `Function` it summarizes. That distinction is the subject of the next lesson.

---

## 23.3 - Aggregation

Aggregation is composition's quieter sibling. It is still a part-whole relationship — the part really is "part of" the whole, conceptually — but with one decisive difference: **the whole does not own the part's lifetime.**

### Aggregation means non-owning parts

To qualify as aggregation rather than composition, a relationship must satisfy:

- the part is *part of* the whole (the has-a still holds),
- the part may belong to *more than one* whole at the same time,
- the part is *not created or destroyed* by the whole,
- the part does *not* need to know about the whole.

The body-and-heart analogy from the last lesson was composition. The analogy for aggregation is a person and the car they drive. A car can be "part of" a commute, a person owns it in the everyday sense — but if the trip ends, the car does not cease to exist. The same car might be used by a different driver tomorrow. The relationship is real but non-owning.

Here is a textbook example — a study group made of students who exist independently of it:

```cpp
class Student
{
private:
    std::string m_name;

public:
    explicit Student(std::string name)
        : m_name { std::move(name) }
    {
    }

    const std::string& name() const { return m_name; }
};

class StudyGroup
{
private:
    std::vector<std::reference_wrapper<const Student>> m_students;

public:
    void add(const Student& student)
    {
        m_students.push_back(student);
    }

    void print() const
    {
        for (const Student& student : m_students)
            std::cout << student.name() << '\n';
    }
};
```

A `StudyGroup` refers to students, but those students live their own lives outside the group. Several groups can contain the same student. Destroying a group does *not* destroy its students:

```
Student alice  <-----+
Student bob    <---+ |
                   | |
StudyGroup --------+ |
AnotherGroup --------+
```

When a `StudyGroup` is destroyed, only the group's *bookkeeping* (the vector of references) is destroyed. Alice and Bob carry on, undisturbed, ready to join another group. That is the whole point of aggregation.

### Choosing how to store the part

Since the whole does not own the part, it stores some kind of *non-owning handle* to it. You have several options, and the right one depends on the situation:

| Representation | Use when |
|---|---|
| `T&` member | the part is required, and the aggregate need not be reassigned |
| `T*` member | the part is optional, or you need to reseat it later |
| `std::reference_wrapper<T>` | you need to store references inside a container |
| ID / handle | you do not have (or do not want) a direct pointer |

That third row deserves a word, because it trips people up. You cannot make a `std::vector<T&>` — references are not objects, they cannot be reassigned, and a vector needs assignable elements. The fix is `std::reference_wrapper<T>`, a small object that wraps a reference and *is* assignable, so it can live in a container:

```cpp
std::vector<std::reference_wrapper<Student>> roster;
roster.push_back(alice);

roster.front().get().name();     // .get() recovers the underlying Student&
```

You call `.get()` to retrieve the real reference back out. (As you saw in the `StudyGroup::print` loop above, a range-based `for` over a container of `reference_wrapper` can bind each element to a plain `const Student&` directly, because `reference_wrapper` implicitly converts to its reference type — so you often do not even need an explicit `.get()`.)

### The lifetime risk you are signing up for

Aggregation is a *borrowed* relationship, and borrowing comes with an obligation: **the aggregate must not outlive the parts it refers to.** If it does, you are left holding references to objects that no longer exist — dangling references, the same category of bug as returning a reference to a local variable:

```cpp
StudyGroup makeBadGroup()
{
    Student temporary { "temporary" };
    StudyGroup group {};
    group.add(temporary);
    return group;                // group now references a dead local
}
```

When this function returns, `temporary` is destroyed, but the returned `group` still holds a reference to it. Every later use of that reference is undefined behavior. Aggregation does not free you from thinking about lifetimes — it *moves* that thinking somewhere else. Someone, somewhere, must own the parts and keep them alive at least as long as every aggregate that borrows them.

> **Warning:** With composition, the language guarantees the part outlives nothing it shouldn't, because the whole owns it. With aggregation, *you* are responsible for ordering lifetimes correctly. The aggregate is only valid while its parts are alive. The chapter lab makes this concrete: a `Car` holds a `Driver*` it does not own, and the test confirms the `Driver` is still alive after the `Car` that referenced it has been destroyed.

### Composition versus aggregation: the one question

The line between composition and aggregation comes down to a single question you ask of every part:

> If the whole is destroyed, should the part be destroyed too?

"Yes" means composition. "No" means aggregation. A few worked examples:

| Example | Relationship |
|---|---|
| `Diagnostic` owns its message string | Composition |
| `CourseRoster` refers to students that exist elsewhere | Aggregation |
| `FunctionCoverage` owns copied counter values | Composition |
| `PassWorklist` points at LLVM instructions owned by a function | Aggregation |

### CS6340 tie-in

Aggregation is the bread and butter of LLVM pass code. A pass collects pointers to IR objects it wants to act on, but those objects belong to the function or module, not to the pass:

```cpp
class MutationCandidates
{
private:
    std::vector<llvm::Instruction*> m_candidates;

public:
    void add(llvm::Instruction& instruction)
    {
        m_candidates.push_back(&instruction);
    }
};
```

`MutationCandidates` owns the *vector* — that vector is a composed value member, born and buried with the object. But the `Instruction` objects the vector points at are owned by the surrounding IR. If the IR is rewritten or deleted out from under you, those pointers go stale even though your `MutationCandidates` object is untouched. Owning the container is not the same as owning the contents — a distinction we will sharpen in the container-classes lesson.

---

## 23.4 - Association

So far the relationships have all been part-whole: the part belongs *inside* the whole, conceptually. **Association** drops that. An association is a relationship between two objects that are genuinely *separate* — neither is part of the other — but that nonetheless need to know about each other.

### Association means related, not part-of

The defining properties:

- the associated object is *not a part of* the object,
- the associated object may be associated with *many* objects,
- *neither object manages the other's lifetime*,
- awareness can be *one-way or two-way*.

The analogy: a doctor and a patient. A patient is not part of the doctor, and a doctor is not part of the patient. A doctor sees many patients; a patient may see several doctors. Neither creates or destroys the other. They are simply *related* — the doctor knows their patients, and perhaps each patient knows their doctor. That loose, mutual "knows-about" is association.

```cpp
class Issue;

class Developer
{
private:
    std::vector<Issue*> m_assignedIssues;    // associated, not owned
};
```

A developer is not *made of* issues, and the issues are not destroyed when the `Developer` object goes away. The relationship is "is-assigned-to" or "works-on" — emphatically not "part-of."

### Unidirectional association

The simplest case: one object knows about the other, but not the reverse.

```cpp
class Project;

class BuildJob
{
private:
    const Project* m_project {};

public:
    explicit BuildJob(const Project& project)
        : m_project { &project }
    {
    }
};
```

A `BuildJob` can find its `Project` — it holds a pointer to it. But the `Project` keeps no list of its build jobs; it does not even know they exist. The arrow of awareness points one way. This is the easier kind of association to maintain, because there is only one side to keep correct.

### Bidirectional association

Sometimes both objects must know about each other:

```cpp
class Reviewer;

class Review
{
private:
    Reviewer* m_reviewer {};
};

class Reviewer
{
private:
    std::vector<Review*> m_reviews {};
};
```

A `Review` knows its `Reviewer`, and a `Reviewer` knows their `Review`s. This is more powerful but more fragile, because now there are *two* sides that must stay consistent. Every change has to be made in both places at once. Reassigning a review to a new reviewer is not one operation but three:

```
old reviewer must remove the review from its list
new reviewer must add the review to its list
the review must update its pointer to the new reviewer
```

Miss any one of those and the two sides disagree — a review points at a reviewer who has never heard of it, or vice versa. Such inconsistencies are a classic source of subtle bugs.

> **Best practice:** Prefer a one-way association whenever it actually answers the question your program needs to ask. Add the second direction only when you genuinely need to navigate both ways, and accept that you have taken on the burden of keeping both sides in sync.

### Reflexive association

An association can relate two objects of the *same* type. This is a **reflexive** association.

```cpp
class Course
{
private:
    std::string m_code;
    const Course* m_prerequisite {};

public:
    Course(std::string code, const Course* prerequisite = nullptr)
        : m_code { std::move(code) },
          m_prerequisite { prerequisite }
    {
    }
};
```

A `Course` can point at another `Course` as its prerequisite. Because the relationship links courses to courses, it can form chains:

```
Advanced Analysis -> Software Testing -> Data Structures -> Programming I
```

Reflexive associations turn up whenever objects naturally reference others of their own kind — a node and its parent in a tree, an employee and their manager, a course and its prerequisite.

### Indirect association: you don't always need a pointer

An association does not have to be stored as a pointer or reference. Sometimes an *identifier* is enough to find the associated object when you need it:

```cpp
class UserSession
{
private:
    int m_userId {};

public:
    explicit UserSession(int userId)
        : m_userId { userId }
    {
    }
};
```

A `UserSession` is associated with a user — but it stores only an integer ID, not a `User*`. When the session needs the actual user record, it looks it up by ID in whatever holds the users (a database, a map, a registry). This indirection is often *safer* than a raw pointer: an ID cannot dangle. If the user is gone, the lookup simply fails, instead of the pointer silently pointing at freed memory. It is also handy when no live pointer is available — for instance, when the user lives in a database row, not in memory.

### Association versus aggregation

Both aggregation and association are *non-owning* — neither controls the other's lifetime — so what separates them? The difference is conceptual, about whether the target is a *part of* the whole:

| Question | Aggregation | Association |
|---|---|---|
| Is the target *part of* the whole? | Yes | No |
| Is its lifetime owned here? | No | No |
| Is the relationship often bidirectional? | Usually no | Often |
| Typical example | a department has teachers | a doctor sees patients |

In practice the C++ code can look identical — both might be a `T*` member or a vector of pointers. The distinction lives in your design intent, which is exactly why you document it.

CS6340 examples make the contrast vivid:

```cpp
std::vector<llvm::BasicBlock*> blocksInFunction;             // aggregation: blocks are parts of the function
std::unordered_map<int, llvm::Instruction*> idToInstruction; // association: a lookup keyed by ID
```

The first is aggregation — the blocks really are constituent parts of the function, gathered for processing. The second is association by ID — a map that lets you find an instruction given a numeric key, with no implication that the map "contains" the instructions as parts.

---

## 23.5 - Dependencies

The four relationships so far have all been *stored* — they live as data members, persisting as part of an object's state. **Dependency** is different. It is the most fleeting relationship of all: one object or function *uses* another to get a job done, then forgets about it entirely.

### Dependency means uses-a, temporarily

A dependency exists when code needs another object to accomplish a task *during a single operation*, without keeping a lasting relationship to it.

```cpp
class Point
{
private:
    int m_x {};
    int m_y {};

public:
    void print(std::ostream& out) const
    {
        out << '(' << m_x << ", " << m_y << ')';
    }
};
```

`Point::print` *depends on* a `std::ostream` to do its work — it needs somewhere to write the text. But the `Point` does not own the stream, does not store it as a member, and the moment `print` returns, the relationship is over. The next call to `print` might use an entirely different stream. The dependency exists only for the duration of the call.

The analogy: you depend on a bus to get to work. You use it for the trip, then you are done with it — you do not take the bus home with you and store it in your garage. Tomorrow you might catch a different bus. The relationship is real but momentary, scoped to the journey.

### Dependencies are usually function-level

Because they are temporary, dependencies typically show up as *function parameters* rather than data members:

```cpp
class RandomChoice
{
public:
    int pick(std::mt19937& rng, int upperBound)
    {
        std::uniform_int_distribution<int> dist(0, upperBound);
        return dist(rng);
    }
};
```

`pick` depends on a random-number engine for *this one call*. The `RandomChoice` object does not remember the engine afterward — it holds no `m_rng` member. Pass a different engine next time, and `pick` happily uses that instead. The relationship is created when `pick` is called and dissolved when it returns.

### Dependency versus association: is it stored?

This is the test that separates a dependency from an association, and it is wonderfully simple: **is the relationship stored?**

```cpp
class LoggerDependency
{
public:
    void write(std::ostream& out, std::string_view message) const;
};

class LoggerAssociation
{
private:
    std::ostream* m_out {};

public:
    explicit LoggerAssociation(std::ostream& out)
        : m_out { &out }
    {
    }
};
```

`LoggerDependency::write` takes a stream as a parameter — it *depends on* a stream per call, and remembers nothing. `LoggerAssociation` stores a `std::ostream*` as a member — it keeps a *standing association* with one particular stream for its whole life. Same underlying resource, two different relationships, distinguished entirely by whether the stream is a parameter or a member.

> **Key insight:** If you find yourself promoting a function parameter to a data member so the object can "remember" it between calls, you have just converted a dependency into an association. That is a real design decision with real consequences for lifetime: now the object must not outlive the thing it stores a pointer to. The chapter lab makes you feel this directly — `Car::tuneUp(Mechanic&)` takes the mechanic as a parameter and must *not* store it, keeping it a pure dependency.

### Even temporary dependencies create coupling

It is tempting to think a temporary relationship costs nothing. It does not. Every dependency *couples* your code to whatever it depends on. A function that takes `std::ostream&` is coupled to stream-style output. A function that takes `llvm::IRBuilder<>&` is coupled to the LLVM IR-construction API:

```cpp
void emitCounterIncrement(llvm::IRBuilder<>& builder,
                          llvm::Value& counter)
{
    // This helper depends on LLVM's IRBuilder API to emit instructions.
}
```

That coupling is not automatically bad — coupling is how code cooperates. The goal is for it to be *honest and localized*: visible right there in the parameter list, so a reader sees exactly what the function needs, and confined to the places that genuinely require it rather than smeared across the whole program. A dependency announced in a signature is a dependency you can reason about. A hidden one — say, a function that reaches out to a global stream — is the kind that surprises you later.

---

## 23.6 - Container classes

So far we have looked at relationships between an object and *one* other object (or a handful). A **container class** specializes in the opposite: holding *many* objects and giving you a clean way to store, organize, and access them.

### What a container class is

A container class is a class whose whole job is to manage a collection of other objects. You have been using them since early in this course:

```cpp
std::array<int, 4> fixed {};
std::vector<int> dynamic {};
std::string text {};
```

`std::string` is, after all, a container of characters; `std::vector` and `std::array` are containers of whatever you put in them. The typical operations a container supports are exactly what you would expect:

- create an empty container,
- add elements,
- remove elements,
- report how many elements it holds,
- clear all elements,
- access an element,
- optionally sort or search.

A container models the **member-of** relationship: the elements are *members of* the container. That is its place in this chapter's vocabulary.

### Value containers versus reference containers

Here the ownership question from earlier in the chapter returns, sharper than ever. A container can hold its elements *by value* or hold *pointers/references* to elements that live elsewhere — and that choice decides who is responsible for the elements' lifetimes.

| Kind | Stores | Who creates/destroys the elements |
|---|---|---|
| Value container | copies / owned values | the container |
| Reference container | pointers or references to external objects | some external owner |

```cpp
std::vector<std::string> names;               // value container
std::vector<llvm::Instruction*> instructions; // reference container
```

The first vector *owns* its strings: they are born inside it, and when the vector is destroyed they are destroyed with it. The second vector owns its *pointer values* — the little addresses — but **not** the `llvm::Instruction` objects those pointers refer to. Destroy that vector and the instructions live on, owned by the IR. This is the aggregation idea from earlier, now at container scale: owning the container is not the same as owning its contents.

> **Tip:** When you see a `std::vector<T*>`, immediately ask the ownership question: does anyone here `delete` those pointers? If the container deletes them, it owns the pointees (a fragile design — prefer `std::vector<std::unique_ptr<T>>` for that). If nobody deletes them, the pointees are owned elsewhere and the vector is just borrowing. The CS6340 pattern is almost always the borrowing kind.

### What a container has to manage: a minimal example

To appreciate what standard containers do for you, it helps to build a tiny one by hand. Here is a bare-bones dynamic array of integers — the kind of thing `std::vector` replaces:

```cpp
class ScoreList
{
private:
    int m_length {};
    int* m_scores {};

public:
    ScoreList() = default;

    explicit ScoreList(int length)
        : m_length { length },
          m_scores { length > 0 ? new int[static_cast<std::size_t>(length)]{} : nullptr }
    {
        assert(length >= 0);
    }

    ~ScoreList()
    {
        delete[] m_scores;
    }

    int length() const { return m_length; }

    int& operator[](int index)
    {
        assert(index >= 0 && index < m_length);
        return m_scores[index];
    }
};
```

Look at how much a *minimal* container already has to juggle: allocation in the constructor, deallocation in the destructor, length tracking, and bounds checking on access. And there is a lurking trap. This class has a raw owning pointer but *no* copy constructor or copy assignment, so the compiler generates shallow-copy versions. Copy a `ScoreList` and you get two objects whose `m_scores` point at the *same* array — and when both destructors run, that array is `delete[]`d twice. That is undefined behavior, almost certainly a crash.

> **Warning:** Any class that owns a raw resource through a pointer must define (or delete) its copy and move operations — the "rule of five" you met in Chapter 22. Forget, and the compiler-generated shallow copy will double-free your resource. This is the single biggest reason to prefer `std::vector` over a hand-rolled array.

### Resizing is allocate-copy-free

Why is `std::vector` worth so much? Because operations that sound trivial are fiddly to get right by hand. Growing a dynamic array is not "make it bigger" — there is no such operation at the memory level. It is a five-step dance:

```
old array: [ 10 20 30 ]
resize to 5

1. allocate a new, larger array:        [ 0 0 0 0 0 ]
2. copy over min(old, new) elements:    [ 10 20 30 0 0 ]
3. delete[] the old array
4. point m_scores at the new array
5. update m_length
```

`std::vector` does exactly this for you, correctly and efficiently, every time you `push_back` past its capacity. Reimplementing it is a fine learning exercise and a poor production decision.

### Insert and remove shift elements

Two more operations expose the cost of a contiguous array. Inserting into the *middle* means shifting every later element right to open a gap:

```
before: [ A B C D ]
insert X at index 1
after:  [ A X B C D ]
              ^ B C D each moved one slot right
```

Removing from the middle shifts every later element left to close the gap:

```
before: [ A B C D ]
remove index 1
after:  [ A C D ]
            ^ C D each moved one slot left
```

Both are linear-time operations. For many tasks — and most CS6340 worklists, which tend to be small and append-heavy — this is perfectly fine. But if your code does frequent inserts and removes in the *middle* of a large collection, a contiguous array is the wrong data structure, and you should reach for something else (a list, a different layout) rather than paying that cost repeatedly.

### Prefer standard containers in real code

The takeaway is not "go write containers." It is the reverse:

```cpp
std::vector<int> scores;
scores.push_back(10);
scores.push_back(20);
```

In production C++, **use the standard containers.** They are correct, fast, exhaustively tested, and they implement the rule of five properly so you never face the double-free trap. Write a custom container only when it provides a genuine abstraction or invariant that the standard library does not — a ring buffer, a fixed-capacity small vector, a domain-specific collection with special rules. "I wanted my own vector" is not such a reason.

### CS6340 tie-in

The container shapes you will meet in Lab 1 separate cleanly along the value/reference line:

```cpp
std::vector<llvm::Instruction*> mutationSites; // reference container — borrowed IR nodes
std::vector<std::string> generatedInputs;      // value container — owned strings
std::unordered_map<std::string, int> counters; // value container — owned keys and values
```

The rule is consistent: a container owns its elements when the elements are *values*. When the elements are *pointers*, the container owns the pointer values but not necessarily the things they point to. Whenever you see a container of pointers into LLVM IR, read it as borrowing — the IR is the owner, and your container is just keeping a list of addresses it cares about.

---

## 23.7 - std::initializer_list

You have been initializing standard containers with brace lists since Chapter 16:

```cpp
std::vector<int> values { 5, 4, 3, 2, 1 };
```

That syntax is so natural you may never have wondered how it works — how `std::vector` knows to treat the five numbers as *elements* rather than constructor arguments. This lesson reveals the mechanism, `std::initializer_list`, and shows you how to give your own classes the same brace-initialization superpower. It is the last tool the chapter lab needs.

### Letting a class accept a brace list

A custom class can support brace-list initialization by providing a constructor that takes a `std::initializer_list<T>`:

```cpp
#include <initializer_list>
#include <vector>

class ScoreVector
{
private:
    std::vector<int> m_scores;

public:
    ScoreVector() = default;

    ScoreVector(std::initializer_list<int> scores)
        : m_scores { scores }
    {
    }
};

ScoreVector scores { 90, 87, 95 };
```

When you write `ScoreVector scores { 90, 87, 95 }`, the compiler bundles those three values into a `std::initializer_list<int>` and passes it to the matching constructor. A `std::initializer_list<T>` is a lightweight, read-only *view* of the elements in the brace list — it does not own them; it points at temporary storage the compiler set up. Because it is lightweight, you normally take it *by value*, exactly as shown.

### Reading the elements out

A `std::initializer_list` is deliberately minimal. It supports just three things:

```cpp
list.size()      // how many elements
list.begin()     // iterator to the first
list.end()       // iterator past the last
```

Notably, it does **not** provide `operator[]`. You cannot write `list[0]`. To read its elements you iterate — either with a range-based `for` loop or with the begin/end iterators:

```cpp
ScoreVector(std::initializer_list<int> scores)
{
    for (int score : scores)
        m_scores.push_back(score);
}
```

> **Tip:** Because `std::initializer_list` has no subscript operator, a range-based `for` loop is the idiomatic way to walk it. The chapter lab's `RouteList` constructor is built around exactly this loop — iterate the list, `push_back` each waypoint into the internal vector.

### Storing into a manually-managed array

If your class manages a raw array itself rather than delegating to `std::vector`, the constructor copies the elements out of the list into its own storage:

```cpp
class TinyArray
{
private:
    int m_length {};
    int* m_data {};

public:
    TinyArray(std::initializer_list<int> values)
        : m_length { static_cast<int>(values.size()) },
          m_data { new int[values.size()]{} }
    {
        std::copy(values.begin(), values.end(), m_data);
    }

    ~TinyArray()
    {
        delete[] m_data;
    }

    TinyArray(const TinyArray&) = delete;
    TinyArray& operator=(const TinyArray&) = delete;
};
```

Two details to notice. First, `std::copy(values.begin(), values.end(), m_data)` walks the list's iterators and copies each element into the freshly allocated array. Second — and this is the recurring lesson of any raw-pointer-owning class — copy operations are **deleted**. Without that, the shallow copy would hand two `TinyArray`s the same `m_data` pointer and lead to a double `delete[]`. A complete container would instead implement correct copy and move operations, or simply store its data in a `std::vector<int>` and let the standard library handle all of this.

### Braces prefer the list constructor

Here is a subtlety that has surprised many a C++ programmer. When a class has *both* an ordinary constructor and an `initializer_list` constructor, brace initialization will *prefer the list constructor* whenever the brace contents can match it. The classic demonstration is `std::vector` itself:

```cpp
std::vector<int> a(5);   // parentheses: the size constructor -> five value-initialized ints
std::vector<int> b{5};   // braces: the list constructor    -> one int, with value 5
```

Those two lines look almost identical and do completely different things. The full picture:

| Syntax | What `std::vector` does |
|---|---|
| `std::vector<int> v(5)` | size constructor — a vector of 5 zeros |
| `std::vector<int> v{5}` | list constructor — a vector of 1 element, value 5 |
| `std::vector<int> v{5, 6}` | list constructor — a vector of 2 elements, 5 and 6 |

> **Best practice:** Use braces `{ }` when the values inside really *are* the elements you want to store. Use parentheses `( )` when the arguments are constructor parameters such as a size or a capacity. Conflating the two is one of the easiest mistakes to make in modern C++, and the compiler will not save you — both compile.

### Be deliberate when delegating constructors

The "braces prefer the list constructor" rule has a sharp edge when one constructor *delegates* to another. If you delegate using braces in a way that happens to match the list constructor, you can accidentally make a constructor delegate *to itself* — infinite recursion at construction time. The safe habit is to use parentheses (direct initialization) when delegating to a non-list constructor:

```cpp
class TinyArray
{
public:
    explicit TinyArray(int length);

    TinyArray(std::initializer_list<int> values)
        : TinyArray(static_cast<int>(values.size())) // parentheses -> the size ctor, not itself
    {
        // copy elements into the storage the size ctor allocated
    }
};
```

The parentheses around `static_cast<int>(values.size())` make this unambiguously a call to the `int` size constructor. Had you written it with braces, you would be inviting the compiler to match the `initializer_list` constructor — and delegate the list constructor to itself.

### Offer list assignment too, if you offer list construction

If your users can *construct* with a brace list, they will reasonably expect to *assign* one as well:

```cpp
Scores scores { 1, 2, 3 };
scores = { 4, 5, 6 };          // users will want this to work
```

To support that second line, provide an assignment operator taking a `std::initializer_list`:

```cpp
class Scores
{
private:
    std::vector<int> m_scores;

public:
    Scores(std::initializer_list<int> scores)
        : m_scores { scores }
    {
    }

    Scores& operator=(std::initializer_list<int> scores)
    {
        m_scores.assign(scores.begin(), scores.end());
        return *this;
    }
};
```

Backing the class with a `std::vector` makes this almost free — `assign` replaces the contents in one call. If you were managing a raw array by hand, list assignment would have to reallocate and copy carefully (or be deleted) to avoid the same shallow-copy traps as the copy constructor.

### Adding a list constructor can quietly change existing code

A final caution, because the consequences are easy to miss. Since brace initialization *prefers* the list constructor, adding an `initializer_list` constructor to an *existing* class can silently change which constructor old code calls:

```cpp
class Window
{
public:
    Window(int width, int height);
    Window(std::initializer_list<int> sizes);
};

Window w { 800, 600 };  // now calls the initializer_list constructor
```

Before the list constructor existed, `Window w { 800, 600 }` called `Window(int, int)`. Adding the `initializer_list` overload re-routes that exact same call to the new constructor — without a single change to the call site. If both constructors do different things, behavior shifts beneath code that nobody edited.

> **Warning:** Introducing an `initializer_list` constructor is a potentially source-breaking change to a class's existing brace-initialization call sites. Add one with full awareness of every place the class is constructed with braces.

---

## 23.x - Chapter 23 summary and quiz

This chapter was about a single, unifying idea: **how objects relate to one another, and who owns whose lifetime.** The same pointer or reference in C++ can express several relationships; what distinguishes them is intent — ownership, part-of-ness, and whether the link is stored or fleeting.

### The relationships at a glance

| Relationship | Owns lifetime? | Part-whole? | Stored long-term? | Typical C++ form |
|---|:---:|:---:|:---:|---|
| Composition | Yes | Yes | Yes | value member, `std::unique_ptr` |
| Aggregation | No | Yes | Yes | pointer, reference, `reference_wrapper` |
| Association | No | No | Yes | pointer, reference, ID, handle |
| Dependency | No | No | Usually no | parameter, local object |
| Container | depends on element type | member-of | Yes | `std::vector<T>`, custom container |

Read the table along two axes. *Ownership* (column one) separates composition — the only owning relationship — from all the rest. *Storage* (column three) separates the standing relationships, which live as data members, from dependency, which lives only for the length of a function call. Container is the odd one out: it is about quantity (many elements) and inherits its ownership from whether those elements are values or borrowed pointers.

### A checklist for choosing a representation

When you are designing a class and have to decide how it should refer to some other object, walk these six questions. They map directly onto the relationships above:

1. Is the target object *owned* here? (Yes points to composition.)
2. Should destroying this object *destroy* the target? (Yes confirms composition; no rules it out.)
3. Can the target be *absent*? (Pushes you toward a pointer or `std::optional` over a value or reference.)
4. Can *multiple* objects refer to the same target at once? (Suggests aggregation or association, not composition.)
5. Should the target *know about this object too*? (A bidirectional association — accept the sync burden.)
6. Is the relationship needed only *during a function call*? (That is a dependency — make it a parameter, not a member.)

### Reading ownership off LLVM-style code

| Code shape | Likely relationship |
|---|---|
| `std::string functionName` | composition / value ownership |
| `std::vector<int> coveredBlocks` | value container |
| `llvm::Function& F` | dependency, or a borrowed required object |
| `llvm::Instruction* I` | borrowed — association or aggregation |
| `std::vector<llvm::Instruction*> candidates` | reference container / aggregation |
| `std::ostream& out` passed to `print` | dependency |

> **Key insight:** Do not let C++ syntax hide ownership from you. A pointer member can mean aggregation, association, or outright ownership — the *type* alone never tells you which. The class's invariants and its cleanup policy are what decide. When you write such a member, make the intent explicit in a comment or, better, in the type itself (`std::unique_ptr` for owning, raw `T*` for borrowing).

### What the chapter lab makes you prove

The Garage Simulation lab turns this entire vocabulary into something you can *watch happen*. Every constructor and destructor appends a line to a shared trace log, and the tests assert the exact contents and order of that log — so you cannot fake your way through. You implement four small classes, each modeling one relationship:

- **`Engine` is composed inside `Car`** — a value member declared before `m_make`. Because of the member-init-order rule from 23.2, you will observe the engine *constructed first* (before the `Car` body runs) and *destroyed last* (after the `Car` destructor body runs). The trace proves it: `"Engine built"` precedes `"Car built"`, and `"Car destroyed"` precedes `"Engine destroyed"`.
- **`Driver` is aggregated by `Car`** — held as a `Driver*` and *never deleted*. The test confirms the driver is still alive after the car that referenced it is gone. Writing `delete m_driver` in the car's destructor would violate the aggregation rule (23.3) and corrupt a driver the car never owned.
- **`Mechanic` is a dependency** — passed by reference to `Car::tuneUp()` and *not stored*. The relationship lasts exactly one function call (23.5). Storing the mechanic in a member would silently turn the dependency into an association.
- **`RouteList` is a container class** with a `std::initializer_list` constructor, so you can write `RouteList r { "Home", "Gas station", "Destination" }` and have it iterate the list and copy each waypoint into its vector (23.6, 23.7).

When the trace log says the right things in the right order, you have not just memorized the vocabulary — you have implemented the C++ lifetime rules that give it meaning. That is the whole point of the chapter.
