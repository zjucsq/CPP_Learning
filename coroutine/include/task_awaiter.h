#pragma once

#include <coroutine>
#include <utility>

template <typename T> struct Task;

template <typename R> struct TaskAwaiter {
  explicit TaskAwaiter(Task<R> &&task) noexcept : task(std::move(task)) {}

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
  Task<R> task;
};
