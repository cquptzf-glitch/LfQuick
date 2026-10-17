#!/usr/bin/env python3
"""LfQuick repository lint driver.

Runs architecture boundaries unconditionally and layers lightweight source checks
on top. Optional qmllint integration is intentionally opt-in so the script stays
usable on machines that only have Python installed.
"""

from __future__ import annotations

import argparse
import re
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
QML_ROOTS = [ROOT / "src", ROOT / "examples"]
MAGIC_COLOR = re.compile(r"#[0-9A-Fa-f]{6}(?:[0-9A-Fa-f]{2})?\b")
FORBIDDEN_IMPORTS = ("LfQuick.impl",)


def qml_files() -> list[Path]:
    result: list[Path] = []
    for root in QML_ROOTS:
        if root.exists():
            result.extend(root.rglob("*.qml"))
    return sorted(set(result))


def run_architecture() -> bool:
    command = [sys.executable, str(ROOT / "tools" / "check_architecture.py")]
    return subprocess.run(command, cwd=ROOT, check=False).returncode == 0


def scan_qml(strict_colors: bool) -> tuple[list[str], list[str]]:
    errors: list[str] = []
    warnings: list[str] = []

    for path in qml_files():
        text = path.read_text(encoding="utf-8", errors="replace")
        rel = path.relative_to(ROOT)

        for line_number, line in enumerate(text.splitlines(), start=1):
            stripped = line.strip()
            if stripped.startswith("import "):
                for forbidden in FORBIDDEN_IMPORTS:
                    if stripped.split()[1].startswith(forbidden):
                        errors.append(f"{rel}:{line_number}: forbidden import {forbidden}")

            if path.name != "Tokens.qml" and MAGIC_COLOR.search(line):
                message = f"{rel}:{line_number}: hard-coded color literal outside Tokens.qml"
                (errors if strict_colors else warnings).append(message)

            if line.rstrip() != line:
                warnings.append(f"{rel}:{line_number}: trailing whitespace")

    return errors, warnings


def run_qmllint(executable: str) -> list[str]:
    errors: list[str] = []
    for path in qml_files():
        result = subprocess.run(
            [executable, str(path)],
            cwd=ROOT,
            text=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            check=False,
        )
        if result.returncode != 0:
            output = result.stdout.strip()
            errors.append(f"qmllint failed for {path.relative_to(ROOT)}\n{output}")
    return errors


def main() -> int:
    parser = argparse.ArgumentParser(description="Run LfQuick architecture and QML lint checks")
    parser.add_argument(
        "--strict-colors",
        action="store_true",
        help="treat hard-coded QML color literals outside Tokens.qml as errors",
    )
    parser.add_argument(
        "--qmllint",
        nargs="?",
        const="qmllint",
        help="also run qmllint; optionally provide the executable path",
    )
    args = parser.parse_args()

    ok = run_architecture()
    errors, warnings = scan_qml(args.strict_colors)

    if args.qmllint:
        executable = shutil.which(args.qmllint) if Path(args.qmllint).name == args.qmllint else args.qmllint
        if not executable or not Path(executable).exists():
            errors.append(f"qmllint executable not found: {args.qmllint}")
        else:
            errors.extend(run_qmllint(executable))

    if warnings:
        print("LfQuick lint warnings:")
        for warning in warnings:
            print(f" - {warning}")

    if errors:
        print("LfQuick lint errors:")
        for error in errors:
            print(f" - {error}")
        ok = False

    if ok:
        print("LfQuick lint: PASS")
        return 0

    print("LfQuick lint: FAILED")
    return 1


if __name__ == "__main__":
    raise SystemExit(main())
