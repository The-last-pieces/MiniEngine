//
// Created by MnZn on 2022/7/19.
//

#ifndef MINI_ENGINE_CAMERA_HPP
#define MINI_ENGINE_CAMERA_HPP

#include "../math/vec.hpp"
#include "../math/mat.hpp"
#include "../data/ray.hpp"

namespace mne {

/*
右手坐标系约定 : 
+x为屏幕从左到右的方向
+y为屏幕从下到上的方向
+z为屏幕从里到外的方向    
*/

// Todo 将光栅化渲染器中使用的非标准相机替换为光追渲染器使用的标准相机

// 非标准摄像机
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

    // 视口坐标转世界坐标,dis为到near面的垂直距离
    Vec3 loc2world(number i, number j, number dis) const {
        number oh = i - number(vh) / number(2);
        number ow = j - number(vw) / number(2);
        Vec3   n = eye_dir, t = eye_norm, b = n.cross(t);
        // p*n = dn , p*t = oh , p*b = ow
        // (n ; t ; b) * p = (dn ; oh ; ow)
        Mat33 mat = {n, t, b};
        Vec3  p   = mat.invert() * make_vec(dn, oh, ow);
        return eye_pos + (p - eye_pos) * ((dis + dn) / dn);
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

    // 从视口上某个坐标投射出一条射线和其最大射程,(i,j)在[0,vh]x[0,vw]内
    std::pair<Ray, number> makeRay(number i, number j) const {
        Vec3   pn = loc2world(i, j, 0), pf = loc2world(i, j, df - dn);
        Vec3   dir  = (pn - eye_pos).normalize();
        number tick = (pf - pn).x() / dir.x();
        return {Ray{pn, dir}, tick};
    }
};

// 标准摄像机
class RtCamera {
    Vec3 eye_pos;          // 摄像机位置
    Vec3 view_right;       // 视图右方向(+x)
    Vec3 view_up;          // 视图上方向(+y)
    Vec3 view_left_bottom; // 视图的左下角(0,0)
public:
    RtCamera(
        const Vec3& eye_pos,     // 摄像机的位置
        const Vec3& target_pos,  // 观测点的位置,成像后位于图片中心
        const Vec3& up_dir,      // 摄像机的上方 Todo up_dir和view_inner可能不垂直的问题
        number      fov,         // 仰角的两倍,[0,pi/2]
        number      aspect_ratio // 宽高比
        ):
        eye_pos(eye_pos) {
        // 离eye_pos一个单位距离的视图宽高
        number view_height = 2 * std::tan(fov / 2);
        number view_width  = aspect_ratio * view_height;
        // 求出三个标架
        Vec3 view_inner = (target_pos - eye_pos).normalize(); // -z
        view_up         = up_dir.normalize();                 // +y
        view_right      = view_inner.cross(view_up);          // xy=z , yz=+x=-zy
        // 求出宽高矢量
        view_up *= view_height;
        view_right *= view_width;
        // 求出左下角
        view_left_bottom = eye_pos + view_inner - view_right / 2_n - view_up / 2_n;
    }

    // 从视口上某个坐标投射出一条射线,(x,y)在[0,1]x[0,1]内
    Ray makeRay(number x, number y) const {
        return Ray{eye_pos, (view_left_bottom + view_right * x + view_up * y - eye_pos).normalize()};
    }
};

} // namespace mne

#endif //MINI_ENGINE_CAMERA_HPP
