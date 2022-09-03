//
// Created by MnZn on 2022/7/16.
//

#ifndef MINI_ENGINE_UTILS_HPP
#define MINI_ENGINE_UTILS_HPP

#include <random>
#include <ctime>
#include "../math/vec.hpp"
#include "../math/mat.hpp"
#include <concepts>

namespace mne {

/// 随机数工具函数
class RandomUtils {
public:
    // [l,r)
    static int randInt(int l, int r) {
        static auto seed  = 0;                  // time(nullptr);
        static auto maker = std::mt19937(seed); // NOLINT(cert-msc51-cpp)
        return int(maker() % (r - l)) + l;
    }

    // [0,n)
    static int randInt(int n) {
        return randInt(0, n);
    }

    // [0,1)
    static number randFloat() {
        return (number) randInt(RAND_MAX) / number(RAND_MAX);
    }

    // [l,r)
    static number randFloat(number l, number r) {
        return l + (r - l) * randFloat();
    }

    // 随机bool , per的概率返回true
    static bool randBool(number per) {
        return randFloat() < per;
    }

    // 随机选择一个下标
    template<class Iterable>
    static int randChoose(const Iterable& seq)
        requires requires(Iterable obj) {
        std::begin(obj);
        std::end(obj);
    }
    &&std::is_convertible_v<std::iter_value_t<Iterable>, number> {
        int    idx = 0;
        number sum = 0, pb = 0, rd = randFloat();
        for (auto& v : seq) {
            if (v < 0) throw std::runtime_error("probability can less than 0");
            sum += v;
        }
        for (auto& v : seq) {
            pb += v / sum;
            if (pb > rd) break;
            ++idx;
        }
        return idx;
    }
};

/// 通用工具函数
class MathUtils {
public:
    // 约束v到[l,r]区间
    template<class T>
    static constexpr T clamp(const T& l, const T& v, const T& r) {
        return std::max(l, std::min(r, v));
    }

    // 线性插值, t \in [0,1]
    template<class T>
    static constexpr T lerp(const T& a, const T& b, number t) {
        return a * (1 - t) + b * t;
    }

    // 浮点求膜 , 假定m > 0 , 返回[0, val)
    static number mod(number val, number m) {
        number ret = std::fmod(val, m);
        if (ret < 0) ret += m;
        return ret;
    }

    // 弧度转角度
    static constexpr number rad2deg(number rad) {
        return rad * 180_n / pi;
    }

    // 角度转弧度
    static constexpr number deg2rad(number deg) {
        return deg * pi / 180_n;
    }
};

/// Vec的工具函数
class VecUtils {
public:
    // 向量反射 , 假定v*n <= 0
    static Vec3 reflect(const Vec3& in, const Vec3& normal) {
        // v' - v = 2 * (v * n) * n
        return in - (2 * (in * normal)) * normal;
    }

    // 向量折射 , 假定v*n <= 0 , eta为入射介质和折射介质的折射率比
    static Vec3 refract(const Vec3& in, const Vec3& normal, number eta) {
        // https://zhuanlan.zhihu.com/p/91129191
        number dot = in * normal;
        number k   = 1_n - eta * eta * (1_n - dot * dot);
        if (k < 0) return {};
        return eta * in - (eta * dot + sqrt(k)) * normal;
    }

    // 投影在向量上
    static Vec3 mapToDir(const Vec3& from, const Vec3& toDir) {
        return (from * toDir) * toDir;
    }

    // 关于向量对折
    static constexpr Vec3 flapByVec(Vec3 v, Vec3 n) {
        // v + v' = 2 * (v * n) * n
        return (2 * (v * n)) * n - v;
    }

    // 在单位球上均匀采样
    static Vec3 sampleSphere() {
        // https://blog.csdn.net/yjr3426619/article/details/102706968
        number a = RandomUtils::randFloat(), b = RandomUtils::randFloat();
        // 随机生成转角和仰角
        number phi = 2 * pi * a, theta = acos(b);
        number x = cos(theta) * cos(phi);
        number y = cos(theta) * sin(phi);
        number z = sin(theta);
        if (RandomUtils::randBool(50)) z = -z;
        return make_vec(x, y, z);
    }

    // Todo 函数意义注释
    static Vec3 toWorld(Vec3 a, Vec3 n) {
        //施密特标准正交化
        auto N    = n.normalize();
        auto temp = std::abs(n.x()) > .1 ? make_vec(0, 1, 0) : make_vec(1, 0, 0);
        auto U    = (temp - (temp * N) * N).normalize();
        auto V    = N.cross(U);
        return a.x() * U + a.y() * V + a.z() * N;
    };

    // 在法线为n的半球采样,概率密度为dir*n/pi
    static Vec3 sampleHalfSphere(Vec3 n) {
        number phi    = 2 * pi * RandomUtils::randFloat();
        number z      = std::fabs(1.0f - 2 * RandomUtils::randFloat());
        number radius = std::sqrt(1.0f - z * z);

        number x = radius * std::cos(phi);
        number y = radius * std::sin(phi);

        // Todo 转换到世界坐标系意义注释
        return toWorld(make_vec(x, y, z), n);
    }

public:
    static constexpr Vec3 up    = make_vec(0, 1, 0);
    static constexpr Vec3 down  = make_vec(0, -1, 0);
    static constexpr Vec3 left  = make_vec(-1, 0, 0);
    static constexpr Vec3 right = make_vec(1, 0, 0);
    static constexpr Vec3 front = make_vec(0, 0, 1);
    static constexpr Vec3 back  = make_vec(0, 0, -1);
};

// 矩阵相关的工厂函数
class MatUtils {
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

    /**
     * @brief 透视变换矩阵
     * @param angle 视锥上下面之间的夹角
     * @param aspect viewPort的宽高比
     * @param n near面的距离
     * @param f far面的距离
     * @return
     */
    static inline Mat44 perspective(number angle, number aspect, number n, number f) {
        number q = 1.0f / tan(0.5f * angle);
        number A = q / aspect;
        number B = (n + f) / (n - f);
        number C = (2.0f * n * f) / (n - f);

        return {
            make_vec(A, 0, 0, 0),
            make_vec(0, q, 0, 0),
            make_vec(0, 0, -B, -1),
            make_vec(0, 0, -C, 0)};
    }

    /**
     * @brief 正交变换矩阵 Todo
     */
    static inline Mat44 ortho(number left, number right, number bottom, number top, number zNear, number zFar) {
        //        return {
        //            make_vec(2.0f / (right - left), 0.0f, 0.0f, 0.0f),
        //            make_vec(0.0f, 2.0f / (top - bottom), 0.0f, 0.0f),
        //            make_vec(0.0f, 0.0f, -2.0f / (zFar - zNear), 0.0f),
        //            make_vec(-(right + left) / (right - left), -(top + bottom) / (top - bottom), -(zFar + zNear) / (zFar - zNear), 1.0f)};
        number width = right - left, height = top - bottom, depth = zFar - zNear;
        return {
            make_vec(2 / width, 0, 0, -(right + left) / width),
            make_vec(0, 2 / height, 0, -(top + bottom) / height),
            make_vec(0, 0, -2 / depth, -(zFar + zNear) / depth),
            make_vec(0, 0, 0, 1)};
    }
#pragma endregion
};

} // namespace mne

#endif //MINI_ENGINE_UTILS_HPP
