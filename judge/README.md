# CPPforAll Judge

The code-execution judge for [CPPforAll](https://cppforall.com): a small, sandboxed
HTTP service that compiles and grades a learner's C++ drill submission against the
chapter's hidden tests, returning a red/green verdict. It is the **judge-server**
component of the Phase-B runner described in `docs/runner-plan.md`.

The client (the in-page `ExerciseRunner` island) sends only the learner's **editable
starter files**; the tests, expected outputs, and reference solutions live server-side
and never leave the container. Learner code never touches the website's own origin.

## What's here

| File | Role |
|------|------|
| `server.mjs` | HTTP front: `POST /run`, `GET /healthz`, `GET /manifest/:chapter`; CORS, rate limit, concurrency queue. Node 22 stdlib `http` only — no dependencies. |
| `runner.mjs` | Executes one job: mkdtemp, copy pristine drill (no `solution/`), overlay learner files, compile + run + grade per the manifest, clean up. |
| `manifests/chapter-NN.json` | Per-chapter grader recipe (authored by the manifests pipeline). |
| `test-manifests.mjs` | Replay gate: every manifest must go **red** on the pristine starter and **green** on the solution. Run at image-build time. |
| `Dockerfile` | Slim Debian + Node 22 + clang/lld + `prlimit`; non-root `judge` user. |
| `.dockerignore` | Trims the build context (see note below). |

## API

`POST /run` — body ≤ 256 KB JSON:

```json
{ "chapter": 9, "mode": "submit", "files": { "starter/desk.cpp": "<full text>" } }
```

- `chapter`: integer 1–28.
- `mode`: `"run"` (compile + execute, stdout returned) or `"submit"` (full grader).
- `files`: object whose keys MUST be a subset of the chapter manifest's `editable`
  list — anything else is `400`.

Response `200`:

```json
{ "chapter": 9, "overall": "red|green|error",
  "steps": [ { "name": "unit tests", "phase": "compile|run|grade",
               "exit": 0, "verdict": "pass|fail|error",
               "stdout": "≤64KB", "stderr": "≤64KB", "detail": "optional" } ],
  "timings": { "queueMs": 0, "compileMs": 0, "runMs": 0 } }
```

Errors: `400` invalid · `413` body too large · `429` rate-limited · `503` at capacity.
All errors are JSON: `{ "error": "<code>", "message": "<human text>" }`.

`GET /healthz` → `{ "status": "ok", "active": N, "queued": M }` (probe target).

`GET /manifest/:chapter` → the **public** subset only:
`{ "chapter", "project", "editable": [...] }`. Tests, expected outputs, grep
patterns, and compile sources are never exposed (anti-hardcoding).

## Build

The build context is the **repo root** so the Dockerfile can bake in `content/drills`:

```bash
# from the repo root
docker build -f judge/Dockerfile -t cppforall-judge .
```

The build runs the **replay gate** (`test-manifests.mjs`) against every manifest while
`solution/` is still present; a manifest that doesn't go red→green **fails the image
build**, so a broken grader can never reach a learner. After the gate, every chapter's
`solution/` directory is removed from the image.

### `.dockerignore` placement

Docker reads the ignore file from the **build-context root** (here, the repo root) or,
with BuildKit, from a Dockerfile-adjacent `judge/Dockerfile.dockerignore`. This repo
keeps the canonical list at `judge/.dockerignore`; to make it take effect with the
root-context build, either copy it to `<repo-root>/.dockerignore` or rename a copy to
`judge/Dockerfile.dockerignore`. (The patterns are written relative to the repo root,
so they work in either location.) It is an optimization only — the Dockerfile is
correct without it, just with a larger build context.

## Run locally

```bash
# Native (needs clang on PATH; prlimit is Linux-only and is skipped on macOS):
cd judge
DRILLS_DIR=../content/drills MANIFESTS_DIR=./manifests PORT=8080 node server.mjs

# Or the built container:
docker run --rm -p 8080:8080 cppforall-judge
curl localhost:8080/healthz
```

Replay gate on demand (any machine with clang):

```bash
cd judge
DRILLS_DIR=../content/drills \
DRILLS_WITH_SOLUTIONS_DIR=../content/drills \
MANIFESTS_DIR=./manifests \
node test-manifests.mjs           # add chapter numbers to test a subset, e.g. `... 1 9 28`
```

## Environment variables

| Var | Default | Meaning |
|-----|---------|---------|
| `PORT` | `8080` | HTTP listen port. |
| `DRILLS_DIR` | `/app/drills` | Pristine drill tree the judge serves (no `solution/`). |
| `MANIFESTS_DIR` | `/app/manifests` | Grader manifests. |
| `ALLOWED_ORIGINS` | `https://cppforall.com,http://localhost:4321` | Comma-separated CORS allow-list. |
| `DRILLS_WITH_SOLUTIONS_DIR` | (falls back to `DRILLS_DIR`) | **Replay gate only** — a tree that still has `solution/`, for the green-case run. |
| `JUDGE_CXX` | `clang++` | Compiler binary (image-fixed; not learner-controllable). |
| `JUDGE_DISABLE_PRLIMIT` | unset | Set to `1` to skip the `prlimit` wrapper (dev only). |

## Security model

The brief is robust / fast / secure. How this judge gets there:

- **No learner code on our origin.** The judge is a separate host; the site only POSTs
  the editable starter files. Tests, expected outputs, and solutions stay server-side.
- **Strict input validation.** `chapter ∈ 1..28`, `mode ∈ {run, submit}`, body ≤ 256 KB
  (`413`), `files` keys must be a subset of the manifest's `editable` list (`400`).
  Bad JSON → `400`. Paths are validated to stay inside the per-request job dir.
- **No shell, ever.** Compiler and program are spawned with `execFile` and **argument
  arrays** — learner text can never be interpreted as a shell command.
- **Per-request isolation.** Each job gets a fresh `mkdtemp` workdir; the pristine
  chapter is copied in with `solution/` filtered out (and it isn't on disk at all in the
  serving image); the learner's editable files overwrite their starter counterparts;
  the dir is removed in a `finally`.
- **Resource limits.** Enforced in Node on every host: 15 s wall-clock kill per child,
  64 KB cap per stdout/stderr stream (child killed on exceed). Enforced by `prlimit` in
  the Linux container (util-linux): CPU 10 s, address space 512 MB, NPROC 64, FSIZE
  10 MB. The container additionally runs **non-root** (`judge` user) and writes only to
  `/tmp`.
- **Concurrency + rate limiting.** In-process queue: ≤ 4 concurrent jobs, queue cap 16
  → `503`. Per-IP token bucket: 10 req/min sustained, burst 20 → `429`. The client IP is
  taken from the **first hop** of `x-forwarded-for` only (set by our ingress); the rest
  of the chain is untrusted.
- **CORS allow-list, no cookies, no auth, no state.** Only the configured origins get an
  `Access-Control-Allow-Origin`; credentials are never allowed.
- **Replay-gated image.** The build fails if any manifest doesn't go red on the starter
  and green on the solution — a bad grader can't ship.

## Verified (on the build machine)

- `node --check` clean on `server.mjs`, `runner.mjs`, `test-manifests.mjs`.
- Replay gate: **all 28 chapters** red→green, both natively (macOS clang) and inside the
  built Linux container (Debian clang 14, `prlimit` active).
- HTTP: `/healthz`, `/manifest/:chapter` (public subset only), CORS allow/deny, `POST
  /run` happy path (green), and the error matrix (`400` bad chapter / mode / non-editable
  file / bad JSON, `413` oversized, `429` after burst of 20).
- An infinite-loop submission is killed (~10 s, CPU rlimit) and returns a clean `error`,
  not a hang.

## Notes for the orchestrator

- Deploy is handled outside this component (ACR build + ACA app, per the plan). Build
  with the repo root as context: `docker build -f judge/Dockerfile .`.
- The client needs the judge base URL via `PUBLIC_JUDGE_URL` (build-time) and the
  judge's `ALLOWED_ORIGINS` must include the preview/prod site origins.
- Manifests are authored by a separate pipeline; this server codes to their committed
  schema (`{ chapter, project, editable[], steps[], runStep }`, with per-step
  `compile{sources,flags,include}`, `run{stdin?}`, and `verdict{type,...}`).
