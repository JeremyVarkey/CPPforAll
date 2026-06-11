/**
 * build-practice.mjs — src/practice-content/chapter-NN.json → src/generated/practice/chapter-NN.json
 *
 * Standalone, NOT wired into build-content.mjs (the orchestrator owns that file).
 * Run directly:   node scripts/build-practice.mjs
 *
 * For each authored practice file it:
 *   1. Validates the schema (chapter number, drill ids `cNN-dM`, tiers, titles,
 *      lesson anchors resolve against src/generated/notes/chapter-NN.json,
 *      tier-mix rules, predict choices/answer sanity, fix/write harness presence).
 *   2. Emits a CLIENT-SAFE JSON with `harness` and `solution` STRIPPED — those
 *      bytes must never reach the browser (they live only on the judge image,
 *      baked from src/practice-content/ directly).
 *   3. Writes a per-chapter + total count summary.
 *
 * Authoring agents (C01–C12) run concurrently, so MISSING chapter files are
 * tolerated (skipped, reported). A file that EXISTS but is invalid is a LOUD
 * failure: the script prints every problem and exits nonzero.
 */

import { readFile, readdir, mkdir, writeFile } from 'node:fs/promises';
import { existsSync } from 'node:fs';
import path from 'node:path';

const ROOT = path.resolve(new URL('..', import.meta.url).pathname);
const SRC = path.join(ROOT, 'src/practice-content');
const NOTES = path.join(ROOT, 'src/generated/notes');
const OUT = path.join(ROOT, 'src/generated/practice');

const TIERS = new Set(['predict', 'fix', 'write']);
const MIN_DRILLS = 4;
const MAX_DRILLS = 6;

const pad = (n) => String(n).padStart(2, '0');

/** Load a chapter's notes anchors → Set, or null if the notes file is absent. */
async function loadAnchors(chapterNum) {
  const file = path.join(NOTES, `chapter-${pad(chapterNum)}.json`);
  if (!existsSync(file)) return null;
  try {
    const j = JSON.parse(await readFile(file, 'utf8'));
    const anchors = new Set();
    for (const lesson of j.lessons || []) {
      if (lesson && typeof lesson.anchor === 'string') anchors.add(lesson.anchor);
    }
    return anchors;
  } catch {
    return null;
  }
}

/**
 * Validate one parsed practice file. Returns { errors:[], clientDrills:[], counts }.
 * `errors` non-empty ⇒ the file is rejected (no output written for it).
 */
function validateChapter(num, data, anchors, errors) {
  const E = (msg) => errors.push(`chapter-${pad(num)}: ${msg}`);

  if (data === null || typeof data !== 'object' || Array.isArray(data)) {
    E('top-level JSON must be an object');
    return { clientDrills: [], counts: {} };
  }
  if (data.chapter !== num) {
    E(`"chapter" field is ${JSON.stringify(data.chapter)}, expected ${num} (from filename)`);
  }
  if (!Array.isArray(data.drills)) {
    E('"drills" must be an array');
    return { clientDrills: [], counts: {} };
  }

  const drills = data.drills;
  if (drills.length < MIN_DRILLS || drills.length > MAX_DRILLS) {
    E(`has ${drills.length} drills; spec requires ${MIN_DRILLS}–${MAX_DRILLS}`);
  }

  const idRe = new RegExp(`^c${pad(num)}-d\\d+$`);
  const seenIds = new Set();
  const tierCounts = { predict: 0, fix: 0, write: 0 };
  const clientDrills = [];

  drills.forEach((d, i) => {
    const where = `drill[${i}]`;
    if (d === null || typeof d !== 'object' || Array.isArray(d)) {
      E(`${where} must be an object`);
      return;
    }

    // id
    if (typeof d.id !== 'string' || !idRe.test(d.id)) {
      E(`${where} id ${JSON.stringify(d.id)} must match c${pad(num)}-dM`);
    } else if (seenIds.has(d.id)) {
      E(`${where} duplicate id "${d.id}"`);
    } else {
      seenIds.add(d.id);
    }

    // tier
    if (!TIERS.has(d.tier)) {
      E(`${where} (${d.id}) tier ${JSON.stringify(d.tier)} must be predict|fix|write`);
    } else {
      tierCounts[d.tier] += 1;
    }

    // lesson anchor — "" allowed (chapter-general); otherwise must resolve.
    if (typeof d.lesson !== 'string') {
      E(`${where} (${d.id}) "lesson" must be a string (or "")`);
    } else if (d.lesson !== '') {
      if (anchors === null) {
        E(`${where} (${d.id}) lesson "${d.lesson}" cannot be checked — notes chapter-${pad(num)}.json missing`);
      } else if (!anchors.has(d.lesson)) {
        E(`${where} (${d.id}) lesson anchor "${d.lesson}" not found in notes chapter-${pad(num)}.json`);
      }
    }

    // title / prompt
    if (typeof d.title !== 'string' || d.title.trim() === '') {
      E(`${where} (${d.id}) "title" must be a non-empty string`);
    }
    if (typeof d.promptMd !== 'string' || d.promptMd.trim() === '') {
      E(`${where} (${d.id}) "promptMd" must be a non-empty string`);
    }
    if (typeof d.code !== 'string' || d.code.trim() === '') {
      E(`${where} (${d.id}) "code" must be a non-empty string`);
    }
    if (d.explanationMd != null && typeof d.explanationMd !== 'string') {
      E(`${where} (${d.id}) "explanationMd" must be a string when present`);
    }

    // tier-specific
    if (d.tier === 'predict') {
      if (!Array.isArray(d.choices) || d.choices.length < 2) {
        E(`${where} (${d.id}) predict needs a "choices" array of ≥2 strings`);
      } else {
        if (!d.choices.every((c) => typeof c === 'string' && c.length > 0)) {
          E(`${where} (${d.id}) every choice must be a non-empty string`);
        }
        const uniq = new Set(d.choices);
        if (uniq.size !== d.choices.length) {
          E(`${where} (${d.id}) choices must be distinct`);
        }
        if (!Number.isInteger(d.answer) || d.answer < 0 || d.answer >= d.choices.length) {
          E(`${where} (${d.id}) "answer" must be an integer index into choices (0..${d.choices.length - 1})`);
        }
      }
      // predict must NOT carry harness/solution (those are fix/write-only).
      if (d.harness != null) E(`${where} (${d.id}) predict must not have a "harness"`);
      if (d.solution != null) E(`${where} (${d.id}) predict must not have a "solution"`);
    } else if (d.tier === 'fix' || d.tier === 'write') {
      if (typeof d.harness !== 'string' || d.harness.trim() === '') {
        E(`${where} (${d.id}) ${d.tier} needs a non-empty "harness" (test driver owning main())`);
      }
      if (typeof d.solution !== 'string' || d.solution.trim() === '') {
        E(`${where} (${d.id}) ${d.tier} needs a non-empty "solution" (reference fix)`);
      }
      if (d.choices != null) E(`${where} (${d.id}) ${d.tier} must not have "choices"`);
      if (d.answer != null) E(`${where} (${d.id}) ${d.tier} must not have an "answer"`);
    }

    // Build the CLIENT-SAFE projection (harness + solution STRIPPED).
    const safe = {
      id: d.id,
      lesson: typeof d.lesson === 'string' ? d.lesson : '',
      tier: d.tier,
      title: d.title,
      promptMd: d.promptMd,
      code: d.code,
    };
    if (d.tier === 'predict') {
      safe.choices = d.choices;
      safe.answer = d.answer;
    }
    if (d.explanationMd != null) safe.explanationMd = d.explanationMd;
    // EXPLICITLY never copy d.harness or d.solution.
    clientDrills.push(safe);
  });

  // Tier-mix rule: ≥1 predict, ≥1 fix, ≥1 write.
  for (const tier of ['predict', 'fix', 'write']) {
    if (tierCounts[tier] < 1) {
      E(`tier mix: needs ≥1 "${tier}" drill (has ${tierCounts[tier]})`);
    }
  }

  return {
    clientDrills,
    counts: { total: drills.length, ...tierCounts },
  };
}

export async function buildPractice() {
  if (!existsSync(SRC)) {
    console.log(`[practice] no src/practice-content/ dir yet — nothing to build`);
    return { chapters: [], total: 0, missing: [] };
  }
  await mkdir(OUT, { recursive: true });

  const files = (await readdir(SRC))
    .filter((f) => /^chapter-\d{2}\.json$/.test(f))
    .sort();

  // Chapters 1–12 are the R2 scope; report which are not yet authored.
  const present = new Set(files.map((f) => Number(f.match(/chapter-(\d{2})\.json/)[1])));
  const missing = [];
  for (let n = 1; n <= 12; n++) if (!present.has(n)) missing.push(n);

  const errors = [];
  const built = [];

  for (const f of files) {
    const num = Number(f.match(/chapter-(\d{2})\.json/)[1]);
    let data;
    try {
      data = JSON.parse(await readFile(path.join(SRC, f), 'utf8'));
    } catch (e) {
      errors.push(`chapter-${pad(num)}: invalid JSON — ${e.message}`);
      continue;
    }
    const anchors = await loadAnchors(num);
    const before = errors.length;
    const { clientDrills, counts } = validateChapter(num, data, anchors, errors);
    if (errors.length > before) continue; // had errors → don't emit

    const out = { chapter: num, drills: clientDrills };
    await writeFile(path.join(OUT, `chapter-${pad(num)}.json`), JSON.stringify(out));
    built.push({ num, ...counts });
  }

  // Loud failure.
  if (errors.length) {
    console.error(`\n[practice] ✗ ${errors.length} validation problem(s):`);
    for (const e of errors) console.error('  - ' + e);
    throw new Error(`practice validation failed (${errors.length} problem(s))`);
  }

  // Count summary.
  console.log(`[practice] built ${built.length} chapter file(s):`);
  let total = 0;
  for (const b of built) {
    total += b.total;
    console.log(
      `  chapter-${pad(b.num)}: ${b.total} drills ` +
      `(predict ${b.predict}, fix ${b.fix}, write ${b.write})`,
    );
  }
  console.log(`[practice] ${total} drills total across ${built.length} chapter(s).`);
  if (missing.length) {
    console.log(`[practice] not yet authored (ch 1–12): ${missing.map(pad).join(', ')}`);
  }

  return { chapters: built, total, missing };
}

if (import.meta.url === `file://${process.argv[1]}`) {
  buildPractice().catch((e) => {
    console.error(`[practice] FATAL: ${e.message}`);
    process.exit(1);
  });
}
