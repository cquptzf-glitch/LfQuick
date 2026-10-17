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

Qt 6.12-only 能力必须通过独立 adapter / feature gate 进入，不污染 Qt 6.8.3 公共 API。

---

# 2. 构建选项

当前 `cmake/LfQuickOptions.cmake`：

```text
BUILD_SHARED_LIBS                  ON
LFQUICK_BUILD_PLATFORM             ON
LFQUICK_BUILD_UI                   ON
LFQUICK_BUILD_DATAVIEW             ON
LFQUICK_BUILD_WIDGET_BRIDGE        OFF
LFQUICK_BUILD_HYBRID_DEMO          OFF
LFQUICK_BUILD_CHARTS               OFF
LFQUICK_BUILD_QT_GRAPHS_ADAPTER    OFF
LFQUICK_BUILD_DIAGNOSTICS          ON
LFQUICK_BUILD_MEDICAL_DEMO         ON
LFQUICK_BUILD_BENCHMARKS           OFF
LFQUICK_ENABLE_HOT_RELOAD          OFF
LFQUICK_ENABLE_QMLTC               OFF
LFQUICK_EXPERIMENTAL_STYLEKIT      OFF
```

原则：开发辅助、迁移桥、Graphs、Qt 6.12 实验能力默认都不是 Core/Runtime 的必需依赖。

---

# 3. 架构边界

## Core

```text
LfQuickCore -> QtCore only
```

已有：

- Error / Result
- OperationHandle
- Permission contract
- Validation contract / `validateValue()`
- Command
- Event / Dispatcher / Subscription
- Route contracts

禁止进入 Qt Quick / QWidget / 产品业务依赖。

## Runtime

```text
LfQuickRuntime -> Core
```

禁止：

```text
QWidget
QQuickItem
Qt6::Widgets
Qt6::Quick
```

已有：

- PageRegistry / PageProviderRegistry
- PageRouter
- IPageProvider / IPageInstance
- lifecycle / history
- None / KeepAlive / Singleton / weighted LRU
- cross-provider fallback
- structured provider error
- NavigationGuard
- WindowRouter

## Optional leaf modules

以下模块必须保持叶子性质：

```text
Diagnostics
Tooling
StyleKit
```

`tools/check_architecture.py` 已禁止 core/runtime/ui/dataview/platform/widgetbridge/charts 反向 include/link 这些叶子模块。

---

# 4. Runtime hardening

## NavigationGuard

支持：

```text
KeepFirst
ReplaceLatest
Queue
```

Queue 有界，origin route stale request 会安全 drop，避免旧确认请求触发隐式二次导航。

## Structured provider failure

`IPageProvider`：

```text
lastError()       // legacy
lastErrorInfo()   // structured
```

结构：

```text
PageProviderError
- code
- message
- details
```

Router diagnostics 同时有：

```text
primaryFailure
fallbackFailure
errorFallbackFailure
```

Diagnostics 直接消费结构化数据，不解析拼接字符串。

## Route-global Singleton

```text
route -> one shared instance
history entries -> shared reference
history pop != singleton destroy
router reset -> singleton cache destroy
```

`IPageInstance::updateParameters()` 用于 singleton reuse 时刷新页面参数。

## Weighted LRU

支持：

```text
maxRetainedLruPages
maxRetainedLruWeight
PageDescriptor.cacheWeight
```

淘汰条件：

```text
count > limit OR totalWeight > limit
```

按 inactive LRU 的 lastTouched 从旧到新淘汰。

---

# 5. WindowRouter — 已完成

Runtime contract：

```text
WindowDescriptor
WindowOpenPolicy
OwnerClosePolicy
WindowState
IWindowProvider
IWindowInstance
WindowProviderRegistry
WindowRouter
```

支持：

- open / activate / close / closeAll
- single-instance / multi-instance
- owner / child relation
- owner close propagation
- close guard
- provider failure diagnostics

WidgetBridge 已实现 `WidgetWindowProvider`。

Hybrid Demo 使用真实 top-level QWidget 验证 single/multi instance、owned child、close guard。

---

# 6. UI / Navigation Shell — 已完成

已有：

- QuickRuntime / QuickPageProvider
- QmlErrorCollector / PageLoadError
- OverlayCoordinator
- Busy / Toast / Dialog
- FocusNavigator
- NavigationGuardController
- PermissionEvaluator
- `Lf.permission`
- ValidationEvaluator
- ValidationRuleTable
- ValidationFormState
- NavigationItemModel
- NavigationView
- ApplicationShell

Navigation shell 支持：

```text
route
label/icon
section/group
badge
compact/expanded
selected route sync
back
permission visible/enabled
QuickRuntime
NavigationGuard
Overlay
```

Medical Demo 已使用 model-driven ApplicationShell，不再依赖手写 NavigationRail。

---

# 7. Permission / Validation

## Permission attached API

```qml
Lf.permission
```

支持：

```text
evaluator
permissionId
context
active
state
allowed
visible
enabled
editable
```

约束：

- UI permission 只做 experience mapping；
- Command / Domain 仍需权威权限校验；
- framework 不写医疗业务 Permission ID；
- configured evaluator missing 时 fail-closed。

## Validation

已有：

```text
ValidationRuleTable
ValidationFormState
ValidationFieldState
```

支持 local rule / provider fallback / touched / external business error / aggregate validity，并复用 Core `validateValue()`。

---

# 8. Overlay dual-stack — 已完成

统一协议：

```text
IOverlayBackend
ToastRequest
DialogRequest
OverlayCoordinator
```

QML：

```text
ToastPresenter
DialogPresenter
OverlayHost
```

QWidget：

```text
WidgetOverlayBackend
QFrame Toast
non-blocking QMessageBox::open()
```

NavigationGuard confirmation/hard-block 与业务 Toast 已统一走 OverlayCoordinator。

---

# 9. WidgetBridge

已有：

```text
QuickSurface
WidgetPageProvider
WidgetWindowProvider
WidgetFocusBridge
WidgetOverlayBackend
```

## QuickSurface metrics

已有：

```text
ready / loadState / lastError
loadDurationMs
firstFrameMs
effectiveDevicePixelRatio
surfaceSize
screenName
resizeCount
```

监听 Qt 6.8 支持的 DevicePixelRatioChange / ScreenChangeInternal，便于 multi-DPI 人工验证。

## WidgetFocusBridge

支持：

```text
QWidget <-> QML Tab / Backtab
remember / restore
scanner isolation
modal focus trap
boundary transfer policy
```

新增 `tst_widgetbridge_stress.cpp`：

- 30 轮双向 focus roundtrip；
- scanner isolation 重复开关；
- modal focus trap；
- QuickSurface 120 次 resize；
- root size consistency；
- metric reset。

无真实窗口 focus 的测试环境会明确 skip。

---

# 10. DataView

已有：

- DataTableModel
- DataFilterProxyModel
- DataGrid
- stable column id
- ColumnStateModel / ColumnConfigStore
- width/order/visibility persistence
- DataEditTransaction
- typed delegates
- batch selection
- context menu
- keyboard/cross-grid navigation
- proxy/source mapping

## DataGridTelemetry

新增可选 QML helper：

```text
fps
frameTimeMs
maxFrameTimeMs
sampleCount
slowFrameCount
slowFrameThresholdMs
firstVisibleMs
snapshot()
reset()
```

它基于 Qt 6.8 `FrameAnimation`，不依赖 Diagnostics。

Medical Demo Sample Rack 已按 1 Hz 将这些数据送入 Diagnostics metrics。

## Benchmark

20k / 50k × 8 / 20 / 40 columns 保留：

```text
populate
filter
sort
1/10/100 update batch
```

新增 RSS：

```text
rss_before_bytes
rss_after_populate_bytes
rss_populate_delta_bytes
```

Windows 使用 Psapi，Linux 使用 `/proc/self/statm`。

---

# 11. Platform / Scanner

已有：

```text
ScannerSession
KeyboardWedgeScanner
InputRouter
```

业务页面使用：

```text
pageActive + activeFocus
```

双门控避免后台 cached page 消费 scanner input。

Diagnostics 不记录原始扫码值，只记录长度/拒绝原因等非业务载荷指标。

---

# 12. Charts / Visualization — MVP 已完成

模块：

```text
LfQuick::Charts
LfQuick.Charts
```

已有：

```text
ChartDataModel
SeriesModel
QtGraphsSeriesAdapter
QtGraphsAxisAdapter
ChartView.qml
```

Medical Demo 使用 synthetic chart data。

Qt 6.8.3 Graphs CI 历史上已确认依赖：

```text
qtgraphs
qtquick3d
qtshadertools
```

框架不包含 QC / Calibration 算法。

---

# 13. Diagnostics / Telemetry — 主体完成

模块：

```text
LfQuick::Diagnostics
LfQuick.Diagnostics
```

## DiagnosticsModel

支持：

- bounded event model
- dropped count
- generic metrics
- timing
- event lookup
- snapshot
- JSON
- atomic export
- reset / clear

## DiagnosticsHud

独立 QML HUD，可在 Demo 中动态加载。

## Medical Demo

记录：

```text
route/depth
cache/singleton/LRU
navigation blocked/failed
fallback
QML errors
scanner counters
DataView rows/live mutations
DataGrid FPS/slow frames/first visible
```

## Hybrid Demo

记录：

```text
QuickSurface load/first frame
focus transfers
route/fallback/QML errors
WindowRouter lifecycle
JSON export
```

---

# 14. Hot Reload Tooling — 核心完成

开关：

```text
LFQUICK_ENABLE_HOT_RELOAD=OFF
```

模块：

```text
LfQuick::Tooling
LfQuick.Tooling
```

## QmlHotReloadController

支持：

```text
logical URL -> local development URL
file + directory watch
debounce
atomic-save watcher recovery
QQmlEngine::clearComponentCache()
revision
manual reload
explicit unwatch/clear
```

## QmlHotReloadHost

采用 candidate-first atomic swap：

```text
new item create success
-> restore preservedProperties
-> attach/show new item
-> retire old item
```

创建/语法失败：

```text
old item stays alive
status=Failed
lastError available
```

支持：

```text
initialProperties
preservedProperties
autoReload
revision
reloadSucceeded/reloadFailed
```

watch registry 由 Controller 管理，Host 不自动 unwatch，避免共享 logical source 的 Host 互相破坏。

测试覆盖 success/preserve/failure rollback。

---

# 15. Qt version feature gate / Qt 6.12 adapter

`cmake/LfQuickQtFeatures.cmake`：

```text
SafeArea                  >= 6.9
QRangeModel               >= 6.10
QCanvasPainter            >= 6.11
QRangeModel drag/drop     >= 6.12
qt_add_qml_preview        >= 6.12
StyleKit / QStyleKitStyle >= 6.12
```

feature detection 已放到 `find_package(Qt6 Core)` 之后，确保 `Qt6_VERSION` 有效。

Qt 6.12 `qmlpreview` 对 Qt6::Qml executable 默认就会配置，因此不重复造 production API；自定义 Tooling 解决的是跨版本、状态保留和失败回滚。

## StyleKit adapter

开关：

```text
LFQUICK_EXPERIMENTAL_STYLEKIT=OFF
```

仅 Qt 6.12+：

```text
LfQuick::StyleKit
LfQuick.StyleKit
StyleKitBridge
```

公共 API 不暴露 `QStyleKitStyle` 类型。

用于：

```text
one Style QML definition
-> Qt Quick Controls
-> QWidget via QStyleKitStyle
```

StyleKit 是 technology preview，默认关闭。

---

# 16. Medical Demo

页面：

```text
Dashboard
Samples
Sample Register
Sample Rack
Results
Results Review
Charts (adapter enabled)
Instrument
Alarms
Developer
```

纵向链路：

### Sample Register

```text
Scanner/InputRouter
+ ValidationRuleTable/FormState
+ external business error
+ FocusNavigator
+ NavigationGuard
+ Overlay
+ DataTableModel append
```

### Sample Rack

```text
Scanner
+ exact filter
+ DataGrid persistence
+ progress delegate
+ extended selection
+ proxy/source mapping
+ batch/context/keyboard
+ DataGrid telemetry
```

### Results Review

```text
DataGrid typed edit
+ Lf.permission
+ Validation
+ NavigationGuard
+ Overlay/Dialog
+ persistence
```

所有 Demo 数据均为 synthetic。

---

# 17. CI / package 状态

用户已明确要求先移除 CI，所以当前仓库**没有**：

```text
.github/workflows/build.yml
.github/workflows/release.yml
```

删除 Actions 之前的历史真实基线曾达到：

```text
Windows Qt 6.8.3 / MSVC2022
Build PASS
CTest 13/13 PASS
Deploy PASS
Package validate PASS
Artifact upload PASS
```

之后新增了 Diagnostics / Tooling / StyleKit / hardening，当前 HEAD 尚未重新做完整编译验证。

因此：

```text
Current HEAD CI Green: NO CLAIM
Current HEAD full build verified: NO
Current HEAD CTest verified: NO
```

这是当前开发策略导致的主动状态，不是未处理的 CI 故障。

---

# 18. 接下来

在不恢复 GitHub Actions 的前提下：

1. 完成 multi-DPI / z-order / native-window 的手工 hardening harness/checklist；
2. 继续审查 Hot Reload / Diagnostics 生命周期与资源释放；
3. 补 README、模块使用示例、CMake option 说明；
4. 做全仓静态 API / include / ownership review；
5. 清理 warning 和明显实现债务；
6. 用户明确允许后，再统一恢复：
   - MSVC Build
   - CTest
   - performance benchmark
   - deploy
   - package
   - artifact
   - release

---

# 19. 明确禁止

- 不要未经用户指令恢复 GitHub Actions；
- 不要整分支 merge 陈旧 `feat/runtime-foundation`；
- 不要做巨大 FluentUI clone；
- 不要让 Diagnostics/Tooling/StyleKit 反向绑住基础模块；
- 不要把医疗领域模型/Permission ID/QC 算法写进 framework；
- 不要逆向 FluentUI2/Pro DLL。

参考边界继续遵守 `docs/REFERENCE_POLICY.md`。
