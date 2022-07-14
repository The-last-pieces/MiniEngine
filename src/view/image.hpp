//
// Created by MnZn on 2022/7/13.
//

#ifndef MINI_ENGINE_IMAGE_HPP
#define MINI_ENGINE_IMAGE_HPP

#include <vector>
#include "color.hpp"

namespace mne {

// 储存bmp图片
class BMPImage {
    std::vector<std::vector<Color>> data;

    int h, w;

public:
    BMPImage(int h, int w, Color fill = {}):
        data(h, std::vector<Color>(w, fill)), h(h), w(w) {
    }

public:
    std::pair<int, int> size() const {
        return {h, w};
    }

    Color getPixel(int i, int j) const {
        return data[i][j];
    }

    void setPixel(int i, int j, const Color& color) {
        data[i][j] = color;
    }

    void saveToDisk(std::string_view path) {
        // 保存为bmp图片到硬盘中 Todo
    }
};

} // namespace mne

#endif //MINI_ENGINE_IMAGE_HPP
