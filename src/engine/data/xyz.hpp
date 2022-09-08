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
