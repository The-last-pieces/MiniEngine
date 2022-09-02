//
// Created by IMEI on 2022/8/31.
//

#ifndef MINI_ENGINE_SOLID_HPP
#define MINI_ENGINE_SOLID_HPP

#include "../../interface/texture.hpp"

namespace mne {

// 纯色纹理
class TextureSolid: public ITexture {
    Color emission;

public:
    TextureSolid(const Color& _emission):
        emission(_emission) {}

    Color value(const Vec2& uv) const override {
        return emission;
    }
};

} // namespace mne

#endif //MINI_ENGINE_SOLID_HPP
