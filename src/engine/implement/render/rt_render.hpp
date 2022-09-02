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
        for (int y = 0; y < vh; y++) {
#pragma omp parallel for
            for (int x = 0; x < vw; x++) {
                image->setPixel(y, x, samplePixel(number(x), number(y)));
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
            auto   ray = camera2->makeRay(sx / (number) vw, sy / (number) vh);
            // 检查和场景的碰撞
            if (scene->rayCast(ray, hit)) {
                sum += scene->shade(-ray.dir, hit);
            }
        }
        return sum / number(spp);
    }

private:
    // 在[0,1)x[0,1)中随机采样一个点
    static std::pair<number, number> sampleArea() {
        return {randFloat(), randFloat()};
    }
};
} // namespace mne

#endif //MINI_ENGINE_RT_RENDER_HPP
