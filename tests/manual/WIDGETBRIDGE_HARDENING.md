# WidgetBridge / QuickSurface Manual Hardening Checklist

> 目的：覆盖自动化 GUI 测试不适合稳定模拟的 multi-DPI、跨屏、native z-order、modal/native child 组合。  
> 基线：Qt 6.8.3，优先 Windows 10/11 + MSVC2022。  
> 推荐入口：`LfQuickHybridDemo`。

## 1. 准备

建议显示器组合至少包含两种缩放，例如：

```text
Screen A: 100%
Screen B: 150% / 175% / 200%
```

记录 QuickSurface 暴露的指标：

```text
effectiveDevicePixelRatio
surfaceSize
screenName
resizeCount
loadDurationMs
firstFrameMs
```

### 通过条件

- 不崩溃；
- 不出现持续黑屏；
- QML root 始终跟随 QuickSurface 尺寸；
- 跨屏后 devicePixelRatio / screenName 能更新；
- 焦点可从 QWidget 正常进入/离开 QML；
- modal 打开时焦点不会逃出 modal；
- native/top-level window 关闭仍由 WindowRouter authoritative path 管理。

---

# 2. 单屏 resize stress

1. 启动 Hybrid Demo；
2. 确认 QML Page 已 Ready；
3. 连续拖动主窗口四边与四角至少 30 秒；
4. 最小化/恢复 10 次；
5. 最大化/还原 10 次；
6. 观察 `surfaceSize`、`resizeCount`。

通过：

```text
rootObject size == QuickSurface size
resizeCount 单调增加
无黑帧长期停留
无 QML root 溢出/裁剪异常
```

---

# 3. Multi-DPI / cross-screen

1. 主窗口从 100% 屏拖到高 DPI 屏；
2. 完全进入高 DPI 屏后停留；
3. 再拖回；
4. 重复 20 次；
5. 每次跨屏后进行 resize；
6. 切换 QML Page / Legacy QWidget Page / Back。

重点观察：

```text
screenName
effectiveDevicePixelRatio
surfaceSize
focus transfer
```

失败判据：

- DPR 不随屏幕变化；
- QML 逻辑尺寸与 QWidget 明显错位；
- 鼠标 hit-test 偏移；
- TextField caret 与点击位置明显错位；
- 切页后 QML 缩放突然跳回旧屏 DPR；
- focus transfer 失效。

---

# 4. 系统缩放运行时改变

Windows：

1. 应用保持运行；
2. Settings -> Display 修改当前屏缩放；
3. 切回应用；
4. resize 主窗口；
5. 检查 DPR 与布局。

注意：Windows 对部分缩放变更可能要求注销后才完全生效，因此需记录操作系统行为，不把 OS 限制误判为框架缺陷。

---

# 5. QWidget / QQuickWidget stacking

构造/观察以下组合：

```text
QWidget sibling
QQuickWidget / QuickSurface
QMenu / QComboBox popup
QToolTip
QMessageBox non-blocking
WindowRouter top-level QWidget
```

操作：

1. 打开 QML page；
2. 连续打开 Toast / Dialog；
3. 打开 QWidget popup/menu；
4. 打开 Inspector Window / Owned Tool Window；
5. 在窗口之间 Alt+Tab；
6. 改变主窗口 z-order；
7. 重复关闭/重新打开。

通过：

- popup 不被 QuickSurface 错误遮挡；
- modal/dialog 不被 QML 内容盖住；
- QML 不错误覆盖独立 top-level QWidget；
- 返回主窗口后 QuickSurface 正常刷新。

---

# 6. Focus roundtrip

自动 stress 已覆盖 30 轮，人工再验证真实输入法/键盘路径：

```text
QWidget before
Tab
QML TextField
Tab / Shift+Tab
QWidget after
```

至少重复 20 轮。

同时验证：

- scanner isolation ON：不发生 focus transfer；
- scanner isolation OFF：恢复 transfer；
- modal dialog 打开：focus 不越界；
- dialog 关闭：原 focus chain 可继续使用。

---

# 7. WindowRouter native close path

1. 打开 Inspector Window；
2. 再打开多个 Owned Tool Window；
3. 打开 child 的 close guard；
4. 点击 owner 原生 ×；
5. 确认 close 被阻止；
6. 清除 guard；
7. 再关闭 owner；
8. 验证 CloseWithOwner child 一并关闭；
9. 重复 single-instance Inspector 打开，确认 reuse。

重点检查：

```text
window count
instance id
close blocked reason
owner close propagation
```

---

# 8. QuickSurface load / first-frame

冷启动至少 10 次，记录：

```text
loadDurationMs
firstFrameMs
```

不要在此阶段写绝对性能门槛；先收集分布作为回归基线。

异常：

- `Ready` 但长期没有 firstFrame；
- repeated load 后 firstFrame 不再更新；
- error 后下一次有效 source 无法恢复。

---

# 9. Long-run soak

建议 30–60 分钟：

循环：

```text
QML page
-> legacy page
-> fallback page
-> back
-> open/close inspector
-> open/close owned child
-> resize
-> focus transfer
```

每 5 分钟导出 Diagnostics JSON。

观察：

- event bounded history 是否稳定；
- droppedCount 是否符合 maxEvents；
- window count 是否回到 0；
- QuickSurface load failure count 是否异常增长；
- RSS 是否持续无界增长。

---

# 10. 记录模板

```text
OS:
Qt version:
GPU:
Qt rendering backend:
Screen A resolution / scale:
Screen B resolution / scale:

Initial screenName:
Initial DPR:
Cross-screen DPR:

QuickSurface loadDurationMs:
QuickSurface firstFrameMs:
Resize count:

Focus roundtrip: PASS/FAIL
Scanner isolation: PASS/FAIL
Modal trap: PASS/FAIL
Cross-DPI layout: PASS/FAIL
Popup stacking: PASS/FAIL
WindowRouter native close: PASS/FAIL
Long-run soak: PASS/FAIL

Notes:
```

---

# 11. 与自动测试的边界

自动测试负责：

- deterministic contract；
- focus policy；
- repeated resize；
- state transitions；
- cache / routing / diagnostics / tooling semantics。

人工测试负责：

- compositor / DWM；
- OS DPI transition；
- GPU/backend；
- native popup/z-order；
- 多显示器物理行为。

不要为了“全自动”而用脆弱模拟替代真实系统行为。
