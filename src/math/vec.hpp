//
// Created by MnZn on 2022/7/13.
//

#ifndef MINI_ENGINE_VEC_HPP
#define MINI_ENGINE_VEC_HPP

#include <cmath>

namespace mne {

using number = float;

constexpr number eps = 1e-8f;

template<int N>
struct Vec {
    static_assert(N > 0);

    number data[N] = {};

public:
#pragma region 杂项函数
    // 下标访问
    constexpr number& operator[](int i) { return data[i]; }
    constexpr number  operator[](int i) const { return data[i]; }

    // 长度的平方
    constexpr number norm2() const { return *this * *this; }
    // 向量长度
    constexpr number length() const { return std::sqrt(norm2()); }
    // 归一化
    constexpr Vec normalize() const { return *this / length(); }

    // 转换为其他长度的向量,如果N<M则用fill填充
    template<int M>
    constexpr Vec<M> as(number fill = 1) const {
        Vec<M> ret{};
        if constexpr (N < M) {
            for (int i = 0; i < N; ++i) ret[i] = data[i];
            for (int i = N; i < M; ++i) ret[i] = fill;
        } else {
            for (int i = 0; i < M; ++i) ret[i] = data[i];
        }
        return ret;
    }

    // 叉积
    constexpr Vec cross(const Vec& rhs) const {
        static_assert(N == 3, "cross only support 3D");
        const Vec& lhs = *this;
        return {lhs[1] * rhs[2] - lhs[2] * rhs[1],
                lhs[2] * rhs[0] - lhs[0] * rhs[2],
                lhs[0] * rhs[1] - lhs[1] * rhs[0]};
    }
#pragma endregion
public:
#pragma region 特化函数
    // 平面坐标/向量
    constexpr number x() const {
        static_assert(N >= 1);
        return data[0];
    }
    constexpr number y() const {
        static_assert(N >= 2);
        return data[1];
    }

    // 空间坐标/向量
    constexpr number z() const {
        static_assert(N >= 3);
        return data[2];
    }

    // 四元数
    constexpr number w() const {
        static_assert(N >= 4);
        return data[3];
    }

    // 仅将w归一化
    constexpr Vec trim() const {
        static_assert(N == 4);
        auto W = w();
        if (W > eps || W < -eps) return *this / W;
        return *this;
    }
#pragma endregion
public:
#pragma region 运算符重载
    constexpr Vec& operator+=(const Vec& rhs) {
        for (int i = 0; i < N; ++i) data[i] += rhs[i];
        return *this;
    }

    constexpr Vec& operator-=(const Vec& rhs) {
        for (int i = 0; i < N; ++i) data[i] -= rhs[i];
        return *this;
    }

    constexpr Vec& operator*=(number k) {
        for (int i = 0; i < N; ++i) data[i] *= k;
        return *this;
    }

    constexpr Vec& operator/=(number k) {
        for (int i = 0; i < N; ++i) data[i] /= k;
        return *this;
    }

    constexpr friend Vec operator+(const Vec& lhs, const Vec& rhs) { return Vec{lhs} += rhs; }

    constexpr friend Vec operator-(const Vec& lhs, const Vec& rhs) { return Vec{lhs} -= rhs; }

    constexpr Vec operator-() const { return *this * -1; }

    // 点乘
    constexpr friend number operator*(const Vec& lhs, const Vec& rhs) {
        number ret{};
        for (int i = 0; i < N; ++i) ret += lhs[i] * rhs[i];
        return ret;
    }

    constexpr friend Vec operator*(const Vec& lhs, number k) { return Vec{lhs} *= k; }

    constexpr friend Vec operator*(number k, const Vec& rhs) { return rhs * k; }

    constexpr friend Vec operator/(const Vec& lhs, number k) { return Vec{lhs} /= k; }
#pragma endregion
};

// 平面坐标/向量
using Vec2 = Vec<2>;
// 空间坐标/向量
using Vec3 = Vec<3>;
// 四元数
using Vec4 = Vec<4>;

} // namespace mne

#endif //MINI_ENGINE_VEC_HPP
