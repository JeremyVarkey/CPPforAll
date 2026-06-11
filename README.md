# CPPforAll

**Free, structured C++ learning — read a lesson, write real code against it, get
graded red→green, all in the browser.** Live at **[cppforall.com](https://cppforall.com)**.

> LeetCode trains you to pass interviews. CPPforAll teaches you C++.

- **29 chapters** of lecture-grade course notes (fundamentals → exceptions → I/O)
- **28 hands-on labs** — a real project per chapter with starter code, an in-browser
  editor, and an automated grader (`Run` / `Submit`, ~1.5s round-trip)
- Free, no account, no ads. Calm, fast, readable.

## How it's built

| Piece | What |
|---|---|
| Site | [Astro](https://astro.build) static pages + one React island (the code runner). Hosted on Azure Static Web Apps. |
| Content | `content/` is a vendored snapshot of upstream notes/drills (`scripts/sync-content.sh`); `src/content-authored/` is the site's own authored rewrite (preferred by the build); `src/content-overlays/` adds validated study-aid callouts. `npm run content` generates `src/generated/` and validates everything. |
| Judge | `judge/` — a sandboxed clang service (Azure Container Apps, scale-to-zero). Grader manifests are generated from each lab's Makefile and replay-gated at image build (solution→green, starter→red). Solutions are stripped from the serving image; tests are evaluated server-side. |
| Design | `design/` — the committed design-system handoff the UI is built against. |

### Local development
```bash
npm install
npm run dev          # content pipeline runs automatically, then astro dev
```
The code runner needs a judge: either set `PUBLIC_JUDGE_URL` (copy `.env.example`
to `.env`) or run one locally:
```bash
DRILLS_DIR=$PWD/content/drills MANIFESTS_DIR=$PWD/judge/manifests node judge/server.mjs
```
Pages degrade gracefully when no judge is reachable (static code viewer).

### Deploying
Push to `main` → GitHub Actions builds and deploys the site. The judge deploys
separately: `az acr build --registry cppforallacr -f judge/Dockerfile .` then
`az containerapp update`.

## Contributing
Issues and fixes are welcome — especially typos, factual corrections in chapter
content, and bug reports with screenshots. Two boundaries to know:
- `content/` is a synced snapshot of an upstream source tree — don't edit it here;
  chapter text fixes belong in `src/content-authored/`.
- `src/generated/` is build output — never hand-edit.

Operating guide for AI-agent contributors: `CLAUDE.md`. Product direction:
`docs/roadmap.md`.

## License & provenance
Course content is original synthesis written for this site (topic sequence inspired
by the excellent [learncpp.com](https://www.learncpp.com), which we link to and
recommend). A formal open-source license for the code is pending — until then,
all rights reserved.
