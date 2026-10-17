# LfQuick Hybrid Demo

This example validates gradual QWidget → QML migration while both stacks share LfQuick routing, lifecycle, overlays and diagnostics.

## What it demonstrates

### Shared page runtime

```text
QML Page <-> QuickRuntime / PageRouter <-> Legacy QWidget Page
```

The demo registers both:

- `QuickPageProvider`;
- `WidgetPageProvider`.

It exercises:

- common route/history APIs;
- `KeepAlive` lifecycle across UI technologies;
- explicit fallback from a deliberately missing QML page to a legacy QWidget page;
- shared back navigation.

### QuickSurface

`QuickSurface` embeds the single QML host into the QWidget shell and exposes:

```text
ready / loadState / lastError
loadDurationMs
firstFrameMs
effectiveDevicePixelRatio
surfaceSize
screenName
resizeCount
```

These metrics are intended for migration hardening, especially resize and multi-DPI scenarios.

### Focus bridge

`WidgetFocusBridge` demonstrates:

```text
QWidget -> QML Tab transfer
QML -> QWidget Tab/Backtab transfer
remember / restore
scanner focus isolation
modal focus trap
```

### Shared overlays

The same `OverlayCoordinator` protocol is rendered through `WidgetOverlayBackend`:

- Toast;
- non-blocking dialog.

This validates that overlay request semantics do not depend on the rendering stack.

### WindowRouter

The toolbar can open real top-level QWidget windows through:

```text
WindowRouter
WindowProviderRegistry
WidgetWindowProvider
```

Scenarios include:

- single-instance Inspector reuse;
- multiple owned child windows;
- owner relationship;
- native title-bar close routed back through `WindowRouter`;
- close guard;
- close-all / owner-close propagation.

### Diagnostics

When `LFQUICK_BUILD_DIAGNOSTICS=ON` the demo records:

```text
QuickSurface load / first frame
focus transfer
route changes
QML load errors
fallback lifecycle
WindowRouter opened/closed/blocked/failed
```

Use **Export Diagnostics** to write a JSON snapshot to the system temporary directory.

Diagnostics is composed by the demo; `WidgetBridge` itself does not depend on the Diagnostics module.

## Build

```bash
cmake -S . -B build \
  -DLFQUICK_BUILD_WIDGET_BRIDGE=ON \
  -DLFQUICK_BUILD_HYBRID_DEMO=ON \
  -DLFQUICK_BUILD_DIAGNOSTICS=ON
cmake --build build --parallel
```

The repository currently has GitHub Actions intentionally removed during the feature/hardening phase. Build the demo locally when validating current HEAD.

## Hardening

Automated/source-level stress coverage includes repeated focus transfers and QuickSurface resize checks.

Real desktop behaviors such as compositor stacking and cross-monitor DPI transitions should be validated with:

- [`../../tests/manual/WIDGETBRIDGE_HARDENING.md`](../../tests/manual/WIDGETBRIDGE_HARDENING.md)

Recommended real-device cases include:

- 100% ↔ 150/175/200% monitor transitions;
- QMenu / QComboBox / QToolTip / QMessageBox stacking over QuickSurface;
- repeated top-level WindowRouter open/close;
- modal focus confinement;
- 30–60 minute mixed QWidget/QML soak.

## Architecture boundary

The demo is intentionally framework-focused. Product-specific adapters such as legacy application widgets, analyzer scanner protocols, print/export backends or a product window manager belong in a product integration module, not in `LfQuickWidgetBridge` or `LfQuickRuntime`.
