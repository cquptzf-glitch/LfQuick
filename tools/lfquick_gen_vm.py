#!/usr/bin/env python3
"""Generate a small QObject ViewModel from a JSON specification.

Example spec:
{
  "namespace": "Demo",
  "class": "SampleViewModel",
  "qmlElement": true,
  "properties": [
    {"name": "sampleId", "type": "QString"},
    {"name": "busy", "type": "bool", "default": "false"}
  ]
}
"""

from __future__ import annotations

import argparse
import json
import re
from pathlib import Path

IDENTIFIER = re.compile(r"^[A-Za-z_][A-Za-z0-9_]*$")
TYPE_INCLUDE = {
    "QString": "QString",
    "QStringList": "QStringList",
    "QVariant": "QVariant",
    "QVariantList": "QVariantList",
    "QVariantMap": "QVariantMap",
    "QDateTime": "QDateTime",
    "QUrl": "QUrl",
    "QColor": "QColor",
    "QSize": "QSize",
    "QPoint": "QPoint",
    "QRect": "QRect",
}


def fail(message: str) -> "NoReturn":
    raise SystemExit(f"lfquick-gen-vm: {message}")


def validate_identifier(value: str, label: str) -> str:
    if not IDENTIFIER.match(value):
        fail(f"invalid {label}: {value!r}")
    return value


def setter_name(name: str) -> str:
    return "set" + name[0].upper() + name[1:]


def signal_name(name: str) -> str:
    return name + "Changed"


def load_spec(path: Path) -> dict:
    try:
        data = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as exc:
        fail(f"cannot read spec {path}: {exc}")
    if not isinstance(data, dict):
        fail("spec root must be a JSON object")
    return data


def normalize_spec(raw: dict) -> dict:
    class_name = validate_identifier(str(raw.get("class", "")), "class name")
    namespace = str(raw.get("namespace", "")).strip()
    if namespace:
        for part in namespace.split("::"):
            validate_identifier(part, "namespace segment")

    properties = raw.get("properties", [])
    if not isinstance(properties, list):
        fail("properties must be an array")

    normalized_properties: list[dict] = []
    seen: set[str] = set()
    for entry in properties:
        if not isinstance(entry, dict):
            fail("every property must be an object")
        name = validate_identifier(str(entry.get("name", "")), "property name")
        if name in seen:
            fail(f"duplicate property: {name}")
        seen.add(name)
        type_name = str(entry.get("type", "")).strip()
        if not type_name:
            fail(f"property {name} has no type")
        normalized_properties.append(
            {
                "name": name,
                "type": type_name,
                "default": str(entry.get("default", "{}")),
                "readOnly": bool(entry.get("readOnly", False)),
            }
        )

    includes = [str(value).strip() for value in raw.get("includes", []) if str(value).strip()]
    return {
        "class": class_name,
        "namespace": namespace,
        "qmlElement": bool(raw.get("qmlElement", False)),
        "properties": normalized_properties,
        "includes": includes,
    }


def render_header(spec: dict) -> str:
    class_name = spec["class"]
    includes = {"QObject"}
    for prop in spec["properties"]:
        include = TYPE_INCLUDE.get(prop["type"])
        if include:
            includes.add(include)
    includes.update(spec["includes"])
    if spec["qmlElement"]:
        includes.add("QtQmlIntegration/qqmlintegration.h")

    lines = ["#pragma once", ""]
    for include in sorted(includes):
        lines.append(f"#include <{include}>")
    lines.extend(["", "#include <utility>", ""])

    if spec["namespace"]:
        lines.extend([f"namespace {spec['namespace']} {{", ""])

    lines.extend([f"class {class_name} : public QObject", "{", "    Q_OBJECT"])
    if spec["qmlElement"]:
        lines.append("    QML_ELEMENT")

    for prop in spec["properties"]:
        write = "" if prop["readOnly"] else f" WRITE {setter_name(prop['name'])}"
        lines.append(
            f"    Q_PROPERTY({prop['type']} {prop['name']} READ {prop['name']}"
            f"{write} NOTIFY {signal_name(prop['name'])} FINAL)"
        )

    lines.extend(["", "public:", f"    explicit {class_name}(QObject *parent = nullptr);"])
    if spec["properties"]:
        lines.append("")

    for prop in spec["properties"]:
        lines.append(f"    {prop['type']} {prop['name']}() const {{ return m_{prop['name']}; }}")
        if not prop["readOnly"]:
            lines.append(f"    void {setter_name(prop['name'])}({prop['type']} value);")

    if spec["properties"]:
        lines.extend(["", "signals:"])
        for prop in spec["properties"]:
            lines.append(f"    void {signal_name(prop['name'])}();")

        lines.extend(["", "private:"])
        for prop in spec["properties"]:
            lines.append(f"    {prop['type']} m_{prop['name']} = {prop['default']};")

    lines.append("};")
    if spec["namespace"]:
        lines.extend(["", f"}} // namespace {spec['namespace']}"])
    lines.append("")
    return "\n".join(lines)


def render_cpp(spec: dict) -> str:
    class_name = spec["class"]
    qualifier = f"{spec['namespace']}::{class_name}" if spec["namespace"] else class_name
    lines = [f'#include "{class_name}.h"', ""]
    lines.extend(
        [
            f"{qualifier}::{class_name}(QObject *parent)",
            "    : QObject(parent)",
            "{",
            "}",
        ]
    )

    for prop in spec["properties"]:
        if prop["readOnly"]:
            continue
        name = prop["name"]
        lines.extend(
            [
                "",
                f"void {qualifier}::{setter_name(name)}({prop['type']} value)",
                "{",
                f"    if (m_{name} == value)",
                "        return;",
                f"    m_{name} = std::move(value);",
                f"    emit {signal_name(name)}();",
                "}",
            ]
        )
    lines.append("")
    return "\n".join(lines)


def write_file(path: Path, content: str, force: bool) -> None:
    if path.exists() and not force:
        fail(f"refusing to overwrite {path}; pass --force")
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(content, encoding="utf-8", newline="\n")


def main() -> int:
    parser = argparse.ArgumentParser(description="Generate a QObject ViewModel from JSON")
    parser.add_argument("spec", type=Path, help="JSON specification")
    parser.add_argument("--out", type=Path, default=Path.cwd(), help="output directory")
    parser.add_argument("--force", action="store_true", help="overwrite existing generated files")
    args = parser.parse_args()

    spec = normalize_spec(load_spec(args.spec))
    header = args.out / f"{spec['class']}.h"
    source = args.out / f"{spec['class']}.cpp"
    write_file(header, render_header(spec), args.force)
    write_file(source, render_cpp(spec), args.force)
    print(f"generated {header}")
    print(f"generated {source}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
