//
// Created by MnZn on 2022/7/16.
//

#ifndef MINI_ENGINE_UTILS_HPP
#define MINI_ENGINE_UTILS_HPP

#include <random>
#include <ctime>

namespace mne {

/// 通用工具函数

// [l,r)
inline int randInt(int l, int r) {
    static auto seed  = 0;                  // time(nullptr);
    static auto maker = std::mt19937(seed); // NOLINT(cert-msc51-cpp)
    return int(maker() % (r - l)) + l;
}

// [0,n)
inline int randInt(int n) {
    return randInt(0, n);
}

// [0,1)
inline number randFloat() {
    return (number) randInt(RAND_MAX) / number(RAND_MAX);
}

// [l,r)
inline number randFloat(number l, number r) {
    return l + (r - l) * randFloat();
}

// 约束v到[l,r]区间
template<class T>
constexpr T clamp(const T& l, const T& v, const T& r) {
    return std::max(l, std::min(r, v));
}

// 弧度转角度
constexpr number rad2deg(number rad) {
    return rad * 180_n / pi;
}

// 角度转弧度
constexpr number deg2rad(number deg) {
    return deg * pi / 180_n;
}

/// Vec的工具函数
class VecUtils {
public:
    // 关于平面对折
    static constexpr Vec3 flapByFlat(Vec3 v, Vec3 n) {
        // v' - v = -2 *(v * n) * n
        return v - (2 * (v * n)) * n;
    }

    // 关于向量对折
    static constexpr Vec3 flapByVec(Vec3 v, Vec3 n) {
        // v + v' = 2 * (v * n) * n
        return (2 * (v * n)) * n - v;
    }

    // 在单位球上均匀采样
    static Vec3 sampleSphere() {
        // https://blog.csdn.net/yjr3426619/article/details/102706968
        number a = randFloat(), b = randFloat();
        // 随机生成转角和仰角
        number phi = 2 * pi * a, theta = acos(b);
        number x = cos(theta) * cos(phi);
        number y = cos(theta) * sin(phi);
        number z = sin(theta);
        if (randInt(2)) z = -z;
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
        number phi    = 2 * pi * randFloat();
        number z      = std::fabs(1.0f - 2 * randFloat()); //[0,1]
        number radius = std::sqrt(1.0f - z * z);           //[0,1]

        number x = radius * std::cos(phi);
        number y = radius * std::sin(phi);

        // Todo 转换到世界坐标系意义注释
        return toWorld(make_vec(x, y, z), n);
    }
};

} // namespace mne

#endif //MINI_ENGINE_UTILS_HPP
