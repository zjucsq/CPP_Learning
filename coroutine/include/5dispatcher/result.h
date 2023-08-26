#pragma once

#include <exception>
#include <variant>

struct NoResultError : std::exception {
  [[nodiscard]] const char *what() const noexcept override { return "result is unset"; }
};

template <typename T> struct Result {
  explicit Result() = default;

  explicit Result(T &&value) : result_(value){};

  explicit Result(std::exception_ptr &&exception_ptr) : result_(exception_ptr) {}

  T get_or_throw() {
    if (auto exception = std::get_if<std::exception_ptr>(&result_)) {
      std::rethrow_exception(*exception);
    }
    if (auto res = std::get_if<T>(&result_)) {
      return *res;
    }
    throw NoResultError{};
  }

private:
  std::variant<std::monostate, T, std::exception_ptr> result_;
};

template <> struct Result<void> {
  explicit Result() = default;

  explicit Result(std::exception_ptr &&exception_ptr) : _exception_ptr(exception_ptr) {}

  void get_or_throw() {
    if (_exception_ptr) {
      std::rethrow_exception(_exception_ptr);
    }
  }

private:
  std::exception_ptr _exception_ptr;
};