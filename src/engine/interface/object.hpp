//
// Created by IMEI on 2022/8/18.
//

#ifndef MINI_ENGINE_OBJECT_HPP
#define MINI_ENGINE_OBJECT_HPP

#include "../accelerator/AABB.hpp"
#include "../data/ray.hpp"
#include "../material/default.hpp"

namespace mne {

// 光源在某个点周围面积的采样结果
struct LightResult {
    Vec3 normal{}; // 表面法线
    Vec3 point{};  // 采样坐标
    Vec2 uv;       // 纹理坐标
};

class IObject {
public:
    std::shared_ptr<IMaterial> material{}; // 物体的材质

public:
    IObject(std::shared_ptr<IMaterial> _mat):
        material(std::move(_mat)) {
        if (!material) {
            material = std::make_shared<MaterialDefault>();
        }
    }

    // 是否为光源
    bool isLight() const {
        auto& emission = material->emission;
        return emission.r > 0 || emission.g > 0 || emission.b > 0;
    }

public:
    // 光源重要性采样,随机在物体表面上采样一个点
    virtual void sampleLight(LightResult& result) const = 0;

    // 表面积
    virtual number area() const = 0;

    // sampleLight的概率密度,即1/表面积
    number PDF() const { return 1_n / area(); }

    // 光线和物体的首个交点
    bool intersect(const Ray& ray, HitResult& hit) const {
        bool ret = bbox.intersect(ray) && intersection(ray, hit);
        if (ret) {
            hit.point = ray.at(hit.tick);
            hit.obj   = this;
        }
        return ret;
    }

protected:
    AABB bbox; // 包围盒

    virtual bool intersection(const Ray& ray, HitResult& hit) const { return false; }
};

} // namespace mne

#endif //MINI_ENGINE_OBJECT_HPP
