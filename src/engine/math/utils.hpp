//
// Created by MnZn on 2022/7/16.
//

#ifndef MINI_ENGINE_UTILS_HPP
#define MINI_ENGINE_UTILS_HPP

#include <random>
#include <ctime>

namespace mne {

// [l,r)
inline int randInt(int l, int r) {
    static auto seed  = 0;                  // time(nullptr);
    static auto maker = std::mt19937(seed); // NOLINT(cert-msc51-cpp)
    return int(maker() % (r - l)) + l;
}

// [0,n)
inline int randInt(int n) {
    return randInt(0, n);
}

// [0,1)
inline number randFloat() {
    return (number) randInt(RAND_MAX) / number(RAND_MAX);
}

// [l,r)
inline number randFloat(number l, number r) {
    return l + (r - l) * randFloat();
}

template<class T>
inline T clamp(const T& l, const T& r, const T& v) {
    return std::max(l, std::min(r, v));
}

} // namespace mne

#endif //MINI_ENGINE_UTILS_HPP
