// server.mjs — HTTP front of the CPPforAll judge.
//
// Endpoints (per docs/runner-plan.md "API contract"):
//   POST /run            — body ≤256KB JSON {chapter,mode,files}; queues a job.
//   GET  /healthz        — liveness/readiness probe.
//   GET  /manifest/:chapter — public subset of the manifest (editable list + project).
//
// Cross-cutting:
//   • CORS allow-list (https://cppforall.com, http://localhost:4321). No cookies.
//   • Per-IP token bucket: 10 req/min sustained, burst 20 → 429. Trusts ONLY the
//     first hop of x-forwarded-for (the ingress/proxy sets it; we don't trust the
//     rest of the chain, which a client could forge).
//   • In-process concurrency: ≤4 simultaneous jobs, queue cap 16 → 503.
//
// Uses Node 22 stdlib http only — no express. The work is I/O-light at the HTTP
// layer (the heavy lifting is the child compiler/program in runner.mjs), so the
// built-in server is the right tool and keeps the attack surface + image small.

import { createServer } from 'node:http';
import { readFile, readdir } from 'node:fs/promises';
import { join } from 'node:path';
import { runJob } from './runner.mjs';

// ── Config ───────────────────────────────────────────────────────────────────
const PORT = Number(process.env.PORT || 8080);
const MANIFESTS_DIR = process.env.MANIFESTS_DIR || '/app/manifests';
const MAX_BODY_BYTES = 256 * 1024; // 256KB → 413 over this
const MIN_CHAPTER = 1;
const MAX_CHAPTER = 28;

const ALLOWED_ORIGINS = (process.env.ALLOWED_ORIGINS
  || 'https://cppforall.com,http://localhost:4321')
  .split(',')
  .map((s) => s.trim())
  .filter(Boolean);

// Concurrency
const MAX_CONCURRENT = 4;
const QUEUE_CAP = 16;

// Rate limit (token bucket per IP)
const RL_REFILL_PER_MIN = 10; // sustained tokens/min
const RL_BURST = 20;          // bucket capacity

// ── Manifest cache ───────────────────────────────────────────────────────────
// Manifests are baked into the image and never change at runtime, so cache them.
const manifestCache = new Map();

async function loadManifest(chapter) {
  if (manifestCache.has(chapter)) return manifestCache.get(chapter);
  const file = join(MANIFESTS_DIR, `chapter-${String(chapter).padStart(2, '0')}.json`);
  let parsed = null;
  try {
    const raw = await readFile(file, 'utf8');
    parsed = JSON.parse(raw);
  } catch {
    parsed = null;
  }
  manifestCache.set(chapter, parsed);
  return parsed;
}

// ── Token-bucket rate limiter ────────────────────────────────────────────────
const buckets = new Map(); // ip -> { tokens, last }

function takeToken(ip) {
  const now = Date.now();
  let b = buckets.get(ip);
  if (!b) {
    b = { tokens: RL_BURST, last: now };
    buckets.set(ip, b);
  }
  // Refill proportional to elapsed time.
  const elapsedMin = (now - b.last) / 60_000;
  b.tokens = Math.min(RL_BURST, b.tokens + elapsedMin * RL_REFILL_PER_MIN);
  b.last = now;
  if (b.tokens >= 1) {
    b.tokens -= 1;
    return true;
  }
  return false;
}

// Periodically evict idle buckets so the map can't grow unbounded.
const bucketSweeper = setInterval(() => {
  const now = Date.now();
  for (const [ip, b] of buckets) {
    // Idle for >10 min and fully refilled → drop it.
    if (now - b.last > 600_000) buckets.delete(ip);
  }
}, 300_000);
bucketSweeper.unref?.();

function clientIp(req) {
  // Trust ONLY the first hop of x-forwarded-for (set by our ingress). The rest of
  // the chain is client-controllable and must not be trusted for rate limiting.
  const xff = req.headers['x-forwarded-for'];
  if (typeof xff === 'string' && xff.length > 0) {
    return xff.split(',')[0].trim();
  }
  return req.socket.remoteAddress || 'unknown';
}

// ── Concurrency queue ────────────────────────────────────────────────────────
let active = 0;
const waiting = []; // array of { resolve, reject }

function acquireSlot() {
  return new Promise((resolve, reject) => {
    if (active < MAX_CONCURRENT) {
      active += 1;
      resolve();
      return;
    }
    if (waiting.length >= QUEUE_CAP) {
      const e = new Error('at capacity');
      e.code = 'CAPACITY';
      reject(e);
      return;
    }
    waiting.push({ resolve, reject, enqueuedAt: Date.now() });
  });
}

function releaseSlot() {
  const next = waiting.shift();
  if (next) {
    next.queueMs = Date.now() - next.enqueuedAt;
    next.resolve(next.queueMs);
  } else {
    active = Math.max(0, active - 1);
  }
}

// ── HTTP helpers ─────────────────────────────────────────────────────────────
function corsHeaders(origin) {
  const h = {
    Vary: 'Origin',
    'Access-Control-Allow-Methods': 'POST, GET, OPTIONS',
    'Access-Control-Allow-Headers': 'Content-Type',
    'Access-Control-Max-Age': '600',
  };
  if (origin && ALLOWED_ORIGINS.includes(origin)) {
    h['Access-Control-Allow-Origin'] = origin;
    // No Access-Control-Allow-Credentials: cookies are explicitly not used.
  }
  return h;
}

function sendJson(res, status, obj, extraHeaders = {}) {
  const body = JSON.stringify(obj);
  res.writeHead(status, {
    'Content-Type': 'application/json; charset=utf-8',
    'Content-Length': Buffer.byteLength(body),
    'Cache-Control': 'no-store',
    ...extraHeaders,
  });
  res.end(body);
}

function sendError(res, status, code, message, extraHeaders = {}) {
  sendJson(res, status, { error: code, message }, extraHeaders);
}

// Read the request body with a hard byte cap (→ 413).
function readBody(req, limit) {
  return new Promise((resolve, reject) => {
    let size = 0;
    const chunks = [];
    let done = false;
    req.on('data', (chunk) => {
      if (done) return;
      size += chunk.length;
      if (size > limit) {
        done = true;
        // Reject so the caller can send a 413, but DON'T destroy the socket —
        // doing so would abort the connection before the response flushes. Drop
        // the accumulated chunks and resume-then-discard any remaining inbound
        // bytes so the response can be written cleanly.
        chunks.length = 0;
        const e = new Error('payload too large');
        e.code = 'TOO_LARGE';
        req.on('data', () => {}); // discard the rest without buffering
        req.resume();
        reject(e);
        return;
      }
      chunks.push(chunk);
    });
    req.on('end', () => {
      if (!done) resolve(Buffer.concat(chunks));
    });
    req.on('error', (e) => {
      if (!done) reject(e);
    });
  });
}

// ── Request validation for POST /run ─────────────────────────────────────────
function validateRunBody(parsed, manifest) {
  // chapter
  const chapter = parsed.chapter;
  if (!Number.isInteger(chapter) || chapter < MIN_CHAPTER || chapter > MAX_CHAPTER) {
    return { error: `chapter must be an integer in ${MIN_CHAPTER}..${MAX_CHAPTER}` };
  }
  // mode
  if (parsed.mode !== 'run' && parsed.mode !== 'submit') {
    return { error: 'mode must be "run" or "submit"' };
  }
  // files
  const files = parsed.files;
  if (files === null || typeof files !== 'object' || Array.isArray(files)) {
    return { error: 'files must be an object of { path: text }' };
  }
  if (!manifest) {
    return { error: `no manifest for chapter ${chapter}`, status: 404 };
  }
  const editable = Array.isArray(manifest.editable) ? manifest.editable : [];
  const editableSet = new Set(editable);
  for (const [key, val] of Object.entries(files)) {
    if (!editableSet.has(key)) {
      return { error: `file "${key}" is not editable for chapter ${chapter}` };
    }
    if (typeof val !== 'string') {
      return { error: `file "${key}" content must be a string` };
    }
  }
  return { ok: true, chapter, mode: parsed.mode, files };
}

// ── Route handlers ───────────────────────────────────────────────────────────
async function handleHealthz(req, res, cors) {
  sendJson(res, 200, { status: 'ok', active, queued: waiting.length }, cors);
}

async function handleManifest(req, res, cors, chapterStr) {
  const chapter = Number(chapterStr);
  if (!Number.isInteger(chapter) || chapter < MIN_CHAPTER || chapter > MAX_CHAPTER) {
    sendError(res, 400, 'invalid', `chapter must be an integer in ${MIN_CHAPTER}..${MAX_CHAPTER}`, cors);
    return;
  }
  const manifest = await loadManifest(chapter);
  if (!manifest) {
    sendError(res, 404, 'not_found', `no manifest for chapter ${chapter}`, cors);
    return;
  }
  // Public subset ONLY: editable list + project name. Never expose steps,
  // expected outputs, grep patterns, or compile sources (anti-hardcoding).
  sendJson(res, 200, {
    chapter,
    project: manifest.project || null,
    editable: Array.isArray(manifest.editable) ? manifest.editable : [],
  }, cors);
}

async function handleRun(req, res, cors, ip) {
  // Rate limit first (cheapest rejection).
  if (!takeToken(ip)) {
    sendError(res, 429, 'rate_limited', 'too many requests; slow down', {
      ...cors,
      'Retry-After': '6',
    });
    return;
  }

  // Read + size-guard the body.
  let raw;
  try {
    raw = await readBody(req, MAX_BODY_BYTES);
  } catch (e) {
    if (e.code === 'TOO_LARGE') {
      sendError(res, 413, 'too_large', `body exceeds ${MAX_BODY_BYTES} bytes`, cors);
    } else {
      sendError(res, 400, 'invalid', 'could not read request body', cors);
    }
    return;
  }

  // Parse JSON.
  let parsed;
  try {
    parsed = JSON.parse(raw.toString('utf8'));
  } catch {
    sendError(res, 400, 'invalid', 'body must be valid JSON', cors);
    return;
  }
  if (parsed === null || typeof parsed !== 'object' || Array.isArray(parsed)) {
    sendError(res, 400, 'invalid', 'body must be a JSON object', cors);
    return;
  }

  // Need the manifest to validate editable keys.
  const chapterMaybe = parsed.chapter;
  let manifest = null;
  if (Number.isInteger(chapterMaybe) && chapterMaybe >= MIN_CHAPTER && chapterMaybe <= MAX_CHAPTER) {
    manifest = await loadManifest(chapterMaybe);
  }

  const v = validateRunBody(parsed, manifest);
  if (v.error) {
    sendError(res, v.status || 400, v.status === 404 ? 'not_found' : 'invalid', v.error, cors);
    return;
  }

  // Acquire a concurrency slot (→ 503 if the queue is full).
  let queueMs = 0;
  try {
    const r = await acquireSlot();
    if (typeof r === 'number') queueMs = r; // resolved from the waiting queue
  } catch (e) {
    if (e.code === 'CAPACITY') {
      sendError(res, 503, 'at_capacity', 'judge is busy; retry shortly', {
        ...cors,
        'Retry-After': '5',
      });
    } else {
      sendError(res, 500, 'internal', 'queue error', cors);
    }
    return;
  }

  // Run the job, always releasing the slot.
  try {
    const result = await runJob({
      chapter: v.chapter,
      mode: v.mode,
      files: v.files,
      manifest,
      queueMs,
    });
    sendJson(res, 200, result, cors);
  } catch (e) {
    sendError(res, 500, 'internal', `judge failed: ${e.message}`, cors);
  } finally {
    releaseSlot();
  }
}

// ── Server ───────────────────────────────────────────────────────────────────
const server = createServer(async (req, res) => {
  const origin = req.headers.origin;
  const cors = corsHeaders(origin);
  const ip = clientIp(req);

  // Preflight.
  if (req.method === 'OPTIONS') {
    res.writeHead(204, cors);
    res.end();
    return;
  }

  let url;
  try {
    url = new URL(req.url, `http://${req.headers.host || 'localhost'}`);
  } catch {
    sendError(res, 400, 'invalid', 'bad request url', cors);
    return;
  }
  const path = url.pathname;

  try {
    if (req.method === 'GET' && path === '/healthz') {
      await handleHealthz(req, res, cors);
      return;
    }
    if (req.method === 'GET' && path.startsWith('/manifest/')) {
      const chapterStr = decodeURIComponent(path.slice('/manifest/'.length));
      await handleManifest(req, res, cors, chapterStr);
      return;
    }
    if (req.method === 'POST' && path === '/run') {
      await handleRun(req, res, cors, ip);
      return;
    }
    sendError(res, 404, 'not_found', 'no such route', cors);
  } catch (e) {
    sendError(res, 500, 'internal', `unhandled: ${e.message}`, cors);
  }
});

// Guardrails on the socket so a slow/idle client can't tie up a connection.
server.requestTimeout = 30_000;
server.headersTimeout = 10_000;
server.keepAliveTimeout = 5_000;

// Only auto-listen when run as the entrypoint (not when imported by a test).
const isMain = process.argv[1] && import.meta.url === `file://${process.argv[1]}`;
if (isMain) {
  server.listen(PORT, () => {
    // eslint-disable-next-line no-console
    console.log(`[judge] listening on :${PORT}  origins=${ALLOWED_ORIGINS.join(',')}  manifests=${MANIFESTS_DIR}`);
  });

  for (const sig of ['SIGTERM', 'SIGINT']) {
    process.on(sig, () => {
      // eslint-disable-next-line no-console
      console.log(`[judge] ${sig} → draining`);
      clearInterval(bucketSweeper);
      server.close(() => process.exit(0));
      // Force-exit if connections linger.
      setTimeout(() => process.exit(0), 5_000).unref?.();
    });
  }
}

export { server, validateRunBody, takeToken, corsHeaders, loadManifest };
