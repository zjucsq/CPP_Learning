#include "variant.h"
#include <iostream>
#include <string>
#include <type_traits>
#include <variant>

template <template <typename...> typename v> void test() {
  v<int, double, std::string> v1 = 42;
  v<int, double, std::string> v2 = 3.14;
  v<int, double, std::string> v3 = std::string("hello");

  std::cout << get<int>(v1) << '\n';
  std::cout << get<double>(v2) << '\n';
  std::cout << get<std::string>(v3) << '\n';

  std::cout << get<0>(v1) << '\n';
  std::cout << get<1>(v2) << '\n';
  std::cout << get<2>(v3) << '\n';

  std::cout << v1.index() << '\n';
  std::cout << v2.index() << '\n';
  std::cout << v3.index() << '\n';

  // std::cout << get<v1.index()>(v1) << '\n';

  v1 = v2;

  std::cout << get<double>(v1) << '\n';
}

int main() {
  static_assert(std::is_same_v<int, mystl::get_type<0, mystl::variant<int>>>);
  test<std::variant>();
  test<mystl::variant>();
}