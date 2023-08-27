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

#include "../io_utils.h"
#include "task_promise.h"

template <typename T, typename E> struct Task {
  using promise_type = promise_type_<T, E>;
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

  explicit Task(std::coroutine_handle<promise_type> handle) noexcept : handle_(handle) {
    debug((std::string("create coroutine this = ") + std::to_string(reinterpret_cast<long long>(this))).c_str());
  }

  Task(Task &&task) noexcept : handle_(std::exchange(task.handle_, {})) {debug((std::string("move to coroutine this = ") + std::to_string(reinterpret_cast<long long>(this))).c_str());}

  Task(Task &) = delete;

  Task &operator=(Task &) = delete;

  ~Task() {
    debug((std::string("try destroy task ") + std::to_string(reinterpret_cast<long long>(handle_.address()))).c_str());
    debug((std::string("try destroy task this = ") + std::to_string(reinterpret_cast<long long>(this))).c_str());
    if (handle_) {
      // if (!is_destroy) {
      debug((std::string("destroy task ") + std::to_string(reinterpret_cast<long long>(handle_.address()))).c_str());
      handle_.destroy();
      // is_destroy = true;
      debug((std::string("now task address ") + std::to_string(reinterpret_cast<long long>(handle_.address()))).c_str());
    }
  }

private:
  std::coroutine_handle<promise_type> handle_;
  // std::atomic_bool is_destroy{false};
};
