//
// Created by IMEI on 2022/9/4.
//

#ifndef MINI_ENGINE_SIMPLE_HPP
#define MINI_ENGINE_SIMPLE_HPP

#include "interface/shader.hpp"
#include "store/model.hpp"

namespace mne {

// 极简shader实现

// 映射纹理信息
class ShaderTexture: public IShader {
    Model& model;

public:
    ShaderTexture(Model& model):
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

// 插值顶点信息
class ShaderVertex: public IShader {
public:
    void vertex(
        Vec3&        gl_Position,
        const Vec2&  gl_TexCoord,
        const Mat44& gl_Transform,
        Color&       gl_Color) final {
        gl_Position = gl_Transform * gl_Position;
        gl_Color    = {gl_TexCoord.x(), gl_TexCoord.y(), 0};
    }
    void fragment(
        const Vec3& gl_FragCoord,
        const Vec2& gl_TexCoord,
        Color&      gl_FragColor,
        bool&       gl_Discard) final {
        gl_Discard = true;
    }
};

} // namespace mne

#endif //MINI_ENGINE_SIMPLE_HPP
