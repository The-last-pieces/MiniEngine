#include "view/gui.hpp"

using namespace mne;

// 窗口尺寸
const int SCR_WIDTH  = 800;
const int SCR_HEIGHT = 800;

// 转发纹理的shader
class TextureShader: public IShader {
    Model& model;

public:
    TextureShader(Model& model):
        model(model) {
    }

    void vertex(
        Vec3&        gl_Position,
        const Vec2&  gl_TexCoord,
        const Mat44& gl_Transform,
        Color&       gl_Color) final {
        gl_Position = gl_Transform * gl_Position;
    }
    void fragment(
        const Vec3& gl_FragCoord,
        const Vec2& gl_TexCoord,
        Color&      gl_FragColor,
        bool&       gl_Discard) final {
        gl_FragColor = model.colorTexture.getPixel(gl_TexCoord);
    }
};

// 使用顶点着色的shader
class VertexShader: public IShader {
public:
    void vertex(
        Vec3&        gl_Position,
        const Vec2&  gl_TexCoord,
        const Mat44& gl_Transform,
        Color&       gl_Color) final {
        gl_Position = gl_Transform * gl_Position;
        gl_Color    = {gl_TexCoord.x(), gl_TexCoord.y(), 0};
        // Color::fromRGB256(randInt(256), randInt(256), randInt(256));
    }
    void fragment(
        const Vec3& gl_FragCoord,
        const Vec2& gl_TexCoord,
        Color&      gl_FragColor,
        bool&       gl_Discard) final {
        gl_Discard = true;
    }
};

int main() {
    std::string_view paths[][2] = {
        {
            R"(res\boggie\body.obj)",
            R"(res\boggie\body_diffuse.tga)",
        },
        {
            R"(res\boggie\eyes.obj)",
            R"(res\boggie\eyes_diffuse.tga)",
        },
        {
            R"(res\boggie\head.obj)",
            R"(res\boggie\head_diffuse.tga)",
        },
        //        {
//            R"(res\african_head\african_head.obj)",
//            R"(res\african_head\african_head_diffuse.tga)",
//        },
    };

    auto render = std::make_shared<Render>();

    auto add_model = [&](std::shared_ptr<Model>& model) {
        model->transform.scale      = {50, 50, 50};
        model->transform.rotate.y() = pi;
        model->transform.rotate.z() = -pi / 2;
        render->models.push_back(model);
    };

    for (auto [obj, tga] : paths) {
        std::shared_ptr<Model> model = std::make_shared<Model>(obj);
        model->colorTexture.loadFromDisk(tga);
        model->shader = std::make_shared<TextureShader>(std::ref(*model));

        add_model(model);
    }

    auto model    = std::make_shared<Model>(R"(res\floor.obj)");
    model->shader = std::make_shared<VertexShader>();
    add_model(model);

    mne::MainWindow window(
        "MnZn's Graphics Engine",
        SCR_WIDTH, SCR_HEIGHT, render);
    window.show();
    return 0;
}