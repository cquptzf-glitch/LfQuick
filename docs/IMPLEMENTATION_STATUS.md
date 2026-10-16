# LfQuick 当前实现状态

> 更新时间：2026-10-16  
> 开发策略：**main-only**。不为普通功能创建新分支；已有旧分支只做审查，不把被 main supersede 的早期实现重新合入。

## 当前基线

- Qt：6.8.3
- C++：20
- CMake：3.24+
- 目标升级：Qt 6.12 LTS
- 当前主线：`main`
- 本轮功能 HEAD：`7bc28f77616c897a9d7379df32d7d1bbacaae5d6`

## 本轮新增提交

```text
013053b feat(runtime): add cross-provider page fallback
ab06e2c fix(runtime): keep provider fallback ABI incomplete-type safe
703cbc1 feat(ui): add QML-friendly navigation guard controller
7bc28f7 feat(widgetbridge): add QWidget QML focus bridge
```

## Repository / Git

- main-only workflow 持续执行。
- 旧分支 `feat/runtime-foundation` 已审查：相对 main `ahead 9 / behind 49`，内容属于早期 Runtime/QuickSurface 基础实现，main 已覆盖并继续扩展；本轮不合并，避免把旧实现重新引入。
- 每批功能保持语义清晰小提交。

## Core

已完成：

- Error / Result
- OperationHandle
- Permission / Validation contracts
- ICommand / CommandContext
- Event / EventDispatcher / RAII subscription
- RouteRequest / RouteResult

边界保持：`LfQuickCore -> QtCore only`。

## Runtime

已完成：

- PageDescriptor / PageRegistry / PageProviderRegistry
- IPageProvider / IPageInstance
- PageLifecycle
- PageRouter push / replace / back / reset
- INavigationGuard / NavigationContext / NavigationDecision
- None / KeepAlive / Lru cache behavior
- LRU eviction signal
- hard reset bypass guard

### Router 级 cross-provider fallback

新增通用 contract：

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

Runtime 不写死 QWidget / QML / UnionLab / LfWidget，只根据 provider id 工作。

当前 fallback 顺序：

```text
primary provider create
        │ failure
        ▼
LegacyBeforeActivate
or LegacyIfStateless + metadata["stateless"] == true
        │
        ▼
cross-provider fallback descriptor
        │ failure / unsafe
        ▼
primary provider native error fallback
        │
        ▼
final RouteResult failure with diagnostics
```

新增 diagnostics：

- primaryProvider
- primaryError
- fallbackProvider
- fallbackError
- errorFallbackError
- activeProvider / fallbackStrategy（成功时通过 signal 暴露）

新增 signals：

- `pageFallbackAttempted`
- `pageFallbackActivated`
- `pageFallbackFailed`

IPageProvider 增加通用 `lastError()` 与可选 `createErrorFallback()`，默认实现放在 `.cpp`，避免 incomplete `IPageInstance` + `std::unique_ptr` 头文件实现风险。

Runtime unit tests 新增：

- LegacyBeforeActivate 跨 provider 成功
- LegacyIfStateless 必须显式 stateless
- cross-provider 失败后 ErrorPage 优先级
- 全 fallback chain 失败 diagnostics

## UI / Quick Runtime

已有：

- QML module `LfQuick`
- LfPage / QuickHostRoot
- QuickPageProvider / QuickRuntime
- Theme / Tokens
- OverlayHost / BusyOverlay / OverlayCoordinator
- FocusNavigator
- QmlErrorCollector
- PageLoadError framework fallback
- provider / descriptor / C++ navigation guard extension points

### QML-friendly Navigation Guard

新增：

```text
NavigationGuardState
NavigationGuardController
```

State properties：

- dirty
- busy
- enabled
- reason
- confirmRequired

完整流程：

```text
route request
  ↓
controller guard evaluates state
  ↓ blocked
store exact NavigationContext
  ↓
UI receives confirmationRequested / hardBlocked
  ├─ cancelPending() -> stay on current page
  └─ confirmPending() -> bypass THIS controller once
                         + replay exact action/route/parameters once
                         + other guards still execute
```

Guard 不创建 Dialog；Dialog 完全由 UI 层负责。

`QuickRuntime.navigationGuard` 暴露默认 controller。`clearNavigationGuards()` 清理外部 guards 后会重新挂回默认 controller，避免 QuickRuntime guard API 被意外清空。

UI unit tests 新增：

- confirm retry exactly once
- cancel keeps route
- hard block cannot confirm
- parameters preserved through retry

## WidgetBridge

已有：

- QuickSurface
- WidgetPageProvider
- QStackedWidget lifecycle adapter
- legacy QWidget factory registry

### WidgetFocusBridge

新增：

```text
WidgetFocusBridge
FocusTransferPolicy
```

当前能力：

- before QWidget + Tab -> QML entry item
- QML exit item + Tab -> after QWidget
- after QWidget + Backtab -> QML exit item
- QML entry item + Backtab -> before QWidget
- rememberFocus / restoreFocus，供 route back / technology switch 使用
- scannerIsolation 开关：开启时禁止跨技术栈 focus transfer
- modalFocusTrap：有 active modal QWidget 时禁止跨栈转移

该逻辑完全位于 WidgetBridge，不进入 Runtime。

WidgetBridge tests 增加基础 transfer / scanner isolation 场景；仍需 Qt 6.8.3 Windows 实编和 GUI test 环境验证。

## Hybrid Demo

当前 Demo 验证：

```text
QApplication / QWidget Shell
          │
      QStackedWidget
       /          \
QuickSurface   Legacy QWidget
     │               │
QuickProvider   WidgetProvider
       \          /
        QuickRuntime
            │
        PageRouter
```

新增实战场景：

1. 正常 QML ↔ Legacy QWidget 共用 Router/History；
2. 故意缺失 QML source → Router → WidgetPageProvider → legacy results page；
3. legacy fallback 失败时继续走 provider-native ErrorPage；
4. QML 页面未保存编辑：Dialog confirm/cancel；
5. maintenance/calibration busy：hard block；
6. `QLineEdit -> QML TextField -> QLineEdit` Tab/Backtab；
7. scanner focus isolation 开关；
8. route back focus remember/restore 基础。

## Platform

已有：

- ScannerSession
- KeyboardWedgeScanner
- InputRouter

## DataView

已有：

- DataTableModel
- DataFilterProxyModel
- DataGrid MVP
- synchronized header
- row selection
- keyboard navigation
- movable / resizable columns
- column titles
- Medical Demo 10k / 5k synthetic rows

尚未进入本轮实现的第二阶段：

- stable column id
- ColumnStateModel / ColumnConfigStore
- width/order/visibility persistence
- edit transaction / validation rollback
- Check/Combo/Progress/Icon/Editor delegates
- batch selection / context menu
- cross-table nav
- 20k / 50k stress benchmark

## Medical Demo

已有 Dashboard / Samples / Results / Instrument / Alarms / Developer，以及 DataGrid、scanner、overlay、focus 等基础验证。

下一批优先实战页面：

- Sample Register
- Sample Rack
- Results Review

每页必须验证 2~4 个 LfQuick 子系统，不做纯视觉页面。

## CI 当前状态

本轮结束前最新 GitHub Actions：

```text
run: 38892175280
head: 7bc28f77616c897a9d7379df32d7d1bbacaae5d6
job: windows-qt683
status: completed
conclusion: failure
steps: none / null
logs_url: null
```

因此仍属于 runner / 基础设施异常：job 创建后几秒即失败，没有 checkout / Install Qt / Configure / Build / Test。

当前结论：

```text
Architecture/source implementation: progressed
Workflow triggered: yes
Real Qt 6.8.3 + MSVC2022 compile: NOT EXECUTED
CI Green: NO CLAIM
Source compile failure: NO COMPILER EVIDENCE
```

Runner 一旦恢复并出现真实 steps，必须第一优先级处理所有 Configure / Build / CTest 问题，再继续大功能。

重点实编检查：

- PageDescriptor `std::optional<PageFallbackDescriptor>` public header
- IPageProvider virtual fallback ABI
- NavigationGuardController Qt moc / Q_PROPERTY pointer types
- Hybrid Demo `QVariant::fromValue(QObject*)` initial properties
- WidgetFocusBridge QQuickWidget/QQuickWindow activeFocusItem behavior
- GUI unit tests 在 Windows runner 的 focus 稳定性
- QML Dialog / TextField registration and resource URLs

## 已知设计债务

- PageCachePolicy::Singleton 还没有 route-global singleton 去重语义。
- LRU 仍按 inactive page 数量，不按 memory weight。
- `LegacyIfStateless` 当前使用 `metadata["stateless"]` 显式声明；后续可升级为 typed descriptor field。
- provider-native ErrorPage 目前通过 IPageProvider extension 实现；后续可增加更正式的 structured provider error contract。
- NavigationGuardController 当前提供单 pending request 模型；连续快速请求会由最后一个阻塞请求覆盖 pending，后续需要明确 queue/drop policy。
- Focus Bridge 当前是 boundary-based MVP，不尝试重写完整 Qt/QML focus chain。
- route back focus restore 还需要真实 GUI stress 测试。
- CI 尚无真实编译结果。

## 下一开发顺序

1. **Build gate**：runner 一恢复立即修 Qt 6.8.3/MSVC2022。
2. **DataGrid 第二阶段**：stable id + persistence + transaction，优先于 NavigationView。
3. DataGrid delegates + batch/context/keyboard + 20k/50k benchmark。
4. Medical Demo：Sample Register / Sample Rack / Results Review。
5. Permission attached API + Validation RuleTable + Overlay/Dialog 双栈一致性。
6. 再考虑 WindowRouter / NavigationView / Charts / Diagnostics / Hot Reload。

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

禁止把 EventFunc / LfWidget.dll / WindowManager / QXlsx / LimeReport / HookProc / ScanerHook / Patient-Sample-Reagent domain / medical Permission IDs / QC-Calibration algorithms 放入 Core。

外部参考继续遵守 `docs/REFERENCE_POLICY.md`：FluentUI MIT 源码可在许可证下研究；FluentUI2/Pro 仅 public API/behavior/qmltypes/demo clean-room 参考，不逆向 DLL、不还原私有实现。
