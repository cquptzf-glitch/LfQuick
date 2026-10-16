#!/usr/bin/env python3
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
RULES = {
    "src/core": [r"#include\s*[<\"]QQuick", r"#include\s*[<\"]QWidget", r"Qt6::Quick", r"Qt6::Widgets"],
    "src/runtime": [r"#include\s*[<\"]QQuick", r"#include\s*[<\"]QWidget", r"Qt6::Quick", r"Qt6::Widgets"],
}

violations = []
for rel, patterns in RULES.items():
    for path in (ROOT / rel).rglob("*"):
        if not path.is_file() or path.suffix not in {".h", ".hpp", ".cpp", ".cmake", ".txt"} and path.name != "CMakeLists.txt":
            continue
        text = path.read_text(encoding="utf-8", errors="ignore")
        for pattern in patterns:
            if re.search(pattern, text):
                violations.append(f"{path.relative_to(ROOT)} violates {pattern}")

if violations:
    print("Architecture boundary check FAILED:")
    print("\n".join(f" - {v}" for v in violations))
    sys.exit(1)
print("Architecture boundary check: PASS")
