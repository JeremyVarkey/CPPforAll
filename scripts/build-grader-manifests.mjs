#!/usr/bin/env node
// build-grader-manifests.mjs
// ---------------------------------------------------------------------------
// Generates judge/manifests/chapter-NN.json for all 28 drills by PARSING each
// chapter's Makefile `test:` / `test-solution:` recipe. The judge replays these
// manifests (solution -> green, starter -> red) in the same toolchain that
// serves learners; see docs/runner-plan.md "Grader manifests".
//
// FAIL-LOUD policy: if a Makefile does not match a known grader pattern, the
// generator exits non-zero with a clear message — no silent guessing.
//
// Schema written per chapter (consumed by the judge runner):
//   {
//     "chapter": N,
//     "project": "<README H1 project name>",
//     "editable": ["starter/..."],            // the ONLY client-suppliable paths
//     "steps": [ { name, compile:{sources,flags,include}, run:{stdin?}, verdict } ],
//     "runStep": { ... }                       // single compile+run for mode:"run"
//   }
// verdict.type is one of: "exitCode" | "diffExpected" | "grepDiff".
// A step that needs a writable copy of tests/ carries "workdir": true (ch28).
// ---------------------------------------------------------------------------

import { readFileSync, readdirSync, writeFileSync, mkdirSync, existsSync, statSync } from 'node:fs';
import { join, dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';

const __dirname = dirname(fileURLToPath(import.meta.url));
const REPO = resolve(__dirname, '..');
const DRILLS = join(REPO, 'content', 'drills');
const OUT_DIR = join(REPO, 'judge', 'manifests');

const COMMON_FLAGS = ['-std=c++17', '-Wall', '-Wextra'];

function die(msg) {
  console.error(`\n✗ build-grader-manifests: ${msg}\n`);
  process.exit(1);
}

// Read a Makefile and return { recipes, prereqs } where:
//   recipes[target] = array of recipe lines (tab-stripped)
//   prereqs[target] = array of prerequisite tokens from the target line
function parseMakefile(text) {
  const lines = text.split('\n');
  const recipes = {};
  const prereqs = {};
  let current = null;
  for (const raw of lines) {
    if (raw.startsWith('\t')) {
      if (current) recipes[current].push(raw.replace(/^\t/, ''));
      continue;
    }
    // A target line: `name: deps` (not a variable assignment `X := ...`).
    const m = raw.match(/^([A-Za-z0-9_./-]+)\s*:(?!=)\s*(.*)$/);
    if (m && !raw.includes(':=')) {
      current = m[1];
      if (!recipes[current]) recipes[current] = [];
      prereqs[current] = m[2].trim() ? m[2].trim().split(/\s+/) : [];
    } else if (raw.trim() === '') {
      current = null;
    } else {
      current = null;
    }
  }
  return { recipes, prereqs };
}

// Build the effective recipe lines for `test:` by inlining any prerequisite
// target that the test recipe runs as a built binary. ch07 does this: `test:`
// runs ./tests/run-starter, which is produced by a separate `tests/run-starter:`
// rule that holds the actual compile command.
function effectiveTestRecipe(recipes, prereqs) {
  const out = [...(recipes.test || [])];
  for (const dep of prereqs.test || []) {
    if (recipes[dep] && recipes[dep].length) out.push(...recipes[dep]);
  }
  return out;
}

// Pull CXXFLAGS extras (anything beyond the common flags) from the Makefile,
// e.g. `-I.`. Returns { flags:[...extra], include:[...] }.
function parseCxxflags(text) {
  const m = text.match(/^CXXFLAGS\s*:?=\s*(.+)$/m);
  if (!m) die('no CXXFLAGS assignment found');
  const toks = m[1].trim().split(/\s+/);
  const extra = [];
  const include = [];
  for (const t of toks) {
    if (COMMON_FLAGS.includes(t)) continue;
    if (t.startsWith('-I')) include.push(t);
    extra.push(t);
  }
  return { extra, include };
}

// Extract the project name from the README H1.
//   "# Chapter 2 — Functions and Files · Project: the `geo` mini-library"
//     -> "geo mini-library"
//   "# Chapter 14 — Introduction to Classes: The Fraction Class"
//     -> "The Fraction Class"
function projectName(readme) {
  const h1 = readme.split('\n').find((l) => l.startsWith('# ')) || '';
  let s = h1.replace(/^#\s*/, '').trim();
  // Prefer the segment after the last middle-dot separator if present,
  // otherwise the segment after the first colon (the "Chapter N — Title:" split).
  if (s.includes('·')) {
    s = s.split('·').pop().trim();
  } else if (s.includes(':')) {
    // drop the "Chapter N — Title" head, keep the project after the first colon
    s = s.slice(s.indexOf(':') + 1).trim();
  }
  // Strip a leading "Project:" label and articles, and backticks.
  s = s.replace(/^Project\s*:\s*/i, '').trim();
  s = s.replace(/^the\s+/i, '').trim();
  s = s.replace(/`/g, '').trim();
  if (!s) die(`could not derive project name from H1: "${h1}"`);
  return s;
}

// List every file under starter/ as "starter/<name>" (the editable set).
function editableFiles(chapterDir) {
  const dir = join(chapterDir, 'starter');
  if (!existsSync(dir)) die(`${chapterDir}: no starter/ directory`);
  const out = [];
  for (const name of readdirSync(dir).sort()) {
    if (statSync(join(dir, name)).isFile()) out.push(`starter/${name}`);
  }
  if (!out.length) die(`${chapterDir}: starter/ is empty`);
  return out;
}

function fileExists(chapterDir, rel) {
  return existsSync(join(chapterDir, rel));
}

// ---------------------------------------------------------------------------
// Pattern recognizers. Each inspects the `test:` recipe (joined) and returns
// either null (no match) or a fully-built manifest. The driver tries them in
// order and FAILS LOUD if none matches.
// ---------------------------------------------------------------------------

function makeManifest(chapter, project, editable, steps, runStep) {
  return { chapter, project, editable, steps, runStep };
}

// Style A (ch01): compile starter/main.cpp -> run < tests/input.txt ->
// diff against tests/expected.txt.
function tryStyleA(ctx) {
  const { num, chapterDir, testRecipe, include, project, editable } = ctx;
  // Recognize: `./starter/app < tests/input.txt` + `diff tests/expected.txt`.
  const joined = testRecipe.join('\n');
  if (!/starter\/app\s*<\s*tests\/input\.txt/.test(joined)) return null;
  if (!/diff\s+tests\/expected\.txt/.test(joined)) return null;
  if (!fileExists(chapterDir, 'starter/main.cpp')) {
    die(`ch${num}: Style A expected starter/main.cpp`);
  }
  if (!fileExists(chapterDir, 'tests/input.txt') || !fileExists(chapterDir, 'tests/expected.txt')) {
    die(`ch${num}: Style A expected tests/input.txt + tests/expected.txt`);
  }
  const compile = {
    sources: ['starter/main.cpp'],
    flags: [...COMMON_FLAGS, ...include],
    include,
  };
  const steps = [
    {
      name: 'output check',
      compile,
      run: { stdin: 'tests/input.txt' },
      verdict: {
        type: 'diffExpected',
        stdin: 'tests/input.txt',
        expected: 'tests/expected.txt',
      },
    },
  ];
  // runStep for mode:"run": same compile, run the program against the test stdin.
  const runStep = { compile, run: { stdin: 'tests/input.txt' } };
  return makeManifest(num, project, editable, steps, runStep);
}

// Style B2 (ch11/14/26): `$(CXX) $(CXXFLAGS) -Istarter tests/tests.cpp -o ... && ./...`
// Single TU; the learner's header is found via -Istarter. verdict exitCode.
function tryStyleB2(ctx) {
  const { num, chapterDir, testRecipe, include, project, editable, makeText } = ctx;
  const joined = testRecipe.join('\n');
  if (!/-Istarter\b/.test(joined)) return null;
  if (!/tests\/tests\.cpp/.test(joined)) return null;
  // Must NOT link a starter .cpp (header-only).
  if (/starter\/[A-Za-z0-9_]+\.cpp/.test(joined)) {
    die(`ch${num}: looked like B2 but recipe links a starter .cpp`);
  }
  if (!fileExists(chapterDir, 'tests/tests.cpp')) die(`ch${num}: B2 expected tests/tests.cpp`);
  // Sanity: test-solution must use -Isolution (proves the substitution model).
  if (!/-Isolution\b/.test(makeText)) {
    die(`ch${num}: B2 test-solution did not use -Isolution`);
  }
  const include2 = include.includes('-Istarter') ? include : [...include, '-Istarter'];
  const compile = {
    sources: ['tests/tests.cpp'],
    flags: [...COMMON_FLAGS, ...include, '-Istarter'],
    include: include2,
  };
  const steps = [
    { name: 'unit tests', compile, run: {}, verdict: { type: 'exitCode' } },
  ];
  const runStep = { compile, run: {} };
  return makeManifest(num, project, editable, steps, runStep);
}

// ch09: TWO steps. Step 1 = Style B unit (tests.cpp + starter/desk.cpp).
// Step 2 = cin_driver.cpp + starter/desk.cpp, run < tests/cin_input.txt,
// grepDiff on lines matching ^ACCEPTED= == captured expected lines.
function tryChapter09(ctx) {
  const { num, chapterDir, testRecipe, include, project, editable, makeText } = ctx;
  if (num !== 9) return null;
  // ch09 `test:` depends on `test-pure` and `cin-test`; the real recipes live
  // in those targets. Parse them from the full Makefile text.
  if (!fileExists(chapterDir, 'tests/tests.cpp')) die('ch09: missing tests/tests.cpp');
  if (!fileExists(chapterDir, 'tests/cin_driver.cpp')) die('ch09: missing tests/cin_driver.cpp');
  if (!fileExists(chapterDir, 'tests/cin_input.txt')) die('ch09: missing tests/cin_input.txt');

  // Capture the expected ACCEPTED lines from the `printf 'ACCEPTED=...' ` in the
  // cin-test recipe (authoritative — read from the Makefile, do not hardcode).
  const printfM = makeText.match(/printf\s+'([^']*ACCEPTED=[^']*)'/);
  if (!printfM) die('ch09: could not find the printf with expected ACCEPTED lines');
  const expectedLines = printfM[1]
    .split('\\n')
    .map((s) => s.trim())
    .filter((s) => s.length > 0);
  if (!expectedLines.every((l) => l.startsWith('ACCEPTED='))) {
    die(`ch09: parsed unexpected ACCEPTED lines: ${JSON.stringify(expectedLines)}`);
  }
  // Capture the grep pattern from the recipe.
  const grepM = makeText.match(/grep\s+'(\^ACCEPTED=)'/);
  const pattern = grepM ? grepM[1] : '^ACCEPTED=';

  const pureCompile = {
    sources: ['tests/tests.cpp', 'starter/desk.cpp'],
    flags: [...COMMON_FLAGS, ...include],
    include,
  };
  const cinCompile = {
    sources: ['tests/cin_driver.cpp', 'starter/desk.cpp'],
    flags: [...COMMON_FLAGS, ...include],
    include,
  };
  const steps = [
    { name: 'unit tests', compile: pureCompile, run: {}, verdict: { type: 'exitCode' } },
    {
      name: 'cin recovery',
      compile: cinCompile,
      run: { stdin: 'tests/cin_input.txt' },
      verdict: { type: 'grepDiff', pattern, stdin: 'tests/cin_input.txt', expectedLines },
    },
  ];
  // mode:"run" for ch09 = run the unit-test binary (the primary grader).
  const runStep = { compile: pureCompile, run: {} };
  return makeManifest(num, project, editable, steps, runStep);
}

// Style B (most chapters): `$(CXX) $(CXXFLAGS) tests/tests.cpp <impl>.cpp -o ...`
// where <impl> is a starter/*.cpp. verdict exitCode. Captures any -I flags and,
// for ch28, marks the step workdir:true (writable tests/ for fstream).
function tryStyleB(ctx) {
  const { num, chapterDir, testRecipe, include, project, editable, makeText } = ctx;
  const joined = testRecipe.join('\n');
  // Find the compile line that links tests/tests.cpp with a starter .cpp.
  const line = testRecipe.find(
    (l) => /tests\/tests\.cpp/.test(l) && /starter\/[A-Za-z0-9_]+\.cpp/.test(l)
  );
  if (!line) return null;
  const implM = line.match(/(starter\/[A-Za-z0-9_]+\.cpp)/);
  if (!implM) die(`ch${num}: Style B could not extract starter source`);
  const impl = implM[1];
  if (!fileExists(chapterDir, 'tests/tests.cpp')) die(`ch${num}: Style B expected tests/tests.cpp`);
  if (!fileExists(chapterDir, impl)) die(`ch${num}: Style B impl ${impl} not found`);
  // Sanity: test-solution must swap to a solution/*.cpp of the same name.
  const solName = impl.replace(/^starter\//, 'solution/');
  if (!new RegExp(solName.replace(/[/.]/g, '\\$&')).test(makeText)) {
    die(`ch${num}: Style B test-solution did not reference ${solName}`);
  }
  const compile = {
    sources: ['tests/tests.cpp', impl],
    flags: [...COMMON_FLAGS, ...include],
    include,
  };
  const step = { name: 'unit tests', compile, run: {}, verdict: { type: 'exitCode' } };
  // ch28 writes/reads tests/tmp_report.txt at run time -> needs writable tests/.
  if (num === 28) step.workdir = true;
  const steps = [step];
  const runStep = { compile, run: {} };
  return makeManifest(num, project, editable, steps, runStep);
}

const RECOGNIZERS = [tryChapter09, tryStyleA, tryStyleB2, tryStyleB];

function buildChapter(num) {
  const dir = join(DRILLS, `chapter-${String(num).padStart(2, '0')}`);
  const makePath = join(dir, 'Makefile');
  if (!existsSync(makePath)) die(`chapter-${num}: no Makefile at ${makePath}`);
  const makeText = readFileSync(makePath, 'utf8');
  const { recipes, prereqs } = parseMakefile(makeText);
  if (!recipes.test) die(`chapter-${num}: no \`test:\` target`);

  const readme = readFileSync(join(dir, 'README.md'), 'utf8');
  const project = projectName(readme);
  const editable = editableFiles(dir);
  const { include } = parseCxxflags(makeText);

  const ctx = {
    num,
    chapterDir: dir,
    testRecipe: effectiveTestRecipe(recipes, prereqs),
    recipes,
    prereqs,
    include,
    project,
    editable,
    makeText,
  };

  for (const fn of RECOGNIZERS) {
    const manifest = fn(ctx);
    if (manifest) return { manifest, style: fn.name };
  }
  die(
    `chapter-${num}: \`test:\` recipe matched NO known grader pattern.\n` +
      `  recipe:\n${targets.test.map((l) => '    ' + l).join('\n')}`
  );
}

function main() {
  mkdirSync(OUT_DIR, { recursive: true });
  const summary = [];
  for (let n = 1; n <= 28; n++) {
    const { manifest, style } = buildChapter(n);
    const out = join(OUT_DIR, `chapter-${String(n).padStart(2, '0')}.json`);
    writeFileSync(out, JSON.stringify(manifest, null, 2) + '\n');
    summary.push({
      ch: n,
      style: style.replace(/^try/, ''),
      steps: manifest.steps.length,
      verdicts: manifest.steps.map((s) => s.verdict.type).join('+'),
    });
  }
  console.log('Generated 28 grader manifests in judge/manifests/\n');
  console.log('ch  style       steps  verdicts');
  for (const r of summary) {
    console.log(
      `${String(r.ch).padStart(2, '0')}  ${r.style.padEnd(10)}  ${String(r.steps).padStart(5)}  ${r.verdicts}`
    );
  }
}

main();
