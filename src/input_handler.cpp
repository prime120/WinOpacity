#include "input_handler.h"

InputHandler::InputHandler(const Config& cfg, WindowPicker& picker,
                           TransparencyController& ctl)
    : cfg_(cfg), picker_(picker), ctl_(ctl) {}

LRESULT InputHandler::OnMouseEvent(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode < 0) {
        return CallNextHookEx(nullptr, nCode, wParam, lParam);
    }

    // 只在 Alt 按下时处理滚轮消息；其余消息一律放行。
    if (wParam != WM_MOUSEWHEEL || !IsKeyDown(VK_MENU)) {
        return CallNextHookEx(nullptr, nCode, wParam, lParam);
    }

    const auto* ms =
        reinterpret_cast<const MSLLHOOKSTRUCT*>(lParam);
    short wheelDelta = static_cast<short>(
        static_cast<DWORD>(ms->mouseData) >> 16);

    if (wheelDelta != 0) {
        HandleWheel(wheelDelta);
        // 吞掉此滚轮事件，避免目标窗口同时滚动。
        return 1;
    }

    return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

bool InputHandler::IsKeyDown(int vk) {
    return (GetAsyncKeyState(vk) & 0x8000) != 0;
}

void InputHandler::HandleWheel(short wheelDelta) {
    HWND target = picker_.PickAtCursor();
    if (!target) {
        return;  // 无可安全操作的目标，忽略。
    }

    bool ctrl = IsKeyDown(VK_CONTROL);
    bool shift = IsKeyDown(VK_SHIFT);

    if (ctrl) {
        // Ctrl+Alt+滚轮：恢复不透明。
        ctl_.Apply(target, 255);
        return;
    }

    // 决定步长与方向：向上（>0）增加 alpha（更不透明），向下减少。
    int step = shift ? cfg_.fineStep : cfg_.coarseStep;
    int delta = (wheelDelta > 0) ? step : -step;

    ctl_.Adjust(target, delta);
}
