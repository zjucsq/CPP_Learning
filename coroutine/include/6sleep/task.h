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

#include "task_promise.h"

template <typename T, typename E> struct Task {
  using promise_type = promise_type<T, E>;
  T get_result() { return handle_.promise().get_result(); }

  Task &then(std::function<void(T)> &&func) {
    handle_.promise().on_completed([func](auto result) {
      try {
        func(result.get_or_throw());
      } catch (std::exception &e) {
      }
    });
    return *this;
  }

  Task &catching(std::function<void(std::exception &)> &&func) {
    handle_.promise().on_completed([func](auto result) {
      try {
        result.get_or_throw();
      } catch (std::exception &e) {
        func(e);
      }
    });
    return *this;
  }

  Task &finally(std::function<void()> &&func) {
    handle_.promise().on_completed([func](auto result) { func(); });
    return *this;
  }

  explicit Task(std::coroutine_handle<promise_type> handle) noexcept : handle_(handle) {}

  Task(Task &&task) noexcept : handle_(std::exchange(task.handle_, {})) {}

  Task(Task &) = delete;

  Task &operator=(Task &) = delete;

  ~Task() {
    if (handle_)
      handle_.destroy();
  }

private:
  std::coroutine_handle<promise_type> handle_;
};
