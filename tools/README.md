# LfQuick Developer Tools

These tools are intentionally local/reusable and do not depend on GitHub Actions.

## `lfquick_lint.py`

Runs the architecture boundary checker plus lightweight QML source checks.

```bash
python tools/lfquick_lint.py
python tools/lfquick_lint.py --strict-colors
python tools/lfquick_lint.py --qmllint C:/Qt/6.8.3/msvc2022_64/bin/qmllint.exe
```

Default behavior reports hard-coded QML colors outside `Tokens.qml` as warnings. `--strict-colors` upgrades them to errors so existing code can be migrated incrementally.

## `lfquick_gen_vm.py`

Generates a QObject ViewModel header/source pair from JSON.

```json
{
  "namespace": "Demo",
  "class": "SampleViewModel",
  "qmlElement": true,
  "properties": [
    {"name": "sampleId", "type": "QString"},
    {"name": "busy", "type": "bool", "default": "false"}
  ]
}
```

```bash
python tools/lfquick_gen_vm.py SampleViewModel.json --out generated
```

Use `--force` only when overwriting generated files is intentional.

## `lfquick_gen_token.py`

Generates a QML singleton from JSON or YAML and can optionally generate a C++ token header.

```bash
python tools/lfquick_gen_token.py tokens.json \
  --qml generated/Tokens.qml \
  --cpp generated/GeneratedTokens.h
```

YAML input is optional and requires PyYAML. JSON has no extra dependency.

A token may provide:

- `value` for a constant;
- `light` + `dark` for color-scheme values;
- `expression` for a QML expression;
- `cppExpression` when a QML-only expression also needs a C++ representation.

## `lfquick_deploy_check.py`

Validates a staged package using a JSON manifest.

```bash
python tools/lfquick_deploy_check.py package/LfQuick-Demos-Windows-x64 \
  --manifest tools/deploy_manifest_demos.json
```

Add `--json` for machine-readable output. The checker supports required globs, required-any groups, forbidden globs, package-size limits and zero-byte-file diagnostics.

## `lfquick-screenshot`

Optional C++ CLI for deterministic QML screenshots. Build it with:

```bash
cmake -S . -B build -DLFQUICK_BUILD_SCREENSHOT_TOOL=ON
cmake --build build --target lfquick-screenshot
```

Example:

```bash
lfquick-screenshot \
  --source path/to/Preview.qml \
  --output artifacts/preview.png \
  --width 1280 --height 720 \
  --import-path build/qml
```

The QML source root must be a `QQuickItem`. Use repeated `--import-path` arguments when the preview depends on generated/private build-tree modules.

## Self-tests

Python tool self-tests live in `tools/test_tooling.py`. When Python is available, CMake registers them in CTest as `tooling_python_selftest`.
