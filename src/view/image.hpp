//
// Created by MnZn on 2022/7/13.
//

#ifndef MINI_ENGINE_IMAGE_HPP
#define MINI_ENGINE_IMAGE_HPP

#include "color.hpp"
#include <vector>
#include <fstream>
#include <windows.h>

#undef min
#undef max

namespace mne {

// 储存bmp图片
class BMPImage {
private:
    std::vector<Color> data;

    int ih{}, iw{}; // 高宽

public:
    BMPImage() = default;

    BMPImage(int h, int w, Color fill = {}) {
        init(h, w, fill);
    }

    void init(int h, int w, Color fill = {}) {
        data.assign(h * w, fill);
        ih = h, iw = w;
    }

public:
    std::pair<int, int> size() const {
        return {ih, iw};
    }

    Color getPixel(int i, int j) const {
#ifndef NDEBUG
        if (i < 0 || i >= ih || j < 0 || j >= iw)
            throw std::out_of_range("Color::getPixel");
#endif
        return data[i * iw + j];
    }

    void setPixel(int i, int j, const Color& color) {
#ifndef NDEBUG
        if (i < 0 || i >= ih || j < 0 || j >= iw)
            throw std::out_of_range("Color::setPixel");
#endif
        data[i * iw + j] = color;
    }

    void saveToDisk(std::string_view path) {
        // 保存为bmp图片到硬盘中 Todo
    }

    static BMPImage loadFromDisk(std::string_view path) {
        // 从硬盘中读取bmp图片
        BITMAPFILEHEADER bmpFile; // 文件头
        BITMAPINFOHEADER bmpInfo; // 内容头

        FILE* file;
        if (!fopen_s(&file, path.data(), "rb")) return {};

        // 读取文件头
        fread((char*) &bmpFile, sizeof(bmpFile), 1, file);
        fread((char*) &bmpInfo, sizeof(bmpInfo), 1, file);
        if (bmpInfo.biBitCount != 32) return {};

        // 尺寸数据
        int w = bmpInfo.biWidth, h = -bmpInfo.biHeight;
        int pixel_size  = bmpInfo.biBitCount / CHAR_BIT;
        int buffer_size = w * h * pixel_size;

        BMPImage image(h, w);
        auto*    buf = new uint8_t[buffer_size];

        for (int i = h - 1; i >= 0; --i) {
            uint8_t rgba[4]{};
            for (int j = 0; j < w; ++j) {
                fread((char*) rgba, 1, 4, file);
                auto [b, g, r, _] = rgba;
                image.setPixel(i, j, Color::fromRGB256(r, g, b));
            }
        }

        delete[] buf;
        return image;
    }
};

} // namespace mne

#endif //MINI_ENGINE_IMAGE_HPP
