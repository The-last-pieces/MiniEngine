//
// Created by IMEI on 2022/8/17.
//

#ifndef MINI_ENGINE_MATERIAL_HPP
#define MINI_ENGINE_MATERIAL_HPP

#include "../data/color.hpp"
#include "../math/vec.hpp"
#include "../math/utils.hpp"

namespace mne {

struct IMaterial {
public:
    // 物体自身内部辐射的能量,即光源
    Color emission{};
    // 反射率系数,分别控制rgb的反射率,直观体现出来就是物体的颜色
    Color albedo{};

public:
    IMaterial(const Color& _emission, const Color& _albedo):
        emission(_emission), albedo(_albedo) {}

public:
    // 随机采样一个方向 . i为光源的采样方向,o为观察方向,n为表面法线方向
    virtual Vec3 sample(const Vec3& i, const Vec3& o, const Vec3& n) const = 0;

    // sample函数的概率密度 . dir为sample函数的返回值,n为表面法线方向
    virtual number PDF(const Vec3& dir, const Vec3& n) const = 0;

    // 返回在入射方向为i出射方向为o条件下的光线反射率 , n为表面法线方向
    virtual Color reflect(const Vec3& i, const Vec3& o, const Vec3& n) const = 0;

protected:
    /// 供子类使用的一些工具函数

    // 关于平面对折
    static Vec3 flapByFlat(Vec3 v, Vec3 n) {
        // v' - v = -2 *(v * n) * n
        return v - (2 * (v * n)) * n;
    }

    // 关于向量对折
    static Vec3 flapByVec(Vec3 v, Vec3 n) {
        // v + v' = 2 * (v * n) * n
        return (2 * (v * n)) * n - v;
    }

    // 在单位球上均匀采样
    static Vec3 sampleSphere() {
        // https://www.cnblogs.com/cofludy/p/5894270.html
        while (true) {
            number u = randFloat(-1, 1), v = randFloat(-1, 1);
            number r2 = u * u + v * v;
            if (r2 < 1) {
                number sqr2 = sqrt(1 - r2);
                number x    = 2 * u * sqr2;
                number y    = 2 * v * sqr2;
                number z    = 1 - 2 * r2;
                return make_vec(x, y, z);
            }
        }
    }

    // 在法线为n的半球均匀采样
    static Vec3 sampleHalfSphere(Vec3 n) {
        // 在球面均匀采样
        Vec3 dir = sampleSphere();
        // 映射到+n上
        if (dir * n < 0) {
            dir = flapByFlat(dir, n);
        }
        // 转到n所在平面
        return dir;
    }
};

} // namespace mne

#endif //MINI_ENGINE_MATERIAL_HPP
