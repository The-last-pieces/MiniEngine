//
// Created by IMEI on 2022/8/18.
//

#ifndef MINI_ENGINE_RECTANGLE_HPP
#define MINI_ENGINE_RECTANGLE_HPP

#include "interface/object.hpp"

namespace mne {

// 矩形
class Rectangle: public IObject {
    Vec3 leftBottom = make_vec(-0.5_n, -0.5_n, 0); // 左下角
    // +x和+y轴, z=normal
    Vec3   x = VecUtils::X, y = VecUtils::Y, z = VecUtils::Z;
    number width{}, height{}; // x长度宽,y长度高

public:
    Rectangle() = default;

public:
    // 随机在物体表面上采样一个点
    void sampleLight(LightResult& result) const final {
        result.normal = z;
        result.point  = leftBottom + x * (RandomUtils::randFloat() * width) + y * (RandomUtils::randFloat() * height);
        result.uv     = mapping_uv(result.point);
    }

    number area() const final { return width * height; }

protected:
    void intersection(const Ray& ray, HitResult& hit) const final {
        // 和平面求交
        number tick = ray.flat(leftBottom, z);
        if (tick < 0_n) return;
        hit.point = ray.at(tick);
        hit.uv    = mapping_uv(hit.point);
        if (hit.uv.v_min() < 0 || hit.uv.v_max() > 1) return;
        hit.setTick(tick);
        hit.setNormal(z, ray);
    }

    void onSetTransform() final {
        // 更新宽高轴
        std::tie(x, y) = std::make_tuple(
            dToWorld(VecUtils::X),
            dToWorld(VecUtils::Y));
        std::tie(width, height) = std::make_pair(x.length(), y.length());
        x /= width, y /= height, z = x.cross(y);
        leftBottom = pToWorld(make_vec(-0.5_n, -0.5_n, 0));
    }

private:
    Vec2 mapping_uv(const Vec3& p) const {
        // 求偏移量
        Vec3   vc = p - leftBottom;
        number ox = x * vc, oy = y * vc;
        return {ox / width, oy / height};
    }
};

} // namespace mne

#endif //MINI_ENGINE_RECTANGLE_HPP
