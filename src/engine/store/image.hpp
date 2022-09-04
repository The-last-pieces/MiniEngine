//
// Created by MnZn on 2022/7/13.
//

#ifndef MINI_ENGINE_IMAGE_HPP
#define MINI_ENGINE_IMAGE_HPP

#include "data/color.hpp"
#include "math/vec.hpp"
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "STB/stb_image.h"
#undef STB_IMAGE_IMPLEMENTATION

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "STB/stb_image_write.h"
#undef STB_IMAGE_WRITE_IMPLEMENTATION

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "STB/stb_image_resize.h"
#undef STB_IMAGE_RESIZE_IMPLEMENTATION

namespace mne {

// 通用图片,使用stb_image库
// 左下角为(0,0) , 右上角为(width-1, height-1)
class Image {
    static constexpr int bpp = 3; // bytesPerPixel

    std::vector<Color> data;

    int width{}, height{};

    void loadBuffer(stbi_uc* buffer, int w, int h) {
        resize(w, h);
        for (int i = height - 1, k = 0; i >= 0; --i) {
            for (int j = 0; j < width; j++, k++) {
                setPixel(j, i, Color::fromRGB256(buffer[k * bpp], buffer[k * bpp + 1], buffer[k * bpp + 2]));
            }
        }
    }

    std::vector<stbi_uc> generateBuffer() const {
        std::vector<uint8_t> buffer(width * height * bpp);
        for (int i = height - 1, k = 0; i >= 0; --i) {
            for (int j = 0; j < width; j++, k++) {
                auto c              = getPixel(j, i);
                buffer[k * bpp]     = int(c.r * 255_n);
                buffer[k * bpp + 1] = int(c.g * 255_n);
                buffer[k * bpp + 2] = int(c.b * 255_n);
            }
        }
        return buffer;
    }

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
            loadBuffer(buffer, w, h);
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

        auto buffer = generateBuffer();

        if (ext == "bmp") {
            return stbi_write_bmp(path.c_str(), width, height, bpp, buffer.data());
        } else if (ext == "png") {
            return stbi_write_png(path.c_str(), width, height, bpp, buffer.data(), 0);
        } else if (ext == "jpg") {
            return stbi_write_jpg(path.c_str(), width, height, bpp, buffer.data(), 100);
        } else return false;
    }

    Image scale(int nw, int nh) const {
        auto buf_in = generateBuffer();
        auto iw = width, ih = height;
        auto ow = nw, oh = nh;
        auto buf_out = std::vector<stbi_uc>(ow * oh * bpp);

        stbir_resize(buf_in.data(), iw, ih, 0, buf_out.data(), ow, oh, 0,
                     STBIR_TYPE_UINT8, bpp, STBIR_ALPHA_CHANNEL_NONE, 0,
                     STBIR_EDGE_CLAMP, STBIR_EDGE_CLAMP,
                     STBIR_FILTER_BOX, STBIR_FILTER_BOX,
                     STBIR_COLORSPACE_SRGB, nullptr);

        Image scaled;
        scaled.loadBuffer(buf_out.data(), ow, oh);
        return scaled;
    }

public:
    int getWidth() const { return width; }

    int getHeight() const { return height; }

    // 获取宽高
    std::pair<int, int> getWH() const { return {width, height}; }

    bool invalid(int x, int y) const { return x < 0 || x >= width || y < 0 || y >= height; }

public:
    // 二次插值获取像素信息
    Color getPixel(const Vec2& uv) const {
        auto   nw = number(width - 1), nh = number(height - 1);
        number x = uv.x() * nw, y = uv.y() * nh;
        int    x0 = MathUtils::mod_i((int) x, width), y0 = MathUtils::mod_i((int) y, height);
        int    x1 = (x0 + 1) % width, y1 = (y0 + 1) % height;
        number tx = MathUtils::mod(x - (number) x0, 1);
        number ty = MathUtils::mod(y - (number) y0, 1);
        return MathUtils::lerp(
            MathUtils::lerp(getPixel(x0, y0), getPixel(x1, y0), tx), // 水平插值
            MathUtils::lerp(getPixel(x0, y1), getPixel(x1, y1), tx), // 水平插值
            ty                                                       // 垂直插值
        );
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
};

} // namespace mne

#endif //MINI_ENGINE_IMAGE_HPP
