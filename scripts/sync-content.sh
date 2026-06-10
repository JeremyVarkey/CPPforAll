#!/usr/bin/env bash
# sync-content.sh — vendor the CS6340 notes + drills into this repo's content/ dir.
#
# The CS6340 cpp-llvm-foundations folder is the SINGLE SOURCE OF TRUTH for content
# (Codex authors notes/, Claude builds drills/). This script copies a clean snapshot
# into content/ so the site can build from it. Re-run any time upstream changes.
#
# Usage:  bash scripts/sync-content.sh
set -euo pipefail

SRC="/Users/jeremyvarkey/Documents/Computer Science Courses/CS6340 - Software Analysis and Testing/cpp-llvm-foundations"
REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
DEST="$REPO_ROOT/content"

if [[ ! -d "$SRC" ]]; then
  echo "ERROR: source not found: $SRC" >&2
  exit 1
fi

mkdir -p "$DEST"

# Notes — chapter markdowns (Codex's tree)
rsync -a --delete "$SRC/notes/" "$DEST/notes/"

# Drills — exercise project files (Claude's tree). Exclude build artifacts so we
# only vendor source (starter/solution/tests/Makefile/README + the authoring guide).
rsync -a --delete \
  --exclude '*.o' --exclude '*.out' --exclude 'a.out' \
  --exclude '*.dSYM/' --exclude 'tests/run' --exclude '/**/run' \
  --exclude '.DS_Store' \
  "$SRC/drills/" "$DEST/drills/"

notes_n="$(find "$DEST/notes" -name '*.md' | wc -l | tr -d ' ')"
drills_n="$(find "$DEST/drills" -maxdepth 1 -type d -name 'chapter-*' | wc -l | tr -d ' ')"
echo "Synced → $DEST"
echo "  notes:  $notes_n markdown files"
echo "  drills: $drills_n chapter exercises"
