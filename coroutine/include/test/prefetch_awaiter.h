#pragma once

#include <coroutine>
#include <utility>
#include <mmintrin.h>
#include <xmmintrin.h>

#include "executor.h"

template <typename T> struct PrefectchAwaiter {
  explicit PrefectchAwaiter(AbstractExecutor *executor, T &value) noexcept : executor_(executor), value_(value) {}

  PrefectchAwaiter(PrefectchAwaiter &) = delete;

  PrefectchAwaiter &operator=(PrefectchAwaiter &) = delete;

  constexpr bool await_ready() const noexcept { return false; }

  void await_suspend(std::coroutine_handle<> handle) noexcept {
    _mm_prefetch((char const*)(std::addressof(value_)), _MM_HINT_NTA);
    executor_->execute([handle]() { handle.resume(); });
  }

  T& await_resume() noexcept { return value_; }

private:
  AbstractExecutor *executor_;
  T& value_;
};
