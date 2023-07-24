// https://blog.csdn.net/weixin_42073412/article/details/99876532
// The effect of const on function overloading
// const A&/const A*: work
// const A/ const *A: not work

#include <iostream>

class A {
public:
  A(int a_) : a(a_) {}
  A(A &other) : a(other.a) {}
  int a;
};

int main() {
  const A a1(1);
  // A a2(a1);   Error
}
