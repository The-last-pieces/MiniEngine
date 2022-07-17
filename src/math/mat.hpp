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

    constexpr Mat(const std::array<Vec<N>, M>& init) {
        int i = 0;
        for (auto& row : init) {
            data[i++] = row;
        }
    }

    constexpr Mat(std::initializer_list<std::initializer_list<number>> init) {
        int i = 0;
        for (auto& row : init) {
            int j = 0;
            for (auto elem : row) {
                atc(i, j++) = elem;
            }
            i++;
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
        return (*this * rhs.as<4>()).template as<3>();
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

// 矩阵相关的工厂函数
class Factory {
public:
#pragma region 杂项函数
    // 单位矩阵
    template<int N>
    constexpr static Mat<N, N> identity() {
        Mat<N, N> m;
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                m.at(i, j) = i == j ? 1.0f : 0.0f;
            }
        }
        return m;
    }

    // 3维向量转反对称矩阵
    constexpr static Mat33 anti(const Vec3& input) {
        auto [x, y, z] = input.data;
        return {
            {0, -z, y},
            {z, 0, -x},
            {-y, x, 0}};
    }

    // 合并变换,如果需要连续执行A1~An,则返回An*...*A1
    template<class T, class... Args>
    constexpr static T merge(const T& first, const Args&... args) {
        if constexpr (sizeof...(args) == 0) {
            return first;
        } else {
            return merge(args...) * first;
        }
    }

#pragma endregion
public:
#pragma region 旋转矩阵3x3
    // 绕X轴转
    static Mat33 rotateX(number angle) {
        number c = std::cos(angle);
        number s = std::sin(angle);
        return Mat33{
            {1, 0, 0},
            {0, c, -s},
            {0, s, c}};
    }

    // 绕Y轴转
    static Mat33 rotateY(number angle) {
        number c = std::cos(angle);
        number s = std::sin(angle);
        return Mat33{
            {c, 0, s},
            {0, 1, 0},
            {-s, 0, c}};
    }

    // 绕Z轴转
    static Mat33 rotateZ(number angle) {
        number c = std::cos(angle);
        number s = std::sin(angle);
        return Mat33{
            {c, -s, 0},
            {s, c, 0},
            {0, 0, 1}};
    }

    // 绕(过原点的)任意轴旋转,右手拇指朝向k,四指方向为旋转方向
    static Mat33 rotate(const Vec3& k, number angle) {
        // R = I + sin(θ) * K + (1 - cos(θ)) * K^2
        number c = std::cos(angle);
        number s = std::sin(angle);
        Mat33  K = anti(k.normalize());
        return identity<3>() + K * s + (K * K) * (1 - c);
    }

    // 绕任意点和任意轴旋转
    static Mat44 rotateAt(const Vec3& p, const Vec3& k, number angle) {
        return merge(translate(-p), rotate(k, angle).as4(), translate(p));
    }

    // 从某个方向旋转到+Z轴方向
    static Mat33 rotateToZ(const Vec3& dir) {
        // 无视x轴,绕x轴旋转到z
        auto x_mat = rotateX(dir.pick<1, 2>().rotate({0, 1}));
        // 无视y轴,绕y轴旋转到z
        auto y_mat = rotateY((x_mat * dir).pick<0, 2>().rotate({0, 1}));
        return merge(x_mat, y_mat);
    }

    // 从source旋转到dest方向
    static Mat33 rotateFT(const Vec3& s, const Vec3& d) {
        // s->z -(d->z)
        return merge(rotateToZ(s), rotateToZ(d).invert());
    }

#pragma endregion
public:
#pragma region 其他变换
    // 平移变换Ax=x+v
    static Mat44 translate(const Vec3& v) {
        return Mat44{
            {1, 0, 0, v.x()},
            {0, 1, 0, v.y()},
            {0, 0, 1, v.z()},
            {0, 0, 0, 1}};
    }

    // 缩放变换(坐标轴方向)
    static Mat33 scaleXYZ(const Vec3& sz) {
        return Mat33{
            {sz.x(), 0, 0},
            {0, sz.y(), 0},
            {0, 0, sz.z()}};
    }

    // 缩放变换(原点,任意轴方向)
    static Mat33 scale(const Vec3& dir, const Vec3& sz) {
        // d->z *sz -(d->z)
        auto dz = rotateToZ(dir);
        return merge(dz, scaleXYZ(sz), dz.invert());
    }

#pragma endregion
};

} // namespace mne

#endif //MINI_ENGINE_MAT_HPP
