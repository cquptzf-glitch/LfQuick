# LfQuick 新窗口开发交接

> 交接日期：2026-10-16  
> 仓库：`https://github.com/cquptzf-glitch/LfQuick`  
> Git 策略：**main-only**，不要创建普通 feature branch。

## 1. 开场直接做什么

不要重新做需求分析。直接：

1. 读取 `main` 最新 HEAD；
2. 读取本文件、`docs/IMPLEMENTATION_STATUS.md`、`docs/LfQuick_详细开发计划.md`、`docs/LfQuick_V2.1_架构审查与优化方案.md`、`docs/REFERENCE_POLICY.md`；
3. 检查最新 GitHub Actions；
4. 如果出现真实 Configure / Build / Test steps，先修 Qt 6.8.3 + MSVC2022；
5. 如果仍然 `steps = null`，明确记录为 runner/infrastructure issue，然后直接继续下一批开发。

## 2. 当前 main 功能基线

本轮功能 HEAD：

```text
7bc28f77616c897a9d7379df32d7d1bbacaae5d6
feat(widgetbridge): add QWidget QML focus bridge
```

本轮新增提交：

```text
013053b feat(runtime): add cross-provider page fallback
ab06e2c fix(runtime): keep provider fallback ABI incomplete-type safe
703cbc1 feat(ui): add QML-friendly navigation guard controller
7bc28f7 feat(widgetbridge): add QWidget QML focus bridge
```

上一批关键提交仍然有效：

```text
c0136fd feat(runtime): add navigation guards and LRU page eviction
d3615ed feat(ui): add QML error collection and fallback page
d45a4d5 feat(widgetbridge): add shared-router hybrid migration path
bfebd2c fix: harden QML fallback and optional widget lifecycle hooks
1c8f639 feat(ui): expose provider and navigation guard extension points
```

## 3. 旧分支审查结果

仓库当前普通旧分支只有：

```text
feat/runtime-foundation
```

相对 main：

```text
ahead_by: 9
behind_by: 49
status: diverged
```

其内容是早期 Runtime / QuickSurface / reference policy 等实现，main 已覆盖并继续扩展。本轮没有合并它，避免旧实现覆盖当前结构。后续继续只在 main 开发。

## 4. 当前关键能力

### Runtime

已具备：

- PageRouter / Lifecycle / History
- NavigationGuard C++ contract
- None / KeepAlive / Lru cache
- QML/provider-agnostic PageDescriptor
- Router 级 cross-provider fallback
- fallback diagnostics signals

新增 descriptor：

```cpp
struct PageFallbackDescriptor {
    QString provider;
    QVariantMap source;
};

struct PageDescriptor {
    ...
    FallbackPolicy fallbackPolicy;
    std::optional<PageFallbackDescriptor> fallback;
};
```

Router 完全不写死 QWidget/QML。

当前 fallback 语义：

```text
primary create failure
  ↓
LegacyBeforeActivate
or LegacyIfStateless + metadata["stateless"] == true
  ↓
cross-provider create
  ↓ failure
primary provider createErrorFallback()
  ↓ failure
RouteResult failure + provider diagnostics
```

### Quick/UI

已有：

- QuickRuntime / QuickPageProvider
- QmlErrorCollector
- PageLoadError
- Overlay / Busy
- FocusNavigator
- provider / descriptor extension points

本轮新增：

```text
NavigationGuardState
NavigationGuardController
```

properties：

```text
dirty
busy
enabled
reason
confirmRequired
```

流程：

```text
route request
  ↓ guard block
controller stores exact NavigationContext
  ↓
UI confirmationRequested / hardBlocked
  ├─ cancelPending -> current route unchanged
  └─ confirmPending -> retry original action exactly once
                       bypass only this controller once
                       other guards still execute
```

Guard 不弹 Dialog；Hybrid Demo 的 QML UI 负责 Dialog。

### WidgetBridge

已有：

- QuickSurface
- WidgetPageProvider
- lifecycle adapter

本轮新增：

```text
WidgetFocusBridge
FocusTransferPolicy
```

MVP 支持：

- QWidget -> QML Tab
- QML -> QWidget Tab
- Backtab reverse transfer
- rememberFocus / restoreFocus
- scannerIsolation
- modalFocusTrap

Focus Bridge 只存在于 WidgetBridge，不进入 Runtime。

## 5. Hybrid Demo 当前验证链路

```text
QApplication / QWidget Shell
→ QStackedWidget
→ QuickSurface + Legacy QWidget
→ QuickPageProvider + WidgetPageProvider
→ same QuickRuntime / PageRouter / History
```

Demo 新增：

1. Broken QML → Router cross-provider fallback → Legacy results QWidget；
2. fallback signal/status 展示；
3. dirty guard + confirm/cancel Dialog；
4. busy guard + hard block；
5. `QLineEdit -> QML TextField -> QLineEdit` Tab/Backtab；
6. scanner focus isolation toggle；
7. route back focus remember/restore 基础。

## 6. 测试新增

### Runtime tests

- LegacyBeforeActivate fallback success
- LegacyIfStateless explicit safety gate
- cross-provider failure -> ErrorPage
- full fallback chain failure diagnostics

### UI tests

- guard confirm retries once
- cancel remains current route
- hard block cannot be confirmed
- original parameters preserved

### WidgetBridge tests

- provider lastError for unknown factory
- widget -> quick boundary transfer MVP
- scanner isolation suppresses transfer

这些目前只有源码/静态实现，**还没有真实 Qt 6.8.3 Windows CI 编译结果**。

## 7. CI 状态

本轮最新 run：

```text
run: 38892175280
head: 7bc28f77616c897a9d7379df32d7d1bbacaae5d6
job: windows-qt683
status: completed
conclusion: failure
steps: null
logs_url: null
```

仍然是同一基础设施异常：job 创建后约 3 秒结束，完全没有 checkout / Install Qt / Configure / Build / Test。

必须继续使用下面表述：

```text
GitHub Actions runner/infrastructure abnormal
Real Qt 6.8.3 compile has NOT executed
Do not claim CI green
Do not claim source compile failure without compiler output
```

Runner 恢复后重点检查：

- optional PageFallbackDescriptor + MSVC public header
- IPageProvider fallback virtual ABI
- NavigationGuardController moc / Q_PROPERTY pointer types
- QVariant QObject* initial property injection
- WidgetFocusBridge QQuickWidget focus event behavior
- GUI unit test stability
- QML Dialog/TextField/resource registration

## 8. 下一开发批次

严格按以下顺序：

### Step 0 — Build gate

先查 Actions。只要出现真实 steps，就暂停大功能，修完 Configure / Build / CTest。

### Step 1 — DataGrid 第二阶段（当前最高功能优先级）

优先做一个完整纵向切片：

```text
stable column id
  +
ColumnStateModel / ColumnConfigStore
  +
width/order/visibility persistence
  +
schemaVersion/tableId/modelSchema
```

Persistence contract 不要直接写死 QSettings；可以先提供 interface + QSettings adapter，方便产品替换存储。

然后做：

```text
edit transaction
Begin -> Temporary -> Validate -> Commit / Rollback
```

再加入：

- CheckDelegate
- ComboDelegate
- ProgressDelegate
- IconDelegate
- EditorDelegate
- batch selection
- context menu
- keyboard / cross-table navigation
- 20k / 50k row benchmark

DataGrid 不能出现 Sample / Patient / Reagent 业务字段。

### Step 2 — Medical Demo 真实页面

优先：

```text
Sample Register
Sample Rack
Results Review
```

每页至少验证 2~4 个框架系统。

Results Review 推荐：

```text
DataGrid
+ NavigationGuard
+ Permission
+ Validation
+ Overlay
```

### Step 3 — 后续

- Permission attached API
- Validation RuleTable
- Toast/Dialog dual-stack consistency
- QuickSurface DPI/resize/z-order stress
- Focus Bridge GUI stress / restore policy refinement
- WindowRouter
- NavigationView（仍不优先做巨大版本）
- Charts abstraction
- Diagnostics HUD
- Hot Reload
- Qt 6.12 adapter

## 9. 已知债务

- Singleton 仍非 route-global singleton。
- LRU 无 memory cost/weight。
- LegacyIfStateless 暂用 metadata["stateless"]，后续可 typed field。
- NavigationGuardController 当前只保存一个 pending request；快速连续请求需要后续明确覆盖/丢弃策略。
- Focus Bridge 是 boundary MVP，不重写完整 Qt/QML focus chain。
- route back focus restore 需要真实 GUI stress。
- CI 尚未实编。

## 10. 不要做

- 不要先开发巨大 NavigationView。
- 不要为了全 QML 强行移除 QWidget。
- 不要把 QWidget/QQuickItem 写进 Runtime。
- 不要把业务 Permission ID / domain model 放进 Core/DataGrid。
- 不要让 Scanner 承担业务规则。
- 不要在 CI 无 steps 时宣称构建失败或 CI Green。
- 不要创建新的普通 feature branch。

## 11. 架构边界

```text
LfQuickCore      -> QtCore only
LfQuickRuntime   -> Core，不允许 QWidget/QQuickItem
LfQuickUi        -> Core + Runtime + Qt Quick
LfQuickDataView  -> Ui + Qt Model/View/Quick
LfQuickPlatform  -> platform/input abstraction
WidgetBridge     -> optional Qt Widgets / QuickWidgets
Integration.*    -> legacy/product-specific adapters
```

Core 禁止：EventFunc Adapter、LfWidget.dll Adapter、WindowManager Adapter、QXlsx、LimeReport、HookProc、ScanerHook、Patient/Sample/Reagent domain、医疗业务 Permission ID、QC/Calibration 算法。

## 12. External Reference

继续遵守 `docs/REFERENCE_POLICY.md`：

- FluentUI MIT 公开源码允许研究；复制/实质衍生必须保留 MIT 记录；
- FluentUI2/Pro 只做 public API/qmltypes/demo/behavior clean-room 参考；
- 不逆向 DLL；
- 不还原私有 Pro 实现；
- LfQuick 架构边界优先于模仿 FluentUI。
