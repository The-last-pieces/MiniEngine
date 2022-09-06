//
// Created by IMEI on 2022/9/5.
//

#ifndef MINI_ENGINE_CUBE_HPP
#define MINI_ENGINE_CUBE_HPP

#include "aggregate.hpp"
#include "rectangle.hpp"

namespace mne {

// 立方体
class Cube: public Aggregate {
public:
    void onSetTransform() final {
        children.clear();
        // 前后
        auto front = load(
            std::make_shared<Rectangle>(), material,
            Transform::Translate(make_vec(0, 0, 0.5)));
        auto back = load(
            std::make_shared<Rectangle>(), material,
            Transform::Translate(make_vec(0, 0, -0.5)));
        // 上下
        auto top = load(
            std::make_shared<Rectangle>(), material,
            Transform::RotateTranslate(make_vec(-pi_half, 0, 0), make_vec(0, 0.5, 0)));
        auto bottom = load(
            std::make_shared<Rectangle>(), material,
            Transform::RotateTranslate(make_vec(pi_half, 0, 0), make_vec(0, -0.5, 0)));
        // 左右
        auto left = load(
            std::make_shared<Rectangle>(), material,
            Transform::RotateTranslate(make_vec(0, -pi_half, 0), make_vec(-0.5, 0, 0)));
        auto right = load(
            std::make_shared<Rectangle>(), material,
            Transform::RotateTranslate(make_vec(0, pi_half, 0), make_vec(0.5, 0, 0)));
        // 添加到子对象中
        addChild({front, back, top, bottom, left, right});
    }
};

} // namespace mne

#endif //MINI_ENGINE_CUBE_HPP
