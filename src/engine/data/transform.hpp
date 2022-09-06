//
// Created by IMEI on 2022/9/3.
//

#ifndef MINI_ENGINE_TRANSFORM_HPP
#define MINI_ENGINE_TRANSFORM_HPP

#include "math/utils.hpp"

namespace mne {

struct Transform {
    Vec3 scale{1, 1, 1};
    Vec3 rotate{};
    Vec3 translate{};

    Transform(const Vec3& scale = {1, 1, 1}, const Vec3& rotate = {}, const Vec3& translate = {}):
        scale(scale), rotate(rotate), translate(translate) {}

    Mat44 get_matrix() const {
        return MatUtils::merge(
            MatUtils::scaleXYZ(scale).as4(),
            MatUtils::rotateXYZ(rotate).as4(),
            MatUtils::translate(translate));
    }

public:
    static Transform Scale(const Vec3& scale) {
        Transform transform;
        return transform.scale = scale, transform;
    }

    static Transform Rotate(const Vec3& rotate) {
        Transform transform;
        return transform.rotate = rotate, transform;
    }

    static Transform Translate(const Vec3& translate) {
        Transform transform;
        return transform.translate = translate, transform;
    }

    static Transform RotateTranslate(const Vec3& rotate, const Vec3& translate) {
        Transform transform;
        return transform.rotate = rotate, transform.translate = translate, transform;
    }
};

} // namespace mne

#endif //MINI_ENGINE_TRANSFORM_HPP
