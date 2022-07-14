//
// Created by MnZn on 2022/7/13.
//

#ifndef MINI_ENGINE_COLOR_HPP
#define MINI_ENGINE_COLOR_HPP

#include <cstdint>

/*
 提供颜色相关的api
 */

namespace mne {
struct Color {
    // 范围为[0,1]
    float r, g, b; // Todo 是否考虑alpha通道?

    // 颜色混合
    friend constexpr Color operator+(const Color& lhs, const Color& rhs) {
        return {lhs.r + rhs.r, lhs.g + rhs.g, lhs.b + rhs.b};
    }

    // 数乘
    friend constexpr Color operator*(const Color& lhs, float mut) {
        return {lhs.r * mut, lhs.g * mut, lhs.b * mut};
    }

    friend constexpr Color operator*(float mut, const Color& rhs) {
        return rhs * mut;
    }

    // 数除
    friend constexpr Color operator/(const Color& lhs, float mut) {
        return {lhs.r / mut, lhs.g / mut, lhs.b / mut};
    }

    // 将[0,256)映射到[0,1]
    static constexpr Color fromRGB256(std::uint8_t r, std::uint8_t g, std::uint8_t b) {
        constexpr int div = 255;
        return {float(r) / div, float(g) / div, float(b) / div};
    }
};
} // namespace mne

#endif //MINI_ENGINE_COLOR_HPP
