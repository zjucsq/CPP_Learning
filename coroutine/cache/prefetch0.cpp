#include "prefetch.h"

int main() {
    std::vector<int> nums, pad, targets;
    build(nums);
    build(pad);
    build(targets);
    std::mt19937 g(42);
    std::shuffle(targets.begin(), targets.end(), g);
    int not_found_cnt = 0;
    for (int t = 0; t < 10; ++t) {
        clean_cache(pad);
        clock_t start, end;
        start = clock();
        for (int i = 0; i < 1000000; ++i) {
            if (binary_search(nums, targets[i]) == -1) {
                ++not_found_cnt;
            }
        }
        end = clock();
        std::cout << t << " time = " << double(end - start) / CLOCKS_PER_SEC << "s" << std::endl;
    }
    std::cout << not_found_cnt << std::endl;
}