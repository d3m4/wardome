#!/usr/bin/env python3
import json
import os
from pathlib import Path

from parsers.wld import parse_wld_dir
from parsers.zon import parse_zon_dir
from parsers.shp import parse_shp_dir
from parsers.obj import parse_obj_dir
from parsers.mob import parse_mob_dir
from parsers.help import parse_help_file
from parsers.c_arrays import extract_c_string_array
from parsers.commands import extract_command_table


ROOT = Path(__file__).resolve().parent.parent
GAME_LIB = ROOT / 'wdii' / 'lib'
WORLD = GAME_LIB / 'world'
TEXT = GAME_LIB / 'text'
OUT = ROOT / 'extract' / 'out'


def ensure_dirs():
    for p in [OUT / 'rooms', OUT / 'zones', OUT / 'shops', OUT / 'help', OUT / 'tables', OUT / 'summaries']:
        p.mkdir(parents=True, exist_ok=True)


def write_json(path: Path, data):
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open('w', encoding='utf-8') as f:
        json.dump(data, f, ensure_ascii=False, indent=2)


def main():
    ensure_dirs()

    summary = {
        'rooms': 0,
        'zones': 0,
        'shops': 0,
        'help_entries': 0,
        'tables': {},
        'errors': []
    }

    # Rooms (.wld)
    try:
        rooms, r_warn = parse_wld_dir(WORLD / 'wld')
        for r in rooms:
            write_json(OUT / 'rooms' / f"{r['id']}.json", r)
        summary['rooms'] = len(rooms)
        if r_warn:
            summary['errors'] += r_warn
    except Exception as e:
        summary['errors'].append(f"rooms: {e}")

    # Zones (.zon)
    try:
        zones, z_warn = parse_zon_dir(WORLD / 'zon')
        for z in zones:
            write_json(OUT / 'zones' / f"{z['id']}.json", z)
        summary['zones'] = len(zones)
        if z_warn:
            summary['errors'] += z_warn
    except Exception as e:
        summary['errors'].append(f"zones: {e}")

    # Shops (.shp)
    try:
        shops, s_warn = parse_shp_dir(WORLD / 'shp')
        for s in shops:
            write_json(OUT / 'shops' / f"{s['id']}.json", s)
        summary['shops'] = len(shops)
        if s_warn:
            summary['errors'] += s_warn
    except Exception as e:
        summary['errors'].append(f"shops: {e}")

    # Objects (.obj)
    try:
        objs, o_warn = parse_obj_dir(WORLD / 'obj')
        for o in objs:
            write_json(OUT / 'objects' / f"{o['id']}.json", o)
        summary['objects'] = len(objs)
        if o_warn:
            summary['errors'] += o_warn
    except Exception as e:
        summary['errors'].append(f"objects: {e}")

    # Mobiles (.mob)
    try:
        mobs, m_warn = parse_mob_dir(WORLD / 'mob')
        for m in mobs:
            write_json(OUT / 'mobs' / f"{m['id']}.json", m)
        summary['mobs'] = len(mobs)
        if m_warn:
            summary['errors'] += m_warn
    except Exception as e:
        summary['errors'].append(f"mobs: {e}")

    # Help (text/help/help)
    try:
        help_path = TEXT / 'help' / 'help'
        if help_path.exists():
            entries, h_warn = parse_help_file(help_path)
            for idx, h in enumerate(entries):
                key = (h.get('keywords') or ['entry'])[0]
                safe = key.replace(' ', '_').replace('/', '_')[:64] or f"{idx:05d}"
                write_json(OUT / 'help' / f"{safe}.json", h)
            summary['help_entries'] = len(entries)
            if h_warn:
                summary['errors'] += h_warn
    except Exception as e:
        summary['errors'].append(f"help: {e}")

    # Tables from C sources
    try:
        # commands
        commands = extract_command_table(ROOT / 'wdii' / 'src' / 'interpreter.c')
        write_json(OUT / 'tables' / 'commands.json', commands)
        summary['tables']['commands'] = len(commands.get('commands', []))
    except Exception as e:
        summary['errors'].append(f"commands: {e}")

    try:
        # spells + skills share one C array in spell_parser.c
        spells_skills = extract_c_string_array(
            ROOT / 'wdii' / 'src' / 'spell_parser.c', array_name='spells')
        # Partition by index: spells usually start at 1, skills at ~301
        out = {
            'raw': spells_skills['values'],
            'spells': [],
            'skills': []
        }
        for idx, name in enumerate(spells_skills['values']):
            if name == '\\n' or name == '!RESERVED!':
                continue
            if idx <= 300:
                out['spells'].append({'id': idx, 'name': name})
            else:
                out['skills'].append({'id': idx, 'name': name})
        write_json(OUT / 'tables' / 'spells.json', {'spells': out['spells']})
        write_json(OUT / 'tables' / 'skills.json', {'skills': out['skills']})
        summary['tables']['spells'] = len(out['spells'])
        summary['tables']['skills'] = len(out['skills'])
    except Exception as e:
        summary['errors'].append(f"spells/skills: {e}")

    try:
        classes = extract_c_string_array(
            ROOT / 'wdii' / 'src' / 'class.c', array_name='pc_class_types')
        write_json(OUT / 'tables' / 'classes.json', classes)
        summary['tables']['classes'] = len(classes.get('values', []))
    except Exception as e:
        summary['errors'].append(f"classes: {e}")

    # Constants (sector types, room flags, exit flags, zone flags) from constants.c
    try:
        const_src = ROOT / 'wdii' / 'src' / 'constants.c'
        constants = {}
        for name in ['sector_types', 'room_bits', 'exit_bits', 'zone_bits']:
            try:
                arr = extract_c_string_array(const_src, array_name=name)
                constants[name] = arr.get('values', [])
            except Exception as ce:
                summary['errors'].append(f"constants:{name}: {ce}")
        write_json(OUT / 'tables' / 'constants.json', constants)
        summary['tables']['constants'] = sum(len(v) for v in constants.values())
    except Exception as e:
        summary['errors'].append(f"constants: {e}")

    write_json(OUT / 'summaries' / 'summary.json', summary)
    print(json.dumps(summary, indent=2))


if __name__ == '__main__':
    main()
