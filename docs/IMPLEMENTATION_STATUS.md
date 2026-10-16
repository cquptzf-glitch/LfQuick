# LfQuick 当前实现状态

> 更新时间：2026-10-16  
> 开发策略：**main-only**。普通功能不创建 feature branch；后续开发直接提交 main，并保持语义清晰的小提交。  
> 当前阶段策略：**优先完成功能，再统一收口 CI/CD、性能与 GUI stress**。

## 当前基线

- Qt：6.8.3
- C++：20
- CMake：3.24+
- 当前主线：`main`
- 当前功能 HEAD：`20cdd1bf318652f14a894d3369f9594321d4ff00`
- 当前 HEAD 提交：`feat(runtime): wire weighted cache state through quick runtime`

## 本轮关键提交

### Permission attached API

```text
edd3750 feat(ui): add Lf attached permission context API
ca5073a feat(ui): implement reactive attached permission state
5324d53 feat(ui): register Lf attached permission context
e118c9a test(ui): cover attached permission and form validation state
8cf55f7 feat(demo): use Lf.permission attached context in results review
```

### Validation RuleTable / FormState

```text
2c0b2c2 feat(validation): add reusable validation rule table
0119415 feat(validation): implement rule lookup and provider fallback
f09721e feat(validation): add field and form validation state
5ff6e84 feat(validation): implement aggregate form feedback state
0e451fe feat(validation): register rule table and form state types
f201ae7 feat(demo): migrate sample register to form validation state
48654d8 fix(validation): harden rule table type conversions
```

### Toast / Dialog dual-stack overlay

```text
c430291 feat(overlay): define backend-neutral toast and dialog contract
bbac238 feat(overlay): add toast and dialog orchestration
18d1ace feat(overlay): implement toast lifecycle and dialog queue
2effb3b feat(overlay): add QML toast presenter
3129e29 feat(overlay): add QML modal dialog presenter
5a8e347 feat(overlay): wire toast and dialog presenters into overlay host
08b818f feat(widgetbridge): add QWidget overlay backend
59edc0b test(overlay): cover toast policy and dialog backend queue
a9c4b99 feat(demo): route navigation guard dialogs through overlay coordinator
9656dc6 feat(demo): surface registration feedback through toast coordinator
7d066fe feat(demo): exercise shared overlay backend in hybrid app
a584fc6 test(widgetbridge): cover shared widget toast backend
```

### Runtime hardening

```text
d154e3e feat(runtime): add navigation guard pending request policies
24bf4c7 feat(runtime): implement guarded navigation queue and drop semantics
e9de201 test(runtime): cover navigation guard pending request policies
24ea1c0 feat(runtime): add structured page provider errors
30a17a8 feat(runtime): provide default structured provider error adapter
a80f523 feat(ui): expose structured QML page provider errors
761c7e5 feat(widgetbridge): expose structured widget provider errors
450daf4 feat(runtime): add reusable page parameter update contract
d995314 feat(runtime): add explicit page cache weight
9c742bf feat(runtime): refresh QML singleton parameters on reuse
5ae0417 feat(runtime): refresh widget singleton parameters on reuse
98a300f feat(runtime): implement route-global singleton weighted LRU and structured provider diagnostics
f095988 test(runtime): cover singleton weighted LRU and structured errors
d45625d feat(runtime): expose weighted cache and singleton state to QML
20cdd1b feat(runtime): wire weighted cache state through quick runtime
```

上一批关键提交继续有效：

```text
013053b feat(runtime): add cross-provider page fallback
703cbc1 feat(ui): add QML-friendly navigation guard controller
7bc28f7 feat(widgetbridge): add QWidget QML focus bridge
1b835b0 feat(dataview): persist stable DataGrid column state
19129fd feat(dataview): add validated edit transactions
67508a8 feat(dataview): add typed delegates and selection navigation
8bc8926 perf(dataview): add opt-in 20k 50k benchmark matrix
9cdbdd2 feat(demo): add complete results review workflow
2248dd8 feat(validation): add reusable Core and QML validation pipeline
0600237 feat(demo): add sample register and rack workflows
```

## 完成标准

一个能力只有至少具备下面链路，才记入“已完成”：

```text
contract / API
+ actual implementation
+ failure / edge handling
+ unit/integration test where practical
+ Demo / real vertical integration for UI capability
+ architecture boundary review
```

当前新增能力按源码链路已达到上述标准；但本轮后半段功能提交尚未重新跑完 Qt 6.8.3 + MSVC2022 全量 Build/CTest，因此**不能把当前 HEAD 标记为 CI Green**。

---

# Core

已完成：

- Error / Result
- OperationHandle
- Permission contract
- Validation contract / `validateValue()`
- ICommand / CommandContext
- Event / EventDispatcher / RAII subscription
- Route contracts

Validation 支持：

- required
- minLength / maxLength
- regularExpression
- minimum / maximum
- precision
- optional empty passthrough

Core 继续保持 **QtCore only**。

---

# Runtime / Migration

## 基础能力

已完成：

- PageDescriptor / PageRegistry / PageProviderRegistry
- IPageProvider / IPageInstance
- PageLifecycle
- PageRouter push / replace / back / reset
- None / KeepAlive / Lru / Singleton
- Router-level cross-provider fallback
- QML create failure -> alternate provider -> WidgetPageProvider -> legacy QWidget
- provider-native final ErrorPage fallback
- NavigationGuard C++ contract

Runtime 继续只认 provider id，不感知 QWidget / QML / LfWidget / UnionLab。

## Route-global Singleton — 本轮完成

Singleton 不再只是“某个 history entry 不销毁”，而是由 Router 的 route-global cache 持有：

```text
route
  -> one shared page instance
  -> multiple history entries may reference it
  -> history pop does not destroy singleton
  -> router reset destroys global singleton cache
```

同时新增 `IPageInstance::updateParameters()` 可选契约。

内建 provider 已接入：

- QuickPageProvider：复用 singleton 时刷新 QML page parameters；
- WidgetPageProvider：复用 singleton 时刷新 QWidget dynamic properties / 可选页面回调。

## Weighted LRU — 本轮完成

LRU 同时支持：

```text
maxRetainedLruPages
maxRetainedLruWeight
```

页面 descriptor 可配置显式 `cacheWeight`，默认 weight 为 1。

当 inactive LRU cache 的：

```text
count 超限 OR weight 超限
```

按 lastTouched 从最旧页面开始淘汰。

QuickRuntime 已暴露：

```text
maxRetainedLruPages
maxRetainedLruWeight
retainedLruWeight
singletonCount
```

`registerPage()` 同时可以传 cache weight。

## Structured provider error — 本轮完成

`IPageProvider` 继续兼容旧 `lastError()`，同时新增结构化错误：

```text
PageProviderError
- code
- message
- details
```

及：

```text
lastErrorInfo()
```

QuickPageProvider / WidgetPageProvider 已填充结构化错误。

Router failure diagnostics 同时保留旧字段：

```text
primaryError
fallbackError
errorFallbackError
```

并增加：

```text
primaryFailure
fallbackFailure
errorFallbackFailure
```

方便 Diagnostics / Telemetry 后续直接消费。

---

# Navigation Guard

已有：

- NavigationGuardState
- NavigationGuardController
- dirty / busy / enabled / reason / confirmRequired
- confirm/cancel/retry-once
- Guard 本身不弹 Dialog

## 多请求策略 — 本轮完成

修复旧行为：已有确认请求时，后续导航不能静默覆盖当前 pending request。

新增显式策略：

```text
KeepFirst      // 默认，保留第一个 pending，后续丢弃
ReplaceLatest  // 用最新请求替换 pending
Queue          // 有界队列
```

队列请求在 origin route 已变化后会安全丢弃，避免确认旧页面后发生隐式二次跳转。

已有独立测试覆盖 pending / queue / drop policy。

---

# UI / Quick Runtime

已完成：

- QuickRuntime / QuickPageProvider
- QmlErrorCollector / PageLoadError
- OverlayCoordinator / Busy
- FocusNavigator
- NavigationGuardState / NavigationGuardController
- PermissionEvaluator
- ValidationEvaluator
- `Lf` attached context root
- ValidationRuleTable / ValidationFormState
- backend-neutral Toast/Dialog
- QML Toast/Dialog presenters

## Permission attached API — 本轮完成

新增：

```qml
Lf.permission
```

attached state 支持：

```text
evaluator
permissionId
context
state
allowed
visible
enabled
editable
active
```

设计规则：

- 未配置 permissionId 时不改变控件行为；
- 配置 permissionId 后如果 evaluator/provider 缺失则 fail-closed；
- provider/context 变化后 reactive 更新；
- Attached API 只做 UI experience mapping；
- Command / Domain 仍必须做权威权限检查；
- 框架不包含任何医疗业务 Permission ID。

Results Review 已迁移到 `Lf.permission`，验证 Reviewer / Operator / Guest 的可编辑、只读与禁止行为。

## Validation RuleTable / FormState — 本轮完成

新增：

```text
ValidationRuleTable
ValidationFormState
ValidationFieldState
```

RuleTable：

- QML local rule map；
- `IValidationRuleProvider` fallback；
- missing rule 明确失败；
- 继续复用 Core `validateValue()`，不重复造 validator。

FormState：

- fieldId / ruleId；
- value；
- touched；
- rule error；
- external business error；
- valid / hasError / message；
- form valid / errorCount / fieldCount；
- rule table 变化后自动 revalidate touched fields。

Sample Register 已迁移：

```text
barcode / sampleId / rack
-> ValidationFormState
-> duplicate business error as externalError
-> touched 后实时重校验
```

避免页面继续维护三套手写 error string 和重复 aggregate validate 逻辑。

---

# Overlay / Toast / Dialog

## Backend-neutral contract — 本轮完成

新增统一协议：

```text
IOverlayBackend
ToastRequest
DialogRequest
```

OverlayCoordinator 负责行为，不负责技术栈绘制：

### Toast

- severity
- position
- duration
- dedup key
- max visible
- explicit dismiss
- backend switch 时状态同步

### Dialog

- title/message
- button ids
- default / destructive button semantics
- serial queue
- result callback/signal
- cancel fallback
- one active dialog at a time

## QML backend

OverlayHost 已接入：

```text
ToastPresenter
DialogPresenter
BusyOverlay
```

Dialog 为 modal，统一处理 result / cancel。

## QWidget backend

WidgetBridge 新增：

```text
WidgetOverlayBackend
```

实现：

- QWidget Toast：轻量非模态 QFrame；
- Dialog：非阻塞 `QMessageBox::open()`；
- result 回传同一 OverlayCoordinator。

## Demo integration

Medical Demo：

- NavigationGuard confirmation / hard-block 不再自己维护独立 Dialog；
- 统一通过 OverlayCoordinator；
- Sample Register 成功反馈使用 success Toast；
- scanner reject 使用 warning Toast。

Hybrid Demo：

- 同一个 OverlayCoordinator 绑定 WidgetOverlayBackend；
- legacy QWidget 侧直接验证统一 Toast/Dialog 协议。

---

# WidgetBridge

已有：

- QuickSurface
- WidgetPageProvider
- WidgetFocusBridge / FocusTransferPolicy
- QWidget <-> QML Tab / Backtab
- focus remember / restore
- scanner isolation
- modal focus trap
- WidgetOverlayBackend
- singleton parameter refresh
- structured provider errors

仍待最后 hardening 阶段做真实 GUI stress：

- route-back focus restore
- dialog trap
- scanner focus isolation
- DPI / resize / z-order

---

# DataView

功能侧已具备：

- DataTableModel
- DataFilterProxyModel
- DataGrid virtualization foundation
- stable column id
- ColumnStateModel / ColumnConfigStore
- width / order / visibility persistence
- DataEditTransaction
- Check / Combo / Progress / Icon / Editor delegates
- batch selection
- context menu
- keyboard / cross-table navigation
- proxy/source row mapping
- `DataTableModel.findFirst`
- `DataFilterProxyModel.exactMatch`
- `DataFilterProxyModel.count`
- opt-in 20k / 50k benchmark matrix

当前功能目标基本完成，后续优先是性能 telemetry / GUI stress，而不是继续堆 delegate 数量。

---

# Platform / Scanner

已有：

- ScannerSession
- KeyboardWedgeScanner
- InputRouter
- cached page scanner isolation
- active-focus scanner enable
- temporary session configuration restore

业务页面继续通过 `pageActive + activeFocus` 双门控，避免后台缓存页消费全局扫码。

---

# Medical Demo

当前页面：

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

重点纵向链路：

### Sample Register

```text
Scanner / InputRouter
+ ValidationRuleTable / ValidationFormState
+ duplicate external business error
+ FocusNavigator
+ NavigationGuard dirty/busy
+ Overlay Busy / Toast
+ DataTableModel append
```

### Sample Rack

```text
Scanner
+ exact DataFilterProxyModel
+ DataGrid state persistence
+ Progress delegate
+ extended selection
+ proxy -> source mapping
+ batch/context/keyboard
```

### Results Review

```text
DataGrid typed editor
+ Lf.permission attached state
+ Validation
+ NavigationGuard
+ Overlay/Dialog
+ batch/context
+ persistence
```

Demo 数据均为 synthetic，不是医疗诊断软件。

---

# CI/CD 当前状态

CI/CD workflow 源码已存在：

```text
.github/workflows/build.yml
.github/workflows/release.yml
```

仓库公开后 GitHub runner 已恢复真实执行，之前已经实际进入：

```text
checkout
Qt install
Configure
MSVC Build
CTest
```

公开 runner 暴露并已修过：

- 错选 MinGW toolchain；
- shared DLL export / import library；
- RouteResult / IPageProvider / IPermissionProvider ABI；
- Qt 6.8 QML registrar include / module source；
- moc complete type；
- Qt 6.8 QTest API；
- Windows test DLL staging。

**最后一次已观察到的基础链状态**：

```text
MSVC full Build: PASS
CTest: 6/7 PASS
tst_dataview: process exit code 1，尚未拿到有效 QtTest assertion output
```

之后已加入大量 Permission / Validation / Overlay / Runtime 功能提交，尚未重新做当前 HEAD 的全量 MSVC Build/CTest。

根据当前开发指令：

```text
先完成主要功能开发
-> 再统一修 CI/CD
-> 再做 package/release/artifact hardening
```

因此当前必须准确表述：

```text
CI/CD workflow configured: YES
Runner can execute real steps: YES
Previous baseline full Build: PASS
Previous baseline CTest: 6/7
Current HEAD full Build/CTest verified: NO
Demo artifact verified against current HEAD: NO
GitHub Release verified: NO
CI Green: NO CLAIM
```

不要在下一窗口一开场就围绕 CI 循环修复，除非用户改变“功能优先”的指令。

---

# Git / stale branch 状态

仓库当前仍有旧分支：

```text
feat/runtime-foundation
```

已审查比较：

```text
relative to main:
ahead_by: 9
behind_by: 145
status: diverged
```

该分支是明显陈旧的早期 Runtime/Foundation 分叉；当前 main 已经演进出更多且更新的 Core/Runtime/UI 能力。

**不要把该分支整体 merge 回 main**，否则有高风险把旧实现带回主线。

如后续清理旧分支：

1. 只审查具体单个 commit；
2. 仅当确认 main 缺失且仍有价值时做等价迁移/cherry-pick；
3. 不允许用整分支 merge 覆盖当前 main 结构。

main-only 策略继续保持。

---

# 当前已知债务 / 下一步

按“功能完成优先”排序：

1. **WindowRouter**：建立多窗口/窗口生命周期与 route 的通用契约，保持 Runtime 技术栈无关。
2. **轻量 NavigationView**：model-driven navigation shell，不做巨大 FluentUI clone；与 QuickRuntime / Guard / Overlay 集成。
3. **Charts / Visualization MVP**：先抽象 data/series/view contract，再选择 Qt Graphs 接入，不让业务模型进入 Charts core。
4. **Diagnostics HUD / Telemetry**：消费现有 route/provider/cache/error/fallback 数据，加入 runtime 可视诊断。
5. **Hot Reload tooling**：仅开发态，围绕 single QQmlEngine / QuickSurface，不破坏 production lifecycle。
6. **Qt 6.12 adapter / feature gate**：保持 6.8.3 主基线，隔离 6.12-only API。
7. 完成功能后统一做 GUI/performance stress：DataGrid FPS/memory、WidgetFocusBridge、QuickSurface DPI/resize/z-order。
8. 最后回头解决 `tst_dataview`、全量 CI、deploy/package/artifact/release。

仍然不要优先开发巨大 NavigationView，也不要为“控件数量”偏离混合迁移主目标。

---

# 架构边界继续保持

```text
LfQuickCore      -> QtCore only
LfQuickRuntime   -> Core，不允许 QWidget/QQuickItem
LfQuickUi        -> Core + Runtime + Qt Quick
LfQuickDataView  -> Ui + Qt Model/View/Quick
LfQuickPlatform  -> platform/input abstraction
WidgetBridge     -> optional Qt Widgets / QuickWidgets
Integration.*    -> legacy/product-specific adapters
```

Core 禁止进入：

- EventFunc Adapter
- LfWidget.dll Adapter
- WindowManager Adapter
- QXlsx
- LimeReport
- HookProc
- ScanerHook
- Patient / Sample / Reagent domain model
- 医疗业务 Permission ID
- QC / Calibration 业务算法

外部参考继续遵守 `docs/REFERENCE_POLICY.md`：FluentUI MIT 可研究并按许可证记录；FluentUI2 / Pro 只做 clean-room public API / qmltypes / Demo / behavior 参考，不逆向 DLL，不还原私有实现。
