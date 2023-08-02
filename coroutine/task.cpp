// https://en.cppreference.com/w/cpp/language/coroutines
#include "jthread.h"
#include <__coroutine/coroutine_handle.h>
#include <__coroutine/trivial_awaitables.h>
#include <coroutine>
#include <iostream>
#include <stdexcept>

auto switch_to_new_thread(mystl::jthread &out) {
  struct awaitable {
    mystl::jthread *p_out;
    bool await_ready() { return false; }
    void await_suspend(std::coroutine_handle<> h) {
      mystl::jthread &out = *p_out;
      if (out.joinable())
        throw std::runtime_error("Output jthread parameter not empty");
      out = mystl::jthread([h] { h.resume(); });
      std::cout << "New thread ID: " << out.get_id() << '\n';
    }
    int await_resume() { return 1101; }
  };
  return awaitable{&out};
}

struct task {
  struct promise_type {
    task get_return_object() { return {}; }
    std::suspend_never initial_suspend() { return {}; }
    std::suspend_never final_suspend() noexcept { return {}; }
    void return_void() {}
    void unhandled_exception() {}
  };
};

task resuming_on_new_thread(mystl::jthread &out) {
  std::cout << "Coroutine started on thread: " << std::this_thread::get_id()
            << '\n';
  std::cout << co_await switch_to_new_thread(out) << '\n';
  // awaiter destroyed here
  std::cout << "Coroutine resumed on thread: " << std::this_thread::get_id()
            << '\n';
}

int main() {
  mystl::jthread out;
  resuming_on_new_thread(out);
}