from pathlib import Path
import re


def extract_command_table(path: Path):
    text = path.read_text(encoding='utf-8', errors='replace')
    start_pat = re.compile(r"cmd_info\[\]\s*=\s*\{", re.MULTILINE)
    m = start_pat.search(text)
    if not m:
        raise RuntimeError("cmd_info[] array not found")
    start = m.end()
    end = text.find('};', start)
    if end == -1:
        raise RuntimeError("cmd_info[] terminator not found")
    body = text[start:end]

    # Match struct initializers like: { "north", "north", POS_STANDING, do_move , 0, SCMD_NORTH },
    entries = []
    # Remove comments to simplify
    body_nocomments = re.sub(r"//.*", "", body)
    body_nocomments = re.sub(r"/\*.*?\*/", "", body_nocomments, flags=re.S)

    # Split by braces at top level
    for mentry in re.finditer(r"\{([^{}]*)\}", body_nocomments):
        fields = [f.strip() for f in mentry.group(1).split(',')]
        if len(fields) < 2:
            continue
        # Expect at least: command, alias, position, function, level, subcmd
        # Strings have quotes; others are identifiers or numbers
        def strip_quotes(s: str):
            s = s.strip()
            if s.startswith('"') and s.endswith('"'):
                return s[1:-1]
            return s
        entry = {
            'command': strip_quotes(fields[0]) if len(fields) > 0 else '',
            'alias': strip_quotes(fields[1]) if len(fields) > 1 else '',
            'minimum_position': fields[2] if len(fields) > 2 else '',
            'function': fields[3] if len(fields) > 3 else '',
            'minimum_level': fields[4] if len(fields) > 4 else '',
            'subcmd': fields[5] if len(fields) > 5 else ''
        }
        entries.append(entry)

    return { 'commands': entries, 'source_path': str(path) }

