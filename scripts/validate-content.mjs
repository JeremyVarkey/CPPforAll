/**
 * validate-content.mjs — pipeline output sanity gate. Nonzero exit fails the
 * build: a broken parse must never ship as a broken page.
 */
import { readFile, readdir } from 'node:fs/promises';
import path from 'node:path';

const ROOT = path.resolve(new URL('..', import.meta.url).pathname);
const GEN = path.join(ROOT, 'src/generated');

const errors = [];
const check = (cond, msg) => { if (!cond) errors.push(msg); };

export async function validateContent() {
  const chapters = JSON.parse(await readFile(path.join(GEN, 'chapters.json'), 'utf8'));
  check(chapters.length === 29, `chapters.json: expected 29, got ${chapters.length}`);
  check(chapters.filter((c) => c.hasDrill).length === 28, 'chapters.json: expected 28 drills');

  // notes
  const noteFiles = (await readdir(path.join(GEN, 'notes'))).filter((f) => f.endsWith('.json'));
  check(noteFiles.length === 29, `notes: expected 29 JSONs, got ${noteFiles.length}`);
  for (const f of noteFiles.sort()) {
    const n = JSON.parse(await readFile(path.join(GEN, 'notes', f), 'utf8'));
    check(n.lessons.length >= 1, `${f}: no lessons`);
    check(Boolean(n.title), `${f}: no title`);
    for (const l of n.lessons) {
      check(Boolean(l.anchor), `${f}: lesson "${l.title}" has no anchor`);
      check(l.html.length > 0, `${f}: lesson "${l.title}" is empty`);
    }
    // anchors must be unique within a chapter
    const anchors = n.lessons.map((l) => l.anchor);
    check(new Set(anchors).size === anchors.length, `${f}: duplicate lesson anchors`);
    // no unrendered markdown artifacts in lesson HTML
    for (const l of n.lessons) {
      if (/^##\s/m.test(l.html.replace(/<[^>]+>/g, ''))) {
        errors.push(`${f}: lesson "${l.title}" contains raw markdown heading`);
      }
    }
  }

  // drills
  const drillFiles = (await readdir(path.join(GEN, 'drills'))).filter((f) => f.endsWith('.json'));
  check(drillFiles.length === 28, `drills: expected 28 JSONs, got ${drillFiles.length}`);
  for (const f of drillFiles.sort()) {
    const d = JSON.parse(await readFile(path.join(GEN, 'drills', f), 'utf8'));
    check(Boolean(d.project), `${f}: no project name`);
    check(d.tasks.length > 0 || Boolean(d.tasksHtml), `${f}: no tasks`);
    check(d.criteria.length > 0 || Boolean(d.criteriaHtml), `${f}: no criteria`);
    check(d.files.some((x) => x.kind === 'starter'), `${f}: no starter file`);
    check(d.files.some((x) => x.kind === 'solution'), `${f}: no solution file`);
    check(d.files.some((x) => x.kind === 'tests'), `${f}: no tests file`);
    for (const k of ['concepts', 'constraints', 'buildRun', 'hints', 'stretch']) {
      check(Boolean(d.sections[k]?.length), `${f}: section "${k}" empty`);
    }
    for (const file of d.files) {
      check(file.code.length > 0, `${f}: file ${file.path} is empty`);
    }
  }

  return errors;
}

if (import.meta.url === `file://${process.argv[1]}`) {
  const errs = await validateContent();
  if (errs.length) {
    console.error(`✗ validate-content: ${errs.length} problem(s)`);
    for (const e of errs) console.error('  - ' + e);
    process.exit(1);
  }
  console.log('✓ validate-content: all checks passed');
}
