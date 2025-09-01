from pathlib import Path


def parse_obj_dir(path: Path):
    objs = []
    warnings = []
    if not path.exists():
        return objs, warnings
    for file in sorted(path.glob('*.obj')):
        try:
            objs += parse_obj_file(file)
        except Exception as e:
            warnings.append(f"{file}: {e}")
    return objs, warnings


def parse_obj_file(path: Path):
    out = []
    with path.open('r', encoding='utf-8', errors='replace') as f:
        lines = [line.rstrip('\n') for line in f]
    i = 0
    while i < len(lines):
        l = lines[i].strip()
        if not l:
            i += 1; continue
        if l == '$':
            break
        if l.startswith('#') and l[1:].strip().isdigit():
            vnum = int(l[1:].strip())
            i += 1
            alias = lines[i].rstrip('~'); i += 1
            short = lines[i].rstrip('~'); i += 1
            longd = lines[i].rstrip('~'); i += 1
            action_desc = lines[i].rstrip('~'); i += 1
            # type/flags/wear
            header = lines[i].strip(); i += 1
            # values[0..3]
            values = lines[i].strip(); i += 1
            # weight cost rent
            costline = lines[i].strip(); i += 1

            extra_desc = []
            affects = []

            while i < len(lines):
                s = lines[i].strip()
                if s == 'E':
                    i += 1
                    kw = lines[i].rstrip('~'); i += 1
                    desc = []
                    while i < len(lines) and lines[i].strip() != '~':
                        desc.append(lines[i])
                        i += 1
                    i += 1  # skip '~'
                    extra_desc.append({'keywords': kw, 'description': '\n'.join(desc).strip()})
                elif s == 'A':
                    i += 1
                    if i < len(lines):
                        parts = lines[i].split()
                        i += 1
                        if len(parts) >= 2:
                            affects.append({'location': try_int(parts[0]), 'modifier': try_int(parts[1])})
                elif s.startswith('#') or s == '$' or s == 'S':
                    break
                else:
                    i += 1

            out.append({
                'id': vnum,
                'alias': alias,
                'short_desc': short,
                'long_desc': longd,
                'action_desc': action_desc,
                'header_raw': header,
                'values_raw': values,
                'cost_raw': costline,
                'extra_descriptions': extra_desc,
                'affects': affects,
                'source_path': str(path)
            })
        else:
            i += 1
    return out


def try_int(s):
    try:
        return int(s)
    except Exception:
        return None

