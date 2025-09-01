from pathlib import Path


def parse_mob_dir(path: Path):
    mobs = []
    warnings = []
    if not path.exists():
        return mobs, warnings
    for file in sorted(path.glob('*.mob')):
        try:
            mobs += parse_mob_file(file)
        except Exception as e:
            warnings.append(f"{file}: {e}")
    return mobs, warnings


def parse_mob_file(path: Path):
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
            desc = []
            while i < len(lines) and lines[i].strip() != '~':
                desc.append(lines[i])
                i += 1
            i += 1  # skip '~'
            flags_line = lines[i].strip(); i += 1
            nums1 = lines[i].strip(); i += 1
            nums2 = lines[i].strip(); i += 1
            nums3 = lines[i].strip(); i += 1
            out.append({
                'id': vnum,
                'alias': alias,
                'short_desc': short,
                'long_desc': longd,
                'detailed_desc': '\n'.join(desc).strip(),
                'flags_raw': flags_line,
                'stats1_raw': nums1,
                'stats2_raw': nums2,
                'stats3_raw': nums3,
                'source_path': str(path)
            })
        else:
            i += 1
    return out

