//
// Created by MnZn on 2022/7/13.
//

#ifndef MINI_ENGINE_IMAGE_HPP
#define MINI_ENGINE_IMAGE_HPP

#include "color.hpp"
#include "../math/vec.hpp"
#include <vector>
#include <fstream>
#include <Windows.h>

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

// 储存tga文件
class TGAImage {
    enum Format {
        GRAYSCALE = 1,
        RGB       = 3,
        RGBA      = 4
    };

    struct TGAColor {
        std::uint8_t color[4]{}; // blue , green , red , alpha
        std::uint8_t bytePerPixel{};

        TGAColor() = default;

        TGAColor(std::uint8_t R, std::uint8_t G, std::uint8_t B, std::uint8_t A = 255):
            color{B, G, R, A}, bytePerPixel(4) {}

        TGAColor(const std::uint8_t* p, int bpp):
            bytePerPixel(bpp) {
            for (int i = bpp; i--; color[i] = p[i])
                ;
        }

        std::uint8_t& operator[](int i) { return color[i]; }

        Color toColor() const {
            return Color::fromRGB256(color[2], color[1], color[0]);
        }
    };

#pragma pack(push, 1)
    struct TGAHeader {
        std::uint8_t  idSize{};
        std::uint8_t  colorMapType{};
        std::uint8_t  dataType{};
        std::uint16_t colorMapOrigin{};
        std::uint16_t colorMapSize{};
        std::uint8_t  colorMapDepth{};
        std::uint16_t xOrigin{};
        std::uint16_t yOrigin{};
        std::uint16_t width{};
        std::uint16_t height{};
        std::uint8_t  bitsPerPixel{};
        std::uint8_t  imageDescriptor{};
    };
#pragma pack(pop)

private:
    std::vector<std::uint8_t> data;

    int w{}, h{}, bpp{};

public:
    TGAColor getPixel(int i, int j) const {
#ifndef NDEBUG
        if (i < 0 || i >= h || j < 0 || j >= w)
            throw std::out_of_range("TGAImage::getPixel");
#endif
        return {&data[i * w * bpp + j * bpp], bpp};
    }

    // 通过进行插值的方式获取颜色信息 Todo
    Color getPixel(const Vec2& uv) const {
        auto tmp = getPixel(int(uv.y() * number(h)), int(uv.x() * number(w)));
        return tmp.toColor();
    }

public:
    void loadFromDisk(std::string_view tag_path) {
        std::ifstream in;
        in.open(tag_path.data(), std::ios::binary);
        if (!in.is_open()) {
            return;
        }
        TGAHeader header;
        in.read(reinterpret_cast<char*>(&header), sizeof(header));
        if (!in.good()) {
            return;
        }
        w   = header.width;
        h   = header.height;
        bpp = header.bitsPerPixel >> 3;
        if (w <= 0 || h <= 0 || (bpp != GRAYSCALE && bpp != RGB && bpp != RGBA)) {
            return;
        }
        int bSize = bpp * w * h;
        data.assign(bSize, 0);
        if (3 == header.dataType || 2 == header.dataType) {
            in.read(reinterpret_cast<char*>(data.data()), bSize);
            if (!in.good()) return;
        } else if (10 == header.dataType || 11 == header.dataType) {
            if (!loadRleData(in)) return;
        } else {
            in.close();
        }
        // Todo 处理翻转
        //        if (!(header.imageDescriptor & 0x20))
        //            ; //flip_vertically();
        //        if (header.imageDescriptor & 0x10)
        //            ; //flip_horizontally();
        in.close();
    }

private:
    bool loadRleData(std::istream& in) {
        size_t   pixelCount   = w * h;
        size_t   currentPixel = 0;
        size_t   currentByte  = 0;
        TGAColor colorBuffer;
        do {
            std::uint8_t chunkHeader = 0;
            chunkHeader              = in.get();
            if (!in.good()) {
                return false;
            }
            if (chunkHeader < 128) {
                chunkHeader++;
                for (int i = 0; i < chunkHeader; i++) {
                    in.read(reinterpret_cast<char*>(colorBuffer.color), bpp);
                    if (!in.good()) {
                        return false;
                    }
                    for (int t = 0; t < bpp; t++)
                        data[currentByte++] = colorBuffer.color[t];
                    currentPixel++;
                    if (currentPixel > pixelCount) {
                        return false;
                    }
                }
            } else {
                chunkHeader -= 127;
                in.read(reinterpret_cast<char*>(colorBuffer.color), bpp);
                if (!in.good()) {
                    return false;
                }
                for (int i = 0; i < chunkHeader; i++) {
                    for (int t = 0; t < bpp; t++)
                        data[currentByte++] = colorBuffer.color[t];
                    currentPixel++;
                    if (currentPixel > pixelCount) {
                        return false;
                    }
                }
            }
        } while (currentPixel < pixelCount);
        return true;
    }
};

} // namespace mne

#endif //MINI_ENGINE_IMAGE_HPP
