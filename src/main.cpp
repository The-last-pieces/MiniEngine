#include "./engine/implement/render/rs_render.hpp"
#include "./engine/implement/render/rt_render.hpp"
#include "./engine/implement/objects/rectangle.hpp"
#include "./engine/implement/objects/sphere.hpp"
#include "./engine/implement/material/mirror.hpp"
#include "./engine/implement/texture/mapping.hpp"
#include "view/gui.hpp"

using namespace mne;

// 窗口尺寸
const int SCR_WIDTH  = 720;
const int SCR_HEIGHT = 720;

void show(bool ui, std::shared_ptr<IRender> render) {
    if (ui) {
        mne::MainWindow window(
            "MnZn's Graphics Engine",
            SCR_WIDTH, SCR_HEIGHT, render);
        window.show();
    } else {
        // Todo 自动递增
        auto path = R"(..\demo\out.png)";
        render->render();
        render->image->saveToDisk(path);
    }
}

void testRsRender() {
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
    std::string paths[][2] = {
        {
            R"(art\model\boggie\body.obj)",
            R"(art\model\boggie\body_diffuse.tga)",
        },
        {
            R"(art\model\boggie\eyes.obj)",
            R"(art\model\boggie\eyes_diffuse.tga)",
        },
        {
            R"(art\model\boggie\head.obj)",
            R"(art\model\boggie\head_diffuse.tga)",
        },
        //        {
        //            R"(art\model\african_head\african_head.obj)",
        //            R"(art\model\african_head\african_head_diffuse.tga)",
        //        },
    };

    auto render = std::make_shared<RsRender>();

    auto add_model = [&](std::shared_ptr<Model>& model) {
        // model->transform.rotate.y() = pi * 0.5 * 7 / 6;
        render->scene->models.push_back(model);
    };

    for (auto [obj, tga] : paths) {
        std::shared_ptr<Model> model = std::make_shared<Model>(obj);

        model->colorTexture = std::make_shared<TextureImage>(tga);
        model->shader       = std::make_shared<TextureShader>(std::ref(*model));

        add_model(model);
    }

    auto model    = std::make_shared<Model>(R"(art\model\floor.obj)");
    model->shader = std::make_shared<VertexShader>();
    add_model(model);

    auto camera = std::make_shared<Camera>(
        make_vec(0, 5, 30),
        make_vec(0, 0, 0),
        SCR_WIDTH, SCR_HEIGHT,
        MathUtils::deg2rad(50),
        0_n);

    render->camera = camera;

    if (false) // 不使用ui
        return show(false, render);
    return show(true, render);
}

void testRtRender() {
    // 材质
    //材质
    //-------------------------------------------------------------
    auto redDiffuseMat = std::make_shared<MaterialDiffuse>(
        Color::fromRGB256(161, 16, 12));
    auto greenDiffuseMat = std::make_shared<MaterialDiffuse>(
        Color::fromRGB256(36, 115, 23));
    auto blueDiffuseMat = std::make_shared<MaterialDiffuse>(
        Color::fromRGB256(0, 0, 255));
    auto whiteDiffuseMat = std::make_shared<MaterialDiffuse>(
        Color::fromRGB256(185, 181, 174));
    auto lightMat = std::make_shared<MaterialDiffuseLight>(
        Color::fromRGB256(255, 255, 255) * 125);
    auto mirrorMat = std::make_shared<MaterialMirror>(
        Color::fromRGB256(255, 255, 255) / 1);
    auto earthMat = std::make_shared<MaterialDiffuse>(
        std::make_shared<TextureImage>(R"(art\texture\earth.bmp)"));

    //坐标
    //-------------------------------------------------------------
    constexpr number rectW = 20_n, radius = 40_n;

    //球体坐标
    Vec3 o = make_vec(-100 + radius, -100 + radius, -100 + radius);
    // 矩形 Cornell Box 顶点坐标
    Vec3 A = make_vec(-100, 100, 100);
    Vec3 B = make_vec(-100, -100, 100);
    Vec3 C = make_vec(100, -100, 100);
    Vec3 D = make_vec(100, 100, 100);
    Vec3 E = make_vec(-100, 100, -100);
    Vec3 F = make_vec(-100, -100, -100);
    Vec3 G = make_vec(100, -100, -100);
    Vec3 H = make_vec(100, 100, -100);
    // 矩形灯光顶点坐标
    Vec3 L1 = make_vec(rectW, 99.999, rectW);
    Vec3 L2 = make_vec(-rectW, 99.999, rectW);
    Vec3 L3 = make_vec(-rectW, 99.999, -rectW);
    Vec3 L4 = make_vec(rectW, 99.999, -rectW);

    //物体
    //-------------------------------------------------------------
    // 球体
    auto redSphere = IObject::load(
        std::make_shared<Sphere>(o, radius),
        earthMat);
    auto redSphere1 = IObject::load(
        std::make_shared<Sphere>(o + make_vec(-radius * 1.5, sqrt(3) * radius * 1.5, 0), radius),
        mirrorMat);
    auto redSphere2 = IObject::load(
        std::make_shared<Sphere>(o + make_vec(radius * 1.5, sqrt(3) * radius * 1.5, 0), radius),
        mirrorMat);
    // Cornell Box
    auto bottomRectangle = IObject::load(
        std::make_shared<mne::Rectangle>(G, F, B, C),
        mirrorMat); //whiteDiffuseMat
    auto topRectangle = IObject::load(
        std::make_shared<mne::Rectangle>(H, D, A, E),
        mirrorMat); //whiteDiffuseMat
    auto leftRectangle = IObject::load(
        std::make_shared<mne::Rectangle>(A, B, F, E),
        mirrorMat); // redDiffuseMat
    auto rightRectangle = IObject::load(
        std::make_shared<mne::Rectangle>(C, D, H, G),
        mirrorMat); // greenDiffuseMat
    auto backRectangle = IObject::load(
        std::make_shared<mne::Rectangle>(E, F, G, H),
        mirrorMat); // whiteDiffuseMat
    auto frontRectangle = IObject::load(
        std::make_shared<mne::Rectangle>(A, B, C, D),
        mirrorMat); // whiteDiffuseMat

    //光源
    //-------------------------------------------------------------
    // 顶部光源
    auto lightRectangle = IObject::load(
        std::make_shared<mne::Rectangle>(L1, L2, L3, L4),
        lightMat);
    // 环境光源
    auto envLightRectangle = IObject::load(
        std::make_shared<mne::Rectangle>(make_vec(0, -100 + 40, 400), make_vec(0, 0, -1), make_vec(1, 0, 0), 100_n, 100_n),
        std::make_shared<MaterialDiffuseLight>(
            Color::fromRGB256(255, 255, 255) * 5));

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
    auto camera = std::make_shared<Camera>(
        make_vec(0, 0, 380), // make_vec(99, 99, 99)
        make_vec(0, 0, 0),
        SCR_WIDTH, SCR_HEIGHT,
        number(atan2(100, 380 - 100) * 2),
        0_n);

    auto render    = std::make_shared<RtRender>();
    render->scene  = scene;
    render->camera = camera;
    render->spp    = 50;

    //if (false) // 不使用ui
    return show(false, render);
    return show(true, render);
}

int main() {
    testRtRender();
    return 0;
}