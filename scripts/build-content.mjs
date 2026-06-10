/**
 * build-content.mjs — content pipeline entrypoint, run as `npm run content`
 * (pre-dev and pre-build). Order matters; each step throws on bad input so
 * a broken pipeline fails the build rather than shipping a broken page.
 *
 * Phase A0: chapters index only. A1 adds notes + drills + validation;
 * Phase B adds grader manifests.
 */
import { buildChaptersIndex } from './build-chapters-index.mjs';

const chapters = await buildChaptersIndex();
console.log(`[content] chapters index: ${chapters.length} chapters`);
