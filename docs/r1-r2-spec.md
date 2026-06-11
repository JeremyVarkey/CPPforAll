# R1 + R2 — Build Spec (binding)

> The contract for the R1 (progress + streaks) and R2 (micro-drills, ch 1–12) build.
> Agents build EXACTLY to this. File ownership is exclusive; touching files outside
> your grant is a defect. The orchestrator owns all shared seams and integrates last.

## Naming
The existing per-chapter labs are called **drills** in this repo (`content/drills/`).
To avoid collision, R2's micro-exercises are **practice** everywhere: routes
(`/chapters/NN/practice/`), content (`src/practice-content/`), scripts
(`build-practice.mjs`), judge mode (`practice`).

---

## R1 — Progress, streaks, resume (local-first, no accounts)

### localStorage schema (versioned, shared contract)
Key `cppforall:progress:v1`:
```json
{
  "chapters": {
    "04": { "read": 1760000000000, "lab": 1760000000000, "practice": { "c04-d1": 1760000000000 } }
  },
  "activity": { "2026-06-11": 1 },
  "lastVisited": { "path": "/chapters/04/", "title": "Fundamental Data Types", "ts": 1760000000000 }
}
```
- `read` — set when the learner reaches ≥90% scroll on a chapter's notes page
  (or the last lesson section enters the viewport). Timestamp = first time.
- `lab` — set when the runner island receives a `submit` verdict of `green`.
- `practice.<drillId>` — set when a practice drill is completed (predict: correct
  answer chosen; fix/write: green).
- `activity["YYYY-MM-DD"]` — set on ANY of the above (local timezone). Prune > 400 days.
- All writes go through one module: `src/lib/progress.ts` (R1 agent owns it).
  API (exact names): `getProgress()`, `markRead(slug, title)`, `markLabGreen(slug)`,
  `markPractice(slug, drillId)`, `touchVisit(slug, title, path)`, `getStreak()`
  (consecutive days ending today or yesterday), `chapterState(slug)` →
  `{ read, lab, practiceDone, practiceTotal? }`. Module must be importable from
  islands AND usable from a tiny inline script (expose `window.cppProgress` via a
  thin Astro component the orchestrator can mount).

### UX requirements (calm — no gamification slop)
1. **Rail indicators:** right-aligned in each chapter row: lab green → small
   `--success` check; read-but-no-green → small hollow dot (`--muted`). Nothing
   for untouched. Must work with the persisted rail across soft navs
   (update on `astro:page-load`).
2. **Streak:** in the rail footer, replacing "29 chapters · free & open" when
   streak ≥ 2: `⟡ N-day streak` (spark icon, `--accent`, sans 11.5px). Below 2,
   keep the current footer text.
3. **Landing becomes the journey:** if `lastVisited` exists, the landing hero adds
   a "Welcome back" block ABOVE the start-chips: resume CTA (`btn-primary`,
   "Continue Chapter N — Title") + a compact 29-cell progress strip (tiny squares:
   green = lab done, outline = read, empty = untouched; sans labels on hover via
   title attr). First-time visitors see the page exactly as today.
4. **Read tracking** on notes pages; **green hook** in the runner island
   (dispatch + record on submit green). No layout shift, no popups, no toasts.

---

## R2 — Practice (micro-drills), chapters 1–12

### Content schema — `src/practice-content/chapter-NN.json` (one file per chapter)
```json
{
  "chapter": 4,
  "drills": [
    {
      "id": "c04-d1",
      "lesson": "44--signed-integers",
      "tier": "predict",
      "title": "What does this print?",
      "promptMd": "Read carefully — then commit to an answer before running anything.",
      "code": "<full C++ snippet shown to the learner>",
      "choices": ["4", "3", "undefined behavior", "compile error"],
      "answer": 2,
      "explanationMd": "Why: ..."
    },
    {
      "id": "c04-d2",
      "lesson": "412--introduction-to-type-conversion-and-static_cast",
      "tier": "fix",
      "title": "Make the narrowing explicit",
      "promptMd": "This fails to express intent — fix the conversion so the tests pass.",
      "code": "<the BROKEN student file the editor opens with>",
      "harness": "<test driver: declares the required function(s), owns main(), CHECK macros, exit code 0 = pass>",
      "solution": "<reference fix — never sent to clients>",
      "explanationMd": "Why: ..."
    }
  ]
}
```
Rules:
- `id` format `cNN-dM`, stable forever. `lesson` MUST be a real anchor from
  `src/generated/notes/chapter-NN.json` (or `""` for chapter-general).
- 4–6 drills per chapter: ≥1 predict, ≥1 fix, ≥1 write (write = same shape as fix,
  `code` is a starter with a TODO block instead of broken code).
- **Function-scoped execution model:** for fix/write, the student file contains
  function definition(s) only — NO `main()`. The harness owns `main()` and declares
  the functions. Compile = `clang++ -std=c++17 -Wall -Wextra student.cpp harness.cpp`,
  run, exit code 0 = green. Predict-tier `code` may be a full program (never compiled
  client-side).
- **Template / overload-visibility exception (amended after ch11):** when the drill's
  point requires the harness's call site to SEE the student's definitions (function
  templates, or overload-resolution bugs like missing-overload fall-through), the
  harness uses `#include "student.cpp"` instead of forward declarations. In that
  model every non-template function in code/solution MUST be `inline` (the judge
  still compiles student.cpp standalone as a second TU; inline keeps the ODR happy).
  Default remains forward-declaration; use the include model only when semantics
  demand it.
- Concepts must not exceed the drill's chapter (course sequencing — same scope map
  as the labs, `content/drills/CLAUDE.md`).
- Every claimed predict answer must be the snippet's REAL behavior (compile + run it).
  Each distractor must be genuinely wrong. If the answer is "undefined behavior",
  the explanation must name the UB rule.

### Judge extension (practice mode)
- Bake `src/practice-content/` into the image at `/app/practice` (PRACTICE_DIR env).
- `POST /run` accepts EITHER the existing lab body OR
  `{ "practice": { "chapter": 4, "drill": "c04-d2" }, "files": { "student.cpp": "..." } }`.
  Response shape identical to labs (`steps[]`, `overall`, `timings`).
- Server compiles student.cpp + the drill's harness (from the baked JSON — `harness`
  and `solution` are NEVER in any response; strip them from `/manifest`-style
  endpoints). Same rlimits/queue/rate limits as labs.
- `GET /practice/:chapter` → public subset only: drill ids + tiers (the client page
  already has prompts/code statically; it only needs the endpoint to confirm
  availability).
- **Replay gate extended:** image build fails unless, for every fix/write drill:
  authored `code` (broken/starter) → red AND `solution` → green, natively.

### Practice page — `/chapters/NN/practice/` (ch 1–12 only at first)
- Same AppShell; `view` passed as `"practice"` (the orchestrator wires the
  3-segment toggle — do NOT edit AppShell/ViewToggle).
- Drill cards grouped by lesson (lesson title as group header, linked to the notes
  anchor). Card: tier tag (predict/fix/write — kindtag visual idiom), title, prompt,
  then:
  - **predict:** code in the standard dark codebox; radio choices; "Check" button;
    on answer → correct/incorrect state + explanation callout (aid-style); record
    via `window.cppProgress.markPractice()` only when correct.
  - **fix/write:** CodeMirror editor (same theme as the runner island) seeded with
    `code`, Run button → judge practice mode → per-step result + console; on green:
    success state + explanation + `markPractice()`.
- One island component `src/islands/PracticeDrill.tsx` handles a single drill
  (props: chapter, drill data minus harness/solution); the page maps drills to
  islands (`client:visible` — lazy, many per page). Editor loads lazily.
- Page must render fine with the judge unreachable (predict drills fully work;
  fix/write show editor + a quiet "runner offline" note on Run).
- Drill completion states visually quiet: a small `--success` check in the card
  header once done (read from progress module on mount).

### Page data hygiene
`harness` and `solution` must be stripped from anything that reaches the client
(page props, island props, serialized JSON). Build step enforces: grep dist for a
sentinel from any harness must find zero hits.

---

## File ownership (exclusive grants)
| Owner | Files |
|---|---|
| **Agent A (Opus) — R1** | `src/lib/progress.ts` (new) · `src/components/ProgressMap.astro` (new) · `src/components/ProgressBridge.astro` (new, exposes window.cppProgress + read-tracking) · EDIT `src/components/Rail.astro` (indicators) · EDIT `src/pages/index.astro` (welcome-back) · EDIT `src/pages/chapters/[slug]/index.astro` (mount read-tracking) · EDIT `src/islands/ExerciseRunner.tsx` (green hook only) · styles ONLY inside its own components' scoped `<style>` |
| **Agent B (Opus) — R2 infra** | `scripts/build-practice.mjs` (new) · `src/pages/chapters/[slug]/practice/index.astro` (new) · `src/islands/PracticeDrill.tsx` (new) · EDIT `judge/server.mjs`, `judge/runner.mjs`, `judge/Dockerfile`, `judge/test-manifests.mjs` (practice mode) · judge README · styles ONLY scoped/in-island |
| **Agents C01–C12 (Sonnet) — content** | exactly one `src/practice-content/chapter-NN.json` each |
| **Validators (Opus)** | read-only + verdicts |
| **Orchestrator** | this spec · `scripts/build-content.mjs` wiring · AppShell/ViewToggle 3-segment toggle · `src/styles/cpp-styles.css` · CLAUDE.md/roadmap · judge image rebuild + ACA deploy · final verification |

**Hard rules:** never `git commit`/`push` (orchestrator only). Never edit
`cpp-styles.css`, `AppShell.astro`, `build-content.mjs`, or another agent's grant.
A and B run concurrently — zero file overlap exists by construction; respect it.

## Quality gates (orchestrator-verified, not self-reported)
1. `npm run build` green; predict/fix/write all functional in preview.
2. Native replay: every fix/write drill red→green; every predict answer
   machine-verified (compile + run + compare).
3. No harness/solution bytes in `dist/` or any HTTP response.
4. Progress: read/green/practice/streak/resume all observable in preview;
   zero console errors; soft-nav safe (`astro:page-load`).
5. Lighthouse on a practice page ≥ 90 across the board.
