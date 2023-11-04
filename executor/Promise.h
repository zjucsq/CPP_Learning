#pragma once

#include "Common.h"
#include "Executor.h"
#include "Future.h"
#include "FutureState.h"
#include "Promise.h"
#include "Try.h"
#include <exception>

namespace executor {
template <typename T> class Future;
template <typename T> class FutureState;

template <typename T> class Promise {
public:
  Future<T> getFuture() {
    logicAssert(!hasFuture_, "already has a future");
    hasFuture_ = true;
    return Future<T>(sharedState_);
  }

  bool valid() const { return sharedState_ != nullptr; }

  void setException(std::exception_ptr error) {
    logicAssert(valid(), "Promise is broken");
    sharedState_->setResult(Try<T>(error));
  }
  void setValue(T value) {
    logicAssert(valid(), "Promise is broken");
    sharedState_->setResult(Try<T>(value));
  }

private:
  FutureState<T> *sharedState_ = nullptr;
  bool hasFuture_ = false;
};
}; // namespace executor