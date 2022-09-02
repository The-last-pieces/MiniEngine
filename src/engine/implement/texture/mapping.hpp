//
// Created by IMEI on 2022/8/30.
//

#ifndef MINI_ENGINE_MAPPING_HPP
#define MINI_ENGINE_MAPPING_HPP

#include "../../interface/texture.hpp"
#include "../../store/image.hpp"

namespace mne {

// 将一张图片映射到u,v坐标上
class TextureImage: public ITexture {
    Image image;
    int   w{}, h{};

public:
    TextureImage(const std::string& path):
        image(path) {
        std::tie(w, h) = image.getWH();
    }

    Color value(const Vec2& uv) const override {
        return image.getPixel(uv);
    }
};

} // namespace mne

#endif //MINI_ENGINE_MAPPING_HPP
