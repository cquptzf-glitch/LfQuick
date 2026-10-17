# LfQuick

LfQuick is a modular Qt UI Runtime Framework targeting **Qt 6.8.3** as the primary baseline, with version-gated adapters for newer Qt capabilities.

It is designed for large desktop/device applications that need:

- Qt Quick/QML application runtime;
- gradual QWidget → QML migration;
- technology-stack-neutral page/window routing;
- reusable permission / validation / overlay infrastructure;
- scanner/input/focus integration;
- large-table DataGrid capabilities;
- Qt Graphs visualization adapters;
- structured diagnostics and runtime export;
- development-only QML hot reload with rollback;
- an optional Qt 6.12 StyleKit bridge for shared Quick/Widget styling.

## Current development policy

Development currently proceeds directly on `main`.

**GitHub Actions / CI / release workflows are intentionally removed during the current feature-completion and hardening phase.** Do not infer that the newest HEAD is CI-green. A previous Qt 6.8.3 / MSVC2022 revision did pass full Build + 13/13 CTest + deploy/package validation, but newer Diagnostics/Tooling/StyleKit/hardening changes have not yet been put through the final full build matrix.

The workflows will be restored only after the feature/hardening phase is complete.

## Modules

### `LfQuick::Core`

QtCore-only contracts and primitives:

```text
Error / Result
OperationHandle
Command
Event / Dispatcher / Subscription
Permission
Validation / validateValue
Route contracts
```

### `LfQuick::Runtime`

Technology-stack-neutral runtime:

```text
PageRegistry / PageProviderRegistry
PageRouter
Lifecycle / History
NavigationGuard
None / KeepAlive / Singleton / weighted LRU
cross-provider fallback
structured provider diagnostics
WindowProviderRegistry / WindowRouter
```

`Runtime` must not depend on QWidget or QQuickItem.

### `LfQuick::Ui` / `LfQuick`

Qt Quick application layer:

```text
QuickRuntime / QuickPageProvider
QmlErrorCollector / PageLoadError
Theme / Tokens / LfPage
OverlayCoordinator / Busy / Toast / Dialog
FocusNavigator
NavigationGuardController
Lf.permission attached context
ValidationRuleTable / ValidationFormState
NavigationItemModel / NavigationView / ApplicationShell
```

### `LfQuick::DataView` / `LfQuick.DataView`

Large-table/view subsystem:

```text
DataTableModel
DataFilterProxyModel
DataGrid
ColumnStateModel / ColumnConfigStore
DataEditTransaction
typed delegates
extended selection
proxy/source row mapping
context menu / keyboard navigation
DataGridTelemetry
```

`DataGridTelemetry` is optional and independent of Diagnostics. It exposes frame/FPS data using Qt 6.8 `FrameAnimation`:

```qml
DataGridTelemetry {
    target: grid
    enabled: pageActive
}
```

Metrics include FPS, current/max frame time, slow-frame count and first-visible time.

### `LfQuick::Platform`

Input/platform adapters:

```text
ScannerSession
KeyboardWedgeScanner
InputRouter
```

### `LfQuick::WidgetBridge`

Optional QWidget/Quick migration layer:

```text
QuickSurface
WidgetPageProvider
WidgetWindowProvider
WidgetFocusBridge
WidgetOverlayBackend
```

`QuickSurface` exposes:

```text
loadDurationMs
firstFrameMs
effectiveDevicePixelRatio
surfaceSize
screenName
resizeCount
```

for mixed-stack performance and multi-DPI hardening.

### `LfQuick::Charts` / `LfQuick.Charts`

Optional visualization contract and Qt Graphs adapter:

```text
ChartDataModel
SeriesModel
QtGraphsSeriesAdapter
QtGraphsAxisAdapter
ChartView.qml
```

For Qt 6.8.3, the Graphs build needs the Qt modules:

```text
qtgraphs
qtquick3d
qtshadertools
```

### `LfQuick::Diagnostics` / `LfQuick.Diagnostics`

Optional leaf module for development/runtime observability:

```text
DiagnosticsModel
DiagnosticsHud
bounded event history
generic metrics / timing
snapshot / JSON
atomic JSON export
```

Foundation modules do not depend on Diagnostics; applications/Demos compose it from the outside.

### `LfQuick::Tooling` / `LfQuick.Tooling`

Development-only hot reload module, enabled with:

```text
-DLFQUICK_ENABLE_HOT_RELOAD=ON
```

Core types:

```text
QmlHotReloadController
QmlHotReloadHost
```

Features:

- logical resource URL → local development file mapping;
- file + directory watching for editor atomic-save workflows;
- debounce and QML component-cache invalidation;
- reference-counted shared source watches;
- candidate-first atomic UI replacement;
- selected-property state preservation;
- failed reload rollback that keeps the previous UI alive.

Example:

```qml
import QtQuick
import LfQuick.Tooling

QmlHotReloadController {
    id: reloader
    debounceMs: 120
}

QmlHotReloadHost {
    anchors.fill: parent
    controller: reloader
    source: "qrc:/qt/qml/MyApp/EditorPage.qml"
    developmentSource: devEditorPageUrl   // local file URL supplied by dev configuration
    preservedProperties: ["selectedIndex", "filterText"]
}
```

Hot reload is not part of Core/Runtime production lifecycle and is OFF by default.

### `LfQuick::StyleKit` / `LfQuick.StyleKit`

Qt 6.12-only experimental adapter, enabled with:

```text
-DLFQUICK_EXPERIMENTAL_STYLEKIT=ON
```

`StyleKitBridge` applies a Qt Labs StyleKit style to QWidget through `QStyleKitStyle`, allowing one Style QML definition to drive both Qt Quick Controls and Qt Widgets.

The adapter is version-gated and does not expose `QStyleKitStyle` in the Qt 6.8-compatible public API. Qt Labs StyleKit is technology preview, so this option remains OFF by default.

## Architecture boundaries

```text
LfQuickCore        -> QtCore only
LfQuickRuntime     -> Core; no QWidget/QQuickItem
LfQuickUi          -> Core + Runtime + Qt Quick
LfQuickDataView    -> Ui + Qt Model/View/Quick
LfQuickPlatform    -> platform/input adapters
LfQuickWidgetBridge-> optional Widgets / QuickWidgets
LfQuickCharts      -> optional visualization
Diagnostics        -> optional leaf
Tooling            -> optional development leaf
StyleKit           -> optional Qt 6.12 leaf
Product adapters   -> legacy/product dependencies
```

`tools/check_architecture.py` also prevents Core/Runtime/UI/DataView/Platform/WidgetBridge/Charts from acquiring reverse dependencies on Diagnostics, Tooling or StyleKit.

## Build

Default Qt 6.8.3-oriented build:

```bash
cmake -S . -B build
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

Enable the migration demo:

```bash
cmake -S . -B build \
  -DLFQUICK_BUILD_WIDGET_BRIDGE=ON \
  -DLFQUICK_BUILD_HYBRID_DEMO=ON
cmake --build build --parallel
```

Enable Charts / Qt Graphs adapter:

```bash
cmake -S . -B build \
  -DLFQUICK_BUILD_CHARTS=ON \
  -DLFQUICK_BUILD_QT_GRAPHS_ADAPTER=ON
```

Enable development Hot Reload:

```bash
cmake -S . -B build \
  -DLFQUICK_ENABLE_HOT_RELOAD=ON
```

Enable benchmarks:

```bash
cmake -S . -B build \
  -DLFQUICK_BUILD_BENCHMARKS=ON
```

Enable Qt 6.12 StyleKit experiment only with an appropriate Qt 6.12 installation:

```bash
cmake -S . -B build \
  -DLFQUICK_EXPERIMENTAL_STYLEKIT=ON
```

## Important CMake options

```text
LFQUICK_BUILD_PLATFORM
LFQUICK_BUILD_UI
LFQUICK_BUILD_DATAVIEW
LFQUICK_BUILD_WIDGET_BRIDGE
LFQUICK_BUILD_HYBRID_DEMO
LFQUICK_BUILD_CHARTS
LFQUICK_BUILD_QT_GRAPHS_ADAPTER
LFQUICK_BUILD_DIAGNOSTICS
LFQUICK_BUILD_MEDICAL_DEMO
LFQUICK_BUILD_BENCHMARKS
LFQUICK_ENABLE_HOT_RELOAD
LFQUICK_ENABLE_QMLTC
LFQUICK_EXPERIMENTAL_STYLEKIT
```

## Medical Demo

`examples/medical-demo` is the main vertical reference application for analyzer-style desktop/device UI workflows. It uses **synthetic data only** and is **not medical diagnostic software**.

Current pages include:

```text
Dashboard
Samples
Sample Register
Sample Rack
Results
Results Review
Charts (when Graphs adapter is enabled)
Instrument
Alarms
Developer
```

The demo validates Scanner/InputRouter, permission, validation, navigation guard, overlay, DataGrid, Charts and Diagnostics together.

## Hybrid Demo

`examples/hybrid-demo` validates gradual QWidget migration:

```text
QML Page <-> QuickRuntime/PageRouter <-> Legacy QWidget Page
```

It also exercises:

```text
WidgetFocusBridge
WidgetOverlayBackend
WindowRouter / WidgetWindowProvider
QuickSurface telemetry
fallback from broken QML to legacy QWidget
Diagnostics JSON export
```

## Hardening

Automated/source-level hardening currently includes:

- DataView 20k/50k × 8/20/40-column benchmark matrix;
- process RSS before/after DataTableModel population;
- DataGrid FPS/slow-frame telemetry;
- repeated WidgetFocusBridge focus transfer stress;
- scanner isolation + modal focus trap stress;
- QuickSurface repeated resize/root-size consistency;
- Hot Reload success/state-preserve/failure-rollback tests;
- Diagnostics bounded model/export tests.

Multi-DPI, compositor and native z-order behavior require real desktop verification. See:

- [`tests/manual/WIDGETBRIDGE_HARDENING.md`](tests/manual/WIDGETBRIDGE_HARDENING.md)

## Qt version gates

Current feature detection:

```text
SafeArea                  >= Qt 6.9
QRangeModel               >= Qt 6.10
QCanvasPainter            >= Qt 6.11
QRangeModel drag/drop     >= Qt 6.12
qt_add_qml_preview        >= Qt 6.12
Qt Labs StyleKit          >= Qt 6.12
```

The primary API baseline remains Qt 6.8.3.

## External reference policy

LfQuick may learn from public Qt/Fluent implementations, but external references must follow the project's clean-room and license rules:

- public MIT `zhuzichu520/FluentUI` source may be studied; copied/substantially-derived code requires the relevant MIT notice/source record;
- `FluentUI2/FluentUI Pro` is treated as a clean-room public API/behavior reference unless separate compatible source licensing exists;
- decompilation or reconstruction of proprietary/binary implementations is not permitted;
- LfQuick architecture takes priority over source-level similarity to reference projects.

See [External Reference & Clean-room Policy](docs/REFERENCE_POLICY.md).

## Documents

- [Implementation Status](docs/IMPLEMENTATION_STATUS.md)
- [Next Chat Handoff](docs/NEXT_CHAT_HANDOFF.md)
- [Architecture Review & V2.1 Optimization](docs/LfQuick_V2.1_架构审查与优化方案.md)
- [Development Plan](docs/LfQuick_详细开发计划.md)
- [Medical Demo Design](docs/LfQuick_Medical_Demo_详细设计方案.md)
- [WidgetBridge hardening checklist](tests/manual/WIDGETBRIDGE_HARDENING.md)
- [Hybrid Demo](examples/hybrid-demo/README.md)
- [External Reference & Clean-room Policy](docs/REFERENCE_POLICY.md)
- [Third-party Notices](THIRD_PARTY_NOTICES.md)
