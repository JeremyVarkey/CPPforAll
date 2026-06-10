/* CPPforAll — chapter learning interface (interactive prototype) */
const { useState, useEffect, useRef, useLayoutEffect, useCallback } = React;

/* ---------------- Icons (simple, stroke-based) ---------------- */
const I = {
  spark: (p) => (<svg viewBox="0 0 24 24" fill="currentColor" {...p}><path d="M12 2c.3 3.1 1.1 4.9 2.5 6.3C15.9 9.7 17.7 10.5 20.8 10.8c0 .3-3.1.7-4.5 2.1S14 16.9 13.7 20c-.3 0-.7-3.1-2.1-4.5S7.1 13.1 4 12.8c0-.3 3.1-.7 4.5-2.1S11.1 5.1 11.4 2c.2 0 .4 0 .6 0Z" transform="translate(.8 0)"/></svg>),
  sidebar: (p) => (<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="1.8" strokeLinecap="round" {...p}><rect x="3" y="4" width="18" height="16" rx="2.5"/><path d="M9 4v16"/></svg>),
  search: (p) => (<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="1.8" strokeLinecap="round" {...p}><circle cx="11" cy="11" r="7"/><path d="m20 20-3.2-3.2"/></svg>),
  x: (p) => (<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2" strokeLinecap="round" {...p}><path d="M6 6l12 12M18 6 6 18"/></svg>),
  chevR: (p) => (<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round" {...p}><path d="m9 6 6 6-6 6"/></svg>),
  book: (p) => (<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="1.8" strokeLinecap="round" strokeLinejoin="round" {...p}><path d="M4 5.5A2.5 2.5 0 0 1 6.5 3H20v15H6.5A2.5 2.5 0 0 0 4 20.5z"/><path d="M4 20.5A2.5 2.5 0 0 1 6.5 18H20"/></svg>),
  code: (p) => (<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="1.8" strokeLinecap="round" strokeLinejoin="round" {...p}><path d="m8 8-4 4 4 4M16 8l4 4-4 4"/></svg>),
  copy: (p) => (<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="1.7" strokeLinecap="round" strokeLinejoin="round" {...p}><rect x="9" y="9" width="11" height="11" rx="2"/><path d="M5 15V5a2 2 0 0 1 2-2h8"/></svg>),
  check: (p) => (<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round" {...p}><path d="m20 6-11 11-5-5"/></svg>),
  download: (p) => (<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="1.7" strokeLinecap="round" strokeLinejoin="round" {...p}><path d="M12 3v12m0 0 4-4m-4 4-4-4M4 19h16"/></svg>),
  play: (p) => (<svg viewBox="0 0 24 24" fill="currentColor" {...p}><path d="M7 5.5v13l11-6.5z"/></svg>),
  list: (p) => (<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="1.8" strokeLinecap="round" {...p}><path d="M8 6h12M8 12h12M8 18h12M3.5 6h.01M3.5 12h.01M3.5 18h.01"/></svg>),
  target: (p) => (<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="1.8" {...p}><circle cx="12" cy="12" r="8"/><circle cx="12" cy="12" r="3.4"/></svg>),
  bulb: (p) => (<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="1.8" strokeLinecap="round" strokeLinejoin="round" {...p}><path d="M9 18h6M10 21h4M12 3a6 6 0 0 0-3.5 10.9c.6.5.9 1.1.9 1.8V16h5.2v-.3c0-.7.3-1.3.9-1.8A6 6 0 0 0 12 3Z"/></svg>),
  alert: (p) => (<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="1.8" strokeLinecap="round" strokeLinejoin="round" {...p}><path d="M12 3 2.5 20h19z"/><path d="M12 10v4M12 17.5h.01"/></svg>),
  link: (p) => (<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="1.8" strokeLinecap="round" strokeLinejoin="round" {...p}><path d="M10 14a3.5 3.5 0 0 0 5 0l3-3a3.5 3.5 0 0 0-5-5l-1.5 1.5"/><path d="M14 10a3.5 3.5 0 0 0-5 0l-3 3a3.5 3.5 0 0 0 5 5L12.5 16.5"/></svg>),
  menu: (p) => (<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="1.9" strokeLinecap="round" {...p}><path d="M4 7h16M4 12h16M4 17h16"/></svg>),
};

const CODE = (s) => ({ __html: window.cppHighlight(s) });

/* ---------------- Code box ---------------- */
function CodeBox({ file, lang, code, radius }) {
  const [copied, setCopied] = useState(false);
  const doCopy = () => {
    navigator.clipboard?.writeText(code).then(() => {
      setCopied(true); setTimeout(() => setCopied(false), 1400);
    }).catch(() => {});
  };
  const doDownload = () => {
    const blob = new Blob([code], { type: "text/plain" });
    const url = URL.createObjectURL(blob);
    const a = document.createElement("a");
    a.href = url; a.download = file; a.click();
    setTimeout(() => URL.revokeObjectURL(url), 500);
  };
  return (
    <div className="codebox" style={radius ? { borderRadius: radius } : null}>
      <div className="codebox-head">
        <div className="cb-dots"><i></i><i></i><i></i></div>
        <span className="cb-file">{file}</span>
        {lang && <span className="cb-lang">{lang}</span>}
        <div className="cb-actions">
          <button className={"cb-btn" + (copied ? " ok" : "")} onClick={doCopy}>
            {copied ? <I.check/> : <I.copy/>}{copied ? "Copied" : "Copy"}
          </button>
          <button className="cb-btn" onClick={doDownload}><I.download/>Download</button>
        </div>
      </div>
      <div className="codebox-body">
        <pre dangerouslySetInnerHTML={CODE(code)}></pre>
      </div>
    </div>
  );
}

/* ---------------- Callouts ---------------- */
function Callout({ kind, title, html, onJump }) {
  const ico = kind === "warn" ? <I.alert/> : kind === "xref" ? <I.link/> : <I.bulb/>;
  const label = title || (kind === "warn" ? "Watch out" : kind === "xref" ? "Builds on" : "Tip");
  return (
    <div className={"callout " + kind} onClick={kind === "xref" ? onJump : undefined} role={kind === "xref" ? "button" : undefined}>
      <span className="ico">{ico}</span>
      <div className="body">
        <div className="ctitle">{label}</div>
        <p dangerouslySetInnerHTML={{ __html: html }}></p>
      </div>
    </div>
  );
}

/* ---------------- Notes reader ---------------- */
function Reader({ chapter, onJump }) {
  return (
    <div className="reader swap" key={chapter.n}>
      <div className="ch-kicker">{chapter.kicker}</div>
      <h1 className="ch-display">{chapter.title}</h1>
      <div className="ch-sub">
        <span>{chapter.readingTime}</span>
        <span className="dot"></span>
        <span>{chapter.tag}</span>
      </div>
      {chapter.notes.map((b, i) => {
        if (b.t === "h2") return <h2 key={i}>{b.text}</h2>;
        if (b.t === "lead") return <p key={i} className="lead" dangerouslySetInnerHTML={{ __html: b.html }}></p>;
        if (b.t === "p") return <p key={i} dangerouslySetInnerHTML={{ __html: b.html }}></p>;
        if (b.t === "list") return <ul key={i}>{b.items.map((it, j) => <li key={j} dangerouslySetInnerHTML={{ __html: it }}></li>)}</ul>;
        if (b.t === "code") return <div key={i} className="snippet"><CodeBox file={b.file} lang={b.lang === "cpp" ? "C++" : b.lang} code={b.code}/></div>;
        if (b.t === "tip") return <Callout key={i} kind="tip" title={b.title} html={b.html}/>;
        if (b.t === "warn") return <Callout key={i} kind="warn" title={b.title} html={b.html}/>;
        if (b.t === "xref") return <Callout key={i} kind="xref" html={b.html} onJump={() => onJump(b.to)}/>;
        return null;
      })}
    </div>
  );
}

/* ---------------- Code view (exercise) ---------------- */
function Exercise({ chapter }) {
  const ex = chapter.exercise;
  const [tab, setTab] = useState(0);
  useEffect(() => setTab(0), [chapter.n]);

  if (!ex) {
    return (
      <div className="exercise swap" key={chapter.n}>
        <div className="handout" style={{ textAlign: "center" }}>
          <div className="ex-kicker">Exercise</div>
          <h1>Coming soon</h1>
          <p className="ex-sum">The lab for <strong>{chapter.title}</strong> is being authored. Open <strong>Chapter 10 — Pointers &amp; References</strong> to try a complete exercise with starter, solution, and test files.</p>
        </div>
      </div>
    );
  }
  const f = ex.files[tab];
  return (
    <div className="exercise swap" key={chapter.n}>
      <div className="handout">
        <div className="ex-kicker">Exercise · {chapter.kicker}</div>
        <h1>{ex.title}</h1>
        <p className="ex-sum" dangerouslySetInnerHTML={{ __html: ex.summary }}></p>
        <div className="ex-cols">
          <div className="ex-block">
            <h3><I.list/> Your tasks</h3>
            <ol className="ex-tasks">
              {ex.tasks.map((t, i) => <li key={i} dangerouslySetInnerHTML={{ __html: t }}></li>)}
            </ol>
          </div>
          <div className="ex-block">
            <h3><I.target/> Success criteria</h3>
            <ul className="ex-crit">
              {ex.criteria.map((c, i) => <li key={i}><I.check/><span dangerouslySetInnerHTML={{ __html: c }}></span></li>)}
            </ul>
          </div>
        </div>
      </div>

      <div className="files-head">
        {ex.files.map((file, i) => (
          <button key={i} className={"file-tab" + (i === tab ? " on" : "")} onClick={() => setTab(i)}>
            <span className={"kindtag " + file.kind}>{file.kind}</span>
            {file.name}
          </button>
        ))}
      </div>
      <CodeBox key={f.name} file={f.name} lang={f.lang} code={f.code} radius="0 12px 12px 12px"/>

      {/* Future Run / Submit slot (Phase 2) */}
      <div className="run-slot">
        <div className="run-fake"><I.play/> Run</div>
        <div className="run-fake" style={{ background: "var(--success)" }}><I.check/> Submit</div>
        <div className="run-note"><b>Phase 2 — reserved</b>An in-browser editor &amp; results strip slot in here. No redesign needed.</div>
        <div className="results-strip"><i></i><i></i><i></i><i></i></div>
      </div>
    </div>
  );
}

/* ---------------- Segmented toggle ---------------- */
function Toggle({ view, setView }) {
  const wrap = useRef(null);
  const [thumb, setThumb] = useState({ left: 3, width: 0 });
  useLayoutEffect(() => {
    if (!wrap.current) return;
    const btns = wrap.current.querySelectorAll("button");
    const idx = view === "notes" ? 0 : 1;
    const b = btns[idx];
    if (b) setThumb({ left: b.offsetLeft, width: b.offsetWidth });
  }, [view]);
  return (
    <div className="seg" ref={wrap} role="tablist" aria-label="View">
      <div className="thumb" style={{ left: thumb.left, width: thumb.width }}></div>
      <button role="tab" aria-selected={view === "notes"} className={view === "notes" ? "on" : ""} onClick={() => setView("notes")}><I.book/> Notes</button>
      <button role="tab" aria-selected={view === "code"} className={view === "code" ? "on" : ""} onClick={() => setView("code")}><I.code/> Code</button>
    </div>
  );
}

/* ---------------- Chapter rail ---------------- */
function Rail({ chapters, selected, onSelect, query, setQuery, onCollapse, onHome }) {
  const inputRef = useRef(null);
  const filtered = chapters.filter((c) => {
    const q = query.trim().toLowerCase();
    if (!q) return true;
    return (c.title.toLowerCase().includes(q) || c.tag.toLowerCase().includes(q) || String(c.n) === q || ("chapter " + c.n).includes(q));
  });
  return (
    <div className="rail-inner">
      <div className="rail-head">
        <div className="brand">
          <button className="brand-mark" onClick={onHome} title="Home" aria-label="Home">
            <I.spark className="spark"/>
            <span className="brand-name">CPP<b>for</b>All</span>
          </button>
          <button className="icon-btn" onClick={onCollapse} aria-label="Collapse sidebar" title="Collapse sidebar"><I.sidebar/></button>
        </div>
        <div className="search">
          <I.search/>
          <input ref={inputRef} value={query} onChange={(e) => setQuery(e.target.value)} placeholder="Search chapters…" aria-label="Search chapters"/>
          {query && <button className="clear" onClick={() => { setQuery(""); inputRef.current?.focus(); }} aria-label="Clear search"><I.x/></button>}
        </div>
      </div>
      <div className="rail-label">{query ? `${filtered.length} result${filtered.length === 1 ? "" : "s"}` : "All chapters"}</div>
      <div className="chapters">
        {filtered.length === 0 && <div className="no-results">No chapters match “{query}”.</div>}
        {filtered.map((c) => (
          <button key={c.n} className={"ch-row" + (selected === c.n ? " active" : "")} onClick={() => onSelect(c.n)} aria-current={selected === c.n}>
            <span className="ch-num">{String(c.n).padStart(2, "0")}</span>
            <span className="ch-main">
              <span className="ch-title">{c.title}</span>
              <span className="ch-tag">{c.tag}</span>
            </span>
          </button>
        ))}
      </div>
      <div className="rail-foot">
        <span className="foot-meta">28 chapters · free &amp; open</span>
      </div>
    </div>
  );
}

/* ---------------- Tweaks ---------------- */
const TWEAK_DEFAULTS = /*EDITMODE-BEGIN*/{
  "dark": false,
  "accentHue": 16,
  "readingFont": "serif",
  "density": "regular",
  "railWidth": 300
}/*EDITMODE-END*/;

/* ---------------- App ---------------- */
function App() {
  const [t, setTweak] = useTweaks(TWEAK_DEFAULTS);
  const chapters = window.CPP_CHAPTERS;

  const [selected, setSelected] = useState(10);   // hero chapter open by default
  const [view, setView] = useState("notes");
  const [collapsed, setCollapsed] = useState(false);
  const [drawer, setDrawer] = useState(false);
  const [query, setQuery] = useState("");
  const scrollRef = useRef(null);

  const getChapter = useCallback((n) => {
    if (n === window.CPP_HERO.n) return window.CPP_HERO;
    const ch = chapters.find((c) => c.n === n);
    return window.CPP_STUB(ch);
  }, [chapters]);

  const chapter = selected ? getChapter(selected) : null;

  const select = (n) => {
    setSelected(n);
    setDrawer(false);
    if (scrollRef.current) scrollRef.current.scrollTop = 0;
  };
  const jump = (n) => { setView("notes"); select(n); };

  // density → gap scale
  const gapScale = t.density === "compact" ? 0.78 : t.density === "comfy" ? 1.25 : 1;

  const rootStyle = {
    "--accent-h": t.accentHue,
    "--rail-w": t.railWidth + "px",
    "--gap-scale": gapScale,
  };

  return (
    <div
      className={"app" + (collapsed ? " collapsed" : "") + (drawer ? " drawer-open" : "")}
      data-theme={t.dark ? "dark" : "light"}
      data-read={t.readingFont}
      style={rootStyle}
    >
      {/* collapsed strip (desktop) */}
      {collapsed && (
        <div className="strip">
          <button className="icon-btn" onClick={() => setCollapsed(false)} aria-label="Expand sidebar" title="Expand sidebar"><I.sidebar/></button>
          <I.spark className="spark" style={{ marginTop: 8 }}/>
          <button className="icon-btn" onClick={() => setCollapsed(false)} aria-label="Search" title="Search"><I.search/></button>
        </div>
      )}

      <div className="scrim" onClick={() => setDrawer(false)}></div>

      <aside className="rail" aria-label="Chapters">
        <Rail
          chapters={chapters} selected={selected} onSelect={select}
          query={query} setQuery={setQuery}
          onHome={() => { setSelected(null); setDrawer(false); }}
          onCollapse={() => { setCollapsed(true); setDrawer(false); }}
        />
      </aside>

      <main className="canvas">
        <div className="canvas-top">
          <button className="icon-btn hamb" onClick={() => setDrawer(true)} aria-label="Open chapters"><I.menu/></button>
          {collapsed && (
            <div className="expand-zone">
              <button className="icon-btn" onClick={() => setCollapsed(false)} aria-label="Expand sidebar" title="Expand sidebar"><I.chevR/></button>
            </div>
          )}
          {chapter && (
            <div className="crumb">Chapter {chapter.n} · <b>{chapter.title}</b></div>
          )}
          <div className="top-spacer"></div>
          {chapter && <Toggle view={view} setView={setView}/>}
        </div>

        <div className="scroll" ref={scrollRef}>
          {!chapter && (
            <div className="landing">
              <I.spark className="spark-lg"/>
              <h1>Learn C++ from scratch.</h1>
              <p>A calm, free place to read and practice — 28 chapters of notes and hands-on labs, right in your browser. Pick a chapter to begin.</p>
              <div className="start-tags">
                {[1, 7, 10, 14, 20, 24].map((n) => {
                  const c = chapters.find((x) => x.n === n);
                  return <button key={n} className="start-chip" onClick={() => select(n)}>{String(n).padStart(2, "0")} · {c.title}</button>;
                })}
              </div>
            </div>
          )}
          {chapter && view === "notes" && <Reader chapter={chapter} onJump={jump}/>}
          {chapter && view === "code" && <Exercise chapter={chapter}/>}
        </div>
      </main>

      {/* Tweaks */}
      <TweaksPanel>
        <TweakSection label="Theme"/>
        <TweakToggle label="Dark mode" value={t.dark} onChange={(v) => setTweak("dark", v)}/>
        <TweakSlider label="Accent hue" value={t.accentHue} min={0} max={360} unit="°" onChange={(v) => setTweak("accentHue", v)}/>
        <TweakSection label="Typography"/>
        <TweakRadio label="Reading font" value={t.readingFont} options={["serif", "sans"]} onChange={(v) => setTweak("readingFont", v)}/>
        <TweakRadio label="Density" value={t.density} options={["compact", "regular", "comfy"]} onChange={(v) => setTweak("density", v)}/>
        <TweakSection label="Layout"/>
        <TweakSlider label="Rail width" value={t.railWidth} min={240} max={360} unit="px" onChange={(v) => setTweak("railWidth", v)}/>
      </TweaksPanel>
    </div>
  );
}

ReactDOM.createRoot(document.getElementById("root")).render(<App/>);
