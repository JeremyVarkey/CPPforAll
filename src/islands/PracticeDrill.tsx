/* ============================================================================
 *  PracticeDrill.tsx — one micro-drill card (R2 practice), client:visible.
 * ----------------------------------------------------------------------------
 *  Renders a SINGLE drill of one of three tiers:
 *    • predict — code shown read-only (dark codebox), radio choices, "Check";
 *      on answer → correct/incorrect state + explanation callout. Records
 *      completion via window.cppProgress.markPractice() ONLY when correct.
 *    • fix / write — a CodeMirror editor seeded with the drill's `code`, a Run
 *      button that POSTs to the judge in PRACTICE mode, per-step results +
 *      console; on green → success state + explanation + markPractice().
 *
 *  The drill prop is ALREADY client-safe: build-practice.mjs strips `harness`
 *  and `solution`, so neither this component nor its props ever see them. The
 *  harness lives only on the judge image; the server compiles student.cpp + the
 *  baked harness and never serializes either back.
 *
 *  CodeMirror setup (theme + the font-load gating before first measure) is
 *  copied from ExerciseRunner.tsx — that gating fixed a real production bug
 *  (stale font metrics → overlapping text). Judge URL + offline behavior mirror
 *  the runner island too.
 * ==========================================================================*/

import { useCallback, useEffect, useRef, useState } from 'react';
import { EditorState, Compartment } from '@codemirror/state';
import { EditorView, keymap, lineNumbers, highlightActiveLine } from '@codemirror/view';
import { defaultKeymap, history, historyKeymap, indentWithTab } from '@codemirror/commands';
import { syntaxHighlighting, HighlightStyle, indentUnit } from '@codemirror/language';
import { cpp } from '@codemirror/lang-cpp';
import { tags as t } from '@lezer/highlight';

/* ---- Props ---------------------------------------------------------------- */
type Tier = 'predict' | 'fix' | 'write';
export type ClientDrill = {
  id: string;
  lesson: string;
  tier: Tier;
  title: string;
  promptMd: string;
  code: string;
  choices?: string[];
  answer?: number;
  explanationMd?: string;
};
type Props = { chapter: number; drill: ClientDrill };

/* ---- Runner protocol (shared with the judge / ExerciseRunner) ------------- */
type StepResult = {
  name: string;
  phase: 'compile' | 'run' | 'grade';
  exit: number | null;
  verdict: 'pass' | 'fail' | 'error';
  stdout?: string;
  stderr?: string;
  detail?: string;
};
type RunResponse = {
  overall: 'red' | 'green' | 'error';
  steps: StepResult[];
  timings?: { queueMs?: number; compileMs?: number; runMs?: number };
};
type RunStatus = 'idle' | 'busy' | 'warming' | 'done';

const JUDGE_URL: string =
  (import.meta.env.PUBLIC_JUDGE_URL as string | undefined) || 'http://localhost:8080';

const draftKey = (chapter: number, id: string) => `cppforall:practice-draft:c${chapter}:${id}`;

/* ---- progress bridge (Agent A ships window.cppProgress concurrently) ------ */
function markPractice(slug: string, drillId: string) {
  try {
    (window as unknown as { cppProgress?: { markPractice?: (s: string, d: string) => void } })
      .cppProgress?.markPractice?.(slug, drillId);
  } catch {
    /* progress bridge not mounted yet — non-fatal */
  }
}
function isDoneInProgress(slug: string, drillId: string): boolean {
  try {
    const p = (window as unknown as {
      cppProgress?: { getProgress?: () => { chapters?: Record<string, { practice?: Record<string, number> }> } };
    }).cppProgress;
    const prog = p?.getProgress?.();
    return Boolean(prog?.chapters?.[slug]?.practice?.[drillId]);
  } catch {
    return false;
  }
}
const slugOf = (chapter: number) => String(chapter).padStart(2, '0');

/* ============================================================================
 *  CodeMirror theme — copied from ExerciseRunner (token-bound; CM needs literal
 *  hex for the syntax colors, mirrored from --tk-* in cpp-styles.css).
 * ==========================================================================*/
const editorTheme = EditorView.theme({
  '&': {
    backgroundColor: 'var(--code-bg)',
    color: 'var(--code-fg)',
    fontSize: '13px',
    borderRadius: '0 0 12px 12px',
  },
  '.cm-scroller': { fontFamily: 'var(--mono)', lineHeight: '1.62', padding: '6px 0' },
  '.cm-content': { caretColor: 'var(--accent)' },
  '&.cm-focused': { outline: 'none' },
  '.cm-cursor, .cm-dropCursor': { borderLeftColor: 'var(--accent)' },
  '&.cm-focused .cm-selectionBackground, .cm-selectionBackground, ::selection': {
    backgroundColor: 'var(--accent-tint-2)',
  },
  '.cm-gutters': {
    backgroundColor: 'var(--code-bg-2)',
    color: '#6f6d5e',
    border: 'none',
    borderRight: '1px solid var(--code-line)',
  },
  '.cm-activeLine': { backgroundColor: 'rgba(255,255,255,.025)' },
  '.cm-activeLineGutter': { backgroundColor: 'transparent', color: '#a8a596' },
  '.cm-lineNumbers .cm-gutterElement': { padding: '0 12px 0 14px' },
});

const cppHighlight = HighlightStyle.define([
  { tag: [t.lineComment, t.blockComment, t.comment], color: '#84826f', fontStyle: 'italic' },
  { tag: [t.keyword, t.modifier, t.controlKeyword, t.operatorKeyword], color: '#e2a07f' },
  { tag: [t.typeName, t.namespace, t.className, t.standard(t.typeName)], color: '#a6c178' },
  { tag: [t.string, t.special(t.string), t.character], color: '#d8b878' },
  { tag: [t.number, t.bool, t.literal], color: '#d59a86' },
  { tag: [t.function(t.variableName), t.function(t.propertyName), t.macroName], color: '#88b0d6' },
  { tag: [t.processingInstruction, t.meta, t.annotation], color: '#c8a16a' },
  { tag: [t.punctuation, t.separator, t.bracket, t.operator, t.derefOperator], color: '#a8a596' },
  { tag: [t.variableName, t.propertyName], color: 'var(--code-fg)' as unknown as string },
]);

const baseExtensions = [
  lineNumbers(),
  highlightActiveLine(),
  history(),
  indentUnit.of('    '),
  cpp(),
  syntaxHighlighting(cppHighlight),
  editorTheme,
  keymap.of([...defaultKeymap, ...historyKeymap, indentWithTab]),
  EditorView.lineWrapping,
];

/* ---- CodeEditor — same font-gating-before-first-measure as the runner. ---- */
function CodeEditor({
  value,
  readOnly,
  onChange,
  viewRef,
}: {
  value: string;
  readOnly: boolean;
  onChange?: (text: string) => void;
  viewRef?: (view: EditorView | null) => void;
}) {
  const host = useRef<HTMLDivElement>(null);
  const view = useRef<EditorView | null>(null);
  const readOnlyComp = useRef(new Compartment());
  const onChangeRef = useRef(onChange);
  onChangeRef.current = onChange;

  useEffect(() => {
    if (!host.current) return;
    let cancelled = false;
    let v: EditorView | null = null;

    // CodeMirror caches font metrics at mount. If the mono webfont swaps in
    // afterwards, every cached line-height/char-width is stale and scrolled text
    // paints over itself. So: load the font BEFORE first measure, and re-measure
    // on any later font-load event. (Pattern copied from ExerciseRunner — it
    // fixed a real production bug.)
    const mount = () => {
      if (cancelled || !host.current) return;
      const state = EditorState.create({
        doc: value,
        extensions: [
          ...baseExtensions,
          readOnlyComp.current.of([
            EditorState.readOnly.of(readOnly),
            EditorView.editable.of(!readOnly),
          ]),
          EditorView.updateListener.of((u) => {
            if (u.docChanged && onChangeRef.current) onChangeRef.current(u.state.doc.toString());
          }),
        ],
      });
      v = new EditorView({ state, parent: host.current });
      view.current = v;
      viewRef?.(v);
    };

    const fonts = (document as Document & { fonts?: FontFaceSet }).fonts;
    if (fonts?.load) {
      const want = fonts.load('13px "JetBrains Mono Variable"').catch(() => []);
      Promise.race([want, new Promise((r) => setTimeout(r, 800))]).then(mount);
      const remeasure = () => view.current?.requestMeasure();
      fonts.addEventListener?.('loadingdone', remeasure);
      fonts.ready?.then(remeasure).catch(() => {});
      return () => {
        cancelled = true;
        fonts.removeEventListener?.('loadingdone', remeasure);
        viewRef?.(null);
        v?.destroy();
        view.current = null;
      };
    }

    mount();
    return () => {
      cancelled = true;
      viewRef?.(null);
      v?.destroy();
      view.current = null;
    };
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, []);

  useEffect(() => {
    view.current?.dispatch({
      effects: readOnlyComp.current.reconfigure([
        EditorState.readOnly.of(readOnly),
        EditorView.editable.of(!readOnly),
      ]),
    });
  }, [readOnly]);

  return <div ref={host} className="cm-host pd-cm" />;
}

/* ---- tiny inline icons (match the site stroke idiom) ---------------------- */
const PlayIcon = () => (
  <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="1.8" strokeLinecap="round" strokeLinejoin="round" aria-hidden="true">
    <polygon points="6 4 20 12 6 20 6 4" fill="currentColor" stroke="none" />
  </svg>
);
const CheckIcon = () => (
  <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round" aria-hidden="true">
    <path d="M20 6 9 17l-5-5" />
  </svg>
);
const ResetIcon = () => (
  <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="1.7" strokeLinecap="round" strokeLinejoin="round" aria-hidden="true">
    <path d="M3 12a9 9 0 1 0 3-6.7L3 8" /><path d="M3 3v5h5" />
  </svg>
);
const Spinner = () => (
  <svg className="cm-spin" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2.2" strokeLinecap="round" aria-hidden="true">
    <path d="M12 3a9 9 0 1 0 9 9" />
  </svg>
);

/* ---- very small inline-markdown renderer (bold + `code`) ------------------ */
/* Drill prompts/explanations are short, authored, trusted text. We render only
 * **bold** and `inline code`, escaping everything else — no HTML injection. */
function renderInline(md: string): { __html: string } {
  const esc = (s: string) =>
    s.replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;');
  // protect code spans, then bold, then restore.
  const codes: string[] = [];
  let s = md.replace(/`([^`]+)`/g, (_, c) => {
    codes.push(`<code>${esc(c)}</code>`);
    return ` ${codes.length - 1} `;
  });
  s = esc(s);
  s = s.replace(/\*\*([^*]+)\*\*/g, '<strong>$1</strong>');
  s = s.replace(/ (\d+) /g, (_, i) => codes[Number(i)]);
  s = s.replace(/\n/g, '<br/>');
  return { __html: s };
}

/* ============================================================================
 *  The card.
 * ==========================================================================*/
export default function PracticeDrill({ chapter, drill }: Props) {
  const slug = slugOf(chapter);
  const [done, setDone] = useState(false);

  // Reflect already-completed state from the progress module on mount + soft-nav.
  useEffect(() => {
    const sync = () => setDone(isDoneInProgress(slug, drill.id));
    sync();
    document.addEventListener('astro:page-load', sync);
    return () => document.removeEventListener('astro:page-load', sync);
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, []);

  return (
    <div className={`pd-card pd-${drill.tier}${done ? ' pd-done' : ''}`}>
      <DrillStyles />
      <div className="pd-head">
        <span className={`kindtag pd-tier pd-${drill.tier}`}>{drill.tier}</span>
        <h3 className="pd-title">{drill.title}</h3>
        {done && (
          <span className="pd-done-check" title="Completed" aria-label="completed">
            <CheckIcon />
          </span>
        )}
      </div>
      <p className="pd-prompt" dangerouslySetInnerHTML={renderInline(drill.promptMd)} />

      {drill.tier === 'predict' ? (
        <PredictBody drill={drill} slug={slug} onComplete={() => setDone(true)} />
      ) : (
        <CodeBody chapter={chapter} drill={drill} slug={slug} onComplete={() => setDone(true)} />
      )}
    </div>
  );
}

/* ============================================================================
 *  Predict body — read-only code + radio choices + Check.
 * ==========================================================================*/
function PredictBody({
  drill,
  slug,
  onComplete,
}: {
  drill: ClientDrill;
  slug: string;
  onComplete: () => void;
}) {
  const [picked, setPicked] = useState<number | null>(null);
  const [checked, setChecked] = useState(false);
  const choices = drill.choices ?? [];
  const correctIdx = drill.answer ?? -1;
  const isCorrect = checked && picked === correctIdx;

  const check = () => {
    if (picked === null) return;
    setChecked(true);
    if (picked === correctIdx) {
      markPractice(slug, drill.id);
      onComplete();
    }
  };

  return (
    <div className="pd-body">
      {/* Read-only code in the standard dark codebox (no client-side compile). */}
      <div className="codebox pd-codebox">
        <div className="codebox-head">
          <div className="cb-dots"><i></i><i></i><i></i></div>
          <span className="cb-lang">C++</span>
        </div>
        <div className="codebox-body">
          <pre><code>{drill.code}</code></pre>
        </div>
      </div>

      <fieldset className="pd-choices" disabled={isCorrect}>
        <legend className="pd-sr">Choose the output</legend>
        {choices.map((c, i) => {
          const showCorrect = checked && i === correctIdx;
          const showWrong = checked && i === picked && i !== correctIdx;
          return (
            <label
              key={i}
              className={
                'pd-choice' +
                (picked === i ? ' on' : '') +
                (showCorrect ? ' correct' : '') +
                (showWrong ? ' wrong' : '')
              }
            >
              <input
                type="radio"
                name={`pd-${drill.id}`}
                checked={picked === i}
                onChange={() => { setPicked(i); setChecked(false); }}
              />
              <span className="pd-choice-text">{c}</span>
              {showCorrect && <span className="pd-mark pd-mark-ok" aria-hidden="true">✓</span>}
              {showWrong && <span className="pd-mark pd-mark-no" aria-hidden="true">✕</span>}
            </label>
          );
        })}
      </fieldset>

      <div className="pd-actions">
        {!isCorrect && (
          <button className="btn btn-primary pd-check" onClick={check} disabled={picked === null}>
            Check
          </button>
        )}
        {checked && (
          <span className={'pd-verdict ' + (isCorrect ? 'ok' : 'no')} role="status">
            {isCorrect ? 'Correct' : 'Not quite — try again'}
          </span>
        )}
      </div>

      {isCorrect && drill.explanationMd && (
        <div className="callout aid pd-explain">
          <div className="body">
            <div className="ctitle">Why</div>
            <p dangerouslySetInnerHTML={renderInline(drill.explanationMd)} />
          </div>
        </div>
      )}
    </div>
  );
}

/* ============================================================================
 *  Code body (fix / write) — CodeMirror editor + Run against the judge.
 * ==========================================================================*/
function CodeBody({
  chapter,
  drill,
  slug,
  onComplete,
}: {
  chapter: number;
  drill: ClientDrill;
  slug: string;
  onComplete: () => void;
}) {
  // Seed from a saved draft if present, else the starter `code`.
  const [doc, setDoc] = useState<string>(() => {
    try {
      const d = localStorage.getItem(draftKey(chapter, drill.id));
      if (d != null) return d;
    } catch { /* ignore */ }
    return drill.code;
  });
  const [status, setStatus] = useState<RunStatus>('idle');
  const [result, setResult] = useState<RunResponse | null>(null);
  const [errorMsg, setErrorMsg] = useState<string | null>(null);
  const [openConsole, setOpenConsole] = useState<Record<number, boolean>>({});
  const warmTimer = useRef<number | null>(null);
  const viewRef = useRef<EditorView | null>(null);

  const onEdit = useCallback((text: string) => {
    setDoc(text);
    try { localStorage.setItem(draftKey(chapter, drill.id), text); } catch { /* ignore */ }
  }, [chapter, drill.id]);

  const reset = useCallback(() => {
    setDoc(drill.code);
    try { localStorage.removeItem(draftKey(chapter, drill.id)); } catch { /* ignore */ }
    const v = viewRef.current;
    if (v) v.dispatch({ changes: { from: 0, to: v.state.doc.length, insert: drill.code } });
  }, [drill.code]);

  const run = useCallback(async () => {
    if (status === 'busy' || status === 'warming') return;
    setErrorMsg(null);
    setStatus('busy');
    setOpenConsole({});
    if (warmTimer.current) window.clearTimeout(warmTimer.current);
    warmTimer.current = window.setTimeout(() => {
      setStatus((s) => (s === 'busy' ? 'warming' : s));
    }, 2000);

    const controller = new AbortController();
    const tid = window.setTimeout(() => controller.abort(), 30000);
    try {
      const res = await fetch(`${JUDGE_URL}/run`, {
        method: 'POST',
        signal: controller.signal,
        headers: { 'Content-Type': 'application/json', Accept: 'application/json' },
        body: JSON.stringify({
          practice: { chapter, drill: drill.id },
          files: { 'student.cpp': doc },
        }),
      });
      if (!res.ok) {
        setResult(null);
        setStatus('done');
        if (res.status === 429) setErrorMsg('Take a breath — too many runs. Try again in a minute.');
        else if (res.status === 413) setErrorMsg('That submission is too large. Trim it down.');
        else if (res.status === 503) setErrorMsg('The compiler is at capacity right now. Retry shortly.');
        else if (res.status === 400 || res.status === 404) setErrorMsg('The judge rejected this drill. Please report it.');
        else setErrorMsg(`The judge returned an error (${res.status}). Try again shortly.`);
        return;
      }
      const data = (await res.json()) as RunResponse;
      setResult(data);
      setStatus('done');
      const firstBad = data.steps.findIndex((s) => s.verdict !== 'pass');
      if (firstBad >= 0) setOpenConsole({ [firstBad]: true });
      if (data.overall === 'green') {
        markPractice(slug, drill.id);
        onComplete();
      }
    } catch (err) {
      setResult(null);
      setStatus('done');
      const aborted = err instanceof DOMException && err.name === 'AbortError';
      setErrorMsg(
        aborted
          ? 'The run timed out — the compiler may be waking up. Try again in a moment.'
          : "Couldn't reach the compiler. Check your connection and try again.",
      );
    } finally {
      window.clearTimeout(tid);
      if (warmTimer.current) window.clearTimeout(warmTimer.current);
    }
  }, [status, doc, chapter, drill.id, slug, onComplete]);

  useEffect(() => () => { if (warmTimer.current) window.clearTimeout(warmTimer.current); }, []);

  const running = status === 'busy' || status === 'warming';

  return (
    <div className="pd-body">
      <div className="codebox pd-codebox pd-editbox">
        <div className="codebox-head">
          <div className="cb-dots"><i></i><i></i><i></i></div>
          <span className="cb-file">student.cpp</span>
          <span className="cb-lang">editable</span>
          <div className="cb-actions">
            <button className="cb-btn" onClick={reset} aria-label="Reset to starter">
              <ResetIcon /> <span>Reset</span>
            </button>
          </div>
        </div>
        <CodeEditor
          value={doc}
          readOnly={false}
          onChange={onEdit}
          viewRef={(v) => { viewRef.current = v; }}
        />
      </div>

      <div className="pd-actions">
        <button className="btn btn-primary pd-run" onClick={run} disabled={running} aria-label="Compile and run the tests">
          {running ? <Spinner /> : <PlayIcon />}
          {running ? (status === 'warming' ? 'Warming up…' : 'Running…') : 'Run'}
        </button>
        {status === 'warming' && (
          <span className="pd-warm">First run after idle is slow — warming the compiler…</span>
        )}
      </div>

      <div className="pd-results" aria-live="polite">
        {errorMsg && <div className="pd-friendly" role="alert">{errorMsg}</div>}

        {result && (
          <div className={`pd-verdict-card pd-${result.overall}`}>
            <div className="pd-verdict-head">
              <span className={`pd-overall pd-${result.overall}`}>
                {result.overall === 'green' ? 'Tests passed'
                  : result.overall === 'red' ? 'Tests failed'
                  : 'Error'}
              </span>
            </div>
            <div className="pd-pills" role="list">
              {result.steps.map((s, i) => (
                <button
                  key={i}
                  role="listitem"
                  className={`pd-pill pd-${s.verdict} ${openConsole[i] ? 'on' : ''}`}
                  onClick={() => setOpenConsole((o) => ({ ...o, [i]: !o[i] }))}
                  aria-expanded={!!openConsole[i]}
                >
                  <span className="pd-pill-ico">
                    {s.verdict === 'pass' ? '✓' : s.verdict === 'fail' ? '✕' : '!'}
                  </span>
                  {s.name}
                </button>
              ))}
            </div>
            {result.steps.map((s, i) =>
              openConsole[i] ? (
                <div className="pd-console" key={`c${i}`}>
                  <div className="pd-console-meta">
                    <span className={`pd-tag pd-${s.verdict}`}>{s.phase}</span>
                    <span className="pd-exit">exit {s.exit}</span>
                  </div>
                  {s.stdout ? <pre className="pd-stream">{s.stdout}</pre> : null}
                  {s.stderr ? <pre className="pd-stream pd-stderr">{s.stderr}</pre> : null}
                  {s.detail ? <pre className="pd-stream pd-detail">{s.detail}</pre> : null}
                  {!s.stdout && !s.stderr && !s.detail ? <pre className="pd-stream pd-empty">(no output)</pre> : null}
                </div>
              ) : null,
            )}
          </div>
        )}

        {result?.overall === 'green' && drill.explanationMd && (
          <div className="callout aid pd-explain">
            <div className="body">
              <div className="ctitle">Why</div>
              <p dangerouslySetInnerHTML={renderInline(drill.explanationMd)} />
            </div>
          </div>
        )}

        {!result && !errorMsg && !running && (
          <p className="pd-hint">Edit <b>student.cpp</b>, then <b>Run</b> to check it against the tests.</p>
        )}
      </div>
    </div>
  );
}

/* ============================================================================
 *  Scoped styles — token-bound; mirrors the runner island's visual idiom.
 * ==========================================================================*/
function DrillStyles() {
  return (
    <style>{`
.pd-card {
  border: 1px solid var(--hairline); border-radius: var(--r-card); background: var(--surface);
  padding: 16px 18px; transition: border-color .15s;
}
.pd-card.pd-done { border-color: rgba(120,140,93,.45); }
.pd-head { display: flex; align-items: center; gap: 10px; flex-wrap: wrap; }
.pd-tier.kindtag {
  font-family: var(--mono); font-size: 10px; text-transform: uppercase; letter-spacing: .04em;
  padding: 2px 7px; border-radius: 4px;
}
.pd-tier.pd-predict { background: rgba(106,155,204,.18); color: var(--info); }
.pd-tier.pd-fix { background: var(--accent-tint); color: var(--accent-press); }
[data-theme="dark"] .pd-tier.pd-fix { color: var(--accent); }
.pd-tier.pd-write { background: rgba(120,140,93,.18); color: var(--success); }
.pd-title {
  font-family: var(--sans); font-size: 16px; font-weight: 640; letter-spacing: -.01em;
  color: var(--ink); margin: 0; flex: 1 1 auto;
}
.pd-done-check {
  width: 20px; height: 20px; border-radius: 50%; background: var(--success); color: #fff;
  display: grid; place-items: center; flex: 0 0 auto;
}
.pd-done-check svg { width: 13px; height: 13px; }
.pd-prompt {
  font-family: var(--serif); font-size: 14.5px; line-height: 1.55; color: var(--ink2);
  margin: 8px 0 12px;
}
.pd-prompt code, .pd-explain code {
  font-family: var(--mono); font-size: .85em; background: var(--subtle);
  padding: 1px 5px; border-radius: 4px;
}
.pd-body { margin-top: 2px; }

.pd-codebox { margin: 0 0 12px; }
.pd-codebox .codebox-body pre { white-space: pre; }
.pd-editbox .codebox-head .cb-file { font-family: var(--mono); font-size: 12px; color: #c8c6ba; }
.cm-host.pd-cm { background: var(--code-bg); }
.cm-host.pd-cm .cm-editor { max-height: 360px; }
.cm-host.pd-cm .cm-scroller { overflow: auto; }

/* spinner */
.cm-spin { animation: pd-spin .8s linear infinite; }
@keyframes pd-spin { to { transform: rotate(360deg); } }
@media (prefers-reduced-motion: reduce) { .cm-spin { animation: none; } }

/* predict choices */
.pd-choices { border: 0; margin: 0; padding: 0; display: flex; flex-direction: column; gap: 8px; }
.pd-sr { position: absolute; width: 1px; height: 1px; padding: 0; margin: -1px; overflow: hidden; clip: rect(0,0,0,0); border: 0; }
.pd-choice {
  display: flex; align-items: center; gap: 10px; cursor: pointer;
  font-family: var(--mono); font-size: 13px; color: var(--ink);
  border: 1px solid var(--hairline); border-radius: 10px; padding: 9px 12px;
  background: var(--bg); transition: border-color .14s, background .14s;
}
.pd-choice:hover { border-color: var(--accent-tint-2); }
.pd-choice.on { border-color: var(--accent); }
.pd-choice input { accent-color: var(--accent); margin: 0; }
.pd-choice-text { white-space: pre-wrap; flex: 1 1 auto; }
.pd-choice.correct { border-color: var(--success); background: rgba(120,140,93,.1); }
.pd-choice.wrong { border-color: var(--accent); background: var(--accent-tint); }
.pd-mark { font-weight: 700; }
.pd-mark-ok { color: var(--success); }
.pd-mark-no { color: var(--accent); }

/* actions */
.pd-actions { margin-top: 12px; display: flex; align-items: center; gap: 12px; flex-wrap: wrap; }
.btn.pd-run, .btn.pd-check { display: inline-flex; align-items: center; gap: 8px; }
.btn:disabled { opacity: .6; cursor: progress; }
.pd-check:disabled { cursor: not-allowed; }
.pd-warm, .pd-hint { font-family: var(--sans); font-size: 12.5px; color: var(--ink2); line-height: 1.5; }
.pd-hint b { color: var(--ink); font-weight: 600; }
.pd-verdict { font-family: var(--sans); font-size: 13px; font-weight: 600; }
.pd-verdict.ok { color: var(--success); }
.pd-verdict.no { color: var(--accent-press); }
[data-theme="dark"] .pd-verdict.no { color: var(--accent); }

/* explanation callout (reuses .callout.aid but no icon column here) */
.pd-explain { margin-top: 14px; }
.pd-explain .body { min-width: 0; }
.pd-explain .ctitle { font-family: var(--sans); font-size: 13.5px; font-weight: 600; margin-bottom: 4px; }
.pd-explain p { font-family: var(--serif); font-size: 14.5px; line-height: 1.6; margin: 0; color: var(--ink2); }

/* results (mirrors the runner island) */
.pd-results { margin-top: 12px; }
.pd-friendly {
  font-family: var(--sans); font-size: 13px; line-height: 1.5; color: var(--ink);
  background: var(--accent-tint); border: 1px solid var(--accent-tint-2);
  border-radius: var(--r-card); padding: 10px 14px;
}
.pd-verdict-card { border: 1px solid var(--hairline); border-radius: var(--r-card); background: var(--bg); padding: 12px 14px; }
.pd-verdict-head { margin-bottom: 10px; }
.pd-overall { font-family: var(--sans); font-size: 13.5px; font-weight: 650; display: inline-flex; align-items: center; gap: 8px; }
.pd-overall::before { content: ""; width: 9px; height: 9px; border-radius: 50%; display: inline-block; }
.pd-overall.pd-green { color: var(--success); }
.pd-overall.pd-green::before { background: var(--success); }
.pd-overall.pd-red, .pd-overall.pd-error { color: var(--accent-press); }
[data-theme="dark"] .pd-overall.pd-red, [data-theme="dark"] .pd-overall.pd-error { color: var(--accent); }
.pd-overall.pd-red::before, .pd-overall.pd-error::before { background: var(--accent); }

.pd-pills { display: flex; gap: 8px; flex-wrap: wrap; }
.pd-pill {
  font-family: var(--sans); font-size: 12px; font-weight: 600; cursor: pointer;
  display: inline-flex; align-items: center; gap: 7px; padding: 5px 11px; border-radius: 999px;
  border: 1px solid transparent;
}
.pd-pill .pd-pill-ico { width: 15px; height: 15px; border-radius: 50%; display: grid; place-items: center; font-size: 9px; color: #fff; }
.pd-pill.pd-pass { background: rgba(120,140,93,.16); color: var(--success); border-color: rgba(120,140,93,.3); }
.pd-pill.pd-pass .pd-pill-ico { background: var(--success); }
.pd-pill.pd-fail, .pd-pill.pd-error { background: var(--accent-tint); color: var(--accent-press); border-color: var(--accent-tint-2); }
[data-theme="dark"] .pd-pill.pd-fail, [data-theme="dark"] .pd-pill.pd-error { color: var(--accent); }
.pd-pill.pd-fail .pd-pill-ico, .pd-pill.pd-error .pd-pill-ico { background: var(--accent); }
.pd-pill.on { box-shadow: 0 0 0 2px var(--accent-tint); }

.pd-console { margin-top: 10px; background: var(--code-bg); border: 1px solid var(--code-line); border-radius: 10px; overflow: hidden; }
.pd-console-meta { display: flex; align-items: center; gap: 8px; padding: 7px 12px; background: var(--code-bg-2); border-bottom: 1px solid var(--code-line); }
.pd-tag { font-family: var(--mono); font-size: 10px; text-transform: uppercase; letter-spacing: .04em; padding: 2px 7px; border-radius: 5px; color: #fff; }
.pd-tag.pd-pass { background: var(--success); }
.pd-tag.pd-fail, .pd-tag.pd-error { background: var(--accent); }
.pd-exit { font-family: var(--mono); font-size: 11px; color: #8a8775; }
.pd-stream { margin: 0; padding: 11px 14px; font-family: var(--mono); font-size: 12.5px; line-height: 1.6; color: var(--code-fg); white-space: pre-wrap; word-break: break-word; max-height: 280px; overflow: auto; }
.pd-stream.pd-stderr { color: #e0a08a; border-top: 1px solid var(--code-line); }
.pd-stream.pd-detail { color: #cfcabb; border-top: 1px solid var(--code-line); background: rgba(0,0,0,.12); }
.pd-stream.pd-empty { color: #6f6d5e; font-style: italic; }
    `}</style>
  );
}
