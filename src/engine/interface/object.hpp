//
// Created by IMEI on 2022/8/18.
//

#ifndef MINI_ENGINE_OBJECT_HPP
#define MINI_ENGINE_OBJECT_HPP

#include "../accelerator/AABB.hpp"
#include "../data/ray.hpp"
#include "../implement/material/default.hpp"

namespace mne {

// 光源在某个点周围面积的采样结果
struct LightResult {
    Vec3 normal{}; // 表面法线
    Vec3 point{};  // 采样坐标
    Vec2 uv;       // 纹理坐标
};

class IObject {
protected:
    std::shared_ptr<IMaterial> material{}; // 物体的材质

    void setMaterial(std::shared_ptr<IMaterial> mat) {
        if (!mat) mat = std::make_shared<MaterialDefault>();
        material = std::move(mat);
    }

public:
    const IMaterial& matRef() const { return *material; }

public:
    IObject() { setMaterial(nullptr); }

    // 是否为光源
    bool isLight() const { return material->isLight(); }

public:
    // 光源重要性采样,随机在物体表面上采样一个点
    virtual void sampleLight(LightResult& result) const = 0;

    // 表面积
    virtual number area() const = 0;

public:
    // sampleLight的概率密度,即1/表面积
    number PDF() const { return 1_n / area(); }

    // 光线和物体的首个交点
    bool intersect(const Ray& ray, HitResult& hit) const {
        hit.success = false;
        bool ret    = bbox.intersect(ray) && (intersection(ray, hit), hit.success);
        hit.obj     = ret ? this : nullptr;
        return ret;
    }

    template<class T>
    static std::shared_ptr<T> load(
        std::shared_ptr<T>                obj,
        const std::shared_ptr<IMaterial>& material, const Mat44& transform)
        requires((std::is_base_of_v<IObject, T>) ) {
        obj->afterTransform(transform);
        obj->material = material;
        return obj;
    }

    template<class T>
    static std::shared_ptr<T> load(
        std::shared_ptr<T>                obj,
        const std::shared_ptr<IMaterial>& material = nullptr)
        requires((std::is_base_of_v<IObject, T>) ) {
        obj->material = material;
        return obj;
    }

protected:
    AABB bbox; // 包围盒

    virtual void intersection(const Ray& ray, HitResult& hit) const {}

    // 图元经历一个仿射变换,如果矩阵不是仿射变换则行为未定义
    virtual void afterTransform(const Mat44& transform) {
    }
};

} // namespace mne

#endif //MINI_ENGINE_OBJECT_HPP
