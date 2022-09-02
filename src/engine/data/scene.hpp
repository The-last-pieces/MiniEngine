//
// Created by IMEI on 2022/8/18.
//

#ifndef MINI_ENGINE_SCENE_HPP
#define MINI_ENGINE_SCENE_HPP

#include "../store/model.hpp"
#include "../interface/object.hpp"
#include "../material/mirror.hpp"
#include <numeric>

namespace mne {

class Scene {
public:
    std::vector<std::shared_ptr<IObject>> objects{}; // 要渲染的对象集合(光追使用此字段)
    std::vector<std::shared_ptr<Model>>   models{};  // 要渲染的模型集合(光栅化使用此字段)

    void addObject(std::shared_ptr<IObject> object) {
        objects.push_back(std::move(object));
    }

    // 射线检测 Todo 使用BVH优化
    bool rayCast(const Ray& ray, HitResult& hit) const {
        HitResult temp;
        bool      isHit = false;

        for (const auto& ptr : objects) {
            if (ptr->intersect(ray, temp)) {
                isHit = true;
                hit   = temp;
            }
        }
        return isHit;
    }

private:
    // 按面积比决定概率比然后选择光源
    const IObject& selectLight() const {
        std::vector<number> areas;
        for (auto& ptr : objects) areas.push_back(ptr->isLight() ? ptr->area() : 0_n);
        return *(objects[randChoose(areas)]);
    }

    // 返回光源采样的pdf
    number lightPDF() const {
        number areaSum = 0_n;
        for (auto& ptr : objects) {
            if (ptr->isLight()) {
                areaSum += ptr->area();
            }
        }
        return 1_n / areaSum;
    }

public:
    // 以-out_dir为观测方向 , hit为观测点 , 观测全局光照下的像素结果 . out_dir*n , in_dir*n >=0
    Color shade(const Vec3& out_dir, const HitResult& hit, int depth = 0) const {
        /// 配置 -----------------------------
        constexpr int    min_dep = 3, max_dep = 10; // 深度限制
        constexpr number p_rr = 0.8_n;              // 轮盘赌机制

        /// 简写 -----------------------------
        auto& obj = *(hit.obj);      // 观测点所在的图元
        auto& mat = *(obj.material); // 观测点的材质

        /// 递归终止条件 ----------------------
        if (obj.isLight()) return mat.emission.clamp(); // 直接观测到光源
        if (depth > max_dep) return Color{0, 0, 0};     // 超过最大深度

        /// 直接光照 --------------------------
        Color L_direct{};
        auto& light = selectLight(); // 随机选择一个光源

        LightResult ems; // 随机采样
        light.sampleLight(ems);

        auto l_in     = ems.point - hit.point; // 光线矢量
        auto l_in_dir = l_in.normalize();      // 光线方向

        HitResult hit2; // 检测是否被遮挡
        if (rayCast(Ray{hit.point, l_in_dir}, hit2) && hit2.obj == &light) {
            Color  f_r_l = mat.reflect(l_in_dir, out_dir, hit.normal); // 反射率
            number pdf_l = light.PDF();                                // 光源采样的pdf
            number dot   = hit.normal * l_in_dir;                      // 与观测点夹角
            number dot_l = -(ems.normal * l_in_dir);                   // 与光源夹角
            Color  le_l  = ems.emission;                               // 直接光照

            L_direct = (le_l * f_r_l) * (dot * dot_l / pdf_l / l_in.norm2());
        }

        /// 间接光照 --------------------------
        Color L_indirect{};
        if (depth > min_dep && randFloat() > p_rr) return L_direct; // 轮盘赌机制
        Vec3 in_dir = mat.sample(l_in_dir, out_dir, hit.normal);    // 随机采样一个方向
        // Todo 镜面反射材质需要特殊处理
        // 间接光照来自物体
        bool isMirror = dynamic_cast<MaterialMirror*>(&mat);
        if (rayCast(Ray{hit.point, in_dir}, hit2) && (isMirror || !hit2.obj->isLight())) {
            Color  f_r = mat.reflect(in_dir, out_dir, hit.normal); // 反射率
            number pdf = mat.PDF(in_dir, hit.normal);              // 采样的pdf
            number dot = hit.normal * in_dir;                      // 与观测点夹角
            Color  le  = shade(-in_dir, hit2, depth + 1);          // 递归计算间接光照

            L_indirect = (le * f_r) * (dot / pdf / p_rr);
        }

        /// 返回结果 --------------------------
        //  镜面材质只需要间接光照
        if (isMirror) return L_indirect;
        return (L_direct + L_indirect);
    }
};

} // namespace mne

#endif //MINI_ENGINE_SCENE_HPP
