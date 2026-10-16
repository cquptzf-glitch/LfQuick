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

Development has entered the first runtime implementation stage:

- repository and CMake baseline
- `LfQuickCore` (QtCore-only) with Result/Error, Command, Event, Route, Operation and validation/permission contracts
- `LfQuickRuntime` with PageRegistry, PageProviderRegistry, PageLifecycle and PageRouter foundations
- `LfQuickUi` with QuickPageProvider, Theme/Tokens, LfPage lifecycle hooks and Overlay primitives
- optional `LfQuickWidgetBridge` with shared-engine capable QuickSurface
- unit test coverage for Core event lifetime and Runtime navigation/lifecycle
- Medical Demo wired to LfQuick page/theme/overlay primitives

## Architecture rules

```text
LfQuickCore      -> QtCore only
LfQuickRuntime   -> Core, no QWidget/QQuickItem
LfQuickUi        -> Core + Runtime + Qt Quick
WidgetBridge     -> optional Qt Widgets / QuickWidgets
Product adapters -> legacy/product dependencies
```

## Medical Demo

`examples/medical-demo` is the official reference application for biochemistry/immunoassay analyzer UI scenarios. It uses synthetic data only and is **not medical diagnostic software**.

Initial page groups:

- Dashboard
- Samples
- Results
- Instrument
- Alarms
- Developer

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
- [External Reference & Clean-room Policy](docs/REFERENCE_POLICY.md)
- [Third-party Notices](THIRD_PARTY_NOTICES.md)

## Build

```bash
cmake -S . -B build -DLFQUICK_BUILD_WIDGET_BRIDGE=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

Requires Qt 6.8.3 or newer.
