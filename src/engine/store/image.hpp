//
// Created by MnZn on 2022/7/13.
//

#ifndef MINI_ENGINE_IMAGE_HPP
#define MINI_ENGINE_IMAGE_HPP

#include "../math/vec.hpp"
#include "../data/color.hpp"
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

namespace mne {

// 通用图片,使用stb_image库
class Image {
    static constexpr int bpp = 3; // bytesPerPixel

public:
    Image() = default;

    Image(int w, int h, Color fill = {}) {
        resize(w, h, fill);
    }

    Image(const std::string& path) {
        loadFromDisk(path);
    }

    void resize(int w, int h, Color fill = {}) {
        data.assign(w * h, fill);
        width = w, height = h;
    }

    bool loadFromDisk(const std::string& path) {
        int   w, h, c;
        auto* buffer = stbi_load(path.c_str(), &w, &h, &c, bpp);
        if (buffer) {
            resize(w, h);
            for (int i = height - 1, k = 0; i >= 0; --i) {
                for (int j = 0; j < width; j++, k++) {
                    setPixel(j, i, Color::fromRGB256(buffer[k * bpp], buffer[k * bpp + 1], buffer[k * bpp + 2]));
                }
            }
            stbi_image_free(buffer);
            return true;
        } else {
            resize(0, 0);
            return false;
        }
    }

    bool saveToDisk(const std::string& path) const {
        auto it = path.find_last_of('.');
        if (it == std::string::npos) return false;
        std::string ext = path.substr(it + 1);

        std::vector<uint8_t> buffer(width * height * bpp);
        for (int i = height - 1, k = 0; i >= 0; --i) {
            for (int j = 0; j < width; j++, k++) {
                auto c              = getPixel(j, i);
                buffer[k * bpp]     = int(c.r * 255_n);
                buffer[k * bpp + 1] = int(c.g * 255_n);
                buffer[k * bpp + 2] = int(c.b * 255_n);
            }
        }

        if (ext == "bmp") {
            return stbi_write_bmp(path.c_str(), width, height, bpp, buffer.data());
        } else if (ext == "png") {
            return stbi_write_png(path.c_str(), width, height, bpp, buffer.data(), 0);
        } else if (ext == "jpg") {
            return stbi_write_jpg(path.c_str(), width, height, bpp, buffer.data(), 100);
        } else return false;
    }

public:
    int getWidth() const { return width; }

    int getHeight() const { return height; }

    // 获取宽高
    std::pair<int, int> getWH() const { return {width, height}; }

    bool invalid(int x, int y) const { return x < 0 || x >= width || y < 0 || y >= height; }

public:
    // Todo 二次插值获取像素信息
    Color getPixel(const Vec2& uv) const {
        int x = clamp(0, int(uv.x() * number(width - 1)), width - 1);
        int y = clamp(0, int(uv.y() * number(height - 1)), height - 1);
        return getPixel(x, y);
    }

    Color getPixel(int x, int y) const {
#ifndef NDEBUG
        if (invalid(x, y)) throw std::out_of_range("Color::getPixel");
#endif
        return data[x * height + y];
    }

    void setPixel(int x, int y, const Color& color) {
#ifndef NDEBUG
        if (invalid(x, y)) throw std::out_of_range("Color::setPixel");
#endif
        data[x * height + y] = color.clamp();
    }

private:
    std::vector<Color> data;

    int width{}, height{};
};

} // namespace mne

#endif //MINI_ENGINE_IMAGE_HPP
