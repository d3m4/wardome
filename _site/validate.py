#!/usr/bin/env python3
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parent
OUT = ROOT / 'out'


def load_jsons(path: Path):
    data = []
    for f in sorted(path.glob('*.json')):
        try:
            data.append(json.loads(f.read_text(encoding='utf-8')))
        except Exception as e:
            print(f"WARN: {f}: {e}")
    return data


def main():
    rooms = load_jsons(OUT / 'rooms')
    zones = load_jsons(OUT / 'zones')
    room_ids = {r['id'] for r in rooms}

    problems = []

    # Validate exits point to existing rooms
    for r in rooms:
        for ex in r.get('exits', []):
            to_v = ex.get('to')
            if isinstance(to_v, int) and to_v not in room_ids:
                problems.append({
                    'type': 'unknown_exit',
                    'from': r['id'],
                    'to': to_v
                })

    # Validate zone commands referencing rooms
    for z in zones:
        for c in z.get('commands', []):
            # heuristics: for D command (door), arg3 might be room vnum
            if c['cmd'] == 'D' and len(c['args']) >= 3:
                to_room = c['args'][2]
                if isinstance(to_room, int) and to_room not in room_ids:
                    problems.append({
                        'type': 'zone_door_unknown_room',
                        'zone': z['id'],
                        'to': to_room
                    })

    report = {
        'room_count': len(rooms),
        'zone_count': len(zones),
        'problems': problems[:200],
        'problem_count': len(problems)
    }
    print(json.dumps(report, indent=2))


if __name__ == '__main__':
    main()

