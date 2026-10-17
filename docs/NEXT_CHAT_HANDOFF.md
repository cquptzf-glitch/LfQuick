# LfQuick 开发交接（当前权威版本）

> 更新时间：2026-10-17  
> 仓库：`https://github.com/cquptzf-glitch/LfQuick`  
> 分支策略：**main-only**。  
> 当前阶段：**主要功能已基本完成；继续源码/API/生命周期/安装包收口与最终验证。**  
> 当前明确约束：**不要恢复 GitHub Actions / CI / release workflow，除非用户明确要求。**

## 1. 新窗口开场直接做什么

不要重新做需求分析，也不要重复实现已经完成的：

```text
PageRouter / WindowRouter
Navigation / ApplicationShell
Permission / Validation / Overlay
DataGrid
Charts / Qt Graphs
Diagnostics
Hot Reload
Design System / Base Controls
Platform Services
PlatformQml / Integration scanner bridge
QRangeModel / Canvas2D / StyleKit adapter
local developer tooling
```

直接执行：

1. 读取 main 最新 HEAD；
2. 读取本文件与 `docs/IMPLEMENTATION_STATUS.md`；
3. 保持 main-only；
4. 不创建 `.github/workflows/*`；
5. 优先修静态/API/生命周期/CMake/package 明确问题；
6. 不再扩张新的大框架模块；
7. 下一关键里程碑是可用 Windows + Qt 6.8.3 环境的 full Configure / Build / CTest；
8. 之后验证 optional Qt 版本、真实 GUI、deploy/package；
9. 最后才恢复 CI/release。

---

## 2. 当前可信状态

```text
Major functionality implemented: YES
Source-level hardening substantially implemented: YES
Current HEAD full Build verified: NO
Current HEAD full CTest verified: NO
Current HEAD deploy/package verified: NO
CI workflows present: NO (intentional)
CI Green: NO CLAIM
```

删除 Actions 之前，一个较早的 Windows Qt 6.8.3 / MSVC2022 revision 曾通过：

```text
Architecture
Configure
full Build
13/13 CTest
Qt deploy
package validation
artifact upload
```

它只是历史基线。当前 HEAD 之后加入了大量功能与 hardening，不能把历史结果当作当前验证结果。

---

## 3. 架构边界

```text
Core -> QtCore only
Runtime -> Core；禁止 QWidget/QWindow/QQuickItem
Ui -> Core + Runtime + Qt Quick；无产品业务
Platform -> Core + QtCore/Gui
PlatformQml -> Platform 的可选 QML leaf
Integration -> Ui + Platform 的可选 cross-module leaf
DataView -> Ui；无医疗业务算法
WidgetBridge -> optional Widgets / QuickWidgets
Charts -> backend-neutral visualization；无 QC/Calibration 算法
Diagnostics / Tooling / Styles / QtAdapters -> leaf-only
```

Qt 主基线：

```text
Qt: 6.8.3
C++: C++20
CMake: 3.24+
```

高版本能力必须隔离：

```text
SafeArea                  >= 6.9
QRangeModel               >= 6.10
QCanvasPainter C++        >= 6.11
QRangeModel drag/drop     >= 6.12
QtCanvas2D QML            >= 6.12
qt_add_qml_preview        >= 6.12
Qt Labs StyleKit          >= 6.12
```

---

## 4. Runtime / 页面与窗口生命周期

PageRouter 已具备：

```text
None / KeepAlive / Singleton / weighted LRU
route-global Singleton
parameter refresh
cross-provider fallback
structured provider errors
IPageInstance::isValid()
invalid cached instance cleanup/rebuild
invalid singleton history/forward cleanup
invalid current instance 不再 deactivate/suspend/resume
```

QuickRuntime / QuickPageProvider 最新 contract：

```text
QuickRuntime.host 是 routed Quick page scene root
host switch/destruction -> router reset + provider host 同步
缺 host -> structured quick.host-missing
activation callback 允许重入；callback 若同步销毁 host/page，不再继续解引用旧 item
QML lifecycle hook 自销毁 page 后不再访问旧 item
parameter setter 若同步销毁 page，立即停止后续 property write
```

WindowRouter / WidgetWindowProvider 已具备：

```text
single/multi instance
owner cycle rejection
reparent / detach
CloseWithOwner / Detach
children-first close
provider-native saveState / restoreState
native close request -> Router
external delete QWidget -> Router record reconciliation
Widget QObject parent 与 native transient owner 分离
```

---

## 5. UI / Overlay / Focus / Design System

已有：

```text
QuickRuntime / QuickPageProvider
QmlErrorCollector / PageLoadError
OverlayCoordinator / BusyToken / Toast / Dialog
FocusWalker / FocusManager / FocusNavigator
NavigationGuardController
NavigationItemModel / NavigationView / ApplicationShell
Theme / Tokens / Units / Typography / Motion / DpiHelper
ThemeSystemAdapter / IconLoader
```

Overlay 生命周期已收口：

```text
BusyToken release / external-end / destructor
coordinator destruction
OperationHandle 正常结束释放 busy
OperationHandle 被业务侧直接销毁也释放 busy entry
```

`Lf` attached context 有真实消费路径：

```text
Lf.permission
Lf.validationRule
Lf.focusPolicy
Lf.focusOrder
Lf.acceptScanner
Lf.helpContext
```

Base controls / Design System Playground 已完成，不再优先扩充组件数量。

---

## 6. Platform / Integration 生命周期

Platform：

```text
ScannerSession
KeyboardWedgeScanner / MockScanner
InputTarget / InputRouter
HotkeyRouter
SystemAppearance
ScreenService
ClipboardService
FileWatcher
RuntimeCapabilities
```

生命周期 hardening：

```text
InputRouter source/scanner/install target destruction synchronization
InputTarget target destruction -> targetChanged
repeated register/unregister connection ownership closure
HotkeyRouter install target destruction -> installedChanged
HotkeyRouter reinstall 不保留旧 target destroyed callback
```

PlatformQml / Integration 保持 leaf 方向：

```text
LfQuick.Platform
LfQuick.Integration
LfScannerTarget -> Lf.acceptScanner + activeFocus -> InputRouter
```

---

## 7. DataView / Charts / Diagnostics / Tooling

这些主线已经实现，不重新设计：

```text
DataView
    DataTableModel / DataFilterProxyModel
    DataGrid / Column / Header / Viewport
    persistence / editing / selection / keyboard / typed delegates
    20k/50k benchmark + DataGridTelemetry

Charts
    ChartDataModel / Series model
    Qt Graphs adapter
    Medical Demo Charts page

Diagnostics
    bounded events
    metrics/timing
    JSON snapshot/export
    ObjectInspector / AccessibilityAudit
    cross-thread DiagnosticsLogSink serialization

Tooling
    Hot Reload candidate-first swap
    rollback / state preservation
    shared watch ref-count
    architecture/lint/generator/deploy/screenshot tooling
```

---

## 8. CMake / SDK / package hardening（最新重点）

### 8.1 Build linkage contract

当前显式定义：

```text
BUILD_SHARED_LIBS=OFF  -> 默认完整 static LfQuick SDK
BUILD_SHARED_LIBS=ON   -> 完整 shared LfQuick SDK
```

Qt 6.8 构建设置：

```cmake
qt_policy(SET QTP0003 NEW)
```

目的：让 `qt_add_library()/qt_add_qml_module()` 与普通 `add_library()` 一致遵循 `BUILD_SHARED_LIBS`，禁止同一次构建出现 foundation static、QML backing shared 的隐式混合模式。

`lfquick_install_target()` 还会在 configure 阶段检查每个公开 SDK library 的实际 target type 是否与 `BUILD_SHARED_LIBS` 一致。

### 8.2 Installed CMake package

已收口：

```text
find_package(LfQuick REQUIRED COMPONENTS Core Ui ...)
    -> LfQuick_<Component>_FOUND 由真实 exported target 决定

PUBLIC Qt dependencies
    -> package config 同步加载对应 Qt imported targets

static SDK
    -> LFQUICK_STATIC 作为 INTERFACE compile definition 传播给下游

LfQuick_QT_MIN_VERSION
    -> 根据实际 build adapters 导出有效最低 Qt 版本
       base 6.8.3
       SafeArea 6.9
       RangeModel 6.10
       CanvasPainter / StyleKit 6.12
```

### 8.3 Installed static QML modules

所有当前 `qt_add_qml_module()` backing target 已显式捕获 `OUTPUT_TARGETS`，包括：

```text
LfQuickUi
LfQuickPlatformQml
LfQuickIntegration
LfQuickDataView
LfQuickCharts (Qt Graphs branch)
LfQuickDiagnostics
LfQuickTooling
LfQuickStyleKit
LfQuickSafeAreaAdapter
LfQuickCanvasPainterAdapter
LfQuickCustomQuickCharts
LfQuickFluentStyle
LfQuickBrandTestStyle
```

安装 helper 会：

```text
install/export QML OUTPUT_TARGETS
install object-library artifacts
避免 plugin 与 OUTPUT_TARGETS 重复安装
static plugin 导出稳定 target 名：LfQuick::<Component>QmlPlugin
shared plugin 安装到 QML module 目录
```

### 8.4 Downstream package smoke

CTest 已注册 `package_downstream_smoke`：

```text
1. cmake --install 当前 build 到临时 prefix
2. 独立 consumer 执行 find_package(LfQuick REQUIRED COMPONENTS ...)
3. 校验所有已构建 component targets
4. 校验 effective Qt minimum version
5. static 时校验 LFQUICK_STATIC interface define
6. static 时校验稳定 QML plugin exported targets
7. consumer 实际 build/link，不只 configure/generate
```

注意：这些测试已经写入源码，但**当前 HEAD 仍未在真实 Qt 6.8.3 环境执行**。

---

## 9. 当前 test/hardening 增量

除原有测试外，近期重点增加：

```text
PageRouter invalid cache/singleton lifecycle tests
QuickRuntime host switch/destruction tests
Quick page missing-host diagnostics
Quick page activation reentrancy/self-destruction test
OperationHandle destruction -> busy release test
HotkeyRouter install target lifetime tests
WidgetPage host generation/rebuild tests
WidgetWindow external-delete reconciliation
WidgetOverlay native parent destruction reconciliation
PlatformQml / Integration QML smoke tests
installed-package downstream build/link smoke
static QML plugin export/link smoke
```

真实 Windows GUI 仍需：

```text
100% <-> high-DPI multi-screen move
QQuickWidget popup/modal/native z-order
focus restore
native WindowRouter close
scanner isolation
30-60 minute soak
```

---

## 10. 下一步严格顺序

```text
1. 继续最后一轮 C++/QML/CMake 静态审查
2. 复核 README / package 使用说明与 BUILD_SHARED_LIBS contract
3. Windows + Qt 6.8.3：default static full Configure + Build
4. default static：全量 CTest（重点 package_downstream_smoke）
5. Qt 6.8.3：BUILD_SHARED_LIBS=ON full Configure + Build + CTest
6. Qt 6.10：QRangeModel optional build/test
7. Qt 6.12：StyleKit / Canvas2D optional build/test
8. 真实 GUI hardening
9. deploy/package + lfquick_deploy_check
10. 最后才恢复 GitHub Actions/release
```

在第 3-9 步完成之前，不得宣称当前 HEAD full-build/CTest/deploy green。

---

## 11. 参考/许可证边界

继续遵守：

```text
FluentUI (MIT) 可按许可证研究
FluentUI2 / FluentUI Pro 仅 clean-room public API/behavior reference
不逆向 proprietary DLL
Canvas Painter / QtCanvas2D 仍是 Technology Preview，并有独立 Qt 许可审查要求
```

见：

```text
docs/REFERENCE_POLICY.md
docs/QT_OPTIONAL_ADAPTERS.md
```
