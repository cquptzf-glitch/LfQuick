# LfQuick 当前实现状态

> 更新时间：2026-10-16  
> 开发策略：**main-only**。后续不再创建功能分支；已有功能分支完成审查后直接合并到 `main`，新的实现直接提交 `main`。

## 当前基线

- Qt：6.8.3
- C++：20
- CMake：3.24+
- 目标升级：Qt 6.12 LTS
- 当前主线：`main`

## 已完成

### Repository / Build
- CMake module skeleton
- Qt 6.8.3 package requirement
- GitHub Actions Windows/MSVC baseline
- architecture boundary checker
- unit-test wiring
- main-only development workflow

### Core
- Error / Result
- OperationHandle
- Permission
- Validation
- ICommand / CommandContext
- Event / EventDispatcher / RAII subscription
- RouteRequest / RouteResult

### Runtime
- PageDescriptor
- PageRegistry
- PageProviderRegistry
- IPageProvider / IPageInstance
- PageLifecycle
- PageRouter push / replace / back / reset
- cache-aware instance lifecycle (None vs retained cache policies)

### UI
- QML module `LfQuick`
- LfPage
- QuickPageProvider
- QuickRuntime
- QuickHostRoot
- Theme / Tokens
- OverlayHost / BusyOverlay
- OverlayCoordinator
- FocusNavigator

### Platform
- ScannerSession
- KeyboardWedgeScanner
- InputRouter

### DataView
- DataTableModel
- DataFilterProxyModel
- DataGrid MVP
- row selection / keyboard navigation
- synchronized header
- resizable / movable columns

### Medical Demo
- Dashboard
- Samples
- Results
- Instrument
- Alarms
- Developer
- 10k synthetic sample rows
- 5k synthetic result rows
- scanner simulation
- busy overlay scenario
- focus playground
- DataGrid/filter/sort scenario

## 本轮实现原则

1. **不以控件数量为 KPI**，以真实医疗业务链路验证框架 API。
2. Core/Runtime 不依赖 QWidget/Qt Quick。
3. Platform 通过可选 target 隔离 GUI 输入适配。
4. DataView 使用 Qt Model/View contract，不在 QML 中维护大规模业务数据。
5. DataGrid 以 Qt Quick `TableView` 为底座，LfQuick 负责统一 API、交互和视觉 token。
6. Overlay 只负责状态协调与呈现，不承载业务流程。
7. Scanner 只提供输入适配，不参与样本业务规则。
8. Theme 不绑定 FluentUI；后续可以替换为自有 Style。
9. FluentUI MIT 源码只用于允许范围内的实现研究；FluentUI2/Pro 按 clean-room / public-contract 策略处理。

## 下一阶段

### P0
- dedicated LRU cache manager / eviction telemetry
- navigation guard / unsaved-change guard
- QML error collector + fallback
- QuickSurface first-frame telemetry
- WidgetBridge hybrid sample

### P1
- DataGrid column state persistence
- column visibility / reorder persistence
- editing delegates
- batch selection actions
- virtualized stress benchmark
- scanner timeout / prefix / suffix / manual-input conflict tests

### P2
- NavigationView
- WindowRouter
- Charts abstraction
- QC / Calibration / Reaction Curve
- diagnostics dashboard
- QML hot reload adapter

## CI 说明

当前 GitHub Actions 的 Windows runner 曾出现“Job 创建后立即结束、未分配 runner、无编译 steps”的基础设施状态。

因此：
- 不把该状态误判为 C++ 编译失败；
- 不在没有真实 Qt 编译结果时宣称 CI Green；
- 每次恢复 runner 后必须重新验证 Qt 6.8.3 + MSVC2022 全量构建和 CTest。

## 分支规则

从本阶段开始：

```text
main
 │
 ├── 直接开发
 ├── 直接提交
 └── 必要时回滚/修复

已有 feature branch
        ↓
审查
        ↓
合并 main
        ↓
不再继续从 feature branch 开发
```

禁止：
- 为普通功能创建新 branch；
- 在 branch 上长期堆积未验证代码；
- 未完成 main 合并就从旧 branch 继续开发。

## Commit 规则

推荐：
```text
feat(core): ...
feat(runtime): ...
feat(ui): ...
feat(platform): ...
feat(dataview): ...
feat(demo): ...
fix(...): ...
docs(...): ...
test(...): ...
build(...): ...
```

一个 commit 尽量对应一个可以独立说明的功能变化。

## 参考 FluentUI / Pro 的执行规则

详见：
- `docs/REFERENCE_POLICY.md`
- `docs/REFERENCE_IMPLEMENTATION_MAP.md`
- `THIRD_PARTY_NOTICES.md`

任何 AI/开发者提交参考外部项目的代码，都必须说明来源、版本、许可证影响以及是否复制代码。
