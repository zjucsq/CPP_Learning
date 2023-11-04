#pragma once

#include "Executor.h"
#include "Future.h"
#include "FutureState.h"
#include "Promise.h"
#include <type_traits>

namespace executor {

template <typename T> class Promise;
template <typename T> class FutureState;

template <typename T> class Future {
public:
  Future(FutureState<T> *fs) : sharedState_(fs) {}

  void setExecutor(Executor *ex) {}

  Executor *getExecutor() {}

  template <typename F> void setContinuation(F &&func) {}

  Try<T> result() requires(!std::is_void_v<T>) { return sharedState_->getTry(); }

  T get() {
    wait();
    return std::move(*this).value();
  }

  T value() { return result().value(); }

  void wait() {
    while (sharedState_->hasResult())
      ;
  }

private:
  FutureState<T> *sharedState_;
};

}; // namespace executor