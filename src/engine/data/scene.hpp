//
// Created by IMEI on 2022/8/18.
//

#ifndef MINI_ENGINE_SCENE_HPP
#define MINI_ENGINE_SCENE_HPP

#include "../store/model.hpp"
#include "../interface/object.hpp"
#include <numeric>

namespace mne {

// Todo 将渲染逻辑移动到rt_render中,这里只负责对象储存
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
    Color background = Color::fromRGB256(255, 255, 255); // 背景色/环境光

    // Todo 转循环
    // 以in_dir方向的射线打到hit上的全局光照信息
    Color shade(const Vec3& in_dir, const HitResult& hit, int depth = 0) const {
        /// 配置 -----------------------------
        constexpr int min_dep = 3, max_dep = 10; // 深度限制
        // Todo 意义何在?
        constexpr number p_rr = 1_n; // 轮盘赌机制

        /// 简写 -----------------------------
        auto& obj = *(hit.obj);   // 观测点所在的图元
        auto& mat = obj.matRef(); // 观测点的材质

        /// 递归终止条件 ----------------------
        if (obj.isLight()) return mat.emit(hit.uv).clamp(1_n); // 直接观测到光源
        if (depth > max_dep) return Color{0, 0, 0};            // 超过最大深度

        /// BxDF信息 -------------------------
        BxDFResult bxdf;
        mat.sample(in_dir, hit, bxdf);

        /// 间接光照 --------------------------
        Color     L_indirect{}, le{};
        HitResult hit2;
        if (rayCast(Ray{hit.point, bxdf.out_dir}, hit2) && (bxdf.specular || !hit2.obj->isLight())) {
            le = shade(bxdf.out_dir, hit2, depth + 1);
        } else {
            le = background * (hit2.obj ? 0_n : 0.3_n);
        }
        {
            Color  f_r = bxdf.albedo;               // 反射率
            number pdf = bxdf.pdf;                  // 采样的pdf
            number dot = hit.normal * bxdf.out_dir; // 与观测点夹角

            L_indirect = (le * f_r) * (dot / (pdf)); // * p_rr
        }
        //  镜面材质只需要间接光照
        if (bxdf.specular) return L_indirect;

        /// 直接光照 --------------------------
        Color L_direct{};
        auto& light = selectLight(); // 随机选择一个光源

        LightResult ems; // 随机采样
        light.sampleLight(ems);

        auto l_out     = ems.point - hit.point; // 光线矢量
        auto l_out_dir = l_out.normalize();     // 光线方向

        // 检测是否被遮挡
        if (rayCast(Ray{hit.point, l_out_dir}, hit2) && hit2.obj == &light) {
            Color  f_r_l = bxdf.albedo;                              // 反射率
            number pdf_l = lightPDF();                               // 光源采样的pdf, light.PDF();
            number dot   = hit.normal * l_out_dir;                   // 与观测点夹角
            number dot_l = std::max(0_n, -(ems.normal * l_out_dir)); // 与光源夹角
            Color  le_l  = light.matRef().emit(ems.uv);              // 直接光照

            L_direct = (le_l * f_r_l) * (dot * dot_l / (pdf_l * l_out.norm2()));
        }

        /// 返回结果 --------------------------
        return (L_direct + L_indirect);
    }
};

} // namespace mne

#endif //MINI_ENGINE_SCENE_HPP
