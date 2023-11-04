#pragma once

#include <algorithm>
#include <iostream>
#include <thread>

namespace concepts {
template <typename T>
concept container = requires(T t) {
  t.begin();
  t.end();
  typename T::iterator;
  requires requires(typename T::iterator it) { ++it; };
};
} // namespace concepts

template <concepts::container C> void print(C c) {
  std::cout << "threadid=" << std::this_thread::get_id() << " input=";
  std::for_each(c.begin(), c.end(), [](const auto &v) { std::cout << v << ' '; });
  std::cout << std::endl;
}