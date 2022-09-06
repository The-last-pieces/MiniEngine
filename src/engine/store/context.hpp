//
// Created by IMEI on 2022/9/3.
//

#ifndef MINI_ENGINE_CONTEXT_HPP
#define MINI_ENGINE_CONTEXT_HPP

#include "implement/render/rt_render.hpp"
#include "implement/render/rs_render.hpp"

#include "implement/objects/sphere.hpp"
#include "implement/objects/rectangle.hpp"
#include "implement/objects/cube.hpp"

#include "implement/material/diffuse.hpp"
#include "implement/material/mirror.hpp"
#include "implement/material/refraction.hpp"

#include "implement/texture/solid.hpp"
#include "implement/texture/mapping.hpp"

#include "implement/shader/simple.hpp"

#include "tools/json.hpp"
#include <filesystem>
#include <map>

namespace mne {

// 渲染器运行的上下文
class RenderContext {
public:
    bool ui = true;

    std::string savePath;

    std::shared_ptr<IRender> render;

private:
    std::map<std::string, json> constants;

    std::map<std::string, std::shared_ptr<IMaterial>> materials;

public:
    // 从json配置导入渲染场景
    void loadFromJson(const json& config) {
        try {
            load(config);
        } catch (const std::exception& err) {
            printf("loadFromJson error: %s", err.what());
        }
    }

private:
    void load(const json& config) {
        /// 加载render字段 ---------------------
        json render = config.at("render");
        // 是否开启ui
        this->ui = render.value("ui", false);
        // 创建渲染器
        this->render = toRender(render.at("type"));
        // 填充基础字段
        this->render->spp        = render.at("spp");
        this->render->background = toColor(render.at("background"));

        /// 加载image字段 -----------------------
        json image = config.at("image");
        // 加载文件名
        std::string sceneName = image.at("sceneName"), fileSuffix = image.at("fileSuffix");
        this->savePath = makeSavePath(sceneName, fileSuffix, image.at("version"), this->render->spp);
        // 图片尺寸
        int width = image.at("width"), height = image.at("height");

        /// 加载camera字段 ----------------------
        json   camera  = config.at("camera");
        Vec3   eye_pos = toVec3(camera.at("eye"));
        Vec3   look_at = toVec3(camera.at("target"));
        number rotate  = MathUtils::deg2rad(camera.value("rotate", 0_n));
        number fov     = MathUtils::deg2rad(camera.at("fov"));
        // 创建摄像机
        this->render->camera = std::make_shared<Camera>(eye_pos, look_at, width, height, fov, rotate);

        /// 加载场景字段 ------------------------
        auto&& scene   = config.at("scene");
        json   vars    = scene.value("vars", json::object());
        json   objects = scene.value("objects", json::object());
        json   models  = scene.value("models", json::array());
        // 引入符号表
        loadVars(vars);
        // Todo 优化上下文结构和导入逻辑
        // 加载物体
        for (auto&& [type, objs] : objects.items()) {
            for (auto& obj : objs) {
                if (!obj.value("hide", false)) {
                    this->render->scene->addObject(toObject(type, obj));
                }
            }
        }
        // 加载模型
        for (auto& model : models) {
            if (!model.value("hide", false)) {
                this->render->scene->addModel(toModel(model));
            }
        }
    }

    // Todo 所有变量都要支持常量表查询
    // 加载constants表
    void loadConstants(const json& obj) {
        for (auto&& [name, val] : obj.items()) {
            check(!materials.contains(name), "constant name conflict", true);
            constants[name] = val;
        }
    }

    // 引入其他vars表
    void loadVars(const json& obj) {
        // 加载常量
        loadConstants(obj.value("constants", json::object()));
        // 引入其他符号表
        for (auto& path : obj.value("imports", json::array())) loadVars(JsonUtils::load(path));
        // 加载材质
        loadMaterials(obj.value("materials", json::object()));
    }

    // 加载materials表
    void loadMaterials(const json& obj) {
        for (auto&& [type, items] : obj.items()) {
            for (auto&& [name, obj] : items.items()) {
                std::shared_ptr<IMaterial> ptr = buildMaterial(type, obj);
                check(!materials.contains(name), "material name conflict", true);
                materials[name] = ptr;
            }
        }
    }

private:
    static std::shared_ptr<IRender> toRender(const std::string& type) {
        if (type == "rt") {
            return std::make_shared<RtRender>();
        } else if (type == "rs") {
            return std::make_shared<RsRender>();
        } else {
            throw std::runtime_error("render type error");
        }
    }

    number toNumber(const json& obj) {
        if (obj.is_string()) { // 查询变量表
            auto it = constants.find(obj);
            check(it != constants.end(), "constants not found");
            return it->second;
        }
        return obj;
    }

    Color toColor(const json& obj) {
        // 查询变量表
        if (obj.is_string()) return toColor(getConstantArray(obj));
        check(obj.is_array(), "color must be array");
        int n = (int) obj.size();
        check(n == 3, "color must be array of 3");
        return Color::fromRGB256(toNumber(obj[0]), toNumber(obj[1]), toNumber(obj[2]));
    }

    Vec3 toVec3(const json& obj) {
        // 查询变量表
        if (obj.is_string()) return toVec3(getConstantArray(obj));
        check(obj.is_array(), "vec3 must be array");
        int n = (int) obj.size();
        check(n == 3, "vec3 must be array of 3");
        return Vec3{toNumber(obj[0]), toNumber(obj[1]), toNumber(obj[2])};
    }

    Vec2 toVec2(const json& obj) {
        // 查询变量表
        if (obj.is_string()) return toVec2(getConstantArray(obj));
        check(obj.is_array(), "vec2 must be array");
        int n = (int) obj.size();
        check(n == 2, "vec2 must be array of 2");
        return Vec2{toNumber(obj[0]), toNumber(obj[1])};
    }

    std::shared_ptr<IObject> toObject(const std::string& type, const json& obj) {
        // json structure = obj.at("structure"); Todo 构造需要的参数
        std::shared_ptr<IObject> ret{};
        // 构造基础对象
        if (type == "sphere") {
            ret = std::make_shared<Sphere>();
        } else if (type == "flat") {
            ret = std::make_shared<Rectangle>();
        } else if (type == "cube") {
            ret = std::make_shared<Cube>();
        } else {
            throw error("object type error");
        }
        // 将细节转移到afterTransform中
        return IObject::load(ret, toMaterial(obj.at("material")), toTransform(obj.value("transform", json::object())));
    }

    std::shared_ptr<Model> toModel(const json& obj) {
        std::string objPath     = obj.at("objPath");
        std::string texturePath = obj.at("texturePath");
        std::string shaderType  = obj.at("shaderType");

        std::shared_ptr<Model> model = std::make_shared<Model>(objPath);

        model->colorTexture = std::make_shared<TextureImage>(texturePath);
        if (shaderType == "fragment") {
            model->shader = std::make_shared<ShaderTexture>(std::ref(*model));
        } else if (shaderType == "vertex") {
            model->shader = std::make_shared<ShaderVertex>();
        } else {
            throw error("shader type error");
        }
        model->transform = toTransform(obj.value("transform", json::object()));

        return model;
    }

    std::shared_ptr<IMaterial> toMaterial(const json& obj) {
        if (obj.is_string()) { // 查询材质表
            auto it = materials.find(obj);
            check(it != materials.end(), "material not found");
            return it->second;
        }
        // 构造临时材质
        return buildMaterial(obj.at("type"), obj);
    }

    Transform toTransform(const json& obj) {
        Transform transform;
        // 缩放
        transform.scale = toVec3(obj.value("scale", "[1,1,1]"_json));
        // 旋转
        json rotate = obj.value("rotate", "[0,0,0]"_json);
        if (rotate.size() == 2) {
            // 转欧拉角
            transform.rotate = MatUtils::angle2xyz(MathUtils::deg2rad(toVec2(rotate)));
        } else {
            transform.rotate = MathUtils::deg2rad(toVec3(rotate));
        }
        // 平移
        transform.offset = toVec3(obj.value("translate", "[0,0,0]"_json));
        return transform;
    }

private:
    json getConstantArray(const std::string& name) {
        auto it = constants.find(name);
        check(it != constants.end(), "constant not found");
        check(it->second.is_array(), "constant type error");
        return it->second;
    }

    std::shared_ptr<IMaterial> buildMaterial(const std::string& type, const json& obj) {
        // "diffuse" | "diffuse_light" | "mirror" | "refract"
        if (type == "diffuse") {
            if (obj.count("solid")) {
                return std::make_shared<MaterialDiffuse>(toColor(obj.at("solid")));
            } else {
                return std::make_shared<MaterialDiffuse>(std::make_shared<TextureImage>(obj.at("image")));
            }
        } else if (type == "diffuse_light") {
            return std::make_shared<MaterialDiffuseLight>(toColor(obj.at("emit")));
        } else if (type == "mirror") {
            return std::make_shared<MaterialMirror>(toColor(obj.at("albedo")));
        } else if (type == "refract") {
            return std::make_shared<MaterialRefraction>(obj.at("index"));
        } else {
            throw error("material type error");
        }
    }

    static std::string makeSavePath(const std::string& sceneName, const std::string& fileSuffix, int version, int spp) {
        // 文件夹 = `../result/${sceneName}` , 文件名 = `v{version}_spp{spp}.{fileSuffix}`
        std::string dir, path;
        dir += "result/", dir += sceneName;
        std::filesystem::create_directories(dir);
        path += "/v", path += std::to_string(version);
        path += "_spp", path += std::to_string(spp);
        path += ".", path += fileSuffix;
        return dir + path;
    }

    static void check(bool cond, const std::string& msg, bool warn = false) {
        if (!cond) {
            if (warn) printf("Warning: %s\n", msg.c_str());
            else throw error(msg);
        }
    }

    static std::exception error(const std::string& msg) {
        return std::runtime_error(msg);
    }
};

} // namespace mne

#endif //MINI_ENGINE_CONTEXT_HPP
