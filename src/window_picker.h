#pragma once

#include <windows.h>

// 根据鼠标屏幕坐标识别"应被透明化的目标窗口"。
//
// 流程：GetCursorPos -> WindowFromPoint -> GetAncestor(GA_ROOT) -> 过滤。
// 关键点：透明化针对根窗口，而不是 WindowFromPoint 直接返回的子窗口/控件，
//         否则只会透明化一个按钮而非整个应用。
class WindowPicker {
public:
    WindowPicker();

    // 返回鼠标当前指向的、可安全透明化的根窗口；无可选目标返回 nullptr。
    // 会跳过：本进程自身窗口、桌面、任务栏、系统菜单等系统窗口。
    HWND PickAtCursor() const;

    // 直接给定屏幕坐标解析目标（便于测试）。
    HWND PickAt(POINT pt) const;

private:
    // 判断 HWND 是否应被跳过（系统窗口 / 本进程窗口 / 无效窗口）。
    bool ShouldSkip(HWND hwnd) const;

    DWORD selfPid_;
};
