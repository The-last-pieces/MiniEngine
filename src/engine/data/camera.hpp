//
// Created by MnZn on 2022/7/19.
//

#ifndef MINI_ENGINE_CAMERA_HPP
#define MINI_ENGINE_CAMERA_HPP

#include "../math/vec.hpp"
#include "../math/mat.hpp"

namespace mne {

// 摄像机
class Camera {
public:
    Vec3 eye_pos{0, 0, -700}; // 视线位置
    Vec3 eye_dir{0, 0, 1};    // 视线方向
    Vec3 eye_norm{0, 1, 0};   // 视线平面法线

    int    vh{800}, vw{800};     // 视口大小height&width
    number dn{0.1f}, df{800.0f}; // 视锥范围near&far
private:
    Mat44 view_mat{};   // 视图矩阵
    Mat44 proj_mat{};   // 投影矩阵
    Mat44 screen_mat{}; // 屏幕矩阵
private:
    // 观察变换矩阵,世界坐标转观察坐标
    Mat44 calViewMat() const {
        // eye_dir->+z
        auto to_z = Factory::rotateToZ(eye_dir);
        // eye_norm->+y
        auto fact_norm = (to_z * eye_norm).as<2>();
        // 绕z轴变换到+y方向
        auto to_y = Factory::rotateZ(fact_norm.rotate({0, 1}));
        // eye_pos->原点
        return Factory::merge(Factory::translate(-eye_pos), to_z.as4(), to_y.as4());
    }

    // 投影变换矩阵(透视/正交),观察坐标转裁剪坐标([-1,1]范围内)
    Mat44 calProjectionMat() const {
        //Todo z轴未投影到[-1,1]
        return Factory::perspective(pi / 2, number(vw) / number(vh), dn, df);
        //return Factory::ortho(0, number(vw), 0, number(vh), dn, df); Todo 正交投影
    }

    // 视口变换矩阵,视口坐标转屏幕坐标
    Mat44 calScreenMat() const {
        return Factory::merge(
            Factory::scaleXYZ(make_vec(vw, vh, getDepth())).as4(),
            Factory::translate(make_vec(vw / 2, vh / 2, 0)));
    }

public:
    // 更新缓存
    void update() {
        view_mat   = calViewMat();
        proj_mat   = calProjectionMat();
        screen_mat = calScreenMat();
    }

    // 视锥的深度
    number getDepth() const { return df - dn; }

    // 观察变换矩阵,世界坐标转观察坐标
    const Mat44& getViewMat() const { return view_mat; }

    // 投影变换矩阵(透视/正交),观察坐标转裁剪坐标([-1,1]范围内)
    const Mat44& getProjectionMat() const { return proj_mat; }

    // 视口变换矩阵,视口坐标转屏幕坐标
    const Mat44& getScreenMat() const { return screen_mat; }
};

} // namespace mne

#endif //MINI_ENGINE_CAMERA_HPP
