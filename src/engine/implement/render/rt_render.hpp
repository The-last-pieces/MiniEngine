//
// Created by IMEI on 2022/8/16.
//

#ifndef MINI_ENGINE_RT_RENDER_HPP
#define MINI_ENGINE_RT_RENDER_HPP

#include "../../interface/render.hpp"
#include <iostream>
#include <mutex>

namespace mne {
// 基于光线追踪的渲染器
class RtRender: public IRender {
    std::mutex      lock;
    std::atomic_int process;

    int     total, vh, vw;
    clock_t start;

    void initProcess() {
        process = 0;
        total   = vh * vw;
        start   = clock();
    }

    void updateProcess() {
        int cur = ++process;
        if (cur % 10000 == 0) {
            std::unique_lock locker(lock);
            printf("process : %.4f%% , ", (double) cur / total * 100);
            // speed = process / (clock() - start) , 1像素/ms
            int  leave  = int(((double) (total - cur) * (clock() - start)) / (cur * 1000.0)); // 还剩多少秒
            auto second = leave % 60;
            auto minute = (leave / 60) % 60;
            auto hour   = leave / 3600;
            printf("leave : %02dh:%02dm:%02ds \n", hour, minute, second);
        }
    }

public:
    void render() final {
        // 初始化输出缓冲区
        vh = camera->vh, vw = camera->vw; // 视口大小
        image->resize(vw, vh);
        // 初始化进度
        initProcess();
        updateProcess();

        // 枚举每个像素
#pragma omp parallel for
        for (int x = 0; x < vw; x++) {
#pragma omp parallel for
            for (int y = 0; y < vh; y++) {
                image->setPixel(x, y, samplePixel(number(x), number(y)));
                updateProcess();
            }
        }
    }

    std::shared_ptr<RtCamera> camera2;

private:
    Color samplePixel(number x, number y) {
        Color     sum{};
        HitResult hit;
        for (int k = 0; k < spp; ++k) {
            // 在[x,x+1)x[y,y+1)内随机采样
            auto [ox, oy] = sampleArea();
            number sx = x + ox, sy = y + oy;
            auto   ray = camera2->makeRay(sx, sy);
            // 检查和场景的碰撞
            if (intersect(ray, hit)) {
                sum += trace(ray.dir, hit);
            } else {
                sum += background;
            }
        }
        return sum / number(spp);
    }

private:
    // 在[0,1)x[0,1)中随机采样一个点
    static std::pair<number, number> sampleArea() {
        return {randFloat(), randFloat()};
    }

private:
    // 背景色/环境光
    Color background = Color::fromRGB256(255, 255, 255) * 0.3_n;

    // Todo 转循环
    // 以in_dir方向的射线打到hit上的全局光照信息
    Color trace(const Vec3& in_dir, const HitResult& hit, int depth = 0) const {
        /// 配置 -----------------------------
        constexpr int max_dep = 10; // 深度限制

        /// 简写 -----------------------------
        auto& obj = *(hit.obj);   // 观测点所在的图元
        auto& mat = obj.matRef(); // 观测点的材质

        /// 递归终止条件 ----------------------
        if (obj.isLight()) return mat.emit(hit.uv).clamp(1_n); // 直接观测到光源
        if (depth > max_dep) return Color{0, 0, 0};            // 超过最大深度

        /// BxDF信息 -------------------------
        BxDFResult bxdf;
        mat.sample(in_dir, hit, bxdf);

        /// 间接光照 --------------------------
        Color     L_indirect{}, le{};
        HitResult hit2;
        if (intersect(Ray{hit.point, bxdf.out_dir}, hit2) && (bxdf.specular || !hit2.obj->isLight())) {
            le = trace(bxdf.out_dir, hit2, depth + 1);
        } else if (!hit2.success) {
            le = background;
        }
        {
            Color  f_r = bxdf.albedo;               // 反射率
            number pdf = bxdf.pdf;                  // 采样的pdf
            number dot = hit.normal * bxdf.out_dir; // 与观测点夹角

            L_indirect = (le * f_r) * (dot / (pdf)); // * p_rr
        }
        //  镜面材质只需要间接光照
        if (bxdf.specular) return L_indirect;

        /// 直接光照 --------------------------
        Color L_direct{};
        auto& light = selectLight(); // 随机选择一个光源

        LightResult ems; // 随机采样
        light.sampleLight(ems);

        auto l_out     = ems.point - hit.point; // 光线矢量
        auto l_out_dir = l_out.normalize();     // 光线方向

        // 检测是否被遮挡
        if (intersect(Ray{hit.point, l_out_dir}, hit2) && hit2.obj == &light) {
            Color  f_r_l = bxdf.albedo;                              // 反射率
            number pdf_l = lightPDF();                               // 光源采样的pdf, light.PDF();
            number dot   = hit.normal * l_out_dir;                   // 与观测点夹角
            number dot_l = std::max(0_n, -(ems.normal * l_out_dir)); // 与光源夹角
            Color  le_l  = light.matRef().emit(ems.uv);              // 直接光照

            L_direct = (le_l * f_r_l) * (dot * dot_l / (pdf_l * l_out.norm2()));
        }

        /// 返回结果 --------------------------
        return (L_direct + L_indirect);
    }

private:
    // 辅助函数

    // Todo 使用BVH优化
    // 射线检测
    bool intersect(const Ray& ray, HitResult& hit) const {
        HitResult temp;
        hit.reset();
        for (const auto& ptr : scene->objects) {
            if (ptr->intersect(ray, temp)) {
                hit = temp;
            }
        }
        return hit.success;
    }

    // 按面积比决定概率比然后选择光源
    const IObject& selectLight() const {
        std::vector<number> areas;
        for (auto& ptr : scene->objects) areas.push_back(ptr->isLight() ? ptr->area() : 0_n);
        return *(scene->objects[randChoose(areas)]);
    }

    // 返回光源采样的pdf
    number lightPDF() const {
        number areaSum = 0_n;
        for (auto& ptr : scene->objects) {
            if (ptr->isLight()) {
                areaSum += ptr->area();
            }
        }
        return 1_n / areaSum;
    }
};

} // namespace mne

#endif //MINI_ENGINE_RT_RENDER_HPP
