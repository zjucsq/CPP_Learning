#include "../include/5dispatcher/executor.h"
#include "../include/5dispatcher/task.h"
#include "../include/io_utils.h"

// 使用了 Async 调度器
// 这意味着每个恢复的位置都会通过 std::async 上执行
Task<int, AsyncExecutor> simple_task2() {
  debug("task 2 start ...");
  using namespace std::chrono_literals;
  std::this_thread::sleep_for(1s);
  debug("task 2 returns after 1s.");
  co_return 2;
}

// 使用了 NewThread 调度器
// 这意味着每个恢复的位置都会新建一个线程来执行
Task<int, NewThreadExecutor> simple_task3() {
  debug("in task 3 start ...");
  using namespace std::chrono_literals;
  std::this_thread::sleep_for(2s);
  debug("task 3 returns after 2s.");
  co_return 3;
}

// 使用了 Looper 调度器
// 这意味着每个恢复的位置都会在同一个线程上执行
Task<int, LooperExecutor> simple_task() {
  debug("task start ...");
  auto result2 = co_await simple_task2();
  debug("returns from task2: ", result2);
  auto result3 = co_await simple_task3();
  debug("returns from task3: ", result3);
  co_return 1 + result2 + result3;
}

int main() {
  auto simpleTask = simple_task();
  simpleTask.then([](int i) { debug("simple task end: ", i); }).catching([](std::exception &e) { debug("error occurred", e.what()); });
  try {
    auto i = simpleTask.get_result();
    debug("simple task end from get: ", i);
  } catch (std::exception &e) {
    debug("error: ", e.what());
  }
  return 0;
}
