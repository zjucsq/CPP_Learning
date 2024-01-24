#pragma once

#include <mmintrin.h>
#include <xmmintrin.h>
#include <vector>
#include <iostream>
#include <ctime>
#include <random>
#include <algorithm>

#define SIZE 1024*512

void build(std::vector<int> &nums) {
    for (int i = 0; i < SIZE; ++i) {
        nums.push_back(i);
    }
}

int binary_search(const std::vector<int> &nums, int val) {
    auto first = nums.data();
    auto last = first + nums.size();
    auto len = nums.size();

    while (len > 0) {
        auto half = len / 2;
        auto middle = first + half;
        // _mm_prefetch((char const*)middle, _MM_HINT_NTA);
        if (*middle < val) {
            first = middle + 1;
            len = len - half - 1;
        } else {
            len = half;
        }
        if (*middle == val) {
            return middle - nums.data();
        }
    }
    return -1;
}

int clean_cache(const std::vector<int> &nums) {
    int res = 0;
    for (auto n : nums) {
        res += n;
    }
    return res;
}

int binary_search_prefetch1(const std::vector<int> &nums, int val) {
    auto first = nums.data();
    auto last = first + nums.size();
    auto len = nums.size();

    while (len > 0) {
        auto half = len / 2;
        auto middle = first + half;
        _mm_prefetch((char const*)middle, _MM_HINT_NTA);
        if (*middle < val) {
            first = middle + 1;
            len = len - half - 1;
        } else {
            len = half;
        }
        if (*middle == val) {
            return middle - nums.data();
        }
    }
    return -1;
}

int binary_search_prefetch2(const std::vector<int> &nums, int val) {
    auto first = nums.data();
    auto last = first + nums.size();
    auto len = nums.size();

    while (len > 0) {
        auto half = len / 2;
        auto middle = first + half;
        _mm_prefetch((char const*)(middle + 1 + (len - half - 1) / 2), _MM_HINT_NTA);
        _mm_prefetch((char const*)(first + half / 2), _MM_HINT_NTA);
        if (*middle < val) {
            first = middle + 1;
            len = len - half - 1;
        } else {
            len = half;
        }
        if (*middle == val) {
            return middle - nums.data();
        }
    }
    return -1;
}

