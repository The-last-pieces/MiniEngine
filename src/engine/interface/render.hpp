//
// Created by IMEI on 2022/8/16.
//

#ifndef MINI_ENGINE_RENDER_HPP
#define MINI_ENGINE_RENDER_HPP

#include "../store/image.hpp"
#include "../data/camera.hpp"
#include "../store/model.hpp"

namespace mne {
// 渲染器接口,输入摄像机+光源+模型信息,输出图片
class IRender {
public:
    Camera camera; // 摄像机对象

    std::vector<std::shared_ptr<Model>> models{}; // 要渲染的模型集合

public:
    virtual void drawAt(BMPImage& image) = 0;
};
} // namespace mne

#endif //MINI_ENGINE_RENDER_HPP
