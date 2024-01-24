// https://www.bennyhuo.com/2022/03/11/cpp-coroutines-02-generator/
#include <coroutine>
#include <utility>

template <typename T> struct Generator {
  struct promise_type {
    void unhandled_exception() noexcept {}
    Generator get_return_object() { return Generator{this}; }
    std::suspend_always initial_suspend() { return {}; }
    std::suspend_always final_suspend() noexcept { return {}; }
    std::suspend_always await_transform(int value) {
      value_ = value;
      return {};
    }
    void return_void() {}
    T value_;
  };

  using Handle = std::coroutine_handle<promise_type>;
  explicit Generator(promise_type *p) : handle_(Handle::from_promise(*p)) {}
  Generator(Generator &&g) : handle_(std::exchange(g.handle_, nullptr)) {}
  ~Generator() { handle_.destroy(); }

  void next() { handle_.resume(); }
  int get() { return handle_.promise().value_; }
  bool done() { return handle_.done(); }

private:
  Handle handle_;
};

template <typename T> struct Generator {
  struct promise_type {
    // ...
  };

  using Handle = std::coroutine_handle<promise_type>;
  explicit Generator(promise_type *p) : handle_(Handle::from_promise(*p)) {}
  Generator(Generator &&g) : handle_(std::exchange(g.handle_, nullptr)) {}
  ~Generator() { handle_.destroy(); }

  void next() { handle_.resume(); }
  int get() { return handle_.promise().value_; }
  bool done() { return handle_.done(); }

private:
  Handle handle_;
};