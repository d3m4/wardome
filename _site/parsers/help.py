from pathlib import Path


def parse_help_file(path: Path):
    entries = []
    warnings = []
    if not path.exists():
        return entries, warnings
    with path.open('r', encoding='utf-8', errors='replace') as f:
        lines = [line.rstrip('\n') for line in f]

    i = 0
    while i < len(lines):
        if lines[i].strip().startswith('#0'):
            i += 1
            # keywords line (space-separated)
            if i >= len(lines):
                break
            kw_line = lines[i].strip()
            i += 1
            # title or uppercase alias line(s) may follow; collect until blank line or content
            aliases = []
            while i < len(lines) and lines[i].strip() and not lines[i].strip().startswith('#0'):
                # consider all-caps or likely titles as alias
                if lines[i].strip().endswith('\t') or lines[i].strip().isupper():
                    aliases.append(lines[i].strip().strip('\t'))
                    i += 1
                else:
                    break
            # content until next '#0'
            content = []
            while i < len(lines) and not lines[i].strip().startswith('#0'):
                content.append(lines[i])
                i += 1
            entries.append({
                'keywords': [w for w in kw_line.split() if w],
                'aliases': aliases,
                'body': '\n'.join(content).strip(),
                'source_path': str(path)
            })
        else:
            i += 1
    return entries, warnings

