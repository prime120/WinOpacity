#pragma once

// 用户可配置项。默认值即为 MVP 的推荐值，若 INI 缺失则使用这些默认值。
struct Config {
    int coarseStep = 64;   // 普通调整步长（Alt+滚轮）
    int fineStep   = 8;    // 精细调整步长（Alt+Shift+滚轮）
    int minAlpha   = 32;   // 透明度下限（避免窗口意外完全不可见）
    int maxAlpha   = 255;  // 透明度上限（255 表示完全不透明）
};

// 从 exe 同目录的 WinOpacity.ini 读取配置，缺省项回落到默认值。
// 读取结果总是经过边界保护，保证数值合法。
Config LoadConfig();
