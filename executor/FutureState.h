#pragma once

#include "Common.h"
#include "Executor.h"
#include "Future.h"
#include "Promise.h"
#include "Try.h"
#include <atomic>
#include <cassert>
#include <cstdint>

namespace executor {

namespace detail {

enum class State : uint8_t {
  START = 0,
  ONLY_RESULT = 1,
  ONLY_CONTINUATION = 1 << 1,
  DONE = 1 << 5,
};

constexpr State operator|(const State lhs, const State rhs) { return State((uint8_t)lhs | (uint8_t)rhs); }

constexpr State operator&(const State lhs, const State rhs) { return State((uint8_t)lhs & (uint8_t)rhs); }

}; // namespace detail

template <typename T> class FutureState {
public:
  FutureState() : state_(detail::State::START), result_(), executor_(nullptr) {}

  bool hasResult() const noexcept {
    auto state = state_.load(std::memory_order_acquire);
    return state == detail::State::ONLY_RESULT || state == detail::State::DONE;
  }

  bool hasContinuation() const noexcept {
    auto state = state_.load(std::memory_order_acquire);
    return state == detail::State::ONLY_CONTINUATION || state == detail::State::DONE;
  }

  void setResult(Try<T> &&value) {
    logicAssert(!hasResult(), "FutureState already has a result");

    result_ = std::move(value);
    auto state = state_.load(std::memory_order_acquire);
    switch (state) {
    case detail::State::START:
      if (state_.compare_exchange_strong(state, detail::State::ONLY_RESULT, std::memory_order_release)) {
        return;
      }
      assert(state_.load(std::memory_order_relaxed) == detail::State::ONLY_CONTINUATION);
    case detail::State::ONLY_CONTINUATION:
      if (state_.compare_exchange_strong(state, detail::State::DONE, std::memory_order_release)) {

        return;
      }
    default:
      logicAssert(false, "State Transfer Error");
    }
  }

  template <typename F> void setContinuation(F &&func) {
    logicAssert(!hasContinuation(), "FutureState already has a continuation");

    result_ = std::move(value);
    auto state = state_.load(std::memory_order_acquire);
    switch (state) {
    case detail::State::START:
      if (state_.compare_exchange_strong(state, detail::State::ONLY_RESULT, std::memory_order_release)) {
        return;
      }
      assert(state_.load(std::memory_order_relaxed) == detail::State::ONLY_CONTINUATION);
    case detail::State::ONLY_CONTINUATION:
      if (state_.compare_exchange_strong(state, detail::State::DONE, std::memory_order_release)) {

        return;
      }
    default:
      logicAssert(false, "State Transfer Error");
    }
  }

private:
  std::atomic<detail::State> state_;
  Try<T> result_;
  Executor *executor_;
};
}; // namespace executor