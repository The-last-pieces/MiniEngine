//
// Created by MnZn on 2022/7/13.
//

#ifndef MINI_ENGINE_VEC_HPP
#define MINI_ENGINE_VEC_HPP

#include <cmath>
#include <algorithm>
#include <iostream>

namespace mne {

using number = float;

constexpr number eps = 1e-8f;

constexpr inline number pi = 3.14159265358979323846f;

template<int N>
requires(N >= 1) struct Vec {
    number data[N] = {};

public:
#pragma region 容器相关
    // 下标访问
    constexpr number& operator[](int i) { return data[i]; }
    constexpr number  operator[](int i) const { return data[i]; }

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

    // 挑选出某个维度的向量
    template<int... idx>
    constexpr Vec<sizeof...(idx)> pick() const {
        return Vec<sizeof...(idx)>{data[idx]...};
    }

#pragma endregion
public:
#pragma region 非运算符运算
    // 长度的平方
    constexpr number norm2() const { return *this * *this; }
    // 向量长度
    constexpr number length() const { return std::sqrt(norm2()); }
    // 归一化
    constexpr Vec normalize() const { return *this / length(); }

    // 最值
    constexpr number v_min() const {
        return *std::min_element(data, data + N);
    }
    constexpr number v_max() const {
        return *std::max_element(data, data + N);
    }

    // 叉积
    constexpr Vec cross(const Vec& rhs) const requires(N == 3) {
        const Vec& lhs = *this;
        return {lhs[1] * rhs[2] - lhs[2] * rhs[1],
                lhs[2] * rhs[0] - lhs[0] * rhs[2],
                lhs[0] * rhs[1] - lhs[1] * rhs[0]};
    }

    constexpr number cdot(const Vec& rhs) const requires(N == 2) {
        const Vec& lhs = *this;
        return lhs[1] * rhs[0] - lhs[0] * rhs[1];
    }

    // this和rhs的夹角, [-pi, pi]
    constexpr number inner(const Vec& rhs) const {
        const Vec& lhs = *this;
        return std::acos((lhs * rhs) / (lhs.length() * rhs.length()));
    }

    // this顺时针旋转到target需要的角度
    constexpr number rotate(const Vec& target) const {
        const Vec &lhs = *this, &rhs = target;
        number     angle = inner(rhs);
        if (lhs.cdot(rhs) < 0) angle = 2 * pi - angle;
        return angle;
    }
#pragma endregion
public:
#pragma region 特化函数
    // 平面坐标/向量
    constexpr number x() const requires(N >= 1) {
        return data[0];
    }
    constexpr number& x() requires(N >= 1) {
        return data[0];
    }
    constexpr number y() const requires(N >= 2) {
        return data[1];
    }
    constexpr number& y() requires(N >= 2) {
        return data[1];
    }

    // 空间坐标/向量
    constexpr number z() const requires(N >= 3) {
        return data[2];
    }
    constexpr number& z() requires(N >= 3) {
        return data[2];
    }

    // 四元数
    constexpr number w() const requires(N >= 4) {
        return data[3];
    }
    constexpr number& w() requires(N >= 4) {
        return data[3];
    }

    // 仅将w归一化
    constexpr Vec trim() const requires(N == 4) {
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

    // IO相关
    friend std::ostream& operator<<(std::ostream& os, const Vec& vec) {
        for (int i = 0; i < N - 1; ++i) os << vec[i] << ' ';
        return os << vec[N - 1];
    }
    friend std::istream& operator>>(std::istream& is, Vec& vec) {
        for (int i = 0; i < N; ++i) is >> vec[i];
        return is;
    }
#pragma endregion
};

// 平面坐标/向量
using Vec2 = Vec<2>;
// 空间坐标/向量
using Vec3 = Vec<3>;
// 四元数
using Vec4 = Vec<4>;

template<class... Args>
inline auto make_vec(Args&&... args) requires((std::is_nothrow_convertible_v<Args, number> && ...)) {
    return Vec<sizeof...(args)>{number(args)...};
}

// 拼接多个向量
template<int A, int B, int... N>
inline constexpr auto concat_vec(const Vec<A>& a, const Vec<B>& b, const Vec<N>&... args) {
    if constexpr (sizeof...(N) == 0) {
        Vec<A + B> ret{};
        for (int i = 0; i < A; ++i) ret[i] = a[i];
        for (int i = 0; i < B; ++i) ret[i + A] = b[i];
        return ret;
    } else return concat_vec(concat_vec(a, b), args...);
}

} // namespace mne

#endif //MINI_ENGINE_VEC_HPP
