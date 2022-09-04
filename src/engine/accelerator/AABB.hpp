//
// Created by IMEI on 2022/8/18.
//

#ifndef MINI_ENGINE_AABB_HPP
#define MINI_ENGINE_AABB_HPP

#include "math/vec.hpp"
#include "data/ray.hpp"

namespace mne {

struct AABB {
    Vec3 min;
    Vec3 max;

    // Todo 使用AABB优化射线检测
    bool intersect(const Ray& ray) const {
        return true;
    }
};

} // namespace mne

#endif //MINI_ENGINE_AABB_HPP
