#include "any.h"
#include <any>
#include <iostream>

template <typename TAny> void test() {
  std::cout << "Testing " << typeid(TAny).name() << std::endl;

  TAny a = 123;
  std::cout << any_cast<int>(a) << std::endl;

  TAny b = std::string("hello");
  std::cout << any_cast<std::string>(b) << std::endl;

  try {
    std::cout << any_cast<std::string>(a) << std::endl;
    std::cout << any_cast<int>(b) << std::endl;
  } catch (std::bad_any_cast &e) {
    std::cout << e.what() << std::endl;
  }

  TAny c;
  std::cout << a.has_value() << b.has_value() << c.has_value() << std::endl;
  c = std::move(a);
  std::cout << a.has_value() << b.has_value() << c.has_value() << std::endl;
  std::cout << sizeof(a) << sizeof(b) << sizeof(c) << std::endl;
}

int main() {
  test<std::any>();
  test<mystl::any_vf>();
  test<mystl::any_base>();
  test<mystl::any>();

  return 0;
}
