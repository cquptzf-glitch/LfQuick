#!/usr/bin/env python3
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]

# Module-specific technology boundaries. Keep the foundation graph monotonic:
# Core -> Runtime -> UI/DataView, while Widgets/Quick/version-specific details
# stay in leaf adapters.
RULES = {
    "src/core": [
        r"#include\s*[<\"]QQuick",
        r"#include\s*[<\"]QWidget",
        r"Qt6::Quick",
        r"Qt6::Widgets",
        r"LfQuick/(Runtime|Ui|DataView|Platform|WidgetBridge|Charts)/",
        r"LfQuick::(Runtime|Ui|DataView|Platform|WidgetBridge|Charts)",
    ],
    "src/runtime": [
        r"#include\s*[<\"]QQuick",
        r"#include\s*[<\"]QWidget",
        r"#include\s*[<\"]QWindow",
        r"Qt6::Quick",
        r"Qt6::Widgets",
        r"LfQuick/(Ui|DataView|Platform|WidgetBridge|Charts)/",
        r"LfQuick::(Ui|DataView|Platform|WidgetBridge|Charts)",
    ],
    "src/ui": [
        r"LfQuick/(WidgetBridge)/",
        r"LfQuick::WidgetBridge",
    ],
}

# Diagnostics, development tooling, styles, cross-module integration and
# version/technology-specific adapters are leaves. Foundation modules must
# never depend back on them.
LEAF_CPP_PATTERNS = [
    r"LfQuick/(Diagnostics|Tooling|StyleKit|QtAdapters|Integration)/",
    r"LfQuick::(Diagnostics|Tooling|StyleKit|Integration|RangeModelAdapter|CanvasPainterAdapter|SafeAreaAdapter|PlatformQml|CustomQuickCharts|LegacyQCustomPlotAdapter|FluentStyle|BrandTestStyle)",
]
LEAF_QML_PATTERN = re.compile(
    r"^\s*import\s+LfQuick\.(Diagnostics|Tooling|StyleKit|Integration|Platform|SafeArea|CanvasPainter|CustomQuickCharts|Fluent|BrandTest)\b",
    re.MULTILINE,
)

FOUNDATION_MODULES = [
    "src/core",
    "src/runtime",
    "src/ui",
    "src/dataview",
    "src/platform",
    "src/widgetbridge",
    "src/charts",
]
for rel in FOUNDATION_MODULES:
    RULES.setdefault(rel, []).extend(LEAF_CPP_PATTERNS)

violations: list[str] = []

for rel, patterns in RULES.items():
    module_root = ROOT / rel
    if not module_root.exists():
        continue
    for path in module_root.rglob("*"):
        if not path.is_file():
            continue
        if path.suffix not in {".h", ".hpp", ".cpp", ".cmake", ".txt", ".qml"} and path.name != "CMakeLists.txt":
            continue

        text = path.read_text(encoding="utf-8", errors="ignore")
        for pattern in patterns:
            if re.search(pattern, text):
                violations.append(f"{path.relative_to(ROOT)} violates {pattern}")

        if path.suffix == ".qml" and LEAF_QML_PATTERN.search(text):
            violations.append(
                f"{path.relative_to(ROOT)} imports an optional leaf QML module"
            )

# `LfQuick.impl` is an internal namespace and may never become a public QML
# dependency, regardless of which source module contains the QML file.
for source_root in (ROOT / "src", ROOT / "examples"):
    if not source_root.exists():
        continue
    for path in source_root.rglob("*.qml"):
        text = path.read_text(encoding="utf-8", errors="ignore")
        if re.search(r"^\s*import\s+LfQuick\.impl\b", text, re.MULTILINE):
            violations.append(f"{path.relative_to(ROOT)} imports forbidden LfQuick.impl")

if violations:
    print("Architecture boundary check FAILED:")
    print("\n".join(f" - {v}" for v in violations))
    sys.exit(1)

print("Architecture boundary check: PASS")
