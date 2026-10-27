// WinOpacity —— 极简窗口透明度工具
//
// 程序入口：WinMain + 消息循环。
// 职责：安装/卸载低级鼠标钩子、创建仅含"退出"的托盘图标、退出时恢复被改窗口。
#include <windows.h>

#include "config.h"
#include "input_handler.h"
#include "transparency_controller.h"
#include "window_picker.h"

namespace {

constexpr UINT WM_TRAYICON = WM_APP + 1;  // 托盘回调消息
constexpr UINT TRAY_ID = 1;

HHOOK g_mouseHook = nullptr;
HWND g_msgWnd = nullptr;          // 仅用于接收消息的隐藏窗口
NOTIFYICONDATAW g_nid = {};      // 托盘图标数据（保持生命周期）

// 全局对象，由 WinMain 初始化后持有到退出。
TransparencyController* g_ctl = nullptr;

// 低级鼠标钩子回调：转发给 InputHandler。
InputHandler* g_handler = nullptr;

// 目标窗口定位与过滤。
WindowPicker* g_picker = nullptr;

// 安装/卸载低级鼠标钩子。
void InstallHook() {
    if (!g_mouseHook) {
        g_mouseHook = SetWindowsHookExW(
            WH_MOUSE_LL,
            [](int nCode, WPARAM wParam, LPARAM lParam) -> LRESULT {
                if (g_handler) {
                    return g_handler->OnMouseEvent(nCode, wParam, lParam);
                }
                return CallNextHookEx(nullptr, nCode, wParam, lParam);
            },
            GetModuleHandleW(nullptr), 0);
    }
}

void UninstallHook() {
    if (g_mouseHook) {
        UnhookWindowsHookEx(g_mouseHook);
        g_mouseHook = nullptr;
    }
}

// 创建仅含"退出"的托盘图标。
void AddTrayIcon(HWND hwnd) {
    g_nid.cbSize = sizeof(NOTIFYICONDATAW);
    g_nid.hWnd = hwnd;
    g_nid.uID = TRAY_ID;
    g_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    g_nid.uCallbackMessage = WM_TRAYICON;
    g_nid.hIcon = LoadIconW(nullptr,
        reinterpret_cast<LPCWSTR>(static_cast<UINT_PTR>(32512)));
    wcscpy_s(g_nid.szTip, L"WinOpacity - Alt+滚轮调透明度");
    Shell_NotifyIconW(NIM_ADD, &g_nid);
}

void RemoveTrayIcon() {
    Shell_NotifyIconW(NIM_DELETE, &g_nid);
}

// 显示"退出"上下文菜单，供用户手动退出。
void ShowTrayMenu(HWND hwnd) {
    HMENU menu = CreatePopupMenu();
    AppendMenuW(menu, MF_STRING, 1, L"退出");

    POINT pt;
    GetCursorPos(&pt);
    SetForegroundWindow(hwnd);  // 确保菜单能被点击/正确关闭
    TrackPopupMenu(menu, TPM_RIGHTBUTTON | TPM_RETURNCMD, pt.x, pt.y, 0, hwnd, nullptr);
    DestroyMenu(menu);
    PostMessageW(hwnd, WM_CLOSE, 0, 0);  // 用户选择"退出"即关闭程序
}

// 隐藏消息窗口的窗口过程。
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_TRAYICON:
            if (lParam == WM_RBUTTONUP || lParam == WM_LBUTTONUP) {
                ShowTrayMenu(hwnd);
            }
            return 0;
        case WM_CLOSE:
            DestroyWindow(hwnd);
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        default:
            return DefWindowProcW(hwnd, msg, wParam, lParam);
    }
}

}  // namespace

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
    Config cfg = LoadConfig();
    WindowPicker picker;
    TransparencyController ctl(cfg.minAlpha, cfg.maxAlpha);
    InputHandler handler(cfg, picker, ctl);

    // 暴露给钩子回调和退出清理。
    g_ctl = &ctl;
    g_picker = &picker;
    g_handler = &handler;

    // 注册一个隐藏窗口类，用于接收托盘消息。
    const wchar_t* kCls = L"WinOpacityMsgWnd";
    WNDCLASSW wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = kCls;
    RegisterClassW(&wc);

    g_msgWnd = CreateWindowExW(0, kCls, L"", 0, 0, 0, 0, 0,
                               HWND_MESSAGE, nullptr, hInstance, nullptr);
    if (!g_msgWnd) {
        return 1;
    }

    AddTrayIcon(g_msgWnd);
    InstallHook();

    // 消息循环。
    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    // 退出清理：卸载钩子、移除托盘、恢复所有被改窗口。
    UninstallHook();
    RemoveTrayIcon();
    ctl.RestoreAll();

    return 0;
}
