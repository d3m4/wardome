# WarDome Extract Browser

This repository contains a static browser for the exported WarDome world data. You can view it locally, or publish it via GitHub Pages.

## What’s Here

- `extract/site/` — Static viewer (HTML/CSS/JS)
- `extract/out/` — Extracted JSON content (rooms, zones, mobs, objects, shops, help, tables, summaries)
- `_site/` — gh-pages worktree (created by publish steps), mirrors `extract/` for Pages

Note: This repo hosts the already-built site/data only. The extraction scripts run in the private repository.

## View Locally

Option 1 — Serve the extracted site folder:

```
python3 -m http.server -d extract 8080
# open http://localhost:8080/site/
```

Option 2 — Serve the gh-pages worktree (if already created):

```
python3 -m http.server -d _site 8080
# open http://localhost:8080/site/
```

## Publish to GitHub Pages

This repo uses the `gh-pages` branch to host the site.

1) Ensure `_site/` exists and contains the site files (created by sync from the private repo).
2) Commit and push the `_site` worktree:

```
cd _site
git status
git add .
git commit -m "Publish extract browser"
git push -u origin gh-pages
```

3) In GitHub → Settings → Pages → set source to `gh-pages` / `/ (root)`.
4) Visit: `https://<user>.github.io/<repo>/site/`

## Updating Content

Run these in the PRIVATE repo, then commit here:

```
# In private repo root
PUBLIC_REPO_DIR=../wardome_public ./extract/publish.sh

# Then in this public repo:
cd _site
git add .
git commit -m "Publish"
git push -u origin gh-pages
```

