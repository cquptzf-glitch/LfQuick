# LfQuick

LfQuick is a modular Qt UI Runtime Framework targeting **Qt 6.8.3** with a planned migration path to **Qt 6.12 LTS**.

The project focuses on large desktop/device applications that need:

- Qt Quick/QML application runtime
- gradual QWidget → QML migration
- style-agnostic design system
- routing and page lifecycle
- overlay, input, focus and scanner infrastructure
- DataGrid and chart subsystems
- production diagnostics and fallback

## Current status

Development is proceeding directly on `main` using a main-only workflow.

Implemented foundations now include:

- `LfQuickCore`: Result/Error, Command, Event, Route, Operation and validation/permission contracts
- `LfQuickRuntime`: PageRegistry, PageProviderRegistry, PageLifecycle, PageRouter, NavigationGuard and cache policies
- `LfQuickUi`: QuickRuntime, QuickPageProvider, Theme/Tokens, LfPage, Overlay, Focus, QML error collection and ErrorPage fallback
- `LfQuickPlatform`: scanner session, keyboard-wedge scanner and input router
- `LfQuickDataView`: DataTableModel, DataFilterProxyModel and DataGrid MVP
- optional `LfQuickWidgetBridge`: QuickSurface telemetry and WidgetPageProvider
- Medical Demo with synthetic 10k/5k row DataGrid scenarios
- Hybrid Demo with QML and legacy QWidget pages sharing one PageRouter/history

## Architecture rules

```text
LfQuickCore      -> QtCore only
LfQuickRuntime   -> Core, no QWidget/QQuickItem
LfQuickUi        -> Core + Runtime + Qt Quick
LfQuickDataView  -> Ui + Qt Model/View/Quick
LfQuickPlatform  -> platform/input adapters
WidgetBridge     -> optional Qt Widgets / QuickWidgets
Product adapters -> legacy/product dependencies
```

## Medical Demo

`examples/medical-demo` is the official reference application for biochemistry/immunoassay analyzer UI scenarios. It uses synthetic data only and is **not medical diagnostic software**.

Current page groups:

- Dashboard
- Samples
- Results
- Instrument
- Alarms
- Developer

## Hybrid Demo

`examples/hybrid-demo` validates the gradual migration path where a QWidget application can route between:

```text
QML Page <-> QuickRuntime/PageRouter <-> Legacy QWidget Page
```

Build it with:

```bash
cmake -S . -B build \
  -DLFQUICK_BUILD_WIDGET_BRIDGE=ON \
  -DLFQUICK_BUILD_HYBRID_DEMO=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

## External reference policy

LfQuick may learn from public Qt/Fluent implementations, but all external references must follow the project's clean-room and license rules. In particular:

- public MIT `zhuzichu520/FluentUI` source may be studied; copied/substantially-derived code requires the relevant MIT notice and source record;
- `FluentUI2/FluentUI Pro` is treated as a clean-room public API/behavior reference unless a separate compatible source license is explicitly obtained;
- decompilation or reconstruction of proprietary/binary implementations is not permitted;
- LfQuick architecture takes priority over source-level similarity to any reference project.

See [External Reference & Clean-room Policy](docs/REFERENCE_POLICY.md).

## Documents

- [Architecture Review & V2.1 Optimization](docs/LfQuick_V2.1_架构审查与优化方案.md)
- [Development Plan](docs/LfQuick_详细开发计划.md)
- [Medical Demo Design](docs/LfQuick_Medical_Demo_详细设计方案.md)
- [Implementation Status](docs/IMPLEMENTATION_STATUS.md)
- [Next Chat Handoff](docs/NEXT_CHAT_HANDOFF.md)
- [Hybrid Demo](examples/hybrid-demo/README.md)
- [External Reference & Clean-room Policy](docs/REFERENCE_POLICY.md)
- [Third-party Notices](THIRD_PARTY_NOTICES.md)

## Build

```bash
cmake -S . -B build -DLFQUICK_BUILD_WIDGET_BRIDGE=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

Requires Qt 6.8.3 or newer.

> CI note: recent GitHub Actions jobs have been ending before any workflow steps are assigned/executed. Until a real Configure/Build/Test run is observed, the project does **not** claim CI-green status for the newest commits.
