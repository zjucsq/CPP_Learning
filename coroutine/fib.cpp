#include <iostream>

struct fib_frame {
  int a = 1, b = 1;
  int resume() {
    b += a;
    std::swap(a, b);
    return b;
  }
};

int main() {
  fib_frame fib;

  for (int i = 0; i < 10; ++i) {
    int x = fib.resume();
    std::cout << x << std::endl;
  }

  return 0;
}