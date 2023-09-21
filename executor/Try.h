#pragma once

#include "Common.h"
#include <cassert>
#include <exception>
#include <optional>
#include <variant>

namespace executor {
template <typename T> class Try {
public:
  Try() = default;
  ~Try() = default;
  Try(const Try &rhs) = delete;
  Try &operator=(const Try &rhs) = delete;
  Try(Try &&rhs) = default;
  Try &operator=(Try &&rhs) = default;

  constexpr bool available() const noexcept { return !std::holds_alternative<std::monostate>(result_); }
  constexpr bool hasError() const noexcept { return std::holds_alternative<std::exception_ptr>(result_); }
  constexpr bool hasValue() const noexcept { return std::holds_alternative<T>(result_); }

  const T &value() const & {
    checkHasTry();
    return std::get<T>(result_);
  }
  T &value() & {
    checkHasTry();
    return std::get<T>(result_);
  }
  T &&value() && {
    checkHasTry();
    return std::move(std::get<T>(result_));
  }
  const T &&value() const && {
    checkHasTry();
    return std::move(std::get<T>(result_));
  }

  void setException(std::exception_ptr ptr) { result_ = ptr; }
  std::exception_ptr getException() {
    logicAssert(!std::holds_alternative<std::exception_ptr>(result_));
    return std::get<std::exception_ptr>(result_);
  }

private:
  AS_INLINE void checkHasTry() const {
    if (std::holds_alternative<T>(result_))
      AS_LIKELY { return; }
    else if (std::holds_alternative<std::exception_ptr>(result_)) {
      std::rethrow_exception(std::get<std::exception_ptr>(result_));
    } else if (std::holds_alternative<std::monostate>(result_)) {
      throw std::logic_error("Try object is empty");
    } else {
      assert(false);
    }
  }

private:
  std::variant<std::monostate, T, std::exception_ptr> result_;
};

template <> class Try<void> {
public:
  Try() = default;
  Try(std::exception_ptr ptr) : result_(ptr) {}
  ~Try() = default;
  Try(const Try &rhs) = delete;
  Try &operator=(std::exception_ptr ptr) {
    result_ = ptr;
    return *this;
  }
  Try &operator=(const Try &rhs) = delete;
  Try(Try &&rhs) = default;
  Try &operator=(Try &&rhs) = default;

  void setException(std::exception_ptr ptr) { result_ = ptr; }
  std::exception_ptr getException() { return result_; }

private:
  std::exception_ptr result_;
};
} // namespace executor