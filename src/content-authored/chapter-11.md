# Chapter 11 — Function Overloading and Function Templates

> Source: <https://www.learncpp.com/> (Chapter 11)

So far, every function you've written has had a unique name and worked on one fixed set of types. That's a fine starting point, but it scales badly. Imagine you need a function that returns the smaller of two values — and you need it for `int`, for `double`, for `char`, and probably for a few more types down the road. Do you write `smallerInt`, `smallerDouble`, `smallerChar`, each forcing the caller to remember which name goes with which type? And when the underlying algorithm is *identical* in every case, do you really want to maintain four copies of the same code?

This chapter is about two C++ features that attack exactly this problem. **Function overloading** lets several functions share one name, distinguished by their parameters — so the caller writes `smaller(...)` and the compiler figures out which version to run. **Function templates** go further: you write the algorithm *once*, with the type left as a blank to be filled in, and the compiler generates a concrete function for each type you actually use. Along the way we'll cover the rules that make these features predictable — how the compiler tells overloads apart, how it picks the best match for a call, how `= delete` lets you forbid calls you never want to happen, and how default arguments let one function serve several call shapes. By the end you'll understand why `std::vector<T>` and `std::array<int, 5>` look the way they do, and you'll be ready to build a small generic toolkit of your own.

## Contents
- [11.1 — Introduction to function overloading](#111--introduction-to-function-overloading)
- [11.2 — Function overload differentiation](#112--function-overload-differentiation)
- [11.3 — Function overload resolution and ambiguous matches](#113--function-overload-resolution-and-ambiguous-matches)
- [11.4 — Deleting functions](#114--deleting-functions)
- [11.5 — Default arguments](#115--default-arguments)
- [11.6 — Function templates](#116--function-templates)
- [11.7 — Function template instantiation](#117--function-template-instantiation)
- [11.8 — Function templates with multiple template types](#118--function-templates-with-multiple-template-types)
- [11.9 — Non-type template parameters](#119--non-type-template-parameters)
- [11.10 — Using function templates in multiple files](#1110--using-function-templates-in-multiple-files)
- [11.x — Chapter 11 summary and quiz](#11x--chapter-11-summary-and-quiz)

---

## 11.1 — Introduction to function overloading

### One operation, many type shapes

Here's the situation overloading is built for. You have a single conceptual operation — "score this value," say — but the value might arrive as an `int` or as a `double`. Without overloading, you're forced to encode the type into the name:

```cpp
int scoreInt(int x)
{
    return x + 10;
}

double scoreDouble(double x)
{
    return x + 10.0;
}
```

This works, but look at what it asks of the *caller*. To call the right function, they have to know and remember the type they're holding and which suffix corresponds to it. The function names are cluttered with implementation detail (`Int`, `Double`) that has nothing to do with what the function *means*.

**Function overloading** lets multiple functions share the same name, as long as their parameter lists differ enough for the compiler to tell them apart. The same pair, overloaded:

```cpp
int score(int x)
{
    return x + 10;
}

double score(double x)
{
    return x + 10.0;
}
```

Now the caller just asks for the operation by its real name, and the compiler chooses the right version based on the argument's type:

```cpp
score(4);    // calls score(int)
score(4.5);  // calls score(double)
```

The caller's intent — "score this thing" — is front and center; the type-juggling happens silently and correctly.

### What is an overload?

Each function that shares the name is called an **overload**, and together they form an **overload set**.

```cpp
void logValue(int value);
void logValue(double value);
void logValue(std::string_view value);
```

All three are named `logValue`. Each accepts a different parameter type, so the compiler can keep them straight. When you call `logValue(...)`, it inspects your argument and dispatches to the matching overload.

### When overloading is the right tool

Overloading shines when several functions do the *same conceptual job* on different inputs:

- print a value,
- compare two values,
- construct an object from different kinds of source data,
- parse data from different sources,
- handle one item versus a whole collection.

The unifying test is this: the overload set should still feel like *one* operation seen from the caller's side.

Good — both overloads clearly mean "print a token":

```cpp
void printToken(int tokenId);
void printToken(std::string_view spelling);
```

Questionable — these share a name but do completely unrelated things:

```cpp
void process(int count);          // run count iterations
void process(std::string path);   // delete a file at path
```

The second pair will compile, but it's a trap for readers. Overloading is not a license to reuse one name for unrelated behavior; if two functions don't belong to the same idea, give them different names.

> **Best practice:** Overload a name only when the functions perform the same conceptual operation. If a reader couldn't predict what an overload does from the name alone, it shouldn't be in the set.

### A look ahead: overloads in real APIs

This pattern is everywhere in production C++. In the LLVM-style code you'll meet later in your studies, you'll see functions overloaded across a family of related types:

```cpp
void dumpName(const Function& function);
void dumpName(const BasicBlock& block);
void dumpName(const Instruction& instruction);
```

These read cleanly precisely *because* they're overloads of one idea — "print the name of this program element." You don't have to learn three function names; you learn one operation that happens to accept three kinds of input.

---

## 11.2 — Function overload differentiation

There are really two separate questions hiding behind "overloading," and keeping them apart will save you a lot of confusion. The first is: *can these declarations legally coexist?* That's **differentiation**, and it's the subject of this lesson. The second — *given a call, which overload runs?* — is **overload resolution**, which we tackle in 11.3. Differentiation comes first, because the compiler has to accept your overload set before it can ever choose among its members.

### Differentiation: can these declarations coexist?

For two functions to share a name, the compiler must be able to tell them apart from their declarations alone. The thing that distinguishes them is some difference in their **parameter list**.

```cpp
void print(int);
void print(double); // ok: parameter type differs
```

These two are differentiated — one takes an `int`, the other a `double`. But this is not:

```cpp
void print(int);
void print(int);    // error: same parameter list, not a new overload
```

Declaring `print(int)` twice doesn't create two overloads; it's just the same function declared twice, and if you tried to *define* both, you'd get a redefinition error.

### What counts as a difference?

Here is the full picture of what does and doesn't differentiate overloads:

| Function property | Differentiates? | Why |
|---|---|---|
| Number of parameters | Yes | `f(int)` and `f(int, int)` take different argument counts |
| Parameter types | Yes | `f(int)` and `f(double)` accept different types |
| Parameter order | Yes | `f(int, double)` and `f(double, int)` differ in arrangement |
| Return type | **No** | the caller may ignore the return value, leaving no way to choose |
| Type aliases | **No** | an alias is the same underlying type |
| Top-level `const` on a by-value parameter | **No** | the parameter is copied either way |

The "No" rows are where beginners get tripped up, so let's walk through each one carefully.

### Number, type, and order of parameters

These three all change the parameter list in a way the compiler can see, so all three differentiate.

Different *number* of parameters:

```cpp
int combine(int a, int b)
{
    return a + b;
}

int combine(int a, int b, int c)
{
    return a + b + c;
}
```

Different *type* or *order*:

```cpp
void record(int line, std::string_view file);
void record(std::string_view file, int line);
```

The second pair takes the same two types but in a swapped order, and that's enough to make them distinct overloads. Use this sparingly, though — call sites with the arguments swapped can be genuinely hard to read, since you have to know the parameter order to know what a call means.

### Type aliases do not create new types

This one surprises people. A type alias (`using`, from Chapter 10) is just a second name for an existing type — it does not invent a new, distinct type. So aliases can't differentiate overloads:

```cpp
using UserId = int;
using GroupId = int;

void printId(int id);
void printId(UserId id);  // same as printId(int) — not a new overload
void printId(GroupId id); // same as printId(int) — not a new overload
```

`UserId` and `GroupId` make your *declarations* more readable, but to the compiler they are all just `int`. Declaring all three is a redefinition error.

If you genuinely want the type system to keep two ID kinds separate — so that a `UserId` and a `GroupId` are not interchangeable — you need actual distinct types, which structs (Chapter 13) give you:

```cpp
struct UserId
{
    int value {};
};

struct GroupId
{
    int value {};
};

void printId(UserId id);
void printId(GroupId id); // now genuinely different types
```

> **Key insight:** A type alias is a readability tool, not a type-safety tool. If you need the compiler to enforce a distinction, you need a real type, not an alias.

### Return type alone is not enough

You might expect a different return type to differentiate overloads. It doesn't:

```cpp
int nextValue();
double nextValue(); // error: return type alone cannot differentiate
```

The reason is fundamental to how calls work. Consider:

```cpp
nextValue(); // return value ignored
```

Here the return value is simply thrown away, and the compiler has *no information* to decide which version you meant. Because such a call must always be legal, C++ refuses to let return type be the deciding factor — even in contexts where the return value *is* used. The rule is blunt and absolute: return type never differentiates overloads.

### Top-level `const` on a by-value parameter is not enough

A by-value parameter receives a *copy* of the argument. Whether that copy is `const` is purely the function's internal business — it changes nothing about how the function is called.

```cpp
void consume(int value);
void consume(const int value); // NOT a separate overload
```

Both functions get a copied `int`; the `const` only governs whether the function body is allowed to modify its own local copy. From the outside, the two signatures are indistinguishable, so they don't differentiate.

This will change once you reach references and pointers (Chapter 12), where `const` is part of the parameter *type* and genuinely does differentiate:

```cpp
void inspect(int& value);
void inspect(const int& value); // these ARE different — different parameter types
```

For now, just remember the narrow rule: top-level `const` on a *by-value* parameter is invisible to the caller and therefore can't create a new overload.

### How overloads survive to link time: name mangling

You might wonder how this works under the hood. Compiled object files identify functions by name, and at the level of raw symbols, two functions can't share a name — so how do `print(int)` and `print(double)` coexist after compilation?

The answer is **name mangling**. The compiler encodes the parameter information into the symbol name it emits, producing a distinct symbol for each overload:

```text
source name:     visit(int)
mangled idea:    visit__int

source name:     visit(double)
mangled idea:    visit__double
```

The exact mangled spelling is compiler-specific and not something you'll ever type. The point to take away is that overloading isn't magic at link time: distinguishable signatures become distinguishable symbols, which is exactly why two functions with the *same* signature (differing only in return type) can't coexist — they'd mangle to the same symbol.

---

## 11.3 — Function overload resolution and ambiguous matches

Now for the second question. Differentiation got your overload set accepted by the compiler. **Overload resolution** is the process that runs at every call site to decide *which* overload that particular call invokes. When your argument types exactly match a parameter list, this is trivial. The interesting cases — and the bugs — happen when no overload is a perfect fit and the compiler has to consider conversions.

### The easy case: exact matches

```cpp
void print(int);
void print(double);

print(5);   // exact int match
print(5.5); // exact double match
```

When an argument's type exactly matches a parameter's type, that overload is chosen and there's nothing to debate. The rules below only come into play when *no* overload matches exactly.

### The matching ladder

When there's no exact match, the compiler tries progressively more permissive ways to make the call work. It checks each rung of this ladder in order, and stops at the first rung that yields a usable match:

1. **Exact match** (including a few trivial adjustments).
2. **Numeric promotion** — e.g. `char`/`short` → `int`, `float` → `double`.
3. **Numeric conversion** — e.g. `int` → `double`, `double` → `int`.
4. **User-defined conversion** — a conversion you defined (e.g. via a constructor).
5. **Ellipsis match** — the `...` catch-all (rare; you'll likely not use it).
6. **No match** — compile error.

The crucial rule: at each rung, the compiler looks for *exactly one* best candidate. If exactly one overload matches at the current rung, it wins. If *two or more* match equally well at the same rung and neither is better, the call is **ambiguous** and won't compile.

```text
call expression
    |
    v
exact match?      -- exactly one --> use it
    |
numeric promotion? -- exactly one --> use it
    |
numeric conversion? -- exactly one --> use it
    |
user-defined conv?  -- exactly one --> use it
    |
ellipsis?           -- exactly one --> use it
    |
no match, or two equally good at one rung --> compile error
```

> **Key insight:** Overload resolution is greedy by rung. A *worse-ranked* match on the better overload still loses to a *better-ranked* match — the compiler prefers a promotion over a conversion, and an exact match over either, regardless of which overload you might have "meant."

### Exact matches and trivial adjustments

A handful of harmless adjustments still count as part of the "exact match" family — for instance, dropping the top-level `const` from a by-value argument. You don't need to memorize the list; just know that "exact" is slightly more forgiving than a literally identical type.

```cpp
void handle(int);
void handle(double);

handle(3);   // exact: handle(int)
handle(3.0); // exact: handle(double)
```

### Promotions beat conversions

This is the most important practical rule on the ladder, because it explains a lot of "wait, why did *that* overload run?" moments. A **numeric promotion** (a safe widening, like `char` → `int`) is preferred over a broader **numeric conversion** (like `char` → `double`).

```cpp
void show(int);
void show(double);

char c { 'A' };
show(c); // calls show(int)
```

Why `show(int)`? Because `char` → `int` is a *promotion* (rung 2), while `char` → `double` is a *conversion* (rung 3). The compiler finds a match on the earlier rung, so it never even considers the later one. Promotion wins.

### When the call is ambiguous

Ambiguity happens when two overloads sit on the same rung and neither is clearly better:

```cpp
void draw(long);
void draw(double);

draw(10); // ERROR: ambiguous
```

Here `10` is an `int`. Converting `int` → `long` and `int` → `double` are *both* numeric conversions (rung 3), and neither is ranked above the other. The compiler has a genuine tie and refuses to guess. You resolve it by making your intent explicit:

```cpp
draw(10L);                        // make the literal a long
draw(static_cast<double>(10));    // ask for double explicitly
```

> **Tip:** When a call is ambiguous, don't fight the compiler by adding more overloads — that usually makes it worse. Make the argument's type unambiguous at the call site, with a literal suffix or a `static_cast`.

### Resolution with multiple arguments

When a call has several arguments, the winning overload must be *at least as good* for every argument and *strictly better* for at least one. If no single overload meets that bar, the call is ambiguous.

```cpp
void mix(int, double);
void mix(double, int);

mix(1, 2); // ERROR: ambiguous
```

For the first argument (`1`, an `int`), `mix(int, double)` is better — it's an exact match on the first parameter. For the second argument (`2`, an `int`), `mix(double, int)` is better. Each overload wins one argument and loses the other, so neither dominates, and the call is ambiguous.

### Non-template functions versus templates

Here's a rule you'll lean on once templates enter the picture (and it's why the lab works the way it does). When a *non-template* function and a *function template* both match a call equally well, the **non-template function wins**.

```cpp
void print(bool value)
{
    std::cout << std::boolalpha << value << '\n';
}

template <typename T>
void print(T value)
{
    std::cout << value << '\n';
}

print(true); // calls print(bool), the non-template exact match
```

This is almost always what you want: the non-template overload exists precisely to give special treatment to one type, and the rule makes sure that special version is honored rather than swallowed by the generic template. Keep this rule in mind — in this chapter's lab, a `describe(bool)` overload exists exactly so a `bool` argument doesn't fall through to a more generic match.

### Keeping overload sets sane

A few habits keep overload resolution from surprising you:

- Avoid overloads that differ only by easily-converted arithmetic types (like `int` vs `long`) — they invite ambiguity.
- Be careful mixing default arguments (next lesson) with many overloads; together they can make several calls viable at once.
- When the intended overload matters, pin it down at the call site with a `static_cast` or a literal suffix.
- Use deleted overloads — coming up next — to *block* implicit conversions you don't want.

---

## 11.4 — Deleting functions

Sometimes the most useful thing a function can do is refuse to be called. That sounds paradoxical, but consider: overload resolution will happily reach for a *conversion* to make a call work, even when that conversion is exactly what you'd want to forbid. Passing a `char` where you meant an `int`, or a `double` where truncation would lose data — these compile silently and bite you at runtime. The `= delete` specifier is the tool for slamming that door shut at compile time.

### A deleted function is intentionally uncallable

Marking a function `= delete` declares that it exists for the purposes of overload resolution but **must not be called**. If resolution ever selects a deleted function, the program fails to compile.

```cpp
void printCode(int code)
{
    std::cout << code << '\n';
}

void printCode(char) = delete;
void printCode(bool) = delete;
```

With those deletions in place, the following calls are now rejected:

```cpp
printCode('A');  // error: char overload is deleted
printCode(true); // error: bool overload is deleted
```

This is far better than the alternative, where `'A'` would silently promote to `65` and `true` to `1`. If those calls don't make sense for your API, you want them to fail loudly, at compile time, where you can fix them.

### Deleted overloads still participate in resolution

This is the subtle, essential point: a deleted function is **not invisible**. It takes part in overload resolution like any other candidate. If it's the *best* match, it gets selected — and *then* the deletion turns that selection into an error.

```cpp
void setRetryCount(int count);
void setRetryCount(double) = delete;

setRetryCount(3);   // ok: exact match for int
setRetryCount(3.5); // error: best match is the deleted double overload
```

If the `double` overload didn't exist at all, `setRetryCount(3.5)` would *convert* `3.5` to `int` and silently truncate to `3`. By providing a deleted `double` overload, you make that exact match fail instead. The deleted function is acting as a tripwire: "if a `double` shows up here, stop the build."

> **Key insight:** `= delete` doesn't remove a function from consideration — it adds a candidate that, if chosen, is a compile error. You use it to *capture* a dangerous conversion as the best match and reject it, rather than letting resolution silently fall through to a different overload.

### Deleting broad matches to forbid risky conversions

A common pattern is to provide one good overload and then delete the overloads that would otherwise accept risky conversions:

```cpp
void reserveSlots(std::size_t count);

void reserveSlots(int) = delete;    // a negative int would convert to a huge size_t
void reserveSlots(double) = delete; // a double would truncate
```

How strict to be is a judgment call that depends on your codebase — sometimes the convenience of implicit conversion is worth the risk. For now, the takeaway is simply that `= delete` is a precise instrument for making specific calls illegal.

### Looking ahead

In API-design contexts, deleted overloads encode intent. For example, forcing a caller to pass a real object rather than a pointer that might be null:

```cpp
void instrument(Function& function);
void instrument(Function*) = delete; // caller must pass a real object, not a maybe-null pointer
```

You'll appreciate this kind of guardrail once you're working with pointers in Chapter 12 — but you can see the shape of it now.

---

## 11.5 — Default arguments

Often a function has a parameter that *usually* takes the same value, but occasionally needs a different one. A logging function that almost always includes a timestamp; a die-rolling function that's usually a six-sided die. You don't want to force every caller to spell out the common value every time, but you do want them to be *able* to override it. **Default arguments** are how you express "use this value unless told otherwise."

### A default fills in for an omitted argument

A **default argument** is a value supplied for a parameter when the caller leaves that argument out.

```cpp
void repeat(std::string_view text, int times = 1)
{
    for (int count { 0 }; count < times; ++count)
        std::cout << text;
}
```

Now both of these calls work:

```cpp
repeat("x", 3); // times is 3
repeat("x");    // times defaults to 1
```

The mechanism is worth understanding precisely: the default isn't stored inside the function and consulted at runtime. Instead, **the compiler inserts the default value at the call site** — `repeat("x")` is compiled exactly as if you'd written `repeat("x", 1)`. The function itself always receives two arguments; it just may not be the caller who supplied the second.

### Good defaults are common-but-overridable values

Reach for a default when there's a value most callers want, but some will need to change:

```cpp
void writeLog(std::string_view message, bool includeTimestamp = true);
int rollDie(int sides = 6);
```

Defaults are also a graceful way to *extend* an existing function. If you add a new parameter and give it a default, every existing call site keeps compiling unchanged, while new code can opt into the new behavior. That's a real maintenance superpower.

### Defaults must be right-aligned

There's one structural rule, and it follows directly from how calls are matched. Arguments are matched to parameters **left to right**, and C++ gives you no syntax to skip over a parameter in the middle. So once a parameter has a default, *every* parameter to its right must also have one.

```cpp
void connect(std::string_view host,
             int port = 443,
             bool useTls = true); // ok: defaults fill the right end
```

This is illegal, because a parameter *without* a default sits to the right of one *with* a default:

```cpp
void connect(std::string_view host = "localhost",
             int port); // error: non-default parameter after a default
```

And there's no way to supply a later argument while skipping an earlier defaulted one:

```cpp
connect("example.com", , true); // no such "skip the middle" syntax exists
```

If you provide *any* trailing argument, you must provide all the ones before it, because matching is strictly positional from the left.

> **Note:** The reason defaults must be on the right is purely mechanical: with no skip syntax, the compiler fills in defaults from the right inward. A gap in the middle would be unmatchable.

### Put the default in the declaration, not the definition

When a function has a separate forward declaration (typically in a header), the default argument belongs on the **declaration**, and you do *not* repeat it on the definition.

Header:

```cpp
// logger.h
void writeLog(std::string_view message, bool includeTimestamp = true);
```

Source:

```cpp
// logger.cpp
void writeLog(std::string_view message, bool includeTimestamp)
{
    // implementation — no default repeated here
}
```

The logic is simple once you see it. The default has to be visible *at the call site*, because that's where it gets inserted. Call sites include the header, so the default must be there. The `.cpp` definition is often *not* visible to callers — so a default placed only there would be useless, and a default placed in *both* (within one translation unit) is an error.

```text
the default must be visible where the function is called
    -> callers see the header declaration   -> default goes here
    -> callers may not see the .cpp definition -> default does NOT go here
```

> **Best practice:** Put a default argument on the declaration the callers can see (usually the header), and never repeat it on the definition.

### Defaults can create ambiguity with overloads

Because a default makes a call viable with *fewer* arguments, mixing defaults and overloads can produce surprising ambiguities:

```cpp
void draw(int width);
void draw(int width, int height = 10);

draw(5); // ERROR: ambiguous — could be draw(int) or draw(int, 10)
```

A call to `draw(5)` matches `draw(int)` directly, but it *also* matches `draw(int, int)` with the default filling in `10`. Both are equally good, so the call is ambiguous. The default argument isn't part of the signature for differentiation purposes — both overloads are legal to declare — but it affects which calls are viable. Keep overload sets that involve defaults small and obvious.

### Default argument or overload?

These two features overlap, and choosing between them is a design question:

- Use a **default argument** when *one implementation* naturally handles all the cases — the only thing that changes is a value:

  ```cpp
  void printLine(std::string_view text, char ending = '\n');
  ```

- Use **overloads** when the alternate form needs meaningfully *different logic*, not just a different value:

  ```cpp
  void loadConfig();                       // load from the default location
  void loadConfig(std::string_view path);  // genuinely different: load from a given path
  ```

Both make call sites nicer. The mistake to avoid is combining them in a way that forces the reader to guess what a bare call actually does.

---

## 11.6 — Function templates

Overloading solved the *naming* problem — several functions, one name. But it left a problem untouched. Look again at a `smaller` function written for two types:

```cpp
int smaller(int a, int b)
{
    return (a < b) ? a : b;
}

double smaller(double a, double b)
{
    return (a < b) ? a : b;
}
```

The bodies are byte-for-byte identical. The *only* difference is the type, and the type appears purely as a label. If you need a third version for `char`, you copy the body again. If you find a bug, you fix it in every copy. This is duplication that no amount of overloading can remove — because overloading still requires you to write each function's body by hand. What you actually want is to write the algorithm *once* and have the compiler stamp out the typed versions for you. That's a **function template**.

### A template is a pattern, not a function

A **function template** is a description from which the compiler *generates* functions. You write it with the type left as a placeholder:

```cpp
template <typename T>
T smaller(T a, T b)
{
    return (a < b) ? a : b;
}
```

Read this as a stencil. `T` is a **type template parameter** — a stand-in for a real type that will be supplied later. Everywhere `T` appears (the return type and both parameters), it means "whatever type this gets used with." The compiler doesn't run the template directly; instead, when you call `smaller` with `int` arguments, it generates an `int` version, and when you call it with `double` arguments, it generates a `double` version:

```text
the template (a stencil):
    T smaller(T, T)

generated functions (the real code that runs):
    int         smaller(int, int)
    double      smaller(double, double)
    std::string smaller(std::string, std::string)
```

> **Key insight:** The template itself never executes. It's a recipe the compiler uses to *bake* concrete functions for the specific types your program actually uses. The generated functions are what run.

### Reading the template parameter declaration

That first line does three things at once:

```cpp
template <typename T>
```

- The keyword `template` announces that the *next* declaration is a template, not an ordinary function.
- `typename T` declares `T` as a type placeholder.
- `T` is scoped to this one template — it means nothing outside it.

You'll see `class` used instead of `typename` in older code, and the two are interchangeable here:

```cpp
template <class T>
T identity(T value)
{
    return value;
}
```

We'll prefer `typename` throughout this course, for a small readability reason: `T` can be *any* type — `int`, `double`, a pointer, a class — not only a class type, and `typename` says that more honestly than `class` does.

> **Best practice:** Use `typename` rather than `class` for type template parameters. They behave identically; `typename` just reads more accurately.

### Naming template parameters

By convention, simple type parameters get short uppercase names:

| Name | Typical role |
|---|---|
| `T` | the first/only type parameter |
| `U` | a second type parameter |
| `N` | an integer *non-type* parameter (see 11.9) |
| a descriptive name | when the type's requirements aren't obvious |

When the placeholder has expectations attached, a descriptive name documents them:

```cpp
template <typename Iterator>
void advanceOne(Iterator& iterator)
{
    ++iterator;
}
```

Calling it `Iterator` rather than `T` tells the reader: this type is expected to support iterator-like increment. The name is documentation.

### Templates still have requirements

Here's the catch that keeps templates honest. A template is not a promise that the code works for *every* type — it's a promise that the compiler will *try* to generate a function for whatever type you ask. That generation only succeeds if every operation in the body makes sense once `T` is replaced by the real type.

```cpp
template <typename T>
T addOne(T value)
{
    return value + 1;
}
```

This generates fine for arithmetic types:

```cpp
addOne(3);   // T = int:    3 + 1
addOne(2.5); // T = double: 2.5 + 1
```

But it may not do what you'd hope here:

```cpp
addOne("abc"); // T = const char*: this is pointer arithmetic, not "abc1"
```

A string literal is a pointer, and `pointer + 1` is pointer arithmetic — it moves one character forward, it does not append `'1'`. The template "works" in the narrow sense that it compiles, but it doesn't mean what a beginner might expect. The lesson: a template body must still be *meaningful* after substitution. Templates are flexible, not magical.

---

## 11.7 — Function template instantiation

We've called a template a stencil that the compiler uses to "stamp out" functions. That stamping has a name — **instantiation** — and understanding exactly when and how it happens explains several behaviors that otherwise look mysterious: why deduction sometimes fails, why each typed version has its own static variables, and why an unused template generates no code at all.

### Instantiation generates a real function

**Function template instantiation** is the process of generating an actual function from a template for a specific set of template arguments.

```cpp
template <typename T>
T larger(T a, T b)
{
    return (a < b) ? b : a;
}

int x { larger<int>(1, 2) };
```

That call causes the compiler to instantiate `larger` for `T = int`, producing — conceptually — exactly the function you'd otherwise have hand-written:

```cpp
int larger(int a, int b)
{
    return (a < b) ? b : a;
}
```

This generated function is called an **instantiated function** (or informally a *function instance*). It's an ordinary function in every respect; it just came from a stencil rather than from your keyboard.

### Two ways to supply the type: explicit, or deduced

You can tell the compiler the type outright, in angle brackets:

```cpp
larger<int>(1, 2);
larger<double>(1, 2); // T = double; the int arguments are converted to double
```

When you write `larger<double>(1, 2)`, you've *forced* `T = double`, so the instantiated function takes `double` parameters. The integer literals `1` and `2` are then converted to `double` to be passed in. This is **explicit template arguments**.

More often, though, you let the compiler figure out `T` from the arguments themselves — **template argument deduction**:

```cpp
larger(1, 2);     // deduces T = int   from int arguments
larger(1.5, 2.0); // deduces T = double from double arguments
```

This is the preferred form when ordinary call syntax expresses what you mean — it reads like a normal function call.

### Deduction does *not* convert types

Here is the rule that catches everyone at least once. Template argument deduction matches your argument types *exactly*; it does not perform the numeric conversions that overload resolution would. So a single-`T` template cannot deduce one type from two *different* argument types:

```cpp
larger(1, 2.5); // ERROR: argument 1 says T=int, argument 2 says T=double
```

The compiler sees `int` from the first argument and `double` from the second, can't reconcile them into a single `T`, and gives up. You have a few ways out:

```cpp
larger<double>(1, 2.5);                 // force T = double explicitly
larger(static_cast<double>(1), 2.5);    // make both arguments the same type
```

Or — when mixed types are genuinely what you want — write a template with *two* type parameters, which is exactly the subject of 11.8.

> **Warning:** Deduction is not conversion. `larger(1, 2.5)` does not "promote" the `int` to match the `double` — it fails, because deduction needs every argument to agree on `T`. This is a deliberate design choice that keeps deduction predictable.

### `<>` forces the template, even when a non-template exists

Recall from 11.3 that when a non-template function and a template both match, the non-template wins. Sometimes you want the template anyway. Empty angle brackets, `<>`, say "consider templates only, but still deduce the type from the arguments":

```cpp
int larger(int a, int b)
{
    return (a < b) ? b : a;
}

template <typename T>
T larger(T a, T b)
{
    return (a < b) ? b : a;
}
```

```cpp
larger(1, 2);      // prefers the non-template larger(int, int)
larger<>(1, 2);    // templates only, T deduced as int
larger<int>(1, 2); // templates only, T explicitly int
```

In day-to-day code you'll use the plain `larger(1, 2)` form. The `<>` syntax is there for the rare case where you specifically need the generic version.

### Templates instantiate only when used

A template definition can sit in your code generating *nothing* until something actually calls it. The compiler instantiates only the versions a translation unit needs:

```cpp
template <typename T>
T doubleValue(T value)
{
    return value + value;
}
```

If your program only ever calls `doubleValue(3)`, the compiler generates just the `int` version. It does not — and cannot — generate a version for every type in existence; it waits to see which types you actually use. This is why a template can offer infinite possible functions without bloating your program: only the realized ones cost anything.

### Each instantiation has its own static locals

This one can genuinely surprise you. Because each instantiation is a *separate* function, each gets its own copy of any `static` local variable.

```cpp
template <typename T>
int nextId(T)
{
    static int id { 0 };
    return ++id;
}
```

There isn't one shared `id` across all uses of `nextId`. There's a *separate* `id` for each instantiated type:

```text
nextId<int>    has its own static id
nextId<double> has its own static id
nextId<char>   has its own static id
```

So the counters run independently:

```cpp
nextId(1);   // int version:    id becomes 1
nextId(2);   // int version:    id becomes 2
nextId(3.5); // double version: its own id becomes 1
```

If you expected a single global counter shared by all types, this behavior will bite you. Most of the time it's exactly what you want; just know it's per-instantiation.

### Generic programming

Templates unlock a style called **generic programming**: writing an algorithm in terms of the *operations it needs* rather than any one concrete type.

```cpp
template <typename T>
bool isInsideClosedRange(T value, T low, T high)
{
    return !(value < low) && !(high < value);
}
```

Notice this function never mentions `int` or `double`. It only requires that `T` support `<` with the usual meaning. It works for any such type — and notice, too, that it's written entirely in terms of `<` (using `!(a < b)` instead of `a >= b`), so it needs *only* `operator<`, the minimal requirement many standard-library algorithms also assume. That's generic thinking: express the algorithm through the operations, and let any type that provides those operations come along for the ride.

---

## 11.8 — Function templates with multiple template types

A single-`T` template carries a hidden assumption: that all the `T` arguments are the *same* type. That's perfect for `smaller(a, b)`, where comparing two values of one type is the whole point. But plenty of operations naturally combine *different* types — multiply an `int` by a `double`, scale a value by a factor — and there a single `T` gets in the way. This lesson is about templates with more than one type parameter, and the `auto` return type that makes them work cleanly.

### One `T` forces the argument types to match

We saw the failure in 11.7, but it's worth restating in context:

```cpp
template <typename T>
T add(T a, T b)
{
    return a + b;
}

add(1, 2);     // ok: T = int
add(1.0, 2.0); // ok: T = double
add(1, 2.0);   // ERROR: int says T=int, double says T=double
```

With one type parameter, both arguments must agree on what `T` is. The mixed call has no single answer.

### Two type parameters let the arguments differ

Give each parameter its own type, and the arguments are free to differ:

```cpp
template <typename T, typename U>
auto add(T a, U b)
{
    return a + b;
}
```

Now the mixed call just works — `T` is deduced from the first argument, `U` from the second, independently:

```cpp
add(1, 2.0); // T = int, U = double
```

### Why the return type is `auto`

There's a subtlety in that example you shouldn't skip past: the return type is `auto`, and it has to be. Suppose you'd written the return type as `T` instead:

```cpp
template <typename T, typename U>
T badAdd(T a, U b)
{
    return a + b; // the sum may be narrowed back into T
}
```

Call `badAdd(1, 2.5)` — `T = int`, `U = double`. The sum `1 + 2.5` is `3.5`, a `double`. But the return type is `T`, which is `int`, so `3.5` gets narrowed down to `3` on the way out. You've silently lost the fractional part, and the result type doesn't reflect what addition actually produced.

`auto` fixes this by letting the compiler deduce the return type *from the return expression itself*:

```cpp
template <typename T, typename U>
auto add(T a, U b)
{
    return a + b; // return type is whatever (a + b) is — here, double
}
```

Now `add(1, 2.5)` returns `3.5` as a `double`, because the type of `a + b` (after the usual arithmetic conversions) *is* `double`. The return type follows the math instead of fighting it.

> **Key insight:** When a function's result type depends on combining its arguments, let `auto` deduce the return type from the return expression. Pinning it to one of the parameter types can silently narrow the result.

One limitation comes with `auto` return types: a function whose return type is deduced must be *fully defined before it's called*, because the compiler needs to see the body to figure out what the function returns. For templates in headers (the usual case, per 11.10) this is naturally satisfied.

### A note on C++20's abbreviated form

If you're reading newer code, you may encounter `auto` used directly in the parameter list:

```cpp
auto add(auto a, auto b)
{
    return a + b;
}
```

This is a C++20 **abbreviated function template** — each `auto` parameter quietly becomes its own independent type parameter, so the snippet above is equivalent to the two-parameter template we wrote. It's compact and convenient for small, obvious helpers. We mention it for recognition only — this course targets C++17, and the chapter's lab uses the explicit `template <typename T, typename U>` form. Reach for the explicit form anyway whenever the *relationship* between the types matters or their requirements deserve names.

### Templates can be overloaded too

Function templates participate in overloading just like ordinary functions. You can have several templates with the same name and different parameter shapes:

```cpp
template <typename T>
T combine(T a, T b)
{
    return a + b;
}

template <typename T, typename U>
auto combine(T a, U b)
{
    return a + b;
}

template <typename T, typename U, typename V>
auto combine(T a, U b, V c)
{
    return a + b + c;
}
```

The compiler picks the viable template for each call — here, mostly by argument count. As with non-template overloads, if a more specialized template fits better it can be preferred, and if no single best template emerges, the call is ambiguous.

### Choosing one type parameter or several

Let the number of type parameters *document the relationship* you intend:

- **One** type parameter when the arguments should be the *same* type:

  ```cpp
  template <typename T>
  void swapValues(T& a, T& b); // swapping requires matching types
  ```

- **Several** type parameters when the arguments are *expected* to differ:

  ```cpp
  template <typename T, typename U>
  auto scaled(T value, U factor); // value and factor may be different types
  ```

That choice isn't just mechanics — it tells the next reader whether mixing types is a bug or the whole point.

---

## 11.9 — Non-type template parameters

Every template parameter so far has stood for a *type*. But there's a second flavor: a template parameter can stand for a *value* — a constant that the compiler knows at compile time. These are **non-type template parameters**, and once you've seen them you'll recognize them immediately, because you've already been using them: the `5` in `std::array<int, 5>` is one.

### A template parameter can be a value

Compare a familiar type parameter with the new kind:

```cpp
template <typename T> // T is a TYPE
T identity(T value);
```

```cpp
template <int N>      // N is a VALUE
void printN()
{
    std::cout << N << '\n';
}

printN<5>();
```

`N` is not a type — it's an `int` whose value is fixed at compile time and chosen at the call site (`printN<5>` makes `N` equal to `5`). Inside the template, `N` behaves like a compile-time constant.

### Why you'd want one

The point of a non-type parameter is that its value is available *as a constant expression* inside the template — usable anywhere a compile-time constant is required:

```cpp
template <int Max>
bool isValidIndex(int index)
{
    static_assert(Max > 0, "Max must be positive");
    return index >= 0 && index < Max;
}
```

Because `Max` is a compile-time value, it can appear in a `static_assert` (Chapter 7), which is checked during compilation. A regular function parameter couldn't do this — its value isn't known until the program runs. Non-type parameters bridge that gap: they carry a value *into the compile step*.

### What kinds of values are allowed

Modern C++ permits several categories of non-type parameter, including:

- integral values (like `int`, `char`),
- enumeration values,
- `std::nullptr_t`,
- pointers and references with appropriate linkage,
- floating-point values (since C++20),
- certain literal class-type values (since C++20).

For this course, the cases that matter are the simple ones: **integral values** and **enumeration values**. Those cover everything you'll do here.

### You've already seen these: `std::array` and `std::bitset`

Non-type parameters aren't exotic — the standard library hands them to you constantly:

```cpp
#include <array>
#include <bitset>

std::array<int, 5> values {};
std::bitset<8> flags {};
```

Dissecting `std::array<int, 5>`:

| Argument | Kind | Meaning |
|---|---|---|
| `int` | type parameter | the element type |
| `5` | non-type (value) parameter | the array length |

The length is *part of the type*. That's why a `std::array<int, 5>` and a `std::array<int, 6>` are different types — and why the size has to be known when you compile.

### Non-type arguments must be compile-time constants

Since the value is baked into the type, it has to be knowable at compile time. A runtime variable won't do:

```cpp
int length { 5 };
// std::array<int, length> values {}; // ERROR: length is a runtime value

constexpr int fixedLength { 5 };
std::array<int, fixedLength> values {}; // ok: constexpr is a compile-time constant
```

This is the dividing line between `std::array` and `std::vector`: if the size is fixed and known at compile time, `std::array` can encode it in the type; if the size is only known at runtime, you need `std::vector` (Chapter 16), whose size lives at runtime.

### `auto` non-type parameters (C++17)

C++17 lets you write `auto` in place of the non-type parameter's type and have the compiler deduce it:

```cpp
template <auto Value>
void printConstant()
{
    std::cout << Value << '\n';
}

printConstant<42>();  // Value is int 42
printConstant<'x'>(); // Value is char 'x'
```

This is concise, but it hides *what kind* of value the template expects. For beginner code, a spelled-out `template <int N>` is often clearer when you specifically mean an integer — the reader can see the constraint at a glance.

### Looking ahead

Compile-time sizes let array-shaped functions stay self-describing — the length travels with the data instead of being passed (and possibly mismatched) separately:

```cpp
template <std::size_t N>
void printCounters(const std::array<int, N>& counters)
{
    for (std::size_t i { 0 }; i < N; ++i)
        std::cout << i << ": " << counters[i] << '\n';
}
```

Here `N` is deduced from the array's type, so the function always knows the correct length without being told. (This builds on `std::array` from Chapter 17 — it's a preview of how non-type parameters pay off.) In this chapter's lab, you'll use a non-type parameter `template <int N>` to size a buffer at compile time, giving each distinct `N` its own separate instantiation.

---

## 11.10 — Using function templates in multiple files

There's one practical wrinkle about templates that trips up nearly everyone the first time they split a program across files: the usual header/source separation you learned for ordinary functions *does not work* for templates. Understanding *why* — and what to do instead — is the difference between a clean multi-file template and a baffling linker error.

### Ordinary functions: declaration here, definition there

For a normal function, the compiler needs only a *declaration* to compile a call. It emits a "call this function" placeholder, and the linker fills in the address later from the definition, which may live in an entirely different file.

```cpp
// main.cpp
int addOne(int); // declaration is enough to compile the call below

int main()
{
    return addOne(3);
}
```

```cpp
// add.cpp
int addOne(int value) // definition lives here; the linker connects them
{
    return value + 1;
}
```

This works because there's exactly *one* `addOne` function, and compiling a call to it doesn't require its body — only its signature.

### Templates break this, and here's why

A template is fundamentally different: to *instantiate* a function from it, the compiler needs the template's **body**, not just its declaration. Try the same split with a template and watch it fail:

```cpp
// main.cpp
template <typename T>
T addOne(T value); // declaration only — no body

int main()
{
    return addOne(3); // needs to instantiate addOne<int>
}
```

```cpp
// add.cpp
template <typename T>
T addOne(T value) // body lives here
{
    return value + 1;
}
```

When the compiler processes `main.cpp` and hits `addOne(3)`, it must generate `addOne<int>` right there — but it only has the *declaration*, with no body to stamp out. It can't generate the function:

```text
compiling main.cpp:
    sees call addOne<int>
    needs the template body to instantiate
    body is not visible (it's over in add.cpp)
    -> cannot instantiate  -> failure
```

Meanwhile `add.cpp` has the body, but if *nothing in that file* calls `addOne<int>`, the compiler never instantiates it there either — a template generates nothing until something uses it (recall 11.7). So neither file produces `addOne<int>`, and the linker, finding no such function anywhere, reports an undefined-reference error.

### The fix: put template definitions in the header

The solution is to put the *entire template definition* — body and all — in a header, and include that header everywhere the template is used:

```cpp
// add_one.h
#ifndef ADD_ONE_H
#define ADD_ONE_H

template <typename T>
T addOne(T value)
{
    return value + 1;
}

#endif
```

```cpp
#include "add_one.h"

int main()
{
    return addOne(3); // the full template is visible, so addOne<int> can be generated
}
```

Now every translation unit that uses the template sees its full body and can instantiate exactly the versions it needs.

> **Best practice:** Define a function template fully in a header (with include guards or `#pragma once`), and include that header wherever the template is used. Unlike ordinary functions, templates need their *definition*, not just a declaration, at the point of use.

### Why this doesn't violate the one-definition rule

A reasonable worry: if the header lands in five `.cpp` files, won't you end up with five definitions of the same `addOne<int>` and a multiple-definition link error? You won't — and the reason is built into the language. Functions generated from templates are **implicitly inline**, which permits identical definitions to appear across translation units; the linker merges them into one. So the same instantiation showing up in multiple files is expected and handled, not an error.

Hold on to the distinction between the two things in play:

```text
the template definition in the header:
    a pattern, made available to every translation unit that includes it

an instantiated function:
    generated only for the types actually used in a given translation unit,
    and implicitly inline so duplicates across files merge cleanly
```

### A practical rule of thumb

For templates, then:

- define the whole template in the header,
- keep the body small enough to read comfortably (headers get included a lot),
- guard the header (`#ifndef`/`#define`/`#endif` or `#pragma once`),
- and don't drag unrelated implementation detail into the header just because one template lives there.

The flip side is just as important: if a helper is *not* a template, prefer the ordinary split — declaration in the `.h`, definition in the `.cpp` — so you don't recompile its body in every file that includes it. This is exactly the model the chapter's lab uses: every function in the toolkit, templates and overloads alike, lives in a single header, because the templates *must* be there and it keeps the small library in one place.

---

## 11.x — Chapter 11 summary and quiz

You now have two complementary tools for writing flexible functions. Overloading lets several functions share a name when they perform the same conceptual job; templates let you write one algorithm and have the compiler generate typed versions on demand. The rules in between — differentiation, resolution, deletion, defaults, instantiation — are what make these features predictable instead of mysterious.

### Core takeaways

- **Overloading** lets related functions share a name; the compiler picks one by the arguments.
- Overloads are **differentiated** by parameter *number*, *type*, or *order*.
- **Return type alone** never differentiates overloads.
- **Type aliases** don't create distinct overloadable types — an alias is the same type.
- **Top-level `const`** on a by-value parameter doesn't create a new overload.
- **Overload resolution** picks the best match on a ladder: exact beats promotion beats conversion.
- A call is **ambiguous** when two candidates are equally good at the same rung.
- **`= delete`** makes a chosen overload a compile error — use it to forbid unwanted conversions.
- **Default arguments** are inserted at the call site; they must be **right-aligned** and you can't skip a middle argument.
- Put default arguments on the **declaration** callers see (usually the header), not the definition.
- A **function template** is a pattern; **instantiation** generates a real, typed function from it.
- Template argument **deduction does not convert types** — every argument must agree on `T`.
- **Multiple type parameters** allow mixed argument types; pair them with an **`auto` return type** so the result isn't narrowed.
- **Non-type template parameters** carry a compile-time *value* (like the size in `std::array<int, 5>`).
- Template **definitions belong in headers**, because callers need the body to instantiate them.

### Overload-resolution quick reference

| Call situation | Likely outcome |
|---|---|
| an exact parameter-type match exists | that exact overload wins |
| no exact match, but a promotion fits | the promotion match wins |
| only conversions fit | one conversion wins, or it's ambiguous |
| the best match is a deleted overload | compile error |
| a non-template and a template both match exactly | the non-template is preferred |
| defaults make several calls viable | possible ambiguity |

### The template mental model

```text
template source code
    |
    | used with int
    v
generated int function   (real, type-checked)

template source code
    |
    | used with double
    v
generated double function (real, type-checked)
```

Write the template once — but remember that every distinct type you use it with creates a *real* function that gets its own full type-checking. The template is a promise to generate; instantiation is when the promise is kept.

### How this connects forward

These features decode a huge amount of real C++. Overloads explain why one function name accepts different kinds of objects. Deleted overloads show how libraries forbid invalid conversions before they cause runtime bugs. Templates explain why `std::vector<T>`, `std::array<T, N>`, and generic helper functions look the way they do — and why their definitions live in headers. When a function call ever surprises you, split the puzzle into two questions, exactly the two this chapter was built around:

1. **Which** overload or template instance did the compiler choose?
2. **What** conversion or deduction made that choice possible?

Answer those two, and the behavior stops being a surprise.

### Practice: the Generic Toolkit lab

The chapter's lab puts every one of these ideas to work in a small mini-library you build in a single header. You'll write four overloaded `describe()` functions — for `int`, `double`, `bool`, and `std::string_view` — and the grader is designed to *catch* a missing `bool` overload, because without it `describe(true)` falls through to the `int` overload via promotion and returns the telltale wrong answer. You'll write function templates (`myMin<T>`, `myClamp<T>`), a non-type-parameter template (`repeatChar<N>`) whose compile-time `N` sizes a per-instantiation buffer, a two-type template with an `auto` return (`scaled<T, U>`), and a function with a default argument (`formatCount`). Every concept in this chapter shows up there as something you can *observe* — which is the whole point. Go make the resolution rules physical.
