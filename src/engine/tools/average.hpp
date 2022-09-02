//
// Created by IMEI on 2022/9/2.
//

#ifndef MINI_ENGINE_AVERAGE_HPP
#define MINI_ENGINE_AVERAGE_HPP

#include <queue>

namespace mne {

// 平滑统计量
template<size_t N>
requires(N > 0) class Average {
private:
    std::queue<double> q;

    double sum{};

public:
    operator double() const { return sum / q.size(); }

    Average& operator+=(double val) { return expand(val), *this; }

private:
    void expand(double cur) {
        sum += cur, q.push(cur);
        if (q.size() > N) sum -= q.front(), q.pop();
    }
};

} // namespace mne

#endif //MINI_ENGINE_AVERAGE_HPP
