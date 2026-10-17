# LfQuick

LfQuick is a modular Qt UI Runtime Framework targeting **Qt 6.8.3** as the primary compatibility baseline, with isolated adapters for newer Qt capabilities.

The project is aimed at large desktop/device applications that need:

- Qt Quick/QML application runtime;
- gradual QWidget → QML migration;
- technology-stack-neutral page/window routing;
- style-agnostic UI components and semantic tokens;
- permission / validation / overlay infrastructure;
- scanner/input/focus/platform services;
- large-table DataGrid capabilities;
- optional Qt Graphs visualization;
- structured Diagnostics / telemetry;
- development-only QML hot reload with rollback;
- opt-in adapters for QRangeModel, Canvas2D and Qt Labs StyleKit;
- local generation/lint/deploy/screenshot tooling.

## Current development policy

Development currently proceeds directly on `main`.

**GitHub Actions / CI / release workflows are intentionally removed during the current feature-completion and source-hardening phase.** Do not infer that the newest HEAD is CI-green.

A previous Qt 6.8.3 / MSVC2022 revision did pass:

```text
Architecture
Configure
full Build
13/13 CTest
Qt deployment
package validation
artifact upload
```

A large set of later Design System, Platform, Tooling and Qt-adapter changes has not yet been put through the final full build matrix. The intended order is:

```text
finish source functionality
-> local/full Qt build + CTest + GUI hardening
-> deploy/package verification
-> restore CI/release workflows last
```

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
TelemetryEvent / ILogSink
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
window owner / reparent / close policy / state save+restore hooks
```

`Runtime` must not depend on QWidget, QWindow or QQuickItem.

### `LfQuick::Ui` / `LfQuick`

Qt Quick application layer:

```text
QuickRuntime / QuickPageProvider
QmlErrorCollector / PageLoadError
Theme / Tokens / Units / Typography / Motion / DpiHelper
LfPage
OverlayCoordinator / BusyToken / Toast / Dialog
FocusWalker / FocusManager / FocusNavigator compatibility API
NavigationGuardController
Lf attached context: permission / validationRule / focusPolicy / focusOrder / acceptScanner / helpContext
ValidationEvaluator / LfValidationBinding / ValidationRuleTable / ValidationFormState
HelpContextRegistry / LfHelpShortcut
NavigationItemModel / NavigationView / ApplicationShell
ThemeSystemAdapter / IconLoader
```

The Design System uses semantic `Theme` / `Tokens` and provides reusable controls:

```text
LfButton
LfTextField
LfCheckBox / LfRadioButton / LfSwitch
LfComboBox / LfSlider
LfProgressBar / LfProgressRing
LfSearchBox / LfFilterInput / LfPasswordField
LfNumberInput / LfRangeInput
LfStatusIndicator
LfDatePicker / LfTimePicker
LfDateTimePicker / LfDateRangePicker
```

Theme behavior includes:

```text
Light / Dark / System
Compact / Standard / Comfortable / Touch density
motion enable/disable
high-contrast semantic tokens
accent color
```

### `LfQuick::DataView` / `LfQuick.DataView`

Large-table/view subsystem:

```text
DataTableModel
DataFilterProxyModel
DataGrid
DataGridColumn / DataGridHeader / DataGridViewport
ColumnStateModel / ColumnConfigStore
DataEditTransaction
typed delegates
extended selection
proxy/source row mapping
context menu / keyboard navigation
DataGridTelemetry
```

`DataGridTelemetry` is independent from Diagnostics and exposes frame/FPS data using Qt 6.8 `FrameAnimation`:

```qml
DataGridTelemetry {
    target: grid
    enabled: pageActive
}
```

Metrics include FPS, current/max frame time, slow-frame count and first-visible time.

### `LfQuick::Platform`

Reusable QtCore/Gui input/platform services:

```text
ScannerSession
KeyboardWedgeScanner / MockScanner
InputTarget / InputRouter / HotkeyRouter
SystemAppearance
ScreenService
ClipboardService
FileWatcher
RuntimeCapabilities
```

`FileWatcher` keeps desired file watches alive across editor-style atomic file replacement by also watching parent directories.

`RuntimeCapabilities` reports build-time optional modules/adapters/styles plus Qt-version features, so applications do not need to scatter build-option or Qt-version checks throughout product code.

### `LfQuick::PlatformQml` / `LfQuick.Platform`

Optional QML registration leaf, enabled with:

```text
-DLFQUICK_BUILD_PLATFORM_QML=ON
```

It keeps `LfQuick::Platform` free of QtQml dependencies while exposing Platform services to QML through foreign registration. Declarative helpers include:

```text
ScannerTarget
InputSourceRegistration
```

### `LfQuick::Integration` / `LfQuick.Integration`

Optional cross-module leaf, enabled with:

```text
-DLFQUICK_BUILD_INTEGRATION=ON
```

It is the approved home for adapters that intentionally depend on multiple foundation modules. Current integration:

```text
LfScannerTarget
    Lf.acceptScanner + QQuickItem activeFocus
    -> Platform InputTarget / InputRouter
```

The dependency direction is one-way: `Integration -> Ui + Platform`; foundation modules do not depend back on Integration.

### `LfQuick::WidgetBridge`

Optional QWidget/Quick migration layer:

```text
QuickSurface
WidgetPageProvider
WidgetWindowProvider
WidgetFocusBridge
WidgetOverlayBackend
```

`WidgetWindowProvider` keeps Runtime owner state synchronized with native transient-window ownership without introducing QWidget types into Runtime.

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

For Qt 6.8.3, the Graphs build needs:

```text
qtgraphs
qtquick3d
qtshadertools
```

### `LfQuick::Diagnostics` / `LfQuick.Diagnostics`

Optional leaf module for development/runtime observability:

```text
DiagnosticsModel
DiagnosticsLogSink
DiagnosticsHud / inspectors
AccessibilityAudit
bounded event history
generic metrics / timing
snapshot / JSON
atomic JSON export
```

`DiagnosticsLogSink` accepts Core `TelemetryEvent` records and queues model updates onto the DiagnosticsModel thread when needed. Foundation modules do not depend on Diagnostics; applications compose it from the outside.

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

- logical resource URL → local development-file mapping;
- file + directory watching for editor atomic-save workflows;
- debounce and QML component-cache invalidation;
- reference-counted shared source watches;
- candidate-first atomic UI replacement;
- selected-property state preservation;
- failed reload rollback that keeps the previous UI alive.

### Optional styles and modern-Qt adapters

Optional leaves are isolated from foundation modules and are OFF by default.

```text
LFQUICK_BUILD_FLUENT_STYLE
    -> LfQuick::FluentStyle / LfQuick.Fluent

LFQUICK_BUILD_BRAND_TEST_STYLE
    -> LfQuick::BrandTestStyle / LfQuick.BrandTest

LFQUICK_BUILD_SAFEAREA_ADAPTER
    Qt 6.9+
    -> LfQuick::SafeAreaAdapter

LFQUICK_BUILD_QRANGE_MODEL_ADAPTER
    Qt 6.10+
    -> LfQuick::RangeModelAdapter / RangeModelBridge
    -> Qt 6.12 adds drag/drop action support

LFQUICK_EXPERIMENTAL_CANVAS_PAINTER
    Qt 6.12+ for the declarative QtCanvas2D adapter
    -> LfQuick::CanvasPainterAdapter / LfQuick.CanvasPainter
    -> Technology Preview; licensing review required before product use

LFQUICK_EXPERIMENTAL_STYLEKIT
    Qt 6.12+
    -> LfQuick::StyleKit
    -> QStyleKitStyle bridge for shared Quick/Widget styling experiments
```

See [`docs/QT_OPTIONAL_ADAPTERS.md`](docs/QT_OPTIONAL_ADAPTERS.md).

## Architecture boundaries

```text
LfQuickCore         -> QtCore only
LfQuickRuntime      -> Core; no QWidget/QWindow/QQuickItem
LfQuickUi           -> Core + Runtime + Qt Quick
LfQuickDataView     -> Ui + Qt Model/View/Quick
LfQuickPlatform     -> Core + QtCore/Gui platform/input services
LfQuickPlatformQml  -> optional Platform + QtQml leaf
LfQuickIntegration  -> optional Ui + Platform cross-module leaf
LfQuickWidgetBridge -> optional Widgets / QuickWidgets
LfQuickCharts       -> optional visualization
Diagnostics         -> optional leaf
Tooling             -> optional development leaf
Styles              -> optional leaves
QtAdapters          -> optional version-specific leaves
Product adapters    -> legacy/product dependencies
```

`tools/check_architecture.py` prevents foundation modules from acquiring reverse dependencies on Diagnostics, Tooling, Integration, PlatformQml, styles, WidgetBridge or version-specific Qt adapters where those directions are forbidden.

## Build

Default Qt 6.8.3-oriented build:

```bash
cmake -S . -B build
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

Enable QWidget migration demo:

```bash
cmake -S . -B build \
  -DLFQUICK_BUILD_WIDGET_BRIDGE=ON \
  -DLFQUICK_BUILD_HYBRID_DEMO=ON
```

Enable declarative Platform services and attached scanner integration:

```bash
cmake -S . -B build \
  -DLFQUICK_BUILD_PLATFORM_QML=ON \
  -DLFQUICK_BUILD_INTEGRATION=ON
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

Enable DataView benchmarks:

```bash
cmake -S . -B build \
  -DLFQUICK_BUILD_BENCHMARKS=ON
```

Enable QRangeModel adapter with Qt 6.10+:

```bash
cmake -S . -B build \
  -DLFQUICK_BUILD_QRANGE_MODEL_ADAPTER=ON
```

Enable local screenshot tool:

```bash
cmake -S . -B build \
  -DLFQUICK_BUILD_SCREENSHOT_TOOL=ON
cmake --build build --target lfquick-screenshot
```

## Important CMake options

```text
LFQUICK_BUILD_PLATFORM
LFQUICK_BUILD_PLATFORM_QML
LFQUICK_BUILD_UI
LFQUICK_BUILD_INTEGRATION
LFQUICK_BUILD_DATAVIEW
LFQUICK_BUILD_WIDGET_BRIDGE
LFQUICK_BUILD_HYBRID_DEMO
LFQUICK_BUILD_CHARTS
LFQUICK_BUILD_QT_GRAPHS_ADAPTER
LFQUICK_BUILD_CUSTOM_QUICK_CHARTS
LFQUICK_BUILD_LEGACY_QCUSTOMPLOT_ADAPTER
LFQUICK_BUILD_SAFEAREA_ADAPTER
LFQUICK_BUILD_QRANGE_MODEL_ADAPTER
LFQUICK_EXPERIMENTAL_CANVAS_PAINTER
LFQUICK_BUILD_FLUENT_STYLE
LFQUICK_BUILD_BRAND_TEST_STYLE
LFQUICK_BUILD_DIAGNOSTICS
LFQUICK_BUILD_MEDICAL_DEMO
LFQUICK_BUILD_BENCHMARKS
LFQUICK_BUILD_SCREENSHOT_TOOL
LFQUICK_ENABLE_HOT_RELOAD
LFQUICK_ENABLE_QMLTC
LFQUICK_EXPERIMENTAL_STYLEKIT
```

## Medical Demo

`examples/medical-demo` is the main vertical reference application for analyzer-style desktop/device UI workflows. It uses **synthetic data only** and is **not medical diagnostic software**.

Current pages:

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

The Developer page is also the live Design System playground: Theme mode/density/high-contrast, base controls, date/time controls, focus, busy overlay and scanner behavior can be exercised in one place.

## Hybrid Demo

`examples/hybrid-demo` validates gradual QWidget migration:

```text
QML Page <-> QuickRuntime/PageRouter <-> Legacy QWidget Page
```

It also exercises WidgetFocusBridge, shared overlays, WindowRouter/WidgetWindowProvider, QuickSurface telemetry, QML→legacy fallback and Diagnostics export.

## Hardening and tests

Current source-level test coverage registered in CMake includes:

- Runtime 1000-cycle PageRouter push/back stress;
- WindowRouter owner/reparent/detach/state-save and topology-close tests;
- Keyboard-wedge 1000-scan loss test and InputRouter lifetime tests;
- Integration scanner-policy/lifetime tests;
- Focus Walker/Manager/Navigator scope-lifetime tests;
- DataView 20k/50k × 8/20/40-column benchmark matrix;
- process RSS before/after DataTableModel population;
- DataGrid FPS/slow-frame telemetry;
- base-control QML construction smoke test;
- repeated WidgetFocusBridge focus transfer stress;
- scanner isolation + modal focus-trap stress;
- QuickSurface repeated resize/root-size consistency;
- Hot Reload success/state-preserve/failure-rollback/shared-watch tests;
- Diagnostics bounded model/export tests;
- optional QRangeModel bridge tests;
- Python tooling self-tests.

These tests are source-registered but the current HEAD has **not** yet completed the final full Qt 6.8.3 build/CTest matrix.

Multi-DPI, compositor and native z-order behavior require a real desktop environment. See [`tests/manual/WIDGETBRIDGE_HARDENING.md`](tests/manual/WIDGETBRIDGE_HARDENING.md).

## Developer tools

`tools/` now provides:

```text
check_architecture.py
lfquick_lint.py
lfquick_gen_vm.py
lfquick_gen_token.py
lfquick_deploy_check.py
lfquick-screenshot (optional C++ CLI)
```

See [`tools/README.md`](tools/README.md).

## Qt version gates

```text
SafeArea                  >= Qt 6.9
QRangeModel               >= Qt 6.10
QCanvasPainter C++        >= Qt 6.11
QRangeModel drag/drop     >= Qt 6.12
QtCanvas2D QML            >= Qt 6.12
qt_add_qml_preview        >= Qt 6.12
Qt Labs StyleKit          >= Qt 6.12
```

The primary public API baseline remains Qt 6.8.3.

## External reference policy

LfQuick may learn from public Qt/Fluent implementations, but external references must follow the project's clean-room and license rules:

- public MIT `zhuzichu520/FluentUI` source may be studied; copied/substantially-derived code requires the relevant MIT notice/source record;
- `FluentUI2/FluentUI Pro` is treated as a clean-room public API/behavior reference unless separate compatible source licensing exists;
- decompilation or reconstruction of proprietary/binary implementations is not permitted;
- LfQuick architecture takes priority over source-level similarity to reference projects.

See [`docs/REFERENCE_POLICY.md`](docs/REFERENCE_POLICY.md).

## Documents

- [Implementation Status](docs/IMPLEMENTATION_STATUS.md)
- [Next Chat Handoff](docs/NEXT_CHAT_HANDOFF.md)
- [Architecture Review & V2.1 Optimization](docs/LfQuick_V2.1_架构审查与优化方案.md)
- [Development Plan](docs/LfQuick_详细开发计划.md)
- [Medical Demo Design](docs/LfQuick_Medical_Demo_详细设计方案.md)
- [Optional modern Qt adapters](docs/QT_OPTIONAL_ADAPTERS.md)
- [WidgetBridge hardening checklist](tests/manual/WIDGETBRIDGE_HARDENING.md)
- [Developer tools](tools/README.md)
- [Hybrid Demo](examples/hybrid-demo/README.md)
- [External Reference & Clean-room Policy](docs/REFERENCE_POLICY.md)
- [Third-party Notices](THIRD_PARTY_NOTICES.md)
