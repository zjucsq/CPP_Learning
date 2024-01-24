#include "prefetch.h"
#include "../../executor/ThreadPool.h"
#include <atomic>

int MultiLookUp(std::vector<int> &nums, std::vector<int> &targets) {
    executor::ThreadPoolSimple threadpool{std::thread::hardware_concurrency(), true, true};
    std::atomic<int> not_found_cnt = 0;
    std::atomic<int> count_down = targets.size();
    for (auto t : targets) {
        threadpool.submit([&not_found_cnt, &count_down, &nums, t]() {
            auto ret = binary_search(nums, t);
            if (ret == -1) {
                not_found_cnt.fetch_add(1, std::memory_order_relaxed);
            }
            count_down.fetch_add(-1, std::memory_order_relaxed);
        });
    }
    while (count_down.load(std::memory_order_relaxed));
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
    double mean_time = 0;
    for (int t = 0; t < 5; ++t) {
        clean_cache(pad);
        clock_t start, end;
        start = clock();
        int not_found_cnt = MultiLookUp(nums, targets);
        end = clock();
        std::cout << t << " time = " << double(end - start) / CLOCKS_PER_SEC << "s" << std::endl;
        mean_time += double(end - start) / CLOCKS_PER_SEC;
        std::cout << not_found_cnt << std::endl;
    }
    std::cout << "mean time = " << mean_time / 5 << "s" << std::endl;
}