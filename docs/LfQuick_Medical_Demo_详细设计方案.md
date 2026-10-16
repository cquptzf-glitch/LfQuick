# LfQuick Medical Demo 详细设计方案

> **项目名称**：LfQuick Medical Demo  
> **定位**：LfQuick 官方示例程序 / 医疗器械生化 + 免疫分析仪 UI 样板工程  
> **目标 Qt 基线**：Qt 6.8.3  
> **目标兼容版本**：Qt 6.12 LTS  
> **用途**：框架验证、组件验证、架构示范、UI/交互样板、性能基线、医疗器械业务页面迁移参考  
> **Demo 属性**：纯模拟数据，不接真实患者数据、不连接真实仪器、不执行真实诊断流程  
> **依赖文档**：LfQuick V2.1 架构方案、LfQuick 详细开发计划

---

# 1. Demo 的定位

LfQuick Medical Demo 不是“控件 Gallery”，也不是简单的设计稿展示。

它承担四个角色：

```text
1. LfQuick Framework Verification App
2. Medical UI Reference Application
3. Widget → QML Migration Reference
4. Performance / Interaction Benchmark
```

Demo 必须以接近真实生化/免疫分析仪上位机的页面结构和数据复杂度验证框架。

---

# 2. Demo 设计目标

## 2.1 验证框架能力

必须覆盖：

```text
Runtime
Routing
Lifecycle
QuickSurface
WidgetBridge
Theme
Style
Overlay
Dialog
Toast
Busy
Focus
Scanner
Permission
Validation
DataGrid
Charts
Windowing
i18n
Diagnostics
```

## 2.2 验证医疗器械典型 UI

Demo 至少应覆盖：

```text
工作台
样本
试剂
质控
校准
检测结果
设备状态
报警
维护
耗材
统计
系统设置
用户权限
日志
```

## 2.3 验证高风险能力

必须重点覆盖：

```text
复杂 DataGrid
高频结果更新
曲线
扫码枪
触摸屏
多语言
弹层
告警
Focus
长时间运行
```

---

# 3. Demo 与业务产品的边界

Demo 不直接进入：

```text
LfQuickCore
LfQuickUi
LfQuickRuntime
```

推荐结构：

```text
examples/
└── medical-demo/
```

Demo 可以使用产品级扩展模块：

```text
MedicalDemo.Domain
MedicalDemo.Mock
MedicalDemo.Components
```

这些模块不属于通用 LfQuick SDK。

---

# 4. 总体架构

```text
Medical Demo Application
        │
        ▼
Demo ViewModels / Commands
        │
        ▼
Mock Domain / Simulator
        │
        ▼
LfQuick Runtime / UI / DataView / Charts
```

Mock 层包含：

```text
SampleSimulator
ReagentSimulator
InstrumentSimulator
QcSimulator
CalibrationSimulator
ResultSimulator
AlarmSimulator
ConsumableSimulator
```

---

# 5. Demo 运行模式

提供三种运行模式。

## 5.1 Pure QML

```text
QQmlApplicationEngine
+
QQuickWindow
```

用于验证未来最终架构。

## 5.2 Hybrid

```text
QWidget MainWindow
+
Single QQuickWidget QuickSurface
```

用于验证现有医疗项目迁移。

## 5.3 Legacy Comparison

同一页面同时提供 QML 与 Widget mock implementation，用于对比 Focus、Layout、Table、Performance 与 Interaction。

---

# 6. 启动参数

```bash
LfQuickMedicalDemo
    --mode quick
    --mode hybrid
    --theme light
    --theme dark
    --density touch
    --locale zh_CN
    --scenario normal
    --scenario alarm
    --scenario stress
```

---

# 7. Demo 主界面布局

```text
┌─────────────────────────────────────────────────────────┐
│ App Bar                                                 │
│ Logo | Instrument | User | Network | Time | Alarm       │
├─────────────┬───────────────────────────────────────────┤
│ Navigation  │                                           │
│ Dashboard   │                Page Content               │
│ Samples     │                                           │
│ Reagents    │                                           │
│ QC          │                                           │
│ Calibration │                                           │
│ Results     │                                           │
│ Instrument  │                                           │
│ Maintenance │                                           │
│ Settings    │                                           │
├─────────────┴───────────────────────────────────────────┤
│ Status Bar / Instrument Running State / Progress        │
└─────────────────────────────────────────────────────────┘
```

---

# 8. 一级导航

```text
工作台       /dashboard
样本         /samples
试剂         /reagents
质控         /qc
校准         /calibration
结果         /results
设备         /instrument
报警         /alarms
耗材         /consumables
维护         /maintenance
统计         /statistics
设置         /settings
```

---

# 9. Dashboard 工作台

用于验证 Card、StatusIndicator、Charts、Responsive Layout、Navigation 与 Live Data。

页面包含：

```text
Instrument Summary
Running State
Sample Throughput
Reagent Warning
QC Status
Recent Alarm
Daily Statistics
```

仪器状态：Ready / Running / Maintenance / Error / Offline。

今日统计：样本数、测试数、完成数、异常数、复检数。

当前运行进度显示 Current Batch、Completed / Total、ETA，并验证 ProgressBar、Busy 和 Live Binding。

实时吞吐趋势：X = Time，Y = Test Count / Minute，验证 QtGraphsBackend。

---

# 10. Samples 样本模块

页面：

```text
/samples
/samples/detail
/samples/register
/samples/rack
```

这是 Demo 的第一个核心模块。

---

# 11. 样本列表

DataGrid Columns：

```text
Select
Sample ID
Patient Name (mock)
Sample Type
Rack
Position
Priority
Status
Test Count
Created Time
```

Demo 中 Patient Name 仅使用虚拟数据。

DataGrid 必须验证：

```text
10000+ rows
multi selection
sort
filter
column resize
column reorder
column visibility
persistent config
checkbox
icon
status delegate
keyboard
double click
context menu
```

样本状态：Pending / Loading / Running / Completed / Failed / Retest。

Toolbar：新增、批量删除、开始测试、暂停、复检、导出，验证 Permission、Command、Busy、MessageBox。

---

# 12. 样本登记页面

字段：

```text
Sample ID
Barcode
Sample Type
Rack
Position
Patient Gender (mock)
Age
Emergency
Requested Tests
```

验证 required、range、regex、max length、duplicate ID。

扫码字段：

```qml
TextField {
    Lf.acceptScanner: true
}
```

测试 keyboard wedge、scan timeout、duplicate scan、scan while dialog opened。

---

# 13. Sample Rack 样本架

组件：RackView、SlotView、SampleTube。

状态：Empty / Loaded / Running / Completed / Error。

---

# 14. Reagents 试剂模块

页面：

```text
/reagents
/reagents/detail
/reagents/load
```

列表字段：Reagent、Assay、Position、Lot、Remaining、Remaining Tests、Expiration、Calibration Status、QC Status、State。

使用 BottleView、LevelIndicator、ProgressBar 表示 Normal / Low / Critical / Empty / Expired。

详情展示 Basic Info、Usage、Calibration、QC、Consumption Trend。

---

# 15. QC 质控模块

页面：

```text
/qc
/qc/run
/qc/detail
/qc/lj
```

QC DataGrid：QC Name、Assay、Level、Lot、Target、SD、Result、Z Score、Rule、Status、Time。

Levey-Jennings Chart 必须包含：Mean、±1SD、±2SD、±3SD、QC points、Rule violation、hover tooltip、zoom、pan、range selection。

Westgard Rule 只做 UI 模拟：1-2s、1-3s、2-2s、R-4s，不作为真实医疗判定算法。

---

# 16. Calibration 校准模块

页面：

```text
/calibration
/calibration/run
/calibration/detail
```

列表：Assay、Reagent Lot、Calibration Lot、Type、Last Calibration、Status、Slope、Intercept。

校准曲线：X = Concentration，Y = Signal，支持 scatter points、fitted curve、tooltip、zoom、parameter panel。

Calibration Wizard：Select assay → Select reagent → Select calibrator → Confirm → Running → Result，验证 StepNavigator、Busy、Progress、Dialog。

---

# 17. Results 结果模块

页面：

```text
/results
/results/detail
/results/review
```

Results Table：Sample ID、Assay、Result、Unit、Reference Range、Flag、Status、Time、Operator。

状态：Normal / High / Low / Critical / Error / Pending Review / Reviewed。

详情展示 Sample、Assay、Result、Reference、Reaction Curve、History、Audit Trail。

Reaction Curve 模拟 Absorbance / Signal vs Time，支持 multiple series、cursor、tooltip、zoom、marker、range。

Review Workflow：Approve、Reject、Retest、Comment，并验证 Permission、Command、Dialog、Validation、Audit display。

---

# 18. Instrument 设备模块

页面：

```text
/instrument
/instrument/modules
/instrument/process
```

生化模块模拟：Sample Module、Reaction Module、Reagent Module、Washing Module、ISE Module、Temperature、Waste、Water。

免疫模块模拟：Sample Handling、Reagent Carousel、Incubation、Magnetic Separation、Detection、Waste、Consumables。

Device Topology View 使用 ModuleCard、ConnectionIndicator、StateLight、Gauge、ProcessSteps。

Process View 演示 Load → Aspirate → React → Measure → Complete，状态 active / waiting / completed / error。

---

# 19. Alarms 报警模块

Alarm List：Time、Severity、Module、Code、Message、State、Acknowledged By。

Severity：Info、Warning、Error、Critical。

硬规则：Critical Alarm != Toast。

Critical Alert Presenter 支持 persistent、requires acknowledgement、optional sound state、history。

LfQuick 只提供 UI Presenter，业务 Alarm Service 由 Demo Mock 实现。

---

# 20. Consumables 耗材

显示 Wash Solution、Waste、Reaction Cups、Pipette Tips、Substrate、Cleaner。

组件：BottleView、LevelIndicator、Gauge、StatusBadge。

---

# 21. Maintenance 维护

页面：

```text
/maintenance
/maintenance/tasks
/maintenance/manual
```

任务：Daily Maintenance、Weekly Maintenance、Wash、Prime、Drain、Probe Cleaning。

Maintenance Wizard 使用 StepNavigator、OperationHandle、Progress、Busy、Cancel，专门验证 Page lifetime != Operation lifetime。

---

# 22. Statistics 统计

展示 Daily Sample Count、Assay Distribution、Error Trend、Reagent Usage。

---

# 23. Settings 设置

一级设置：General、Language、Display、Instrument、Tests、Network、Users、Diagnostics。

语言支持：zh_CN、en_US、ru_RU、es_ES、Pseudo，并通过 QQmlEngine::retranslate() 切换。

Display 支持 Light / Dark、Standard / Touch Density、Animation、Performance Profile。

---

# 24. Permission Demo

模拟角色：Operator、Senior Operator、Engineer、Administrator。

Operator 不能编辑校准和进入工程维护；Engineer 可维护但不能用户管理；Administrator 为完整 UI permission。

QML 示例：

```qml
Button {
    Lf.permission: "Sample.Delete"
}
```

Mock PermissionProvider 提供 UI 状态，但 Command 执行端仍需再次校验。

---

# 25. Validation Demo

独立页面 `/settings/validation-demo`，演示 required、regex、range、length、precision、async validation。

```qml
TextField {
    Lf.validationRule: "Sample.Barcode"
}
```

Demo Provider 定义 Sample.Barcode、Sample.Id、Reagent.Lot、QC.Value。

---

# 26. Diagnostics / Developer Playground

开发版提供 `/diagnostics` 和 `/developer`。

显示：FPS、P95 Frame、Current Route、Page Load、QML Objects、Overlay Count、Focus Order、Runtime Backend、Qt Version、LfQuick Version。

Playground 模块：Controls、DataGrid、Charts、Overlay、Focus、Scanner、Theme、i18n。

---

# 27. Demo 导航结构

```text
Dashboard
Samples
 ├── List
 ├── Register
 └── Rack
Reagents
 ├── List
 └── Detail
QC
 ├── Overview
 ├── Data
 └── LJ Chart
Calibration
 ├── Overview
 └── Wizard
Results
 ├── List
 ├── Detail
 └── Review
Instrument
 ├── Overview
 ├── Modules
 └── Process
Alarms
Consumables
Maintenance
 ├── Tasks
 └── Manual
Statistics
Settings
 ├── General
 ├── Language
 ├── Display
 ├── Users
 └── Diagnostics
```

---

# 28. QML 与 C++ 目录

```text
examples/medical-demo/
├── CMakeLists.txt
├── README.md
├── cpp/
│   ├── main.cpp
│   ├── app/
│   ├── viewmodels/
│   ├── commands/
│   ├── models/
│   ├── mock/
│   └── scenarios/
├── qml/
│   ├── App.qml
│   ├── shell/
│   ├── pages/
│   ├── components/
│   └── developer/
├── translations/
├── assets/
└── tests/
    ├── unit/
    ├── qml/
    ├── integration/
    ├── visual/
    └── performance/
```

---

# 29. Demo Scenario System

提供：Normal、BusyDay、LowReagent、QCFail、CalibrationExpired、InstrumentError、AlarmStorm、Stress。

```cpp
class IDemoScenario
{
public:
    virtual QString id() const = 0;
    virtual void activate() = 0;
    virtual void reset() = 0;
};
```

开发版 AppBar 提供 Scenario Selector。

---

# 30. ViewModel 规范

每个 Page 使用：Page.qml + PageVm + PageModel + Commands。

例如：SamplesPage.qml、SamplesPageVm、SampleTableModel、StartSampleCommand、DeleteSampleCommand。

数据模型优先使用 QAbstractListModel、QAbstractTableModel、QSortFilterProxyModel、QItemSelectionModel，不为大数据场景创建纯 JS 模型。

---

# 31. OperationHandle

Calibration、Maintenance、Batch Run 等长操作返回 OperationHandle，包含 state、progress、message、cancelable。

长任务由 Domain/Service 持有，Page VM 只观察，避免页面销毁导致任务消失。

---

# 32. Focus / Scanner / Overlay Stress

Focus 测试页提供 20 / 50 / 100 输入控件，展示 focus order、active focus、group。

Scanner 模拟 SCAN000001...，支持 10~100ms 输入节奏。

Overlay Scenario：Toast + Dialog、Dialog + Busy、Critical + Busy、Nested Dialog，验证 z order、focus、modal、click through。

---

# 33. DataGrid Stress

页面 `/developer/datagrid-stress`：

```text
Rows 5k / 20k / 50k
Columns 8 / 20 / 40
Update Rate 1 / 10 / 100
```

记录 P95 frame、scroll latency、update latency、memory、first population。

---

# 34. Chart Stress

支持 1 / 4 / 16 series，1k / 10k / 100k points，记录 FPS、memory、update latency。

---

# 35. Theme / Brand 验证

同页展示 Button、Input、Table、Status、Chart、Dialog，切换 Light、Dark、Touch、Compact。

Demo 必须能在 LfQuickFluent 与 LfQuickBrand 下运行，业务页面禁止依赖 Fluent-specific API。

---

# 36. 分辨率与输入模式

至少验证：1366×768、1920×1080、2560×1440；100%、125%、150% DPI。

Touch Density：Button/Input/Row 建议 ≥44px，并支持 touch scroll、long press、large hit target。

Keyboard：Tab、Shift+Tab、Enter、Esc、Arrow、Ctrl+A、F1、F2。

---

# 37. i18n 验收

主要页面在 Chinese、English、Russian、Spanish、Pseudo 下截图，验证长文本与布局溢出。

---

# 38. WidgetBridge Demo

Hybrid Mode 中 Navigation 可以 Widget Page → QML Page → Widget Page。

Legacy Comparison 页面可将 Widget Table 与 DataGrid 并排比较 selection、keyboard、editing、column、scroll。

---

# 39. Demo 数据安全

硬规则：

```text
不读取真实数据库
不读取真实患者数据
不访问真实仪器
不连接医疗设备
```

所有 Patient、QC、Calibration、Result 数据均为 synthetic only，不得描述为临床有效算法。

---

# 40. Demo 视觉原则

现代、简洁、高信息密度、可触摸、高可读性；避免过度动画、透明模糊滥用、低对比度和消费级娱乐视觉。

Info / Success / Warning / Error / Critical 必须通过 Semantic Token 表达，业务禁止直接写 `color: "red"`。

---

# 41. Demo Components

建议新增 `MedicalDemo.Components`：StatisticCard、InstrumentModuleCard、SampleStatusCell、ReagentLevelCell、QcStatusCell、AlarmSeverityBadge、OperationProgressCard。

可进入 LfQuick 的通用组件：StatusIndicator、BottleView、LevelIndicator、Gauge、ProcessSteps。

只能留 Demo/Product 的组件：SampleTube、ReagentCard、QcSummary、InstrumentModule。

---

# 42. Demo 开发阶段

```text
D0 Skeleton
Application shell / Mock domain / Navigation / Theme / Route

D1 Interaction Baseline
Samples / Scanner / Focus / Dialog / Toast / Busy

D2 DataGrid Validation
Samples / Results / Reagents

D3 Charts
Dashboard / QC LJ / Calibration / Reaction Curve

D4 Instrument UI
Device modules / Rack / Consumables / Maintenance

D5 Production Validation
i18n / touch / performance / 72h / Qt 6.12
```

---

# 43. Demo 与 LfQuick 开发计划映射

```text
LfQuick S0 → Demo skeleton
LfQuick S2 → Demo route/page lifecycle
LfQuick S3 → Hybrid mode
LfQuick S4 → Sample register + scanner + overlay
LfQuick S5 → Dashboard + base pages
LfQuick S6 → Samples / Results / Reagents
LfQuick S7 → Full navigation / window
LfQuick S8 → QC / Calibration / Reaction
LfQuick S9 → Developer diagnostics
```

---

# 44. 双向驱动模式

LfQuick 和 Demo 不应“框架全部开发完再写 Demo”，而应保持 Framework Feature ↔ Medical Demo Scenario 双向驱动。

每完成一个框架模块立即在 Demo 使用；如果 Demo 使用困难，优先调整 Framework API。

---

# 45. 第一批 Demo Issues

```text
Demo #1  Bootstrap medical demo application
Demo #2  Add mock instrument simulator
Demo #3  Implement app shell
Demo #4  Implement dashboard
Demo #5  Implement sample table model
Demo #6  Implement samples page
Demo #7  Implement sample registration
Demo #8  Add scanner scenario
Demo #9  Implement results page
Demo #10 Implement instrument status page
Demo #11 Implement alarms page
Demo #12 Add DataGrid stress page
Demo #13 Add focus inspector page
Demo #14 Add overlay playground
Demo #15 Implement reagents page
Demo #16 Implement QC LJ chart
Demo #17 Implement calibration chart
Demo #18 Implement reaction curve
Demo #19 Implement maintenance operation
Demo #20 Add language stress tests
```

---

# 46. Demo 验收标准

功能：12 个一级模块可导航、主要 Command 可模拟执行、Mock 数据实时变化。

DataGrid：50k rows stress 可运行，selection/edit/filter/sort/config 可工作。

Charts：Dashboard/QC/Calibration/Reaction 使用同一 Charts abstraction。

Input：scanner、keyboard、touch、focus 均可演示。

Overlay：Dialog、Toast、Busy、Critical Alert 层级正确。

Theme：Light、Dark、Touch、Compact 正常。

i18n：zh/en/ru/es/pseudo 布局无严重溢出。

Performance：普通页面首帧目标 <200ms，切页无明显白屏，持续运行无明显内存增长。

---

# 47. Demo 完成定义

第一阶段不是以页面数量衡量，而是证明 LfQuick 可以支撑真实大型医疗上位机 UI。

最低证明项：

```text
1 个 Dashboard
1 个复杂 DataGrid
1 个扫码输入流程
1 个复杂 Focus 页面
1 个 Overlay 场景
1 个 QC Chart
1 个 Calibration Chart
1 个 Device Status
1 个 Long Operation
1 个 Hybrid Widget/QML 页面
```

---

# 48. Demo 首页状态

首页顶部显示：LfQuick Medical Demo、Qt 6.8.3、LfQuick 0.x、Mode、Scenario、Instrument State。

开发版允许快速切 Theme、Density、Locale、Scenario、Performance Profile。

---

# 49. README 安全声明

Demo README 必须明确：

```text
This is NOT medical diagnostic software.
All data is synthetic.
The application is a UI/framework demonstration.
```

---

# 50. 最终价值与近期实施顺序

Medical Demo 的核心价值：证明 LfQuick 不是纸面架构；给业务页面提供复制模板；长期作为框架回归测试应用。

近期顺序：

```text
D0.1 创建 examples/medical-demo
D0.2 AppShell + Navigation
D0.3 MockInstrumentService
D0.4 Dashboard
D1.1 SampleTableModel
D1.2 SamplesPage
D1.3 SampleRegisterPage
D1.4 Scanner Simulation
D1.5 Focus Test
D1.6 Overlay Test
D2   DataGrid Stress
D3   Results + QC + Calibration
D4   Instrument + Maintenance
```

第一轮优先完成 Dashboard、Samples、Results、Instrument、Alarms、Developer 六个页面群，这六个页面足以验证 LfQuick 最核心的架构能力。
