# CPPforAll judge — build context is the REPO ROOT:
#     docker build -f judge/Dockerfile -t cppforall-judge .
#
# Slim Debian + Node 22 base. We add clang/lld (the toolchain the drills were
# verified with) and util-linux (for `prlimit`, which enforces CPU/AS/NPROC/FSIZE
# rlimits on each child — see runner.mjs LIMITS). Runs as a non-root 'judge' user.

FROM node:22-bookworm-slim

# ── Toolchain + prlimit. clang/lld compile the drills; util-linux gives prlimit.
# ca-certificates so apt/clang behave; nothing else. Clean the apt cache to keep
# the image small. (No `make` is needed — the judge invokes clang directly via
# execFile; the Makefiles are reference-only.)
RUN apt-get update \
 && apt-get install -y --no-install-recommends \
      clang \
      lld \
      libc++-dev \
      libc++abi-dev \
      util-linux \
      ca-certificates \
 && rm -rf /var/lib/apt/lists/* \
 && clang --version \
 && prlimit --version

# ── Non-root user. The judge process and every child run as 'judge'. ──────────
RUN groupadd --system judge && useradd --system --gid judge --home-dir /app --shell /usr/sbin/nologin judge

WORKDIR /app

# ── Judge source. (No npm install: the judge has zero runtime dependencies —
# it uses only the Node 22 stdlib — so there is no package lockfile to restore.)
COPY judge/package.json ./package.json
COPY judge/server.mjs ./server.mjs
COPY judge/runner.mjs ./runner.mjs
COPY judge/test-manifests.mjs ./test-manifests.mjs

# ── Manifests → /app/manifests (MANIFESTS_DIR). ──────────────────────────────
COPY judge/manifests ./manifests

# ── Practice content → /app/practice (PRACTICE_DIR). The R2 micro-drills. Each
# chapter-NN.json carries every drill's `harness` (test driver owning main()) and
# `solution` (reference fix). These live SERVER-SIDE only: the server compiles
# student.cpp + the baked harness, and NEVER serializes harness/solution into any
# response (GET /practice/:chapter returns ids + tiers only). This is the same
# trust model as the drills' tests/ — present on the image, never sent to clients.
# The build context is the repo root, so the source path is src/practice-content.
COPY src/practice-content ./practice

# ── Drills content → /app/drills (DRILLS_DIR). The build context is the repo
# root, so the source path is content/drills. We copy the WHOLE tree (so the
# replay gate below can read each chapter's solution/), run the gate, and THEN
# strip every solution/ dir so the reference never ships in the serving image.
COPY content/drills ./drills

# ── Replay gate (build-time): every manifest must go red on the pristine starter
# and green on the solution; AND every fix/write practice drill must go red on its
# authored code and green on its solution. A bad manifest OR drill fails the IMAGE
# BUILD here, so a learner is never served a broken grader. Runs while solution/
# still exists; the practice gate reads harness+solution from /app/practice.
RUN DRILLS_DIR=/app/drills \
    DRILLS_WITH_SOLUTIONS_DIR=/app/drills \
    MANIFESTS_DIR=/app/manifests \
    PRACTICE_DIR=/app/practice \
    node test-manifests.mjs

# ── Strip the reference solutions from the serving image. After this, DRILLS_DIR
# contains only starter/, tests/, and root headers — runner.mjs ALSO filters
# solution/ at copy time, but removing it from disk is defense in depth so the
# reference text is simply not present in the running container.
RUN find /app/drills -type d -name solution -prune -exec rm -rf {} + \
 && ! find /app/drills -type d -name solution | grep -q . \
 && echo "solution/ dirs stripped from serving image"

# ── Ownership + drop privileges. /app is read-only at runtime via the platform;
# the judge writes only to per-request mkdtemp dirs under /tmp (a tmpfs in prod).
RUN chown -R judge:judge /app
USER judge

ENV NODE_ENV=production \
    PORT=8080 \
    DRILLS_DIR=/app/drills \
    MANIFESTS_DIR=/app/manifests \
    PRACTICE_DIR=/app/practice

EXPOSE 8080

# Liveness/readiness: the platform hits /healthz. Node's built-in fetch (22) keeps
# the image dependency-free; exit nonzero on a bad status so the platform restarts.
HEALTHCHECK --interval=30s --timeout=3s --start-period=5s --retries=3 \
  CMD node -e "fetch('http://127.0.0.1:'+(process.env.PORT||8080)+'/healthz').then(r=>process.exit(r.ok?0:1)).catch(()=>process.exit(1))"

CMD ["node", "server.mjs"]
