# LfQuick 详细开发计划

> **项目**：LfQuick  
> **基线版本**：Qt 6.8.3  
> **目标兼容**：Qt 6.12 LTS  
> **开发模式**：Monorepo + 模块化 + 分阶段交付 + PoC Gate + PR 驱动  
> **计划版本**：V1.0  
> **依据**：LfQuick V2 / V2.1 架构方案

---

# 1. 开发目标

LfQuick 的目标不是优先堆叠控件，而是先建立一套可以长期支撑大型 Qt 应用演进的基础设施：

```text
Core
→ Runtime
→ Quick Host
→ Overlay/Input/Focus
→ Design System
→ DataGrid
→ Navigation/Window
→ Charts/Visualization
→ Tooling/Diagnostics
```

总体要求：

1. Qt 6.8.3 可稳定运行；
2. Qt 6.12 可平滑升级；
3. Core 不依赖 Widgets / Quick；
4. WidgetBridge 可独立关闭；
5. Runtime 不直接依赖 QWidget / QQuickItem；
6. 新功能以真实应用 PoC 验证；
7. 每阶段都有明确的 Go / No-Go Gate；
8. 所有模块必须具备测试与文档；
9. 不追求“一次性全量 QML 化”；
10. 以混合迁移可持续落地为第一优先级。

---

# 2. 总体开发阶段

建议划分为 10 个阶段：

```text
S0  Repository / Engineering Baseline
S1  Core Foundation
S2  Runtime / PageProvider
S3  QuickSurface / Single Engine
S4  Overlay / Input / Focus
S5  Design System / Base Controls
S6  DataGrid
S7  Navigation / Windowing
S8  Charts / Visualization
S9  Diagnostics / Tooling / Qt 6.12
```

其中：

```text
S0-S4 = 框架生存线
S5-S6 = 业务迁移能力线
S7-S8 = 产品能力增强线
S9    = 工程化与长期演进
```

---

# 3. 开发原则

## 3.1 不允许跨阶段偷跑大规模业务迁移

在以下能力未完成前：

```text
QuickSurface
Overlay
Focus
Scanner
DataGrid PoC
```

不允许大规模迁移真实业务页面。

---

## 3.2 所有高风险模块必须先 PoC

高风险模块：

```text
QQuickWidget / QuickSurface
DataGrid
Overlay
FocusWalker
Scanner
Qt Graphs / Legacy QCustomPlot
Qt 6.12 Hot Reload
```

PoC 通过后才正式框架化。

---

## 3.3 核心边界必须通过自动检查

必须自动检查：

```text
Core 不依赖 QtQuick
Core 不依赖 QtWidgets
Runtime 不 include QWidget
UI 不依赖 LfWidget
QML 不 import LfQuick.impl
禁止魔法色值
禁止 Qt.labs.* 进入核心 Public API
```

---

# 4. 仓库结构

第一版直接按以下结构创建：

```text
LfQuick/
├── CMakeLists.txt
├── cmake/
├── src/
│   ├── core/
│   ├── runtime/
│   ├── ui/
│   ├── dataview/
│   ├── platform/
│   ├── widgetbridge/
│   ├── charts/
│   └── diagnostics/
├── qml/
│   ├── LfQuick/
│   ├── LfQuick/DataView/
│   └── LfQuick/Charts/
├── styles/
│   ├── Fluent/
│   └── Brand/
├── tests/
│   ├── unit/
│   ├── qml/
│   ├── integration/
│   ├── visual/
│   └── performance/
├── examples/
│   ├── gallery/
│   ├── quicksurface/
│   ├── datagrid/
│   └── migration/
├── tools/
└── docs/
```

---

# 5. Git / Branch / PR 规范

## 5.1 主分支

```text
main
```

要求：

- 永远可编译；
- 永远通过 CI；
- 不直接提交实验代码。

---

## 5.2 功能分支

格式：

```text
feat/core-command
feat/runtime-page-provider
feat/ui-overlay
feat/datagrid-selection
fix/quicksurface-focus
refactor/theme-token
test/datagrid-performance
docs/architecture
```

---

## 5.3 PR 规则

一个 PR 尽量只做一个完整主题。

PR 必须包含：

```text
Why
What
How
Tests
Risks
Compatibility
```

模板：

```md
## Why

## What

## Implementation

## Tests

## Risk

## Compatibility
- Qt 6.8.3
- Qt 6.12
```

---

# 6. GitHub Issue 标签

建议建立：

```text
type:feature
type:bug
type:refactor
type:test
type:docs
type:performance
type:architecture

area:core
area:runtime
area:ui
area:dataview
area:widgetbridge
area:platform
area:charts
area:diagnostics
area:tooling

priority:P0
priority:P1
priority:P2
priority:P3

status:blocked
status:ready
status:in-progress
status:review

risk:high
risk:medium
risk:low

qt:6.8
qt:6.12
```

---

# 7. Milestone 规划

建议 GitHub Milestone：

```text
M0 Repository Ready
M1 Core Foundation
M2 Runtime Ready
M3 QuickSurface Ready
M4 Interaction Ready
M5 UI Foundation
M6 DataGrid MVP
M7 Application UI
M8 Visualization
M9 Production Hardening
M10 Qt 6.12
```

---

# 8. S0 — Repository / Engineering Baseline

## 目标

建立后续开发的基础环境。

## 任务

### S0.1 项目骨架

创建：

```text
src/
qml/
styles/
tests/
examples/
docs/
tools/
```

### S0.2 CMake

实现：

```text
LfQuickOptions.cmake
LfQuickVersion.cmake
LfQuickQtFeatures.cmake
LfQuickInstall.cmake
```

### S0.3 构建选项

```cmake
LFQUICK_BUILD_WIDGET_BRIDGE
LFQUICK_BUILD_CHARTS
LFQUICK_BUILD_DIAGNOSTICS
LFQUICK_BUILD_FLUENT_STYLE
LFQUICK_BUILD_BRAND_STYLE
LFQUICK_ENABLE_HOT_RELOAD
LFQUICK_ENABLE_QMLTC
```

### S0.4 CI

第一阶段：

```text
Windows
Qt 6.8.3
MSVC 2022
Debug
Release
```

后续加入：

```text
Qt 6.12
```

### S0.5 Code Style

加入：

```text
.clang-format
.clang-tidy
.editorconfig
qmlformat
qmllint
```

### S0.6 基础测试

```text
QtTest smoke
Qt Quick Test smoke
```

## 交付物

```text
repo builds
empty library targets
CI green
```

## 验收

```text
cmake configure PASS
Debug PASS
Release PASS
unit test PASS
qml test PASS
```

---

# 9. S1 — Core Foundation

## 目标

实现 QtCore-only 的基础层。

## 模块

```text
LfQuickCore
```

## 任务

### S1.1 Error / Result

实现：

```text
LfError
LfResult<T>
```

### S1.2 Command Contract

实现：

```text
ICommand
CommandState
CommandResult
```

不引入 QML。

### S1.3 Event Contract

```text
IEventSource
Subscription
Dispatcher
```

### S1.4 Validation Contract

```text
ValidationRule
ValidationResult
IValidationRuleProvider
```

### S1.5 Permission Contract

```text
PermissionState
IPermissionProvider
```

### S1.6 Route Contract

```text
RouteId
RouteRequest
RouteResult
```

### S1.7 OperationHandle

用于长生命周期后台操作。

### S1.8 Logging Hook

定义：

```text
ILogSink
TelemetryEvent
```

## 测试

```text
Result
Command
Subscription lifecycle
Validation contract
Permission contract
Operation state
```

## 验收

硬条件：

```text
LfQuickCore
only links Qt6::Core
```

---

# 10. S2 — Runtime / PageProvider

## 目标

构建 View-Agnostic Runtime。

## 模块

```text
LfQuickRuntime
```

## 任务

### S2.1 PageDescriptor

```text
route
provider
source
cachePolicy
fallbackPolicy
metadata
```

### S2.2 IPageProvider

```text
id()
canCreate()
create()
```

### S2.3 IPageInstance

```text
activate
deactivate
suspend
resume
destroy
```

### S2.4 PageRegistry

```text
register
find
remove
```

### S2.5 PageRouter

实现：

```text
push
replace
back
forward
reset
```

### S2.6 Lifecycle

状态：

```text
Created
Loading
Ready
Active
Suspended
Error
Destroyed
```

### S2.7 Cache Policy

```text
None
Singleton
KeepAlive
LRU
```

### S2.8 FeatureSwitch

```text
Auto
Quick
Legacy
```

### S2.9 FallbackPolicy

```text
None
ErrorPage
LegacyBeforeActivate
LegacyIfStateless
```

### S2.10 Telemetry

始终保留轻量：

```text
route
load duration
create duration
fallback reason
error
```

## 验收

```text
Runtime 不 include QWidget
Runtime 不 include QQuickItem
Unit test coverage 基础达标
```

---

# 11. S3 — QuickSurface / Single Engine

## 目标

验证迁移阶段最重要的宿主架构。

## PoC 架构

```text
QWidget MainWindow
      ↓
QStackedWidget
      ↓
QuickSurface
      ↓
QQuickWidget
      ↓
QuickHostRoot.qml
      ↓
PageRouterView
```

## 任务

### S3.1 QuickPageProvider

### S3.2 QuickSurface

职责：

```text
single QQuickWidget
single QQmlEngine
focus
resize
error
first frame
```

### S3.3 QuickHostRoot.qml

只加载一次。

### S3.4 Page Loader

使用：

```qml
Loader.setSource(url, initialProperties)
```

兼容 Qt 6.8.3。

### S3.5 VM Injection

禁止大面积：

```cpp
rootContext()->setContextProperty
```

优先：

```text
required property
initial properties
```

### S3.6 QML Error Handling

收集：

```text
QQmlError
component error
load error
```

### S3.7 First Frame

记录：

```text
page create
first frame
ready
```

### S3.8 Multi-page Stress Test

1000 次：

```text
push/pop
create/destroy
```

## 验收

```text
Single QQuickWidget
Single QQmlEngine
1000 次切页无持续内存增长
QML 错误不崩主程序
```

---

# 12. S4 — Overlay / Input / Focus

## 目标

完成真实迁移最重要的交互基础设施。

---

## S4A Overlay

### 任务

```text
UiSurface
OverlayCoordinator
OverlayStack
IOverlayBackend
QuickOverlayBackend
WidgetOverlayBackend
```

组件：

```text
MessageBox
Dialog
Toast
Busy
Tooltip
Popup
```

### Busy

实现：

```text
BusyToken
```

避免 start/stop 配对错误。

### Toast

支持：

```text
severity
position
duration
deduplicate
maxVisible
```

---

## S4B Focus

### 任务

```text
FocusGroup
FocusManager
FocusWalker
```

支持：

```text
Native
Explicit
Geometric
Grid
```

### Geometric Algorithm

排序：

```text
group
row tolerance
y
x / RTL
explicit priority
stable order
```

### Diagnostics

```text
dumpOrder
visualize
```

---

## S4C Input

### 任务

```text
IInputSource
InputRouter
InputTarget
HotkeyRouter
```

### Scanner

实现：

```text
KeyboardWedgeScanner
MockScanner
```

后续：

```text
HID
Serial
```

### Scanner Session

```text
prefix
suffix
timeout
minLength
exclusive
```

## 验收

```text
Dialog 无点穿
Busy token 正确
Focus order 可 dump
1000 次 scanner 测试无丢码
```

---

# 13. S5 — Design System / Base Controls

## 目标

建立真正 Style-Agnostic 的 UI 基础。

## S5.1 Token Schema

```text
Primitive
Semantic
Component
```

## S5.2 Token Generator

输入：

```text
yaml
```

输出：

```text
QML
C++
QSS legacy adapter
```

## S5.3 Theme

```text
Light
Dark
HighContrast
Accent
Density
Motion
```

## S5.4 Attached Context

统一：

```qml
Lf.permission
Lf.validationRule
Lf.focusPolicy
Lf.acceptScanner
Lf.helpContext
```

## S5.5 StateLayer

状态：

```text
normal
hover
pressed
focus
checked
disabled
selected
```

## S5.6 Fluent Style

第一版：

```text
Button
TextField
CheckBox
RadioButton
Switch
ComboBox
Slider
ProgressBar
```

## S5.7 Brand Test Style

必须同步至少实现：

```text
Button
TextField
Switch
Dialog
```

用于证明 Core 未 Fluent-coupled。

## S5.8 Base Controls

```text
SearchBox
NumberInput
RangeInput
FilterInput
PasswordField
ProgressRing
StatusIndicator
```

## S5.9 Date / Time

优先：

```text
DatePicker
TimePicker
DateTimePicker
DateRangePicker
```

## 验收

```text
删除 Fluent Style 后，Test/Brand Style 可运行
无 magic color
无 Core → Style 依赖
```

---

# 14. S6 — DataGrid

## 目标

完成最关键业务组件。

## 任务

### S6.1 DataGrid Core

```text
DataGrid
DataGridColumn
DataGridHeader
DataGridViewport
```

### S6.2 ModelRoleAdapter

兼容旧 Model。

### S6.3 Selection

优先复用：

```text
QItemSelectionModel
```

### S6.4 Sort / Filter

优先：

```text
QSortFilterProxyModel
```

### S6.5 Delegate

```text
Text
Check
Progress
Combo
Icon
Badge
Button
Custom
```

### S6.6 EditSession

```text
begin
temporary value
validate
commit
rollback
```

### S6.7 Keyboard Controller

```text
Up
Down
Left
Right
Tab
Shift+Tab
Enter
Esc
Ctrl+A
```

### S6.8 Column Config

```text
width
visible
order
sort
filter
```

必须带：

```text
schemaVersion
tableId
modelSchema
```

### S6.9 Performance Matrix

测试：

```text
5k rows
20k rows
50k rows

8 cols
20 cols
40 cols

1 / 10 / 100 update/sec
```

指标：

```text
P95 frame time
scroll latency
update latency
memory
first population
```

### S6.10 Real Table PoC

必须拿真实业务复杂表验证。

## Go Gate

如果以下失败：

```text
selection
edit
header
config
keyboard
performance
```

则暂停大规模迁移。

---

# 15. S7 — Navigation / Windowing

## Navigation

拆分：

```text
NavigationModel
NavigationController
NavigationView
NavigationLayout
NavigationDelegate
ResponsivePolicy
```

Navigation 不拥有 Router。

---

## Windowing

实现：

```text
FramelessWindow
AppBar
WindowController
WindowBackend
WindowCapabilities
WindowRouter
```

Windows 第一优先。

---

## Platform

```text
SystemAppearance
ScreenService
ClipboardService
FileWatcher
RuntimeCapabilities
```

---

# 16. S8 — Charts / Visualization

## Charts

默认：

```text
QtGraphsBackend
```

兼容：

```text
LegacyQCustomPlotBackend
```

高级：

```text
CustomQuickBackend
```

Qt 6.12：

```text
CanvasPainterBackend
```

---

## Visualization

实现：

```text
StateLight
StatusIndicator
LevelIndicator
BottleView
RackView
SlotView
Gauge
ProcessSteps
ConnectionIndicator
```

业务专用：

```text
ReagentBottle
SampleBasket
LJChart
CalibrationCurve
```

放产品扩展。

---

# 17. S9 — Diagnostics / Tooling / Qt 6.12

## Diagnostics

Always-On：

```text
route
page timing
qml error
fallback
version
```

Development：

```text
FPS HUD
Object Inspector
Focus Inspector
Overlay Inspector
Route Inspector
```

---

## Tooling

实现：

```text
lfquick-gen-vm
lfquick-gen-token
lfquick-lint
lfquick-deploy-check
lfquick-screenshot
```

---

## Qt 6.12

逐项适配：

```text
QML Hot Reload
Canvas Painter
QRangeModel adapter
SafeArea
StyleKit experiment
```

规则：

```text
6.12-only API 不污染 public contract
```

---

# 18. 第一轮真实业务迁移计划

框架 S4 完成后开始第一批。

## 批次 A

10 个简单页面：

```text
Button/Input/Layout/Dialog
```

目标：

验证开发效率。

---

## 批次 B

5 个中等页面：

```text
Model
Permission
Validation
Scanner
DateTime
```

---

## 批次 C

3 个复杂页面：

```text
DataGrid
Chart
复杂 Focus
```

---

# 19. 页面迁移 DoD

每页必须：

```text
VM extracted
QML 无领域 include
lifecycle correct
focus verified
permission verified
validation verified
scanner verified if applicable
4 language screenshot
fallback configured
performance measured
memory release checked
```

---

# 20. Component DoD

每个公共组件必须：

```text
API documented
Light
Dark
Compact
Touch
Keyboard
Focus
Accessibility
High Contrast
Visual test
Quick test
qmllint pass
no magic color
no Fluent hard dependency
```

---

# 21. Performance Gate

建议最低目标：

```text
cold startup +10% max
normal page first frame < 200ms
normal interaction ~60fps target
5000 rows minimum
72h no continuous memory growth
```

DataGrid 独立用更严格 benchmark。

---

# 22. Safety / Medical Quality Gate

对于关键 UI：

```text
Alarm
Permission
Validation
Operation
```

规则：

```text
UI permission != security permission
UI validation != domain validation
Toast != critical alarm
Page lifetime != device operation lifetime
```

---

# 23. 每阶段交付标准

每个阶段必须提交：

```text
Code
Unit tests
Integration tests
Example
README / docs
Benchmark if needed
Migration note
```

没有文档和测试的模块不算完成。

---

# 24. 建议首批 GitHub Issues

创建以下第一批 Issue：

```text
#1  Bootstrap repository and CMake
#2  Add CI for Qt 6.8.3
#3  Implement LfQuickCore skeleton
#4  Implement Result/Error
#5  Implement Command contract
#6  Implement Event contract
#7  Implement PageProvider contract
#8  Implement PageRegistry
#9  Implement PageRouter
#10 Implement lifecycle state machine
#11 Build Single QuickSurface PoC
#12 Build QuickHostRoot.qml
#13 Add QML error collector
#14 Add first-frame telemetry
#15 Build Overlay PoC
#16 Build FocusWalker PoC
#17 Build Scanner InputRouter PoC
#18 Build DataGrid PoC
#19 Add Qt Graphs PoC
#20 Add deployment smoke test
```

---

# 25. 推荐近期开发顺序

后续真正开始开发时，严格按：

```text
Step 1
Repository + CMake + CI

Step 2
LfQuickCore

Step 3
Runtime + PageProvider

Step 4
Single QuickSurface PoC

Step 5
Overlay + Focus + Scanner

Step 6
Theme + Style + Base Controls

Step 7
DataGrid

Step 8
真实页面验证
```

不要提前开发：

```text
完整 NavigationView
完整 Chart system
大量高级控件
复杂动画
```

---

# 26. 每次开发任务的执行模板

以后每个任务统一按照：

```text
1. Review current code
2. Define scope
3. Design API
4. Implement
5. Add tests
6. Add example
7. Run CI
8. Review architecture boundary
9. Commit
10. PR
```

---

# 27. 每次 PR 的检查清单

```text
[ ] Qt 6.8.3 build
[ ] no forbidden dependency
[ ] unit tests
[ ] QML tests
[ ] qmllint
[ ] public API documented
[ ] performance impact reviewed
[ ] lifetime reviewed
[ ] thread boundary reviewed
[ ] migration compatibility reviewed
```

---

# 28. 版本计划

## 0.1

```text
Core
Runtime skeleton
QuickSurface PoC
```

## 0.2

```text
Overlay
Input
Focus
Theme
```

## 0.3

```text
Base Controls
Brand Style
```

## 0.4

```text
DataGrid MVP
```

## 0.5

```text
Real page migration
```

## 0.8

```text
Navigation
Windowing
Charts
Diagnostics
```

## 1.0

```text
Qt 6.8.3 production-ready
```

## 2.0

```text
Qt 6.12 optimized
```

---

# 29. 项目管理建议

建议以：

```text
Milestone
→ Epic Issue
→ Task Issue
→ PR
```

管理。

例如：

```text
Milestone: M3 QuickSurface Ready

Epic:
[Runtime] QuickSurface migration host

Tasks:
- Single QQuickWidget
- Shared QQmlEngine
- QuickHostRoot
- Loader
- error handling
- first-frame telemetry
- focus bridge
```

---

# 30. 后续开发约定

后续开发建议按照本文推进。

每完成一个阶段：

1. 更新对应 Milestone；
2. 更新 README 状态；
3. 更新 Architecture Decision Record；
4. 更新开发计划中的完成状态；
5. 再进入下一阶段。

禁止因为某个组件“看起来比较容易”而绕开关键链路。

---

# 31. 第一阶段立即执行清单

当仓库创建完成后，首先执行：

```text
[ ] 创建基础目录
[ ] 创建 CMakeLists.txt
[ ] 创建 cmake/LfQuickOptions.cmake
[ ] 创建 core target
[ ] 创建 runtime target
[ ] 创建 ui target
[ ] 创建 dataview target
[ ] 创建 widgetbridge optional target
[ ] 添加 example smoke app
[ ] 添加 QtTest smoke
[ ] 添加 QuickTest smoke
[ ] 添加 GitHub Actions
[ ] 验证 Qt 6.8.3 Debug
[ ] 验证 Qt 6.8.3 Release
```

完成后进入 S1。

---

# 32. 最终执行策略

整个项目遵循：

```text
Architecture First
PoC Before Frameworkization
Framework Before Mass Migration
Test Before Expansion
Compatibility Before Optimization
```

优先级：

```text
稳定性
>
边界正确
>
迁移能力
>
性能
>
开发体验
>
视觉完整度
```

对于医疗/工业软件，这是最合适的排序。
