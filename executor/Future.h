#pragma once

#include "Executor.h"
#include "Future.h"
#include "FutureState.h"
#include "Promise.h"

namespace executor {

template <typename T> class Promise;

template <typename T> class Future {
public:
  void setExecutor(Executor *ex) {}

  Executor *getExecutor() {}

  template <typename F> void setContinuation(F &&func) {}

  T get() {
    wait();
    return std::move(*this).value();
  }

  void wait() {}

private:
  FutureState<T> *sharedState_;
};

}; // namespace executor