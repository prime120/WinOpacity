#include "transparency_controller.h"

#include <algorithm>

TransparencyController::TransparencyController(int minAlpha, int maxAlpha)
    : minAlpha_(minAlpha), maxAlpha_(maxAlpha) {}

bool TransparencyController::Apply(HWND hwnd, int alpha) {
    if (!hwnd || !IsWindow(hwnd)) {
        return false;
    }

    // 记录原始状态（首次触碰时）。
    EnsureSaved(hwnd);

    // 边界钳制。
    alpha = std::clamp(alpha, minAlpha_, maxAlpha_);

    LONG_PTR exstyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
    if (exstyle == 0) {
        return false;
    }

    if (alpha >= 255) {
        // 完全不透明：移除分层标志即可（不再需要 SetLayeredWindowAttributes）。
        if (exstyle & WS_EX_LAYERED) {
            SetWindowLongPtr(hwnd, GWL_EXSTYLE, exstyle & ~WS_EX_LAYERED);
        }
        return true;
    }

    // 需要透明：先确保分层标志存在，再设置全局 alpha。
    if (!(exstyle & WS_EX_LAYERED)) {
        SetWindowLongPtr(hwnd, GWL_EXSTYLE, exstyle | WS_EX_LAYERED);
    }
    return SetLayeredWindowAttributes(hwnd, 0,
                                      static_cast<BYTE>(alpha), LWA_ALPHA) != FALSE;
}

int TransparencyController::Adjust(HWND hwnd, int delta) {
    if (!hwnd || !IsWindow(hwnd)) {
        return -1;
    }
    int current = GetCurrentAlpha(hwnd);
    int target = current + delta;
    target = std::clamp(target, minAlpha_, maxAlpha_);
    if (!Apply(hwnd, target)) {
        return -1;
    }
    return target;
}

int TransparencyController::GetCurrentAlpha(HWND hwnd) const {
    if (!hwnd || !IsWindow(hwnd)) {
        return 255;
    }
    LONG_PTR exstyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
    if (!(exstyle & WS_EX_LAYERED)) {
        // 非分层窗口等价于完全不透明。
        return 255;
    }
    BYTE alpha = 0;
    if (GetLayeredWindowAttributes(hwnd, nullptr, &alpha, nullptr)) {
        return alpha;
    }
    return 255;
}

void TransparencyController::RestoreOne(HWND hwnd) {
    auto it = saved_.find(hwnd);
    if (it == saved_.end()) {
        return;
    }
    SavedState st = it->second;
    saved_.erase(it);
    RestoreInternal(hwnd, st);
}

void TransparencyController::RestoreAll() {
    // 遍历并恢复，失败（窗口已关闭）直接跳过，不抛出。
    for (auto& kv : saved_) {
        RestoreInternal(kv.first, kv.second);
    }
    saved_.clear();
}

void TransparencyController::EnsureSaved(HWND hwnd) {
    if (saved_.find(hwnd) != saved_.end()) {
        return;  // 已记录，保持第一次触碰时的原始状态。
    }
    SavedState st;
    LONG_PTR exstyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
    st.wasLayered = (exstyle & WS_EX_LAYERED) != 0;
    if (st.wasLayered) {
        BYTE alpha = 255;
        if (GetLayeredWindowAttributes(hwnd, nullptr, &alpha, nullptr)) {
            st.originalAlpha = alpha;
        }
    } else {
        st.originalAlpha = 255;
    }
    saved_[hwnd] = st;
}

bool TransparencyController::RestoreInternal(HWND hwnd, const SavedState& st) {
    if (!hwnd || !IsWindow(hwnd)) {
        return false;  // 窗口已关闭，无法恢复，安全跳过。
    }

    LONG_PTR exstyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
    if (exstyle == 0) {
        return false;
    }

    if (st.wasLayered) {
        // 原本就是分层窗口：还原分层标志并设回原 alpha。
        SetWindowLongPtr(hwnd, GWL_EXSTYLE, exstyle | WS_EX_LAYERED);
        SetLayeredWindowAttributes(hwnd, 0, st.originalAlpha, LWA_ALPHA);
    } else if (st.originalAlpha >= 255) {
        // 原本不是分层窗口：移除我们加上的分层标志。
        SetWindowLongPtr(hwnd, GWL_EXSTYLE, exstyle & ~WS_EX_LAYERED);
    }
    return true;
}
