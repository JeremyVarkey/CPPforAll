/**
 * build-notes.mjs — content/notes/chapter-NN.md → src/generated/notes/chapter-NN.json
 *
 * The whole document renders through ONE pipeline pass (so heading slugs get
 * github-slugger's global dedup — anchors match the source TOC exactly), then
 * the hast tree is split at <h2> boundaries into lessons.
 *
 * Output: { num, title, source, readingMinutes, lessons:[{ anchor, title, html }], introHtml }
 */
import { readFile, readdir, mkdir, writeFile } from 'node:fs/promises';
import path from 'node:path';
import { renderToTree, toHtml, toString } from './lib/markdown.mjs';

const ROOT = path.resolve(new URL('..', import.meta.url).pathname);
const NOTES = path.join(ROOT, 'content/notes');
const OUT = path.join(ROOT, 'src/generated/notes');

function stripFrontMatter(tree, num) {
  // drop: the h1; the "> Source:" blockquote (capture its text); the
  // "## Contents" heading + its <ul>; any leading <hr>s between sections.
  let source = null;
  const out = [];
  let skipNextUl = false;
  for (const node of tree.children) {
    if (node.type === 'element' && node.tagName === 'h1') continue;
    if (skipNextUl) {
      skipNextUl = false;
      if (node.type === 'element' && node.tagName === 'ul') continue;
      // fall through if the TOC list was missing
    }
    if (node.type === 'element' && node.tagName === 'h2' && /^contents$/i.test(toString(node).trim())) {
      skipNextUl = true;
      continue;
    }
    if (node.type === 'element' && node.tagName === 'blockquote') {
      const text = toString(node).trim();
      if (text.startsWith('Source:')) {
        source = text.replace(/\s+/g, ' ');
        continue;
      }
    }
    // the classifier may have turned the source quote into raw — check raw too
    if (node.type === 'raw' && /Source:/.test(node.value) && /learncpp\.com/.test(node.value) && out.length === 0) {
      continue;
    }
    out.push(node);
  }
  return { children: out, source };
}

function splitLessons(children) {
  const intro = [];
  const lessons = [];
  let current = null;
  for (const node of children) {
    const isH2 = node.type === 'element' && node.tagName === 'h2';
    if (isH2) {
      current = { anchor: node.properties?.id ?? '', title: toString(node).trim(), nodes: [node] };
      lessons.push(current);
      continue;
    }
    // drop bare <hr> separators between lessons
    if (node.type === 'element' && node.tagName === 'hr') continue;
    if (current) current.nodes.push(node);
    else intro.push(node);
  }
  return { intro, lessons };
}

export async function buildNotes() {
  await mkdir(OUT, { recursive: true });
  const files = (await readdir(NOTES)).filter((f) => /^chapter-\d{2}\.md$/.test(f)).sort();
  const results = [];
  for (const f of files) {
    const num = Number(f.match(/chapter-(\d{2})/)[1]);
    const md = await readFile(path.join(NOTES, f), 'utf8');
    const h1 = md.match(/^#\s+(.+)$/m)?.[1] ?? `Chapter ${num}`;
    const title = h1.replace(/^Chapter\s+\d+\s*[—–-]\s*/i, '').trim();
    const words = md.split(/\s+/).length;
    const readingMinutes = Math.max(1, Math.round(words / 220));

    const tree = await renderToTree(md);
    const { children, source } = stripFrontMatter(tree, num);
    const { intro, lessons } = splitLessons(children);

    const json = {
      num,
      title,
      source,
      readingMinutes,
      introHtml: toHtml({ type: 'root', children: intro }, { allowDangerousHtml: true }),
      lessons: lessons.map((l) => ({
        anchor: l.anchor,
        title: l.title,
        html: toHtml({ type: 'root', children: l.nodes }, { allowDangerousHtml: true }),
      })),
    };
    if (json.lessons.length === 0) {
      throw new Error(`notes chapter-${num}: no lessons parsed`);
    }
    await writeFile(path.join(OUT, `chapter-${String(num).padStart(2, '0')}.json`), JSON.stringify(json));
    results.push({ num, lessons: json.lessons.length, readingMinutes });
  }
  return results;
}

if (import.meta.url === `file://${process.argv[1]}`) {
  const r = await buildNotes();
  console.log(`notes: ${r.length} chapters, ${r.reduce((s, x) => s + x.lessons, 0)} lessons`);
}
