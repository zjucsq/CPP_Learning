#include <iostream>
#include <utility>

template <typename T> class A {
public:
  A(const T &t) : t_(t) {}
  A(T &&t) : t_(t) {}
  //   A(A &other) : a(other.a) {}
  T t_;
};

int main() {
  int x = 1;
  A a0(x);
  A a1(1);
  // A a2(a1);   Error
}
