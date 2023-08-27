#pragma once

#include <coroutine>
#include <list>
#include <mutex>
#include <utility>

#include "dispatch_awaiter.h"
#include "result.h"
#include "sleep_awaiter.h"
#include "task_awaiter.h"

template <typename T, typename E> struct Task;

template <typename T, typename E> struct promise_type_ {
  DispatchAwaiter initial_suspend() { return DispatchAwaiter{&executor}; }
  std::suspend_always final_suspend() noexcept { return {}; }
  Task<T, E> get_return_object() { return Task{std::coroutine_handle<promise_type_>::from_promise(*this)}; }
  template <typename T_, typename E_> TaskAwaiter<T_, E_> await_transform(Task<T_, E_> &&task) { return TaskAwaiter<T_, E_>(&executor, std::move(task)); }
  template <typename Rep_, typename Period_> SleepAwaiter await_transform(std::chrono::duration<Rep_, Period_> &&duration) {
    return SleepAwaiter(&executor, std::chrono::duration_cast<std::chrono::milliseconds>(duration).count());
  }

  void unhandled_exception() noexcept {
    std::lock_guard lock(completion_lock_);
    result_ = Result<T>(std::current_exception());
    completion_.notify_all();
    notify_callbacks();
  }
  void return_value(T ret) noexcept {
    std::lock_guard lock(completion_lock_);
    result_ = Result<T>(std::move(ret));
    completion_.notify_all();
    notify_callbacks();
  }
  T get_result() {
    std::unique_lock lock(completion_lock_);
    if (!result_.has_value()) {
      completion_.wait(lock);
    }
    return result_->get_or_throw();
  }
  void on_completed(std::function<void(Result<T>)> &&func) {
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
  E executor;
  std::optional<Result<T>> result_;
  std::mutex completion_lock_;
  std::condition_variable completion_;
  std::list<std::function<void(Result<T>)>> completion_callbacks_;

  void notify_callbacks() {
    auto value = result_.value();
    for (auto &callback : completion_callbacks_) {
      callback(value);
    }
    completion_callbacks_.clear();
  }
};
