#include <coroutine>
#include <exception>
#include <utility>
#include <variant>

struct NoResultError : std::exception {
  [[nodiscard]] const char* what() const noexcept override {
    return "result is unset";
  }
};

template <typename T>
struct Result {
  constexpr bool has_value() const noexcept {
    return std::get_if<std::monostate>(&result_) == nullptr;
  }
  template <typename R>
  constexpr void set_value(R&& value) noexcept {
    result_.template emplace<T>(std::forward<R>(value));
  }
  void set_exception(std::exception_ptr exception) noexcept {
    result_ = exception;
  }
  // void unhandled_exception() noexcept { result_ = std::current_exception(); }
  // template <typename R>  // for promise_type
  // constexpr void return_value(R&& value) noexcept {
  //   return set_value(std::forward<R>(value));
  // }

  constexpr T result() & {
    if (auto exception = std::get_if<std::exception_ptr>(&result_)) {
      std::rethrow_exception(exception);
    }
    if (auto res = std::get_if<T>(&result_)) {
      return *res;
    }
    throw NoResultError{};
  }
  constexpr T result() && {
    if (auto exception = std::get_if<std::exception_ptr>(&result_)) {
      std::rethrow_exception(exception);
    }
    if (auto res = std::get_if<T>(&result_)) {
      return std::move(*res);
    }
    throw NoResultError{};
  }

 private:
  std::variant<std::monostate, T, std::exception_ptr> result_;
};

template <typename R = void>
struct Task {
  struct promise_type {
    void unhandled_exception() noexcept {}
    Task get_return_object() { return Task{this}; }
    std::suspend_always initial_suspend() { return {}; }
    std::suspend_always final_suspend() noexcept { return {}; }
    std::suspend_always await_transform(int value) { return {}; }
    void return_void() {}
  };

  using Handle = std::coroutine_handle<promise_type>;
  explicit Task(promise_type* p) : handle_(Handle::from_promise(*p)) {}
  Task(Task&& g) : handle_(std::exchange(g.handle_, nullptr)) {}
  ~Task() { handle_.destroy(); }

  int next() {
    handle_();
    return handle_.promise().value_;
  }
  bool done() { return handle_.done(); }

 private:
  Handle handle_;
};