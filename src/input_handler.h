#pragma once

#include <windows.h>

#include "config.h"
#include "transparency_controller.h"
#include "window_picker.h"

// 处理全局低级鼠标钩子，识别三种手势：
//   Alt + 滚轮           -> 普通步长调整
//   Alt + Shift + 滚轮    -> 精细步长调整
//   Ctrl + Alt + 滚轮     -> 恢复目标窗口不透明
//
// 只负责"识别手势并触发动作"，不负责消息循环与钩子生命周期（由 App 管理）。
class InputHandler {
public:
    InputHandler(const Config& cfg, WindowPicker& picker,
                 TransparencyController& ctl);

    // 低级鼠标钩子回调，由 App 的 LowLevelMouseProc 转发进来。
    // nCode / wParam / lParam 与 WH_MOUSE_LL 回调签名一致。
    LRESULT OnMouseEvent(int nCode, WPARAM wParam, LPARAM lParam);

private:
    // 判断某个虚拟键当前是否被按下。
    static bool IsKeyDown(int vk);

    // 处理一次滚轮调整。delta > 0 表示滚轮向上。
    void HandleWheel(short wheelDelta);

    const Config& cfg_;
    WindowPicker& picker_;
    TransparencyController& ctl_;
};
