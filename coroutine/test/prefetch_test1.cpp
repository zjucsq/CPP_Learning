#include <iostream>

#include "../cache/prefetch.h"
#include "../include/test/task.h"

Task<int, NoopExecutor> binary_search_prefetch(std::vector<int> &nums, int val) {
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

Task<int, ThreadPoolExecutor> MultiLookUp(std::vector<int> &nums, std::vector<int> &targets, int fraction, int remain) {
    int not_found_cnt = 0;
    for (int i = remain; i < targets.size(); i += fraction) {
        auto t = targets[i];
        auto res = co_await binary_search_prefetch(nums, t);
        if (res == -1) {
            not_found_cnt++;
        }
    }
    co_return not_found_cnt;
}

int do_part(std::vector<int> &nums, std::vector<int> &targets, int fraction, int remain) {
    auto simpleTask = MultiLookUp(nums, targets, fraction, remain);
    int not_found_cnt;
    try {
        not_found_cnt = simpleTask.get_result();
    } catch (std::exception &e) {
        debug("error: ", e.what());
    }
    return not_found_cnt;
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
    
    int not_found_cnt;
    double mean_time = 0;
    int fraction = 2;
    ThreadPoolSimple threadpool{std::thread::hardware_concurrency(), true, true};
    for (int t = 0; t < 5; ++t) {
        // auto simpleTask = MultiLookUp(nums, targets);
        clock_t start, end;
        start = clock();
        std::atomic<int> not_found_cnt = 0;
        std::atomic<int> count_down = targets.size();
        std::cout << targets.size() << std::endl;
        for (int remain = 0; remain < fraction; ++remain) {
            threadpool.submit([&not_found_cnt, &count_down, &nums, &targets, fraction, remain]() {
                auto ret = do_part(nums, targets, fraction, remain);
                not_found_cnt.fetch_add(ret, std::memory_order_relaxed);
                count_down.fetch_add(-targets.size() / fraction, std::memory_order_relaxed);
            });
        }
        while (count_down.load(std::memory_order_relaxed));
        end = clock();
        std::cout << t << " time = " << double(end - start) / CLOCKS_PER_SEC << "s" << std::endl;
        mean_time += double(end - start) / CLOCKS_PER_SEC;
        std::cout << not_found_cnt << std::endl;
    }
    std::cout << "mean time = " << mean_time / 5 << "s" << std::endl;
}