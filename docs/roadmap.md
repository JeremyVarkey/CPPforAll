# CPPforAll — Product Roadmap

> Living document. The vision and the phased plan for taking cppforall.com from
> "a high-quality course" to **the one-stop destination for learning C++**.
> Engineering details live in [runner-plan.md](runner-plan.md) and the repo
> CLAUDE.md; this doc is product strategy.

## Positioning

> **LeetCode trains you to pass interviews. CPPforAll teaches you C++.**

LeetCode optimizes for *assessment* — it assumes you already know the language.
Nobody owns **pedagogy-first practice** for C++: learn a concept, immediately
write code against it, and get feedback that *teaches* rather than judges.
CPPforAll's core loop already exists and is the product's spine:

**read a lesson → open the lab → red → fix → green.**

Everything on this roadmap deepens that loop. Free stays non-negotiable.

## Where we are (June 2026)

| Asset | State |
|---|---|
| Curriculum | 29 chapters of lecture-grade authored content (Opus-authored, adversarially validated) |
| Practice | 28 labs with handouts, starter/solution/tests, in-browser editor |
| Run/Submit | Live: server judge (clang on Azure Container Apps), ~1.4s round-trip, red→green grading, hidden tests |
| Design | Claude-design system: calm, editorial, fast; Lighthouse 98/96/100/100 |
| Infra | $0-ish: SWA free tier + scale-capable judge; agent pipeline that can author + validate content at scale |

What's missing for a *destination*: practice volume, reasons to return,
feedback that teaches, and depth beyond the fundamentals.

## The four product pillars

### 1. Practice volume — from 28 labs to hundreds of drills
One lab per chapter is a course; a destination needs **per-lesson micro-drills**
(5-minute exercises after each of the ~334 lessons). Three tiers:

- **Predict** — read a snippet, choose its output (no compile; instant; great on mobile)
- **Fix** — broken code, make it green (debugging is half of real C++)
- **Write** — blank function + tests, classic red→green

Difficulty ratings, "random drill from chapter N," and per-lesson placement.
Our agent pipeline (author → adversarial validator → replay gate) is the unfair
advantage: drill authoring scales in a way human-authored competitors can't match.

### 2. Feedback that teaches — the killer feature
Today a failed submission returns a diff and an exit code. The leap:

- **AI tutor on failing submissions** — sees the learner's code, the test
  results, and the chapter context; responds *Socratically* (a hint, not the
  answer). Tiered: rate-limited free hints; "explain fully" unlocks after N
  honest attempts. This single feature beats every static tutorial on earth.
- **Runtime diagnosis C++ desperately needs** — compile failing submissions
  with sanitizers (ASan/UBSan) and translate the output into student language:
  *"your program crashed because it used memory after deleting it (line 12)."*
  For the pointer/memory chapters this IS the learning moment, and almost
  nobody does it well.

### 3. Progression & identity — reasons to return
Anonymous and stateless today. Minimum viable progression, **local-first**
(no accounts required):

- Chapter completion map (read ✓ / lab green ✓) — and once a learner starts,
  **the progress map becomes their homepage**: their journey, not our catalog
- Streaks, "resume where you left off," greens history
- Later: optional lightweight accounts for cross-device sync
- Later: free shareable completion certificate (every certificate on LinkedIn
  is marketing)

### 4. Depth — making "one-stop" true
The curriculum currently ends where the fundamentals end. The destination adds:

- **Advanced tracks:** STL mastery · concurrency · modern C++ (20/23: concepts,
  ranges, coroutines) · debugging & tooling · performance/profiling
- **Projects track:** multi-lab arcs — build a JSON parser, a mini shell, a ray
  tracer. The bridge from exercises to real software.
- **Interview track:** C++-specific interview problems (systems/embedded
  audience) — meet the LeetCode-adjacent demand inside the pedagogy-first house.

## The quiet growth engine
Every lesson is already a public, fast, SEO-perfect static page. Lean in:

- **"Common C++ errors" pages** — enormous, underserved search volume
  (`error: expected ';'`, "what is a segfault", linker errors)
- Glossary pages, cheat sheets, "X vs Y" comparison pages (references vs
  pointers, struct vs class)

Cheap to produce with the agent pipeline; compounds forever. Search brings
students in; the practice loop keeps them.

## Operational guardrails (pre-traffic, not post-incident)
The judge already rate-limits per IP and sandboxes execution. Before real
traffic: Application Insights dashboards (volume, latency, verdict mix,
top IPs), per-IP daily quotas, anomaly alerts, Azure budget alerts, and a
documented "under attack" dial (tighten limits → require Turnstile → pause).

## Phased plan

| Phase | What | Why first | Effort* |
|---|---|---|---|
| **R1** | Progress map + streaks (local-first), resume-where-you-left-off | Cheapest retention win; no backend; makes the site feel personal | S |
| **R2** | Micro-drills for ch 1–12 (predict/fix/write) | Proves the practice engine on the foundations everyone struggles with | M (agent fan-out) |
| **R3** | AI tutor on failures + sanitizer diagnosis | The moat. Nothing on the market does this well for C++ | M |
| **R4** | Judge ops: dashboards, quotas, alerts | Before traffic, not after | S |
| **R5** | Per-chapter quizzes | The source notes' quiz sections already exist — cut during authoring, waiting to be productized | S–M |
| **R6** | Drills for ch 13–28 + first advanced track | Content flywheel at full speed | M–L |
| **R7** | Projects track · interview track · certificates · accounts/sync | The full destination | L |

\* S = days, M = a week-ish, L = multi-week — solo founder + agent fleet pace.

**North-star metric:** weekly green submissions per learner — it measures the
whole loop (came back · practiced · succeeded), not vanity pageviews.

## Next phase — decision

> **Status: OPEN.** Candidate: R1 + R2 together (retention foundation + the
> practice engine proof). Decide scope here, then execute.

## Decision log
- **2026-06-10** — Roadmap created. Server-judge architecture (the "LeetCode
  model") confirmed as the execution backend going forward; in-browser WASM
  compilation shelved until toolchains get ~10× faster (see runner-plan.md).
- **2026-06-10** — Repo stays **public** (see CLAUDE.md note): fits the
  free-for-all mission, solutions are already user-visible by design, secrets
  live in GitHub/Azure not the repo, and openness compounds (trust, SEO-adjacent
  goodwill, future contributors).
