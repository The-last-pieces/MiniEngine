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
    Vec3 offset{};

    Mat44 get_matrix() const {
        return MatUtils::merge(
            MatUtils::rotateX(rotate.x()).as4(),
            MatUtils::rotateY(rotate.y()).as4(),
            MatUtils::rotateZ(rotate.z()).as4(),
            MatUtils::scaleXYZ(scale).as4(),
            MatUtils::translate(offset));
    }
};

} // namespace mne

#endif //MINI_ENGINE_TRANSFORM_HPP
