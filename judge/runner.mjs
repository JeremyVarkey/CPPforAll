// runner.mjs — executes ONE judge job in an isolated tmpdir.
//
// Responsibilities (per docs/runner-plan.md "Judge container"):
//   1. mkdtemp a fresh workdir under /tmp.
//   2. Copy the chapter's PRISTINE drill tree from DRILLS_DIR, EXCLUDING solution/
//      (the reference solution must NEVER be on disk for a learner job — only
//      starter/, tests/, and root headers are copied).
//   3. Overwrite the editable starter files with the learner-supplied text.
//   4. Run the manifest steps:
//        - mode "run":    compile the program, execute (with stdin if asked), return stdout.
//        - mode "submit": run every grader step, producing a verdict per step:
//            * exitCode    — pass iff the program exits 0.
//            * diffExpected — normalize trailing whitespace + final newline, then
//                             diff stdout against an expected file (Style A, ch01).
//            * grepDiff    — keep only stdout lines matching a pattern, compare to
//                             an expected list of lines (ch09 cin recovery).
//   5. ALWAYS remove the tmpdir in finally.
//
// Every child process (compiler and program) is spawned with child_process.execFile
// and an ARGS ARRAY — never a shell string — so learner input cannot inject shell
// metacharacters. Each child is wrapped with: a wall-clock kill (15s default),
// per-stream output caps (64KB; the child is killed if it exceeds a cap), and,
// when prlimit is available (Linux container), CPU/AS/NPROC/FSIZE rlimits via a
// `prlimit -- <cmd>` prefix. cwd is set to the job dir so the manifest's relative
// paths (tests/input.txt, tests/tmp_report.txt for ch28, ./tests/run) resolve.

import { execFile } from 'node:child_process';
import { mkdtemp, rm, cp, mkdir, writeFile, readFile, stat } from 'node:fs/promises';
import { existsSync } from 'node:fs';
import { tmpdir } from 'node:os';
import { join, dirname, sep, normalize, isAbsolute } from 'node:path';

// ── Limits ────────────────────────────────────────────────────────────────
// Wall-clock and output caps are enforced HERE, in Node, on every host (macOS dev
// included). The kernel-level rlimits below are enforced by `prlimit` when present;
// in the production container prlimit IS present (util-linux) so all of these apply.
// The Dockerfile documents that the container additionally provides: non-root user,
// tmpfs workdir, and a read-only /app. See README "Security model".
export const LIMITS = {
  wallMs: 15_000,          // hard wall-clock kill per child (plan: 15s)
  outCapBytes: 64 * 1024,  // 64KB per stream; child is killed if a stream exceeds it
  cpuSeconds: 10,          // prlimit --cpu   (plan: CPU rlimit 10s)
  asBytes: 536_870_912,    // prlimit --as    512MB address space
  nproc: 64,               // prlimit --nproc (compile spawns helpers; 64 is safe)
  fsizeBytes: 10_485_760,  // prlimit --fsize 10MB max file write (ch28 report stays small)
};

const DRILLS_DIR = process.env.DRILLS_DIR || '/app/drills';

// prlimit is Linux-only (util-linux). On the dev mac it is absent, so we skip it
// and rely on the Node-level wall-clock + output caps. In the container it exists
// and we wrap every child. Detect once at module load.
function detectPrlimit() {
  if (process.env.JUDGE_DISABLE_PRLIMIT === '1') return null;
  for (const p of ['/usr/bin/prlimit', '/bin/prlimit', '/usr/local/bin/prlimit']) {
    if (existsSync(p)) return p;
  }
  return null;
}
const PRLIMIT = detectPrlimit();

function prlimitWrap(cmd, args) {
  if (!PRLIMIT) return { cmd, args };
  const prefix = [
    `--cpu=${LIMITS.cpuSeconds}`,
    `--as=${LIMITS.asBytes}`,
    `--nproc=${LIMITS.nproc}`,
    `--fsize=${LIMITS.fsizeBytes}`,
    '--',
    cmd,
  ];
  return { cmd: PRLIMIT, args: [...prefix, ...args] };
}

// ── Safe path join: keep a manifest/learner-relative path inside the job dir ──
// Defense in depth: manifests are trusted (replay-gated), but learner-supplied
// editable keys are validated against the manifest BEFORE we get here. We still
// reject anything that escapes the job root.
function safeJoin(root, rel) {
  if (typeof rel !== 'string' || rel.length === 0) {
    throw new Error(`unsafe path: ${JSON.stringify(rel)}`);
  }
  if (isAbsolute(rel)) throw new Error(`absolute path not allowed: ${rel}`);
  const full = normalize(join(root, rel));
  const rootWithSep = root.endsWith(sep) ? root : root + sep;
  if (full !== root && !full.startsWith(rootWithSep)) {
    throw new Error(`path escapes job dir: ${rel}`);
  }
  return full;
}

// ── Run one child with wall-clock kill + output caps. Never uses a shell. ────
function runChild(cmd, args, { cwd, stdin, wallMs = LIMITS.wallMs }) {
  return new Promise((resolve) => {
    const { cmd: realCmd, args: realArgs } = prlimitWrap(cmd, args);
    const child = execFile(
      realCmd,
      realArgs,
      {
        cwd,
        timeout: wallMs,          // execFile sends killSignal after this
        killSignal: 'SIGKILL',
        maxBuffer: LIMITS.outCapBytes, // hard cap; ERR_CHILD_PROCESS_STDIO_MAXBUFFER on exceed
        windowsHide: true,
        env: {
          // Minimal, deterministic environment. No inherited secrets.
          PATH: process.env.PATH || '/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin',
          HOME: cwd,
          LANG: 'C.UTF-8',
          LC_ALL: 'C.UTF-8',
          TMPDIR: cwd,
        },
      },
      (err, stdout, stderr) => {
        let verdict = 'ok';
        let killed = false;
        let timedOut = false;
        let exitCode = null;
        let signal = null;

        if (err) {
          // execFile error shapes we care about:
          if (err.killed && err.signal) {
            killed = true;
            signal = err.signal;
            // execFile sets err.killed=true both for timeout and for maxBuffer kills.
            timedOut = err.code === undefined && err.signal === 'SIGKILL';
          }
          if (err.code === 'ERR_CHILD_PROCESS_STDIO_MAXBUFFER') {
            killed = true;
            verdict = 'output-cap';
          } else if (typeof err.code === 'number') {
            exitCode = err.code;
          } else if (err.signal) {
            killed = true;
            signal = err.signal;
          }
        } else {
          exitCode = 0;
        }

        resolve({
          exitCode,
          signal,
          killed,
          timedOut,
          verdict,
          stdout: capString(stdout),
          stderr: capString(stderr),
        });
      },
    );

    if (stdin != null) {
      child.stdin.on('error', () => {}); // ignore EPIPE if the child exits early
      child.stdin.end(stdin);
    } else {
      child.stdin.end();
    }
  });
}

function capString(s) {
  if (s == null) return '';
  const str = typeof s === 'string' ? s : s.toString('utf8');
  if (Buffer.byteLength(str, 'utf8') <= LIMITS.outCapBytes) return str;
  // Trim to the cap on a byte boundary, mark truncation.
  const buf = Buffer.from(str, 'utf8').subarray(0, LIMITS.outCapBytes);
  return buf.toString('utf8') + '\n…[output truncated at 64KB]';
}

// ── Output normalization for diffExpected (strip trailing ws per line + final NL)
function normalizeOutput(s) {
  return s
    .replace(/\r\n/g, '\n')
    .split('\n')
    .map((line) => line.replace(/[ \t]+$/g, ''))
    .join('\n')
    .replace(/\n+$/g, '');
}

// ── Tiny unified-diff (no deps) for diffExpected fail detail. ────────────────
function unifiedDiff(expected, actual, { maxLines = 60 } = {}) {
  const a = expected.split('\n');
  const b = actual.split('\n');
  const out = [];
  const n = Math.max(a.length, b.length);
  let shown = 0;
  for (let i = 0; i < n && shown < maxLines; i++) {
    const ea = a[i];
    const eb = b[i];
    if (ea === eb) {
      out.push('  ' + (ea ?? ''));
    } else {
      if (ea !== undefined) { out.push('- ' + ea); shown++; }
      if (eb !== undefined) { out.push('+ ' + eb); shown++; }
    }
  }
  if (n > maxLines) out.push(`… (${n - maxLines} more lines)`);
  return out.join('\n');
}

// ── Copy the pristine chapter tree into the job dir, EXCLUDING solution/. ─────
async function copyPristine(chapterDir, jobDir) {
  // fs.cp with a filter lets us skip the solution/ subtree entirely.
  await cp(chapterDir, jobDir, {
    recursive: true,
    dereference: false,
    filter: (src) => {
      // Reject any path component named exactly "solution".
      const rel = src.slice(chapterDir.length).split(sep).filter(Boolean);
      return !rel.includes('solution');
    },
  });
}

// ── Apply learner files (overwrite their starter counterparts). ──────────────
async function applyLearnerFiles(jobDir, files) {
  for (const [rel, content] of Object.entries(files || {})) {
    const dest = safeJoin(jobDir, rel);
    await mkdir(dirname(dest), { recursive: true });
    await writeFile(dest, String(content), 'utf8');
  }
}

// ── Compiler binary: configurable, defaults to clang++ (the toolchain the ──
// drills were verified with). The manifest schema carries flags per-step;
// the binary itself is fixed by the judge image, not learner-controlled.
const CXX = process.env.JUDGE_CXX || 'clang++';

// The judge chooses where each step's program is written, INSIDE the job dir.
// A unique name per step avoids collisions and never overwrites a drill file.
function binPathFor(jobDir, stepKey) {
  return join(jobDir, `__judge_${stepKey}`);
}

// ── Compile a step. Returns {res, ms, bin}. ──────────────────────────────────
// stepCompile schema (from the manifests agent): { sources[], flags[], include[] }.
// There is no `out` in the manifest — the runner picks the binary path. `flags`
// already contains the include dirs in practice, but we also concatenate
// `include` defensively (deduped) so either convention works.
async function compileStep(jobDir, stepCompile, stepKey) {
  const sources = Array.isArray(stepCompile?.sources) ? stepCompile.sources : [];
  const flags = Array.isArray(stepCompile?.flags) ? stepCompile.flags : [];
  const include = Array.isArray(stepCompile?.include) ? stepCompile.include : [];
  // Merge include flags that aren't already present in flags.
  const extraIncludes = include.filter((i) => !flags.includes(i));

  // Validate every source path stays inside the job dir.
  for (const s of sources) safeJoin(jobDir, s);

  const bin = binPathFor(jobDir, stepKey);
  const args = [...flags, ...extraIncludes, ...sources, '-o', bin];
  const t0 = Date.now();
  const res = await runChild(CXX, args, { cwd: jobDir });
  const ms = Date.now() - t0;
  return { res, ms, bin };
}

// ── Execute a compiled program for a step. ───────────────────────────────────
// runSpec schema: { stdin?: "<relative path to a file fed on stdin>" }.
// `bin` is the absolute path produced by compileStep.
async function runStep(jobDir, runSpec, bin) {
  let stdin = null;
  if (runSpec && typeof runSpec.stdin === 'string' && runSpec.stdin.length > 0) {
    const p = safeJoin(jobDir, runSpec.stdin);
    stdin = await readFile(p, 'utf8');
  } else if (runSpec && typeof runSpec.stdinText === 'string') {
    stdin = runSpec.stdinText;
  }
  const args = Array.isArray(runSpec?.args) ? runSpec.args : [];
  const t0 = Date.now();
  const res = await runChild(bin, args, { cwd: jobDir, stdin });
  const ms = Date.now() - t0;
  return { res, ms };
}

// ── Grade a single submit step → a result object for the response. ───────────
async function gradeStep(jobDir, step, stepIndex, compileMsAcc, runMsAcc) {
  const stepKey = `s${stepIndex}`;
  const result = {
    name: step.name || `step ${stepIndex + 1}`,
    phase: 'compile',
    exit: null,
    verdict: 'error',
    stdout: '',
    stderr: '',
  };

  // verdict may be an object {type, ...} (current schema) or a bare string.
  const verdict = typeof step.verdict === 'object' && step.verdict
    ? step.verdict
    : { type: step.verdict || 'exitCode' };
  const verdictType = verdict.type || 'exitCode';

  // 1) Compile (if the step has a compile spec).
  let bin = null;
  if (step.compile) {
    const c = await compileStep(jobDir, step.compile, stepKey);
    compileMsAcc.v += c.ms;
    bin = c.bin;
    result.stdout = c.res.stdout;
    result.stderr = c.res.stderr;
    result.exit = c.res.exitCode;
    if (c.res.killed || c.res.exitCode !== 0) {
      result.phase = 'compile';
      result.verdict = 'error';
      result.detail = c.res.timedOut
        ? 'compile timed out'
        : c.res.verdict === 'output-cap'
          ? 'compiler output exceeded 64KB'
          : 'compilation failed';
      return result;
    }
  }

  // 2) Run (if the step has a run spec — note `run: {}` is still a run step).
  let runRes = null;
  if (step.run !== undefined && step.run !== null) {
    result.phase = 'run';
    if (!bin) {
      result.verdict = 'error';
      result.detail = 'step declares run but no compiled binary';
      return result;
    }
    const r = await runStep(jobDir, step.run, bin);
    runMsAcc.v += r.ms;
    runRes = r.res;
    result.stdout = r.res.stdout;
    result.stderr = r.res.stderr;
    result.exit = r.res.exitCode;
    if (r.res.timedOut) {
      result.verdict = 'error';
      result.detail = 'program timed out (wall-clock limit)';
      return result;
    }
    if (r.res.verdict === 'output-cap') {
      result.verdict = 'error';
      result.detail = 'program output exceeded 64KB';
      return result;
    }
    if (r.res.killed) {
      result.verdict = 'error';
      result.detail = `program killed (signal ${r.res.signal || 'unknown'})`;
      return result;
    }
  }

  // 3) Grade according to verdict type.
  if (verdictType === 'exitCode') {
    result.phase = step.run !== undefined ? 'run' : 'compile';
    const code = runRes ? runRes.exitCode : result.exit;
    result.verdict = code === 0 ? 'pass' : 'fail';
    if (result.verdict === 'fail') {
      result.detail = `expected exit 0, got ${code}`;
    }
    return result;
  }

  result.phase = 'grade';

  if (verdictType === 'diffExpected') {
    const expectedRel = verdict.expected || verdict.expectedFile;
    if (!expectedRel) {
      result.verdict = 'error';
      result.detail = 'diffExpected verdict missing "expected" path';
      return result;
    }
    let expectedRaw;
    try {
      expectedRaw = await readFile(safeJoin(jobDir, expectedRel), 'utf8');
    } catch {
      result.verdict = 'error';
      result.detail = `expected file missing: ${expectedRel}`;
      return result;
    }
    const exp = normalizeOutput(expectedRaw);
    const got = normalizeOutput(runRes ? runRes.stdout : '');
    if (exp === got) {
      result.verdict = 'pass';
    } else {
      result.verdict = 'fail';
      result.detail = unifiedDiff(exp, got);
    }
    return result;
  }

  if (verdictType === 'grepDiff') {
    let pattern;
    try {
      pattern = new RegExp(verdict.pattern);
    } catch {
      result.verdict = 'error';
      result.detail = `invalid grepDiff pattern: ${verdict.pattern}`;
      return result;
    }
    const expectedLines = Array.isArray(verdict.expectedLines) ? verdict.expectedLines : [];
    const gotLines = (runRes ? runRes.stdout : '')
      .replace(/\r\n/g, '\n')
      .split('\n')
      .map((l) => l.replace(/[ \t]+$/g, ''))
      .filter((l) => pattern.test(l));
    const expJoined = expectedLines.join('\n');
    const gotJoined = gotLines.join('\n');
    if (expJoined === gotJoined) {
      result.verdict = 'pass';
    } else {
      result.verdict = 'fail';
      result.detail = unifiedDiff(expJoined, gotJoined);
    }
    return result;
  }

  result.verdict = 'error';
  result.detail = `unknown verdict type: ${verdictType}`;
  return result;
}

// ── mode "run": compile the run-target, execute, return stdout. ──────────────
// The manifest declares `runStep` ({compile, run}) describing how to build +
// execute the program for free-form running (distinct from grading). If absent,
// we fall back to the FIRST step that has a compile spec.
async function doRun(jobDir, manifest, timings) {
  const rt = manifest.runStep || firstRunnableStep(manifest);
  if (!rt || !rt.compile) {
    return {
      overall: 'error',
      steps: [{
        name: 'run', phase: 'compile', exit: null, verdict: 'error',
        stdout: '', stderr: '', detail: 'manifest has no runStep for mode "run"',
      }],
    };
  }
  const compileMsAcc = { v: 0 };
  const runMsAcc = { v: 0 };

  // Compile
  const { res: cres, ms: cms, bin } = await compileStep(jobDir, rt.compile, 'run');
  compileMsAcc.v += cms;
  if (cres.killed || cres.exitCode !== 0) {
    timings.compileMs = compileMsAcc.v;
    return {
      overall: 'error',
      steps: [{
        name: rt.name || 'run', phase: 'compile', exit: cres.exitCode,
        verdict: 'error', stdout: cres.stdout, stderr: cres.stderr,
        detail: cres.timedOut ? 'compile timed out' : 'compilation failed',
      }],
    };
  }

  // Run (with stdin if the manifest says so)
  const { res: rres, ms: rms } = await runStep(jobDir, rt.run || {}, bin);
  runMsAcc.v += rms;
  timings.compileMs = compileMsAcc.v;
  timings.runMs = runMsAcc.v;

  if (rres.timedOut || rres.verdict === 'output-cap' || rres.killed) {
    return {
      overall: 'error',
      steps: [{
        name: rt.name || 'run', phase: 'run', exit: rres.exitCode,
        verdict: 'error', stdout: rres.stdout, stderr: rres.stderr,
        detail: rres.timedOut ? 'program timed out (wall-clock limit)'
          : rres.verdict === 'output-cap' ? 'program output exceeded 64KB'
            : `program killed (signal ${rres.signal || 'unknown'})`,
      }],
    };
  }

  return {
    // For mode "run" we surface stdout regardless of exit code; "green" means it
    // ran to completion (exit 0), "red" means it exited nonzero (e.g. a failing
    // grader the learner is iterating on). The client shows stdout either way.
    overall: rres.exitCode === 0 ? 'green' : 'red',
    steps: [{
      name: rt.name || 'run', phase: 'run', exit: rres.exitCode,
      verdict: rres.exitCode === 0 ? 'pass' : 'fail',
      stdout: rres.stdout, stderr: rres.stderr,
    }],
  };
}

function firstRunnableStep(manifest) {
  return (manifest.steps || []).find((s) => s.compile) || null;
}

// ── mode "submit": run all grader steps, compute overall. ────────────────────
async function doSubmit(jobDir, manifest, timings) {
  const steps = manifest.steps || [];
  const out = [];
  const compileMsAcc = { v: 0 };
  const runMsAcc = { v: 0 };
  let sawError = false;
  let sawFail = false;

  for (let i = 0; i < steps.length; i++) {
    const r = await gradeStep(jobDir, steps[i], i, compileMsAcc, runMsAcc);
    out.push(r);
    if (r.verdict === 'error') sawError = true;
    if (r.verdict === 'fail') sawFail = true;
  }
  timings.compileMs = compileMsAcc.v;
  timings.runMs = runMsAcc.v;

  let overall;
  if (sawError) overall = 'error';
  else if (sawFail) overall = 'red';
  else overall = 'green';
  return { overall, steps: out };
}

// ── Public entry point. ──────────────────────────────────────────────────────
// job: { chapter:number, mode:"run"|"submit", files:{path:text}, manifest:object,
//        queueMs?:number }
export async function runJob(job) {
  const { chapter, mode, files, manifest } = job;
  const timings = { queueMs: job.queueMs || 0, compileMs: 0, runMs: 0 };

  const chapterDir = join(DRILLS_DIR, `chapter-${String(chapter).padStart(2, '0')}`);
  // Confirm the pristine chapter exists on disk.
  try {
    const st = await stat(chapterDir);
    if (!st.isDirectory()) throw new Error('not a dir');
  } catch {
    return {
      chapter,
      overall: 'error',
      steps: [{
        name: 'setup', phase: 'compile', exit: null, verdict: 'error',
        stdout: '', stderr: '', detail: `chapter ${chapter} drill not found on judge`,
      }],
      timings,
    };
  }

  let jobDir;
  try {
    jobDir = await mkdtemp(join(tmpdir(), `cppjudge-ch${chapter}-`));
    await copyPristine(chapterDir, jobDir);
    await applyLearnerFiles(jobDir, files);

    const result = mode === 'run'
      ? await doRun(jobDir, manifest, timings)
      : await doSubmit(jobDir, manifest, timings);

    return { chapter, overall: result.overall, steps: result.steps, timings };
  } catch (err) {
    return {
      chapter,
      overall: 'error',
      steps: [{
        name: 'judge', phase: 'compile', exit: null, verdict: 'error',
        stdout: '', stderr: '', detail: `judge error: ${err.message}`,
      }],
      timings,
    };
  } finally {
    if (jobDir) {
      await rm(jobDir, { recursive: true, force: true }).catch(() => {});
    }
  }
}

export const __test = { normalizeOutput, unifiedDiff, safeJoin, capString, PRLIMIT };
