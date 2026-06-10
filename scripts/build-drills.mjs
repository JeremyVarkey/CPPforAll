/**
 * build-drills.mjs — content/drills/chapter-NN/ → src/generated/drills/chapter-NN.json
 *
 * README splits deterministically on the 8 fixed H2 sections (the authoring
 * guide's contract). Code files are read verbatim and Shiki-highlighted.
 *
 * Output: { num, project, summaryHtml, tasks[], criteria[], sections:{...},
 *           files:[{ kind, name, path, lang, code, codeHtml }] }
 */
import { readFile, readdir, mkdir, writeFile, stat } from 'node:fs/promises';
import path from 'node:path';
import { renderMarkdown, renderToTree, toHtml, highlightCode } from './lib/markdown.mjs';

const ROOT = path.resolve(new URL('..', import.meta.url).pathname);
const DRILLS = path.join(ROOT, 'content/drills');
const OUT = path.join(ROOT, 'src/generated/drills');

const SECTIONS = [
  'The project',
  'Concepts practiced',
  'Your tasks',
  'Constraints',
  'Build & run',
  'Success criteria',
  'Hints',
  'Stretch goals',
];

function splitReadme(md) {
  const out = {};
  const headingRe = /^##\s+(.+)$/gm;
  const found = [];
  let m;
  while ((m = headingRe.exec(md)) !== null) {
    found.push({ title: m[1].trim(), start: m.index, bodyStart: m.index + m[0].length });
  }
  for (let i = 0; i < found.length; i++) {
    const body = md.slice(found[i].bodyStart, found[i + 1]?.start ?? md.length).trim();
    const canonical = SECTIONS.find((s) => found[i].title.toLowerCase().startsWith(s.toLowerCase()));
    if (canonical) out[canonical] = body;
  }
  return out;
}

/** Extract top-level list items from a markdown section → array of HTML fragments. */
async function listItemsHtml(md) {
  const tree = await renderToTree(md);
  const items = [];
  for (const node of tree.children) {
    if (node.type === 'element' && (node.tagName === 'ol' || node.tagName === 'ul')) {
      for (const li of node.children) {
        if (li.type === 'element' && li.tagName === 'li') {
          items.push(toHtml({ type: 'root', children: li.children }, { allowDangerousHtml: true }).trim());
        }
      }
    }
  }
  return items;
}

const KIND_ORDER = { starter: 0, solution: 1, tests: 2, extra: 3 };
function langFor(name) {
  if (/\.(cpp|cc|cxx|h|hpp)$/.test(name)) return 'cpp';
  if (/^makefile$/i.test(name)) return 'make';
  if (/\.(txt|md)$/.test(name)) return null;
  return null;
}

async function collectFiles(dir) {
  const files = [];
  async function fromDir(sub, kind) {
    const full = path.join(dir, sub);
    let entries = [];
    try { entries = await readdir(full); } catch { return; }
    for (const name of entries.sort()) {
      const p = path.join(full, name);
      if (!(await stat(p)).isFile()) continue;
      files.push({ kind, name, path: `${sub}/${name}`, code: await readFile(p, 'utf8') });
    }
  }
  await fromDir('starter', 'starter');
  await fromDir('solution', 'solution');
  await fromDir('tests', 'tests');
  // root-level extras: headers + Makefile (shared contract files)
  for (const name of (await readdir(dir)).sort()) {
    const p = path.join(dir, name);
    if (!(await stat(p)).isFile()) continue;
    if (name === 'README.md') continue;
    files.push({ kind: 'extra', name, path: name, code: await readFile(p, 'utf8') });
  }
  files.sort((a, b) => KIND_ORDER[a.kind] - KIND_ORDER[b.kind] || a.name.localeCompare(b.name));
  return files;
}

export async function buildDrills() {
  await mkdir(OUT, { recursive: true });
  const dirs = (await readdir(DRILLS)).filter((d) => /^chapter-\d{2}$/.test(d)).sort();
  const results = [];
  for (const d of dirs) {
    const num = Number(d.match(/chapter-(\d{2})/)[1]);
    const dir = path.join(DRILLS, d);
    const readme = await readFile(path.join(dir, 'README.md'), 'utf8');
    const h1 = readme.match(/^#\s+(.+)$/m)?.[1] ?? '';
    // "Chapter 12 — Title · Project: Name" or "… · Alias Workshop"
    const project = (h1.split('·').pop() ?? '').replace(/^\s*Project:\s*/i, '').trim() || `Chapter ${num} lab`;

    const sec = splitReadme(readme);
    const missing = SECTIONS.filter((s) => !(s in sec));
    if (missing.length) throw new Error(`drill chapter-${num}: README missing sections: ${missing.join(', ')}`);

    // chapters vary: numbered/bulleted lists vs prose (bold **Task N —** paragraphs,
    // criteria with code blocks). Lists render as the design's numbered/check rows;
    // prose falls back to a rendered block.
    const tasks = await listItemsHtml(sec['Your tasks']);
    const tasksHtml = tasks.length ? null : await renderMarkdown(sec['Your tasks']);
    const criteria = await listItemsHtml(sec['Success criteria']);
    const criteriaHtml = criteria.length ? null : await renderMarkdown(sec['Success criteria']);

    const rawFiles = await collectFiles(dir);
    if (!rawFiles.some((f) => f.kind === 'starter')) throw new Error(`drill chapter-${num}: no starter files`);
    const files = [];
    for (const f of rawFiles) {
      const lang = langFor(f.name);
      files.push({ ...f, lang, codeHtml: await highlightCode(f.code, lang) });
    }

    const json = {
      num,
      project,
      summaryHtml: await renderMarkdown(sec['The project']),
      tasks,
      tasksHtml,
      criteria,
      criteriaHtml,
      sections: {
        concepts: await renderMarkdown(sec['Concepts practiced']),
        constraints: await renderMarkdown(sec['Constraints']),
        buildRun: await renderMarkdown(sec['Build & run']),
        hints: await renderMarkdown(sec['Hints']),
        stretch: await renderMarkdown(sec['Stretch goals']),
      },
      files,
    };
    await writeFile(path.join(OUT, `${d}.json`), JSON.stringify(json));
    results.push({ num, files: files.length, tasks: tasks.length });
  }
  return results;
}

if (import.meta.url === `file://${process.argv[1]}`) {
  const r = await buildDrills();
  console.log(`drills: ${r.length} chapters`);
}
