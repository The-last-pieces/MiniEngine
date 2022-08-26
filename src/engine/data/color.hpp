//
// Created by MnZn on 2022/7/13.
//

#ifndef MINI_ENGINE_COLOR_HPP
#define MINI_ENGINE_COLOR_HPP

#include <cstdint>
#include "../math/utils.hpp"

/*
 提供颜色相关的api
 */

namespace mne {
struct Color {
    // 范围为[0,1]
    number r, g, b; // Todo 是否考虑alpha通道?

    // 约束在[0,1]范围
    constexpr Color clamp() const {
        return {
            mne::clamp(0_n, r, 1_n),
            mne::clamp(0_n, g, 1_n),
            mne::clamp(0_n, b, 1_n)};
    }

    // 颜色混合
    friend constexpr Color operator+(const Color& lhs, const Color& rhs) {
        return {lhs.r + rhs.r, lhs.g + rhs.g, lhs.b + rhs.b};
    }

    // 数乘
    friend constexpr Color operator*(const Color& lhs, number mut) {
        return {lhs.r * mut, lhs.g * mut, lhs.b * mut};
    }

    friend constexpr Color operator*(number mut, const Color& rhs) {
        return {rhs.r * mut, rhs.g * mut, rhs.b * mut};
    }

    // 按位乘
    friend constexpr Color operator*(const Color& lhs, const Color& rhs) {
        return {lhs.r * rhs.r, lhs.g * rhs.g, lhs.b * rhs.b};
    }

    // 数除
    friend constexpr Color operator/(const Color& lhs, number mut) {
        return {lhs.r / mut, lhs.g / mut, lhs.b / mut};
    }

    // 赋值
    Color& operator+=(const Color& rhs) {
        r += rhs.r, g += rhs.g, b += rhs.b;
        return *this;
    }

    Color& operator*=(number mut) {
        r *= mut, g *= mut, b *= mut;
        return *this;
    }

    Color& operator/=(number mut) {
        r /= mut, g /= mut, b /= mut;
        return *this;
    }

    // 将[0,256)映射到[0,1]
    static constexpr Color fromRGB256(std::uint8_t r, std::uint8_t g, std::uint8_t b) {
        constexpr int div = 255;
        return {number(r) / div, number(g) / div, number(b) / div};
    }
};
} // namespace mne

#endif //MINI_ENGINE_COLOR_HPP
