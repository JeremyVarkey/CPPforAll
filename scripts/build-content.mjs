/**
 * build-content.mjs — content pipeline entrypoint, run as `npm run content`
 * (pre-dev and pre-build). Order matters; each step throws on bad input so
 * a broken pipeline fails the build rather than shipping a broken page.
 */
import { buildChaptersIndex } from './build-chapters-index.mjs';
import { buildNotes } from './build-notes.mjs';
import { buildDrills } from './build-drills.mjs';
import { buildPractice } from './build-practice.mjs';
import { validateContent } from './validate-content.mjs';

const t0 = Date.now();
const chapters = await buildChaptersIndex();
console.log(`[content] chapters index: ${chapters.length} chapters`);
const notes = await buildNotes();
console.log(`[content] notes: ${notes.length} chapters, ${notes.reduce((s, n) => s + n.lessons, 0)} lessons`);
const drills = await buildDrills();
console.log(`[content] drills: ${drills.length} exercises`);
const practice = await buildPractice();
console.log(`[content] practice: ${practice.chapters.length} chapters, ${practice.total} drills`);
const errs = await validateContent();
if (errs.length) {
  console.error(`[content] ✗ validation: ${errs.length} problem(s)`);
  for (const e of errs) console.error('  - ' + e);
  process.exit(1);
}
console.log(`[content] ✓ validated in ${((Date.now() - t0) / 1000).toFixed(1)}s`);
