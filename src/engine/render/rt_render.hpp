//
// Created by IMEI on 2022/8/16.
//

#ifndef MINI_ENGINE_RT_RENDER_HPP
#define MINI_ENGINE_RT_RENDER_HPP

#include "../interface/render.hpp"
#include <iostream>

namespace mne {
// 基于光线追踪的渲染器
class RtRender: public IRender {
    std::atomic_int process = 0;

    int total = 0;

    void initProcess() {
        process = 0;
        total   = camera->vh * camera->vw;
    }

    void updateProcess() {
        int cur = process;
        if (cur % 10000 == 0) {
            printf("process : %.4f%% , (%d/%d)\n", (double) cur / total * 100, cur, total);
        }
        ++process;
    }

public:
    void render() final {
        // 初始化进度
        initProcess();
        updateProcess();
        // 初始化输出缓冲区
        int vh = camera->vh,
            vw = camera->vw; // 视口大小
        image->init(vh, vw);

        // 枚举每个像素
#pragma omp parallel for
        for (int i = 0; i < vh; i++) {
#pragma omp parallel for
            for (int j = 0; j < vw; j++) {
                image->setPixel(i, j, samplePixel(number(i), number(j)));
                updateProcess();
            }
        }
    }

private:
    Color samplePixel(number i, number j) {
        Color     sum{};
        int       cnt{};
        HitResult hit;
        // 在[i,i+1)x[j,j+1)内随机采样
        for (int k = 0; k < spp; ++k) {
            number ri = i + randFloat(), rj = j + randFloat();
            // 光线和限制长度
            auto [ray, limit] = camera->makeRay(ri, rj);
            // 检查和场景的碰撞
            if (scene->rayCast(ray, hit)) { // && hit.tick < limit
                Color tmp = scene->shade(-ray.dir, hit);
                ++cnt, sum = sum + tmp;
                //printf("hit at (%f,%f) (%f,%f,%f) , idx = %d\n", ri, rj, tmp.r, tmp.g, tmp.b, idx++);
            }
        }
        return cnt ? sum / number(cnt) : Color{};
    }
};
} // namespace mne

#endif //MINI_ENGINE_RT_RENDER_HPP
