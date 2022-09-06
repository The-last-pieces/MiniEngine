//
// Created by IMEI on 2022/9/3.
//

#ifndef MINI_ENGINE_AGGREGATE_HPP
#define MINI_ENGINE_AGGREGATE_HPP

#include "interface/object.hpp"
#include "rectangle.hpp"

namespace mne {

// 聚合对象
class Aggregate: public IObject {
protected:
    void intersection(const Ray& ray, HitResult& hit) const override {
        HitResult temp = hit;
        for (const auto& ptr : children) {
            if (ptr->intersect(ray, temp)) {
                hit = temp;
            }
        }
    }

    // Todo 随机采样
    void sampleLight(LightResult&) const final {}

    number area() const final {
        number sum = 0_n;
        for (auto& ptr : children) sum += ptr->area();
        return sum;
    }
};

} // namespace mne

#endif //MINI_ENGINE_AGGREGATE_HPP
