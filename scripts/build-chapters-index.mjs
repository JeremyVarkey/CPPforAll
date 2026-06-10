/**
 * build-chapters-index.mjs — generates src/generated/chapters.json,
 * the single binding source for the rail, landing, breadcrumbs, and routes.
 *
 * Titles come from the notes' H1s; project names + grader styles from the
 * drills index table in content/drills/CLAUDE.md. The prototype's chapter
 * data was placeholder — THIS is the real list.
 */
import { readFile, readdir, mkdir, writeFile } from 'node:fs/promises';
import path from 'node:path';

const ROOT = path.resolve(new URL('..', import.meta.url).pathname);
const NOTES = path.join(ROOT, 'content/notes');
const DRILLS = path.join(ROOT, 'content/drills');
const OUT = path.join(ROOT, 'src/generated');

/** Short rail tags, curated per chapter (design wants one compact concept tag). */
const TAGS = {
  0: 'setup', 1: 'basics', 2: 'functions', 3: 'debugging', 4: 'types',
  5: 'const & strings', 6: 'operators', 7: 'scope & linkage', 8: 'control flow',
  9: 'error handling', 10: 'conversions', 11: 'function templates',
  12: 'pointers & refs', 13: 'enums & structs', 14: 'classes', 15: 'classes II',
  16: 'vectors', 17: 'arrays', 18: 'iterators & algos', 19: 'dynamic memory',
  20: 'functions II', 21: 'operator overloading', 22: 'move & smart ptrs',
  23: 'relationships', 24: 'inheritance', 25: 'virtual functions',
  26: 'class templates', 27: 'exceptions', 28: 'streams & files',
};

export async function buildChaptersIndex() {
  // 1) Drills index table → project name + grader style per chapter
  const drillsGuide = await readFile(path.join(DRILLS, 'CLAUDE.md'), 'utf8');
  const drillMeta = new Map();
  for (const line of drillsGuide.split('\n')) {
    // | 01 | Cash Register | variables, I/O, arithmetic expressions | output | ✅ done (template) |
    const m = line.match(/^\|\s*(\d{2})\s*\|\s*([^|]+?)\s*\|\s*([^|]+?)\s*\|\s*([^|]+?)\s*\|/);
    if (m) drillMeta.set(Number(m[1]), { project: m[2], concept: m[3], grader: m[4] });
  }

  // 2) Notes H1s → titles
  const noteFiles = (await readdir(NOTES)).filter((f) => /^chapter-\d{2}\.md$/.test(f)).sort();
  const chapters = [];
  for (const f of noteFiles) {
    const num = Number(f.match(/chapter-(\d{2})/)[1]);
    const src = await readFile(path.join(NOTES, f), 'utf8');
    const h1 = src.match(/^#\s+(.+)$/m)?.[1] ?? `Chapter ${num}`;
    // "Chapter 12 — Compound Types: References and Pointers" → keep the part after the dash
    const title = h1.replace(/^Chapter\s+\d+\s*[—–-]\s*/i, '').trim();
    const meta = drillMeta.get(num);
    // overlay may refine the rail tag (agent-confirmed, ≤22 chars)
    let tag = TAGS[num] ?? '';
    try {
      const overlay = JSON.parse(
        await readFile(path.join(ROOT, 'src/content-overlays', `chapter-${String(num).padStart(2, '0')}.json`), 'utf8'),
      );
      if (typeof overlay.tag === 'string' && overlay.tag.length <= 22) tag = overlay.tag;
    } catch {}
    chapters.push({
      num,
      slug: String(num).padStart(2, '0'),
      title,
      tag,
      hasDrill: Boolean(meta),
      project: meta?.project ?? null,
      concept: meta?.concept ?? null,
      grader: meta?.grader ?? null,
    });
  }
  chapters.sort((a, b) => a.num - b.num);

  if (chapters.length !== 29) {
    throw new Error(`expected 29 chapters (00–28), got ${chapters.length}`);
  }
  const withDrills = chapters.filter((c) => c.hasDrill).length;
  if (withDrills !== 28) {
    throw new Error(`expected 28 chapters with drills, got ${withDrills}`);
  }

  await mkdir(OUT, { recursive: true });
  await writeFile(path.join(OUT, 'chapters.json'), JSON.stringify(chapters, null, 2));
  return chapters;
}

if (import.meta.url === `file://${process.argv[1]}`) {
  const ch = await buildChaptersIndex();
  console.log(`chapters.json: ${ch.length} chapters, ${ch.filter((c) => c.hasDrill).length} with drills`);
}
