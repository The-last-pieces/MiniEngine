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

void runContext(const std::string& path) {
    RenderContext context;
    context.loadFromJson(JsonUtils::load(path));
    show(context.ui, context.render, context.savePath);
}

int main() {
    json task = JsonUtils::load("art/context/task.json");
    for (auto& path : task) runContext(path);
    return 0;
}