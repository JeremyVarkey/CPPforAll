/* ============================================================================
 *  progress.ts — R1 local-first progress store (schema v1).
 * ----------------------------------------------------------------------------
 *  The ONE module every write goes through. No accounts, no network: state
 *  lives in localStorage under a single versioned key. Every storage access is
 *  wrapped in try/catch (private mode, quota, disabled storage all degrade to
 *  a no-op / empty state rather than throwing). Corrupt JSON resets to empty.
 *
 *  Date keys are LOCAL time (`YYYY-MM-DD` from the visitor's own clock) so a
 *  "day" matches what the learner experienced, not UTC.
 *
 *  Importable from React islands AND usable from a tiny inline script: the
 *  thin Astro component ProgressBridge.astro mounts the public surface on
 *  `window.cppProgress`. Keep this file framework-free (no DOM assumptions
 *  beyond `localStorage`, guarded).
 * ==========================================================================*/

const STORAGE_KEY = 'cppforall:progress:v1';
const ACTIVITY_PRUNE_DAYS = 400;

/* ---- schema ---------------------------------------------------------------- */
export interface ChapterRecord {
  /** ms epoch when the chapter notes first reached ≥90% scroll. */
  read?: number;
  /** ms epoch when the runner first returned a `green` submit verdict. */
  lab?: number;
  /** drillId → ms epoch when that practice drill was first completed. */
  practice?: Record<string, number>;
}

export interface LastVisited {
  path: string;
  title: string;
  ts: number;
}

export interface ProgressV1 {
  chapters: Record<string, ChapterRecord>;
  /** `YYYY-MM-DD` (local) → 1, for any read/lab/practice on that day. */
  activity: Record<string, number>;
  lastVisited?: LastVisited;
}

export interface ChapterState {
  read: boolean;
  lab: boolean;
  practiceDone: number;
  practiceTotal?: number;
}

/* ---- internals ------------------------------------------------------------- */
function emptyState(): ProgressV1 {
  return { chapters: {}, activity: {} };
}

/** A 2-digit chapter key (`4` | `"4"` | `"04"` | `"04-foo"` → `"04"`). */
function normSlug(slug: string | number): string {
  const s = String(slug).trim();
  // pull the leading numeric run if present (handles "04", "4", "04--anchor")
  const m = s.match(/^\d+/);
  if (m) return m[0].padStart(2, '0');
  return s;
}

/** Local-time `YYYY-MM-DD` for a Date (default: now). */
function localDateKey(d: Date = new Date()): string {
  const y = d.getFullYear();
  const m = String(d.getMonth() + 1).padStart(2, '0');
  const day = String(d.getDate()).padStart(2, '0');
  return `${y}-${m}-${day}`;
}

/** Parse a `YYYY-MM-DD` key back to a local-midnight Date. */
function dateKeyToDate(key: string): Date | null {
  const m = key.match(/^(\d{4})-(\d{2})-(\d{2})$/);
  if (!m) return null;
  const d = new Date(Number(m[1]), Number(m[2]) - 1, Number(m[3]));
  return Number.isNaN(d.getTime()) ? null : d;
}

/** Whole-day difference a→b in local calendar days (b later → positive). */
function dayDiff(a: Date, b: Date): number {
  const am = new Date(a.getFullYear(), a.getMonth(), a.getDate()).getTime();
  const bm = new Date(b.getFullYear(), b.getMonth(), b.getDate()).getTime();
  return Math.round((bm - am) / 86_400_000);
}

/** Read + validate the persisted blob. Corrupt/absent → fresh empty state. */
function read(): ProgressV1 {
  let raw: string | null = null;
  try {
    raw = localStorage.getItem(STORAGE_KEY);
  } catch {
    return emptyState();
  }
  if (raw == null) return emptyState();
  try {
    const parsed = JSON.parse(raw) as unknown;
    if (!parsed || typeof parsed !== 'object') return emptyState();
    const p = parsed as Partial<ProgressV1>;
    const state: ProgressV1 = {
      chapters:
        p.chapters && typeof p.chapters === 'object' ? (p.chapters as ProgressV1['chapters']) : {},
      activity:
        p.activity && typeof p.activity === 'object' ? (p.activity as ProgressV1['activity']) : {},
    };
    if (p.lastVisited && typeof p.lastVisited === 'object') {
      const lv = p.lastVisited as Partial<LastVisited>;
      if (typeof lv.path === 'string' && typeof lv.ts === 'number') {
        state.lastVisited = {
          path: lv.path,
          title: typeof lv.title === 'string' ? lv.title : '',
          ts: lv.ts,
        };
      }
    }
    return state;
  } catch {
    // Corrupt JSON — reset to empty (and drop the bad blob best-effort).
    try {
      localStorage.removeItem(STORAGE_KEY);
    } catch {
      /* ignore */
    }
    return emptyState();
  }
}

/** Prune activity older than the retention window (mutates in place). */
function prune(state: ProgressV1): void {
  const today = new Date();
  for (const key of Object.keys(state.activity)) {
    const d = dateKeyToDate(key);
    if (!d || dayDiff(d, today) > ACTIVITY_PRUNE_DAYS) {
      delete state.activity[key];
    }
  }
}

function write(state: ProgressV1): void {
  try {
    localStorage.setItem(STORAGE_KEY, JSON.stringify(state));
  } catch {
    /* storage full / unavailable — silently degrade */
  }
}

/** Mark today's activity bit (local date). */
function touchActivity(state: ProgressV1): void {
  state.activity[localDateKey()] = 1;
}

function chapterRecord(state: ProgressV1, key: string): ChapterRecord {
  let rec = state.chapters[key];
  if (!rec || typeof rec !== 'object') {
    rec = {};
    state.chapters[key] = rec;
  }
  return rec;
}

/* ---- public API (exact names per spec) ------------------------------------ */

/** The full persisted progress object (always a valid shape; never throws). */
export function getProgress(): ProgressV1 {
  return read();
}

/** Record that the learner finished reading a chapter's notes. First time wins. */
export function markRead(slug: string | number, title?: string): void {
  const key = normSlug(slug);
  const state = read();
  const rec = chapterRecord(state, key);
  if (rec.read == null) rec.read = Date.now();
  touchActivity(state);
  prune(state);
  write(state);
}

/** Record that the chapter's lab reached a green submit verdict. First time wins. */
export function markLabGreen(slug: string | number): void {
  const key = normSlug(slug);
  const state = read();
  const rec = chapterRecord(state, key);
  if (rec.lab == null) rec.lab = Date.now();
  touchActivity(state);
  prune(state);
  write(state);
}

/** Record completion of a single practice drill within a chapter. First time wins. */
export function markPractice(slug: string | number, drillId: string): void {
  if (!drillId) return;
  const key = normSlug(slug);
  const state = read();
  const rec = chapterRecord(state, key);
  if (!rec.practice || typeof rec.practice !== 'object') rec.practice = {};
  if (rec.practice[drillId] == null) rec.practice[drillId] = Date.now();
  touchActivity(state);
  prune(state);
  write(state);
}

/**
 * Record the most recently visited chapter (for the landing "welcome back").
 * Does NOT touch the activity streak — visiting isn't "doing" work; only
 * read/lab/practice count toward a streak.
 */
export function touchVisit(slug: string | number, title: string, path: string): void {
  const state = read();
  state.lastVisited = { path, title: title || '', ts: Date.now() };
  prune(state);
  write(state);
}

/**
 * Consecutive-day streak ending today or yesterday.
 *  - 0 if there's no activity, or the most recent activity is older than yesterday.
 *  - Counts back day-by-day while each prior calendar day also has activity.
 */
export function getStreak(): number {
  const state = read();
  const keys = Object.keys(state.activity);
  if (keys.length === 0) return 0;

  const days = new Set(keys);
  const today = new Date();
  const todayKey = localDateKey(today);
  const yesterday = new Date(today);
  yesterday.setDate(yesterday.getDate() - 1);
  const yesterdayKey = localDateKey(yesterday);

  // Anchor: today if active today, else yesterday if active yesterday, else broken.
  let cursor: Date;
  if (days.has(todayKey)) cursor = today;
  else if (days.has(yesterdayKey)) cursor = yesterday;
  else return 0;

  let streak = 0;
  // Walk backwards while each consecutive calendar day is present.
  // Guard the loop with the retention window so a pathological store can't spin.
  for (let i = 0; i <= ACTIVITY_PRUNE_DAYS + 1; i++) {
    const k = localDateKey(cursor);
    if (days.has(k)) {
      streak++;
      cursor = new Date(cursor);
      cursor.setDate(cursor.getDate() - 1);
    } else {
      break;
    }
  }
  return streak;
}

/**
 * Compact state for one chapter's rail indicator / progress map / practice page.
 *  - `read`/`lab` are booleans derived from the timestamps.
 *  - `practiceDone` is how many distinct drills are done.
 *  - `practiceTotal` is left undefined here (the page that knows the drill
 *    count fills it in); callers may pass it through if they have it.
 */
export function chapterState(slug: string | number): ChapterState {
  const key = normSlug(slug);
  const state = read();
  const rec = state.chapters[key];
  const practiceDone =
    rec && rec.practice ? Object.keys(rec.practice).length : 0;
  return {
    read: !!(rec && rec.read != null),
    lab: !!(rec && rec.lab != null),
    practiceDone,
  };
}
