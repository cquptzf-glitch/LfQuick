# Optional Qt Adapters

LfQuick keeps **Qt 6.8.3** as the main compatibility baseline. APIs introduced by later Qt releases are isolated behind explicit build options and separate targets so they do not leak into the 6.8 public contract.

## Capability matrix

| Capability | Minimum Qt | LfQuick option | Default | Target / module |
| --- | ---: | --- | --- | --- |
| Safe-area support flag | 6.9 | automatic capability only | n/a | core Qt capability |
| QRangeModel bridge | 6.10 | `LFQUICK_BUILD_QRANGE_MODEL_ADAPTER` | OFF | `LfQuick::RangeModelAdapter` |
| QRangeModel drag/drop actions | 6.12 | same option | OFF | enabled automatically when available |
| QCanvasPainter C++ API | 6.11 | capability only | n/a | `Qt6::CanvasPainter` |
| QtCanvas2D QML adapter | 6.12 | `LFQUICK_EXPERIMENTAL_CANVAS_PAINTER` | OFF | `LfQuick::CanvasPainterAdapter` / `LfQuick.CanvasPainter` |
| QML preview CMake integration | 6.12 | capability only | n/a | Qt CMake integration |
| Qt Labs StyleKit bridge | 6.12 | `LFQUICK_EXPERIMENTAL_STYLEKIT` | OFF | `LfQuick::StyleKit` |

`RuntimeCapabilities` exposes both the compile-time adapter state and the Qt-version capability state, so application code should query capabilities rather than hard-code Qt version checks.

## QRangeModel adapter

The adapter is intentionally separate from `LfQuickDataView`.

```text
LfQuickDataView public API
    -> QAbstractItemModel contracts

LfQuick::RangeModelAdapter (Qt 6.10+)
    -> RangeModelBridge
    -> QRangeModel
```

`RangeModelBridge` owns a `QList<QVariantMap>` backing range and exposes the model through `QAbstractItemModel *`. This keeps `QRangeModel` out of the Qt 6.8 public surface while still allowing newer products to adopt it incrementally.

On Qt 6.12 or newer, `setDragDropEnabled(true)` enables the QRangeModel supported drag/drop actions. On Qt 6.10/6.11 the method returns `false` when enabling is requested.

## Canvas Painter / QtCanvas2D

This adapter is **experimental and OFF by default**.

LfQuick uses the QML `QtCanvas2D` surface rather than adding Canvas Painter types to Charts core. `Canvas2DSurface` accepts generic points and style inputs and performs simple 2D polyline rendering.

Important constraints:

- `QCanvasPainter` C++ APIs are available from Qt 6.11;
- the `QtCanvas2D` QML module used by this adapter is introduced in Qt 6.12 and is Technology Preview;
- the Qt Canvas Painter runtime is offered by Qt under commercial licensing and GPLv3, not the common LGPL-only path used by many Qt modules;
- therefore proprietary/product builds must complete their Qt license review before enabling `LFQUICK_EXPERIMENTAL_CANVAS_PAINTER`.

Do not make this option a default dependency of Charts or Medical Demo.

## Qt Labs StyleKit

StyleKit remains a Qt 6.12 Technology Preview and is OFF by default.

`StyleKitBridge` validates a candidate Style QML file first and only swaps the `QApplication` style after it loads successfully. The same StyleKit definition can therefore be experimented with across Qt Quick Controls and QWidget through `QStyleKitStyle`, without making StyleKit a dependency of Core, Runtime or the base UI module.

## Build examples

### Qt 6.10+ QRangeModel

```bash
cmake -S . -B build \
  -DLFQUICK_BUILD_QRANGE_MODEL_ADAPTER=ON
```

### Qt 6.12 StyleKit experiment

```bash
cmake -S . -B build \
  -DLFQUICK_EXPERIMENTAL_STYLEKIT=ON
```

### Qt 6.12 Canvas2D experiment

```bash
cmake -S . -B build \
  -DLFQUICK_EXPERIMENTAL_CANVAS_PAINTER=ON
```

Version gates fail at CMake configure time with an explicit message when the installed Qt version is too old.
