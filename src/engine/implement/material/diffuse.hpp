//
// Created by IMEI on 2022/8/18.
//

#ifndef MINI_ENGINE_DIFFUSE_HPP
#define MINI_ENGINE_DIFFUSE_HPP

#include "interface/material.hpp"
#include "implement/texture/solid.hpp"

namespace mne {

// 漫反射材质
class MaterialDiffuse: public IMaterial {
    std::shared_ptr<ITexture> albedo;

public:
    MaterialDiffuse(const Color& albedo = Color::fromRGB256(0, 0, 0)):
        albedo(std::make_shared<TextureSolid>(albedo)) {
    }

    MaterialDiffuse(std::shared_ptr<ITexture> albedo):
        albedo(std::move(albedo)) {
    }

    void sample(const Vec3& in_dir, const HitResult& hit, BxDFResult& bxdf) const final {
        bxdf.specular = false;
        bxdf.out_dir  = VecUtils::sampleHalfSphere(hit.normal);
        bxdf.albedo   = albedo->value(hit.uv) / pi2;
        bxdf.pdf      = (bxdf.out_dir * hit.normal) / pi2;
    }
};

// 漫光照材质
class MaterialDiffuseLight: public IMaterial {
public:
    MaterialDiffuseLight(const Color& emission):
        IMaterial(std::make_shared<TextureSolid>(emission)) {
    }
};

} // namespace mne

#endif //MINI_ENGINE_DIFFUSE_HPP
