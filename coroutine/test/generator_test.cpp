#include <iostream>

#include "../include/generator.h"

Generator<int> sequence() {
  int i = 0;
  while (i <= 1) {
    co_await i++;
  }
}

int main() {
  auto gen = sequence();
  for (int i = 0; i < 15; ++i) {
    if (!gen.done()) {
      gen.next();
      if (!gen.done()) {
        std::cout << gen.get() << std::endl;
      }
    }
  }
}
