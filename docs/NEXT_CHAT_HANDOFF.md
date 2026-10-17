# LfQuick 开发交接（当前权威版本）

> 更新时间：2026-10-17  
> 仓库：`https://github.com/cquptzf-glitch/LfQuick`  
> 分支策略：**main-only**。  
> 当前策略：**主要功能已基本完成；继续源码/API/生命周期收口与最终验证。暂时不要恢复 GitHub Actions / CI / release workflow。**

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
PlatformQml / Integration scanner bridge
QRangeModel / Canvas2D / StyleKit adapter
local developer tooling
```

直接执行：

1. 读取 main 最新 HEAD；
2. 读取本文件与 `docs/IMPLEMENTATION_STATUS.md`；
3. 保持 main-only；
4. **不要重新创建 `.github/workflows/*`**，除非用户明确要求恢复 CI；
5. 优先做静态/API/生命周期收口，修最终编译前能发现的问题；
6. 不再为了“组件名对齐”新增空壳模块；
7. 进入可用 Windows + Qt 6.8.3 环境后做 full Configure / Build / CTest；
8. 再做真实 GUI hardening、deploy/package；
9. 最后才恢复 CI/release。

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

这只是历史基线。当前 HEAD 后续又加入了大量功能和 hardening，**不能宣称当前 HEAD 已 full-build green**。

---

# 3. 当前功能面

## Core / Runtime

```text
Error / Result / OperationHandle
Command / Event
Permission / Validation / Route
TelemetryEvent / ILogSink
PageRegistry / ProviderRegistry
PageRouter
None / KeepAlive / Singleton / weighted LRU
route-global Singleton
structured provider errors
cross-provider fallback
NavigationGuard
WindowProviderRegistry / WindowRouter
owner / reparent / close policy / close guard
provider-native saveState / restoreState
```

PageRouter 最新生命周期 contract：

```text
IPageInstance::isValid()
    provider 可报告 backing object 已失效

invalid KeepAlive/LRU cache
    -> cleanup old instance
    -> 按原 descriptor/parameters 自动重建

invalid route-global Singleton
    -> 一次性移出 singleton cache
    -> 清除 history/forward 中同一 shared instance 的引用
    -> destroy 一次
    -> 后续按需重建/复用 replacement

isValid()==false 后
    -> Router 不再调用 deactivate/suspend/resume
    -> 只走 cleanup/recreate
```

WindowRouter 当前关键语义：

```text
single/multi instance
owner cycle rejection
explicit reparent / detach
CloseWithOwner children first
Detach survivor native owner removal
closeAll owner-depth order
windowClosed emit 前已从 registry 移除
native close request 回到 Router
```

Runtime 仍禁止 `QWidget / QWindow / QQuickItem`。

## UI / Design System

```text
QuickRuntime / QuickPageProvider
QML error/fallback
OverlayCoordinator / BusyToken / Toast / Dialog
FocusWalker / FocusManager
FocusNavigator compatibility API
NavigationGuardController
NavigationItemModel / NavigationView / ApplicationShell
Theme / Tokens / Units / Typography / Motion / DpiHelper
ThemeSystemAdapter / IconLoader
```

QuickRuntime / Quick page host contract：

```text
QuickRuntime.host 是 routed Quick pages 的 scene root
host 切换 -> PageRouter reset
host 销毁 -> provider host 清空 + PageRouter reset
QuickPageProvider 缺 host -> structured quick.host-missing
禁止“路由成功但 page 不在任何 scene tree”
```

`Lf` attached context 五条 contract 都已有真实消费路径：

```text
Lf.permission      -> PermissionBinding
Lf.validationRule  -> LfValidationBinding
Lf.focusPolicy     -> FocusManager / FocusWalker
Lf.focusOrder      -> FocusWalker
Lf.acceptScanner   -> LfQuick.Integration::LfScannerTarget
Lf.helpContext     -> HelpContextRegistry / LfHelpShortcut
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
DataGridColumn / DataGridHeader / DataGridViewport
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
KeyboardWedgeScanner / MockScanner
InputTarget / InputRouter / HotkeyRouter
SystemAppearance
ScreenService
ClipboardService
FileWatcher
RuntimeCapabilities
```

InputRouter 已做生命周期收口：

```text
scanner destruction -> scannerChanged/sourceCount sync
clear/unregister source -> compatibility scanner sync
install target destruction -> installedChanged
InputTarget target destruction -> targetChanged
InputTarget repeated register/unregister -> destroyed connection ownership closed
QML QObject source registration entry
```

`RuntimeCapabilities` 已覆盖当前 optional module / adapter / style / tooling stable keys。

## PlatformQml

可选：

```text
-DLFQUICK_BUILD_PLATFORM_QML=ON
LfQuick::PlatformQml / LfQuick.Platform
```

保持 Platform 本体 QtCore/Gui-only，通过 foreign registration 暴露到 QML，并提供：

```text
ScannerTarget
InputSourceRegistration
```

## Integration

可选：

```text
-DLFQUICK_BUILD_INTEGRATION=ON
LfQuick::Integration / LfQuick.Integration
```

当前：

```text
LfScannerTarget
    Lf.acceptScanner + activeFocus
    -> Platform InputTarget / InputRouter
```

依赖方向只能：

```text
Integration -> Ui + Platform
```

基础模块禁止反向依赖 Integration。

## WidgetBridge

```text
QuickSurface
WidgetPageProvider
WidgetWindowProvider
WidgetFocusBridge
WidgetOverlayBackend
DPR/screen/resize/load/first-frame telemetry
```

WidgetPageProvider 当前 host 规则：

```text
page host 必须存在，否则 structured widget.host-missing
setHost() 递增 provider generation
旧 host 上的 cached page -> isValid()==false
PageRouter 下次命中时自动 cleanup/rebuild 到新 host
WidgetWindowProvider 专门负责 top-level window，不用 WidgetPageProvider 代替
```

WidgetWindowProvider 当前 owner/lifetime 规则：

```text
Runtime owner 必须能解析为同 provider QWidget owner，否则结构化失败
factory QWidget QObject parent 一律剥离
native owner 只用 QWindow::transientParent
Detach 不受 QObject parent 析构影响
外部直接 delete QWidget -> 回调 Router 清理 window record
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
DiagnosticsLogSink
DiagnosticsHud / inspectors
AccessibilityAudit
DiagnosticsTokens
bounded events
metrics/timing
JSON snapshot/export
Medical + Hybrid integration
```

`DiagnosticsLogSink` 可接 Core `TelemetryEvent`；跨线程 write 会先序列化回 sink owner thread，再更新 model。

`AccessibilityAudit` 不再在 issue snapshot 中保存裸 QQuickItem 指针；root 切换/销毁会清理旧 issues。

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

# 4. Modern Qt / style feature gates

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
LFQUICK_BUILD_PLATFORM_QML
LFQUICK_BUILD_INTEGRATION
LFQUICK_BUILD_FLUENT_STYLE
LFQUICK_BUILD_BRAND_TEST_STYLE
LFQUICK_BUILD_SAFEAREA_ADAPTER
LFQUICK_BUILD_QRANGE_MODEL_ADAPTER
LFQUICK_BUILD_CUSTOM_QUICK_CHARTS
LFQUICK_BUILD_LEGACY_QCUSTOMPLOT_ADAPTER
LFQUICK_EXPERIMENTAL_CANVAS_PAINTER
LFQUICK_EXPERIMENTAL_STYLEKIT
```

规则：leaf 模块必须保持 leaf-only；不要让 Core/Runtime/UI/DataView/Platform/WidgetBridge/Charts 反向依赖 Diagnostics/Tooling/Integration/PlatformQml/styles/version adapters。

Canvas Painter / QtCanvas2D 是 Technology Preview，且存在独立 Qt 许可要求；禁止默认为产品构建打开。

见 `docs/QT_OPTIONAL_ADAPTERS.md`。

---

# 5. Local Tooling / architecture guard

已完成：

```text
tools/check_architecture.py
tools/lfquick_lint.py
tools/lfquick_gen_vm.py
tools/lfquick_gen_token.py
tools/lfquick_deploy_check.py
tools/screenshot -> lfquick-screenshot
```

Architecture checker 已同步识别：

```text
Diagnostics / Tooling / Integration / PlatformQml
FluentStyle / BrandTestStyle
SafeArea / RangeModel / CanvasPainter / CustomQuickCharts / QCustomPlot / StyleKit
QML leaf imports
LfQuick.impl forbidden public import
```

QML color lint 允许 `*Tokens.qml` 定义色值，其它 QML 继续检查 magic color。

Python self-test：

```text
tools/test_tooling.py
```

检测到 Python 时会注册 `tooling_python_selftest` 到 CTest。

---

# 6. 当前 hardening/test 已增加

```text
1000-cycle PageRouter push/back stress
invalid KeepAlive/Singleton cache rebuild tests
invalid current page no deactivate/suspend lifecycle test
singleton invalidation no-double-destroy/history-reuse test
QuickRuntime host switch/destruction reset tests
Quick page missing-host structured failure test
WindowRouter reparent/detach/cycle/state-save/topology-close tests
1000-scan KeyboardWedge stress
InputRouter source/scanner/install-target lifetime tests
InputTarget target destruction notify test
Integration attached scanner policy/lifetime/QML smoke tests
PlatformQml QML import/construct smoke test
FocusWalker/FocusManager/FocusNavigator scope lifetime tests
BusyToken release/external-end/destructor/coordinator-destruction tests
attached validation/help contract tests
base-control QML smoke test
DataView Column/Header/Viewport/typed-delegate QML smoke test
DataGrid FPS/frame/slow-frame telemetry
DataView 20k/50k benchmark + RSS
WidgetPage host generation/rebuild/missing-host diagnostics tests
WidgetWindow native owner/incompatible owner/QObject parent/external-delete tests
WidgetFocusBridge repeated round-trip stress
scanner isolation + modal focus trap
QuickSurface 120 resize / DPR / root-size stress
Telemetry -> Diagnostics same-thread/worker-thread tests
AccessibilityAudit root lifetime tests
Hot Reload success/state/rollback/shared-watch tests
Diagnostics bounded/export tests
optional QRangeModel bridge tests
Python tooling self-tests
```

这些测试已经注册到源码/CMake，但**当前 HEAD 还没有执行最终 full Build/CTest**。

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
1. 继续静态/API/生命周期审查
2. 修明显 QML/C++/CMake 编译风险与 public API 不一致
3. 复核新增 leaf 的 install/export/downstream discoverability
4. Windows Qt 6.8.3 full Configure + Build
5. 全量 CTest
6. Qt 6.10 QRangeModel optional build
7. Qt 6.12 StyleKit / Canvas2D optional build
8. 手工 GUI hardening
9. deploy/package + lfquick_deploy_check
10. 最后恢复 GitHub Actions/release
```

最终验证完成前必须使用以下表述：

```text
Major functionality implemented: YES
Source-level hardening substantially implemented: YES
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
Runtime -> no QWidget/QWindow/QQuickItem
UI -> no product domain
Platform -> QtCore/Gui only
Integration -> leaf; may depend on Ui + Platform
DataView -> no medical business
Charts -> no QC/Calibration algorithms
Diagnostics/Tooling/Styles/QtAdapters -> leaf-only
```

FluentUI MIT 可按许可证研究；FluentUI2/Pro 继续 clean-room public API/behavior reference，不逆向 DLL。见 `docs/REFERENCE_POLICY.md`。
