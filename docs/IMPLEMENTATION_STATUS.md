# LfQuick 当前实现状态

> 更新时间：2026-10-17  
> 开发策略：**main-only**。  
> 当前阶段：**主要功能已完成，进入源码/API/生命周期/CMake/package hardening 与最终验证阶段。**  
> 当前约束：**GitHub Actions / CI / release workflow 已按用户要求移除，暂时不要恢复。**

## 1. 当前可信状态

```text
Qt primary baseline: 6.8.3
C++: C++20
CMake: 3.24+
Branch: main

Major functionality implemented: YES
Source-level hardening substantially implemented: YES
Current HEAD full Build verified: NO
Current HEAD full CTest verified: NO
Current HEAD deploy/package verified: NO
CI workflows present: NO (intentional)
CI Green: NO CLAIM
```

删除 Actions 之前，一个较早 Windows Qt 6.8.3 / MSVC2022 revision 曾通过 Architecture / Configure / full Build / 13/13 CTest / Qt deploy / package validation / artifact upload。当前 HEAD 后续增加了大量功能、stress、生命周期和安装包改动，因此必须重新验证。

---

## 2. 架构层级

```text
LfQuickCore         -> QtCore only
LfQuickRuntime      -> Core；禁止 QWidget / QWindow / QQuickItem
LfQuickUi           -> Core + Runtime + Qt Quick
LfQuickDataView     -> Ui + Qt Model/View/Quick
LfQuickPlatform     -> Core + QtCore/Gui platform/input services
LfQuickPlatformQml  -> optional QML leaf
LfQuickIntegration  -> optional Ui + Platform leaf
LfQuickWidgetBridge -> optional Widgets / QuickWidgets
LfQuickCharts       -> optional visualization
Diagnostics         -> optional leaf
Tooling             -> optional development leaf
Styles              -> optional leaves
QtAdapters          -> optional version-specific leaves
```

禁止 foundation 模块反向依赖 Diagnostics / Tooling / Integration / PlatformQml / styles / version adapters。

---

## 3. Core / Runtime

已实现：

```text
Error / Result / OperationHandle
Command / Event / Dispatcher / Subscription
Permission / Validation / Route / Telemetry contracts
PageRegistry / PageProviderRegistry
PageRouter push / replace / back / forward / reset
None / KeepAlive / Singleton / weighted LRU
route-global Singleton
cross-provider fallback
structured provider diagnostics
NavigationGuard
WindowProviderRegistry / WindowRouter
single/multi instance
owner / reparent / detach / close policy / close guard
provider-native saveState / restoreState
```

近期生命周期 hardening：

```text
IPageInstance::isValid()
invalid KeepAlive/LRU cache cleanup + rebuild
invalid Singleton 全 history/forward 引用清理
invalid current page 不再继续 deactivate/suspend/resume
1000-cycle PageRouter stress
WidgetWindow external-delete -> Router record reconciliation
```

---

## 4. UI / Design System

已实现：

```text
QuickRuntime / QuickPageProvider
QmlErrorCollector / PageLoadError
OverlayCoordinator / BusyToken / Toast / Dialog
FocusWalker / FocusManager / FocusNavigator
NavigationGuardController
NavigationItemModel / NavigationView / ApplicationShell
Theme / Tokens / Units / Typography / Motion / DpiHelper
ThemeSystemAdapter / IconLoader
Permission / Validation / Help attached contracts
```

Quick page host/lifecycle contract：

```text
QuickRuntime.host = routed Quick pages scene root
host switch/destruction -> PageRouter reset
QuickPageProvider missing host -> quick.host-missing
activation callback 可重入，callback 销毁 host/page 后不再访问旧 QQuickItem
QML lifecycle hook 自销毁 root 后不再访问旧 item
parameter setter 同步销毁 page 时停止后续 property write
```

Overlay 已覆盖：

```text
BusyToken release / external-end / destructor
coordinator destruction
OperationHandle normal terminal state -> busy release
OperationHandle direct destruction -> busy release
```

基础控件与 Design System Playground 已完成，不再优先扩张组件数量。

---

## 5. Platform / Integration

Platform 已实现：

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
InputTarget target destruction notify
repeated register/unregister connection ownership closure
HotkeyRouter install target destruction -> installedChanged
HotkeyRouter reinstall 不保留旧 target destruction callback
1000-scan keyboard-wedge stress
```

PlatformQml / Integration：

```text
LfQuick.Platform
ScannerTarget / InputSourceRegistration
LfQuick.Integration
LfScannerTarget -> Lf.acceptScanner + activeFocus -> InputRouter
```

---

## 6. DataView / WidgetBridge / Charts / Diagnostics / Tooling

这些主线功能已基本完成：

```text
DataView
    DataTableModel / DataFilterProxyModel
    DataGrid / Column / Header / Viewport
    persistence / editing / selection / keyboard / typed delegates
    20k/50k benchmark + RSS + DataGridTelemetry

WidgetBridge
    QuickSurface
    WidgetPageProvider / WidgetWindowProvider
    WidgetFocusBridge / WidgetOverlayBackend
    host generation/rebuild
    native owner/lifetime hardening
    resize/DPR/focus stress

Charts
    ChartDataModel / Series model
    Qt Graphs adapter
    Medical Demo Charts page

Diagnostics
    bounded events
    metrics/timing
    JSON snapshot/export
    object/focus/route/accessibility inspectors
    cross-thread DiagnosticsLogSink serialization

Tooling
    QML hot reload candidate-first swap
    rollback / state preservation / shared watch ref-count
    architecture/lint/generator/deploy/screenshot tooling
```

---

## 7. Modern Qt adapters

```text
SafeArea                  >= Qt 6.9
QRangeModel               >= Qt 6.10
QCanvasPainter C++        >= Qt 6.11
QRangeModel drag/drop     >= Qt 6.12
QtCanvas2D QML            >= Qt 6.12
qt_add_qml_preview        >= Qt 6.12
Qt Labs StyleKit          >= Qt 6.12
```

高版本能力均保持独立 feature gate / leaf adapter，不污染 Qt 6.8.3 public API。

Canvas Painter / QtCanvas2D 仍是 Technology Preview，且需要独立 Qt 许可审查，不作为默认产品依赖。

---

## 8. Build linkage contract

当前构建模式明确为：

```text
BUILD_SHARED_LIBS=OFF   # default, complete static LfQuick SDK
BUILD_SHARED_LIBS=ON    # complete shared LfQuick SDK
```

Qt 6.8 使用：

```cmake
qt_policy(SET QTP0003 NEW)
```

因此普通 `add_library()` 与 `qt_add_library()/qt_add_qml_module()` 统一遵循 `BUILD_SHARED_LIBS`。安装阶段还会检查所有公开 SDK library 的 target type，发现 static/shared 混搭直接 configure fail。

静态 SDK 会把 `LFQUICK_STATIC` 作为 INTERFACE compile definition 传播给下游，避免 Windows public headers 错误使用 `Q_DECL_IMPORT`。

---

## 9. Installed CMake package hardening

当前 `find_package(LfQuick)` 已收口：

```text
find_package(LfQuick REQUIRED COMPONENTS ...)
    -> LfQuick_<Component>_FOUND 根据真实 exported target 设置

PUBLIC Qt dependencies
    -> LfQuickConfig.cmake 加载完整对应 Qt components

LfQuick_QT_BASELINE_VERSION
    -> 6.8.3

LfQuick_QT_MIN_VERSION
    -> 根据当前构建实际 feature floor 导出：
       base 6.8.3
       SafeArea 6.9
       QRangeModel 6.10
       CanvasPainter / StyleKit 6.12
```

---

## 10. Installed static QML module hardening

所有当前 `qt_add_qml_module()` backing target 已显式使用 `OUTPUT_TARGETS` 并注册到统一 installer：

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

安装语义：

```text
QML generated OUTPUT_TARGETS -> install + LfQuickTargets export
object-library artifacts -> dedicated install object directory
future duplicate output/plugin target -> deduplicated
static QML plugin -> stable exported target LfQuick::<Component>QmlPlugin
shared QML plugin -> QML module runtime directory
qmldir/typeinfo -> QML install tree
```

---

## 11. Downstream package smoke

CTest 新增 `package_downstream_smoke`，执行：

```text
cmake --install 当前 build -> temporary prefix
独立 consumer -> find_package(LfQuick REQUIRED COMPONENTS ...)
验证所有实际 built component targets
验证 effective Qt version floor
static 验证 LFQUICK_STATIC interface definition
static 验证 stable QML plugin exported targets
consumer 实际 build/link
```

它用于捕获“源码树内能编译，但安装后的 SDK 不能被另一个工程使用”的问题。

注意：测试已经注册到源码/CMake，**当前 HEAD 尚未在真实 Qt 环境执行**。

---

## 12. 当前主要测试覆盖

```text
PageRouter / WindowRouter lifecycle + stress
invalid cache/singleton rebuild
QuickRuntime host switch/destruction
Quick page missing-host + activation reentrancy
BusyToken / OperationHandle lifetime
KeyboardWedge 1000 scan
InputRouter / InputTarget / HotkeyRouter lifetime
PlatformQml / Integration QML smoke
Focus lifetime / scanner isolation / modal trap
DataView QML smoke + benchmark + telemetry
WidgetPage/WidgetWindow host/native lifetime
QuickSurface resize/DPR stress
Diagnostics / AccessibilityAudit / cross-thread log sink
Hot Reload success/rollback/state/shared-watch
QRangeModel optional bridge
Python tooling self-tests
installed package configure/build/link smoke
```

---

## 13. 真实 GUI hardening 待办

必须在真实 Windows desktop 执行：

```text
100% <-> high-DPI multi-screen move
QQuickWidget popup/modal/native z-order
focus restore
native WindowRouter close
scanner isolation
30-60 minute soak
```

见 `tests/manual/WIDGETBRIDGE_HARDENING.md`。

---

## 14. 下一步严格顺序

```text
1. 最后一轮 C++/QML/CMake/API 静态审查
2. README / package/build 文档收口
3. Windows Qt 6.8.3 default static full Configure + Build
4. default static 全量 CTest，重点看 package_downstream_smoke
5. Qt 6.8.3 BUILD_SHARED_LIBS=ON full Configure + Build + CTest
6. Qt 6.10 QRangeModel optional build/test
7. Qt 6.12 StyleKit / Canvas2D optional build/test
8. 真实 GUI hardening
9. deploy/package + lfquick_deploy_check
10. 最后恢复 GitHub Actions / release workflow
```

在 3-9 完成前，准确表述必须保持：

```text
Major functionality implemented: YES
Source-level hardening substantially implemented: YES
Current HEAD full Build verified: NO
Current HEAD full CTest verified: NO
Current HEAD deploy/package verified: NO
CI workflows present: NO (intentional)
CI Green: NO CLAIM
```
