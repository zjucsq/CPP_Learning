#include "Future.h"
#include "Promise.h"

int main() {
  executor::Promise<int> p;
  executor::Future<int> f = p.getFuture();
  p.setValue(43);
  f.wait();
  int v = f.value();
}