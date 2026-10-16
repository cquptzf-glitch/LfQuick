# LfQuick 当前实现状态

> 更新时间：2026-10-16  
> 开发策略：**main-only**。不为普通功能创建新分支；已有分支审查后合并 `main`，后续直接在 `main` 开发。

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
- hybrid demo build option

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
- `INavigationGuard` / NavigationContext / NavigationDecision
- navigation blocked signal and failure propagation
- `PageCachePolicy::None` 离页释放
- `KeepAlive` suspend/resume
- `Lru` inactive-page retention + configurable limit
- LRU eviction signal
- hard reset bypass guard for shutdown/recovery

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
- QmlErrorCollector
- PageLoadError framework fallback page
- primary/fallback load diagnostics
- QuickRuntime provider registration extension
- QuickRuntime raw PageDescriptor registration extension
- QuickRuntime navigation-guard registration extension
- Quick activation callback for hybrid shells

### WidgetBridge

- QuickSurface
- shared QQmlEngine support
- QuickSurface load state
- QML load duration metric
- first QWidget paint / first-frame metric
- WidgetPageProvider
- legacy QWidget factory registry
- QWidget lifecycle adapter
- QStackedWidget host integration
- `examples/hybrid-demo`
- Quick + Widget shared PageRouter/history PoC

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
- column titles

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
- QuickRuntime/PageRouter page loading

## 本轮新增提交

```text
c0136fd feat(runtime): add navigation guards and LRU page eviction
d3615ed feat(ui): add QML error collection and fallback page
d45a4d5 feat(widgetbridge): add shared-router hybrid migration path
bfebd2c fix: harden QML fallback and optional widget lifecycle hooks
1c8f639 feat(ui): expose provider and navigation guard extension points
```

## 当前关键运行路径

```text
Application / UnionLab Adapter
          │
          ▼
      QuickRuntime
          │
          ▼
      PageRouter
      /       \
 QuickProvider WidgetProvider
      │            │
 QQuickItem      QWidget
      │            │
 QuickSurface ← shared QWidget shell
```

QML 页面加载错误路径：

```text
Primary QML
    │ load/create error
    ▼
QmlErrorCollector
    │
    ├── structured diagnostics
    ▼
PageLoadError.qml
```

## 本轮实现原则

1. **不以控件数量为 KPI**，优先验证真实医疗业务迁移链路。
2. Core/Runtime 不依赖 QWidget/Qt Quick。
3. WidgetBridge 只承担技术栈桥接，不引入 UnionLab/LfWidget 业务依赖。
4. Router 不关心页面是 QML 还是 QWidget。
5. DataView 使用 Qt Model/View contract，不在 QML 中维护大规模业务数据。
6. Overlay 负责状态协调与呈现，不承载业务流程。
7. Scanner 负责输入适配，不参与样本业务规则。
8. Theme 不绑定 FluentUI；Fluent 只是未来 Style 实现之一。
9. FluentUI MIT 可在许可证范围研究；FluentUI2/Pro 只按 public API/behavior clean-room 参考。

## 下一阶段

### P0 — 先验证、再扩展

- 恢复/确认 GitHub Actions Windows runner，取得真实 Qt 6.8.3 + MSVC2022 编译结果
- 修复真实编译器/Qt QML registrar 暴露出的错误
- 自动 cross-provider fallback：`quick -> widget`，而不是只 fallback 到 ErrorPage
- QML/VM 侧 NavigationGuard adapter + confirm workflow
- WidgetBridge focus hand-off：QWidget ↔ QML Tab/Backtab/active focus
- QuickSurface DPI/resize/z-order/first-frame stress
- hybrid demo 增加失败 QML → legacy page 回退场景

### P1 — DataGrid 第二阶段

- stable column id
- column visibility / reorder / width persistence
- QSettings / product storage adapter
- editing transaction：Begin / Validate / Commit / Rollback
- Check / Combo / Progress / Icon / Editor delegates
- context menu / batch selection
- 20k / 50k virtualization stress benchmark
- cross-table keyboard navigation

### P2 — 医疗迁移能力

- Scanner timeout / prefix / suffix / manual-input conflict tests
- Permission attached API + role refresh
- Validation RuleTable + form feedback
- Toast/Dialog/Busy dual-stack consistency
- Sample Register / Sample Rack 页面 PoC
- QCustomPlot/Chart backend PoC

### P3

- NavigationView
- WindowRouter / FramelessWindow
- Charts abstraction
- QC / Calibration / Reaction Curve
- diagnostics dashboard
- QML hot reload adapter

## CI 当前状态

截至 2026-10-16，本轮最新 Actions run：

```text
run: 38889981763
head: 1c8f6398011257cc1a2ff926adc0f9b6d5f90087
job: windows-qt683
status: completed
conclusion: failure
steps: empty / none
```

这仍属于仓库之前已经出现的 GitHub Actions runner/基础设施异常特征：job 在执行任何 checkout/configure/build/test step 前结束。

因此当前结论必须保持为：

```text
Architecture / source implementation: progressed
GitHub Actions workflow triggered: yes
Real Qt 6.8.3 compile result: NOT AVAILABLE
CI Green: NO CLAIM
Code compile failure: NO EVIDENCE YET
```

一旦 runner 恢复，第一件事是重新验证：

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DLFQUICK_BUILD_WIDGET_BRIDGE=ON \
  -DLFQUICK_BUILD_HYBRID_DEMO=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

## 已知未完成/设计债务

- `PageCachePolicy::Singleton` 当前仍按 retained instance 处理，还没有 route-global singleton 去重语义。
- LRU 当前按“历史栈中 inactive Lru page 数量”淘汰，还没有内存预算/页面成本权重。
- fallback 目前 QuickProvider 内置 ErrorPage；还没有 Router 级 fallback provider descriptor。
- `INavigationGuard` 已有 C++ 注册入口，但还没有 QML-friendly guard object / confirm dialog 编排。
- WidgetProvider 已能参与同一 Router，但 QML → QWidget 自动 fallback 尚未实现。
- QuickSurface 已有 load/first-paint 指标，但尚未完成 QWidget↔QML focus bridge。
- GitHub Actions 尚未给出真实编译步骤，因此新代码需要第一时间用 Qt 6.8.3/MSVC2022 实编验证。

## 分支规则

```text
main
 │
 ├── 直接开发
 ├── 直接提交
 └── 必要时修复/回滚

已有 feature branch
        ↓
审查
        ↓
合并 main
        ↓
停止继续从旧 branch 开发
```

禁止：

- 为普通功能创建新 branch；
- 在 branch 上长期堆积未验证代码；
- 从已合并旧 branch 继续开发。

## 外部参考规则

详见：

- `docs/REFERENCE_POLICY.md`
- `docs/REFERENCE_IMPLEMENTATION_MAP.md`
- `THIRD_PARTY_NOTICES.md`

任何 AI/开发者提交参考外部项目的代码，都必须说明来源、版本、许可证影响以及是否复制代码。
