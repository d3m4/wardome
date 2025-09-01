# Extract Toolkit

New, self-contained extractors that parse WarDome (CircleMUD-based) data into portable JSON, without relying on older migration scripts.

## Usage

- Full run:
```
python3 extract/run.py
```

- Outputs under `extract/out/`:
- `rooms/*.json`, `zones/*.json`, `shops/*.json`, `help/*.json`
- `tables/commands.json`, `tables/spells.json`, `tables/skills.json`, `tables/classes.json`
- `summaries/*.json`

## Visualize (Static HTML)

1) Build catalog for the viewer:
```
python3 extract/build_catalog.py
```
2) Serve locally (avoids CORS):
```
python3 -m http.server -d extract 8080
```
3) Open the browser:
```
http://localhost:8080/site/
```

The viewer loads `out/catalog.json` and links to all JSON entities. Click any row to preview full JSON.

Notes
- Parsers are tolerant; when exact structure isn’t known, they keep raw fields alongside parsed ones.
- Provenance is recorded when possible.
- Extend parsers under `extract/parsers/` to add more types (triggers, quests, objects, mobs).

## Publish Options

- GitHub Pages (public): push the repo and enable Pages; browse `/extract/site/` which fetches `/extract/out/` assets.
- Private access (options):
  - Firebase Hosting + Firebase Auth: enforce auth via a Cloud Function rewrite that serves JSON only to signed-in users (adds complexity).
  - Cloudflare Access (zero-trust) in front of GitHub Pages to require SSO.
  - Netlify/Vercel password protection for quick gating.

If simplicity matters, use public GitHub Pages. For private browsing, prefer Cloudflare Access over building custom auth for a static site.
