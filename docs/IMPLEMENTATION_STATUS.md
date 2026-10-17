# LfQuick 当前实现状态

> 更新时间：2026-10-17  
> 开发策略：**main-only**。  
> 当前阶段：主要功能已完成，正在做源码级 hardening / stress / telemetry。  
> 当前明确约束：**GitHub Actions / CI / release workflow 已按用户要求移除，暂时不要恢复。**

## 1. 基线

```text
Qt primary baseline: 6.8.3
C++: C++20
CMake: 3.24+
Branch: main
```

规则：Qt 6.9/6.10/6.11/6.12-only 能力必须通过独立 feature gate / leaf adapter 进入，禁止污染 Qt 6.8.3 公共 API。

当前 HEAD **不能宣称 full-build/CI green**。在删除 Actions 之前，一个较早 revision 已真实完成：

```text
Architecture: PASS
Configure: PASS
MSVC2022 full Build: PASS
CTest: 13/13 PASS
Qt deploy: PASS
package validation: PASS
artifact upload: PASS
```

之后又加入了 Design System、Platform Services、Tooling、QRangeModel/Canvas2D adapter、stress 与更多 hardening，因此最终仍需重新做全量 Qt 构建/测试。

---

# 2. 架构层级

```text
LfQuickCore         -> QtCore only
LfQuickRuntime      -> Core；禁止 QWidget / QQuickItem
LfQuickUi           -> Core + Runtime + Qt Quick
LfQuickDataView     -> Ui + Qt Model/View/Quick
LfQuickPlatform     -> platform/input services
LfQuickWidgetBridge -> optional Widgets / QuickWidgets
LfQuickCharts       -> optional visualization
Diagnostics         -> optional leaf
Tooling             -> optional development leaf
QtAdapters          -> optional version-specific leaves
StyleKit            -> optional Qt 6.12 leaf
Product adapters    -> legacy/product-specific integration
```

`tools/check_architecture.py` 已阻止 Core/Runtime/UI/DataView/Platform/WidgetBridge/Charts 反向依赖：

```text
Diagnostics
Tooling
StyleKit
QtAdapters / RangeModelAdapter / CanvasPainterAdapter
```

---

# 3. Core / Runtime

已完成：

```text
Error / Result
OperationHandle
Command
Event / Dispatcher / Subscription
Permission contract
Validation contract / validateValue
Route contract
```

Runtime 已完成：

```text
PageDescriptor / PageRegistry
PageProviderRegistry
IPageProvider / IPageInstance
PageRouter push / replace / back / reset
None / KeepAlive / Singleton / weighted LRU
route-global Singleton
parameter refresh on Singleton reuse
structured provider diagnostics
cross-provider fallback
provider-native ErrorPage fallback
NavigationGuard
WindowDescriptor / WindowProviderRegistry / WindowRouter
single/multi instance windows
owner relation / close policy / close guard
```

新增 hardening：

- 1000 cycle `PageRouter` push/back stress；
- stress 中验证 depth/history/create/destroy 生命周期闭合；
- WindowRouter 已在 Hybrid Demo 做真实 QWidget top-level window 纵向验证。

---

# 4. UI / Design System

基础框架已完成：

```text
QuickRuntime / QuickPageProvider
QmlErrorCollector / PageLoadError
LfPage
OverlayCoordinator / Busy / Toast / Dialog
FocusNavigator / FocusDebugPanel
NavigationGuardController
Lf.permission attached context
ValidationRuleTable / ValidationFormState
NavigationItemModel / NavigationView / ApplicationShell
```

## Theme / Tokens

Theme 已具备真实语义：

```text
Light / Dark / System
systemDark synchronization
Compact / Standard / Comfortable / Touch density
accent
motion enable/disable
highContrast
```

Tokens 已覆盖：

```text
semantic surfaces / text / border
accent hover/pressed/onAccent
control state fills
selection / focusRing
success / warning / error / critical
spacing / radius / typography
control sizing / motion duration
high-contrast behavior
```

## Style-agnostic base controls

已注册到 `LfQuick` QML module：

```text
LfButton
LfTextField
LfCheckBox
LfRadioButton
LfSwitch
LfComboBox
LfSlider
LfProgressBar
LfProgressRing
LfSearchBox
LfFilterInput
LfPasswordField
LfStatusIndicator
LfNumberInput
LfRangeInput
LfDatePicker
LfTimePicker
LfDateTimePicker
LfDateRangePicker
```

Date 控件使用 Qt 6.8 原生 `MonthGrid / DayOfWeekRow / Calendar`；Time 使用基础 Controls 组合，不引入第三方日历库。

Developer Page 已成为真实 Design System Playground，覆盖：

```text
System/Light/Dark
High Contrast
Density
motion
全部基础控件
日期/时间
Focus
Busy
Scanner
semantic token preview
```

新增 `tst_ui_controls`：一次性实例化全部基础 QML 控件，作为 QML 注册/构造 smoke test。

---

# 5. Overlay / Permission / Validation

Overlay：

```text
IOverlayBackend
OverlayCoordinator
Busy token
Toast queue / severity / position / dedup / duration
Dialog queue / default/destructive/result/cancel
QML presenters
WidgetOverlayBackend
```

Permission：

```qml
Lf.permission
```

支持 allowed/visible/enabled/editable 等 UI experience mapping，Domain/Command 权限仍必须做权威检查。

Validation：

```text
ValidationRuleTable
ValidationFieldState
ValidationFormState
local rule map
IValidationRuleProvider fallback
touched/external business error
form aggregate valid/errorCount
```

Sample Register 已做真实纵向验证。

---

# 6. DataView

已完成：

```text
DataTableModel
DataFilterProxyModel
DataGrid
stable column id
ColumnStateModel / ColumnConfigStore
width/order/visibility persistence
DataEditTransaction
Text/Check/Combo/Progress/Icon/Editor delegates
extended selection
context menu
keyboard/cross-table navigation
proxy/source row mapping
```

性能/hardening：

```text
20k / 50k rows
8 / 20 / 40 columns
1 / 10 / 100 update batches
populate/filter/sort/update metrics
process RSS delta
DataGridTelemetry FPS/frame/slow-frame/first-visible
```

Medical Demo Sample Rack 已把 DataGridTelemetry 以低频 metrics 接入 Diagnostics，不让 DataView 反向依赖 Diagnostics。

---

# 7. Platform / Scanner

原有：

```text
ScannerSession
KeyboardWedgeScanner
InputRouter
```

新增通用平台服务：

```text
SystemAppearance
ScreenService
ClipboardService
FileWatcher
RuntimeCapabilities
```

ScreenService 提供：

```text
screen list
primary screen
screenAt / screenByName
geometry / availableGeometry
DPR / logical+physical DPI
refreshRate / orientation
```

FileWatcher：

- 维护 desired watch；
- 同时监听父目录；
- 能恢复 editor atomic-save 替换后的 file watch。

RuntimeCapabilities 区分：

```text
build option actually compiled
Qt version capability available
```

Scanner 新增 1000 次 keyboard-wedge stress，检查 `S0000` → `S0999` 无丢码/拒绝。

---

# 8. WidgetBridge / Mixed UI

已完成：

```text
QuickSurface
WidgetPageProvider
WidgetWindowProvider
WidgetFocusBridge
FocusTransferPolicy
WidgetOverlayBackend
```

QuickSurface telemetry：

```text
loadDurationMs
firstFrameMs
effectiveDevicePixelRatio
surfaceSize
screenName
resizeCount
```

自动 stress：

```text
30-cycle QWidget <-> QML focus round trip
scanner isolation toggle stress
modal focus trap
120 resize/root-size consistency
DPR validity
metrics reset
```

真实 desktop 仍需执行：

```text
100% <-> high-DPI multi-screen moves
popup/modal/native z-order
native close behavior
30-60 minute soak
```

见 `tests/manual/WIDGETBRIDGE_HARDENING.md`。

---

# 9. Charts / Visualization

Charts core 已完成：

```text
ChartDataModel
SeriesModel / descriptors
backend-neutral data contract
```

Qt Graphs adapter 已完成并在历史真实 MSVC 构建中成功编译/链接：

```text
QtGraphsSeriesAdapter
QtGraphsAxisAdapter
ChartView.qml
Medical Demo Charts page
```

Qt 6.8.3 Graphs 实际依赖：

```text
qtgraphs
qtquick3d
qtshadertools
```

Charts 业务数据仍保持 synthetic；QC/Calibration 算法不进入 Charts core。

---

# 10. Diagnostics / Telemetry

独立 leaf module：

```text
DiagnosticsModel
DiagnosticsHud
```

能力：

```text
bounded event queue
dropped count
metrics
recordTiming
snapshot
toJson
QSaveFile atomic export
```

Medical Demo 记录：

```text
route / depth
cache / singleton / eviction
fallback
QML errors
scanner accepted/rejected counts
DataGrid performance metrics
```

Hybrid Demo 记录：

```text
QuickSurface timing
focus transfer
QML errors/fallback
WindowRouter lifecycle
JSON export
```

Foundation 层不依赖 Diagnostics。

---

# 11. Hot Reload Tooling

可选：

```text
-DLFQUICK_ENABLE_HOT_RELOAD=ON
```

模块：

```text
LfQuick::Tooling / LfQuick.Tooling
QmlHotReloadController
QmlHotReloadHost
```

已实现：

```text
logical qrc URL -> local development file
file + directory watcher
debounce
QQmlEngine component cache clear
atomic candidate-first swap
selected property preservation
failed reload keeps old UI alive
reference-counted shared watch ownership
```

测试覆盖：

```text
mapping/cache invalidation
successful replace
state preservation
syntax failure rollback
shared watch ref-count release
```

---

# 12. Modern Qt adapters

Feature gates：

```text
SafeArea                  >= 6.9
QRangeModel               >= 6.10
QCanvasPainter C++        >= 6.11
QRangeModel drag/drop     >= 6.12
QtCanvas2D QML            >= 6.12
qt_add_qml_preview        >= 6.12
StyleKit                  >= 6.12
```

## QRangeModel adapter

Option：

```text
LFQUICK_BUILD_QRANGE_MODEL_ADAPTER=OFF
```

Target：

```text
LfQuick::RangeModelAdapter
RangeModelBridge
```

对外继续只暴露 `QAbstractItemModel*`，Qt 6.8 public contract 看不到 QRangeModel。

Bridge 使用 `QList<QVariantMap>` backing range + explicit role names；6.12 时可打开 supported drag/drop actions。

## Canvas2D adapter

Option：

```text
LFQUICK_EXPERIMENTAL_CANVAS_PAINTER=OFF
```

Qt 6.12 declarative adapter：

```text
LfQuick::CanvasPainterAdapter
LfQuick.CanvasPainter
Canvas2DSurface
```

它是 Technology Preview，而且 Qt Canvas Painter 存在独立许可要求，因此默认关闭，不允许成为 Charts/Medical Demo 默认依赖。

## StyleKit

Option：

```text
LFQUICK_EXPERIMENTAL_STYLEKIT=OFF
```

`StyleKitBridge` 会先验证 candidate Style，再切 QApplication style；无效 Style 不会把当前 QWidget style 换成 fallback。

详见 `docs/QT_OPTIONAL_ADAPTERS.md`。

---

# 13. Local Developer Tooling

`tools/` 已具备：

```text
check_architecture.py
lfquick_lint.py
lfquick_gen_vm.py
lfquick_gen_token.py
lfquick_deploy_check.py
lfquick-screenshot (optional C++ target)
```

用途：

```text
architecture / QML lint
QObject ViewModel scaffold
semantic token generation
deploy manifest validation
local deterministic QML screenshot
```

Python self-test：`tools/test_tooling.py`；检测到 Python 时会注册进 CTest。

详见 `tools/README.md`。

---

# 14. Demo 状态

Medical Demo 页面：

```text
Dashboard
Samples
Sample Register
Sample Rack
Results
Results Review
Charts (optional)
Instrument
Alarms
Developer
```

主要纵向验证：

```text
Scanner/InputRouter
Validation / Permission
NavigationGuard
Overlay
DataGrid
Charts
Diagnostics
Design System
```

Hybrid Demo：

```text
QML Page <-> PageRouter <-> QWidget Page
WidgetFocusBridge
WidgetOverlayBackend
WindowRouter
QuickSurface telemetry
broken-QML -> legacy fallback
Diagnostics export
```

所有 Demo 数据均为 synthetic，不是医疗诊断软件。

---

# 15. 当前剩余工作

功能层不再优先扩张新的大模块。

下一阶段严格按：

1. 静态/API 收口，修最终编译前可发现的问题；
2. 本地 Qt 6.8.3 MSVC2022 full Configure + Build；
3. 全量 CTest（包含新增 stress/QML smoke/Python tooling self-test）；
4. 可用 Qt 6.10/6.12 环境时分别验证 RangeModel / StyleKit / Canvas2D gates；
5. 真实 Windows GUI 手工 hardening：DPI/z-order/focus/modal/soak；
6. deploy/package + `lfquick_deploy_check.py`；
7. 最后再恢复 GitHub Actions / release workflow。

在执行第 2-6 步完成前，准确表述必须是：

```text
Major functionality implemented: YES
Source-level hardening substantially implemented: YES
Current HEAD full Build verified: NO
Current HEAD full CTest verified: NO
Current HEAD deploy/package verified: NO
CI workflows present: NO (intentionally removed)
CI Green: NO CLAIM
```

不要重新开始 WindowRouter / Navigation / Charts / Diagnostics / Hot Reload 设计；这些主线已经有实现。