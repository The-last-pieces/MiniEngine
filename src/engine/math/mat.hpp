//
// Created by MnZn on 2022/7/13.
//

#ifndef MINI_ENGINE_MAT_HPP
#define MINI_ENGINE_MAT_HPP

#include "vec.hpp"
#include <array>
#include <stdexcept>

/*
 本模块实现了定长矩阵的数值运算,并提供了便捷构造变换矩阵的工厂函数.
 */

namespace mne {

template<int M, int N>
class Mat;

// 别名
using Mat33 = Mat<3, 3>;
using Mat34 = Mat<3, 4>;
using Mat43 = Mat<4, 3>;
using Mat44 = Mat<4, 4>;

template<int M, int N>
class Mat {
    Vec<N> data[M]{};

public:
#pragma region 构造相关
    constexpr Mat() = default;

    constexpr Mat(const std::initializer_list<Vec<N>>& init) {
        int i = 0;
        for (auto& row : init) {
            data[i++] = row;
        }
    }

#pragma endregion
public:
#pragma region 运算符重载
    // * mat
    template<int C>
    constexpr friend Mat<M, C> operator*(const Mat<M, N>& lhs, const Mat<N, C>& rhs) {
        Mat<M, C> res;
        for (int i = 0; i < M; i++) {
            for (int k = 0; k < N; k++) {
                for (int j = 0; j < C; j++) {
                    res.at(i, j) += lhs.at(i, k) * rhs.at(k, j);
                }
            }
        }
        return res;
    }

    // A/B = A*B.invert()
    // <M,N>/<C,N> <M,N> * <N,C>
    template<int C>
    constexpr friend Mat<M, C> operator/(const Mat<M, N>& lhs, const Mat<C, N>& rhs) {
        return lhs * rhs.invert(); // Todo 使用高斯消元优化
    }

    // * vec
    constexpr friend Vec<M> operator*(const Mat<M, N>& lhs, const Vec<N>& rhs) {
        Vec<M> ret;
        for (int i = 0; i < M; i++) {
            ret[i] = lhs.row(i) * rhs;
        }
        return ret;
    }

    // vec *
    constexpr friend Vec<N> operator*(const Vec<M>& rhs, const Mat<M, N>& lhs) {
        Vec<N> ret;
        for (int i = 0; i < N; i++) {
            ret[i] = rhs * lhs.col(i); // Todo 优化右乘
        }
        return ret;
    }

    // +=
    constexpr Mat& operator+=(const Mat& rhs) {
        for (int i = 0; i < M; i++) {
            data[i] += rhs.data[i];
        }
        return *this;
    }

    // -=
    constexpr Mat& operator-=(const Mat& rhs) {
        for (int i = 0; i < M; i++) {
            data[i] -= rhs.data[i];
        }
        return *this;
    }

    // *= k
    constexpr Mat& operator*=(number k) {
        for (int i = 0; i < M; i++) {
            data[i] *= k;
        }
        return *this;
    }

    // /= k
    constexpr Mat& operator/=(number k) {
        for (int i = 0; i < M; i++) {
            data[i] /= k;
        }
        return *this;
    }

    // +
    constexpr friend Mat operator+(const Mat& lhs, const Mat& rhs) { return Mat{lhs} += rhs; }

    // -
    constexpr friend Mat operator-(const Mat& lhs, const Mat& rhs) { return Mat{lhs} -= rhs; }

    // * k
    constexpr friend Mat operator*(const Mat& lhs, number k) { return Mat{lhs} *= k; }
    constexpr friend Mat operator*(number k, const Mat& lhs) { return Mat{lhs} *= k; }

    // / k
    constexpr friend Mat operator/(const Mat& lhs, number k) { return Mat{lhs} /= k; }
#pragma endregion
public:
#pragma region 非运算符函数
    // 求行列式
    constexpr number det() const requires(M == N) {
        if constexpr (N == 1) {
            return at(0, 0);
        } else {
            number ret{};
            for (int j = 0; j < N; ++j) {
                ret += at(0, j) * exclude_det(0, j);
            }
            return ret;
        }
    }

    // 转置
    constexpr Mat<N, M> flap() const {
        Mat<N, M> res;
        for (int j = 0; j < N; j++) {
            for (int i = 0; i < M; i++) {
                res.at(i, j) = data[j][i];
            }
        }
        return res;
    }

    // 逆矩阵
    constexpr Mat<N, M> invert() const requires(M == N) {
        Mat<N, M> res;
        number    dt = det();
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < N; j++) {
                res.at(i, j) = exclude_det(i, j) / dt;
            }
        }
        return res.flap();
    }

    // 转换为其他尺寸的矩阵
    template<int P, int Q>
    constexpr Mat<P, Q> as(number fill = 0) const {
        Mat<P, Q> res;
        for (int i = 0; i < P; i++) {
            for (int j = 0; j < Q; j++) {
                res.at(i, j) = out_range(i, j) ? fill : at(i, j);
            }
        }
        return res;
    }

    // 3x3变换矩阵转为4x4变换矩阵
    constexpr Mat44 as4() const requires(M == 3 && N == 3) {
        Mat44 res    = as<4, 4>();
        res.at(3, 3) = 1;
        return res;
    }

    // 特化Mat44 x Vec3
    constexpr Vec3 operator*(const Vec3& rhs) const requires(M == 4 && N == 4) {
        return (*this * rhs.as<4>()).trim().template as<3>();
    }

#pragma endregion
public:
#pragma region 访问函数
    constexpr number& atc(int i, int j) {
        if (out_range(i, j)) throw std::out_of_range("mat::at");
        return data[i][j];
    }

    constexpr number atc(int i, int j) const {
        if (out_range(i, j)) throw std::out_of_range("mat::at");
        return data[i][j];
    }

    constexpr number& at(int i, int j) { return data[i][j]; }

    constexpr number at(int i, int j) const { return data[i][j]; }

    constexpr Vec<N>& row(int i) { return data[i]; }
    constexpr Vec<N>  row(int i) const { return data[i]; }

    constexpr Vec<M> col(int j) const {
        Vec<M> ret;
        for (int i = 0; i < M; i++) ret[i] = at(i, j);
        return ret;
    }
#pragma endregion
private:
#pragma region 辅助函数
    // 是否越界
    constexpr static bool out_range(int i, int j) {
        return i < 0 || i >= M || j < 0 || j >= N;
    }

    // 获取余子式
    constexpr Mat<M - 1, N - 1> exclude(int ei, int ej) const {
        Mat<M - 1, N - 1> ret;
        for (int i = M - 1; i--;)
            for (int j = N - 1; j--;)
                ret.at(i, j) = at(i < ei ? i : i + 1, j < ej ? j : j + 1);
        return ret;
    }

    // 获取代数余子式的值
    constexpr number exclude_det(int i, int j) const {
        return exclude(i, j).det() * ((i + j) % 2 ? -1 : 1);
    }
#pragma endregion
};

} // namespace mne

#endif //MINI_ENGINE_MAT_HPP
