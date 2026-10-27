#pragma once

#include <windows.h>

#include <unordered_map>

// 负责窗口透明度的实际修改、边界检查，以及被修改窗口的状态记录与恢复。
//
// 关键规则（借鉴 AltSnap 的 SetWindowAlpha，独立实现）：
//   - 只通过位运算增删 WS_EX_LAYERED 这一位，绝不用整体赋值覆盖其他扩展样式位。
//   - alpha == 255 时移除 WS_EX_LAYERED；否则先加上再 SetLayeredWindowAttributes。
//   - 使用 LWA_ALPHA（全局 alpha），不触碰 LWA_COLORKEY，避免破坏色键透明。
class TransparencyController {
public:
    TransparencyController(int minAlpha, int maxAlpha);

    // 把目标窗口的 alpha 设为 [minAlpha, maxAlpha] 内钳制后的值。
    // 首次触碰某窗口时记录其原始状态，供退出时恢复。
    // 返回 true 表示成功应用。
    bool Apply(HWND hwnd, int alpha);

    // 在目标窗口当前 alpha 基础上增减 delta，结果做边界钳制。
    // 返回调整后的 alpha；失败返回 -1。
    int Adjust(HWND hwnd, int delta);

    // 读取目标窗口当前 alpha；无法读取（如窗口失效或非分层）返回 255。
    int GetCurrentAlpha(HWND hwnd) const;

    // 恢复单个窗口到首次触碰前的原始状态（窗口已关闭则跳过）。
    void RestoreOne(HWND hwnd);

    // 恢复所有被本程序修改过的窗口。退出时调用。
    void RestoreAll();

private:
    struct SavedState {
        bool wasLayered = false;   // 原始扩展样式是否含 WS_EX_LAYERED
        BYTE originalAlpha = 255;  // 原始 alpha（非分层窗口视为 255）
    };

    // 记录原始状态（幂等：每个窗口只记录一次）。
    void EnsureSaved(HWND hwnd);
    // 恢复单个窗口的内部实现，返回 false 表示窗口已失效。
    bool RestoreInternal(HWND hwnd, const SavedState& st);

    int minAlpha_;
    int maxAlpha_;
    std::unordered_map<HWND, SavedState> saved_;
};
