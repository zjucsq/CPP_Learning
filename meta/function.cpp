#include "function.h"
#include <functional>
#include <iostream>

void func() { std::cout << "func" << ' '; }

struct functor {
  void operator()() { std::cout << "functor" << ' '; }
};

void func_1() { std::cout << "function" << ' '; }

template <typename T> void print_size(T &t) {
  std::cout << sizeof(T) << ' ' << sizeof(t) << std::endl;
}

template <template <typename> typename F> void test() {
  F<void()> f;
  f = func;
  f();
  print_size(f);
  f = functor();
  f();
  print_size(f);
  f = []() { std::cout << "lambda" << ' '; };
  f();
  print_size(f);
  f = std::function<void()>(func_1);
  f();
  print_size(f);
}

int main() {
  test<std::function>();
  test<mystl::function_vf>();
}