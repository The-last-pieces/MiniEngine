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

protected:
    AABB bbox; // 包围盒

    virtual void intersection(const Ray& ray, HitResult& hit) const = 0;

    // 图元从初始状态经历一个仿射变换
    virtual void setTransform(const Transform& transform) = 0;

    // Todo 更新包围盒
    virtual void updateAABB() {}

public:
    // 光源重要性采样,随机在物体表面上采样一个点
    virtual void sampleLight(LightResult& result) const = 0;

    // 表面积
    virtual number area() const = 0;

    // 是否为光源
    bool isLight() const { return material->isLight(); }

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

    static std::shared_ptr<IObject> load(
        std::shared_ptr<IObject>          obj,
        const std::shared_ptr<IMaterial>& material,
        const Transform&                  transform) {
        obj->setTransform(transform);
        obj->updateAABB();
        obj->material = material;
        return obj;
    }

    static std::shared_ptr<IObject> load(
        std::shared_ptr<IObject>          obj,
        const std::shared_ptr<IMaterial>& material = nullptr) {
        obj->material = material;
        return obj;
    }
};

} // namespace mne

#endif //MINI_ENGINE_OBJECT_HPP
