//
// Created by IMEI on 2022/8/18.
//

#ifndef MINI_ENGINE_SCENE_HPP
#define MINI_ENGINE_SCENE_HPP

#include "../store/model.hpp"
#include "../interface/object.hpp"
#include <vector>

namespace mne {

// 负责图元对象的存储
class Scene {
public:
    // Todo 合并两个字段
    std::vector<std::shared_ptr<IObject>> objects{}; // 要渲染的对象集合(光追使用此字段)
    std::vector<std::shared_ptr<Model>>   models{};  // 要渲染的模型集合(光栅化使用此字段)

    void addObject(std::shared_ptr<IObject> object) {
        objects.push_back(std::move(object));
    }

    void addModel(std::shared_ptr<Model> model) {
        models.push_back(std::move(model));
    }
};

} // namespace mne

#endif //MINI_ENGINE_SCENE_HPP
