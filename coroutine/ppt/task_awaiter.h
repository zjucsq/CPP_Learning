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
    // 当 task 执行完之后调用 handle 句柄表示的协程
    // ...
  }

  R await_resume() noexcept { 
    // 协程恢复执行时，被等待的 Task 已经执行完，把结果作为返回值返回
    // ...
  }

private:
  Task<R> task;
};

