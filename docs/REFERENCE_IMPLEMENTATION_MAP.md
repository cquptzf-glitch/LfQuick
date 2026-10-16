# LfQuick Reference Implementation Map

This document complements `REFERENCE_POLICY.md` with module-level guidance.

## Reference levels

- **A — Design/API concept**: official Qt/Microsoft documentation and general behavior can be used freely as design input.
- **B — MIT source reference**: public `zhuzichu520/FluentUI` source may be inspected; copied/substantially-derived code requires attribution and license tracking.
- **C — Clean-room public surface**: `FluentUI2/FluentUI Pro` public QML/API/qmltypes/demo behavior may be studied, but implementation must be independently written.
- **D — Forbidden/private**: private/paid implementation, decompiled binaries, or non-public source must not be used unless an explicit compatible source license has been obtained for LfQuick.

## Module matrix

| LfQuick module | Public FluentUI (MIT) | FluentUI2/Pro public surface | LfQuick rule |
|---|---|---|---|
| Base Controls | B: interaction states, focus, accessibility, sizing behavior | C: public standard-control API and visual behavior | Reimplement on `QtQuick.Templates`/Qt Quick Controls style; all colors/sizes come from Tokens |
| Theme / Tokens | B: study old dark/light/system behavior and known edge cases | C: study Theme/ColorResource public properties | Independent semantic-token model; do not port monolithic `FluTheme` |
| Navigation | B: study UX and legacy navigation behavior | C: study public NavigationView/PageRouter surface | Split View, Controller, Model and Router; no monolithic navigation component |
| Page Router | B: study route/launch behavior | C: public PageRouter behavior | Independent `PageRegistry + PageProviderRegistry + PageRouter` implementation |
| Window Router | B: study window launch behavior | C: public WindowRouter behavior | Separate from PageRouter and application exit/restart |
| Frameless Window | B: inspect public MIT frameless/window behavior | C: public `Frameless` type/properties | Implement `WindowBackend` abstraction with platform-specific backends |
| Overlay / Dialog / Toast | B: interaction patterns and edge cases | C: public behavior/demo | Implement per-`UiSurface` overlay stacks; do not inherit global arbitrary z values |
| DataGrid | B: legacy table UX can be studied | C: public DataGrid/DataGridModel API surface | `QAbstractItemModel`-first independent subsystem; no private implementation inference |
| TreeDataGrid | B/C behavior only | C public API if exposed | Built on LfQuick DataView contracts, not copied internals |
| Focus / Keyboard | B: inspect focus policies in controls | C: observe public behavior | Independent `FocusGroup/FocusWalker/InputRouter`; old behavior is compatibility test input |
| Icons | B: study API conventions | C: public icon names/usage where exposed | Semantic `LfQuick.Icons`; verify asset/font licenses independently |
| Animation / State Layer | B: state timing/interaction reference | C: public visual behavior | Token-driven motion/state layer; avoid copying hard-coded control logic |
| FileWatcher / Capture / Native primitives | B where public source exists | C: qmltypes can reveal public type/property contracts | Implement independently using Qt/OS public APIs |
| Diagnostics | No strong dependency | Public behavior only | LfQuick-owned implementation |

## Concrete examples

### Button

The MIT FluentUI `FluButton.qml` is useful for identifying behavior that must not regress: accessibility role/name/action, Tab focus, enabled/disabled, hover and pressed states.

LfQuick must **not** copy its hard-coded per-control colors. Those are replaced by primitive/semantic/component tokens and a style implementation rooted in Qt Quick Controls templates.

### Router

The MIT `FluRouter.qml` demonstrates useful user-visible behaviors such as named routes, existing-window activation and launch modes. It also combines route registration, component creation, window registry and application exit in one singleton.

LfQuick keeps the behavior ideas but intentionally separates responsibilities:

```text
PageRegistry
PageProviderRegistry
PageRouter
WindowRouter
ApplicationController
```

### FluentUI2/Pro

Public `qmldir`, `qmltypes`, public QML wrappers and demo pages may be used to understand the external contract—for example, the existence of separate page/window routing or C++ model/native primitives.

The implementation behind compiled Pro plugins is not a source reference. LfQuick must independently implement equivalent requirements using Qt public APIs and its own architecture.

## PR rule

When a PR uses a specific reference, record it in the PR template. If actual MIT code is copied/substantially derived, also update `THIRD_PARTY_NOTICES.md` with the exact source path and commit.

When in doubt, choose clean-room reimplementation.
