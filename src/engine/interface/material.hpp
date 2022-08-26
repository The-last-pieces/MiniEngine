//
// Created by IMEI on 2022/8/17.
//

#ifndef MINI_ENGINE_MATERIAL_HPP
#define MINI_ENGINE_MATERIAL_HPP

#include "../data/color.hpp"
#include "../math/vec.hpp"

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
    // 随机采样一个方向 . light_dir为光线方向 , out_dir为观察方向 , normal为表面法线方向
    virtual Vec3 sample(const Vec3& light_dir, const Vec3& out_dir, const Vec3& normal) const = 0;

    // sample函数的概率密度 . dir为sample函数的返回值,normal为表面法线方向
    virtual number PDF(const Vec3& dir, const Vec3& normal) const = 0;

    // 返回反射率 . in_dir为入射方向 , out_dir为出射方向 , normal为表面法线方向
    virtual Color reflect(const Vec3& in_dir, const Vec3& out_dir, const Vec3& normal) const = 0;
};

} // namespace mne

#endif //MINI_ENGINE_MATERIAL_HPP
