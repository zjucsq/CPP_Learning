#pragma once

#include <coroutine>
#include <utility>

#include "executor.h"

template <typename T, typename E> struct Task;

template <typename R, typename E> struct TaskAwaiter {
  explicit TaskAwaiter(AbstractExecutor *executor, Task<R, E> &&task) noexcept : task_(std::move(task)), executor_(executor) {}

  TaskAwaiter(TaskAwaiter &&completion) noexcept : task_(std::exchange(completion.task, {})), executor_(completion.executor_) {}

  TaskAwaiter(TaskAwaiter &) = delete;

  TaskAwaiter &operator=(TaskAwaiter &) = delete;

  constexpr bool await_ready() const noexcept { return false; }

  void await_suspend(std::coroutine_handle<> handle) noexcept {
    // 当 task 执行完之后调用 resume
    task_.finally([handle, this]() { executor_->execute([handle]() { handle.resume(); }); });
  }

  // 协程恢复执行时，被等待的 Task 已经执行完，调用 get_result 来获取结果
  R await_resume() noexcept { return task_.get_result(); }

private:
  Task<R, E> task_;
  AbstractExecutor *executor_;
};
