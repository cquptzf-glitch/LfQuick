# LfQuick 开发交接（当前权威版本）

> 更新时间：2026-10-17  
> 仓库：`https://github.com/cquptzf-glitch/LfQuick`  
> 分支策略：**main-only**，普通功能开发不要创建 feature branch。  
> 当前策略：**先把功能与源码级 hardening 做完整，暂时不要恢复 GitHub Actions / CI / release workflow。**

## 1. 新窗口开场直接做什么

不要重新做需求分析，也不要重复实现已经完成的 WindowRouter、Navigation、Charts、Diagnostics、Hot Reload。

直接执行：

1. 读取 main 最新 HEAD；
2. 读取本文件、`docs/IMPLEMENTATION_STATUS.md`、`docs/REFERENCE_POLICY.md`；
3. 保持 main-only；
4. **不要重新创建 `.github/workflows/build.yml` / `release.yml`**，除非用户明确要求恢复 CI；
5. 从“功能 hardening / 手工可重复验证”继续推进；
6. 功能完整后，再统一恢复 MSVC Build / CTest / deploy / artifact / release。

---

# 2. CI / GitHub Actions 当前约束

用户在 2026-10-17 明确要求：

```text
先移除 CI / Action 功能
先保证完成开发再说
```

因此已主动删除：

```text
.github/workflows/build.yml
.github/workflows/release.yml
```

对应提交：

```text
7305c6e chore: remove CI workflow during feature development
61a5fe7 chore: remove release workflow during feature development
```

## 历史验证事实

删除 Actions 之前，Windows Qt 6.8.3 / MSVC2022 曾有一次完整真实绿灯：

```text
Architecture: PASS
Configure: PASS
Build: PASS
CTest: 13/13 PASS
Deploy: PASS
Package validation: PASS
Artifact upload: PASS
```

这只是历史基线，**不能用于宣称当前 HEAD 已编译通过**，因为之后又加入了 Diagnostics / Tooling / StyleKit / hardening 代码。

当前准确表述：

```text
GitHub Actions intentionally removed: YES
Current HEAD full MSVC build verified: NO
Current HEAD full CTest verified: NO
Current HEAD package/release verified: NO
Reason: user explicitly requested feature completion first
```

---

# 3. 已完成主功能，不要重复做

## Core / Runtime

已有：

```text
Error / Result
OperationHandle
Permission contract
Validation contract + validateValue
Command
Event / Dispatcher / Subscription
PageRegistry / ProviderRegistry
PageRouter
Lifecycle / History
None / KeepAlive / Singleton / weighted LRU
NavigationGuard
cross-provider fallback
structured provider diagnostics
WindowRouter
```

关键边界：

```text
LfQuickCore    -> QtCore only
LfQuickRuntime -> 不允许 QWidget / QQuickItem / Qt Quick
```

### Runtime hardening 已完成

```text
NavigationGuard: KeepFirst / ReplaceLatest / Queue
route-global Singleton
IPageInstance::updateParameters()
weighted LRU: count + weight 双限制
PageProviderError: code/message/details
primary/fallback/errorFallback structured diagnostics
```

---

# 4. WindowRouter 已完成

已建立技术栈无关 window contract：

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

```text
open / activate / close / closeAll
single-instance / multi-instance
owner relation
owner close propagation
close guard
provider failure diagnostics
```

WidgetBridge 已有 `WidgetWindowProvider`，Hybrid Demo 有真实顶层 QWidget window 纵向验证。

不要再回到“下一步实现 WindowRouter”的旧计划。

---

# 5. Navigation Shell 已完成

已有：

```text
NavigationItemModel
NavigationView
ApplicationShell
```

支持：

```text
route id
label / group / section
badge
selected route sync
compact navigation
back
permission visible/enabled
QuickRuntime integration
NavigationGuard integration
Overlay integration
```

Medical Demo 已从手写导航迁移到 model-driven shell。

---

# 6. Charts / Visualization MVP 已完成

模块：

```text
LfQuick::Charts
LfQuick.Charts
```

已有：

```text
ChartDataModel
SeriesModel
Axis / Series adapter
ChartView.qml
Qt Graphs adapter
```

Qt 6.8.3 Graphs 实际依赖链已确认：

```text
qtgraphs -> qtquick3d -> qtshadertools
```

Medical Demo 使用 synthetic curve 验证；框架中没有 QC / Calibration 业务算法。

---

# 7. Diagnostics / Telemetry 已完成主体

模块：

```text
LfQuick::Diagnostics
LfQuick.Diagnostics
```

它是**可选叶子模块**，Runtime/UI/DataView/Platform 不得反向依赖它。

## DiagnosticsModel

支持：

```text
bounded event history
dropped event count
generic metrics
timing metrics
snapshot
JSON serialization
atomic JSON export
reset / clear
```

## DiagnosticsHud

独立 QML HUD，可动态加载，不要求业务 QML 静态 import Diagnostics。

## Medical Demo 已接入

采集：

```text
route / depth
cache / singleton / LRU weight
navigation blocked / failed
fallback attempted / activated / failed
QML errors
scanner accepted/rejected counts
DataView row count/live mutation count
DataGrid frame telemetry
```

scanner diagnostics **不记录原始扫码内容**，只记录长度和拒绝原因。

## Hybrid Demo 已接入

采集：

```text
QuickSurface load duration / first frame
focus transfer
route / fallback / QML errors
WindowRouter open / close / blocked / failed
JSON export
```

---

# 8. DataGrid hardening 已推进

新增：

```text
DataGridTelemetry.qml
```

基于 Qt 6.8 `FrameAnimation`，提供：

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

Sample Rack 已真实接入，1 Hz 写入 Diagnostics metrics，避免事件刷屏。

20k / 50k benchmark 继续保留，同时新增进程 RSS：

```text
rss_before_bytes
rss_after_populate_bytes
rss_populate_delta_bytes
```

Windows 使用 Process Memory API / Psapi；Linux 使用 `/proc/self/statm`。

---

# 9. Hot Reload Tooling 已完成核心

构建开关：

```text
LFQUICK_ENABLE_HOT_RELOAD=OFF   // 默认关闭
```

独立模块：

```text
LfQuick::Tooling
LfQuick.Tooling
```

不要把 Hot Reload API 塞进 Core / Runtime。

## QmlHotReloadController

支持：

```text
logical resource URL -> local development file mapping
QFileSystemWatcher file + directory watch
atomic-save 文件替换恢复 watcher
debounce
QQmlEngine::clearComponentCache()
revision
manual reload
explicit unwatch / clear watches
```

## QmlHotReloadHost

采用“候选实例成功后再切换”的双实例策略：

```text
create candidate
-> candidate success
-> restore selected state
-> attach/show candidate
-> retire old item
```

失败：

```text
candidate syntax/create failure
-> old UI remains alive
-> status=Failed
-> expose lastError
```

支持：

```text
initialProperties
preservedProperties
autoReload
status / revision
reloadSucceeded / reloadFailed
```

Controller 持有 watch registry；Host 不自动 unwatch，避免多个 Host 共享同一 logical source 时互相破坏映射。

测试文件：

```text
tests/unit/tst_tooling.cpp
```

覆盖成功替换、状态保留、失败回滚。

---

# 10. Qt 6.12 feature gate / StyleKit adapter

主基线继续：

```text
Qt 6.8.3
```

Qt 新版本能力通过 `cmake/LfQuickQtFeatures.cmake` 隔离：

```text
SafeArea                  >= Qt 6.9
QRangeModel               >= Qt 6.10
QCanvasPainter            >= Qt 6.11
QRangeModel drag/drop     >= Qt 6.12
qt_add_qml_preview        >= Qt 6.12
StyleKit / QStyleKitStyle >= Qt 6.12
```

feature detection 已修正为在 `find_package(Qt6)` 之后执行，避免 `Qt6_VERSION` 尚未建立。

## StyleKit adapter

开关：

```text
LFQUICK_EXPERIMENTAL_STYLEKIT=OFF
```

仅 Qt 6.12+ 可打开。

模块：

```text
LfQuick::StyleKit
LfQuick.StyleKit
StyleKitBridge
```

用途：

```text
同一份 Qt Labs StyleKit QML style
-> Qt Quick Controls
-> QStyleKitStyle / QWidget
```

`StyleKitBridge` 的公共头不暴露 `QStyleKitStyle` 类型，6.8 API 不被污染。

采用 candidate-first：Style 加载/主题验证成功后才 `QApplication::setStyle()`，无效 style 不破坏当前 QWidget 样式。

StyleKit 是 Qt 6.12 technology preview，不应成为默认生产依赖。

---

# 11. WidgetBridge hardening

已有：

```text
QuickSurface
WidgetPageProvider
WidgetWindowProvider
WidgetFocusBridge
WidgetOverlayBackend
```

新增 GUI stress：

```text
tests/unit/tst_widgetbridge_stress.cpp
```

覆盖：

```text
30 轮 QWidget -> Quick -> QWidget Tab/Backtab roundtrip
scanner isolation repeated toggles
modal focus trap
QuickSurface host load
120 次 resize / root-size consistency
metrics reset
```

如果测试环境无法获得真实窗口 focus，会明确 skip，而不是制造假失败。

后续仍值得人工验证：

```text
multi-DPI / screen move
native stacking / z-order
复杂 modal/native child window 组合
```

---

# 12. 架构守卫

`tools/check_architecture.py` 已扩展：

基础层：

```text
Core / Runtime 禁止 QQuick / QWidget / Qt6::Quick / Qt6::Widgets
```

可选叶子模块：

```text
Diagnostics
Tooling
StyleKit
```

以下模块禁止反向 include/link 这些叶子模块：

```text
core
runtime
ui
dataview
platform
widgetbridge
charts
```

Demo 可以组合它们；基础框架不可以被它们绑死。

---

# 13. Medical Demo 当前纵向能力

页面包括：

```text
Dashboard
Samples
Sample Register
Sample Rack
Results
Results Review
Charts（Graphs adapter 打开时）
Instrument
Alarms
Developer
```

### Sample Register

```text
Scanner/InputRouter
+ ValidationRuleTable/FormState
+ external business error
+ FocusNavigator
+ NavigationGuard
+ Busy/Toast
+ DataTableModel append
```

### Sample Rack

```text
Scanner
+ exact proxy filter
+ DataGrid persistence
+ Progress delegate
+ extended selection
+ proxy/source mapping
+ batch/context/keyboard
+ DataGrid FPS/slow-frame telemetry
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

Demo 数据均为 synthetic，不是医疗诊断软件。

---

# 14. 当前下一步

当前已从“主要功能开发”进入源码级 hardening，但仍**不要恢复 CI**。

优先顺序：

1. 继续检查 DataGrid / QuickSurface / WidgetFocusBridge 的边界和可重复 stress；
2. 增加 multi-DPI / z-order 的人工测试 harness/checklist，避免写脆弱的假自动化；
3. 审查 Hot Reload 与 Diagnostics 的生命周期/内存边界；
4. 补 README / 使用示例 / CMake option 文档；
5. 做一次全仓静态 API/架构审查；
6. 用户明确允许后，再恢复：
   - Windows MSVC Build
   - CTest
   - Qt deploy
   - package layout
   - artifact
   - release workflow

---

# 15. 不要做的事

- 不要恢复 GitHub Actions，除非用户明确要求；
- 不要整分支 merge 旧 `feat/runtime-foundation`；
- 不要做巨大 FluentUI NavigationView clone；
- 不要让 Diagnostics/Tooling/StyleKit 成为 Runtime 必需依赖；
- 不要把 Patient/Sample/Reagent 领域模型写进框架；
- 不要把医疗业务 Permission ID 写进框架；
- 不要把 QC/Calibration 算法写进 Charts；
- 不要逆向 FluentUI2/Pro DLL。

外部参考继续遵守 `docs/REFERENCE_POLICY.md`。
