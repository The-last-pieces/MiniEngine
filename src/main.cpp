#include "view/gui.hpp"
#include "./engine/render/rs_render.hpp"
#include "./engine/render/rt_render.hpp"
#include "./engine/objects/rectangle.hpp"
#include "./engine/objects/sphere.hpp"
#include "./engine/material/mirror.hpp"

using namespace mne;

// 窗口尺寸
const int SCR_WIDTH  = 720;
const int SCR_HEIGHT = 720;

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
        gl_FragColor = model.colorTexture->value(gl_TexCoord);
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

void testRsRender() {
    std::string_view paths[][2] = {
        {
            R"(art\boggie\body.obj)",
            R"(art\boggie\body_diffuse.tga)",
        },
        {
            R"(art\boggie\eyes.obj)",
            R"(art\boggie\eyes_diffuse.tga)",
        },
        {
            R"(art\boggie\head.obj)",
            R"(art\boggie\head_diffuse.tga)",
        },
        //        {
        //            R"(art\african_head\african_head.obj)",
        //            R"(art\african_head\african_head_diffuse.tga)",
        //        },
    };

    auto render = std::make_shared<RsRender>();

    auto add_model = [&](std::shared_ptr<Model>& model) {
        model->transform.scale      = {50, 50, 50};
        model->transform.rotate.y() = pi;
        model->transform.rotate.z() = -pi / 2;
        render->scene->models.push_back(model);
    };

    for (auto [obj, tga] : paths) {
        std::shared_ptr<Model> model = std::make_shared<Model>(obj);

        model->colorTexture = std::make_shared<TextureImage>(tga);
        model->shader       = std::make_shared<TextureShader>(std::ref(*model));

        add_model(model);
    }

    auto model    = std::make_shared<Model>(R"(art\floor.obj)");
    model->shader = std::make_shared<VertexShader>();
    add_model(model);

    mne::MainWindow window(
        "MnZn's Graphics Engine",
        SCR_WIDTH, SCR_HEIGHT, render);
    window.show();
}

void testRtRender() {
    // 材质
    //材质
    //-------------------------------------------------------------

    //    auto redDiffuseMat = std::make_shared<MaterialDefault>(
    //        Color{}, Color::fromRGB256(255, 0, 0));
    //    auto greenDiffuseMat = std::make_shared<MaterialDefault>(
    //        Color{}, Color::fromRGB256(0, 255, 0));
    //    auto blueDiffuseMat = std::make_shared<MaterialDefault>(
    //        Color{}, Color::fromRGB256(0, 0, 255));
    //    auto whiteDiffuseMat = std::make_shared<MaterialDefault>(
    //        Color{}, Color::fromRGB256(255, 255, 255));
    //    auto lightMat = std::make_shared<MaterialDefault>(
    //        Color::fromRGB256(255, 255, 255) * 50, Color{});

    auto redDiffuseMat = std::make_shared<MaterialDefault>(
        Color{}, Color::fromRGB256(161, 16, 12));
    auto greenDiffuseMat = std::make_shared<MaterialDefault>(
        Color{}, Color::fromRGB256(36, 115, 23));
    auto blueDiffuseMat = std::make_shared<MaterialDefault>(
        Color{}, Color::fromRGB256(0, 0, 255));
    auto whiteDiffuseMat = std::make_shared<MaterialDefault>(
        Color{}, Color::fromRGB256(185, 181, 174));
    auto lightMat = std::make_shared<MaterialDefault>(
        Color::fromRGB256(255, 255, 255) * 50, Color{});
    auto mirrorMat = std::make_shared<MaterialMirror>(
        Color{}, Color::fromRGB256(255, 255, 255) / 1);

    //坐标
    //-------------------------------------------------------------
    const int scale = 1;
    const int rectW = 20;

    Vec3 o  = scale * make_vec(0, -100 + 40, 0); //球体坐标
    Vec3 A  = scale * make_vec(-100, 100, 100);  // 矩形 Cornell Box 顶点坐标
    Vec3 B  = scale * make_vec(-100, -100, 100);
    Vec3 C  = scale * make_vec(100, -100, 100);
    Vec3 D  = scale * make_vec(100, 100, 100);
    Vec3 E  = scale * make_vec(-100, 100, -100);
    Vec3 F  = scale * make_vec(-100, -100, -100);
    Vec3 G  = scale * make_vec(100, -100, -100);
    Vec3 H  = scale * make_vec(100, 100, -100);
    Vec3 L1 = scale * make_vec(rectW, 99.999, rectW); // 矩形灯光顶点坐标
    Vec3 L2 = scale * make_vec(-rectW, 99.999, rectW);
    Vec3 L3 = scale * make_vec(-rectW, 99.999, -rectW);
    Vec3 L4 = scale * make_vec(rectW, 99.999, -rectW);

    //物体
    //-------------------------------------------------------------
    // 球体
    auto redSphere = std::make_shared<Sphere>(o, scale * 30.f, mirrorMat);
    // Cornell Box
    auto bottomRectangle = std::make_shared<mne::Rectangle>(G, F, B, C, whiteDiffuseMat);
    auto topRectangle    = std::make_shared<mne::Rectangle>(H, D, A, E, whiteDiffuseMat);
    auto leftRectangle   = std::make_shared<mne::Rectangle>(A, B, F, E, redDiffuseMat);
    auto rightRectangle  = std::make_shared<mne::Rectangle>(C, D, H, G, greenDiffuseMat);
    auto backRectangle   = std::make_shared<mne::Rectangle>(E, F, G, H, whiteDiffuseMat);

    //光源
    //-------------------------------------------------------------
    auto lightRectangle = std::make_shared<mne::Rectangle>(L1, L2, L3, L4, lightMat); //面光源

    //场景
    //-------------------------------------------------------------
    auto scene = std::make_shared<Scene>();
    scene->addObject(redSphere);
    scene->addObject(bottomRectangle);
    scene->addObject(topRectangle);
    scene->addObject(leftRectangle);
    scene->addObject(rightRectangle);
    scene->addObject(backRectangle);
    scene->addObject(lightRectangle);

    //相机
    //-------------------------------------------------------------
    auto camera = std::make_shared<RtCamera>(
        make_vec(0, 0, 380),
        make_vec(0, 0, 0),
        make_vec(0, 1, 0),
        number(atan2(100, 380 - 100) * 2),
        number(SCR_WIDTH) / SCR_HEIGHT);

    auto render     = std::make_shared<RtRender>();
    render->scene   = scene;
    render->camera2 = camera;
    render->spp     = 100;

    render->camera->vh = SCR_HEIGHT;
    render->camera->vw = SCR_WIDTH;

    constexpr bool ui = true;
    if constexpr (ui) {
        mne::MainWindow window(
            "MnZn's Graphics Engine",
            SCR_WIDTH, SCR_HEIGHT, render);
        window.show();
    } else {
        auto path      = R"(..\demo\out.bmp)";
        *render->image = mne::BMPImage::loadFromDisk(path);
        render->render();
        render->image->saveToDisk(path);
    }
}

int main() {
    testRtRender();
    return 0;
}