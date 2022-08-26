//
// Created by IMEI on 2022/8/25.
//

#ifndef MINI_ENGINE_MIRROR_HPP
#define MINI_ENGINE_MIRROR_HPP

#include "../interface/material.hpp"

namespace mne {

class MaterialMirror: public IMaterial {
public:
    MaterialMirror(const Color& emission = Color::fromRGB256(0, 0, 0),
                   const Color& albedo   = Color::fromRGB256(0, 0, 0)):
        IMaterial(emission, albedo) {
    }

    Vec3 sample(const Vec3& i, const Vec3& o, const Vec3& n) const final {
        return VecUtils::flapByVec(o, n);
    }

    // sample函数的概率密度 . dir为sample函数的返回值,n为表面法线方向
    number PDF(const Vec3& dir, const Vec3& n) const final {
        return 1;
    }

    // Todo 镜面反射高光过于严重
    // 返回在入射方向为i出射方向为o条件下的光线反射率 , n为表面法线方向
    Color reflect(const Vec3& i, const Vec3& o, const Vec3& n) const final {
        return albedo / (n * i);
    }
};

} // namespace mne

#endif //MINI_ENGINE_MIRROR_HPP
