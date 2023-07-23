#include <__ranges/iota_view.h>
#include <__ranges/take_view.h>
#include <__ranges/transform_view.h>
#include <iostream>
#include <ranges>
#include <vector>

int main() {
  std::vector<int> v{1, 2, 3, 4, 5, 6};
  auto res = std::views::iota(1) |
             std::views::filter([](int n) { return n % 2 == 0; }) |
             std::views::transform([](int n) { return 2 * n; }) |
             std::views::take(10);

  for (auto v : res) {
    std::cout << v << std::endl;
  }
}