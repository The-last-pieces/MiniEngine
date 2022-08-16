//
// Created by MnZn on 2022/7/13.
//

#ifndef MINI_ENGINE_SHADER_HPP
#define MINI_ENGINE_SHADER_HPP

#include "../math/mat.hpp"
#include "../data/color.hpp"

namespace mne {

// 着色器接口
class IShader {
public:
    /**
     * @brief 顶点着色器
     * @param gl_Position 顶点坐标 . in-out
     * @param gl_TexCoord 纹理坐标 . in
     * @param gl_Transform 变换矩阵 . in
     * @param gl_Color 顶点颜色 . out , default {0,0,0}
     */
    virtual void vertex(
        Vec3&        gl_Position,
        const Vec2&  gl_TexCoord,
        const Mat44& gl_Transform,
        Color&       gl_Color) = 0;

    /**
     * @brief 片元着色器
     * @param gl_FragCoord 片段坐标(xy为窗口坐标z为深度). in
     * @param gl_TexCoord 纹理坐标 . in
     * @param gl_FragColor 片段颜色 . out , default {0,0,0}
     * @param gl_Discard 是否弃用片元着色器 . out , default false
     */
    virtual void fragment(
        const Vec3& gl_FragCoord,
        const Vec2& gl_TexCoord,
        Color&      gl_FragColor,
        bool&       gl_Discard) = 0;
};

} // namespace mne

#endif //MINI_ENGINE_SHADER_HPP
