# LfQuick 开发交接（当前权威版本）

> 更新时间：2026-10-17  
> 仓库：`https://github.com/cquptzf-glitch/LfQuick`  
> 分支策略：**main-only**。  
> 当前策略：**主要功能已基本完成；继续源码收口与最终验证。暂时不要恢复 GitHub Actions / CI / release workflow。**

## 1. 新窗口开场直接做什么

不要重新做需求分析，也不要重复实现已经完成的：

```text
PageRouter / WindowRouter
Navigation / ApplicationShell
Permission / Validation / Overlay
DataGrid
Charts / Qt Graphs
Diagnostics
Hot Reload
Design System / Base Controls
Platform Services
QRangeModel / Canvas2D / StyleKit adapter
local developer tooling
```

直接执行：

1. 读取 main 最新 HEAD；
2. 读取本文件与 `docs/IMPLEMENTATION_STATUS.md`；
3. 保持 main-only；
4. **不要重新创建 `.github/workflows/*`**，除非用户明确要求恢复 CI；
5. 优先做静态/API 收口，修最终编译前能发现的问题；
6. 进入可用 Windows + Qt 6.8.3 环境后做 full Configure / Build / CTest；
7. 再做真实 GUI hardening、deploy/package；
8. 最后才恢复 CI/release。

---

# 2. GitHub Actions 当前状态

用户明确要求：

```text
先移除 CI / Action 功能
先保证完成开发再说
```

因此已删除：

```text
.github/workflows/build.yml
.github/workflows/release.yml
```

`.github/workflows` 当前不存在。

## 历史验证事实

删除 Actions 之前，一个较早 Windows Qt 6.8.3 / MSVC2022 revision 曾完整通过：

```text
Architecture
Configure
full Build
13/13 CTest
Qt deploy
package validation
artifact upload
```

这只是历史基线。当前 HEAD 后续又加入了大量功能，**不能宣称当前 HEAD 已 full-build green**。

---

# 3. 当前功能面

## Core / Runtime

```text
Error / Result / OperationHandle
Command / Event
Permission / Validation / Route
PageRegistry / ProviderRegistry
PageRouter
None / KeepAlive / Singleton / weighted LRU
route-global Singleton
structured provider errors
cross-provider fallback
NavigationGuard
WindowProviderRegistry / WindowRouter
owner / close policy / close guard
```

## UI / Design System

```text
QuickRuntime / QuickPageProvider
QML error/fallback
Overlay Busy/Toast/Dialog
Focus
Lf.permission
ValidationRuleTable / FormState
NavigationItemModel / NavigationView / ApplicationShell
Theme / Tokens
Light / Dark / System
High Contrast / Density / Motion
```

Base controls：

```text
LfButton
LfTextField
LfCheckBox / LfRadioButton / LfSwitch
LfComboBox / LfSlider
LfProgressBar / LfProgressRing
LfSearchBox / LfFilterInput / LfPasswordField
LfStatusIndicator
LfNumberInput / LfRangeInput
LfDatePicker / LfTimePicker
LfDateTimePicker / LfDateRangePicker
```

`DeveloperPage.qml` 已作为真实 Design System Playground。

## DataView

```text
DataTableModel / DataFilterProxyModel
DataGrid
column persistence
edit transaction
selection/context/keyboard
proxy/source mapping
typed delegates
DataGridTelemetry
20k/50k benchmark + RSS
```

## Platform

```text
ScannerSession
KeyboardWedgeScanner
InputRouter
SystemAppearance
ScreenService
ClipboardService
FileWatcher
RuntimeCapabilities
```

## WidgetBridge

```text
QuickSurface
WidgetPageProvider
WidgetWindowProvider
WidgetFocusBridge
WidgetOverlayBackend
DPR/screen/resize/load/first-frame telemetry
```

## Charts

```text
ChartDataModel / Series model
Qt Graphs adapter
Medical Demo Charts page
```

## Diagnostics

```text
DiagnosticsModel
DiagnosticsHud
bounded events
metrics/timing
JSON snapshot/export
Medical + Hybrid integration
```

## Hot Reload

```text
QmlHotReloadController
QmlHotReloadHost
local source mapping
atomic save watcher
cache clear
debounce
candidate-first swap
state preservation
failure rollback
reference-counted shared watches
```

---

# 4. Modern Qt feature gates

```text
SafeArea                  >= Qt 6.9
QRangeModel               >= Qt 6.10
QCanvasPainter C++        >= Qt 6.11
QRangeModel drag/drop     >= Qt 6.12
QtCanvas2D QML            >= Qt 6.12
qt_add_qml_preview        >= Qt 6.12
Qt Labs StyleKit          >= Qt 6.12
```

Optional OFF-by-default modules/options：

```text
LFQUICK_BUILD_QRANGE_MODEL_ADAPTER
LFQUICK_EXPERIMENTAL_CANVAS_PAINTER
LFQUICK_EXPERIMENTAL_STYLEKIT
```

规则：这些模块必须保持 leaf-only；不要让 Core/Runtime/UI/DataView/Platform/WidgetBridge/Charts 依赖它们。

Canvas Painter / QtCanvas2D 是 Technology Preview，且存在独立 Qt 许可要求；禁止默认为产品构建打开。

见 `docs/QT_OPTIONAL_ADAPTERS.md`。

---

# 5. Local Tooling

已完成：

```text
tools/check_architecture.py
tools/lfquick_lint.py
tools/lfquick_gen_vm.py
tools/lfquick_gen_token.py
tools/lfquick_deploy_check.py
tools/screenshot -> lfquick-screenshot
```

Python self-test：

```text
tools/test_tooling.py
```

检测到 Python 时会注册 `tooling_python_selftest` 到 CTest。

见 `tools/README.md`。

---

# 6. 当前 hardening/test 已增加

```text
1000-cycle PageRouter push/back stress
1000-scan KeyboardWedge stress
base-control QML smoke test
DataGrid FPS/frame/slow-frame telemetry
DataView 20k/50k benchmark + RSS
WidgetFocusBridge repeated round-trip stress
scanner isolation + modal focus trap
QuickSurface 120 resize / DPR / root-size stress
Hot Reload success/state/rollback/shared-watch tests
Diagnostics bounded/export tests
optional QRangeModel bridge tests
Python tooling self-tests
```

Runtime stress 的计数已与 `PageRouter.cpp` 逐行核对：None cache policy 每 cycle 创建/销毁各 2 个实例，测试断言正确。

---

# 7. 真实 GUI hardening

仍必须在真实 Windows desktop 执行：

```text
100% <-> high-DPI 多屏移动
QQuickWidget popup/modal/native z-order
focus restore
native WindowRouter close
scanner isolation
30-60 minute soak
```

见：

```text
tests/manual/WIDGETBRIDGE_HARDENING.md
```

---

# 8. 下一步严格顺序

不要再扩张新的大框架模块。

```text
1. 静态/API/生命周期审查
2. 修明显 QML/C++/CMake 编译风险
3. Windows Qt 6.8.3 full Configure + Build
4. 全量 CTest
5. Qt 6.10 QRangeModel optional build
6. Qt 6.12 StyleKit / Canvas2D optional build
7. 手工 GUI hardening
8. deploy/package + lfquick_deploy_check
9. 最后恢复 GitHub Actions/release
```

最终验证完成前必须使用以下表述：

```text
Major functionality implemented: YES
Current HEAD full Build verified: NO
Current HEAD full CTest verified: NO
Current HEAD deploy/package verified: NO
CI workflows present: NO (intentional)
CI Green: NO CLAIM
```

---

# 9. 架构/参考边界

继续遵守：

```text
Core -> QtCore only
Runtime -> no QWidget/QQuickItem
UI -> no product domain
DataView -> no medical business
Charts -> no QC/Calibration algorithms
Diagnostics/Tooling/QtAdapters -> leaf-only
```

FluentUI MIT 可按许可证研究；FluentUI2/Pro 继续 clean-room public API/behavior reference，不逆向 DLL。见 `docs/REFERENCE_POLICY.md`。
