#pragma once

#include <coroutine>
#include <utility>
#include <mmintrin.h>
#include <xmmintrin.h>

#include "executor.h"

template <typename T> struct PrefectchAwaiter {
  T& value;

  explicit PrefectchAwaiter(T &value) noexcept : value(value) {}

  PrefectchAwaiter(PrefectchAwaiter &) = delete;

  PrefectchAwaiter &operator=(PrefectchAwaiter &) = delete;

  constexpr bool await_ready() const noexcept { return false; }

  auto await_suspend(std::coroutine_handle<> handle) noexcept {
    _mm_prefetch(static_cast<char const*>(std::addressof(value)), _MM_HINT_NTA);
    

  }

  T& await_resume() noexcept { return value; }
};
