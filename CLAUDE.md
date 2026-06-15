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

## Scope — current state (June 2026)
**SHIPPED and live at https://cppforall.com:** 29 authored chapters (lecture-grade,
validated) · 28 labs with in-browser editor + Run/Submit against the server judge
(red→green grading, ~1.4s warm) · instant chapter navigation · scrollspy TOC ·
solution panes gated behind "Reveal solution" ("try first" policy) · **R1**
local-first progress (src/lib/progress.ts schema v1; rail indicators, streaks,
welcome-back resume — no accounts) · **R2** practice micro-drills ch 1–12
(61 drills, predict/fix/write, /chapters/NN/practice/, judge practice mode —
harness+solution server-side only; spec: docs/r1-r2-spec.md). NOTE: islands
inside the app shell must use client:idle or client:only — client:visible never
fires (astro-island is display:contents → zero-size box).

**Next:** see `docs/roadmap.md` (positioning, four pillars, R1–R7 phases; next-phase
decision currently OPEN). Engineering record of the runner architecture decision:
`docs/runner-plan.md`.

## Tech decisions
- **Framework:** plain Astro + React islands (**Starlight dropped** — the bespoke Claude
  Design shell outgrew docs chrome). Pages are static HTML (`/chapters/NN/` notes,
  `/chapters/NN/exercise/` labs); only rail search + theme hydrate. The design source of
  truth is the committed handoff bundle in `design/` — match it pixel-faithfully; its
  chapter data is placeholder, all UI binds to `src/generated/chapters.json`.
- **Content pipeline:** `npm run content` (auto pre-dev/pre-build) → `scripts/build-content.mjs`
  (index → notes → drills → validate). Markdown engine: `scripts/lib/markdown.mjs` (unified +
  Shiki custom warm-dark theme; `**Label:**` blockquotes → tip/warn callouts; lesson anchors
  preserved from source TOC slugs). Generated output in `src/generated/` (gitignored).
  Agent enrichment lives in `src/content-overlays/chapter-NN.json` ("marked study aids"
  policy: lead + aids + xrefs only, validated, visually distinct on page).
- **Authored content tier:** `src/content-authored/chapter-NN.md` — the site's own
  lecture-grade text (Opus-authored, Opus-validated), preferred by `build-notes.mjs` over
  the vendored notes. Heading contract: H1 + `## N.M — Title` lessons must match the source
  (anchors stay stable); bodies are authored. The vendored `content/notes/` remain the
  factual reference and Codex's tree — never hand-edit either tier casually.
- **Runner (LIVE):** server judge, NOT in-browser wasm. B0 spike measured wasmer clang/clang
  at ~25KB/s effective compile throughput (libc++ → 200s+; flattened-header experiment also
  failed) — see docs/runner-plan.md for the full verdict + revisit conditions. Architecture:
  CodeMirror 6 island (`src/islands/ExerciseRunner.tsx`, graceful static fallback) → POST to
  the judge (Azure Container Apps `cppforall-judge` in rg-cppforall, image in ACR
  `cppforallacr`, scale 0–3): clang per grader manifest (`judge/manifests/chapter-NN.json`,
  generated from the Makefiles by `scripts/build-grader-manifests.mjs`), solution/ never on
  the serving image, tests server-side (hidden-tests property), rlimits + wall-kill + rate
  limiting. Judge URL via `PUBLIC_JUDGE_URL` (.env + deploy.yml). Round-trip ≈ 0.9–2.5s.
  Rebuild+deploy judge: `az acr build --registry cppforallacr -f judge/Dockerfile .` then
  `az containerapp update` (always `--subscription "Visual Studio Enterprise Subscription"`).
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
- Static Web App: `cppforall` in `rg-cppforall` (Free tier), default hostname
  `ambitious-grass-0d1b3c60f.7.azurestaticapps.net`. Deploys via
  `.github/workflows/deploy.yml` on push to main (token in repo secret
  `AZURE_STATIC_WEB_APPS_API_TOKEN`; rotate via `az staticwebapp secrets`).
- **LIVE: https://cppforall.com** (launched 2026-06-10). Apex = Azure DNS alias A record →
  the SWA resource; TXT `_dnsauth` validation; DigiCert managed TLS. `www` = CNAME to the
  default hostname — **both Ready with valid DigiCert certs** (www's validation got stuck
  for days; fix was delete + re-add the SWA hostname, 2026-06-11). Note: SWA cannot do
  host-based redirects, so www serves the same content rather than 301ing — canonical tags
  point at the apex, which handles SEO.
- **Analytics:** Azure Application Insights `cppforall-insights` (rg-cppforall). Web SDK
  loads async from the MS CDN via `src/components/Analytics.astro` (conn string is a public
  ingestion key — `PUBLIC_APPINSIGHTS_CONNECTION_STRING` in deploy.yml + .env.example).
  Tracks pageviews + the `lab_green` custom event (north-star). View: Portal →
  cppforall-insights → Usage / Logs (`pageViews | summarize count() by bin(timestamp,1d)`). Licensing gate passed pre-launch (sampled originality
  check: 10 distinctive LearnCpp phrases × 29 notes files, zero hits).
- Lighthouse (chapter page): performance 98 · a11y 96 · best-practices 100 · SEO 100.
  Pages ship ZERO framework JS (rail search is vanilla; React arrives only with the
  Phase B runner island).

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
- **Next roadmap phase:** OPEN — see `docs/roadmap.md` (candidate: R1 progress map +
  R2 micro-drills).
- **Judge ops (R4):** rate limiting is in-memory per replica (resets on cold start,
  multiplies with replicas — currently bounded by maxReplicas). Before real traffic:
  App Insights dashboards, durable per-IP daily quotas, budget alerts, "under attack"
  dial. Tracked in the roadmap.
- **www.cppforall.com:** registered + CNAME correct; Azure cert issuance was still
  pending as of 2026-06-10 — re-check `az staticwebapp hostname list`.

## Resolved decisions (for the record)
- **Licensing:** originality pass done pre-launch (sampled LearnCpp phrases × all 29
  notes, zero hits). Site content is original synthesis + authored rewrite.
- **Runtime architecture:** server judge won (B0 spike falsified in-browser WASM clang
  — ~25KB/s; see docs/runner-plan.md). Revisit only if wasm toolchains get ~10× faster.
- **Test/solution visibility:** repo is public by design; grading integrity lives in
  the judge (server-side tests, solutions stripped from the serving image). On-site,
  solutions sit behind a "Reveal solution" gate; tests stay readable (pedagogy).
- **Repo visibility:** public, deliberately (mission fit; secrets live in GitHub/Azure;
  openness compounds). User-data services (future accounts) stay in Azure regardless.

## Conventions
- Push as Jeremy via the authenticated `gh` CLI (scopes `repo` + `workflow`). No separate
  collaborator to add.
- Branch off `main` for changes; commit/push only when asked.
- Keep this file updated when decisions change. README.md is the public-facing intro;
  this file is the operating guide for agents.
