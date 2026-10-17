# Third-party notices

LfQuick uses public third-party projects as design and implementation references. Where source is copied or substantially adapted, the upstream license and notices must be preserved for the derived portion.

## Quickshell

- Project: `quickshell-mirror/quickshell`
- Reference revision: `2b4501a282689792b62e94f3fb30d8f512324fe6` (`master` as inspected on 2026-10-17)
- License: GNU Lesser General Public License, version 3 (LGPL-3.0)
- Upstream areas referenced/adapted by the runtime, Models, Io and Window refactor:
  - `src/core/reload.hpp` / `reload.cpp` — scoped stable identity, reload propagation, and post-reload lifecycle concepts
  - `src/core/persistentprops.hpp` / `persistentprops.cpp` — explicit property state restoration
  - `src/core/lazyloader.hpp` / `lazyloader.cpp` and `src/core/incubator.hpp` / `incubator.cpp` — asynchronous QML incubation model
  - `src/core/boundcomponent.hpp` / `boundcomponent.cpp` — dynamic Component creation with forwarded properties and signal handlers
  - `src/core/transformwatcher.hpp` / `transformwatcher.cpp` — ancestor transform invalidation used by popup/overlay positioning
  - `src/core/retainable.hpp` — retain/release delayed-destruction semantics
  - `src/core/model.hpp` — lightweight QObject-backed list-model semantics
  - `src/core/scriptmodel.hpp` / `scriptmodel.cpp` — incremental JavaScript-list diff model
  - `src/core/variants.hpp` / `variants.cpp` — repeating non-visual QObject delegates from a model
  - `src/io/datastream.hpp` — parser-oriented byte-stream abstraction and delimiter/collector concepts
  - `src/window/proxywindow.hpp` / `proxywindow.cpp` — separation of logical window state from a stable physical/native window backing, including backing transfer across reload
  - `src/widgets/WrapperItem.qml`, `WrapperRectangle.qml`, clipping/wrapper primitives — small composable layout/content primitive design patterns

LfQuick intentionally does not import Quickshell's shell-specific `EngineGeneration`, Wayland/X11, service, or platform runtime. The reusable semantics are integrated into LfQuick's own Page/Window/Tooling lifecycle and APIs. LfQuick also deliberately extends several adapted concepts, including public-API-only incubation scheduling, explicit QML creation-context propagation, serialized page/window state snapshots, deterministic post-restore hooks, transactional native-window backing rollback, `QAbstractItemModel` support for object repetition, stable-key reuse, safe duplicate-key fallback behavior, bounded stream collection, and a framework-specific stream API.

LfQuick's local IPC framing and RPC message schema are independent clean-room designs built on Qt `QLocalServer` / `QLocalSocket`; Quickshell's IPC wire protocol is not copied.

When distributing source-derived portions, keep this attribution and satisfy the applicable LGPL-3.0 source/license requirements. If additional Quickshell files are copied or substantially adapted, add them to this list together with the upstream revision used.

## Other design / implementation references

- `zhuzichu520/FluentUI` — MIT licensed public Qt/QML FluentUI implementation. Used as an architectural and interaction reference.
- `zhuzichu520/FluentUI2` — public API/demo/binary repository. Used only as a clean-room API and behavior reference; no private implementation is included.
- Qt — framework APIs and documentation used throughout LfQuick.
- Microsoft Fluent Design — visual/design guidance reference.
