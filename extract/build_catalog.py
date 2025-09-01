#!/usr/bin/env python3
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parent
OUT = ROOT / 'out'


def safe_read_json(path: Path):
    try:
        return json.loads(path.read_text(encoding='utf-8'))
    except Exception:
        return None


def list_entities(folder: str, key_fields):
    items = []
    d = OUT / folder
    if not d.exists():
        return items
    for f in sorted(d.glob('*.json')):
        data = safe_read_json(f)
        if not isinstance(data, dict):
            continue
        item = {
            'file': str(f.relative_to(OUT)),
        }
        for field, out_key in key_fields:
            val = data.get(field)
            if val is not None:
                item[out_key] = val
        items.append(item)
    return items


def main():
    catalog = {
        'rooms': list_entities('rooms', [('id', 'id'), ('name', 'name'), ('zone_id', 'zone_id')]),
        'zones': list_entities('zones', [('id', 'id'), ('name', 'name')]),
        'objects': list_entities('objects', [('id', 'id'), ('short_desc', 'short_desc')]),
        'mobs': list_entities('mobs', [('id', 'id'), ('short_desc', 'short_desc')]),
        'shops': list_entities('shops', [('id', 'id')]),
        'help': list_entities('help', [('keywords', 'keywords')]),
    }
    # Tables
    tables = {}
    for tname in ['commands', 'spells', 'skills', 'classes']:
        tf = OUT / 'tables' / f'{tname}.json'
        if tf.exists():
            tables[tname] = safe_read_json(tf)
    catalog['tables'] = tables

    # Summary passthrough
    summary_file = OUT / 'summaries' / 'summary.json'
    if summary_file.exists():
        catalog['summary'] = safe_read_json(summary_file)

    (OUT / 'catalog.json').write_text(json.dumps(catalog, ensure_ascii=False, indent=2), encoding='utf-8')
    print(f"Wrote {OUT/'catalog.json'}")


if __name__ == '__main__':
    main()

