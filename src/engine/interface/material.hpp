//
// Created by IMEI on 2022/8/17.
//

#ifndef MINI_ENGINE_MATERIAL_HPP
#define MINI_ENGINE_MATERIAL_HPP

#include "data/color.hpp"
#include "data/ray.hpp"
#include "math/vec.hpp"
#include "texture.hpp"

namespace mne {

// in_dir和out_dir均为实际射线方向,不需要考虑关于normal的朝向

struct BxDFResult {
    Vec3   out_dir  = {};    // 出射方向
    Color  albedo   = {};    // 光线出射率,即物体的颜色
    number pdf      = 0_n;   // 概率密度
    bool   specular = false; // 是否为反射或折射
};

struct IMaterial {
protected:
    // 光源的纹理,即内部辐射的能量
    std::shared_ptr<ITexture> emission;

public:
    IMaterial(std::shared_ptr<ITexture> emission = nullptr):
        emission(std::move(emission)) {}

public:
    // 光照信息
    Color emit(const Vec2& uv) const { return emission ? emission->value(uv) : Color{}; }

    // 是否为光源
    bool isLight() const { return emission != nullptr; }

public:
    // 进行BxDF采样 . in_dir为入射方向 , normal为碰撞点信息
    virtual void sample(const Vec3& in_dir, const HitResult& hit, BxDFResult& bxdf) const {}
};

} // namespace mne

#endif //MINI_ENGINE_MATERIAL_HPP
