# LfQuick 新窗口开发交接

> 交接日期：2026-10-16  
> 仓库：`https://github.com/cquptzf-glitch/LfQuick`  
> 开发分支策略：**只在 `main` 继续开发，不再创建普通功能分支。**

## 1. 新窗口第一句话应该做什么

新窗口拿到本文件后，不要重新从需求分析开始，也不要询问是否继续。直接：

1. 读取 `main` 最新状态；
2. 读取本文件、`docs/IMPLEMENTATION_STATUS.md`、`docs/LfQuick_详细开发计划.md`；
3. 检查最新 GitHub Actions；
4. 如果 Actions 能真正进入编译步骤，优先修编译/测试问题；
5. 如果 Actions 仍然是 job 无 steps 的 runner 异常，则按本文“下一开发批次”继续实现，同时保持“未实编验证”的明确标记。

## 2. 用户目标

LfQuick 是面向 Qt 6.8.3、未来升级 Qt 6.12 LTS 的通用 UI Runtime Framework，主要用于：

- 现有大型 QWidget 医疗/IVD 上位机渐进迁移到 QML；
- 新项目纯 QML；
- QWidget + QML + C++ 混合运行；
- UI 与业务层隔离；
- Theme/Style 可替换，不能把 Fluent 写进 Core；
- 医疗设备高频能力：DataGrid、扫码、Focus、Overlay、Permission、Validation、Charts、诊断。

当前产品验证 Demo 是生化/免疫分析仪场景，但 **医疗领域模型不能进入 LfQuick Core**。

## 3. 强制开发规则

### 3.1 Git

- 直接在 `main` 开发。
- 不创建普通 feature branch。
- 每一批尽量形成语义清晰的小提交。
- 已有旧分支如果发现未合并内容，先审查并合并 `main`，不要继续从旧分支开发。

### 3.2 架构边界

必须持续满足：

```text
LfQuickCore      -> QtCore only
LfQuickRuntime   -> Core，禁止 QWidget/QQuickItem
LfQuickUi        -> Core + Runtime + Qt Quick
LfQuickDataView  -> Ui + Qt Model/View/Quick
LfQuickPlatform  -> platform/input abstraction
WidgetBridge     -> optional Qt Widgets / QuickWidgets
Integration.*    -> legacy/product-specific adapters
```

以下内容不能进入 Core：

- EventFunc Adapter
- LfWidget.dll Adapter
- WindowManager Adapter
- QXlsx / LimeReport
- HookProc / ScanerHook
- Patient / Sample / Reagent domain model
- concrete medical permission IDs
- QC/Calibration domain algorithms

### 3.3 外部参考

继续遵守：

- `docs/REFERENCE_POLICY.md`
- `docs/REFERENCE_IMPLEMENTATION_MAP.md`
- `THIRD_PARTY_NOTICES.md`

规则：

- `zhuzichu520/FluentUI` MIT 公开源码可研究；如果复制/实质衍生，要保留 MIT 记录和说明。
- FluentUI2 / FluentUI Pro 只作为公开 API/行为/qmltypes/demo 的 clean-room 参考。
- 不逆向 DLL，不反编译私有实现，不声称掌握未公开 Pro 源码。
- LfQuick 自身架构优先于“像 FluentUI”。

## 4. 当前功能代码基线

本轮功能代码完成到：

```text
1c8f6398011257cc1a2ff926adc0f9b6d5f90087
feat(ui): expose provider and navigation guard extension points
```

本交接文档/状态文档提交会比这个 SHA 更新，但功能实现以以上提交及其父提交为核心。

本轮关键提交：

```text
c0136fd feat(runtime): add navigation guards and LRU page eviction
d3615ed feat(ui): add QML error collection and fallback page
d45a4d5 feat(widgetbridge): add shared-router hybrid migration path
bfebd2c fix: harden QML fallback and optional widget lifecycle hooks
1c8f639 feat(ui): expose provider and navigation guard extension points
```

## 5. 当前已经实现的模块

### 5.1 Core

已有：

- `Error`
- `Result`
- `OperationHandle`
- Permission / Validation contracts
- `ICommand`
- `EventDispatcher` + RAII Subscription
- RouteRequest / RouteResult

Core 不依赖 Widgets/Quick。

### 5.2 Runtime

关键文件：

```text
src/runtime/include/LfQuick/Runtime/
  IPageInstance.h
  IPageProvider.h
  INavigationGuard.h
  PageDescriptor.h
  PageLifecycle.h
  PageProviderRegistry.h
  PageRegistry.h
  PageRouter.h

src/runtime/src/PageRouter.cpp
```

已经实现：

- push / replace / back / reset
- PageProvider registry
- Page lifecycle
- `INavigationGuard`
- NavigationContext / NavigationDecision
- guard block signal
- `PageCachePolicy::None`：离开时销毁实例，返回时重建
- `KeepAlive`：suspend/resume
- `Lru`：inactive page 保留数量限制 + oldest eviction
- `cacheEntryEvicted` signal
- hard reset 不经过 guard，用于 shutdown/recovery

当前已知不足：

- `Singleton` 还没有真正 route-global singleton 去重，只是 retained behavior。
- LRU 只按数量，不按 memory cost / weight。
- fallback 还不是 Router 级 cross-provider fallback。

### 5.3 UI / Quick Runtime

关键文件：

```text
src/ui/include/LfQuick/Ui/
  QuickRuntime.h
  QuickPageProvider.h
  QmlErrorCollector.h
  OverlayCoordinator.h
  FocusNavigator.h

src/ui/src/
  QuickRuntime.cpp
  QuickPageProvider.cpp
  QmlErrorCollector.cpp

src/ui/qml/
  LfPage.qml
  QuickHostRoot.qml
  PageLoadError.qml
  OverlayHost.qml
  BusyOverlay.qml
  FocusDebugPanel.qml
  Theme.qml
  Tokens.qml
```

已经实现：

- QuickRuntime facade
- 注册 QML page
- 注册额外 `IPageProvider`
- 注册原始 `PageDescriptor`
- 注册 `INavigationGuard`
- QML host
- Quick provider activation callback
- QML load structured error collector
- framework `PageLoadError.qml`
- primary QML 失败后 fallback ErrorPage
- fallback 不继承原业务 initialProperties，防止错误页二次失败
- LRU limit 转发到 Runtime
- navigation blocked / cache eviction signal 转发

### 5.4 WidgetBridge

关键文件：

```text
src/widgetbridge/include/LfQuick/WidgetBridge/
  LfQuickWidgetBridgeGlobal.h
  QuickSurface.h
  WidgetPageProvider.h

src/widgetbridge/src/
  QuickSurface.cpp
  WidgetPageProvider.cpp
```

已经实现：

#### QuickSurface

- QQuickWidget host
- shared `QQmlEngine`
- load state
- `loadDurationMs`
- QWidget 第一次 paint 作为 first-frame metric
- load error
- strong focus policy

#### WidgetPageProvider

- provider id = `widget`
- legacy QWidget factory registry
- source contract：`descriptor.source["factory"]`
- QStackedWidget host
- activate/deactivate/suspend/resume/destroy adapter
- optional QWidget lifecycle hook，不存在 hook 时不产生 invoke warning

### 5.5 Hybrid Demo

路径：

```text
examples/hybrid-demo/
  CMakeLists.txt
  README.md
  cpp/main.cpp
  qml/HybridQuickPage.qml
```

目标：证明 **一个 PageRouter 同时驱动 QML 页面和旧 QWidget 页面**。

结构：

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

构建：

```bash
cmake -S . -B build \
  -DLFQUICK_BUILD_WIDGET_BRIDGE=ON \
  -DLFQUICK_BUILD_HYBRID_DEMO=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

### 5.6 Platform

已经有：

- ScannerSession
- KeyboardWedgeScanner
- InputRouter

### 5.7 DataView

已经有：

- DataTableModel
- DataFilterProxyModel
- DataGrid MVP
- header
- row selection
- keyboard nav
- movable/resizable columns
- column title
- Medical Demo 10k/5k synthetic rows

DataGrid 还不是完成版。

## 6. 当前 CI 状态 — 非常重要

最新确认的 run：

```text
run: 38889981763
head: 1c8f6398011257cc1a2ff926adc0f9b6d5f90087
job: windows-qt683
conclusion: failure
steps: none / empty
```

前几个 run 也出现同样特征：

```text
Job 被创建
↓
几秒后 failure
↓
没有 checkout
没有 Install Qt
没有 Configure
没有 Build
没有 Test
没有可下载 job logs
```

所以不要把这解释为代码编译错误。

正确表述：

```text
GitHub Actions infrastructure/runner abnormal
Real Qt 6.8.3 compile has NOT executed
Do not claim CI green
Do not claim code compile failure without compiler output
```

新窗口如果看到 runner 恢复并出现真实 steps，必须先处理实际编译错误，再继续大功能。

## 7. 下一开发批次：建议严格按此顺序

### Step 0 — Build gate

先查 GitHub Actions。

如果 runner 恢复：

1. Configure
2. Build
3. CTest
4. 修所有 Qt 6.8.3 / MSVC2022 编译问题
5. 确认 hybrid demo 也编译

重点检查可能暴露问题的位置：

- QML registrar 对 `QmlErrorCollector` 的处理
- `PageLoadError.qml` resource URL
- QQuickWidget/QuickSurface `paintEvent` override
- `qt_add_qml_module` executable module resource path
- WidgetPageProvider/QStackedWidget ownership

### Step 1 — Router 级 cross-provider fallback

目标：

```text
QML page create failed
        ↓
QuickProvider failure
        ↓
PageRouter reads fallback descriptor
        ↓
WidgetProvider
        ↓
legacy QWidget page
```

建议不要在 Runtime 写死 `widget`。

推荐新增通用 contract，例如：

```cpp
struct PageFallbackDescriptor {
    QString provider;
    QVariantMap source;
};

struct PageDescriptor {
    ...
    std::optional<PageFallbackDescriptor> fallback;
};
```

Router 只根据 provider id 尝试 fallback。

要考虑：

- fallback 是否只允许 before-activate
- primary 已经产生 side-effect 后是否允许 fallback
- ErrorPage 和 Legacy fallback 的优先级
- diagnostics 如何携带 primary provider error

不要把 UnionLab/LfWidget 名字放进 Runtime。

### Step 2 — QML-friendly Navigation Guard

C++ `INavigationGuard` 已完成。

下一步做 UI adapter，例如：

```text
NavigationGuardState / NavigationGuardController

properties:
  dirty
  busy
  enabled
  reason
  confirmRequired
```

典型医疗业务：

- 结果编辑未保存
- 校准进行中
- 维护动作进行中
- 样本登记未提交

需要支持：

```text
route request
   ↓
guard blocked
   ↓
confirm dialog
   ├── cancel -> remain
   └── discard/confirm -> retry navigation once
```

避免 guard 自己直接弹 Dialog；guard 只返回 decision，UI 编排确认流程。

### Step 3 — Widget/QML Focus Bridge

必须验证：

- QWidget -> QML Tab
- QML -> QWidget Tab/Backtab
- focus restore after route back
- scanner focus isolation
- Dialog focus trap

建议新增：

```text
WidgetFocusBridge
FocusTransferPolicy
```

并在 hybrid demo 增加 TextField + QLineEdit 联合测试。

### Step 4 — DataGrid 第二阶段

优先于 NavigationView。

实现：

1. stable column id
2. width/order/visibility persistence
3. column configuration model
4. edit transaction
5. validation rollback
6. Check/Combo/Progress/Icon delegates
7. batch selection
8. keyboard/cross-table nav
9. 20k/50k rows benchmark

不要把业务字段写进 DataGrid。

### Step 5 — 医疗 Demo 真实页面

优先：

- Sample Register
- Sample Rack
- Results Review

要求每个页面至少验证两个框架能力，而不是纯视觉页面。

例如 Results Review：

```text
DataGrid
+ NavigationGuard
+ Permission
+ Validation
+ Overlay
```

## 8. 之后的阶段

再推进：

- Permission attached property
- Validation RuleTable
- Toast/Dialog
- WindowRouter
- NavigationView
- Charts abstraction
- QC / Calibration / Reaction Curve
- QCustomPlot wrapper/backend
- Diagnostics HUD
- Hot reload
- Qt 6.12 adapter

## 9. 不要做的事情

- 不要先重写巨大 NavigationView。
- 不要为了“全 QML”强行移除所有 QWidget。
- 不要让 QML 直接依赖 EventFunc/LfWidget。
- 不要让 Scanner 处理业务规则。
- 不要在控件里硬编码大批颜色/z-index。
- 不要把 FluentUI API 当成 LfQuick Core API。
- 不要在 CI 没跑编译步骤时宣称“构建通过”。
- 不要创建新的普通 feature branch。

## 10. 新窗口推荐工作方式

每批开发保持：

```text
读取 main
  ↓
选择 2~4 个强相关模块
  ↓
实现 + tests + demo
  ↓
静态复核
  ↓
直接提交 main
  ↓
查 Actions
  ↓
更新 IMPLEMENTATION_STATUS / HANDOFF
```

如果一次任务较大，优先交付可工作的纵向链路，而不是几十个空类。

## 11. 建议新窗口开场提示词

见聊天中的同名交接块。也可以直接把本文件全文作为上下文，让新窗口从 Step 0 / Step 1 开始继续。
