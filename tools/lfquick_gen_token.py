#!/usr/bin/env python3
"""Generate LfQuick token artifacts from JSON or YAML.

Input schema (minimal):
{
  "namespace": "LfQuick",
  "class": "GeneratedTokens",
  "tokens": {
    "surface": {"type": "color", "light": "#FFFFFF", "dark": "#202020"},
    "radiusSmall": {"type": "int", "value": 4},
    "accent": {"type": "color", "expression": "Theme.accentColor",
               "cppExpression": "QColor(QStringLiteral(\"#0067C0\"))"}
  }
}
"""

from __future__ import annotations

import argparse
import json
import re
from pathlib import Path

IDENTIFIER = re.compile(r"^[A-Za-z_][A-Za-z0-9_]*$")
QML_TYPES = {"color", "int", "real", "bool", "string"}


def fail(message: str) -> None:
    raise SystemExit(f"lfquick-gen-token: {message}")


def load_input(path: Path) -> dict:
    try:
        text = path.read_text(encoding="utf-8")
    except OSError as exc:
        fail(f"cannot read {path}: {exc}")

    if path.suffix.lower() in {".yaml", ".yml"}:
        try:
            import yaml  # type: ignore
        except ImportError:
            fail("YAML input requires PyYAML; use JSON or install PyYAML")
        data = yaml.safe_load(text)
    else:
        try:
            data = json.loads(text)
        except json.JSONDecodeError as exc:
            fail(f"invalid JSON: {exc}")

    if not isinstance(data, dict):
        fail("input root must be an object")
    return data


def validate(raw: dict) -> dict:
    tokens = raw.get("tokens")
    if not isinstance(tokens, dict) or not tokens:
        fail("tokens must be a non-empty object")

    normalized: dict[str, dict] = {}
    for name, entry in tokens.items():
        if not IDENTIFIER.match(str(name)):
            fail(f"invalid token name: {name!r}")
        if not isinstance(entry, dict):
            fail(f"token {name} must be an object")
        token_type = str(entry.get("type", "")).strip()
        if token_type not in QML_TYPES:
            fail(f"token {name} has unsupported type {token_type!r}")
        if "expression" not in entry and "value" not in entry and not {"light", "dark"} <= set(entry):
            fail(f"token {name} needs expression, value, or both light/dark values")
        normalized[str(name)] = dict(entry, type=token_type)

    class_name = str(raw.get("class", "GeneratedTokens"))
    if not IDENTIFIER.match(class_name):
        fail(f"invalid class name: {class_name!r}")
    namespace = str(raw.get("namespace", "LfQuick")).strip()
    for part in filter(None, namespace.split("::")):
        if not IDENTIFIER.match(part):
            fail(f"invalid namespace segment: {part!r}")

    return {"tokens": normalized, "class": class_name, "namespace": namespace}


def qml_literal(value, token_type: str) -> str:
    if token_type in {"color", "string"}:
        return json.dumps(str(value), ensure_ascii=False)
    if token_type == "bool":
        return "true" if bool(value) else "false"
    return str(value)


def qml_expression(entry: dict) -> str:
    if "expression" in entry:
        return str(entry["expression"])
    if "light" in entry and "dark" in entry:
        return (
            f"Theme.dark ? {qml_literal(entry['dark'], entry['type'])}"
            f" : {qml_literal(entry['light'], entry['type'])}"
        )
    return qml_literal(entry.get("value"), entry["type"])


def render_qml(spec: dict) -> str:
    lines = ["pragma Singleton", "import QtQuick", "", "QtObject {"]
    for name, entry in spec["tokens"].items():
        lines.append(f"    readonly property {entry['type']} {name}: {qml_expression(entry)}")
    lines.extend(["}", ""])
    return "\n".join(lines)


def cpp_type(token_type: str) -> str:
    return {
        "color": "QColor",
        "int": "int",
        "real": "double",
        "bool": "bool",
        "string": "QString",
    }[token_type]


def cpp_literal(value, token_type: str) -> str:
    if token_type == "color":
        escaped = str(value).replace('"', '\\"')
        return f'QColor(QStringLiteral("{escaped}"))'
    if token_type == "string":
        escaped = str(value).replace('"', '\\"')
        return f'QStringLiteral("{escaped}")'
    if token_type == "bool":
        return "true" if bool(value) else "false"
    return str(value)


def render_cpp(spec: dict) -> str:
    lines = [
        "#pragma once",
        "",
        "#include <QColor>",
        "#include <QString>",
        "",
    ]
    namespace = spec["namespace"]
    if namespace:
        lines.extend([f"namespace {namespace} {{", ""])
    lines.extend([f"struct {spec['class']}", "{"])

    for name, entry in spec["tokens"].items():
        type_name = cpp_type(entry["type"])
        if "cppExpression" in entry:
            lines.append(f"    static {type_name} {name}() {{ return {entry['cppExpression']}; }}")
        elif "light" in entry and "dark" in entry:
            light = cpp_literal(entry["light"], entry["type"])
            dark = cpp_literal(entry["dark"], entry["type"])
            lines.append(f"    static {type_name} {name}(bool dark) {{ return dark ? {dark} : {light}; }}")
        elif "value" in entry:
            value = cpp_literal(entry["value"], entry["type"])
            lines.append(f"    static {type_name} {name}() {{ return {value}; }}")
        else:
            fail(f"token {name} uses a QML-only expression; add cppExpression to generate C++")

    lines.append("};")
    if namespace:
        lines.extend(["", f"}} // namespace {namespace}"])
    lines.append("")
    return "\n".join(lines)


def write(path: Path, content: str, force: bool) -> None:
    if path.exists() and not force:
        fail(f"refusing to overwrite {path}; pass --force")
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(content, encoding="utf-8")


def main() -> int:
    parser = argparse.ArgumentParser(description="Generate QML/C++ semantic tokens")
    parser.add_argument("input", type=Path)
    parser.add_argument("--qml", type=Path, required=True, help="output QML singleton path")
    parser.add_argument("--cpp", type=Path, help="optional generated C++ header path")
    parser.add_argument("--force", action="store_true")
    args = parser.parse_args()

    spec = validate(load_input(args.input))
    write(args.qml, render_qml(spec), args.force)
    print(f"generated {args.qml}")
    if args.cpp:
        write(args.cpp, render_cpp(spec), args.force)
        print(f"generated {args.cpp}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
