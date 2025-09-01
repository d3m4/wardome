# Publishing the Extract Browser

This guide shows how to publish `extract/site/` (static viewer) and `extract/out/` (data) to GitHub Pages. Two options are covered: gh-pages branch and /docs folder.

## 0) Build artifacts

```
python3 extract/run.py
python3 extract/build_catalog.py
```
Commit the generated `extract/out/**` so Pages can serve the JSON files.

## Option A — gh-pages branch (cleanest)

- Create an orphan branch and publish only `extract/*` as the site root:
```
# From repo root
git worktree add ./_site gh-pages
rsync -a --delete extract/ ./_site/
cd _site && git add . && git commit -m "Publish extract browser" && git push -u origin gh-pages
```
- Enable Pages: Settings → Pages → Build and deployment → Deploy from a branch → `gh-pages` / `/ (root)`
- Open: `https://<user>.github.io/<repo>/site/`

Note: The viewer loads `../out/catalog.json`, so `site/` and `out/` must both exist at the web root. The rsync above copies the entire `extract/` folder structure, which satisfies this.

### Publishing to a different (public) repository

If this repository is private, use the helper script to push only the `extract/` folder to a sibling public repo (e.g., `../wardome_public`). It creates a gh-pages worktree in the public repo and pushes there.

```
# From the private repo root
chmod +x extract/publish.sh
PUBLIC_REPO_DIR=../wardome_public extract/publish.sh
```

Then enable Pages in the public repo (gh-pages / root) and browse `/site/`.

### Option A (Automated) — GitHub Actions

- A workflow is added at `.github/workflows/publish-extract.yml` that:
  - Builds extract artifacts (`extract/run.py`, `extract/build_catalog.py`)
  - Touches `extract/.nojekyll`
  - Publishes `extract/` to `gh-pages` as an orphan branch
- Trigger: push to `main` (paths `extract/**`, `wdii/**`) or manually via “Run workflow”.
- First time: enable Pages to serve from `gh-pages`.

## Option B — /docs folder on main

- Copy the content under `extract/` into `docs/`:
```
rsync -a --delete extract/ docs/
git add docs && git commit -m "Docs: publish extract browser" && git push
```
- Enable Pages: Settings → Pages → Deploy from a branch → `main` / `/docs`
- Open: `https://<user>.github.io/<repo>/site/`

## Private access

GitHub Pages has no built-in auth. To restrict access:
- Cloudflare Access (recommended): put SSO in front of your Pages domain.
- Netlify/Vercel: deploy the same static folder and enable password protection.
- Firebase Hosting + Auth: possible but requires a proxy (Cloud Function) to enforce auth on JSON fetches.

Tip: If privacy matters, use Cloudflare Access for the least friction.
