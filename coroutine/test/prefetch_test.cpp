#include <iostream>

#include "../cache/prefetch.h"
#include "../include/test/task.h"

Task<int, ThreadPoolExecutor> binary_search_prefetch(std::vector<int> &nums, int val) {
    auto first = nums.data();
    auto last = first + nums.size();
    auto len = nums.size();

    while (len > 0) {
        auto half = len / 2;
        auto middle = first + half;
        auto middle_key = co_await *middle;
        if (*middle < val) {
            first = middle + 1;
            len = len - half - 1;
        } else {
            len = half;
        }
        if (*middle == val) {
            co_return middle - nums.data();
        }
    }
    co_return -1;
}

Task<int, ThreadPoolExecutor> MultiLookUp(std::vector<int> &nums, std::vector<int> &targets) {
    int not_found_cnt = 0;
    for (auto t : targets) {
        auto res = co_await binary_search_prefetch(nums, t);
        if (res == -1) {
            not_found_cnt++;
        }
    }
    co_return not_found_cnt;
}


int main() {
    std::vector<int> nums, pad, targets;
    build(nums);
    build(pad);
    build(targets);
    for (auto &t : targets) {
        t -= SIZE / 2;
    }
    std::mt19937 g(42);
    std::shuffle(targets.begin(), targets.end(), g);
    clean_cache(pad);
    auto simpleTask = MultiLookUp(nums, targets);
    simpleTask.then([](int i) { debug("simple task end: ", i); }).catching([](std::exception &e) { debug("error occurred", e.what()); });
    clock_t start, end;
    start = clock();
    try {
        auto not_found_cnt = simpleTask.get_result();
        debug("simple task end from get: ", not_found_cnt);
    } catch (std::exception &e) {
        debug("error: ", e.what());
    }
    end = clock();
    std::cout << "time = " << double(end - start) / CLOCKS_PER_SEC << "s" << std::endl;
}