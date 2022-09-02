//
// Created by IMEI on 2022/8/25.
//

#ifndef MINI_ENGINE_MIRROR_HPP
#define MINI_ENGINE_MIRROR_HPP

#include "../../interface/material.hpp"

namespace mne {

class MaterialMirror: public IMaterial {
    Color albedo;

public:
    MaterialMirror(const Color& albedo):
        albedo(albedo) {
    }

    void sample(const Vec3& in_dir, const HitResult& hit, BxDFResult& bxdf) const final {
        bxdf.specular = true;
        bxdf.out_dir  = VecUtils::flapByVec(-in_dir, hit.normal);             // 镜面反射
        bxdf.albedo   = (albedo / (bxdf.out_dir * hit.normal + eps)).clamp(); // 菲涅尔效应
        bxdf.pdf      = 1_n;
    }
};

} // namespace mne

#endif //MINI_ENGINE_MIRROR_HPP
