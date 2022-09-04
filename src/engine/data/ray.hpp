//
// Created by IMEI on 2022/8/17.
//

#ifndef MINI_ENGINE_RAY_HPP
#define MINI_ENGINE_RAY_HPP

#include "math/vec.hpp"

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
class HitResult {
public:
    Vec3 point;     // 点坐标
    Vec3 normal;    // 面法线
    Vec2 uv;        // 纹理坐标
    bool back{};    // 是否位于背面
    bool success{}; // 是否成功碰撞

    const IObject* obj{}; // 碰撞到的物体
private:
    number tick{}; // point = pos + tick * dir
    number min_tick = 0.001_n;
    number max_tick = inf;

public:
    // 重新开始一次采样
    void reset() { max_tick = inf, success = false, obj = nullptr; }

    // outSide为朝外的法线
    void setNormal(Vec3 outSide, const Ray& ray) {
        back   = (outSide * ray.dir) > 0;
        normal = back ? -outSide : outSide;
    }

    bool setTick(number val) {
        if (val > min_tick && val < max_tick) {
            return tick = max_tick = val, success = true;
        } else {
            return false;
        }
    }

    // 假定要求v1 < v2
    bool setTick(number v1, number v2) {
        return setTick(v1) || setTick(v2);
    }

    Vec3 getPoint(const Ray& ray) const {
        return ray.at(tick);
    }
};

} // namespace mne

#endif //MINI_ENGINE_RAY_HPP
