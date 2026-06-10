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

### Why relationship vocabulary matters

Classes do not exist in isolation. Once programs grow past tiny examples, the
main design questions become relationship questions:

- Is one object part of another object?
- Does one object own another object's lifetime?
- Can several objects refer to the same object?
- Is this relationship temporary, or stored as part of object state?
- Does one object merely use another object to complete a task?

The answer changes the C++ representation.

```cpp
class FunctionSummary
{
    std::string m_name;                  // owned value
    std::vector<int> m_branchIds;        // owned container
    const llvm::Function* m_function;    // borrowed association
};
```

Those three members look similar syntactically, but their ownership stories are
different.

### Common object relationship words

| Relationship | Plain-English phrase | C++ shape |
|---|---|---|
| Composition | part-of / has-a with owned lifetime | value member, owning smart pointer |
| Aggregation | part-of / has-a without owned lifetime | pointer/reference/reference_wrapper to external part |
| Association | related-to / knows-about | pointer, reference, ID, handle, weak link |
| Dependency | uses-a temporarily | function parameter, local helper, included service |
| Container | member-of | class holding many elements |
| Inheritance | is-a | derived class from base class |

This chapter covers everything except inheritance, which comes next.

### Ownership is the central axis

Two designs can both say "has-a" in English but mean different things in C++.

```cpp
class BasicBlockSummary
{
    std::vector<int> m_lineNumbers;      // owns line number values
};

class BasicBlockView
{
    const llvm::BasicBlock& m_block;     // observes an existing block
};
```

The summary owns its data. The view does not own the LLVM block. If the LLVM
block dies, the view is no longer valid.

### CS6340 tie-in

LLVM itself is built around object relationships:

```
Module
  |
  +-- Function
        |
        +-- BasicBlock
              |
              +-- Instruction
```

That hierarchy is close to composition from the perspective of the LLVM data
model: modules contain functions, functions contain blocks, and blocks contain
instructions. But pass code usually receives borrowed references or pointers into
that structure. Reading signatures means reading the relationship:

```cpp
void analyze(const llvm::Function& F);       // dependency/borrowed input
std::vector<llvm::Instruction*> worklist;    // aggregation/reference container
std::string reportName;                      // owned value
```

---

## 23.2 - Composition

### Composition means owned parts

Object composition builds a larger object out of smaller objects. In the strict
composition subtype:

- the part is part of the whole,
- the part belongs to one whole at a time,
- the whole manages the part's lifetime,
- the part does not need to know about the whole.

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

When a `Diagnostic` is constructed, its `SourceLocation` and `std::string`
members are constructed. When the `Diagnostic` dies, those members die too.

### Composition lifetime diagram

```
Diagnostic object lifetime:

construct Diagnostic
    construct m_location
    construct m_message

use Diagnostic

destroy Diagnostic
    destroy m_message
    destroy m_location
```

The user of `Diagnostic` does not separately delete or clean up the location.

### Composition through direct members

The simplest composition uses direct data members:

```cpp
class TestCase
{
private:
    std::string m_name;
    std::vector<std::string> m_inputs;
    std::vector<std::string> m_expectedOutputs;
};
```

This is usually the best design when:

- the part is always present,
- the whole clearly owns the part,
- the part should have the same lifetime as the whole.

### Composition through owning pointers

Sometimes the part is optional, expensive to create, polymorphic, or incomplete
at the point of declaration. Then composition can use an owning smart pointer.

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

This is still composition if `Report` owns the formatter and controls its
lifetime.

### Composition is usually the first design to try

Composition keeps complexity local:

- each class can focus on one job,
- owned members clean themselves up,
- implementation details can be hidden,
- changing a part's internal representation does not require changing users of
  the whole object.

Instead of making one large class know everything:

```cpp
class FuzzerRun
{
    // command-line parsing
    // random seed management
    // execution counters
    // file writing
    // result formatting
};
```

prefer smaller composed objects:

```cpp
class FuzzerRun
{
private:
    RandomSeed m_seed;
    ExecutionCounter m_counter;
    ResultLog m_log;
};
```

### Composition variants

Most compositions create and destroy their parts directly. Some valid variants:

- a part is created lazily when first needed,
- a part is supplied to the constructor and then owned by the whole,
- cleanup is delegated to another RAII object.

The key point is that the whole's public user does not manage the part's
lifetime manually.

### CS6340 tie-in

For helper classes you write, composition is often cleaner than storing many
parallel variables:

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

This owns summary data extracted from LLVM. It does not own the LLVM
`Function` itself.

---

## 23.3 - Aggregation

### Aggregation means non-owning parts

Aggregation is also a part-whole relationship, but the whole does not manage the
part's lifetime.

To qualify as aggregation:

- the part is part of the whole,
- the part may belong to more than one whole,
- the part is not created or destroyed by the whole,
- the part does not need to know about the whole.

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

`StudyGroup` refers to students, but the students exist independently.

```
Student alice  <-----+
Student bob    <---+ |
                  | |
StudyGroup -------+ |
AnotherGroup -------+
```

Destroying one study group should not destroy the students.

### Aggregation implementation choices

Common representations:

| Representation | Use when |
|---|---|
| `T&` data member | the part is required and the aggregate is not easily assignable |
| `T*` data member | the part is optional or can be reseated |
| `std::reference_wrapper<T>` | references need to be stored in containers |
| IDs/handles | direct object pointer is unavailable or undesirable |

References cannot be stored directly in `std::vector<T&>`, because references
are not assignable objects. Use `std::reference_wrapper<T>` when a container of
references is needed.

```cpp
std::vector<std::reference_wrapper<Student>> roster;
roster.push_back(alice);

roster.front().get().name();     // get() returns Student&
```

### Aggregation lifetime risk

Aggregation is a borrowed relationship. The aggregate must not outlive the parts
it references.

```cpp
StudyGroup makeBadGroup()
{
    Student temporary { "temporary" };
    StudyGroup group {};
    group.add(temporary);
    return group;                // group now holds dangling reference
}
```

This is the same lifetime danger as returning references to locals. Aggregation
is useful, but it requires clear ownership elsewhere.

### Composition vs aggregation

Ask: "If the whole is destroyed, should the part be destroyed too?"

| Example | Relationship |
|---|---|
| `Diagnostic` owns its message string | Composition |
| `CourseRoster` refers to existing students | Aggregation |
| `FunctionCoverage` owns copied counter values | Composition |
| `PassWorklist` points at LLVM instructions owned by a function | Aggregation |

### CS6340 tie-in

This is a common pattern in LLVM passes:

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

The vector is owned by `MutationCandidates`, but the `Instruction` objects are
not. The function/module still owns those instructions. If the IR is deleted or
rewritten, the pointers may become invalid.

---

## 23.4 - Association

### Association means related, not part-of

An association is a relationship between otherwise separate objects.

Properties:

- the associated object is not a part of the object,
- the associated object may be associated with multiple objects,
- neither object manages the other's lifetime,
- awareness can be one-way or two-way.

```cpp
class Issue;

class Developer
{
private:
    std::vector<Issue*> m_assignedIssues;    // associated, not owned
};
```

A developer is not made of issues, and issues are not destroyed when the
developer object is destroyed. The relationship is "assigned-to" or "works-on",
not "part-of."

### Unidirectional association

One object knows about the other.

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

`BuildJob` can find its `Project`, but `Project` does not necessarily know about
every `BuildJob`.

### Bidirectional association

Both objects know about each other.

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

Bidirectional associations are harder to maintain because both sides must stay
consistent.

If a review is reassigned:

```
old reviewer must remove review
new reviewer must add review
review must point to new reviewer
```

Prefer one-way associations when they answer the problem.

### Reflexive association

A reflexive association relates objects of the same type.

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

One `Course` can point at another `Course` as its prerequisite. The relationship
can form chains:

```
Advanced Analysis -> Software Testing -> Data Structures -> Programming I
```

### Indirect association

An association does not always need a pointer or reference. An ID can be enough.

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

The session can be associated with a user record through `m_userId`, even though
it does not store a `User*`.

### Association vs aggregation

Both are non-owning. The difference is conceptual:

| Question | Aggregation | Association |
|---|---|---|
| Is the target part of the whole? | Yes | No |
| Is lifetime owned? | No | No |
| Can relationship be bidirectional? | Usually no | Yes |
| Example | department has teachers | doctor sees patients |

CS6340 examples:

```cpp
std::vector<llvm::BasicBlock*> blocksInFunction; // aggregation: blocks are parts
std::unordered_map<int, llvm::Instruction*> idToInstruction; // association by ID
```

---

## 23.5 - Dependencies

### Dependency means uses-a temporarily

A dependency occurs when one object or function uses another object to complete
a task, without storing a long-term relationship.

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

`Point::print` depends on `std::ostream` to write output. `Point` does not own
the stream and does not store it as a member.

### Dependencies are usually function-level relationships

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

`pick` depends on a random-number engine for this call. The `RandomChoice` object
does not remember the engine afterward.

### Dependency vs association

The key test: is the relationship stored?

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

The first class depends on a stream per call. The second class keeps an
association with a stream.

### Dependencies still create coupling

Even a temporary dependency matters. If a function takes `std::ostream&`, it is
coupled to stream-style output. If it takes `llvm::IRBuilder<>&`, it is coupled
to LLVM IR construction.

That coupling can be good when it is honest and localized.

```cpp
void emitCounterIncrement(llvm::IRBuilder<>& builder,
                          llvm::Value& counter)
{
    // This helper depends on LLVM's builder API.
}
```

The dependency is explicit in the parameter list.

---

## 23.6 - Container classes

### What a container class is

A container class stores and organizes multiple objects.

Examples from the standard library:

```cpp
std::array<int, 4> fixed {};
std::vector<int> dynamic {};
std::string text {};
```

Typical container operations:

- create an empty container,
- add elements,
- remove elements,
- report size,
- clear all elements,
- access elements,
- optionally sort or search elements.

Container classes model a **member-of** relationship: elements are members of
the container.

### Value containers vs reference containers

| Container kind | Stores | Lifetime responsibility |
|---|---|---|
| Value container | copies or owned values | container creates/destroys elements |
| Reference container | pointers/references to external objects | external owner creates/destroys elements |

```cpp
std::vector<std::string> names;              // value container
std::vector<llvm::Instruction*> instructions; // reference container
```

The first vector owns strings. The second vector owns pointer values, but not
the LLVM instructions those pointers refer to.

### A minimal custom value container

LearnCpp uses a dynamic integer array to show what a container has to manage.
Here is a compact original version:

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

Even this small class needs:

- allocation,
- deallocation,
- length tracking,
- bounds checks,
- copy/move policy.

Without explicit copy behavior, copying `ScoreList` would shallow-copy
`m_scores`, causing double deletion.

### Resizing requires allocation and copying

A manual resize operation is conceptually:

```
old array: [ 10 20 30 ]
resize to 5

1. allocate new array: [ 0 0 0 0 0 ]
2. copy min(old length, new length) elements:
       [ 10 20 30 0 0 ]
3. delete old array
4. point m_scores at new array
5. update m_length
```

That is why `std::vector` is valuable: it already implements this correctly and
efficiently.

### Insert and remove are expensive in array-like containers

Inserting into the middle of a contiguous array requires moving later elements:

```
before: [ A B C D ]
insert X at index 1
after:  [ A X B C D ]
             ^ B C D shift right
```

Removing from the middle shifts elements left:

```
before: [ A B C D ]
remove index 1
after:  [ A C D ]
           ^ C D shift left
```

For many CS6340 tasks, this is fine because vectors of candidates are small or
append-heavy. If frequent middle inserts/removes become central, reconsider the
data structure.

### Prefer standard containers in real code

Writing a custom container is useful for learning, but production C++ should
usually use standard containers:

```cpp
std::vector<int> scores;
scores.push_back(10);
scores.push_back(20);
```

Use a custom container only when it provides a real abstraction or invariant
that standard containers do not.

### CS6340 tie-in

Common Lab 1 container shapes:

```cpp
std::vector<llvm::Instruction*> mutationSites; // borrowed IR nodes
std::vector<std::string> generatedInputs;      // owned strings
std::unordered_map<std::string, int> counters; // owned key/value data
```

The container owns its elements if the elements are values. If the elements are
pointers, the container owns the pointer values but not necessarily the pointees.

---

## 23.7 - std::initializer_list

### List initialization for custom classes

Built-in arrays and standard containers can be initialized with braces:

```cpp
std::vector<int> values { 5, 4, 3, 2, 1 };
```

Custom classes can support the same style by adding a constructor that takes
`std::initializer_list<T>`.

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

`std::initializer_list<int>` is a lightweight view of the elements in the brace
list. It is normally passed by value.

### Accessing initializer_list elements

`std::initializer_list` supports:

```cpp
list.size()
list.begin()
list.end()
```

It does not provide `operator[]` directly. Use a range-based for loop or
iterators.

```cpp
ScoreVector(std::initializer_list<int> scores)
{
    for (int score : scores)
        m_scores.push_back(score);
}
```

### Manual storage example

For a class managing a dynamic array manually:

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

This example deletes copy operations to avoid shallow-copy bugs. A complete
container would implement correct copy/move operations or store data in
`std::vector<int>` instead.

### Brace initialization prefers list constructors

If a class has both a normal constructor and an initializer-list constructor,
brace initialization prefers the initializer-list constructor when it matches.

```cpp
std::vector<int> a(5);   // five ints, all value-initialized
std::vector<int> b{5};   // one int, value 5
```

That distinction is important:

| Syntax | Meaning for vector |
|---|---|
| `std::vector<int> v(5)` | size constructor |
| `std::vector<int> v{5}` | list constructor with one element |
| `std::vector<int> v{5, 6}` | list constructor with two elements |

Use braces when the values inside the braces are the elements. Use parentheses
when the arguments are constructor parameters such as size or capacity.

### Delegating constructors should be deliberate

Inside an initializer-list constructor, use direct initialization when delegating
to a size constructor.

```cpp
class TinyArray
{
public:
    explicit TinyArray(int length);

    TinyArray(std::initializer_list<int> values)
        : TinyArray(static_cast<int>(values.size())) // parentheses-like direct init
    {
        // copy elements
    }
};
```

If delegation accidentally uses brace initialization in a way that matches the
list constructor, the constructor may try to delegate to itself.

### Provide list assignment if list construction is supported

If users can write:

```cpp
Scores scores { 1, 2, 3 };
```

they may expect to write:

```cpp
scores = { 4, 5, 6 };
```

For a class with manual dynamic storage, list assignment must avoid shallow-copy
traps.

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

Using `std::vector` makes this straightforward. With raw arrays, list assignment
needs explicit reallocation/copying or deleted copy assignment.

### Adding list constructors can change existing behavior

Because brace initialization prefers list constructors, adding an
`std::initializer_list` constructor to an existing class can silently change
which constructor old code calls.

```cpp
class Window
{
public:
    Window(int width, int height);
    Window(std::initializer_list<int> sizes);
};

Window w { 800, 600 };  // calls initializer_list constructor
```

This can be surprising if `Window(int, int)` used to be selected.

---

## 23.x - Chapter 23 summary and quiz

### Relationship summary

| Relationship | Owns lifetime? | Part-whole? | Long-term stored link? | Typical C++ form |
|---|---:|---:|---:|---|
| Composition | Yes | Yes | Yes | value member, `std::unique_ptr` |
| Aggregation | No | Yes | Yes | pointer, reference, `reference_wrapper` |
| Association | No | No | Yes | pointer, reference, ID, weak link |
| Dependency | No | No | Usually no | parameter, local object |
| Container | Depends on element type | member-of | Yes | `std::vector<T>`, custom container |

### Design checklist

Before choosing a representation, ask:

1. Is the target object owned here?
2. Should destroying this object destroy the target?
3. Can the target be absent?
4. Can multiple objects refer to the same target?
5. Should the target know about this object too?
6. Is the relationship only needed during a function call?

### CS6340/LLVM relationship examples

| Code shape | Likely relationship |
|---|---|
| `std::string functionName` | composition/value ownership |
| `std::vector<int> coveredBlocks` | value container |
| `llvm::Function& F` | dependency or borrowed required object |
| `llvm::Instruction* I` | borrowed optional association/aggregation |
| `std::vector<llvm::Instruction*> candidates` | reference container / aggregation |
| `std::ostream& out` passed to `print` | dependency |

The recurring lesson: do not let the C++ syntax hide ownership. A pointer member
can mean aggregation, association, or ownership. The class invariant and cleanup
policy decide which one it is.
