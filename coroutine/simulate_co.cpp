#include <iostream>

struct Fib {
  int state;
  int a;
  int b;
  int c;

  Fib() : state(0) {}

  int resume() {
    switch (state) {
    case 0:
      state = 1;
      goto s0;
    case 1:
      state = 2;
      goto s1;
    case 2:
      state = 3;
      goto s2;
    case 3:
      goto s3;
    }
  s0:
    a = 1;
    b = 1;
    return a;
  s1:
    return b;
  s2:
    while (true) {
      c = a + b;
      return c;
    s3:
      a = b;
      b = c;
    }
  }
};

int main() {
  Fib f1, f2;

  for (int i = 0; i < 10; ++i) {
    std::cout << f1.resume() << ' ' << f2.resume() << std::endl;
  }
}