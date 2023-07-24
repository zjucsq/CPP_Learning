// Two ways to implement any, one is by using virtual functions directly,
// the other is to simulate virtual functions yourself, and the second can use
// small object optimization.

#pragma once

#include <any>
#include <iostream>
#include <sys/_types/_uintptr_t.h>
#include <type_traits>
#include <typeinfo>
#include <utility>

namespace mystl {
// Implemented by virtual functions directly.
// https://zhuanlan.zhihu.com/p/597223487
// https://zhuanlan.zhihu.com/p/386392397
class any_vf {
private:
  struct Base {
    virtual ~Base() {}
    virtual const std::type_info &get_type() = 0;
    virtual Base *clone() = 0;
  };

  template <typename T> struct Data : public Base {
    Data(const T &t) : t_(t) {}
    Data(T &&t) : t_(t) {}
    T t_;
    ~Data() override {}
    const std::type_info &get_type() override { return typeid(T); }
    Base *clone() override { return new Data{t_}; }
  };

public:
  template <typename T> friend T &any_cast(const any_vf &);

  any_vf() : data(nullptr) {}
  template <typename T> any_vf(T &&t) : data(new Data<T>(std::forward<T>(t))) {}

  any_vf(const any_vf &other) : data(other.data->clone()) {}

  any_vf(any_vf &&other) noexcept : data(other.data) { other.data = nullptr; }

  any_vf &operator=(const any_vf &rhs) {
    if (this != &rhs) {
      any_vf tmp{rhs};
      swap(tmp);
    }
    return *this;
  }

  any_vf &operator=(any_vf &&rhs) noexcept {
    if (this != &rhs) {
      swap(rhs);
      rhs.data = nullptr;
    }
    return *this;
  }

  ~any_vf() {
    if (data)
      delete data;
  }

  bool has_value() { return data; }

private:
  Base *data;

  void swap(any_vf &rhs) noexcept { std::swap(data, rhs.data); }

  template <typename T> T &get_data() const {
    return static_cast<Data<T> *>(data)->t_;
  }
};

template <typename T> T &any_cast(const any_vf &operand) {
  if (operand.data->get_type() == typeid(T))
    return operand.get_data<T>();
  else
    throw std::bad_any_cast();
}

// Implemented by simulating virtual functions, but no SSO.
// https://wanghenshui.github.io/2021/02/06/any.html
class any_base {
private:
  using destroy_fn_t = void (*)(void *);
  using clone_fn_t = void *(*)(void *);

  template <typename T> static void destroy_fn(void *p) {
    delete static_cast<T *>(p);
  }

  template <typename T> static void *clone_fn(void *p) {
    return static_cast<void *>(new T(*static_cast<T *>(p)));
  }

public:
  template <typename T> friend T &any_cast(const any_base &);

  any_base() : data(nullptr), destroy(nullptr), clone(nullptr), t(nullptr) {}
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
// https://github.com/gcc-mirror/gcc/blob/master/libstdc++-v3/include/std/any
inline constexpr size_t small_space_size = 2 * sizeof(void *);
inline constexpr size_t trivial_space_size = 3 * sizeof(void *);
struct small_RTTI {
  using destroy_fn_t = void (*)(void *);
  using clone_fn_t = void (*)(void *, const void *);
  using move_fn_t = void (*)(void *, void *);
  template <typename T> static void destroy_fn(void *p) {
    static_cast<T *>(p)->~T();
  }
  template <typename T> static void clone_fn(void *dst, const void *src) {
    new (dst) T(*static_cast<const T *>(src));
  }
  template <typename T> static void move_fn(void *dst, void *src) {
    new (dst) T(std::move(*static_cast<T *>(src)));
  }
  destroy_fn_t _destroy;
  clone_fn_t _clone;
  move_fn_t _move;
};
struct big_RTTI {
  using destroy_fn_t = void (*)(void *);
  using clone_fn_t = void *(*)(const void *);
  template <typename T> static void destroy_fn(void *p) {
    delete static_cast<T *>(p);
  }
  template <typename T> static void *clone_fn(const void *p) {
    return static_cast<void *>(new T(*static_cast<const T *>(p)));
  }
  destroy_fn_t _destroy;
  clone_fn_t _clone;
};
struct small_storage_t {
  char data[small_space_size];
  const small_RTTI *_RTTI;
};
struct big_storage_t {
  char padding[small_space_size - sizeof(void *)];
  void *data_ptr;
  const big_RTTI *_RTTI;
};
struct storage_t {
  union {
    char trival_data[trivial_space_size];
    small_storage_t small_storage;
    big_storage_t big_storage;
  };
  uintptr_t type_data;
};
template <typename T>
inline constexpr small_RTTI small_RTTI_obj = {&small_RTTI::destroy_fn<T>,
                                              &small_RTTI::clone_fn<T>,
                                              &small_RTTI::move_fn<T>};
template <typename T>
inline constexpr big_RTTI big_RTTI_obj = {&big_RTTI::destroy_fn<T>,
                                          &big_RTTI::clone_fn<T>};
template <typename T>
inline constexpr bool is_small =
    alignof(T) <= alignof(double) && sizeof(T) <= small_space_size &&
    std::is_nothrow_move_constructible_v<T>;
template <typename T>
inline constexpr bool is_trival =
    alignof(T) <= alignof(double) && sizeof(T) <= trivial_space_size &&
    std::is_trivially_copyable_v<T>;
class any {
public:
  template <typename T> friend T any_cast(const any &);

  any() { clear(); }

  template <typename T> any(const T &t) {
    storage.type_data = reinterpret_cast<uintptr_t>(&typeid(T));
    if constexpr (is_trival<T>) {
      storage.type_data |= 1;
      memcpy(storage.trival_data, &t, sizeof(T));
    } else if constexpr (is_small<T>) {
      storage.type_data |= 2;
      storage.small_storage._RTTI = &small_RTTI_obj<T>;
      storage.small_storage._RTTI->_clone(storage.small_storage.data, &t);
    } else {
      storage.type_data |= 3;
      storage.big_storage._RTTI = &big_RTTI_obj<T>;
      storage.big_storage.data_ptr = storage.big_storage._RTTI->_clone(&t);
    }
  }

  any(const any &other) {
    storage.type_data = other.storage.type_data;
    if ((storage.type_data & 3) == 1) {
      memcpy(storage.trival_data, other.storage.trival_data,
             sizeof(other.storage.trival_data));
    } else if ((storage.type_data & 3) == 2) {
      storage.small_storage._RTTI = other.storage.small_storage._RTTI;
      storage.small_storage._RTTI->_clone(storage.small_storage.data,
                                          other.storage.small_storage.data);
    } else if ((storage.type_data & 3) == 3) {
      storage.big_storage._RTTI = other.storage.big_storage._RTTI;
      storage.big_storage.data_ptr =
          storage.big_storage._RTTI->_clone(other.storage.big_storage.data_ptr);
    }
  }

  any(any &&other) noexcept : storage(other.storage) {
    if ((other.storage.type_data & 3) == 1) {
      memcpy(storage.trival_data, other.storage.trival_data,
             sizeof(other.storage.trival_data));
    } else if ((storage.type_data & 3) == 2) {
      storage.small_storage._RTTI = other.storage.small_storage._RTTI;
      storage.small_storage._RTTI->_move(storage.small_storage.data,
                                         other.storage.small_storage.data);
    } else if ((storage.type_data & 3) == 3) {
      storage.big_storage._RTTI = other.storage.big_storage._RTTI;
      storage.big_storage.data_ptr = other.storage.big_storage.data_ptr;
    }
    other.clear();
  }

  any &operator=(const any &rhs) {
    if (this != &rhs) {
      any tmp{rhs};
      swap(tmp);
    }
    return *this;
  }

  any &operator=(any &&rhs) noexcept {
    if (this != &rhs) {
      swap(rhs);
      rhs.clear();
    }
    return *this;
  }

  ~any() {
    if (has_value()) {
      if ((storage.type_data & 3) == 2) {
        storage.small_storage._RTTI->_destroy(storage.small_storage.data);
      } else if ((storage.type_data & 3) == 3) {
        storage.big_storage._RTTI->_destroy(storage.big_storage.data_ptr);
      }
    }
  }

  bool has_value() { return storage.type_data != 0; }

private:
  storage_t storage;

  const std::type_info *get_type_info() const {
    return reinterpret_cast<const std::type_info *>(storage.type_data &
                                                    (-1 << 2));
  }

  void clear() { storage.type_data = 0; }

  const void *get_data() const {
    const char *ret = nullptr;
    if ((storage.type_data & 3) == 1) {
      ret = storage.trival_data;
    } else if ((storage.type_data & 3) == 2) {
      ret = storage.small_storage.data;
    } else if ((storage.type_data & 3) == 3) {
      ret = static_cast<const char *>(storage.big_storage.data_ptr);
    }
    return static_cast<const void *>(ret);
  }

  void swap(any &other) noexcept { std::swap(storage, other.storage); }
};

template <typename T> T any_cast(const any &operand) {
  if (*operand.get_type_info() == typeid(T))
    return *static_cast<const T *>(operand.get_data());
  else
    throw std::bad_any_cast();
}

} // namespace mystl