//
// Created by IMEI on 2022/8/18.
//

#ifndef MINI_ENGINE_RECTANGLE_HPP
#define MINI_ENGINE_RECTANGLE_HPP

#include "../../interface/object.hpp"

namespace mne {

// Todo 重构结构 Vec3 leftBottom, left , up
// 矩形
class Rectangle: public IObject {
    Vec3   center; // 矩形中心
    Vec3   normal; // 法线
    Vec3   wd, hd; // 宽高的方向向量,
    number w, h;   // 宽高的一半

public:
    Rectangle(Vec3 _c, Vec3 _n, Vec3 _wd, number w, number h):
        center(_c), normal(_n), wd(_wd), hd(normal.cross(wd)), w(w), h(h) {}

    Rectangle(Vec3 p1, Vec3 p2, Vec3 p3, Vec3 p4) {
        Vec3 ps[] = {p2, p3, p4};
        std::sort(std::begin(ps), std::end(ps), [&](const Vec3& a, const Vec3& b) {
            return (a - p1).norm2() < (b - p1).norm2();
        });
        p2 = ps[0], p3 = ps[1], p4 = ps[2];
        center = (p1 + p4) / 2;

        Vec3 p21 = p2 - p1, p31 = p3 - p1;

        wd = p21.normalize(), w = p21.length() / 2;
        hd = p31.normalize(), h = p31.length() / 2;

        normal = wd.cross(hd);
    }

public:
    // 随机在物体表面上采样一个点
    void sampleLight(LightResult& result) const final {
        result.normal = normal;
        result.point  = center + wd * (RandomUtils::randFloat(-1_n, 1_n) * w) + hd * (RandomUtils::randFloat(-1_n, 1_n) * h);
        result.uv     = mapping_uv(result.point);
    }

    number area() const final { return 4_n * w * h; }

protected:
    void intersection(const Ray& ray, HitResult& hit) const final {
        // 和平面求交
        number tick = ray.flat(center, normal);
        if (tick < 0_n) return;
        hit.point = ray.at(tick);
        hit.uv    = mapping_uv(hit.point);
        if (hit.uv.v_min() < 0 || hit.uv.v_max() > 1) return;
        hit.setTick(tick);
        hit.setNormal(normal, ray);
    }

private:
    Vec2 mapping_uv(const Vec3& p) const {
        // 求偏移量
        number ow = wd * (p - center);
        number oh = hd * (p - center);
        return {ow / (w * 2) + 0.5_n, oh / (h * 2) + 0.5_n};
    }
};

} // namespace mne

#endif //MINI_ENGINE_RECTANGLE_HPP
