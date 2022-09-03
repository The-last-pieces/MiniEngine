//
// Created by IMEI on 2022/8/16.
//

#ifndef MINI_ENGINE_RENDER_HPP
#define MINI_ENGINE_RENDER_HPP

#include "../store/image.hpp"
#include "../data/camera.hpp"
#include "../data/scene.hpp"

namespace mne {
// 渲染器接口,输入摄像机+光源+模型信息,输出图片
class IRender {
public:
    std::shared_ptr<Camera> camera; // 摄像机

    std::shared_ptr<Image> image = std::make_shared<Image>(); // 输出的图片
    std::shared_ptr<Scene> scene = std::make_shared<Scene>(); // 输入的场景

    // 背景色/环境光
    Color background = Color::fromRGB256(255, 255, 255) * 0.5_n;

    // 采样率(sample per pixel)
    int spp = 1;

public:
    virtual void render() = 0;
};
} // namespace mne

#endif //MINI_ENGINE_RENDER_HPP
