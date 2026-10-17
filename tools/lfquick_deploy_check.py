#!/usr/bin/env python3
"""Validate a staged LfQuick deployment against a small JSON manifest."""

from __future__ import annotations

import argparse
import json
import os
from pathlib import Path


def fail(message: str) -> None:
    raise SystemExit(f"lfquick-deploy-check: {message}")


def load_manifest(path: Path) -> dict:
    try:
        data = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as exc:
        fail(f"cannot read manifest {path}: {exc}")
    if not isinstance(data, dict):
        fail("manifest root must be an object")
    return data


def matches(root: Path, pattern: str) -> list[Path]:
    return sorted(path for path in root.glob(pattern) if path.exists())


def check(root: Path, manifest: dict) -> dict:
    errors: list[str] = []
    warnings: list[str] = []
    matched: dict[str, list[str]] = {}

    for pattern in manifest.get("required", []):
        found = matches(root, str(pattern))
        matched[str(pattern)] = [str(path.relative_to(root)) for path in found]
        if not found:
            errors.append(f"required pattern missing: {pattern}")

    for group in manifest.get("requiredAny", []):
        if not isinstance(group, list) or not group:
            errors.append("requiredAny entries must be non-empty arrays")
            continue
        found_group: list[Path] = []
        for pattern in group:
            found_group.extend(matches(root, str(pattern)))
        if not found_group:
            errors.append("none of requiredAny patterns matched: " + ", ".join(map(str, group)))

    for pattern in manifest.get("forbidden", []):
        found = matches(root, str(pattern))
        if found:
            errors.append(
                f"forbidden pattern matched {pattern}: "
                + ", ".join(str(path.relative_to(root)) for path in found[:10])
            )

    if manifest.get("rejectZeroByteFiles", True):
        for path in root.rglob("*"):
            if path.is_file() and path.stat().st_size == 0:
                warnings.append(f"zero-byte file: {path.relative_to(root)}")

    maximum = manifest.get("maximumPackageBytes")
    total_bytes = sum(path.stat().st_size for path in root.rglob("*") if path.is_file())
    if maximum is not None and total_bytes > int(maximum):
        errors.append(f"package size {total_bytes} exceeds maximum {int(maximum)} bytes")

    return {
        "root": str(root),
        "ok": not errors,
        "errors": errors,
        "warnings": warnings,
        "totalBytes": total_bytes,
        "matched": matched,
    }


def main() -> int:
    parser = argparse.ArgumentParser(description="Validate staged deployment contents")
    parser.add_argument("root", type=Path, help="staging/package root")
    parser.add_argument("--manifest", type=Path, required=True)
    parser.add_argument("--json", action="store_true", help="emit machine-readable JSON")
    args = parser.parse_args()

    root = args.root.resolve()
    if not root.is_dir():
        fail(f"deployment root does not exist: {root}")

    result = check(root, load_manifest(args.manifest))
    if args.json:
        print(json.dumps(result, indent=2, ensure_ascii=False))
    else:
        print(f"Deployment root: {root}")
        print(f"Size: {result['totalBytes']} bytes")
        for warning in result["warnings"]:
            print(f"WARNING: {warning}")
        for error in result["errors"]:
            print(f"ERROR: {error}")
        print("Deploy check: PASS" if result["ok"] else "Deploy check: FAILED")

    return 0 if result["ok"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
