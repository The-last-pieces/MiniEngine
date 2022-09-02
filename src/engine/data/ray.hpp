//
// Created by IMEI on 2022/8/17.
//

#ifndef MINI_ENGINE_RAY_HPP
#define MINI_ENGINE_RAY_HPP

#include "../math/vec.hpp"

namespace mne {

// 射线,由光源发出
struct Ray {
    Vec3 pos; // 起点
    Vec3 dir; // 方向

    Vec3 at(number tick) const {
        return pos + tick * dir;
    }

    // 和平面的交点tick
    number flat(Vec3 c, Vec3 n) const {
        // (o + t * d - c) n = 0
        Vec3 oc = c - pos;
        return (n * oc) / (n * dir);
    }
};

class IObject;

// 射线碰撞点信息
struct HitResult {
    Vec3 point;  // 点坐标
    Vec3 normal; // 面法线
    Vec2 uv;     // 纹理坐标
    bool back{}; // 是否位于背面

    const IObject* obj{}; // 碰撞到的物体

    // outSide为朝外的法线
    void setNormal(Vec3 outSide, const Ray& ray) {
        back   = (outSide * ray.dir) > 0;
        normal = back ? -outSide : outSide;
    }

    void setTick(number val) {
        if (val > min_tick && val < max_tick) {
            tick = max_tick = val;
        }
    }

    Vec3 getPoint(const Ray& ray) const {
        return ray.at(tick);
    }

private:
    number tick{}; // point = pos + tick * dir
    number min_tick = 1_n, max_tick = inf;
};

} // namespace mne

#endif //MINI_ENGINE_RAY_HPP
