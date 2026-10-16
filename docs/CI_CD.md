# LfQuick CI/CD

> 当前目标平台：Windows x64 / Qt 6.8.3 / MSVC2022 / Release。

## 1. CI

Workflow：`.github/workflows/build.yml`

触发条件：

- push 到 `main`；
- pull request；
- `workflow_dispatch` 手动触发。

执行链路：

```text
checkout
-> install Qt 6.8.3 win64_msvc2022_64
-> tools/check_architecture.py
-> CMake Configure
-> Build
-> CTest
-> cmake --install
-> Qt QML deployment
-> package layout validation
-> upload artifact
```

构建选项：

```text
BUILD_TESTING=ON
LFQUICK_BUILD_WIDGET_BRIDGE=ON
LFQUICK_BUILD_HYBRID_DEMO=ON
LFQUICK_BUILD_MEDICAL_DEMO=ON
LFQUICK_BUILD_BENCHMARKS=OFF
```

成功后 artifact 名称：

```text
LfQuick-Demos-Windows-x64-<commit-sha>
```

默认保留 14 天。

## 2. Demo deployment

两个 Demo 都使用 Qt 官方 CMake deployment API：

```cmake
install(TARGETS ... RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
qt_generate_deploy_qml_app_script(...)
install(SCRIPT ...)
```

目标可执行文件：

```text
bin/LfQuickMedicalDemo.exe
bin/LfQuickHybridDemo.exe
```

CI 会额外加入：

```text
README.md
THIRD_PARTY_NOTICES.md
BUILD_INFO.txt
```

不通过手写固定 Qt DLL 列表打包，避免 Qt/QML plugin 漏部署和升级版本后清单失效。

## 3. Release

Workflow：`.github/workflows/release.yml`

### Tag release

推送 `v*` tag，例如：

```bash
git tag v0.1.0
git push origin v0.1.0
```

会执行完整 Build + Test + Deploy，并生成：

```text
LfQuick-Demos-Windows-x64-v0.1.0.zip
```

随后通过 GitHub Release 发布 zip，并自动生成 release notes。

### Manual package

`release-demos` 支持 `workflow_dispatch`。

手动运行时会构建并上传 zip artifact，但只有 tag run 才发布 GitHub Release，避免误把普通 main 快照变成正式 Release。

## 4. Local reproduction

在安装 Qt 6.8.3 MSVC2022、CMake 和 Ninja 的 Windows 环境中：

```powershell
cmake -S . -B build -G Ninja `
  -DCMAKE_BUILD_TYPE=Release `
  -DBUILD_TESTING=ON `
  -DLFQUICK_BUILD_WIDGET_BRIDGE=ON `
  -DLFQUICK_BUILD_HYBRID_DEMO=ON `
  -DLFQUICK_BUILD_MEDICAL_DEMO=ON `
  -DLFQUICK_BUILD_BENCHMARKS=OFF

cmake --build build --parallel
ctest --test-dir build --output-on-failure -C Release
cmake --install build --config Release --prefix package/LfQuick-Demos-Windows-x64
```

安装目录应至少包含：

```text
package/LfQuick-Demos-Windows-x64/bin/LfQuickMedicalDemo.exe
package/LfQuick-Demos-Windows-x64/bin/LfQuickHybridDemo.exe
```

## 5. 当前验证状态

截至 2026-10-16，GitHub Actions 的 Windows job 仍存在 runner/infrastructure 异常：job 创建后约数秒失败，`steps=null`，没有 checkout / Configure / Build / Test 日志。

因此：

```text
CI/CD workflow source: implemented
Demo deployment CMake source: implemented
Real Qt 6.8.3 + MSVC2022 CI build: NOT EXECUTED
Artifact produced: NOT VERIFIED
GitHub Release produced: NOT VERIFIED
CI Green: NO CLAIM
```

Runner 一旦恢复并出现真实 steps，优先处理任何 Configure / Build / CTest / deploy/package 错误，成功后再创建正式 `v*` tag。
