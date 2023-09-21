#pragma once

#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <functional>
#include <iostream>
#include <mutex>
#include <queue>
#include <sched.h>
#include <thread>
#include <type_traits>
#include <unistd.h>
#include <vector>

namespace executor {

template <typename T>
requires std::is_move_constructible_v<T>
class ThreadSafeQueue {
public:
  void push(const T &t) {
    {
      std::lock_guard lk(mutex_);
      deque_.push_back(t);
    }
    cond_.notify_one();
  };
  void push(T &&t) {
    {
      std::lock_guard lk(mutex_);
      deque_.push_back(std::move(t));
    }
    cond_.notify_one();
  };

  bool wait_pop(T &value) {
    std::unique_lock lk(mutex_);
    cond_.wait(lk, [this]() { return !deque_.empty() || stop_; });
    if (stop_)
      return false;
    value = std::move(deque_.front());
    deque_.pop_front();
    return true;
  }
  bool try_pop(T &value) {
    std::unique_lock lk(mutex_, std::try_to_lock);
    if (!lk || deque_.empty())
      return false;
    value = std::move(deque_.front());
    deque_.pop_front();
    return true;
  }
  bool try_steal(T &value) {
    std::unique_lock lk(mutex_, std::try_to_lock);
    if (!lk || deque_.empty())
      return false;
    value = std::move(deque_.back());
    deque_.pop_back();
    return true;
  }

  std::size_t size() const {
    std::lock_guard lk(mutex_);
    return deque_.size();
  }
  bool empty() const {
    std::lock_guard lk(mutex_);
    return deque_.empty();
  }
  void stop() {
    {
      std::lock_guard lk(mutex_);
      stop_ = true;
    }
    cond_.notify_all();
  }

private:
  bool stop_;
  std::deque<T> deque_;
  std::mutex mutex_;
  std::condition_variable cond_;
};

#ifdef __linux__
static void getCurrentCpus(std::vector<uint32_t> &ids) {
  cpu_set_t set;
  ids.clear();
  if (sched_getaffinity(0, sizeof(set), &set) == 0) {
    for (uint32_t i = 0; i < CPU_SETSIZE; ++i) {
      if (CPU_ISSET(i, &set)) {
        ids.emplace_back(i);
      }
    }
  }
}
#endif

class ThreadPoolSimple {
public:
  struct workItem {
    bool canSteal_{false};
    std::function<void()> fn{nullptr};
  };

  explicit ThreadPoolSimple(size_t threadNum = std::thread::hardware_concurrency(),
                            bool enableSteal = false, bool enableBindings = false)
      : threadNum_(threadNum), enableSteal_(enableSteal), enableBindings_(enableBindings),
        stop_(false), queues_(threadNum_) {
    auto worker = [this](int id) {
      my_idx_ = id;
      local_work_queue_ptr_ = &queues_[my_idx_];
      while (!stop_) {
        run_pending_task();
      }
    };

#ifdef __linux__
    std::vector<uint32_t> cpu_ids;
    if (enableBindings_) {
      getCurrentCpus(cpu_ids);
    }
#endif

    threads_.reserve(threadNum_);
    for (int i = 0; i < threadNum_; ++i) {
      threads_.emplace_back(worker, i);
#ifdef __linux__
      if (!enableBindings_) {
        continue;
      }
      cpu_set_t cpuset;
      CPU_ZERO(&cpuset);
      CPU_SET(cpu_ids[i % cpu_ids.size()], &cpuset);
      // sched_setaffinity绑定的是进程
      int rc = pthread_setaffinity_np(threads_[i].native_handle(), CPU_SETSIZE, &cpuset);
      if (rc != 0) {
        // std::cerr << "Error calling sched_setaffinity: " << rc << "\n";
        std::cerr << "Failed to call sched_setaffinity: " << std::strerror(errno) << '\n';
      }
#endif
    }
  }

  ~ThreadPoolSimple() {
    stop_ = true;
    for (auto &q : queues_) {
      q.stop();
    }
    for (auto &t : threads_) {
      t.join();
    }
  }

  bool submit(std::function<void()> fn) {
    if (!fn || stop_) {
      return false;
    }
    if (my_idx_ == -1) {
      global_work_queue_.push(workItem{enableSteal_, std::move(fn)});
    } else {
      local_work_queue_ptr_->push(workItem{enableSteal_, std::move(fn)});
    }
    return true;
  }

  bool scheduleById(std::function<void()> fn, int id);

  void run_pending_task() {
    workItem worki = {};
    if (pop_task_from_local_queue(worki) || pop_task_from_global_queue(worki) ||
        (enableSteal_ && pop_task_from_other_thread_queue(worki)))
      ;
    if (worki.fn)
      worki.fn();
    else
      std::this_thread::yield();
  }

private:
  bool pop_task_from_local_queue(workItem &w) { return local_work_queue_ptr_->try_pop(w); }
  bool pop_task_from_global_queue(workItem &w) { return global_work_queue_.try_pop(w); }
  bool pop_task_from_other_thread_queue(workItem &w) {
    for (int i = 0; i < queues_.size() - 1; ++i) {
      auto index = (my_idx_ + i + 1) % queues_.size();
      if (queues_[index].try_steal(w)) {
        return true;
      }
    }
    return false;
  }

private:
  size_t threadNum_;

  ThreadSafeQueue<workItem> global_work_queue_;
  std::vector<ThreadSafeQueue<workItem>> queues_;
  std::vector<std::thread> threads_;

  std::atomic<bool> stop_;
  bool enableSteal_;
  bool enableBindings_;

  static thread_local ThreadSafeQueue<workItem> *local_work_queue_ptr_;
  static thread_local int my_idx_;
};

thread_local ThreadSafeQueue<ThreadPoolSimple::workItem> *ThreadPoolSimple::local_work_queue_ptr_ =
    nullptr;
thread_local int ThreadPoolSimple::my_idx_ = -1;
} // namespace executor