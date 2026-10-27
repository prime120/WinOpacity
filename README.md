<p align="center">
  <img width="120" src="https://img.icons8.com/fluency/240/opacity.png" alt="WinOpacity">
</p>

<h1 align="center">WinOpacity</h1>

<p align="center">
  <b>极简 Windows 窗口透明度工具</b><br>
  按住 <kbd>Alt</kbd> 滚动滚轮，随时让任意窗口变透明，透出底层壁纸
</p>

<p align="center">
  <a href="#license"><img src="https://img.shields.io/badge/License-MIT-blue.svg" alt="License: MIT"></a>
  <img src="https://img.shields.io/badge/Platform-Windows%2011%20x64-0078D6.svg" alt="Platform">
  <img src="https://img.shields.io/badge/C%2B%2B-17-659AD2.svg" alt="C++17">
  <img src="https://img.shields.io/badge/No%20dependencies-pure%20Win32-green.svg" alt="No dependencies">
</p>

---

## 这是什么

一个小巧的常驻托盘工具：把鼠标移到任意窗口上，按住 **Alt** 滚动滚轮，就能实时改变它的透明度。窗口变透明后，底层的壁纸（或其他桌面内容）会透出来，而窗口本身仍然可以正常点击、输入、操作。

功能单一、无主界面、零第三方依赖，空闲时不占 CPU。

## 功能特性

- 🎯 **指向即调**：鼠标指向哪个窗口，就调哪个窗口，无需先选中。
- 🎚️ **两级步长**：`Alt + 滚轮` 大步调，`Alt + Shift + 滚轮` 精细微调。
- ↩️ **一键还原**：`Ctrl + Alt + 滚轮` 恢复目标窗口完全不透明。
- 🛡️ **安全边界**：透明度下限保护，避免窗口意外调到完全不可见。
- 🖱️ **不破坏点击**：只改透明度，不启用鼠标点击穿透。
- 🚪 **退出即还原**：退出程序时自动恢复所有被它修改过的窗口。
- 🪶 **轻量**：消息驱动，零轮询、零定时器，空闲 CPU 占用接近 0。
- 🚫 **自动避让**：跳过自身窗口、桌面、任务栏、系统菜单等系统窗口。

## 快速开始

1. 从 [Releases](../../releases) 下载最新版 `WinOpacity.exe`（或按下方「编译」自行构建）。
2. 双击运行，**无需安装**。程序没有主窗口，只在系统托盘出现一个图标。
3. 鼠标移到目标窗口上，按住 **Alt** 滚动滚轮即可。

就这么简单。

## 使用方法

| 操作 | 效果 |
|---|---|
| 鼠标指向窗口 + `Alt` + 滚轮向上 | 更不透明 |
| 鼠标指向窗口 + `Alt` + 滚轮向下 | 更透明 |
| 鼠标指向窗口 + `Alt + Shift` + 滚轮 | 小步长精细调整 |
| 鼠标指向窗口 + `Ctrl + Alt` + 滚轮 | 恢复完全不透明 |
| 右键托盘图标 → 退出 | 退出并恢复所有被修改过的窗口 |

> 💡 提示：滚轮方向对应关系 —— 向上 = 增加透明度值（更不透明），向下 = 减小透明度值（更透明）。

## 编译

> 需要 Windows、C++17 编译器、CMake ≥ 3.15。

```bat
git clone https://github.com/<你的用户名>/WinOpacity.git
cd WinOpacity
```

**MSVC（Visual Studio 2022）**

```bat
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

**MinGW-w64**

```bat
cmake -S . -B build-mingw -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build build-mingw
```

## 配置（可选）

在 `WinOpacity.exe` 同目录创建 `WinOpacity.ini`，可自定义步长与上下限：

```ini
[Opacity]
CoarseStep=64   ; 普通步长
FineStep=8      ; 精细步长
MinAlpha=32     ; 透明度下限（1~255，越大越不透明）
MaxAlpha=255    ; 透明度上限
```

不创建该文件则使用上述默认值。

## 已知限制

- **UWP / 现代应用**：部分 UWP 窗口（`Windows.UI.Core.CoreWindow` 等）透明化后可能不重绘，已默认跳过核心 UWP 窗口类。
- **硬件加速窗口**：Chrome / Electron 等启用硬件加速的窗口，透明后文字渲染可能异常，属系统分层窗口机制的固有限制。
- **权限不足的窗口**：对以更高权限运行的程序会静默失败，不影响系统稳定性。

## 技术栈

C++17 + 纯 Win32 API（`WH_MOUSE_LL` 钩子 + `SetLayeredWindowAttributes` + `WS_EX_LAYERED`），无任何第三方依赖。交互设计参考了 [AltSnap](https://github.com/RamonUnch/AltSnap)，但为完全独立实现。

## 许可证

[MIT](LICENSE)
