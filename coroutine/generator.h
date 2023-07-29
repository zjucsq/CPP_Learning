#include <coroutine>
#include <iterator>
#include <utility>

struct GeneratorEnd {};

template <typename HandleType, typename ValueType> struct GeneratorIter {
  using iterator_category = std::input_iterator_tag;
  using difference_type = std::ptrdiff_t;
  using value_type = std::remove_reference_t<ValueType>;
  using reference = std::conditional_t<std::is_reference_v<ValueType>,
                                       ValueType, ValueType &>;
  using pointer = std::add_pointer_t<ValueType>;

  bool operator!=(const GeneratorEnd &) { return !h.done(); }
  void operator++() { h(); } // h() = h.resume()
  value_type &operator*() { return h.promise().cur_value_; }

  HandleType h;
};

template <typename T> struct Generator {
  struct promise_type {
    void unhandled_exception() noexcept {}
    Generator get_return_object() { return Generator{this}; }
    std::suspend_always initial_suspend() noexcept { return {}; }
    std::suspend_always final_suspend() noexcept { return {}; }
    void return_void() {}
    std::suspend_always yield_value(int value) {
      cur_value_ = value;
      return {};
    }
    // std::suspend_always await_transform(int value) {
    //   cur_value_ = value;
    //   return {};
    // }
    T cur_value_;
  };
  using Handle = std::coroutine_handle<promise_type>;

  explicit Generator(promise_type *p) : mCoroHdl(Handle::from_promise(*p)) {}
  Generator(Generator &&rhs) : mCoroHdl(std::exchange(rhs.mCoroHdl, nullptr)) {}
  ~Generator() {
    if (mCoroHdl)
      mCoroHdl.destroy();
  }

  void next() { return mCoroHdl.resume(); }
  bool done() { return mCoroHdl.done(); }
  int cur_value() { return mCoroHdl.promise().cur_value_; }

  auto end() const { return GeneratorEnd{}; }
  auto begin() const {
    auto it = GeneratorIter<Handle, T>{mCoroHdl};
    if (!begin_) {
      ++it;
      begin_ = true;
    }
    return it;
  }

private:
  Handle mCoroHdl{};
  mutable bool begin_{false};
};