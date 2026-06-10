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
import { renderToTree, renderMarkdown, toHtml, toString } from './lib/markdown.mjs';

const ROOT = path.resolve(new URL('..', import.meta.url).pathname);
const NOTES = path.join(ROOT, 'content/notes');
const AUTHORED = path.join(ROOT, 'src/content-authored');
const OVERLAYS = path.join(ROOT, 'src/content-overlays');
const OUT = path.join(ROOT, 'src/generated/notes');

/**
 * Authored content (src/content-authored/chapter-NN.md) is the SITE's own
 * lecture-grade text, written by validated agents using the vendored notes
 * as guidance. When present it replaces the vendored chapter; the vendored
 * file remains the factual reference. Same format contract: H1 title,
 * `## NN.M — Title` lesson headings (anchors must stay stable).
 */
async function chapterSource(num, fileName) {
  try {
    const authored = await readFile(path.join(AUTHORED, fileName), 'utf8');
    return { md: authored, authored: true };
  } catch {
    return { md: await readFile(path.join(NOTES, fileName), 'utf8'), authored: false };
  }
}

/** Load a chapter's enrichment overlay (agent-authored, committed) if present. */
async function loadOverlay(num) {
  const p = path.join(OVERLAYS, `chapter-${String(num).padStart(2, '0')}.json`);
  try {
    return JSON.parse(await readFile(p, 'utf8'));
  } catch {
    return null;
  }
}

const AID_ICON = '<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round" stroke-linejoin="round" aria-hidden="true"><path d="M9 18h6M10 21h4M12 3a6 6 0 0 0-3.5 10.9c.6.5.9 1.1.9 1.8V16h5.2v-.3c0-.7.3-1.3.9-1.8A6 6 0 0 0 12 3Z"/></svg>';
const XREF_ICON = '<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round" stroke-linejoin="round" aria-hidden="true"><path d="M10 14a3.5 3.5 0 0 0 5 0l3-3a3.5 3.5 0 0 0-5-5l-1.5 1.5"/><path d="M14 10a3.5 3.5 0 0 0-5 0l-3 3a3.5 3.5 0 0 0 5 5L12.5 16.5"/></svg>';

async function aidHtml(aid) {
  const body = await renderMarkdown(aid.md);
  return `<div class="callout aid"><span class="ico">${AID_ICON}</span><div class="body"><div class="ctitle">${aid.title ?? 'Study aid'}</div>${body}</div></div>`;
}
async function xrefHtml(xref) {
  const body = await renderMarkdown(xref.md);
  const slug = String(xref.to).padStart(2, '0');
  return `<a class="callout xref" href="/chapters/${slug}/"><span class="ico">${XREF_ICON}</span><div class="body"><div class="ctitle">Builds on</div>${body}</div></a>`;
}

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
    const { md, authored } = await chapterSource(num, f);
    const h1 = md.match(/^#\s+(.+)$/m)?.[1] ?? `Chapter ${num}`;
    const title = h1.replace(/^Chapter\s+\d+\s*[—–-]\s*/i, '').trim();
    const words = md.split(/\s+/).length;
    const readingMinutes = Math.max(1, Math.round(words / 220));

    const tree = await renderToTree(md);
    const { children, source } = stripFrontMatter(tree, num);
    const { intro, lessons } = splitLessons(children);
    const overlay = await loadOverlay(num);

    const renderedLessons = [];
    for (const l of lessons) {
      let html = toHtml({ type: 'root', children: l.nodes }, { allowDangerousHtml: true });
      for (const aid of overlay?.aids ?? []) {
        if (aid.lesson === l.anchor) html += await aidHtml(aid);
      }
      for (const x of overlay?.xrefs ?? []) {
        if (x.lesson === l.anchor) html += await xrefHtml(x);
      }
      renderedLessons.push({ anchor: l.anchor, title: l.title, html });
    }

    let leadHtml = null;
    if (overlay?.lead) {
      leadHtml = (await renderMarkdown(overlay.lead))
        .replace(/^<p>/, '<p class="lead">');
    }

    const json = {
      num,
      title,
      source,
      authored,
      readingMinutes,
      leadHtml,
      introHtml: toHtml({ type: 'root', children: intro }, { allowDangerousHtml: true }),
      lessons: renderedLessons,
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
