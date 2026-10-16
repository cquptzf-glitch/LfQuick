# LfQuick 当前实现状态

> 更新时间：2026-10-16  
> 开发策略：**main-only**。普通功能不创建 feature branch；后续开发直接提交 main，并保持语义清晰的小提交。

## 当前基线

- Qt：6.8.3
- C++：20
- CMake：3.24+
- 当前主线：`main`
- 本批功能 HEAD：`073b6de442fb5afdb90247045092ba280480d294`
- CI/CD 首批提交：`a138413ec5f1ea6ddac515f163905609cc708f22`

## 本批新增提交

```text
a138413 ci: build package and release Windows demos
2248dd8 feat(validation): add reusable Core and QML validation pipeline
0600237 feat(demo): add sample register and rack workflows
00a3acd fix(ci): harden release demo asset paths
073b6de fix(demo): isolate scanner consumers and expose proxy count
```

上一批关键提交仍然有效：

```text
67508a8 feat(dataview): add typed delegates and selection navigation
8bc8926 perf(dataview): add opt-in 20k 50k benchmark matrix
dd83729 feat(ui): add QML permission evaluator
2420b4f feat(dataview): expose proxy row mapping
9cdbdd2 feat(demo): add complete results review workflow
1b835b0 feat(dataview): persist stable DataGrid column state
19129fd feat(dataview): add validated edit transactions
013053b feat(runtime): add cross-provider page fallback
703cbc1 feat(ui): add QML-friendly navigation guard controller
7bc28f7 feat(widgetbridge): add QWidget QML focus bridge
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

CI runner 当前未实际执行 Windows 编译，因此“已完成”指源码功能链路与测试代码完整，不等于已获得 Qt 6.8.3 + MSVC2022 实编通过证据。

## Core

已完成：

- Error / Result
- OperationHandle
- Permission contract
- Validation contract
- ICommand / CommandContext
- Event / EventDispatcher / RAII subscription
- Route contracts

### Validation 执行管线 — 本批完成

Core 新增：

```cpp
ValidationResult validateValue(const QVariant &value,
                               const ValidationRule &rule);
```

支持：

- required
- minLength / maxLength
- regularExpression
- minimum / maximum
- precision
- optional empty value passthrough

错误规则（例如非法 regex、非法 numeric boundary）会返回明确失败结果，而不是静默接受。

`tst_core` 已覆盖 required / length / regex / numeric range / precision / optional empty。

Core 继续保持 `QtCore only`。

## Runtime / Migration

已完成：

- PageDescriptor / PageRegistry / PageProviderRegistry
- IPageProvider / IPageInstance
- PageLifecycle
- PageRouter push / replace / back / reset
- NavigationGuard C++ contract
- None / KeepAlive / Lru
- Router 级 cross-provider fallback
- QML page failure -> alternate provider -> WidgetPageProvider -> legacy QWidget
- provider diagnostics
- final provider-native ErrorPage fallback

Runtime 继续只认 provider id，不感知 QWidget / QML / LfWidget / UnionLab。

## UI / Quick Runtime

已完成：

- QuickRuntime / QuickPageProvider
- QmlErrorCollector / PageLoadError
- OverlayCoordinator / Busy
- FocusNavigator
- NavigationGuardState / NavigationGuardController
- confirm/cancel/retry-once navigation workflow
- PermissionEvaluator
- **ValidationEvaluator**

### ValidationEvaluator — 本批完成

QML adapter：

```text
QVariantMap rule
      ↓
ValidationEvaluator
      ↓
Core validateValue()
```

QML API：

```text
validate(value, ruleMap)
isValid(value, ruleMap)
```

返回：

```text
valid
message
ruleId
```

`tst_ui` 已验证 QML rule map -> Core validation 映射。

## WidgetBridge

已有：

- QuickSurface
- WidgetPageProvider
- WidgetFocusBridge / FocusTransferPolicy
- QWidget <-> QML Tab/Backtab
- focus remember/restore
- scanner isolation
- modal focus trap

真实 Windows GUI stress 仍待 runner 恢复后验证。

## DataView

### 已完成基础

- DataTableModel
- DataFilterProxyModel
- DataGrid virtualization foundation
- stable column id
- ColumnStateModel
- ColumnConfigStore
- width/order/visibility persistence
- DataEditTransaction
- typed delegates
- selection/context/keyboard
- proxy/source row mapping
- opt-in 20k / 50k model benchmark

### 本批新增通用能力

`DataTableModel`：

```text
findFirst(key, value, caseSensitivity)
```

用于通用模型字段查重/定位，不包含任何 Sample/Patient/Reagent 专用逻辑。

`DataFilterProxyModel`：

```text
exactMatch
count
```

`exactMatch` 用于 Rack 等需要精确 key 匹配的场景；`count` 作为 QML-safe row count property。

现有：

```text
mapRowToSource(proxyRow)
mapRowFromSource(sourceRow)
```

仍用于批量动作前 snapshot source rows，避免 dynamic filtering / sorting 造成误写。

`tst_dataview` 已增加：

- findFirst case-sensitive / insensitive lookup
- unknown key -> -1
- exact filter match
- partial value rejected in exactMatch mode
- prior row mapping / selection / edit transaction / column persistence tests 保留

## Platform / Scanner

已有：

- ScannerSession
- KeyboardWedgeScanner
- InputRouter

本批修正 Demo scanner consumer：

- `scanAccepted / scanRejected` 实际来自 `scanner.session`；
- KeepAlive/LRU 页面必须使用 `pageActive` 门控，避免后台缓存页消费全局扫码；
- scanner 只在目标输入框 activeFocus 时 enabled；
- Sample Rack 临时将 minimumLength 调到 2，离开页面恢复原值，避免影响 Sample Register/其它扫码场景。

## Medical Demo

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

### Sample Register — 本批完成纵向链路

实际组合：

```text
ScannerSession / KeyboardWedgeScanner
+ InputRouter
+ ValidationEvaluator -> Core validateValue
+ DataTableModel.findFirst duplicate detection
+ FocusNavigator
+ NavigationGuard dirty confirm
+ NavigationGuard busy hard-block
+ OverlayCoordinator
+ DataTableModel appendRow
```

场景：

1. 条码框有焦点且 Scanner armed 时才启用 scanner；
2. scan signal 只由当前 active page 消费；
3. barcode / sampleId / rack 走统一 ValidationEvaluator；
4. barcode / sampleId 做 case-insensitive duplicate check；
5. 表单修改后 dirty guard；
6. 离开页面触发 application-level confirm dialog；
7. Submit 期间 guard busy hard-block；
8. Submit 期间 Overlay Busy；
9. synthetic submit 成功后 appendRow + reset form；
10. F2 继续验证 FocusNavigator。

### Sample Rack — 本批完成纵向链路

实际组合：

```text
Scanner
+ exact DataFilterProxyModel
+ DataGrid stable columns / persistence
+ typed Progress delegate
+ extended selection
+ proxy -> source row mapping
+ batch action
+ context menu
+ keyboard navigation
```

场景：

- Rack ID 精确过滤，`R1` 不误匹配 `R10`；
- scanner 可切换当前 rack；
- Test Count 使用 progress delegate；
- Queue selected / Mark STAT；
- 批量操作先 snapshot source rows；
- right-click context action；
- F6 进入 DataGrid，继续验证 keyboard navigation；
- Rack page 恢复 scanner minimumLength，避免 session 配置泄漏到其它页面。

### Results Review — 上一批已完成

仍验证：

```text
DataGrid typed delegates
+ batch selection
+ context actions
+ proxy row mapping
+ DataEditTransaction
+ Validation
+ PermissionEvaluator
+ NavigationGuard
+ Busy Overlay
+ column persistence
```

Demo 数据均为 synthetic，不是医疗诊断软件。

## CI/CD — 本批新增

### CI workflow

`.github/workflows/build.yml` 当前会在：

- push main
- pull_request
- workflow_dispatch

执行：

```text
checkout
-> Qt 6.8.3 win64_msvc2022_64
-> architecture check
-> CMake Configure
-> Build
-> CTest
-> cmake --install
-> Qt QML deploy
-> package validation
-> upload artifact
```

目标 artifact：

```text
LfQuick-Demos-Windows-x64-<sha>
```

### Release workflow

`.github/workflows/release.yml`：

- `v*` tag -> Build/Test/Deploy -> zip -> GitHub Release
- workflow_dispatch -> Build/Test/Deploy -> zip artifact，不自动发布正式 Release

正式 zip：

```text
LfQuick-Demos-Windows-x64-<tag>.zip
```

### Demo deployment

Medical Demo / Hybrid Demo 均增加：

```cmake
install(TARGETS ...)
qt_generate_deploy_qml_app_script(...)
install(SCRIPT ...)
```

使用 Qt 官方 deployment API，不维护手写 Qt DLL/plugin 清单。

详见 `docs/CI_CD.md`。

## GitHub Actions 当前状态

最新 run：

```text
run: 38898266602
head: 073b6de442fb5afdb90247045092ba280480d294
workflow: ci
job: windows-qt683
status: completed
conclusion: failure
steps: null
logs_url: null
```

仍属于 GitHub Actions runner / infrastructure abnormal：job 创建后约数秒结束，没有 checkout / Qt install / Configure / Build / Test。

因此当前准确结论：

```text
CI/CD workflow source: implemented
Demo deploy/package source: implemented
Sample Register/Rack source vertical slices: implemented
Real Qt 6.8.3 + MSVC2022 compile: NOT EXECUTED
CTest: NOT EXECUTED
Demo artifact: NOT PRODUCED/VERIFIED
GitHub Release: NOT PRODUCED/VERIFIED
CI Green: NO CLAIM
Source compile failure: NO COMPILER EVIDENCE
```

Runner 一旦恢复并出现真实 steps，第一优先级修 Configure / Build / CTest / deploy/package 问题。

## 当前已知债务 / 下一步

1. **Build gate**：Windows runner 恢复后立即做真实编译、测试和 Demo packaging 验证。
2. Permission attached API `Lf.permission`。
3. Validation RuleTable / reusable form feedback；当前执行器已完成，但规则表和字段状态编排仍可继续抽象。
4. Toast/Dialog Widget/QML dual-stack backend。
5. DataGrid GUI FPS / scroll / first-population / memory telemetry。
6. WidgetFocusBridge route-back / modal / scanner GUI stress。
7. QuickSurface DPI / resize / z-order stress。
8. PageCache Singleton route-global semantics、LRU memory weight。
9. NavigationGuardController multi-request queue/drop policy。
10. 再考虑 WindowRouter / NavigationView / Charts / Diagnostics HUD / Hot Reload / Qt 6.12 adapter。

## 架构边界继续保持

```text
LfQuickCore      -> QtCore only
LfQuickRuntime   -> Core，不允许 QWidget/QQuickItem
LfQuickUi        -> Core + Runtime + Qt Quick
LfQuickDataView  -> Ui + Qt Model/View/Quick
LfQuickPlatform  -> platform/input abstraction
WidgetBridge     -> optional Qt Widgets / QuickWidgets
Integration.*    -> legacy/product-specific adapters
```

Core 禁止 EventFunc / LfWidget.dll / WindowManager / QXlsx / LimeReport / HookProc / ScanerHook / Patient-Sample-Reagent domain / medical permission IDs / QC-Calibration algorithms。

外部参考继续遵守 `docs/REFERENCE_POLICY.md`。
