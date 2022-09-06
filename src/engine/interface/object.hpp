//
// Created by IMEI on 2022/8/18.
//

#ifndef MINI_ENGINE_OBJECT_HPP
#define MINI_ENGINE_OBJECT_HPP

#include "data/ray.hpp"
#include "data/transform.hpp"
#include "data/xyz.hpp"
#include "accelerator/AABB.hpp"
#include "implement/material/default.hpp"

namespace mne {

// 光源在某个点周围面积的采样结果
struct LightResult {
    Vec3 normal{}; // 表面法线
    Vec3 point{};  // 采样坐标
    Vec2 uv;       // 纹理坐标
};

class IObject {
public:
    static inline int frame = 0;

protected:
    // Todo 是否需要引入weak_ptr
    // 对象树架构
    IObject*                              parent{};
    std::vector<std::shared_ptr<IObject>> children{};
    // 物体自身的坐标系,物体的坐标全都相对此坐标系
    XYZ xyz_p;
    // 只处理方向的坐标系,o为原点
    XYZ xyz_d;

    // 点的实际坐标
    Vec3 pToWorld(Vec3 point) const {
        auto* cur = this;
        while (cur) {
            point = cur->xyz_p.toWorld(point);
            cur   = cur->parent;
        }
        return point;
    }

    // 向量的实际指向,带长度
    Vec3 dToWorld(Vec3 dir) const {
        auto* cur = this;
        while (cur) {
            dir = cur->xyz_d.toWorld(dir);
            cur = cur->parent;
        }
        return dir;
    }

    // 图元从初始状态经历一个仿射变换
    void setTransform(const Transform& transform) {
        // 重设两个坐标系
        xyz_p = XYZ(transform);
        xyz_d = xyz_p.origin();
        updateVec();
    }

private:
    // 更新位置信息
    void updateVec() {
        onSetTransform();
        updateAABB();
        for (auto& child : children) child->updateVec();
    }

    // 更新transform后的回调,更新绝对坐标
    virtual void onSetTransform() {}

public:
    void addChild(const std::shared_ptr<IObject>& child) {
        if (child->parent) child->parent->removeChild(child);
        child->parent = this; // Todo shared_from_this();
        child->updateVec();
        children.push_back(child);
    }

    void addChild(const std::initializer_list<std::shared_ptr<IObject>>& list) {
        for (auto& child : list) addChild(child);
    }

    void removeChild(const std::shared_ptr<IObject>& child) {
        auto it = std::find(children.begin(), children.end(), child);
        if (it == children.end()) return;
        children.erase(it);
        child->parent = nullptr;
        child->updateVec();
    }

protected:
    std::shared_ptr<IMaterial> material{}; // 物体的材质

public:
    void setMaterial(std::shared_ptr<IMaterial> mat) {
        if (!mat) mat = std::make_shared<MaterialDefault>();
        material = mat;
    }

public:
    const IMaterial& matRef() const { return *material; }

public:
    IObject() { setMaterial(nullptr); }

protected:
    AABB bbox; // 包围盒

    virtual void intersection(const Ray& ray, HitResult& hit) const = 0;

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
        obj->setMaterial(material);
        obj->setTransform(transform);
        return obj;
    }

    static std::shared_ptr<IObject> load(
        std::shared_ptr<IObject>          obj,
        const std::shared_ptr<IMaterial>& material = nullptr) {
        obj->setMaterial(material);
        return obj;
    }
};

} // namespace mne

#endif //MINI_ENGINE_OBJECT_HPP
