# WarDome Extract Browser

This repo contains a static browser for the extracted world. Use either the HTML viewer or this Markdown index.

## Quick Start (HTML Viewer)

- Build catalog: `python3 extract/build_catalog.py`
- Serve locally (avoids browser CORS): `python3 -m http.server -d extract 8080`
- Open: `http://localhost:8080/site/`

The viewer loads `extract/out/catalog.json`, lists Zones/Rooms/Mobs/Objects/Shops/Help, and shows JSON details on click. All links open the raw JSON files.

## Markdown Index (lightweight)

- Summary: `extract/out/summaries/summary.json`
- Catalog: `extract/out/catalog.json`

### Useful Directories

- Rooms: `extract/out/rooms/`
- Zones: `extract/out/zones/`
- Mobs: `extract/out/mobs/`
- Objects: `extract/out/objects/`
- Shops: `extract/out/shops/`
- Help: `extract/out/help/`
- Tables: `extract/out/tables/` (commands, spells, skills, classes)

### Sample Links

- A zone: `extract/out/zones/12.json`
- A room: `extract/out/rooms/1204.json`
- A mob: `extract/out/mobs/1202.json`
- An object: `extract/out/objects/904.json`
- Commands: `extract/out/tables/commands.json`
- Spells: `extract/out/tables/spells.json`

> Tip: On GitHub, these relative links render and are easy to click through.

