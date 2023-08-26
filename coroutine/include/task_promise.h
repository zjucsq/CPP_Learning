#pragma once

#include <coroutine>
#include <list>
#include <mutex>
#include <utility>

#include "result.h"
#include "task_awaiter.h"

template <typename T> struct Task;

template <typename T> struct promise_type {
  std::suspend_never initial_suspend() { return {}; }
  std::suspend_always final_suspend() noexcept { return {}; }
  Task<T> get_return_object() { return Task{std::coroutine_handle<promise_type>::from_promise(*this)}; }
  template <typename R> TaskAwaiter<R> await_transform(Task<R> &&task) { return TaskAwaiter<R>(std::move(task)); }

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
    completion_.notify_all();
  }
  void on_completed(std::function<void(Result<T>)> &&func) {
    std::unique_lock lock(completion_lock_);
    if (result_.has_value()) {
      completion_.wait(lock);
      auto value = result_.value();
      lock.unlock();
      func(value);
    } else {
      completion_callbacks_.push_back(std::move(func));
    }
  }

private:
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
