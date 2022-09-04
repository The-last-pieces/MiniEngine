//
// Created by MnZn on 2022/7/19.
//

#ifndef MINI_ENGINE_CAMERA_HPP
#define MINI_ENGINE_CAMERA_HPP

#include "data/ray.hpp"
#include "math/utils.hpp"

namespace mne {

/*
右手坐标系约定 : 
+x为屏幕从左到右的方向
+y为屏幕从下到上的方向
+z为屏幕从里到外的方向
(0,0,0)为屏幕中心坐标
*/

// 标准摄像机
class Camera {
public:
    Vec3 eye_pos;          // 摄像机位置
    Vec3 view_right;       // 视图右方向(+x)
    Vec3 view_up;          // 视图上方向(+y)
    Vec3 view_inner;       // 视图内方向(-z)
    Vec3 view_left_bottom; // 视图的左下角(0,0)

    int view_width{}, view_height{}; // 屏幕大小

    number view_near = 0.1_n, view_far = 800.0_n; // 视锥范围near&far
    number aspect_ratio = 1_n, fov;

public:
    Camera(const Vec3& eye_pos,    // 摄像机的位置
           const Vec3& target_pos, // 观测点的位置,成像后位于图片中心
           int width, int height,  // 屏幕大小
           number fov,             // 仰角的两倍,[0,pi]
           number rotate           // up_dir绕view_inner的旋转角度
           ):
        eye_pos(eye_pos),
        fov(fov), view_width(width), view_height(height) {
        // 宽高比
        aspect_ratio = number(width) / number(height);
        /// 求出三个标架
        // 视线方向 -z
        view_inner = (target_pos - eye_pos).normalize();
        // 头顶方向 +y
        view_up = VecUtils::afterRotate(VecUtils::Up, VecUtils::dir2angle(-view_inner)).normalize();
        view_up = MatUtils::rotate(view_inner, rotate) * view_up;
        // 右手方向 +x , xy=z , yz=+x=-zy
        view_right = view_inner.cross(view_up);

        // 离eye_pos一个单位距离的视图宽高
        number norm_view_height = 2 * std::tan(fov / 2);
        number norm_view_width  = aspect_ratio * norm_view_height;
        // 求出宽高矢量
        view_up *= norm_view_height;
        view_right *= norm_view_width;
        // 求出左下角
        view_left_bottom = eye_pos + view_inner - view_right / 2_n - view_up / 2_n;
    }

    std::pair<int, int> getWH() const { return {view_width, view_height}; }

public:
    // For RsRender

    // 将世界坐标转换为观察坐标
    Mat44 getViewMat() const {
        return MatUtils::merge(
            MatUtils::translate(-eye_pos),
            Mat33{view_right.normalize(), view_up.normalize(), view_inner}.as4());
    }

    // 将观察坐标转换为裁剪坐标
    Mat44 getProjectionMat() const {
        return MatUtils::perspective(fov, aspect_ratio, view_near, view_far);
        /*
        Todo 实现正交投影
        return MatUtils::ortho(
            -view_width / 2_n, view_width / 2_n,
            -view_height / 2_n, view_height / 2_n,
            view_near, view_far);
         */
    }

    // 将裁剪坐标转换为屏幕坐标
    Mat44 getScreenMat() const {
        return MatUtils::merge(
            MatUtils::scaleXYZ(make_vec(view_width, view_height, view_far - view_near)).as4(),
            MatUtils::translate(make_vec(view_width / 2, view_height / 2, 0)));
    }

public:
    // For RtRender

    // 从视口上某个坐标投射出一条射线,(x,y)在[0,view_width)x[0,view_height)内
    Ray makeRay(number x, number y) const {
        x /= (number) view_width, y /= (number) view_height;
        return Ray{eye_pos, (view_left_bottom + view_right * x + view_up * y - eye_pos).normalize()};
    }
};

} // namespace mne

#endif //MINI_ENGINE_CAMERA_HPP
