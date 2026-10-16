# LfQuick 新窗口开发交接

> 交接日期：2026-10-16  
> 仓库：`https://github.com/cquptzf-glitch/LfQuick`  
> Git 策略：**main-only**，不要创建普通 feature branch。  
> 当前开发策略：**先完成功能，再统一收口 CI/CD、性能与 GUI stress**。

## 1. 新窗口开场直接做什么

不要重新做需求分析，也不要重新实现已经完成的 Permission / Validation / Overlay / Runtime cache 功能。

直接：

1. 读取 main 最新 HEAD；
2. 读取：
   - `docs/NEXT_CHAT_HANDOFF.md`
   - `docs/IMPLEMENTATION_STATUS.md`
   - `docs/LfQuick_详细开发计划.md`
   - `docs/LfQuick_V2.1_架构审查与优化方案.md`
   - `docs/REFERENCE_POLICY.md`
3. 确认 main-only；
4. 直接开始 **WindowRouter**；
5. WindowRouter 纵向链路完成后，继续 **轻量 NavigationView / Application Shell**；
6. 不要因为 GitHub Actions 自动失败而中断功能开发，除非用户明确改变“功能优先”指令。

---

# 2. 当前主线基线

本轮功能开发完成时 main 功能 HEAD：

```text
20cdd1bf318652f14a894d3369f9594321d4ff00
feat(runtime): wire weighted cache state through quick runtime
```

随后交接文档提交：

```text
a128f88 docs: update implementation status for permission overlay and runtime hardening
```

新窗口应重新读取 main HEAD，以最新提交为准。

---

# 3. 本轮已经完成，不要重复做

## 3.1 Permission Attached API

已经完成：

```qml
Lf.permission
```

状态：

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

关键提交：

```text
edd3750 feat(ui): add Lf attached permission context API
ca5073a feat(ui): implement reactive attached permission state
5324d53 feat(ui): register Lf attached permission context
8cf55f7 feat(demo): use Lf.permission attached context in results review
```

规则：

- attached permission 只做 UI experience mapping；
- Command / Domain 层必须继续做权威权限校验；
- 不允许把医疗 Permission ID 写进框架；
- 未配置 permissionId 时不影响控件；
- 配置后 evaluator/provider 缺失 fail-closed。

---

## 3.2 Validation RuleTable / FormState

已经完成：

```text
ValidationRuleTable
ValidationFormState
ValidationFieldState
```

支持：

```text
QML local rules
IValidationRuleProvider fallback
field touched state
rule error
external business error
aggregate form valid/errorCount
rule change -> revalidate touched fields
```

继续复用 Core：

```cpp
validateValue(value, ValidationRule)
```

不要重新造 validator。

Sample Register 已迁移，不再维护 barcode/sampleId/rack 三套独立 error string。

关键提交：

```text
2c0b2c2 feat(validation): add reusable validation rule table
0119415 feat(validation): implement rule lookup and provider fallback
f09721e feat(validation): add field and form validation state
5ff6e84 feat(validation): implement aggregate form feedback state
f201ae7 feat(demo): migrate sample register to form validation state
```

---

## 3.3 Toast / Dialog dual-stack

已经完成 backend-neutral contract：

```text
IOverlayBackend
ToastRequest
DialogRequest
OverlayCoordinator
```

Toast：

```text
severity
position
duration
dedup
maxVisible
explicit dismiss
```

Dialog：

```text
buttons
default/destructive semantics
serial queue
result signal
cancel fallback
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
QFrame toast
non-blocking QMessageBox::open()
```

Medical Demo：

- NavigationGuard confirm/hard-block 统一走 OverlayCoordinator；
- Sample Register success / scanner reject 走 Toast。

Hybrid Demo：

- 同一 OverlayCoordinator 绑定 WidgetOverlayBackend；
- legacy QWidget 验证同一 Toast/Dialog 协议。

关键提交：

```text
c430291 feat(overlay): define backend-neutral toast and dialog contract
bbac238 feat(overlay): add toast and dialog orchestration
18d1ace feat(overlay): implement toast lifecycle and dialog queue
2effb3b feat(overlay): add QML toast presenter
3129e29 feat(overlay): add QML modal dialog presenter
08b818f feat(widgetbridge): add QWidget overlay backend
59edc0b test(overlay): cover toast policy and dialog backend queue
7d066fe feat(demo): exercise shared overlay backend in hybrid app
```

---

## 3.4 NavigationGuard 多请求策略

原来的潜在错误：已有 confirm dialog 时，后续导航会覆盖 pending request。

现在已完成：

```text
KeepFirst      // default
ReplaceLatest
Queue
```

并有 bounded queue / origin route stale drop 语义。

关键提交：

```text
d154e3e feat(runtime): add navigation guard pending request policies
24bf4c7 feat(runtime): implement guarded navigation queue and drop semantics
e9de201 test(runtime): cover navigation guard pending request policies
```

不要退回“每次 setPending 直接覆盖”的旧实现。

---

## 3.5 Structured Provider Error

`IPageProvider` 已兼容：

```text
lastError()       // legacy string
lastErrorInfo()   // structured
```

结构：

```text
PageProviderError
- code
- message
- details
```

QuickPageProvider / WidgetPageProvider 已接入。

PageRouter diagnostics 同时提供：

```text
primaryError
fallbackError
errorFallbackError
```

和：

```text
primaryFailure
fallbackFailure
errorFallbackFailure
```

关键提交：

```text
24ea1c0 feat(runtime): add structured page provider errors
30a17a8 feat(runtime): provide default structured provider error adapter
a80f523 feat(ui): expose structured QML page provider errors
761c7e5 feat(widgetbridge): expose structured widget provider errors
98a300f feat(runtime): implement route-global singleton weighted LRU and structured provider diagnostics
```

后续 Diagnostics HUD 应直接消费这些结构化数据，不要再解析拼接字符串。

---

## 3.6 Route-global Singleton + Weighted LRU

已经完成真正的 route-global Singleton：

```text
route -> one shared instance
history entries -> shared reference
history pop != singleton destroy
router reset -> destroy singleton cache
```

新增：

```text
IPageInstance::updateParameters()
```

Quick/Widget 内建实例都支持 singleton reuse 时刷新参数。

Weighted LRU：

```text
maxRetainedLruPages
maxRetainedLruWeight
PageDescriptor.cacheWeight
```

淘汰条件：

```text
count > limit OR totalWeight > limit
```

按最旧 inactive LRU 淘汰。

QuickRuntime 已暴露：

```text
maxRetainedLruPages
maxRetainedLruWeight
retainedLruWeight
singletonCount
registerPage(..., cacheWeight)
```

关键提交：

```text
450daf4 feat(runtime): add reusable page parameter update contract
d995314 feat(runtime): add explicit page cache weight
9c742bf feat(runtime): refresh QML singleton parameters on reuse
5ae0417 feat(runtime): refresh widget singleton parameters on reuse
98a300f feat(runtime): implement route-global singleton weighted LRU and structured provider diagnostics
f095988 test(runtime): cover singleton weighted LRU and structured errors
d45625d feat(runtime): expose weighted cache and singleton state to QML
20cdd1b feat(runtime): wire weighted cache state through quick runtime
```

---

# 4. 当前已有框架能力总览

## Core

```text
Error / Result
OperationHandle
Permission contract
Validation contract + validateValue
Command
Event / Dispatcher / Subscription
Route contracts
```

Core 保持 QtCore only。

## Runtime

```text
PageRegistry
PageProviderRegistry
PageRouter
Lifecycle
History
None / Singleton / KeepAlive / weighted LRU
NavigationGuard
cross-provider fallback
provider structured diagnostics
ErrorPage final fallback
```

Runtime 禁止 QWidget / QQuickItem。

## UI

```text
QuickRuntime
QuickPageProvider
QmlErrorCollector
PageLoadError
OverlayCoordinator / Busy / Toast / Dialog
FocusNavigator
NavigationGuardController
PermissionEvaluator
Lf.permission attached context
ValidationEvaluator
ValidationRuleTable
ValidationFormState
```

## WidgetBridge

```text
QuickSurface
WidgetPageProvider
WidgetFocusBridge
FocusTransferPolicy
WidgetOverlayBackend
```

## DataView

```text
DataTableModel
DataFilterProxyModel
DataGrid
stable column id
ColumnStateModel / ColumnConfigStore
width/order/visibility persistence
DataEditTransaction
typed delegates
batch selection
context menu
keyboard/cross-table navigation
proxy/source row mapping
20k/50k benchmark matrix
```

## Platform

```text
ScannerSession
KeyboardWedgeScanner
InputRouter
```

---

# 5. Medical Demo 当前纵向验证

页面：

```text
Dashboard
Samples
Sample Register
Sample Rack
Results
Results Review
Instrument
Alarms
Developer
```

### Sample Register

```text
Scanner
+ InputRouter
+ ValidationRuleTable / ValidationFormState
+ duplicate external error
+ FocusNavigator
+ NavigationGuard
+ Overlay Busy / Toast
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
```

### Results Review

```text
DataGrid typed edit
+ Lf.permission
+ Validation
+ NavigationGuard
+ Overlay/Dialog
+ batch/context
+ persistence
```

Demo 数据是 synthetic，不是医疗诊断软件。

---

# 6. 下一开发批次：严格顺序

## Step 1 — WindowRouter

这是下一窗口第一任务。

目标：建立技术栈无关的 window routing / lifecycle contract。

建议先实现：

```text
WindowDescriptor
WindowId
WindowOpenPolicy
WindowState
IWindowProvider
IWindowInstance
WindowProviderRegistry
WindowRouter
```

至少考虑：

```text
open
activate
close
closeAll
find
single-instance window
multi-instance window
owner / parent window relation
route parameters
window state restore hook
close guard hook
```

架构要求：

```text
Runtime 层不能写 QWidget / QWindow / QQuickWindow
```

技术栈适配放：

```text
LfQuickUi / WidgetBridge / Integration.*
```

第一批不要直接做复杂 docking / MDI。

应补测试：

```text
single instance reuse
multi instance
close lifecycle
provider failure diagnostics
owner close propagation policy
```

并在 Hybrid Demo 做最小纵向验证。

---

## Step 2 — 轻量 NavigationView / Application Shell

不要做巨大 FluentUI NavigationView clone。

先做 model-driven shell：

```text
NavigationItemModel
NavigationSection
NavigationView
ApplicationShell
```

优先能力：

```text
route id
label/icon
section/group
selected route sync
compact/expanded state
back action
badge
permission-visible/enabled integration
QuickRuntime push/back integration
NavigationGuard integration
Overlay integration
```

Medical Demo 当前手写 NavigationRail 可迁到新 model-driven API 做真实验证。

不要把 Patient / Sample / Reagent 菜单写死在框架。

---

## Step 3 — Charts / Visualization MVP

先做 contract，再做 Qt Graphs adapter。

建议：

```text
ChartDataModel / SeriesModel
AxisDescriptor
SeriesDescriptor
ChartView adapter
```

不要把 QC/Calibration 算法写进 Charts。

医疗 Demo 只使用 synthetic curve / histogram 做验证。

---

## Step 4 — Diagnostics HUD / Telemetry

直接利用已有：

```text
route
fallback
provider structured error
cache state
QML errors
QuickSurface timing
DataGrid telemetry
scanner state
```

目标：

```text
DiagnosticsModel
DiagnosticsOverlay / HUD
runtime dump/export
```

不要让 Diagnostics 反向成为 Runtime 必需依赖。

---

## Step 5 — Hot Reload tooling

仅开发态：

- 围绕 single QQmlEngine / QuickSurface；
- 保持 production lifecycle 不变；
- reload 失败必须可恢复；
- 不允许 hot reload API 进入 Core。

---

## Step 6 — Qt 6.12 adapter

主基线继续 Qt 6.8.3。

Qt 6.12-only 功能必须 feature-gate / adapter 化，禁止直接污染 6.8 public API。

---

## Step 7 — 功能完成后统一 Hardening

到这里再统一做：

```text
DataGrid FPS / first population / memory
WidgetFocusBridge GUI stress
QuickSurface DPI / resize / z-order
full MSVC Build
CTest
Demo deploy
artifact
release
```

---

# 7. CI/CD 状态与下一窗口约束

仓库公开后 runner 已经恢复真实 steps。

之前已真实解决：

```text
MSVC vs MinGW toolchain
shared DLL export/import library
RouteResult / IPageProvider / IPermissionProvider ABI
Qt 6.8 qmltyperegistrar include
moc incomplete Q_PROPERTY types
Qt 6.8 QTest API
Windows test runtime DLL staging
```

最后一次已观察到：

```text
full Build: PASS
CTest: 6/7 PASS
tst_dataview: exit code 1, no useful assertion output yet
```

然后用户明确要求：

```text
先不管 CI/CD
先确保所有功能开发完成
最后再统一修改优化
```

因此下一窗口：

- 不要自动把 CI 放回第一优先级；
- 不要因为 push 自动触发 workflow 就停下新功能；
- 不要宣称当前 HEAD CI Green；
- 功能阶段完成后再集中解决 `tst_dataview` 和 package/release。

---

# 8. Git / stale branch

当前存在旧分支：

```text
feat/runtime-foundation
```

已比较：

```text
status: diverged
ahead_by: 9
behind_by: 145
merge base: a4aee9f13f74d1327f3f28c46f9004ddf6e45921
```

它是早期 Runtime/Foundation 工作线，main 已经向前演进很多。

规则：

```text
DO NOT merge entire feat/runtime-foundation into main
```

如确实需要清理：

1. 单 commit 审查；
2. 只迁移 main 真正缺失的内容；
3. 优先做等价实现而不是把旧架构 merge 回来。

仍然 main-only。

---

# 9. 架构边界

继续严格保持：

```text
LfQuickCore      -> QtCore only
LfQuickRuntime   -> Core，不允许 QWidget/QQuickItem
LfQuickUi        -> Core + Runtime + Qt Quick
LfQuickDataView  -> Ui + Qt Model/View/Quick
LfQuickPlatform  -> platform/input abstraction
WidgetBridge     -> optional Qt Widgets / QuickWidgets
Integration.*    -> legacy/product-specific adapters
```

禁止进入 Core：

- EventFunc Adapter
- LfWidget.dll Adapter
- WindowManager Adapter
- QXlsx
- LimeReport
- HookProc
- ScanerHook
- Patient/Sample/Reagent domain model
- 医疗业务 Permission ID
- QC/Calibration 业务算法

---

# 10. External Reference

继续遵守 `docs/REFERENCE_POLICY.md`：

- FluentUI MIT 公开源码允许研究；
- 实际复制/实质衍生保留 MIT 来源与许可证记录；
- FluentUI2 / FluentUI Pro 仅基于公开 API、qmltypes、Demo、行为 clean-room 参考；
- 不逆向 DLL；
- 不还原私有 Pro 实现；
- LfQuick 自身架构与混合迁移目标优先。

---

# 11. 核心目标不要偏离

LfQuick 当前最重要的不是“组件数量”，而是：

```text
旧 QWidget 医疗软件
    ↓
按页面逐步迁移到 QML
    ↓
QML 创建失败可 fallback 到 legacy QWidget
    ↓
路由 / 生命周期 / Focus / Overlay / DataGrid / Scanner / Permission / Validation
行为保持一致
```

WindowRouter / NavigationView 也必须服务于这个目标，而不是为了模仿 FluentUI 做表面组件堆积。
