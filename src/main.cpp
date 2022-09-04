#include "engine/store/context.hpp"
#include "view/gui.hpp"

using namespace mne;

void show(bool ui, const std::shared_ptr<IRender>& render, const std::string& path) {
    if (ui) {
        auto [w, h] = render->camera->getWH();
        mne::MainWindow window("MnZn's Graphics Engine", w, h, render);
        window.show();
    } else {
        render->render();
        render->image->saveToDisk(path);
    }
}

int main() {
    // Todo 使用json文件配置执行哪些context
    RenderContext context;
    context.loadFromJson(JsonUtils::load("art/context/corner_box.json"));
    show(context.ui, context.render, context.savePath);
    return 0;
}