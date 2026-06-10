import { defineMiddleware } from 'astro:middleware';

/**
 * Dev-server-only: cross-origin isolation so SharedArrayBuffer (and thus the
 * Wasmer toolchain) works under `astro dev`. Production gets these headers
 * from staticwebapp.config.json — keep the two in sync.
 */
export const onRequest = defineMiddleware(async (_ctx, next) => {
  const res = await next();
  res.headers.set('Cross-Origin-Opener-Policy', 'same-origin');
  res.headers.set('Cross-Origin-Embedder-Policy', 'require-corp');
  return res;
});
