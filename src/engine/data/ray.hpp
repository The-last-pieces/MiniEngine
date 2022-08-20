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

// 射线的碰撞结果
struct HitResult {
    Vec3   point;  // 碰撞点坐标
    Vec3   normal; // 碰撞面的法线
    number tick{}; // point = pos + tick * dir

    const IObject* obj{}; // 碰撞到的物体
};

} // namespace mne

#endif //MINI_ENGINE_RAY_HPP
