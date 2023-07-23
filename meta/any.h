// Two ways to implement any, one is by using virtual functions directly,
// the other is to simulate virtual functions yourself, and the second can use
// small object optimization.

#pragma once

#include <any>
#include <typeinfo>
#include <utility>

namespace mystl {
// Implemented by virtual functions directly.
// https://zhuanlan.zhihu.com/p/597223487
// https://zhuanlan.zhihu.com/p/386392397
class any_vi {};

// Implemented by simulating virtual functions, but no SSO.
// https://wanghenshui.github.io/2021/02/06/any.html
class any_base {
private:
  using destroy_fn_t = void (*)(void *);
  using clone_fn_t = void *(*)(void *);
  using get_type_fn_t = const std::type_info *(*)();

  template <typename T> static void destroy_fn(void *p) {
    delete static_cast<T *>(p);
  }

  template <typename T> static void *clone_fn(void *p) {
    return static_cast<void *>(new T(*static_cast<T *>(p)));
  }

  // template <typename T> static void *get_type_fn() {
  //   return static_cast<void *>(new T(*static_cast<T *>(p)));
  // }

public:
  template <typename T> friend T &any_cast(const any_base &);

  any_base()
      : data(nullptr), destroy(nullptr), clone(nullptr), t(&typeid(void)) {}
  template <typename T>
  any_base(T &&t)
      : data(new T(std::forward<T>(t))), destroy(destroy_fn<T>),
        clone(clone_fn<T>), t(&typeid(T)) {}

  any_base(const any_base &other)
      : data(other.clone(other.data)), destroy(other.destroy),
        clone(other.clone), t(other.t) {}

  any_base(any_base &&other) noexcept
      : data(other.data), destroy(other.destroy), clone(other.clone),
        t(other.t) {
    other.clear();
  }

  any_base &operator=(const any_base &rhs) {
    if (this != &rhs) {
      any_base tmp{rhs};
      swap(tmp);
    }
    return *this;
  }

  any_base &operator=(any_base &&rhs) noexcept {
    if (this != &rhs) {
      swap(rhs);
      rhs.clear();
    }
    return *this;
  }

  ~any_base() {
    if (data)
      destroy(data);
  }

  bool has_value() { return data; }

private:
  void *data;
  destroy_fn_t destroy;
  clone_fn_t clone;
  // get_type_fn_t get_type;
  const std::type_info *t;

  void clear() {
    data = nullptr;
    destroy = nullptr;
    clone = nullptr;
    t = nullptr;
  }

  void swap(any_base &other) noexcept {
    std::swap(data, other.data);
    std::swap(destroy, other.destroy);
    std::swap(clone, other.clone);
    std::swap(t, other.t);
  }
};

template <typename T> T &any_cast(const any_base &operand) {
  if (*operand.t == typeid(T))
    return *static_cast<T *>(operand.data);
  else
    throw std::bad_any_cast();
}

// Implemented by simulating virtual functions with SSO.
// https://zhuanlan.zhihu.com/p/394827450
class any {};

} // namespace mystl