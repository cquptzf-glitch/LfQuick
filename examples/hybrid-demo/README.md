# LfQuick Hybrid Demo

This example validates the migration path where a QWidget shell hosts both new QML pages and legacy QWidget pages while sharing one `QuickRuntime` / `PageRouter` history.

## What it demonstrates

- `QuickSurface` embedding a QML host inside a QWidget application.
- `QuickPageProvider` and `WidgetPageProvider` registered in one runtime.
- Switching between a QML page and a legacy QWidget page through the same route/history API.
- `KeepAlive` lifecycle behavior across technologies.
- `QuickSurface` load and first-paint timing telemetry.

## Build

```bash
cmake -S . -B build \
  -DLFQUICK_BUILD_WIDGET_BRIDGE=ON \
  -DLFQUICK_BUILD_HYBRID_DEMO=ON
cmake --build build --parallel
```

The demo is intentionally framework-focused. Product-specific adapters such as `LfWidget`, scanner hooks, print/export, and UnionLab `WindowManager` belong in `LfQuick.Integration.UnionLab`, not in `LfQuickWidgetBridge`.
