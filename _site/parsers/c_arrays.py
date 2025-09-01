from pathlib import Path
import re


def extract_c_string_array(path: Path, array_name: str):
    """
    Extract a C string array like: const char *name[] = { "a", "b", "\n" };
    Returns { 'name': array_name, 'values': [..], 'source_path': str(path) }
    """
    text = path.read_text(encoding='utf-8', errors='replace')
    # Find start of array
    pattern = re.compile(rf"\b{re.escape(array_name)}\s*\[\s*\]\s*=\s*\{{", re.MULTILINE)
    m = pattern.search(text)
    if not m:
        raise RuntimeError(f"array {array_name} not found in {path}")
    start = m.end()
    # Find matching closing brace followed by semicolon
    end = text.find('};', start)
    if end == -1:
        raise RuntimeError(f"array {array_name} terminator not found")
    body = text[start:end]
    # Extract C strings
    # Handles escaped quotes minimally
    strings = re.findall(r'"((?:\\.|[^"\\])*)"', body)
    # Unescape
    values = [s.encode('utf-8').decode('unicode_escape') for s in strings]
    return {
        'name': array_name,
        'values': values,
        'source_path': str(path)
    }

