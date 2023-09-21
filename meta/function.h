#include <cstddef>
#include <type_traits>
#include <utility>

namespace mystl {
// Implemented by using virtual functions
// https://shaharmike.com/cpp/naive-std-function/
// https://stackoverflow.com/questions/18453145/how-is-stdfunction-implemented
template <typename F> class function_vf;
template <typename Ret, typename... Args> class function_vf<Ret(Args...)> {
public:
  function_vf() : func(nullptr) {}

  template <typename F> function_vf(F &&f) : func(new Callable<F>(std::forward<F>(f))) {}

  function_vf(const function_vf &f) : func(f.func->clone()) {}

  function_vf &operator=(const function_vf &f) {
    if (this != &f) {
      function_vf tmp{f};
      std::swap(func, tmp.func);
    }
    return *this;
  }

  ~function_vf() {
    if (func)
      delete func;
  }

  Ret operator()(Args... args) { return func->operator()(args...); }

private:
  class Base {
  public:
    virtual ~Base() = default;
    virtual Ret operator()(Args... args) = 0;
    virtual Base *clone() = 0;
  };

  template <typename F> class Callable : public Base {
  public:
    Callable(const F &f_) : f(f_) {}
    Ret operator()(Args... args) override { return f(args...); }
    Base *clone() override { return new Callable(f); }

  private:
    F f;
  };

  Base *func;
};
// https://zhuanlan.zhihu.com/p/66301236
// function in libcxx
// https://www.cnblogs.com/jerry-fuyi/p/std_function_interface_implementation.html
constexpr size_t small_space_size = 2 * sizeof(void *);
template <typename T> constexpr bool is_small = sizeof(T) <= small_space_size;

template <typename F> class function;
template <typename Ret, typename... Args> class function<Ret(Args...)> {};
}; // namespace mystl
