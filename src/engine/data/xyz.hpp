//
// Created by IMEI on 2022/9/5.
//

#ifndef MINI_ENGINE_XYZ_HPP
#define MINI_ENGINE_XYZ_HPP

#include "math/utils.hpp"

namespace mne {

// 一个右手坐标系
class XYZ {
private:
    // 坐标原点
    Vec3 o = {};
    // 三个单位向量
    Vec3 x = VecUtils::X;
    Vec3 y = VecUtils::Y;
    Vec3 z = VecUtils::Z;

public:
    XYZ() = default;

    // 按缩放,旋转,平移变换后的新标架
    XYZ(const Transform& transform) {
        // 缩放
        x = transform.scale.x() * x, y = transform.scale.y() * y, z = transform.scale.z() * z;
        // 旋转
        (*this) = rotate(transform.rotate);
        // 平移
        o = transform.translate + o;
    }

    // 按方位角-仰角旋转后的新标架
    XYZ rotate(const Vec2& theta_phi) const {
        auto [theta, phi] = theta_phi.data;
        XYZ   ret;
        Mat33 ry = MatUtils::rotateY(theta);
        // x和z绕+y旋转theta
        ret.x = ry * x, ret.z = ry * z;
        Mat33 rx = MatUtils::rotate(ret.x, -phi);
        // yz再绕旋转后的x旋转-phi
        ret.z = rx * ret.z, ret.y = rx * ret.y;
        return ret;
    }

    // 按欧拉角旋转后的新标架
    XYZ rotate(const Vec3& xyz) const {
        XYZ  ret;
        auto mat = MatUtils::rotateXYZ(xyz);
        // 旋转xyz
        ret.x = mat * x, ret.y = mat * y, ret.z = mat * z;
        return ret;
    }

    // 原点设置为(0,0,0) , xyz设为方向向量 , 此时toLocal和toWorld都是针对方向向量进行变换 , 否则是针对点进行操作
    XYZ origin() const {
        XYZ ret = *this;
        ret.o   = {};
        return ret;
    }

public:
    // 返回满足z轴方向要求的任意标架
    static XYZ randXYZ(const Vec3& n) {
        Vec3 z      = n.normalize();
        Vec2 map_xz = z.pick<0, 2>(), map_xy = z.pick<0, 1>();
        if (map_xz.length() < 0.5_n) {
            // z和xz平面的夹角
            number phi = z.inner(make_vec(map_xz.x(), 0, map_xz.y()));
            if (z.y() < 0) phi = -phi;
            return XYZ().rotate(make_vec(0, phi));
        } else {
            // z和xy平面的夹角
            number phi = z.inner(make_vec(map_xy.x(), map_xy.y(), 0));
            if (z.z() < 0) phi = -phi;
            return XYZ().rotate(make_vec(0, phi));
        }
    }

public:
    // 根据标架反推theta_phi
    Vec2 getThetaPhi() const {
        if (z.pick<0, 2>().length() > 0.5_n) {
            // z在xz平面的投影,必须够大才能保证theta的精度
            Vec2 map_z = z.pick<0, 2>();
            // (0,0,1)要旋转多少度才能追上map_z
            number theta = make_vec(0, 1).rotate(map_z);
            // z和map_z的夹角
            number phi = z.inner(make_vec(map_z.x(), 0, map_z.y()));
            if (z.y() < 0) phi *= -1_n;
            return {theta, phi};
        } else {
            // x在xz平面的投影,在此if分支下必然足够大
            Vec2 map_x = x.pick<0, 2>();
            // (1,0,0)要旋转多少度才能追上map_x
            number theta = make_vec(1, 0).rotate(map_x);
            // x和map_z的夹角
            number phi = x.inner(make_vec(map_x.x(), 0, map_x.y()));
            if (x.y() < 0) phi *= -1_n;
            return {theta, phi};
        }
    }

    // 根据标架反推xyz
    Vec3 getXYZ() const {
        return MatUtils::angle2xyz(getThetaPhi());
    }

    // 获取原始方向
    std::tuple<Vec3, Vec3, Vec3> getRawDir() const { return {x, y, z}; }

    // 获取三个方向向量
    std::tuple<Vec3, Vec3, Vec3> getDir() const { return {x.normalize(), y.normalize(), z.normalize()}; }

    // 获取三个方向的长度
    Vec3 getScale() const { return {x.length(), y.length(), z.length()}; }

public:
    // 标准坐标系中的坐标转换为this坐标系的坐标
    Vec3 toLocal(Vec3 v) const {
        v -= o; // OP在xyz轴上的投影
        return make_vec(v * x, v * y, v * z);
    }

    // world坐标系中的坐标转换为this坐标系的坐标
    Vec3 toLocal(const Vec3& v, const XYZ& world) const {
        return toLocal(world.toWorld(v));
    }

    // this坐标系中的坐标转换为标准坐标系中的坐标
    Vec3 toWorld(const Vec3& v) const {
        return v.x() * x + v.y() * y + v.z() * z + o;
    }

    // this坐标系中的坐标转换为world坐标系中的坐标
    Vec3 toWorld(const Vec3& v, const XYZ& world) const {
        return world.toLocal(toWorld(v));
    }
};

} // namespace mne

#endif //MINI_ENGINE_XYZ_HPP
