#pragma once

#include <condition_variable>
#include <coroutine>
#include <exception>
#include <functional>
#include <list>
#include <mutex>
#include <optional>
#include <utility>
#include <variant>

// Ignore error handling to facilitate understanding of Task key issues

template <typename T> struct STask {
  template <typename R> struct TaskAwaiter {
    explicit TaskAwaiter(STask<R> &&task) noexcept : task(std::move(task)) {}
    TaskAwaiter(TaskAwaiter &&completion) noexcept : task(std::exchange(completion.task, {})) {}
    TaskAwaiter(TaskAwaiter &) = delete;
    TaskAwaiter &operator=(TaskAwaiter &) = delete;

    constexpr bool await_ready() const noexcept { return false; }
    void await_suspend(std::coroutine_handle<> handle) noexcept {
      // 当 task 执行完之后调用 resume
      task.finally([handle]() { handle.resume(); });
    }
    // 协程恢复执行时，被等待的 Task 已经执行完，调用 get_result 来获取结果
    R await_resume() noexcept { return task.get_result(); }

  private:
    STask<R> task;
  };

  struct promise_type {
    std::suspend_never initial_suspend() { return {}; }
    std::suspend_always final_suspend() noexcept { return {}; }
    STask<T> get_return_object() { return STask{std::coroutine_handle<promise_type>::from_promise(*this)}; }
    template <typename R> TaskAwaiter<R> await_transform(STask<R> &&task) { return TaskAwaiter<R>(std::move(task)); }

    void unhandled_exception() noexcept {}
    void return_value(T ret) noexcept {
      std::lock_guard lock(completion_lock_);
      result_ = std::move(ret);
      completion_.notify_all();
      notify_callbacks();
    }
    T get_result() {
      std::unique_lock lock(completion_lock_);
      if (!result_.has_value()) {
        completion_.wait(lock);
      }
      return result_.value();
    }
    void on_completed(std::function<void(T)> &&func) {
      std::unique_lock lock(completion_lock_);
      if (result_.has_value()) {
        auto value = result_.value();
        lock.unlock();
        func(value);
      } else {
        completion_callbacks_.push_back(std::move(func));
      }
    }

  private:
    std::optional<T> result_;
    std::mutex completion_lock_;
    std::condition_variable completion_;
    std::list<std::function<void(T)>> completion_callbacks_;

    void notify_callbacks() {
      auto value = result_.value();
      for (auto &callback : completion_callbacks_) {
        callback(value);
      }
      completion_callbacks_.clear();
    }
  };

  T get_result() { return handle_.promise().get_result(); }

  STask &then(std::function<void(T)> &&func) {
    handle_.promise().on_completed([func](auto result) {
      try {
        func(result);
      } catch (std::exception &e) {
      }
    });
    return *this;
  }

  explicit STask(std::coroutine_handle<promise_type> handle) noexcept : handle_(handle) {}

  STask(STask &&task) noexcept : handle_(std::exchange(task.handle_, {})) {}

  STask(STask &) = delete;

  STask &operator=(STask &) = delete;

  ~STask() {
    if (handle_)
      handle_.destroy();
  }

private:
  // finally only called by await_suspend, so private.
  // result is not used, just as a placeholder
  STask &finally(std::function<void()> &&func) {
    handle_.promise().on_completed([func](auto result) { func(); });
    return *this;
  }
  std::coroutine_handle<promise_type> handle_;
};
