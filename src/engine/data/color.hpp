//
// Created by MnZn on 2022/7/13.
//

#ifndef MINI_ENGINE_COLOR_HPP
#define MINI_ENGINE_COLOR_HPP

#include "math/utils.hpp"
#include <cstdint>

/*
 提供颜色相关的api
 */

namespace mne {

// RGB颜色
struct Color {
    // 范围为[0,1]
    number r, g, b;

    // 约束在[0,limit]范围
    constexpr Color clamp(number limit = 1_n) const {
        return {
            MathUtils::clamp(0_n, r, limit),
            MathUtils::clamp(0_n, g, limit),
            MathUtils::clamp(0_n, b, limit)};
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

    // 按位除
    friend constexpr Color operator/(const Color& lhs, const Color& rhs) {
        return {lhs.r / rhs.r, lhs.g / rhs.g, lhs.b / rhs.b};
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
    static constexpr Color fromRGB256(number r, number g, number b) {
        constexpr int div = 255;
        return {r / div, g / div, b / div};
    }
};

// RGBA颜色
struct ColorA {
    number r, g, b, a;

    constexpr ColorA() = default;

    constexpr ColorA(number r, number g, number b, number a = 1_n):
        r(r), g(g), b(b), a(a) {}

    constexpr ColorA(const Color& color, number a = 1_n):
        r(color.r), g(color.g), b(color.b), a(a) {}

    // 和背景色混合获取真实颜色
    constexpr Color mix(const Color& background = {}) const {
        return background * (1_n - a) + rgb() * a;
    }

    // 直接转rgb
    constexpr Color rgb() const {
        return {r, g, b};
    }

    // 约束在[0,limit]范围
    constexpr ColorA clamp(number limit = 1_n) const {
        return {
            MathUtils::clamp(0_n, r, limit),
            MathUtils::clamp(0_n, g, limit),
            MathUtils::clamp(0_n, b, limit),
            MathUtils::clamp(0_n, a, limit)};
    }

    // +
    friend constexpr ColorA
    operator+(const ColorA& lhs, const ColorA& rhs) {
        Color  c1 = lhs.rgb(), c2 = rhs.rgb();
        number a1 = lhs.a, a2 = rhs.a, a12 = a1 + a2 - a1 * a2;
        Color  c12 = (c1 * a1 * (1_n - a2) + c2 * a2) / (a1 + a2 - a1 * a2);
        return {c12, a12};
    }
};

} // namespace mne

#endif //MINI_ENGINE_COLOR_HPP
