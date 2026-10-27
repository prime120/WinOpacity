#include "window_picker.h"

#include <cstring>

WindowPicker::WindowPicker() {
    selfPid_ = GetCurrentProcessId();
}

HWND WindowPicker::PickAtCursor() const {
    POINT pt;
    if (!GetCursorPos(&pt)) {
        return nullptr;
    }
    return PickAt(pt);
}

HWND WindowPicker::PickAt(POINT pt) const {
    HWND hwnd = WindowFromPoint(pt);
    if (!hwnd) {
        return nullptr;
    }

    // 上溯到根窗口：这是真正的应用顶层窗口。
    HWND root = GetAncestor(hwnd, GA_ROOT);
    if (!root) {
        root = hwnd;
    }

    if (ShouldSkip(root)) {
        return nullptr;
    }
    return root;
}

bool WindowPicker::ShouldSkip(HWND hwnd) const {
    if (!hwnd || !IsWindow(hwnd)) {
        return true;
    }

    // 跳过本进程自身的窗口（避免把自己透明化）。
    DWORD pid = 0;
    GetWindowThreadProcessId(hwnd, &pid);
    if (pid == selfPid_) {
        return true;
    }

    // 跳过系统窗口：桌面、任务栏、系统菜单等。按窗口类名精确匹配。
    char cls[256] = {0};
    GetClassNameA(hwnd, cls, static_cast<int>(sizeof(cls)));

    // 与 AltSnap 默认黑名单重叠的系统窗口类（精简版）。
    static const char* const kSystemClasses[] = {
        "Progman",               // 桌面窗口
        "WorkerW",               // 壁纸承载层
        "Shell_TrayWnd",         // 任务栏
        "Shell_SecondaryTrayWnd",// 副屏任务栏
        "#32768",                // 系统菜单
        "Windows.UI.Core.CoreWindow",  // UWP 核心窗口（透明化常失效）
        "MultitaskingViewFrame",       // Win+Tab 任务视图
        "XamlExplorerHostIslandWindow",// 资源管理器 WinUI 宿主
    };
    for (const char* name : kSystemClasses) {
        if (std::strcmp(cls, name) == 0) {
            return true;
        }
    }

    return false;
}
