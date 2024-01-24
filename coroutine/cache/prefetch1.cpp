#include "prefetch.h"

int main() {
    std::vector<int> nums, pad, targets;
    build(nums);
    build(pad);
    build(targets);
    std::mt19937 g(42);
    std::shuffle(targets.begin(), targets.end(), g);
    for (int t = 0; t < 10; ++t) {
        clean_cache(pad);
        clock_t start, end;
        start = clock();
        for (int i = 0; i < 1000000; ++i) 
            binary_search_prefetch1(nums, targets[i]);
        end = clock();
        std::cout << t << " time = " << double(end - start) / CLOCKS_PER_SEC << "s" << std::endl;
    }
}