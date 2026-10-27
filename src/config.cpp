#include "config.h"

#include <windows.h>

#include <algorithm>
#include <string>

namespace {

// 从 INI 读取一个整数；若键缺失或值非法则返回默认值 def。
int ReadInt(LPCWSTR section, LPCWSTR key, int def) {
    wchar_t buf[64] = {0};
    DWORD n = GetPrivateProfileStringW(section, key, L"", buf,
                                       static_cast<DWORD>(64), nullptr);
    if (n == 0 || n >= 63) {
        return def;
    }
    wchar_t* end = nullptr;
    long v = wcstol(buf, &end, 10);
    if (end == buf) {
        return def;
    }
    return static_cast<int>(v);
}

}  // namespace

Config LoadConfig() {
    Config c;

    // INI 路径：exe 同目录下的 WinOpacity.ini。便携式，不写注册表。
    wchar_t path[MAX_PATH] = {0};
    GetModuleFileNameW(nullptr, path, MAX_PATH);
    std::wstring ini = path;
    ini = ini.substr(0, ini.find_last_of(L'\\') + 1) + L"WinOpacity.ini";

    c.coarseStep = ReadInt(L"Opacity", L"CoarseStep", c.coarseStep);
    c.fineStep   = ReadInt(L"Opacity", L"FineStep", c.fineStep);
    c.minAlpha   = ReadInt(L"Opacity", L"MinAlpha", c.minAlpha);
    c.maxAlpha   = ReadInt(L"Opacity", L"MaxAlpha", c.maxAlpha);

    // 边界保护：步长至少为 1，上下限必须在 [1, 255] 且 min <= max。
    c.coarseStep = std::max(1, c.coarseStep);
    c.fineStep   = std::max(1, c.fineStep);
    c.minAlpha   = std::clamp(c.minAlpha, 1, 255);
    c.maxAlpha   = std::clamp(c.maxAlpha, 1, 255);
    if (c.minAlpha > c.maxAlpha) {
        std::swap(c.minAlpha, c.maxAlpha);
    }

    return c;
}
