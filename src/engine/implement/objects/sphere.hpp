//
// Created by IMEI on 2022/8/18.
//

#ifndef MINI_ENGINE_SPHERE_HPP
#define MINI_ENGINE_SPHERE_HPP

#include <utility>

#include "../../interface/object.hpp"

namespace mne {

class Sphere final: public IObject {
    Vec3   center;
    number radius;

public:
    Sphere(const Vec3& _c, number _r):
        center(_c), radius(_r) {
        bbox = AABB{
            center - Vec3{radius, radius, radius},
            center + Vec3{radius, radius, radius}};
    }

public:
    // 随机在物体表面上采样一个点
    void sampleLight(LightResult& result) const final {
        result.normal   = VecUtils::sampleSphere();
        result.point    = center + radius * result.normal;
        result.emission = material->emission;
    }

    number area() const final {
        return 4 * pi * radius * radius;
    }

protected:
    bool intersection(const Ray& ray, HitResult& hit) const final {
        Vec3   c = center, o = ray.pos, d = ray.dir, oc = o - c;
        number r = radius;
        /* 解 : 
        (o+t*d-c)^2-r^2=0
        (d^2)t^2 + 2*(o-c)*d*t+(o-c)^2-r^2=0
        */
        number A = d.norm2(), B = 2 * oc * d, C = oc.norm2() - r * r;
        number delta = B * B - 4 * A * C;
        if (delta < 0) return false; // 无解
        number sq = std::sqrt(delta);
        number t1 = (-B - sq) / (2 * A), t2 = (-B + sq) / (2 * A);

        if (t1 > 0) {
            hit.tick = t1;
        } else if (t2 > 0) {
            hit.tick = t2;
        } else {
            return false;
        }
        hit.normal = (ray.at(hit.tick) - center).normalize();
        return true;
    }
};

} // namespace mne

#endif //MINI_ENGINE_SPHERE_HPP
