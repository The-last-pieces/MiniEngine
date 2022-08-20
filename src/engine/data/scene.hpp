//
// Created by IMEI on 2022/8/18.
//

#ifndef MINI_ENGINE_SCENE_HPP
#define MINI_ENGINE_SCENE_HPP

#include "../store/model.hpp"
#include "../interface/object.hpp"
#include "../objects/rectangle.hpp"
#include <numeric>

namespace mne {

class Scene {
public:
    std::vector<std::shared_ptr<IObject>> objects{}; // 要渲染的对象集合(光追使用此字段)
    std::vector<std::shared_ptr<Model>>   models{};  // 要渲染的模型集合(光栅化使用此字段)

    void addObject(std::shared_ptr<IObject> object) {
        objects.push_back(std::move(object));
    }

public:
    // 以wr为观测方向 , hit为观测点 , 观测全局光照下的像素结果 . wr wi点乘n>=0
    Color shade(Vec3 wr, const HitResult& hit) const {
        constexpr int    times = 8;
        constexpr number prr   = 1 - 1.f / times; // 轮盘赌概率

        auto* obj = hit.obj;
        auto& mat = *obj->material;

        if (obj->isLight()) {
            return mat.emission;
        }

        Color lr = {};         // 出射光线
        Vec3  n  = hit.normal; // 法线
        Vec3  p  = hit.point;  // 观测点

        HitResult hit2;

        /// 对发光object采样

        auto& light = (Rectangle&) (*objects.back());

        LightResult res;
        light.sampleLight(res);
        auto [li_l, n_l, p_l] = res;

        Vec3 wi_l = (p_l - p).normalize();

        // 检测是否能被光源照射
        if (rayCast(Ray{p, wi_l}, hit2) && hit2.obj == &light) {
            Color  f_r_l = mat.reflect(wi_l, wr, n);
            number pdf_l = light.PDF();

            lr = lr + li_l.bitMut(f_r_l) * ((n * wi_l) * (n_l * -wi_l) / (p_l - p).norm2() / pdf_l);
        }

        /// 对不发光的物体进行采样

        // 轮盘赌
        if (randFloat() > prr) return lr;

        Vec3 wi = mat.sample(wi_l, wr, n); // 随机采样的入射方向

        if (rayCast(Ray{hit.point, wi}, hit2) && !hit2.obj->isLight()) {
            Color  li  = shade(-wi, hit2);       // 外部光照
            number dot = n * wi;                 // 夹角
            Color  f_r = mat.reflect(wi, wr, n); // 反射光
            number pdf = mat.PDF(wi, n);         // wi的概率密度

            lr = lr + li.bitMut(f_r) * (dot / pdf / prr);
        }

        return lr;
    }

    // 射线检测 Todo 使用BVH优化
    bool rayCast(const Ray& ray, HitResult& hit) const {
        HitResult temp;
        bool      isHit = false;
        number    tick  = std::numeric_limits<number>::max();

        for (const auto& ptr : objects) {
            if (ptr->intersect(ray, temp) && temp.tick > 0.01 && temp.tick < tick) { //
                isHit = true;
                tick  = temp.tick;
                hit   = temp;
            }
        }
        return isHit;
    }
};

} // namespace mne

#endif //MINI_ENGINE_SCENE_HPP
