# LfQuick 新窗口开发交接

> 交接日期：2026-10-16  
> 仓库：`https://github.com/cquptzf-glitch/LfQuick`  
> Git 策略：**main-only**，不要创建普通 feature branch。

## 1. 开场直接做什么

不要重新做需求分析。直接：

1. 读取 main 最新 HEAD；
2. 读取本文件、`docs/IMPLEMENTATION_STATUS.md`、`docs/CI_CD.md`、详细开发计划、V2.1 架构审查和 `REFERENCE_POLICY.md`；
3. 检查最新 GitHub Actions；
4. 如果出现真实 Configure / Build / Test steps，第一优先级修 Qt 6.8.3 + MSVC2022 编译/测试/部署问题；
5. 如果仍然 `steps = null`，记录为 runner/infrastructure issue，继续下面开发顺序。

## 2. 当前功能基线

最新功能 HEAD：

```text
073b6de442fb5afdb90247045092ba280480d294
fix(demo): isolate scanner consumers and expose proxy count
```

本批提交：

```text
a138413 ci: build package and release Windows demos
2248dd8 feat(validation): add reusable Core and QML validation pipeline
0600237 feat(demo): add sample register and rack workflows
00a3acd fix(ci): harden release demo asset paths
073b6de fix(demo): isolate scanner consumers and expose proxy count
```

上一批仍有效：

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

## 3. 强制规则

### Git

- 只在 `main` 开发。
- 不创建普通 feature branch。
- 每批保持语义清晰小提交。

### 完成标准

不能因为“有 API / 有 QML 文件”就记为完成。至少要求：

```text
contract/API
+ actual implementation
+ edge/failure handling
+ unit/integration test where practical
+ demo/vertical integration for UI
+ architecture boundary review
```

Runner 没有真实 steps 时，只能写“源码链路完成/待实编”，不能写 CI Green。

### 架构边界

```text
LfQuickCore      -> QtCore only
LfQuickRuntime   -> Core，不允许 QWidget/QQuickItem
LfQuickUi        -> Core + Runtime + Qt Quick
LfQuickDataView  -> Ui + Qt Model/View/Quick
LfQuickPlatform  -> platform/input abstraction
WidgetBridge     -> optional Qt Widgets / QuickWidgets
Integration.*    -> legacy/product-specific adapters
```

不要把 EventFunc / LfWidget.dll / WindowManager / QXlsx / LimeReport / HookProc / ScanerHook / Patient-Sample-Reagent domain / medical Permission IDs / QC-Calibration 算法写进 Core/DataGrid。

## 4. 当前关键能力

### Runtime / Migration

- PageRouter / Lifecycle / History
- None / KeepAlive / Lru
- INavigationGuard
- Router-level cross-provider fallback
- QML create failure -> alternate provider -> WidgetPageProvider -> legacy QWidget
- provider diagnostics
- final provider-native ErrorPage

### Quick/UI

- QuickRuntime / QuickPageProvider
- QmlErrorCollector / PageLoadError
- Overlay / Busy
- FocusNavigator
- NavigationGuardState / NavigationGuardController
- PermissionEvaluator
- ValidationEvaluator

### Validation

Core 已有真正执行器：

```cpp
validateValue(value, ValidationRule)
```

支持 required / length / regex / min/max / precision。

QML：

```text
ValidationEvaluator.validate(value, ruleMap)
ValidationEvaluator.isValid(value, ruleMap)
```

下一步不是重复实现 validator，而是考虑 RuleTable / reusable field feedback。

### WidgetBridge

- QuickSurface
- WidgetPageProvider
- WidgetFocusBridge / FocusTransferPolicy
- QWidget <-> QML Tab/Backtab
- focus remember/restore
- scanner isolation
- modal focus trap

### DataView

已具备：

- stable column id
- ColumnStateModel / ColumnConfigStore
- width/order/visibility persistence
- DataEditTransaction
- typed Text/Check/Combo/Progress/Icon delegates
- DataGridSelectionController
- batch/context actions
- keyboard + cross-grid navigation
- proxy/source mapping
- `DataTableModel.findFirst`
- `DataFilterProxyModel.exactMatch`
- `DataFilterProxyModel.count`
- opt-in 20k/50k benchmark

## 5. Medical Demo 当前页面

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

### Sample Register — 已完成源码纵向链路

验证：

```text
Scanner / InputRouter
+ active-focus scanner isolation
+ ValidationEvaluator / Core Validation
+ duplicate lookup
+ FocusNavigator
+ dirty NavigationGuard
+ submit busy hard-block
+ Overlay Busy
+ DataTableModel append
```

重要边界：

- scanner signal target 必须是 `scanner.session`；
- cached inactive page 必须通过 `pageActive` 忽略 scanner signals；
- scanner.enabled 只在目标 field activeFocus 时打开；
- dirty page 离开需要 confirm；
- submit 进行中 hard block；
- submit 成功 reset form。

### Sample Rack — 已完成源码纵向链路

验证：

```text
Scanner
+ exact rack filtering
+ DataGrid stable columns/persistence
+ Progress delegate
+ extended selection
+ proxy->source mapping
+ batch actions
+ context menu
+ keyboard focus
```

重要边界：

- Rack `R1` 必须 exactMatch，不能匹配 `R10`；
- Rack 页面把 ScannerSession minimumLength 临时调为 2，离开恢复原值；
- batch mutation 前先 snapshot source rows。

### Results Review — 已完成源码纵向链路

仍验证：DataGrid typed editor + Permission + Validation + Guard + Overlay + batch/context + persistence。

## 6. CI/CD

### CI

`.github/workflows/build.yml`

触发：main push / PR / manual。

流程：

```text
checkout
Qt 6.8.3 MSVC2022
architecture check
Configure
Build
CTest
cmake --install
Qt QML deploy
package layout validation
upload artifact
```

成功 artifact：

```text
LfQuick-Demos-Windows-x64-<sha>
```

### Release

`.github/workflows/release.yml`

- `v*` tag：完整 Build/Test/Deploy 后 zip + GitHub Release；
- manual：zip artifact，不发布正式 Release。

zip：

```text
LfQuick-Demos-Windows-x64-<tag>.zip
```

Medical / Hybrid Demo 使用 `qt_generate_deploy_qml_app_script()`，不要退回手写 Qt DLL 列表。

详见 `docs/CI_CD.md`。

## 7. CI 当前真实状态

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

依旧属于 runner/infrastructure issue。

必须继续表述：

```text
CI/CD workflow configured: YES
Real Qt 6.8.3 + MSVC2022 compile: NOT EXECUTED
CTest: NOT EXECUTED
Demo artifact verified: NO
GitHub Release verified: NO
CI Green: NO CLAIM
Source compile failure: NO COMPILER EVIDENCE
```

Runner 一恢复，第一批重点检查：

- `qt_generate_deploy_qml_app_script()` + 两个 Demo 共用 install prefix；
- QML module/plugin deployment；
- ValidationEvaluator moc/QML registration；
- DataFilterProxyModel exactMatch/count signal；
- `DataTableModel.findFirst` QML enum parameter；
- SampleRegister SpinBox `valueModified` / lifecycle handlers；
- SampleRack Menu / Progress delegate / scanner minimum-length restore；
- prior typed DataGrid / WidgetFocusBridge / Router fallback。

## 8. 下一开发批次

### Step 0 — Build Gate

每次先查 Actions。如果出现 actual steps，立即暂停新大功能并修 Configure / Build / CTest / package。

### Step 1 — Permission attached API

基于已完成的 `PermissionEvaluator`，继续设计：

```qml
Lf.permission
```

要求：

- 只做 UI experience mapping；
- Command/Domain 层仍做权威权限校验；
- 不把医疗权限 ID 写入框架。

### Step 2 — Validation RuleTable / Form feedback

已有 `validateValue` + `ValidationEvaluator`，下一步做：

- rule table provider；
- field state / error message binding；
- form aggregate validity；
- 不重复造 validator。

### Step 3 — Toast/Dialog dual-stack

建立 QWidget/QML 一致 backend / behavior，不让各页面自己管理一套提示机制。

### Step 4 — GUI performance / stress

- DataGrid FPS / scroll / first population / memory telemetry；
- WidgetFocusBridge route-back/modal/scanner stress；
- QuickSurface DPI/resize/z-order stress。

### Step 5 — Runtime debts

- route-global Singleton；
- LRU memory weight；
- NavigationGuard multi-request queue/drop policy；
- provider structured error contract refinement。

### Step 6 — 后续

WindowRouter / NavigationView（仍不优先巨大版本）/ Charts / Diagnostics HUD / Hot Reload / Qt 6.12 adapter。

## 9. External Reference

继续遵守 `docs/REFERENCE_POLICY.md`：

- FluentUI MIT 公开源码允许研究；复制/实质衍生保留 MIT 记录；
- FluentUI2/Pro 仅 public API/qmltypes/demo/behavior clean-room；
- 不逆向 DLL；
- 不还原私有实现；
- LfQuick 架构边界优先。
