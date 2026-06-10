// @ts-check
import { defineConfig } from 'astro/config';
import react from '@astrojs/react';
import sitemap from '@astrojs/sitemap';

export default defineConfig({
  site: 'https://cppforall.com',
  trailingSlash: 'always',
  integrations: [react(), sitemap()],
  vite: {
    server: {
      // cross-origin isolation in dev — the Wasmer toolchain needs SharedArrayBuffer
      headers: {
        'Cross-Origin-Opener-Policy': 'same-origin',
        'Cross-Origin-Embedder-Policy': 'require-corp',
      },
    },
    optimizeDeps: { exclude: ['@wasmer/sdk'] },
  },
});
