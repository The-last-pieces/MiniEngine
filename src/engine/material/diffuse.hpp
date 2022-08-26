//
// Created by IMEI on 2022/8/18.
//

#ifndef MINI_ENGINE_DIFFUSE_HPP
#define MINI_ENGINE_DIFFUSE_HPP

#include "../interface/material.hpp"

namespace mne {

// 漫反射材质
class MaterialDiffuse: public IMaterial {
public:
    MaterialDiffuse(const Color& emission = Color::fromRGB256(0, 0, 0),
                    const Color& albedo   = Color::fromRGB256(0, 0, 0)):
        IMaterial(emission, albedo) {
    }

    Vec3 sample(const Vec3& light_dir, const Vec3& out_dir, const Vec3& normal) const final {
        return VecUtils::sampleHalfSphere(normal);
    }

    // sample函数的概率密度 . dir为sample函数的返回值,n为表面法线方向
    number PDF(const Vec3& dir, const Vec3& normal) const final {
        return (dir * normal) / pi; // Todo pi or 2pi?
        //return 1 / (number(2) * pi);
    }

    // 返回在入射方向为i出射方向为o条件下的光线反射率 , n为表面法线方向
    Color reflect(const Vec3& in_dir, const Vec3& out_dir, const Vec3& normal) const final {
        return albedo / pi;
        //return albedo / (number(2) * pi);
    }
};

} // namespace mne

#endif //MINI_ENGINE_DIFFUSE_HPP
