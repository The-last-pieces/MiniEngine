//
// Created by IMEI on 2022/8/16.
//

#ifndef MINI_ENGINE_TEXTURE_HPP
#define MINI_ENGINE_TEXTURE_HPP

#include "../data/color.hpp"

namespace mne {

// 纹理
class ITexture {
public:
    // u,v为纹理坐标,范围为[0,1]
    virtual Color value(const Vec2& uv) const = 0;
};

} // namespace mne

#endif //MINI_ENGINE_TEXTURE_HPP
