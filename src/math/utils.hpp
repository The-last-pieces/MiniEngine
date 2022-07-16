//
// Created by MnZn on 2022/7/16.
//

#ifndef MINI_ENGINE_UTILS_HPP
#define MINI_ENGINE_UTILS_HPP

#include "mat.hpp"
#include <random>
#include <ctime>

namespace mne {

// [l,r)
inline int randInt(int l, int r) {
    static auto seed  = 0;                  // time(nullptr);
    static auto maker = std::mt19937(seed); // NOLINT(cert-msc51-cpp)
    return int(maker() % (r - l)) + l;
}

} // namespace mne

#endif //MINI_ENGINE_UTILS_HPP
