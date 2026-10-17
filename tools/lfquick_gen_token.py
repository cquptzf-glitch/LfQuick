#!/usr/bin/env python3
"""Generate LfQuick token artifacts from JSON or YAML.

The generator accepts either the original flat ``tokens`` object or the
three-layer schema used by the design system:

{
  "primitive": { ... },
  "semantic": { ... },
  "component": { ... }
}

Outputs:
- QML singleton (required)
- optional C++ header
- optional legacy QSS rendered from a ${tokenName} template

Dynamic QML expressions must provide a static ``cppExpression`` for C++ and a
``qssValue`` (or qssLight/qssDark) when referenced by a QSS template.
"""

from __future__ import annotations

import argparse
import json
import re
from pathlib import Path

IDENTIFIER = re.compile(r"^[A-Za-z_][A-Za-z0-9_]*$")
PLACEHOLDER = re.compile(r"\$\{([A-Za-z_][A-Za-z0-9_]*)\}")
QML_TYPES = {"color", "int", "real", "bool", "string"}
TOKEN_LAYERS = ("primitive", "semantic", "component")


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


def collect_tokens(raw: dict) -> dict[str, dict]:
    flat = raw.get("tokens")
    layered = any(layer in raw for layer in TOKEN_LAYERS)
    if flat is not None and layered:
        fail("use either flat 'tokens' or primitive/semantic/component layers, not both")

    if flat is not None:
        if not isinstance(flat, dict) or not flat:
            fail("tokens must be a non-empty object")
        return {str(name): dict(entry, layer="semantic") if isinstance(entry, dict) else entry
                for name, entry in flat.items()}

    result: dict[str, dict] = {}
    for layer in TOKEN_LAYERS:
        entries = raw.get(layer, {})
        if entries is None:
            continue
        if not isinstance(entries, dict):
            fail(f"{layer} must be an object")
        for name, entry in entries.items():
            token_name = str(name)
            if token_name in result:
                fail(f"duplicate token across layers: {token_name}")
            if not isinstance(entry, dict):
                fail(f"token {token_name} must be an object")
            result[token_name] = dict(entry, layer=layer)

    if not result:
        fail("define tokens or at least one primitive/semantic/component layer")
    return result


def validate(raw: dict) -> dict:
    tokens = collect_tokens(raw)
    normalized: dict[str, dict] = {}
    for name, entry in tokens.items():
        if not IDENTIFIER.match(name):
            fail(f"invalid token name: {name!r}")
        if not isinstance(entry, dict):
            fail(f"token {name} must be an object")
        token_type = str(entry.get("type", "")).strip()
        if token_type not in QML_TYPES:
            fail(f"token {name} has unsupported type {token_type!r}")
        if "expression" not in entry and "value" not in entry and not {"light", "dark"} <= set(entry):
            fail(f"token {name} needs expression, value, or both light/dark values")
        normalized[name] = dict(entry, type=token_type, layer=str(entry.get("layer", "semantic")))

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
    last_layer = None
    for name, entry in spec["tokens"].items():
        layer = entry.get("layer", "semantic")
        if layer != last_layer:
            if last_layer is not None:
                lines.append("")
            lines.append(f"    // {layer} tokens")
            last_layer = layer
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

    last_layer = None
    for name, entry in spec["tokens"].items():
        layer = entry.get("layer", "semantic")
        if layer != last_layer:
            lines.append(f"    // {layer} tokens")
            last_layer = layer

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


def qss_value(entry: dict, theme: str, name: str) -> str:
    if theme == "dark" and "qssDark" in entry:
        return str(entry["qssDark"])
    if theme == "light" and "qssLight" in entry:
        return str(entry["qssLight"])
    if "qssValue" in entry:
        return str(entry["qssValue"])
    if theme in entry:
        return str(entry[theme])
    if "value" in entry:
        value = entry["value"]
        if entry["type"] == "bool":
            return "1" if bool(value) else "0"
        return str(value)
    fail(f"token {name} has no static value for QSS theme {theme}; add qssValue/qssLight/qssDark")


def render_qss(spec: dict, template: str, theme: str) -> str:
    referenced = set(PLACEHOLDER.findall(template))
    unknown = sorted(referenced - set(spec["tokens"]))
    if unknown:
        fail("QSS template references unknown tokens: " + ", ".join(unknown))

    def replace(match: re.Match[str]) -> str:
        name = match.group(1)
        return qss_value(spec["tokens"][name], theme, name)

    header = f"/* Generated by lfquick-gen-token · theme={theme}. Do not edit. */\n"
    return header + PLACEHOLDER.sub(replace, template)


def write(path: Path, content: str, force: bool) -> None:
    if path.exists() and not force:
        fail(f"refusing to overwrite {path}; pass --force")
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(content, encoding="utf-8", newline="\n")


def main() -> int:
    parser = argparse.ArgumentParser(description="Generate QML/C++/QSS design tokens")
    parser.add_argument("input", type=Path)
    parser.add_argument("--qml", type=Path, required=True, help="output QML singleton path")
    parser.add_argument("--cpp", type=Path, help="optional generated C++ header path")
    parser.add_argument("--qss", type=Path, help="optional rendered legacy QSS path")
    parser.add_argument("--qss-template", type=Path, help="QSS template containing ${tokenName} placeholders")
    parser.add_argument("--theme", choices=("light", "dark"), default="light", help="theme used for QSS rendering")
    parser.add_argument("--force", action="store_true")
    args = parser.parse_args()

    if bool(args.qss) != bool(args.qss_template):
        fail("--qss and --qss-template must be provided together")

    spec = validate(load_input(args.input))
    write(args.qml, render_qml(spec), args.force)
    print(f"generated {args.qml}")

    if args.cpp:
        write(args.cpp, render_cpp(spec), args.force)
        print(f"generated {args.cpp}")

    if args.qss:
        try:
            template = args.qss_template.read_text(encoding="utf-8")
        except OSError as exc:
            fail(f"cannot read QSS template {args.qss_template}: {exc}")
        write(args.qss, render_qss(spec, template, args.theme), args.force)
        print(f"generated {args.qss}")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
