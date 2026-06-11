// test-manifests.mjs — the REPLAY GATE (docs/runner-plan.md "Replay gate").
//
// For EVERY manifest in MANIFESTS_DIR, this script runs the judge in submit mode:
//   1. against the PRISTINE starter (no learner files)   → MUST be red.
//   2. against the SOLUTION files                        → MUST be green.
//
// A manifest that doesn't go red on the starter would let a learner "pass" without
// doing the work; one that doesn't go green on the solution is unsolvable. Either
// is a bug in the manifest (or a regression in the toolchain), and this script
// exits NONZERO so the container build fails BEFORE it can serve a single learner.
//
// Solution files are read from DRILLS_WITH_SOLUTIONS_DIR (a tree that still
// contains each chapter's solution/ dir). This dir is used by THIS SCRIPT ONLY —
// the running judge copies from DRILLS_DIR with solution/ stripped, so a learner
// can never see the reference. For each editable path "starter/<name>", we read
// the sibling "solution/<name>" and feed it as the learner's submission.
//
// Env:
//   MANIFESTS_DIR             default /app/manifests
//   DRILLS_DIR                pristine (no solution/) tree — what the judge serves
//   DRILLS_WITH_SOLUTIONS_DIR tree that still has solution/ — for the green case
//
// Usage:  node test-manifests.mjs            # all chapters
//         node test-manifests.mjs 1 2 9      # a subset (chapter numbers)

import { readdir, readFile, stat } from 'node:fs/promises';
import { existsSync } from 'node:fs';
import { join } from 'node:path';
import { runJob, runPracticeJob } from './runner.mjs';

const MANIFESTS_DIR = process.env.MANIFESTS_DIR || '/app/manifests';
const DRILLS_WITH_SOLUTIONS_DIR =
  process.env.DRILLS_WITH_SOLUTIONS_DIR || process.env.DRILLS_DIR || '/app/drills';
// Practice files (with harness + solution) baked from src/practice-content.
const PRACTICE_DIR = process.env.PRACTICE_DIR || '/app/practice';

function pad(n) {
  return String(n).padStart(2, '0');
}

async function listManifestChapters() {
  const entries = await readdir(MANIFESTS_DIR);
  const chapters = [];
  for (const e of entries) {
    const m = /^chapter-(\d{2})\.json$/.exec(e);
    if (m) chapters.push(Number(m[1]));
  }
  return chapters.sort((a, b) => a - b);
}

async function loadManifest(chapter) {
  const raw = await readFile(join(MANIFESTS_DIR, `chapter-${pad(chapter)}.json`), 'utf8');
  return JSON.parse(raw);
}

// Build the { editablePath: solutionText } map for the green run.
async function solutionFilesFor(chapter, manifest) {
  const editable = Array.isArray(manifest.editable) ? manifest.editable : [];
  const chapterDir = join(DRILLS_WITH_SOLUTIONS_DIR, `chapter-${pad(chapter)}`);
  const files = {};
  for (const ed of editable) {
    // ed looks like "starter/<name>". The reference lives at "solution/<name>".
    const name = ed.replace(/^starter\//, '');
    const solPath = join(chapterDir, 'solution', name);
    try {
      files[ed] = await readFile(solPath, 'utf8');
    } catch (e) {
      throw new Error(`missing solution file ${solPath} for editable ${ed}: ${e.message}`);
    }
  }
  return files;
}

function summarize(result) {
  const pills = result.steps
    .map((s) => `${s.name}:${s.verdict}`)
    .join(', ');
  return `${result.overall} [${pills}]`;
}

function firstDetail(result) {
  for (const s of result.steps) {
    if (s.verdict !== 'pass' && s.detail) return s.detail;
  }
  // Fall back to stderr of the first non-pass step.
  for (const s of result.steps) {
    if (s.verdict !== 'pass' && s.stderr) return s.stderr.slice(0, 400);
  }
  return '';
}

// ── Practice replay gate ─────────────────────────────────────────────────────
// For every fix/write drill in every baked practice chapter file: the authored
// `code` (broken/starter) must compile+run RED, and `solution` must run GREEN.
// Predict drills have no executable form and are skipped here (their answers are
// machine-verified separately by the orchestrator). Chapters with no practice
// file are simply skipped — practice is authored chapter-by-chapter.
async function listPracticeChapters() {
  if (!existsSync(PRACTICE_DIR)) return [];
  const entries = await readdir(PRACTICE_DIR);
  const chapters = [];
  for (const e of entries) {
    const m = /^chapter-(\d{2})\.json$/.exec(e);
    if (m) chapters.push(Number(m[1]));
  }
  return chapters.sort((a, b) => a - b);
}

async function loadPracticeFile(chapter) {
  const raw = await readFile(join(PRACTICE_DIR, `chapter-${pad(chapter)}.json`), 'utf8');
  return JSON.parse(raw);
}

// Returns the number of failures.
async function practiceGate(onlyChapters) {
  let chapters = await listPracticeChapters();
  if (onlyChapters && onlyChapters.length) {
    const want = new Set(onlyChapters);
    chapters = chapters.filter((c) => want.has(c));
  }
  if (chapters.length === 0) {
    console.log('Practice replay gate: no practice files found — skipping.');
    return 0;
  }

  console.log(`Practice replay gate: ${chapters.length} chapter(s) — code→red, solution→green`);
  console.log(`  PRACTICE_DIR=${PRACTICE_DIR}`);
  console.log('');

  let failures = 0;
  let drillsChecked = 0;
  for (const chapter of chapters) {
    let practice;
    try {
      practice = await loadPracticeFile(chapter);
    } catch (e) {
      console.log(`✗ practice ch${pad(chapter)}  PARSE ERROR: ${e.message}`);
      failures++;
      continue;
    }
    const drills = Array.isArray(practice.drills) ? practice.drills : [];
    const runnable = drills.filter((d) => d && (d.tier === 'fix' || d.tier === 'write'));
    if (runnable.length === 0) {
      console.log(`· practice ch${pad(chapter)}  (no fix/write drills)`);
      continue;
    }

    for (const drill of runnable) {
      drillsChecked++;
      // 1) authored code (broken/starter) → expect red.
      let codeRes;
      try {
        codeRes = await runPracticeJob({
          chapter, drill: drill.id, files: { 'student.cpp': drill.code }, practice,
        });
      } catch (e) {
        console.log(`✗ ${drill.id}  code run threw: ${e.message}`);
        failures++;
        continue;
      }
      const codeRed = codeRes.overall === 'red';

      // 2) solution → expect green.
      let solRes;
      try {
        solRes = await runPracticeJob({
          chapter, drill: drill.id, files: { 'student.cpp': drill.solution }, practice,
        });
      } catch (e) {
        console.log(`✗ ${drill.id}  solution run threw: ${e.message}`);
        failures++;
        continue;
      }
      const solGreen = solRes.overall === 'green';

      if (codeRed && solGreen) {
        console.log(`✓ ${drill.id} (${drill.tier})  code→red, solution→green`);
      } else {
        failures++;
        console.log(`✗ ${drill.id} (${drill.tier})`);
        console.log(`     code     → ${summarize(codeRes)}  ${codeRed ? '' : '(EXPECTED red)'}`);
        if (!codeRed) {
          const d = firstDetail(codeRes);
          if (d) console.log(`        detail: ${d.split('\n').slice(0, 6).join('\n                ')}`);
        }
        console.log(`     solution → ${summarize(solRes)}  ${solGreen ? '' : '(EXPECTED green)'}`);
        if (!solGreen) {
          const d = firstDetail(solRes);
          if (d) console.log(`        detail: ${d.split('\n').slice(0, 8).join('\n                ')}`);
        }
      }
    }
  }

  console.log('');
  if (failures) {
    console.log(`PRACTICE GATE: ${failures} drill(s) misbehaved (${drillsChecked} checked).`);
  } else {
    console.log(`PRACTICE GATE PASSED: ${drillsChecked} fix/write drill(s) red→green correctly.`);
  }
  return failures;
}

async function main() {
  const argChapters = process.argv.slice(2).map(Number).filter((n) => Number.isInteger(n));
  let chapters;
  try {
    chapters = argChapters.length ? argChapters : await listManifestChapters();
  } catch (e) {
    console.error(`FATAL: cannot read MANIFESTS_DIR=${MANIFESTS_DIR}: ${e.message}`);
    process.exit(2);
  }

  // If there are no manifests, fall through to the practice gate (which may have
  // files) rather than hard-failing — supports practice-only runs in dev. The
  // container build always has all 28 manifests, so this branch is dev-only.
  if (chapters.length === 0) {
    console.warn(`No manifests found in ${MANIFESTS_DIR}. Running practice gate only.`);
    const practiceFailures = await practiceGate(argChapters.length ? argChapters : null);
    process.exit(practiceFailures ? 1 : 0);
  }

  // Confirm the with-solutions tree exists.
  try {
    const st = await stat(DRILLS_WITH_SOLUTIONS_DIR);
    if (!st.isDirectory()) throw new Error('not a directory');
  } catch (e) {
    console.error(`FATAL: DRILLS_WITH_SOLUTIONS_DIR=${DRILLS_WITH_SOLUTIONS_DIR} unusable: ${e.message}`);
    process.exit(2);
  }

  console.log(`Replay gate: ${chapters.length} chapter(s) — pristine→red, solution→green`);
  console.log(`  MANIFESTS_DIR=${MANIFESTS_DIR}`);
  console.log(`  DRILLS_DIR=${process.env.DRILLS_DIR || '(default)'}`);
  console.log(`  DRILLS_WITH_SOLUTIONS_DIR=${DRILLS_WITH_SOLUTIONS_DIR}`);
  console.log('');

  let failures = 0;
  for (const chapter of chapters) {
    let manifest;
    try {
      manifest = await loadManifest(chapter);
    } catch (e) {
      console.log(`✗ ch${pad(chapter)}  MANIFEST PARSE ERROR: ${e.message}`);
      failures++;
      continue;
    }

    // 1) Pristine starter → expect red (fail), NOT green, NOT a judge error.
    let starterRes;
    try {
      starterRes = await runJob({ chapter, mode: 'submit', files: {}, manifest });
    } catch (e) {
      console.log(`✗ ch${pad(chapter)}  starter run threw: ${e.message}`);
      failures++;
      continue;
    }
    const starterOk = starterRes.overall === 'red';

    // 2) Solution → expect green.
    let solRes;
    try {
      const solFiles = await solutionFilesFor(chapter, manifest);
      solRes = await runJob({ chapter, mode: 'submit', files: solFiles, manifest });
    } catch (e) {
      console.log(`✗ ch${pad(chapter)}  solution run threw: ${e.message}`);
      failures++;
      continue;
    }
    const solOk = solRes.overall === 'green';

    if (starterOk && solOk) {
      console.log(`✓ ch${pad(chapter)}  ${manifest.project || ''}`);
      console.log(`     starter → ${summarize(starterRes)}`);
      console.log(`     solution → ${summarize(solRes)}`);
    } else {
      failures++;
      console.log(`✗ ch${pad(chapter)}  ${manifest.project || ''}`);
      console.log(`     starter  → ${summarize(starterRes)}  ${starterOk ? '' : '(EXPECTED red)'}`);
      if (!starterOk) {
        const d = firstDetail(starterRes);
        if (d) console.log(`        detail: ${d.split('\n').slice(0, 6).join('\n                ')}`);
      }
      console.log(`     solution → ${summarize(solRes)}  ${solOk ? '' : '(EXPECTED green)'}`);
      if (!solOk) {
        const d = firstDetail(solRes);
        if (d) console.log(`        detail: ${d.split('\n').slice(0, 8).join('\n                ')}`);
      }
    }
  }

  console.log('');
  if (failures) {
    console.error(`REPLAY GATE FAILED: ${failures}/${chapters.length} chapter(s) misbehaved.`);
  } else {
    console.log(`REPLAY GATE PASSED: all ${chapters.length} chapter(s) red→green correctly.`);
  }

  // ── Practice gate (R2) — runs after the lab gate. Scoped by the same chapter
  // args (if any). Skips entirely when no practice files are baked.
  console.log('');
  const practiceFailures = await practiceGate(argChapters.length ? argChapters : null);

  const total = failures + practiceFailures;
  if (total) {
    console.error(`\nGATE FAILED: ${failures} lab + ${practiceFailures} practice problem(s).`);
    process.exit(1);
  }
  process.exit(0);
}

main().catch((e) => {
  console.error(`FATAL: ${e.stack || e.message}`);
  process.exit(2);
});
