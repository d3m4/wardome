from pathlib import Path


def parse_shp_dir(path: Path):
    shops = []
    warnings = []
    if not path.exists():
        return shops, warnings
    header_seen = False
    for file in sorted(path.glob('*.shp')):
        try:
            s = parse_shp_file(file)
            shops.extend(s)
        except Exception as e:
            warnings.append(f"{file}: {e}")
    return shops, warnings


def parse_shp_file(path: Path):
    shops = []
    with path.open('r', encoding='utf-8', errors='replace') as f:
        lines = [line.rstrip('\n') for line in f]
    i = 0
    # optional header first line ends with ~
    if i < len(lines) and lines[i].endswith('~') and 'Shop File' in lines[i]:
        i += 1
    while i < len(lines):
        line = lines[i].strip()
        if not line:
            i += 1
            continue
        if line.startswith('#') and line.endswith('~'):
            keeper = int(line[1:-1])
            i += 1
            # producing until -1
            producing = []
            while i < len(lines) and lines[i].strip() != '-1':
                producing.append(_try_int(lines[i].strip()))
                i += 1
            i += 1  # skip -1
            # profits
            profit_buy = _try_float(lines[i].strip()); i += 1
            profit_sell = _try_float(lines[i].strip()); i += 1
            # buy types until -1 or messages start
            buy_types = []
            while i < len(lines) and lines[i].strip() != '-1' and not lines[i].strip().endswith('~'):
                buy_types.append(lines[i].strip())
                i += 1
            if i < len(lines) and lines[i].strip() == '-1':
                i += 1
            # messages: collect lines ending with '~'
            messages = []
            while i < len(lines) and lines[i].strip().endswith('~'):
                messages.append(lines[i][:-1])
                i += 1
            # the remaining numeric tail (temper, open/close, rooms etc.)
            tail_nums = []
            while i < len(lines) and lines[i].strip() and not lines[i].startswith('#'):
                val = lines[i].strip()
                # stop if we hit a non-numeric line accidentally
                if not _is_num(val):
                    break
                tail_nums.append(_try_int(val))
                i += 1
            shops.append({
                'id': keeper,
                'producing': [v for v in producing if isinstance(v, int)],
                'profit_buy': profit_buy,
                'profit_sell': profit_sell,
                'buy_types': buy_types,
                'messages': messages,
                'tail': tail_nums,
                'source_path': str(path)
            })
        else:
            i += 1
    return shops


def _try_int(s):
    try:
        return int(s)
    except Exception:
        return None


def _try_float(s):
    try:
        return float(s)
    except Exception:
        return None


def _is_num(s: str) -> bool:
    try:
        float(s)
        return True
    except Exception:
        return False

