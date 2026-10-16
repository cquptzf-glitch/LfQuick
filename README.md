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

Development has started with the S0/S1 baseline:

- repository and CMake skeleton
- `LfQuickCore` (QtCore-only)
- `LfQuickRuntime` provider contracts and registry skeleton
- initial `LfQuickUi` QML module
- optional `LfQuickWidgetBridge` skeleton
- unit test skeleton
- Medical Demo application shell

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

## Documents

- [Architecture Review & V2.1 Optimization](docs/LfQuick_V2.1_架构审查与优化方案.md)
- [Development Plan](docs/LfQuick_详细开发计划.md)
- [Medical Demo Design](docs/LfQuick_Medical_Demo_详细设计方案.md)

## Build

```bash
cmake -S . -B build -DLFQUICK_BUILD_WIDGET_BRIDGE=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

Requires Qt 6.8.3 or newer.
