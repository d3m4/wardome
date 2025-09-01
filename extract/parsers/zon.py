from pathlib import Path


def parse_zon_dir(path: Path):
    zones = []
    warnings = []
    if not path.exists():
        return zones, warnings
    for file in sorted(path.glob('*.zon')):
        try:
            z = parse_zon_file(file)
            zones.extend(z)
        except Exception as e:
            warnings.append(f"{file}: {e}")
    return zones, warnings


def parse_zon_file(path: Path):
    zones = []
    with path.open('r', encoding='utf-8', errors='replace') as f:
        lines = [line.rstrip('\n') for line in f]

    i = 0
    while i < len(lines):
        l = lines[i].strip()
        if not l or l.startswith('*'):
            i += 1
            continue
        if l.startswith('#') and l[1:].strip().isdigit():
            znum = int(l[1:].strip())
            i += 1
            name = lines[i].rstrip('~')
            i += 1
            # header numbers: try variants
            header = lines[i].strip()
            i += 1
            nums = [x for x in header.split() if x]
            # Common: <min_room?> <top_room> <lifespan> <reset_mode>
            header_parsed = {
                'raw': header,
                'min_room': try_int(nums[0]) if len(nums) > 0 else None,
                'top_room': try_int(nums[1]) if len(nums) > 1 else None,
                'lifespan': try_int(nums[2]) if len(nums) > 2 else None,
                'reset_mode': try_int(nums[3]) if len(nums) > 3 else None,
            }

            # optional comments lines starting with '*'
            while i < len(lines) and lines[i].strip().startswith('*'):
                i += 1

            # commands until 'S'
            commands = []
            while i < len(lines):
                line = lines[i].strip()
                i += 1
                if not line:
                    continue
                if line.startswith('*'):
                    continue
                if line == 'S':
                    break
                if line == '$':
                    break
                # parse: LETTER <ints...> [comment]
                parts = line.split()
                letter = parts[0]
                nums = []
                rest = ''
                for p in parts[1:]:
                    v = try_int(p)
                    if v is None:
                        # remaining is comment
                        rest = ' '.join(parts[1+len(nums):])
                        break
                    nums.append(v)
                commands.append({
                    'cmd': letter,
                    'args': nums,
                    'comment': rest
                })

            zones.append({
                'id': znum,
                'name': name.strip(),
                'header': header_parsed,
                'commands': commands,
                'source_path': str(path)
            })
        else:
            i += 1
    return zones


def try_int(s):
    try:
        return int(s)
    except Exception:
        return None

