#include <iostream>

#include "../include/6sleep/scheduler.h"
#include "../include/io_utils.h"

int main() {
  auto scheduler = Scheduler();

  debug("start") scheduler.execute([]() { debug("2"); }, 100);
  scheduler.execute([]() { debug("1"); }, 50);
  scheduler.execute([]() { debug("6"); }, 1000);
  scheduler.execute([]() { debug("5"); }, 500);
  scheduler.execute([]() { debug("3"); }, 200);
  scheduler.execute([]() { debug("4"); }, 300);

  scheduler.shutdown();
  scheduler.join();
}