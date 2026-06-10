/**
 * markdown.mjs — the shared markdown → HTML engine for notes and drills.
 *
 * One pipeline so every page renders identically:
 *   remark-parse → remark-gfm → remark-rehype(+raw) → rehype-slug
 *   → blockquote classifier → Shiki code boxes → stringify
 *
 * Code blocks become the design's dark "codebox" (traffic dots, lang chip,
 * copy button) with a custom Shiki theme matching the design tokens exactly.
 */
import { unified } from 'unified';
import remarkParse from 'remark-parse';
import remarkGfm from 'remark-gfm';
import remarkRehype from 'remark-rehype';
import rehypeRaw from 'rehype-raw';
import rehypeSlug from 'rehype-slug';
import rehypeStringify from 'rehype-stringify';
import { visit } from 'unist-util-visit';
import { toHtml } from 'hast-util-to-html';
import { toString } from 'hast-util-to-string';
import { createHighlighter } from 'shiki';

/* ---------- Shiki theme: the design's warm-dark code palette ---------- */
const cppforallDark = {
  name: 'cppforall-dark',
  type: 'dark',
  colors: {
    'editor.background': '#262420',
    'editor.foreground': '#e9e5d9',
  },
  tokenColors: [
    { scope: ['comment', 'punctuation.definition.comment'], settings: { foreground: '#84826f', fontStyle: 'italic' } },
    { scope: ['keyword', 'storage.type', 'storage.modifier', 'keyword.control', 'keyword.operator.new', 'keyword.operator.delete'], settings: { foreground: '#e2a07f' } },
    { scope: ['entity.name.type', 'support.type', 'entity.name.namespace', 'entity.name.class', 'entity.name.scope-resolution'], settings: { foreground: '#a6c178' } },
    { scope: ['string', 'punctuation.definition.string'], settings: { foreground: '#d8b878' } },
    { scope: ['constant.numeric', 'constant.language', 'constant.character'], settings: { foreground: '#d59a86' } },
    { scope: ['entity.name.function', 'support.function', 'meta.function-call.cpp entity.name.function'], settings: { foreground: '#88b0d6' } },
    { scope: ['meta.preprocessor', 'keyword.control.directive', 'punctuation.definition.directive', 'string.quoted.other.lt-gt.include'], settings: { foreground: '#c8a16a' } },
    { scope: ['punctuation', 'meta.brace'], settings: { foreground: '#a8a596' } },
    { scope: ['variable', 'variable.other'], settings: { foreground: '#e9e5d9' } },
  ],
};

let highlighterPromise = null;
export function getHighlighter() {
  highlighterPromise ??= createHighlighter({
    themes: [cppforallDark],
    langs: ['cpp', 'c', 'make', 'shellscript', 'diff'],
  });
  return highlighterPromise;
}

const LANG_ALIASES = {
  'c++': 'cpp', cpp: 'cpp', c: 'c', h: 'cpp', hpp: 'cpp',
  make: 'make', makefile: 'make', mk: 'make',
  sh: 'shellscript', bash: 'shellscript', shell: 'shellscript', zsh: 'shellscript',
  diff: 'diff',
};

/** Highlight raw code → inner HTML for a <pre> (or escaped plain text). */
export async function highlightCode(code, lang) {
  const norm = LANG_ALIASES[(lang ?? '').toLowerCase()] ?? null;
  const hl = await getHighlighter();
  if (norm) {
    const html = hl.codeToHtml(code, { lang: norm, theme: 'cppforall-dark' });
    // shiki emits <pre class="shiki" style…><code>…</code></pre>; keep just the <code>
    const inner = html.replace(/^<pre[^>]*>/, '').replace(/<\/pre>\s*$/, '');
    return inner;
  }
  return toHtml({ type: 'text', value: code });
}

/** The design's codebox wrapper, as a hast `raw` node. */
export function codeboxHtml({ file = null, lang = null, codeHtml }) {
  const chip = lang ? `<span class="cb-lang">${lang}</span>` : '';
  const fname = file ? `<span class="cb-file">${file}</span>` : '';
  return `<div class="codebox"><div class="codebox-head"><div class="cb-dots"><i></i><i></i><i></i></div>${fname}${chip}<div class="cb-actions"><button class="cb-btn" data-cb="copy" aria-label="Copy code"><svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.7" stroke-linecap="round" stroke-linejoin="round" aria-hidden="true"><rect x="9" y="9" width="11" height="11" rx="2"/><path d="M5 15V5a2 2 0 0 1 2-2h8"/></svg><span>Copy</span></button></div></div><div class="codebox-body"><pre>${codeHtml}</pre></div></div>`;
}

const CALLOUT_LANG_LABELS = { cpp: 'C++', c: 'C', make: 'Makefile', shellscript: 'shell', diff: 'diff' };

/** rehype plugin: fenced code → design codebox (async: highlights with Shiki). */
function rehypeCodeboxes() {
  return async (tree) => {
    const jobs = [];
    visit(tree, 'element', (node, index, parent) => {
      if (node.tagName !== 'pre' || !parent || index === null) return;
      const codeEl = node.children?.find((c) => c.type === 'element' && c.tagName === 'code');
      if (!codeEl) return;
      const cls = (codeEl.properties?.className ?? []).join(' ');
      const lang = cls.match(/language-([\w+#-]+)/)?.[1] ?? null;
      const raw = toString(codeEl).replace(/\n$/, '');
      jobs.push(
        (async () => {
          const norm = LANG_ALIASES[(lang ?? '').toLowerCase()] ?? null;
          const codeHtml = await highlightCode(raw, lang);
          const label = norm ? CALLOUT_LANG_LABELS[norm] ?? norm : null;
          parent.children[index] = {
            type: 'raw',
            value: `<div class="snippet">${codeboxHtml({ lang: label, codeHtml })}</div>`,
          };
        })(),
      );
    });
    await Promise.all(jobs);
  };
}

/* LearnCpp-style labelled blockquotes → design callouts */
const WARN_RE = /^warning/i;
const TIP_LABELS = /^(best practice|tip|key insight|key principle|universal rule|memory aid|rule|nomenclature|reminder)/i;

function rehypeCallouts() {
  return (tree) => {
    visit(tree, 'element', (node, index, parent) => {
      if (node.tagName !== 'blockquote' || !parent || index === null) return;
      // find the first <p> and check for a leading <strong>Label:</strong>
      const p = node.children.find((c) => c.type === 'element' && c.tagName === 'p');
      if (!p) return;
      const first = p.children?.[0];
      const isStrong = first?.type === 'element' && first.tagName === 'strong';
      if (!isStrong) return;
      const labelText = toString(first).replace(/:\s*$/, '').trim();
      const endsWithColon = /:\s*$/.test(toString(first)) || (p.children[1]?.type === 'text' && /^:/.test(p.children[1].value));
      if (!endsWithColon) return;

      let kind = null;
      if (WARN_RE.test(labelText)) kind = 'warn';
      else if (TIP_LABELS.test(labelText)) kind = 'tip';
      else kind = 'tip'; // Note / Author's note / Advanced etc — tip-styled, own label
      const icon = kind === 'warn'
        ? '<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round" stroke-linejoin="round" aria-hidden="true"><path d="M12 3 2.5 20h19z"/><path d="M12 10v4M12 17.5h.01"/></svg>'
        : '<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round" stroke-linejoin="round" aria-hidden="true"><path d="M9 18h6M10 21h4M12 3a6 6 0 0 0-3.5 10.9c.6.5.9 1.1.9 1.8V16h5.2v-.3c0-.7.3-1.3.9-1.8A6 6 0 0 0 12 3Z"/></svg>';

      // strip the label (and a following ":" / whitespace) from the first paragraph
      const rest = { ...p, children: p.children.slice(1) };
      if (rest.children[0]?.type === 'text') {
        rest.children[0] = { ...rest.children[0], value: rest.children[0].value.replace(/^:?\s*/, '') };
      }
      const bodyChildren = node.children.map((c) => (c === p ? rest : c));
      const bodyHtml = toHtml({ type: 'root', children: bodyChildren });
      parent.children[index] = {
        type: 'raw',
        value: `<div class="callout ${kind}"><span class="ico">${icon}</span><div class="body"><div class="ctitle">${labelText}</div>${bodyHtml}</div></div>`,
      };
    });
  };
}

/** Full-document processor (hast tree out, before stringify). */
export function createProcessor() {
  return unified()
    .use(remarkParse)
    .use(remarkGfm)
    .use(remarkRehype, { allowDangerousHtml: true })
    .use(rehypeRaw)
    .use(rehypeSlug)
    .use(rehypeCallouts)
    .use(rehypeCodeboxes);
}

/** Render a markdown string to an HTML string (fragment). */
export async function renderMarkdown(md) {
  const proc = createProcessor();
  const tree = await proc.run(proc.parse(md));
  return toHtml(tree, { allowDangerousHtml: true });
}

/** Render to a hast tree (for structural splitting). */
export async function renderToTree(md) {
  const proc = createProcessor();
  return proc.run(proc.parse(md));
}

export { toHtml, toString };
