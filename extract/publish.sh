#!/usr/bin/env bash
set -euo pipefail

# Publish the extract/ folder from THIS private repo to the PUBLIC repo's gh-pages branch.
# Default public repo directory: one level up as ../wardome_public

PRIVATE_ROOT=$(git rev-parse --show-toplevel)
PUBLIC_REPO_DIR=${PUBLIC_REPO_DIR:-"$PRIVATE_ROOT/../wardome_public"}
PUBLIC_REPO_DIR=$(realpath -m "$PUBLIC_REPO_DIR")
PUBLIC_SITE_DIR="$PUBLIC_REPO_DIR/_site"

if [ ! -d "$PUBLIC_REPO_DIR/.git" ]; then
  echo "ERROR: Public repo not found at $PUBLIC_REPO_DIR (missing .git)." >&2
  echo "Set PUBLIC_REPO_DIR to your public repo path, e.g.:" >&2
  echo "  PUBLIC_REPO_DIR=../wardome_public extract/publish.sh" >&2
  exit 1
fi

echo "Building extract artifacts in private repo..."
python3 "$PRIVATE_ROOT/extract/run.py"
python3 "$PRIVATE_ROOT/extract/build_catalog.py"
touch "$PRIVATE_ROOT/extract/.nojekyll"

echo "Preparing gh-pages worktree in public repo at $PUBLIC_SITE_DIR..."
pushd "$PUBLIC_REPO_DIR" >/dev/null
if [ ! -d "$PUBLIC_SITE_DIR" ]; then
  git worktree add "$PUBLIC_SITE_DIR" gh-pages || {
    echo "Creating gh-pages branch in public repo...";
    git branch gh-pages || true
    git worktree add "$PUBLIC_SITE_DIR" gh-pages
  }
fi
popd >/dev/null

echo "Syncing extract/ (private) into public worktree..."
rsync -a --delete "$PRIVATE_ROOT/extract/" "$PUBLIC_SITE_DIR/"

if [ "${PUBLISH_COMMIT:-0}" = "1" ]; then
  echo "Committing and pushing to public gh-pages..."
  pushd "$PUBLIC_SITE_DIR" >/dev/null
  git add .
  git commit -m "Publish extract browser from private repo" || echo "No changes to commit"
  git push -u origin gh-pages
  popd >/dev/null
else
  echo "Sync complete. Review and commit in the public repo manually:"
  echo "  cd $PUBLIC_SITE_DIR && git status && git add . && git commit -m 'Publish' && git push -u origin gh-pages"
fi

echo "Done. Ensure GitHub Pages is enabled on the public repo (gh-pages / root), then open /site/."
