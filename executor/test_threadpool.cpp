#include "ThreadPool.h"
#include "utils.h"
#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <iostream>
#include <list>
#include <mutex>
#include <vector>

template <typename T> std::list<T> seq_quick_sort(std::list<T> input) {
  if (input.empty())
    return input;
  std::list<T> result;
  result.splice(result.begin(), input, input.begin());
  const T &pivot = *result.begin();
  auto divide_point =
      std::partition(input.begin(), input.end(), [&pivot](const T &val) { return val < pivot; });
  std::list<T> lower;
  lower.splice(lower.begin(), input, input.begin(), divide_point);
  std::list<T> lower_sorted = seq_quick_sort(std::move(lower));
  std::list<T> higher_sorted = seq_quick_sort(std::move(input));
  result.splice(result.end(), higher_sorted);
  result.splice(result.begin(), lower_sorted);
  return result;
}

template <typename T> class sorter {
public:
  std::list<T> do_sort(std::list<T> input) {
    if (input.empty())
      return input;
    std::list<T> ret;
    std::mutex mtx;
    std::condition_variable cv;
    bool done{false};
    threadpool.submit([&mtx, &cv, &done, &ret, this, input{std::move(input)}]() {
      std::unique_lock lk(mtx);
      ret = par_quick_sort(std::move(input));
      done = true;
      lk.unlock();
      cv.notify_one();
    });
    std::unique_lock lk(mtx);
    cv.wait(lk, [&done]() { return done; });
    return ret;
  }

private:
  std::list<T> par_quick_sort(std::list<T> input) {
    // print(input);
    if (input.empty())
      return input;
    std::list<T> result;
    result.splice(result.begin(), input, input.begin());
    const T &pivot = *result.begin();
    auto divide_point =
        std::partition(input.begin(), input.end(), [&pivot](const T &val) { return val < pivot; });
    std::list<T> lower;
    lower.splice(lower.begin(), input, input.begin(), divide_point);

    std::list<T> lower_sorted;
    std::atomic<bool> done{false};
    threadpool.submit([&done, &lower_sorted, this, lower{std::move(lower)}]() {
      lower_sorted = par_quick_sort(std::move(lower));
      done.store(true, std::memory_order_release);
    });

    std::list<T> higher_sorted{par_quick_sort(std::move(input))};

    while (!done.load(std::memory_order_acquire)) {
      threadpool.run_pending_task();
    }

    result.splice(result.end(), higher_sorted);
    result.splice(result.begin(), lower_sorted);
    return result;
  }

  executor::ThreadPoolSimple threadpool{2, true, true};
};

int main() {
  std::list<int> l{4, 5, 6, 3, 2, 5, 7, 8, 4, 2, 45, 7, 98, 9, 6, 5, 4};
  // std::list<int> l{4, 5, 6, 3};

  std::vector<int> v{l.begin(), l.end()};
  std::sort(v.begin(), v.end());
  for (auto vv : v) {
    std::cout << vv << ' ';
  }
  std::cout << std::endl;

  auto l1 = l;
  auto newl = seq_quick_sort(l);
  for (auto v : newl) {
    std::cout << v << ' ';
  }
  std::cout << std::endl;

  auto l2 = l;
  auto newl2 = sorter<int>().do_sort(l2);
  for (auto v : newl2) {
    std::cout << v << ' ';
  }
  std::cout << std::endl;
  return 0;
}