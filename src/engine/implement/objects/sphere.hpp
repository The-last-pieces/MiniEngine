//
// Created by IMEI on 2022/8/18.
//

#ifndef MINI_ENGINE_SPHERE_HPP
#define MINI_ENGINE_SPHERE_HPP

#include <utility>

#include "interface/object.hpp"

namespace mne {

// 椭球
class Sphere final: public IObject {
    // 球心位置
    Vec3 center = make_vec(0, 0, 0);
    // xyz方向的长度
    Vec3 length = make_vec(1, 1, 1);
    // 方向向量
    Vec3 x = VecUtils::X, y = VecUtils::Y, z = VecUtils::Z;

public:
    Sphere() = default;

protected:
    void onSetTransform() final {
        // 更新三个轴
        std::tie(x, y, z) = std::make_tuple(
            dToWorld(VecUtils::X / 2_n),
            dToWorld(VecUtils::Y / 2_n),
            dToWorld(VecUtils::Z / 2_n));
        // 更新球心坐标
        center = pToWorld(make_vec(0, 0, 0));
        // 更新轴长
        length = make_vec(x.length(), y.length(), z.length());
        // 轴线归一化
        x /= length.x(), y /= length.y(), z /= length.z();
    }

public:
    // Todo 椭球采样
    // 随机在物体表面上采样一个点
    void sampleLight(LightResult& result) const final {
        result.normal = VecUtils::sampleSphere();
        result.point  = center + length.v_max() * result.normal;
        result.uv     = mapping_uv(result.normal);
    }

    // Todo 椭球面积
    number area() const final {
        return 0;
        //return 4 * pi * radius * radius;
    }

protected:
    // 椭球与光线的交点
    void intersection(const Ray& ray, HitResult& hit) const final {
        // 点到椭圆dx,dy,dz三个轴的距离记作x,y,z , 三轴长度为a,b,c
        // 记n123分别为dx,dy,dz , l123分别为abc , d123分别为x,y,z
        // \sum (d_i/l_i)^2 = 1
        // d_i = n_i * (oc + d * t) = n_i * oc + (n_i * d) * t = A_i + B_i * t
        // oc = o - c
        // (d_i/l_i) ^ 2 = (A_i ^ 2 + 2 * A_i * B_i * t + B_i ^ 2 * t ^ 2)/(l_i ^ 2) = E_i + F_i * t + G_i * t ^ 2
        // E/F/G = \sum E/F/G_i
        // E + F * t + G * t ^ 2 = 1
        // D = sqrt(F^2 - 4 * E * G)
        // t = (-F ± D) / (2 * G)
        Vec3 c = center, o = ray.pos, d = ray.dir, oc = o - c;
        // 填充轴信息
        Vec3   n[3]{x, y, z}, l = length; // 三个轴的方向向量及其长度
        number E{}, F{}, G{};
        for (int i = 0; i < 3; ++i) {
            number A = n[i] * oc;
            number B = n[i] * d;
            number L = l[i] * l[i];
            E += A * A / L;
            F += 2 * A * B / L;
            G += B * B / L;
        }
        number D2 = F * F - 4 * (E - 1_n) * G;
        if (D2 < 0) return; // 无解
        number D  = std::sqrt(D2);
        number t1 = (-F - D) / (2 * G), t2 = (-F + D) / (2 * G);

        if (!hit.setTick(t1, t2)) return;

        // 法线方向与梯度方向一致(x/a,y/b,z/c)
        Vec3 normal = ((hit.point = hit.getPoint(ray)) - center).div(l);
        hit.setNormal(normal, ray);
        hit.uv = mapping_uv(normal);
    }

private:
    // 方向向量映射到纹理坐标
    Vec2 mapping_uv(const Vec3& normal) const {
        auto [theta, phi] =
            VecUtils::dir2angle(VecUtils::toLocal(normal, x, y, z)).data;
        return {
            MathUtils::mod(theta + pi, pi2) / pi2,
            MathUtils::mod(phi + pi_half, pi) / pi};
    }
};

} // namespace mne

#endif //MINI_ENGINE_SPHERE_HPP
