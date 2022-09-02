﻿//
// Created by IMEI on 2022/9/2.
//

#ifndef MINI_ENGINE_PROCESS_HPP
#define MINI_ENGINE_PROCESS_HPP

#include <ctime>
#include <atomic>
#include <mutex>

namespace mne {

// 进度打印器
template<bool ThreadSafe = true>
class Process {
    std::conditional_t<ThreadSafe, std::atomic_int, int> process;

    std::mutex lock;

    int total{}, freq{}; // 总进度,打印频率

    clock_t start{}; // 开始时间

public:
    void init(int target, int inner) {
        process = 0;
        total   = target;
        freq    = inner;
        start   = clock();
        printf("process : %.4f%% , leave : ???\n", 0.0);
    }

    void update() {
        int cur = ++process;
        if (cur % freq == 0) {
            std::unique_lock locker(lock);
            printf("process : %.4f%% , ", (double) cur / total * 100);
            // speed = process / (clock() - start) , 1进度/ms
            int  leave  = int(((double) (total - cur) * (clock() - start)) / (cur * 1000.0)); // 还剩多少秒
            auto second = leave % 60;
            auto minute = (leave / 60) % 60;
            auto hour   = leave / 3600;
            printf("leave : %02dh:%02dm:%02ds \n", hour, minute, second);
        }
    }
};

} // namespace mne

#endif //MINI_ENGINE_PROCESS_HPP
