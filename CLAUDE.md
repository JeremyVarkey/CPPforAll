# CPPforAll — Project Guide

> Read this first. It is the source of truth for what CPPforAll is, the decisions
> already made, and how to operate on this repo. Keep it current as decisions change.

## Mission
A **free, public, accessible** website for learning C++ from the ground up: a student
opens a chapter, reads fully-fleshed-out **notes**, and works the chapter's **coding
exercise right in the browser** — no IDE round-trip just to learn a concept. Built
first for Jeremy's own use (read/work the material outside VS Code), but intended to be
genuinely useful to anyone. Core values, in order: **free, structured, accessible.**

- **Repo:** https://github.com/JeremyVarkey/CPPforAll (public)
- **Domain:** registered via Squarespace (DNS to be pointed at the host at deploy time)
- **Local path:** `~/Desktop/CPPforAll`

## Scope — phased
**Phase 1 (current MVP): static site.**
- Render every chapter's **notes** as web pages (sidebar nav, search, dark mode).
- Show each chapter's **exercise** (handout + starter/solution/tests) as **copy / paste /
  download** code blocks. No execution yet.
- Goal: immediately useful for reading + grabbing the exercises; ships at ~$0.

**Phase 2 (later): embedded runtime.** Run the exercise in-browser (LeetCode-style),
grade red→green. Architecture deferred — **client-side WASM (Clang→WASM, ~$0, scales,
no server abuse surface)** vs **server-side judge (Judge0/Piston on Azure, full fidelity,
costs + security burden)**. Design Phase-1 UI as interactive "islands" so a Run/Submit
button can slot in **without a rewrite**.

## Tech decisions
- **Framework:** Astro + Starlight — Markdown/MDX content, per-chapter sidebar, built-in
  search, dark mode, and interactive islands for the future runtime.
- **Host:** Azure Static Web Apps (free tier). Deploy via **GitHub Actions** (push → live);
  the SWA deploy token lives in GitHub/Azure secrets — **never paste secrets into chat.**
- **Domain:** Squarespace-registered. At deploy, Jeremy adds the CNAME/TXT records in the
  Squarespace DNS panel (only he can edit it); SWA issues free managed TLS.
- **Node:** 25.x / npm 11.x (local).

## Azure subscription policy — **Option B: leave default as-is, isolate the project**
This machine's `az` CLI defaults to the **`Nootka_Labs`** subscription (a company/shared
sub, authenticated as a service principal). **Do NOT change that default** — other Nootka
work depends on it.

- **Never run `az account set`.** Leave `Nootka_Labs` as the machine default.
- Provision and manage **all** CPPforAll resources by passing the subscription
  **explicitly on every command**:
  `--subscription "Visual Studio Enterprise Subscription"`
  (id `5f6c172f-abad-4b16-a972-d8795dc3b27a`, same tenant `7db35776-1daa-4d76-b234-979b83e4ecab`).
- Why this sub: it's separate from company billing and carries a **monthly Azure credit**,
  so the static site runs at **$0 out of pocket**.
- ✅ Confirmed: the VS Enterprise subscription is approved for this project.

## Infrastructure (provisioned)
On the **Visual Studio Enterprise Subscription** (Option B — always pass `--subscription`):
- Resource group: `rg-cppforall` (region `eastus2`)
- Azure DNS zone: `cppforall.com` — nameservers:
  `ns1-09.azure-dns.com` · `ns2-09.azure-dns.net` · `ns3-09.azure-dns.org` · `ns4-09.azure-dns.info`
- ✅ **Delegation live:** cppforall.com nameservers now point to Azure DNS (verified via
  Google + Cloudflare resolvers and an authoritative check). All records managed via `az`.
- Not yet created: the Static Web App (waits for a buildable Astro site) and the
  apex/www records + TLS (added after delegation resolves).

## Content source & pipeline
**Single source of truth** (do not author content here directly — it lives upstream):
`~/Documents/Computer Science Courses/CS6340 - Software Analysis and Testing/cpp-llvm-foundations/`
- `notes/chapter-NN.md` — chapter notes (Codex authors these upstream).
- `drills/chapter-NN/` — exercises (Claude builds these upstream). 28 chapters + chapter-00.

Content is **vendored/synced** into this repo at **`content/`** (`content/notes/` +
`content/drills/`) by **`scripts/sync-content.sh`** — a faithful, build-artifact-free
snapshot. Re-run that script to pull upstream updates (it mirrors with `rsync --delete`).
Do NOT hand-edit `content/` — edit upstream and re-sync. Upstream ownership rules
(Codex = notes, Claude = drills) still apply in the source folder. The site build (Astro)
will consume `content/`; the three personal root files (learning-log, llvm-idioms,
learncpp-progress) are intentionally NOT vendored.

## The exercise model (so the site renders it correctly)
Each `drills/chapter-NN/` exercise has a fixed shape:
- `README.md` — 9-section lab handout (project, concepts, tasks, constraints, build,
  success criteria, hints, stretch goals).
- `Makefile` · `starter/` (learner fills `>>> YOUR CODE HERE <<<`) · `solution/` (reference)
  · `tests/` (auto-grader).
- **Red→green grader:** starter compiles but `make test` fails; solution makes it pass;
  `make test-solution` is always green. Phase 1 surfaces README + code blocks; Phase 2
  wires the grader to a Run/Submit button.

## Open flags / decisions pending
- **Licensing (before go-live):** public repo → notes must be Jeremy's **own synthesis**,
  not LearnCpp's copyrighted prose. Do an originality pass before publishing.
- **Phase-2 runtime architecture:** WASM vs server judge (see Scope). Deferred.
- **Test visibility:** consider hiding/obscuring `tests/` on the public site later so
  answers can't be hardcoded (matters mainly once Run/Submit exists).

## Conventions
- Push as Jeremy via the authenticated `gh` CLI (scopes `repo` + `workflow`). No separate
  collaborator to add.
- Branch off `main` for changes; commit/push only when asked.
- Keep this file updated when decisions change.

## Status
- [x] Repo cloned to `~/Desktop/CPPforAll`, this guide created.
- [x] Azure DNS zone `cppforall.com` created on the VS Enterprise sub; NS delegation live (Azure authoritative).
- [ ] Scaffold Astro + Starlight.
- [ ] Wire **chapter 1 end-to-end** as the template (notes page + exercise code boxes).
- [x] Sync script `scripts/sync-content.sh` → vendored 29 notes + 28 drills into `content/`.
- [ ] Render locally for review, then provision SWA + point the domain.
