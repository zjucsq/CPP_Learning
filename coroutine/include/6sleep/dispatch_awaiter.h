#pragma once

#include <coroutine>

#include "executor.h"

struct DispatchAwaiter {
  explicit DispatchAwaiter(AbstractExecutor *executor) noexcept : executor_(executor) {}

  bool await_ready() const { return false; }

  void await_suspend(std::coroutine_handle<> handle) const {
    executor_->execute([handle]() { handle.resume(); });
  }

  void await_resume() {}

private:
  AbstractExecutor *executor_;
};