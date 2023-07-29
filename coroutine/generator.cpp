// https://zhuanlan.zhihu.com/p/599053058
#include "generator.h"
#include <format>
#include <iostream>

Generator<int> Fib(int n) {
  if (n == 0 || n == 1) {
    co_return;
  }
  int a = 0, b = 1;
  co_yield a;
  co_yield b;

  for (int i = 2; i <= n; ++i) {
    co_yield a + b;

    b = a + b;
    a = b - a;
  }
  co_return;
}

Generator<int> Fib1(int n) {
  if (n == 0 || n == 1) {
    co_return;
  }
  int a = 0, b = 1;
  co_yield a;
  co_yield b;

  for (int i = 2; i <= n; ++i) {
    co_yield a + b;

    b = a + b;
    a = b - a;
  }
  co_return;
}

// Generator<uint32_t> Func(uint32_t n) {
//   co_yield n;
//   if (n > 0) {
//     co_yield Func(n - 1);
//   }
// }

int main() {
  auto g = Fib1(10);
  for (auto i : g)
    std::cout << i << std::endl;
  // for (auto it = g.begin(); it != g.end(); ++it)
  //   std::cout << *it << std::endl;
  // while (!g.done()) {
  //   g.next();
  //   std::cout << g.cur_value() << std::endl;
  // }
}
