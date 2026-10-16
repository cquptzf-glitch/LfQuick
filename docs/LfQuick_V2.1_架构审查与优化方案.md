# LfQuick V2.1 —— 架构可行性审查、质量评估与优化方案

> 基于 LfQuick V2、现有医疗软件 QML 迁移分析以及 Qt 6.8.3 / Qt 6.12 能力重新审查。

## 1. 总体结论

LfQuick V2 的总体方向可行，且适合作为大型 Qt 医疗/工业上位机的长期 UI Runtime Framework。V2 最大价值在于它已经超越普通 QML 控件库，覆盖 Runtime、Widget/QML 迁移、DataView、Overlay、Input/Focus、Validation/Permission、Diagnostics 与 Tooling。

但正式大规模开发前必须修正以下关键边界：

1. 不把 `createWindowContainer()` 设计成“QML 内嵌 QWidget”的通用方案；
2. `QQuickWidget` 只作为迁移 Host，不作为最终 QML 主架构；
3. `LfQuickCore` 收敛为 QtCore-only；
4. Runtime 使用 `PageProvider`，不直接感知 QWidget/QML；
5. Overlay 按 `UiSurface` 管理，而不是单一全局 z-stack；
6. DataGrid 不把 `Qt.labs.qmlmodels.DelegateChooser` 作为核心依赖；
7. 增加 `ModelRoleAdapter`，不承诺旧 Model 100% 零改造；
8. 新 Charts 默认优先 Qt Graphs；
9. Style 增加 Development Runtime / Production Compile-Time 双模式；
10. Hot Reload、StyleKit、Inspector 明确为开发或实验能力。

综合架构评分：V2 约 8.1/10，按 V2.1 修正后目标约 9.0/10。

---

## 2. 正确的 Widget/QML 迁移拓扑

不追求 QWidget 和 QML SceneGraph 永久任意互嵌。

迁移阶段推荐：

```text
QWidget MainWindow
        │
        ▼
QStackedWidget
 ├── Legacy Widget Page A
 ├── Legacy Widget Page B
 └── QuickSurface
       │
       ▼
   QQuickWidget
       │
       ▼
 QuickHostRoot.qml
       │
       ▼
 PageRouterView
   ├── QML Page A
   ├── QML Page B
   └── QML Page C
```

核心原则：**Single QuickSurface + Single QQmlEngine**。

不要一个 QML 页面创建一个 QQuickWidget。QQuickWidget 存在额外离屏渲染 pass，并会影响 threaded render loop，适合迁移期，不适合作为长期纯 QML 架构。

最终纯 QML：

```text
QQmlApplicationEngine / QQuickWindow
        ↓
ApplicationWindow
        ↓
LfQuick Runtime
        ↓
QML Pages
```

未来可完全关闭：

```cmake
LFQUICK_BUILD_WIDGET_BRIDGE=OFF
```

---

## 3. Core 边界

目标：

```text
LfQuickCore -> Qt6::Core only
```

Core 放：

```text
Result / Error
Command contract
Event contract
Validation contract
Permission contract
Route contract
OperationHandle
Telemetry contract
Common utilities
```

不放：

```text
QQuickItem
QQmlEngine
QQuickAttachedPropertyPropagator
QWidget
Fluent Theme
医疗业务
```

QML-specific attached property、Theme Context、Focus Attached Property 等进入 `LfQuickUi`。

---

## 4. Runtime 改为 PageProvider 架构

不建议：

```cpp
enum class PageTechnology { Qml, Widget };
```

推荐：

```cpp
class IPageProvider
{
public:
    virtual ~IPageProvider() = default;
    virtual QString id() const = 0;
    virtual bool canCreate(const PageDescriptor&) const = 0;
    virtual std::unique_ptr<IPageInstance> create(const PageDescriptor&, const QVariantMap&) = 0;
};
```

实现：

```text
QuickPageProvider
WidgetPageProvider
TestPageProvider
```

Runtime 只管理 Route、Lifecycle、State、Cache、Fallback，不 include QWidget / QQuickItem。

PageDescriptor 使用：

```text
route
provider
source
cachePolicy
fallbackPolicy
metadata
```

---

## 5. Overlay 改为 UiSurface 模型

单一全局 OverlayManager 不能很好支持 Main Window、Dialog Window、第二屏、维护窗口等场景。

推荐：

```text
OverlayCoordinator
        │
        ├── UiSurface A
        │     └── OverlayStack
        ├── UiSurface B
        │     └── OverlayStack
        └── UiSurface C
              └── OverlayStack
```

Backend：

```text
QuickOverlayBackend
WidgetOverlayBackend
WindowOverlayBackend
```

Dialog/Toast/Busy 根据当前 Surface 选择正确 Backend。

混合模式存在 native child window 时，关键 Dialog 优先使用 top-level window，避免页面内 Popup 的 z-order 陷阱。

---

## 6. QQmlEngine 与 Qt 6.8 Initial Property 策略

默认整个应用一个 UI QQmlEngine。

优势：Singleton、Theme、Component Cache、类型注册、内存、Hot Reload 一致。

Qt 6.8.3 下推荐 QQuickWidget 永久只加载：

```text
QuickHostRoot.qml
```

业务页由内部 Loader：

```qml
Loader {
    id: pageLoader
}

function loadPage(url, properties) {
    pageLoader.setSource(url, properties)
}
```

这样可兼容 required property / initial properties，不依赖 Qt 6.9 才新增的 QQuickWidget initial property API。

---

## 7. DataGrid 优化

DataGrid 是 LfQuick 的核心子系统之一。

### 7.1 不以 Qt Labs 为硬依赖

正式 DataGrid 不把 `DelegateChooser` 作为核心架构。

推荐：

```qml
DataGridColumn {
    role: "status"
    delegate: StatusCell {}
    editor: StatusEditor {}
}
```

### 7.2 ModelRoleAdapter

旧 QAbstractTableModel 可能依赖 DisplayRole、column index、自定义 numeric role 或 Widget Delegate，因此不能承诺 100% 零改造。

增加：

```text
ModelRoleAdapter
```

把旧 QModelIndex 语义映射到稳定 QML role 名称。

### 7.3 优先复用 Qt Model 基础设施

优先使用：

```text
QAbstractItemModel
QItemSelectionModel
QSortFilterProxyModel
```

LfQuick 封装语义，而不是重新实现 Qt Model/View 基础。

### 7.4 Column Config 版本化

配置至少包含：

```json
{
  "schemaVersion": 2,
  "tableId": "ResultTable",
  "modelSchema": "ResultModel.v3"
}
```

防止未来列变化导致历史配置错位。

### 7.5 性能矩阵

```text
Rows: 5k / 20k / 50k
Columns: 8 / 20 / 40
Update: 1 / 10 / 100 per sec
```

关注 P95 frame time、memory、scroll latency、update latency、first population。

---

## 8. Charts 技术路线

Qt 6.8 新框架优先：

```text
QtGraphsBackend
```

兼容：

```text
LegacyQCustomPlotBackend
```

特殊性能需求：

```text
CustomQuickBackend
```

Qt 6.12 可选：

```text
CanvasPainterBackend
```

Canvas Painter 是 GPU 路线，不应成为弱 GPU/软件渲染设备唯一 Backend。

QCustomPlot 已成熟的 QC/Calibration/Reaction 页面第一阶段不强制重写。

---

## 9. Style 双模式

Development：Runtime Style Selection，方便 Fluent/Brand/Gallery 验证。

Production：Compile-Time Style Selection，降低部署复杂度并利于 QML 编译优化。

建议：

```cmake
LFQUICK_STYLE_SELECTION=Runtime
# or
LFQUICK_STYLE_SELECTION=CompileTime
```

Theme 的 Light/Dark/Accent/Density/HighContrast 仍可运行时切换。

QSS 只作为旧 Widget 兼容层，不作为未来 Design System 核心。

Token 推荐：

```text
Design Token
 ├── QML Token
 ├── C++ Token
 └── Widget Theme Adapter
      ├── QPalette
      ├── QProxyStyle
      └── Legacy QSS Template
```

---

## 10. StyleKit / Qt 6.12

Qt 6.12 StyleKit 思路与 LfQuick 接近，但仍属于 Qt Labs / Technology Preview，因此不得成为 LfQuick 1.x Public API 基础。

正确关系：

```text
LfQuick Token / Theme Contract
        ↓
Optional StyleKitAdapter
```

而不是 Core 依赖 StyleKit。

---

## 11. Permission / Validation 边界

```qml
Lf.permission: "Result.Audit"
```

只控制 UI 体验，不是真正安全边界。Command / Facade / Domain 在执行关键操作时必须再次校验权限。

Validation 同样分两层：

```text
UI Validation -> 即时提示和输入体验
Domain Validation -> 权威业务校验
```

通用框架通过 `IPermissionProvider` / `IValidationRuleProvider` 接入产品规则，Core 不保存 SampleId、PatientId、ReagentLot 等业务规则。

---

## 12. Input / Scanner / Focus

Scanner 不绑定具体 ScanerHook，而定义：

```text
IInputSource
KeyboardWedgeScanner
HidScanner
SerialScanner
MockScanner
```

通过 InputRouter 路由到 InputTarget。

Scanner Session 应支持 prefix、suffix、timeout、minimumLength、exclusive mode。

FocusWalker 几何排序考虑 FocusGroup、Y tolerance、X/RTL、explicit priority、stable order，而不是简单全 Window y/x 排序。

开发 API：

```text
dumpOrder()
visualize()
```

---

## 13. Page VM 与长期业务任务

Page VM 生命周期不能拥有 20 分钟仪器测试、校准、维护等长期 Operation。

应采用：

```text
Domain Service owns OperationHandle
Page VM observes OperationHandle
```

页面销毁后业务任务仍可继续。

PageStateStore 只保存 scroll position、selected tab、filter、column config、expanded state 等 UI state，不保存领域结果或仪器状态。

---

## 14. FallbackPolicy

QML 加载失败不能对所有页面无条件自动切 Legacy UI。

推荐：

```text
None
ErrorPage
LegacyBeforeActivate
LegacyIfStateless
```

只有明确声明 safe fallback 的无状态页面才自动回退，避免正在编辑/运行关键业务操作时切换 UI 实现导致状态不一致。

---

## 15. Diagnostics 两层设计

Always-On Lightweight Telemetry 放 Runtime：

```text
page load time
route
QML errors
fallback reason
version
basic performance
```

Development Inspector 独立：

```text
FPS HUD
Object Inspector
Focus Visualizer
Overlay Inspector
Route Inspector
```

Release 可以不部署重型 Inspector，但不应完全失去现场诊断数据。

---

## 16. Hot Reload

Hot Reload 必须限定 Debug/Developer Build。

Qt 6.8：LfQuick Development Reload Adapter。

Qt 6.12：优先接入 Qt 原生 QML Hot Reload / qmlpreview，不重复维护复杂自研实现。

从 Qt 6.8 阶段就坚持 Single QQmlEngine、qt_add_qml_module、少用 context property，为 6.12 升级准备。

---

## 17. Deployment

推荐：

```text
Qt official deploy discovery
        +
LfQuick manifest validation
        +
product packaging
```

利用 Qt 的 QML import/deploy 工具发现依赖，再用 LfQuick Manifest 做冻结检查，而不是只维护历史 copy list。

Manifest 记录 Qt Version、LfQuick Version、QML Modules、Plugins、Styles、Translations。

---

## 18. Public QML API 命名

统一：

Attached `Lf.*` 表示当前对象上下文：

```qml
Lf.permission
Lf.validationRule
Lf.focusPolicy
Lf.acceptScanner
Lf.helpContext
```

Singleton `LfQuick.*` 表示全局服务：

```qml
LfQuick.router
LfQuick.toast
LfQuick.theme
```

---

## 19. 推荐依赖图

```text
                    Application
                        │
                        ▼
              Product Integration
                        │
          ┌─────────────┴─────────────┐
          ▼                           ▼
   WidgetBridge                QuickPageProvider
          │                           │
          └─────────────┬─────────────┘
                        ▼
                  LfQuickRuntime
                        │
                ┌───────┴───────┐
                ▼               ▼
           LfQuickCore       LfQuickUi
                                 │
                 ┌───────────────┼───────────────┐
                 ▼               ▼               ▼
              DataView        Charts        Navigation
```

硬边界：

```text
Core -> QtCore only
Runtime -> Core
Quick/UI -> Core + Runtime + QtQuick
DataView -> Core + Quick/UI
WidgetBridge -> Runtime + QtWidgets + QuickWidgets
Product Integration -> LfQuick + legacy product libs
```

禁止 Core -> QtQuick/Widgets、Runtime -> QWidget、UI -> LfWidget、DataGrid -> Qt.labs 作为硬依赖。

---

## 20. V2.1 推荐实施顺序

```text
P0 Technical PoC
   Single QuickSurface
   DataGrid
   Overlay
   Focus
   Scanner
   QtGraphs
   Deployment

P1 Core / Runtime

P2 Quick Host / Single Engine

P3 Overlay / Input / Focus

P4 Design System / Base Controls

P5 DataGrid

P6 Real application pages

P7 Navigation / Window

P8 Visualization / Charts

P9 Tooling / Qt 6.12 Adapter
```

大规模业务迁移前必须完成 QuickSurface、DataGrid、Focus、Scanner、Overlay 和 Performance 的 Go/No-Go 验证。

---

## 21. 最终结论

LfQuick 不应让 Widget、QML、QQuickWidget、Native Window 永久处于完全平级和任意互嵌状态。

更健康的演进：

```text
Current QWidget Product
        ↓
Widget Shell + Single QuickSurface
        ↓
Hybrid Product
        ↓
Mostly QML
        ↓
Pure QML Runtime
+
optional legacy top-level tools
```

迁移机制是桥梁，QML Runtime 才是长期终态。只要按 V2.1 收紧边界、坚持 PoC Gate 与真实 Medical Demo 双向验证，LfQuick 具备成为长期高质量 Qt UI Runtime Framework 的可行性。
