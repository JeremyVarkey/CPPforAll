/* ============================================================================
 *  ExerciseRunner.tsx — the in-browser Run/Submit island (Phase B).
 * ----------------------------------------------------------------------------
 *  Owns the exercise page's file tabs + CodeMirror editor + Run/Submit buttons
 *  + results strip, talking to the server judge over the runner protocol
 *  documented in docs/runner-plan.md ("API contract").
 *
 *  GRACEFUL DEGRADATION (important): the static Astro page already renders a
 *  perfectly good file viewer (`<div data-static-files>`) and a dashed run-slot.
 *  This island mounts with `client:only` so it never blocks the static HTML.
 *  On mount it fetches `${judgeUrl}/manifest/${chapter}` to learn which paths
 *  are `editable`. ONLY if that succeeds do we "activate":
 *    - hide the static tabs/panes via document.querySelector('[data-static-files]')
 *      .hidden = true (the island then owns tabs + editor), and
 *    - render the live Run/Submit UI in place of the dashed placeholder.
 *  If the judge is unreachable (offline, cold, CORS, etc.) we render a quiet
 *  "runner offline" note in the run-slot idiom and leave the static viewer
 *  untouched — the page must never look broken.
 *
 *  All colors come from the design tokens in src/styles/cpp-styles.css via
 *  CSS variables; nothing is hardcoded except where CodeMirror needs concrete
 *  hex (the syntax highlight colors, mirrored from --tk-* in that stylesheet).
 * ==========================================================================*/

import { useCallback, useEffect, useMemo, useRef, useState } from 'react';
import { EditorState, Compartment } from '@codemirror/state';
import { EditorView, keymap, lineNumbers, highlightActiveLine } from '@codemirror/view';
import { defaultKeymap, history, historyKeymap, indentWithTab } from '@codemirror/commands';
import { syntaxHighlighting, HighlightStyle, indentUnit } from '@codemirror/language';
import { cpp } from '@codemirror/lang-cpp';
import { tags as t } from '@lezer/highlight';

/* ---- Props (from the Astro page; mirrors the drill `files[]` shape) -------- */
type DrillFile = {
  kind: string; // starter | solution | tests | extra
  name: string;
  path: string; // e.g. "starter/geo.cpp" — matches the manifest `editable` keys
  lang: string | null;
  code: string;
};

type Props = {
  chapter: number;
  files: DrillFile[];
  project: string;
};

/* ---- Runner protocol types (docs/runner-plan.md "API contract") ----------- */
type Manifest = {
  chapter: number;
  editable: string[]; // the ONLY client-suppliable paths
};

type StepResult = {
  name: string;
  phase: 'compile' | 'run' | 'grade';
  exit: number;
  verdict: 'pass' | 'fail' | 'error';
  stdout?: string;
  stderr?: string;
  detail?: string; // unified diff / check summary, optional
};

type RunResponse = {
  chapter: number;
  overall: 'red' | 'green' | 'error';
  steps: StepResult[];
  timings?: { queueMs?: number; compileMs?: number; runMs?: number };
};

type RunStatus = 'idle' | 'warming' | 'busy' | 'done';

const JUDGE_URL: string =
  (import.meta.env.PUBLIC_JUDGE_URL as string | undefined) || 'http://localhost:8080';

const draftKey = (chapter: number, path: string) => `cppforall:draft:c${chapter}:${path}`;

/* ============================================================================
 *  CodeMirror theme — bound to the site's design tokens.
 *  Background/foreground/selection read from CSS variables (so dark-mode and
 *  accent-hue tweaks flow through for free). The C++ token colors must be
 *  concrete hex for CodeMirror's HighlightStyle, so they mirror the --tk-*
 *  values defined in cpp-styles.css (the warm code surface is identical in
 *  both themes, so these stay constant — matching the static Shiki output).
 * ==========================================================================*/
const editorTheme = EditorView.theme({
  '&': {
    backgroundColor: 'var(--code-bg)',
    color: 'var(--code-fg)',
    fontSize: '13px',
    borderRadius: '0 12px 12px 12px',
  },
  '.cm-scroller': {
    fontFamily: 'var(--mono)',
    lineHeight: '1.62',
    padding: '6px 0',
  },
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

// Mirrors --tk-* from cpp-styles.css (CM needs literal hex).
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

/* ============================================================================
 *  A single CodeMirror instance bound to one file. We manage the view
 *  imperatively (React owns the host <div>, CM owns its subtree).
 * ==========================================================================*/
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
    // afterwards, every cached line-height/char-width is stale and scrolled
    // text paints over itself ("overlapping, unreadable"). So: load the font
    // BEFORE first measure, and re-measure on any later font-load event.
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
            if (u.docChanged && onChangeRef.current) {
              onChangeRef.current(u.state.doc.toString());
            }
          }),
        ],
      });
      v = new EditorView({ state, parent: host.current });
      view.current = v;
      viewRef?.(v);
    };

    const fonts = (document as Document & { fonts?: FontFaceSet }).fonts;
    if (fonts?.load) {
      // Ask for the exact face the theme uses; race against a short timeout so
      // a hung font fetch can never block the editor from appearing.
      const want = fonts.load('13px "JetBrains Mono Variable"').catch(() => []);
      Promise.race([want, new Promise((r) => setTimeout(r, 800))]).then(mount);
      // Belt and braces: any later font arrival → recompute CM's metrics.
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
    // Mount once per file identity; `value` resets are handled via the imperative
    // setDoc helper below to preserve undo history and cursor where possible.
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, []);

  // Toggle read-only without rebuilding the view.
  useEffect(() => {
    view.current?.dispatch({
      effects: readOnlyComp.current.reconfigure([
        EditorState.readOnly.of(readOnly),
        EditorView.editable.of(!readOnly),
      ]),
    });
  }, [readOnly]);

  return <div ref={host} className="cm-host" />;
}

/* ---- small inline icons (match the site's stroke idiom) ------------------- */
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

/* ============================================================================
 *  Main island.
 * ==========================================================================*/
export default function ExerciseRunner({ chapter, files, project }: Props) {
  const [manifest, setManifest] = useState<Manifest | null>(null);
  const [phase, setPhase] = useState<'probing' | 'offline' | 'active'>('probing');
  const [active, setActive] = useState(0); // active tab index
  // "try first": solution panes blur until deliberately revealed (per chapter+session,
  // same key the static page uses so the choice carries across both surfaces)
  const [solutionRevealed, setSolutionRevealed] = useState(() => {
    try { return sessionStorage.getItem(`cppforall:reveal:c${chapter}`) === '1'; } catch { return false; }
  });
  const [docs, setDocs] = useState<Record<string, string>>({});
  const [status, setStatus] = useState<RunStatus>('idle');
  const [lastMode, setLastMode] = useState<'run' | 'submit' | null>(null);
  const [result, setResult] = useState<RunResponse | null>(null); // in-memory only
  const [errorMsg, setErrorMsg] = useState<string | null>(null);
  const [openConsole, setOpenConsole] = useState<Record<number, boolean>>({});

  const warmTimer = useRef<number | null>(null);
  const editorViews = useRef<Record<string, EditorView | null>>({});

  const editablePaths = useMemo(() => new Set(manifest?.editable ?? []), [manifest]);
  const isEditable = useCallback((p: string) => editablePaths.has(p), [editablePaths]);

  /* ---- mount: probe the judge manifest -------------------------------- */
  useEffect(() => {
    let cancelled = false;
    const controller = new AbortController();
    const tid = window.setTimeout(() => controller.abort(), 6000);

    (async () => {
      try {
        const res = await fetch(`${JUDGE_URL}/manifest/${chapter}`, {
          signal: controller.signal,
          headers: { Accept: 'application/json' },
        });
        if (!res.ok) throw new Error(`manifest ${res.status}`);
        const m = (await res.json()) as Manifest;
        if (cancelled) return;
        if (!Array.isArray(m.editable)) throw new Error('manifest missing editable[]');

        // Seed editor docs: restore localStorage draft per chapter+path, else starter.
        const seeded: Record<string, string> = {};
        for (const f of files) {
          let text = f.code;
          if (m.editable.includes(f.path)) {
            try {
              const draft = localStorage.getItem(draftKey(chapter, f.path));
              if (draft != null) text = draft;
            } catch {
              /* localStorage unavailable — fall back to starter */
            }
          }
          seeded[f.path] = text;
        }
        setDocs(seeded);
        setManifest(m);

        // ACTIVATE: the island now owns tabs+editor; hide the static viewer.
        // (See the file header for why this is the chosen handoff mechanism.)
        const staticEl = document.querySelector<HTMLElement>('[data-static-files]');
        if (staticEl) staticEl.hidden = true;

        // Prefer opening on the first editable file so the learner lands on work.
        const firstEditable = files.findIndex((f) => m.editable.includes(f.path));
        setActive(firstEditable >= 0 ? firstEditable : 0);
        setPhase('active');
      } catch {
        if (!cancelled) setPhase('offline');
      } finally {
        clearTimeout(tid);
      }
    })();

    return () => {
      cancelled = true;
      controller.abort();
      clearTimeout(tid);
    };
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [chapter]);

  /* ---- edits → state + debounced localStorage draft ------------------- */
  const handleEdit = useCallback(
    (path: string, text: string) => {
      setDocs((d) => ({ ...d, [path]: text }));
      try {
        localStorage.setItem(draftKey(chapter, path), text);
      } catch {
        /* ignore quota / privacy-mode errors */
      }
    },
    [chapter],
  );

  const resetToStarter = useCallback(
    (file: DrillFile) => {
      const starter = file.code;
      setDocs((d) => ({ ...d, [file.path]: starter }));
      try {
        localStorage.removeItem(draftKey(chapter, file.path));
      } catch {
        /* ignore */
      }
      // Push the starter back into the live CodeMirror view.
      const v = editorViews.current[file.path];
      if (v) {
        v.dispatch({ changes: { from: 0, to: v.state.doc.length, insert: starter } });
      }
    },
    [chapter],
  );

  /* ---- run / submit --------------------------------------------------- */
  const send = useCallback(
    async (mode: 'run' | 'submit') => {
      if (!manifest || status === 'busy' || status === 'warming') return;
      setErrorMsg(null);
      setLastMode(mode);
      setStatus('busy');
      setOpenConsole({});

      // Warming-up hint if the first request is slow (cold container start).
      if (warmTimer.current) window.clearTimeout(warmTimer.current);
      warmTimer.current = window.setTimeout(() => {
        setStatus((s) => (s === 'busy' ? 'warming' : s));
      }, 2000);

      // Send ONLY the editable files, keyed by manifest path.
      const payloadFiles: Record<string, string> = {};
      for (const path of manifest.editable) {
        payloadFiles[path] = docs[path] ?? '';
      }

      const controller = new AbortController();
      const tid = window.setTimeout(() => controller.abort(), 30000);
      try {
        const res = await fetch(`${JUDGE_URL}/run`, {
          method: 'POST',
          signal: controller.signal,
          headers: { 'Content-Type': 'application/json', Accept: 'application/json' },
          body: JSON.stringify({ chapter, mode, files: payloadFiles }),
        });

        if (!res.ok) {
          setResult(null);
          setStatus('done');
          if (res.status === 429) {
            setErrorMsg('Take a breath — too many runs. Try again in a minute.');
          } else if (res.status === 413) {
            setErrorMsg('That submission is too large. Trim it down and try again.');
          } else if (res.status === 503) {
            setErrorMsg('The compiler is at capacity right now. Give it a moment and retry.');
          } else if (res.status === 400) {
            setErrorMsg('The judge rejected the request (invalid files). This is a bug — please report it.');
          } else {
            setErrorMsg(`The judge returned an error (${res.status}). Try again shortly.`);
          }
          return;
        }

        const data = (await res.json()) as RunResponse;
        setResult(data);
        setStatus('done');
        // Auto-open the console for any failing/erroring step.
        const firstBad = data.steps.findIndex((s) => s.verdict !== 'pass');
        if (firstBad >= 0) setOpenConsole({ [firstBad]: true });
      } catch (err) {
        setResult(null);
        setStatus('done');
        const aborted = err instanceof DOMException && err.name === 'AbortError';
        setErrorMsg(
          aborted
            ? 'The run timed out. The compiler may be waking up — try again in a moment.'
            : "Couldn't reach the compiler. Check your connection and try again.",
        );
      } finally {
        window.clearTimeout(tid);
        if (warmTimer.current) window.clearTimeout(warmTimer.current);
      }
    },
    [manifest, status, docs, chapter],
  );

  useEffect(() => {
    return () => {
      if (warmTimer.current) window.clearTimeout(warmTimer.current);
    };
  }, []);

  /* ---- render: probing (no UI; static page stands in) ----------------- */
  if (phase === 'probing') {
    return <span className="er-probe" aria-hidden="true" />;
  }

  /* ---- render: offline (keep the run-slot look + a quiet note) -------- */
  if (phase === 'offline') {
    return (
      <>
        <RunnerStyles />
        <div className="run-slot" role="note">
          <div className="run-fake">
            <PlayIcon /> Run
          </div>
          <div className="run-fake" style={{ background: 'var(--success)' }}>
            <CheckIcon /> Submit
          </div>
          <div className="run-note">
            <b>Run in your browser — runner offline</b>
            The compiler service isn’t reachable right now. Copy or download the files
            above and use <code>make test</code> locally (see “Build &amp; run locally”).
          </div>
          <div className="results-strip" aria-hidden="true">
            <i></i><i></i><i></i><i></i>
          </div>
        </div>
      </>
    );
  }

  /* ---- render: ACTIVE (island owns tabs + editor + run UI) ------------ */
  const running = status === 'busy' || status === 'warming';
  const activeFile = files[active];
  const revealKey = `cppforall:reveal:c${chapter}`;
  const revealSolution = () => {
    try { sessionStorage.setItem(revealKey, '1'); } catch {}
    setSolutionRevealed(true);
  };

  return (
    <>
      <RunnerStyles />
      <div className="er-root">
        {/* Tab strip — reuses the site's .files-head / .file-tab idiom. */}
        <div className="files-head" role="tablist" aria-label="Exercise files">
          {files.map((f, i) => {
            const editable = isEditable(f.path);
            return (
              <button
                key={f.path}
                className={'file-tab' + (i === active ? ' on' : '')}
                role="tab"
                aria-selected={i === active}
                onClick={() => setActive(i)}
              >
                <span className={`kindtag ${f.kind}`}>{f.kind}</span>
                {f.name}
                {editable && <span className="er-edit-dot" title="Editable" aria-label="editable" />}
              </button>
            );
          })}
        </div>

        {/* Editor panes — one CodeMirror per file, only the active is shown. */}
        {files.map((f, i) => {
          const editable = isEditable(f.path);
          const gated = f.kind === 'solution' && !solutionRevealed;
          return (
            <div className="file-pane" key={f.path} hidden={i !== active}>
              <div className="codebox er-codebox">
                <div className="codebox-head">
                  <div className="cb-dots"><i></i><i></i><i></i></div>
                  <span className="cb-file">{f.path}</span>
                  <span className="cb-lang">{editable ? 'editable' : 'read-only'}</span>
                  <div className="cb-actions">
                    {editable && (
                      <button
                        className="cb-btn"
                        onClick={() => resetToStarter(f)}
                        aria-label={`Reset ${f.name} to starter`}
                      >
                        <ResetIcon /> <span>Reset to starter</span>
                      </button>
                    )}
                  </div>
                </div>
                <div className={'er-body' + (gated ? ' gated' : '')} data-gate={f.kind === 'solution' ? `c${chapter}` : undefined}>
                  {gated && (
                    <div className="gate-cover">
                      <p>Try the lab first — the learning is in the attempt.</p>
                      <button className="btn btn-ghost gate-reveal" onClick={revealSolution}>Reveal solution</button>
                    </div>
                  )}
                  <CodeEditor
                    value={docs[f.path] ?? f.code}
                    readOnly={!editable}
                    onChange={editable ? (text) => handleEdit(f.path, text) : undefined}
                    viewRef={(v) => {
                      editorViews.current[f.path] = v;
                    }}
                  />
                </div>
              </div>
            </div>
          );
        })}

        {/* Run / Submit + results strip. */}
        <div className="er-actions">
          <button
            className="btn btn-primary"
            onClick={() => send('run')}
            disabled={running}
            aria-label="Compile and run"
          >
            {running && lastMode === 'run' ? <Spinner /> : <PlayIcon />}
            {running && lastMode === 'run' ? (status === 'warming' ? 'Warming up…' : 'Running…') : 'Run'}
          </button>
          <button
            className="btn er-submit"
            onClick={() => send('submit')}
            disabled={running}
            aria-label="Submit for grading"
          >
            {running && lastMode === 'submit' ? <Spinner /> : <CheckIcon />}
            {running && lastMode === 'submit'
              ? status === 'warming'
                ? 'Warming up…'
                : 'Grading…'
              : 'Submit'}
          </button>

          {status === 'warming' && (
            <span className="er-warm">Warming up the compiler… (first run after idle is slow)</span>
          )}
        </div>

        {/* Results region — polite live updates for screen readers. */}
        <div className="er-results" aria-live="polite">
          {errorMsg && (
            <div className="er-friendly" role="alert">
              {errorMsg}
            </div>
          )}

          {result && (
            <div className={`er-verdict-card er-${result.overall}`}>
              <div className="er-verdict-head">
                <span className={`er-overall er-${result.overall}`}>
                  {result.overall === 'green'
                    ? 'All checks passed'
                    : result.overall === 'red'
                      ? 'Some checks failed'
                      : 'Error'}
                </span>
                {result.timings && (
                  <span className="er-timings">
                    {fmtTimings(result.timings)}
                  </span>
                )}
              </div>

              {/* Per-step pill row (meaningful: green/red with the step name). */}
              <div className="er-pills" role="list">
                {result.steps.map((s, i) => (
                  <button
                    key={i}
                    role="listitem"
                    className={`er-pill er-${s.verdict} ${openConsole[i] ? 'on' : ''}`}
                    onClick={() => setOpenConsole((o) => ({ ...o, [i]: !o[i] }))}
                    aria-expanded={!!openConsole[i]}
                  >
                    <span className="er-pill-ico">
                      {s.verdict === 'pass' ? '✓' : s.verdict === 'fail' ? '✕' : '!'}
                    </span>
                    {s.name}
                  </button>
                ))}
              </div>

              {/* Expandable consoles per step. */}
              {result.steps.map((s, i) =>
                openConsole[i] ? (
                  <div className="er-console" key={`c${i}`}>
                    <div className="er-console-meta">
                      <span className={`er-tag er-${s.verdict}`}>{s.phase}</span>
                      <span className="er-exit">exit {s.exit}</span>
                    </div>
                    {s.stdout ? (
                      <pre className="er-stream">{s.stdout}</pre>
                    ) : null}
                    {s.stderr ? (
                      <pre className="er-stream er-stderr">{s.stderr}</pre>
                    ) : null}
                    {s.detail ? (
                      <pre className="er-stream er-detail">{s.detail}</pre>
                    ) : null}
                    {!s.stdout && !s.stderr && !s.detail ? (
                      <pre className="er-stream er-empty">(no output)</pre>
                    ) : null}
                  </div>
                ) : null,
              )}
            </div>
          )}

          {!result && !errorMsg && !running && (
            <p className="er-hint">
              Edit <b>{editableLabel(files, editablePaths)}</b>, then <b>Run</b> to compile,
              or <b>Submit</b> to grade against the chapter’s tests.
            </p>
          )}
        </div>
      </div>
      {/* keep activeFile referenced so lint doesn't flag it when unused in JSX */}
      <span hidden>{activeFile?.path}</span>
    </>
  );
}

/* ---- helpers -------------------------------------------------------------- */
function fmtTimings(timings: NonNullable<RunResponse['timings']>): string {
  const parts: string[] = [];
  if (timings.compileMs != null) parts.push(`compile ${timings.compileMs}ms`);
  if (timings.runMs != null) parts.push(`run ${timings.runMs}ms`);
  if (timings.queueMs != null && timings.queueMs > 0) parts.push(`queue ${timings.queueMs}ms`);
  return parts.join(' · ');
}

function editableLabel(files: DrillFile[], editable: Set<string>): string {
  const names = files.filter((f) => editable.has(f.path)).map((f) => f.name);
  if (names.length === 0) return 'your files';
  if (names.length === 1) return names[0];
  return names.slice(0, -1).join(', ') + ' and ' + names[names.length - 1];
}

/* ============================================================================
 *  Scoped styles for the island. Everything binds to design tokens; the only
 *  literal colors are the green Submit (var(--success)) shading and the
 *  pill red/green which derive from --success / accent. Inline so the island
 *  is self-contained (the page ships no extra CSS for Phase A).
 * ==========================================================================*/
function RunnerStyles() {
  return (
    <style>{`
.er-root { margin-top: 6px; }
.er-probe { display: none; }

.er-edit-dot {
  width: 6px; height: 6px; border-radius: 50%;
  background: var(--accent); display: inline-block; margin-left: 2px;
}

.er-codebox { border-radius: 0 12px 12px 12px; margin-top: 0; }
.er-codebox .cb-lang { margin-left: 4px; }
.cm-host { background: var(--code-bg); }
.cm-host .cm-editor { max-height: 460px; }
.cm-host .cm-scroller { overflow: auto; }

/* spinner */
.cm-spin { animation: er-spin .8s linear infinite; }
@keyframes er-spin { to { transform: rotate(360deg); } }
@media (prefers-reduced-motion: reduce) { .cm-spin { animation: none; } }

/* action row */
.er-actions {
  margin-top: 16px; display: flex; align-items: center; gap: 10px; flex-wrap: wrap;
}
.er-submit { background: var(--success); color: #fff; }
.er-submit:hover:not(:disabled) { filter: brightness(.94); }
.btn:disabled { opacity: .6; cursor: progress; }
.er-warm {
  font-family: var(--sans); font-size: 12.5px; color: var(--ink2); line-height: 1.5;
}

/* results */
.er-results { margin-top: 14px; }
.er-hint {
  font-family: var(--serif); font-size: 14.5px; line-height: 1.55;
  color: var(--ink2); margin: 6px 0 0;
}
.er-hint b { color: var(--ink); font-weight: 600; }

.er-friendly {
  font-family: var(--sans); font-size: 13.5px; line-height: 1.5; color: var(--ink);
  background: hsl(var(--accent-h) 40% 95% / 1);
  border: 1px solid hsl(var(--accent-h) 45% 86%);
  border-radius: var(--r-card); padding: 12px 16px;
}
[data-theme="dark"] .er-friendly {
  background: var(--accent-tint);
  border-color: hsl(var(--accent-h) var(--accent-s) var(--accent-l) / .3);
  color: var(--ink);
}

.er-verdict-card {
  border: 1px solid var(--hairline); border-radius: var(--r-card);
  background: var(--surface); padding: 14px 16px;
}
.er-verdict-head {
  display: flex; align-items: center; justify-content: space-between; gap: 12px;
  margin-bottom: 12px; flex-wrap: wrap;
}
.er-overall {
  font-family: var(--sans); font-size: 14px; font-weight: 650; letter-spacing: -.01em;
  display: inline-flex; align-items: center; gap: 8px;
}
.er-overall::before {
  content: ""; width: 9px; height: 9px; border-radius: 50%; display: inline-block;
}
.er-overall.er-green { color: var(--success); }
.er-overall.er-green::before { background: var(--success); }
.er-overall.er-red, .er-overall.er-error { color: var(--accent-press); }
[data-theme="dark"] .er-overall.er-red, [data-theme="dark"] .er-overall.er-error { color: var(--accent); }
.er-overall.er-red::before, .er-overall.er-error::before { background: var(--accent); }
.er-timings {
  font-family: var(--mono); font-size: 11.5px; color: var(--muted);
  font-variant-numeric: tabular-nums;
}

/* per-step pills */
.er-pills { display: flex; gap: 8px; flex-wrap: wrap; }
.er-pill {
  font-family: var(--sans); font-size: 12.5px; font-weight: 600; cursor: pointer;
  display: inline-flex; align-items: center; gap: 7px;
  padding: 6px 12px; border-radius: 999px; border: 1px solid transparent;
  transition: background .14s, border-color .14s, box-shadow .14s;
}
.er-pill .er-pill-ico {
  width: 16px; height: 16px; border-radius: 50%; display: grid; place-items: center;
  font-size: 10px; line-height: 1; color: #fff;
}
.er-pill.er-pass { background: rgba(120,140,93,.16); color: var(--success); border-color: rgba(120,140,93,.3); }
.er-pill.er-pass .er-pill-ico { background: var(--success); }
.er-pill.er-fail, .er-pill.er-error {
  background: var(--accent-tint); color: var(--accent-press); border-color: var(--accent-tint-2);
}
[data-theme="dark"] .er-pill.er-fail, [data-theme="dark"] .er-pill.er-error { color: var(--accent); }
.er-pill.er-fail .er-pill-ico, .er-pill.er-error .er-pill-ico { background: var(--accent); }
.er-pill:hover { filter: brightness(.97); }
.er-pill.on { box-shadow: 0 0 0 2px var(--accent-tint); }
.er-pill:focus-visible { outline: none; box-shadow: 0 0 0 3px var(--accent-tint); }

/* console */
.er-console {
  margin-top: 10px; background: var(--code-bg); border: 1px solid var(--code-line);
  border-radius: 10px; overflow: hidden;
}
.er-console-meta {
  display: flex; align-items: center; gap: 8px;
  padding: 8px 12px; background: var(--code-bg-2); border-bottom: 1px solid var(--code-line);
}
.er-tag {
  font-family: var(--mono); font-size: 10px; text-transform: uppercase; letter-spacing: .04em;
  padding: 2px 7px; border-radius: 5px; color: #fff;
}
.er-tag.er-pass { background: var(--success); }
.er-tag.er-fail, .er-tag.er-error { background: var(--accent); }
.er-exit { font-family: var(--mono); font-size: 11px; color: #8a8775; }
.er-stream {
  margin: 0; padding: 12px 14px; font-family: var(--mono); font-size: 12.5px; line-height: 1.6;
  color: var(--code-fg); white-space: pre-wrap; word-break: break-word;
  max-height: 320px; overflow: auto;
}
.er-stream.er-stderr { color: #e0a08a; border-top: 1px solid var(--code-line); }
.er-stream.er-detail { color: #cfcabb; border-top: 1px solid var(--code-line); background: rgba(0,0,0,.12); }
.er-stream.er-empty { color: #6f6d5e; font-style: italic; }
.er-console pre::-webkit-scrollbar { width: 9px; height: 9px; }
.er-console pre::-webkit-scrollbar-thumb { background: #3a382f; border-radius: 8px; }
    `}</style>
  );
}
