/**
 * RailSearch — the one piece of the rail that hydrates. Filters the
 * server-rendered chapter rows in place (rows stay real <a> links).
 */
import { useEffect, useRef, useState } from 'react';

export default function RailSearch({ total }: { total: number }) {
  const [q, setQ] = useState('');
  const inputRef = useRef<HTMLInputElement>(null);

  useEffect(() => {
    const query = q.trim().toLowerCase();
    const rows = document.querySelectorAll<HTMLAnchorElement>('[data-ch-row]');
    let shown = 0;
    rows.forEach((row) => {
      const hit = !query || (row.dataset.search ?? '').includes(query);
      row.hidden = !hit;
      if (hit) shown++;
    });
    const label = document.getElementById('rail-label');
    if (label) {
      label.textContent = query ? `${shown} result${shown === 1 ? '' : 's'}` : 'All chapters';
    }
    const none = document.getElementById('no-results');
    if (none) {
      none.hidden = shown !== 0;
      none.textContent = `No chapters match “${q}”.`;
    }
  }, [q]);

  return (
    <div className="search">
      <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="1.8" strokeLinecap="round" aria-hidden="true">
        <circle cx="11" cy="11" r="7" />
        <path d="m20 20-3.2-3.2" />
      </svg>
      <input
        ref={inputRef}
        value={q}
        onChange={(e) => setQ(e.target.value)}
        placeholder="Search chapters…"
        aria-label={`Search ${total} chapters`}
      />
      {q && (
        <button
          className="clear"
          onClick={() => { setQ(''); inputRef.current?.focus(); }}
          aria-label="Clear search"
        >
          <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2" strokeLinecap="round" aria-hidden="true">
            <path d="M6 6l12 12M18 6 6 18" />
          </svg>
        </button>
      )}
    </div>
  );
}
