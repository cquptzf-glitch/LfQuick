# Third-party notices

LfQuick uses public third-party projects as design and implementation references. Where source is copied or substantially adapted, the upstream license and notices must be preserved for the derived portion.

## Quickshell

- Project: `quickshell-mirror/quickshell`
- Reference revision: `2b4501a282689792b62e94f3fb30d8f512324fe6` (`master` as inspected on 2026-10-17)
- License: GNU Lesser General Public License, version 3 (LGPL-3.0)
- Upstream areas referenced/adapted by the runtime and Models refactor:
  - `src/core/reload.hpp` / `reload.cpp` — scoped stable identity and reload propagation model
  - `src/core/persistentprops.hpp` / `persistentprops.cpp` — explicit property state restoration
  - `src/core/lazyloader.hpp` / `lazyloader.cpp` and `src/core/incubator.hpp` / `incubator.cpp` — asynchronous QML incubation model
  - `src/core/retainable.hpp` — retain/release delayed-destruction semantics
  - `src/core/model.hpp` — lightweight QObject-backed list-model semantics
  - `src/core/scriptmodel.hpp` / `scriptmodel.cpp` — incremental JavaScript-list diff model
  - `src/core/variants.hpp` / `variants.cpp` — repeating non-visual QObject delegates from a model

LfQuick intentionally does not import Quickshell's shell-specific `EngineGeneration`, Wayland/X11, service, or platform runtime. The reusable semantics are integrated into LfQuick's own Page/Window/Tooling lifecycle and APIs. LfQuick also deliberately extends several adapted concepts, including public-API-only incubation scheduling, serialized page-state snapshots, `QAbstractItemModel` support for object repetition, stable-key reuse, and safe duplicate-key fallback behavior.

When distributing source-derived portions, keep this attribution and satisfy the applicable LGPL-3.0 source/license requirements. If additional Quickshell files are copied or substantially adapted, add them to this list together with the upstream revision used.

## Other design / implementation references

- `zhuzichu520/FluentUI` — MIT licensed public Qt/QML FluentUI implementation. Used as an architectural and interaction reference.
- `zhuzichu520/FluentUI2` — public API/demo/binary repository. Used only as a clean-room API and behavior reference; no private implementation is included.
- Qt — framework APIs and documentation used throughout LfQuick.
- Microsoft Fluent Design — visual/design guidance reference.
