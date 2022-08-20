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
    std::shared_ptr<Camera>   camera = std::make_shared<Camera>();   // 摄像机
    std::shared_ptr<Scene>    scene  = std::make_shared<Scene>();    // 场景
    std::shared_ptr<BMPImage> image  = std::make_shared<BMPImage>(); // 输出

    int spp = 1; // 采样率(sample per pixel)

public:
    virtual void render() = 0;
};
} // namespace mne

#endif //MINI_ENGINE_RENDER_HPP
