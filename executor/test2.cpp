#include "Future.h"
#include "Promise.h"

int main() {
  Promise<int> p;
  auto future = p.getFuture();
  auto f = std::move(future)
               .thenTry([&output0, record](Try<int> &&t) {
                 record(0);
                 output0 = t.value();
                 return t.value() + 100;
               })
               .thenTry([&output1, &executor, record](Try<int> &&t) {
                 record(1);
                 output1 = t.value();
                 Promise<int> p;
                 auto f = p.getFuture().via(&executor);
                 p.setValue(t.value() + 10);
                 return f;
               })
               .thenValue([&output2, record](int x) {
                 record(2);
                 output2 = x;
                 return std::to_string(x);
               })
               .thenValue([](string &&s) { return 1111.0; });
  p.setValue(1000);
  f.wait();
}