# Awaiter

C++ 通过 co_await 表达式来处理协程的挂起，表达式的操作对象则为等待体（awaiter）。需要实现三个接口。

```C++
struct Awaiter {
  bool await_ready();
  // await_ready 返回 bool 类型，如果返回 true，则表示已经就绪，无需挂起；否则表示需要挂起。
  ??? await_suspend(std::coroutine_handle<> coroutine_handle);
  // await_ready 返回 false 时，协程就挂起了。这时候协程的局部变量和挂起点都会被存入协程的状态当中，await_suspend 被调用到。
  // 返回 void 类型或者返回 true，表示当前协程挂起之后将执行权还给当初调用或者恢复当前协程的函数。
  // 返回 false，则恢复执行当前协程。注意此时不同于 await_ready 返回 true 的情形，此时协程已经挂起，await_suspend 返回 false 相当于挂起又立即恢复。
  // 返回其他协程的 coroutine_handle 对象，这时候返回的 coroutine_handle 对应的协程被恢复执行。
  // 抛出异常，此时当前协程恢复执行，并在当前协程当中抛出异常。
  ??? await_resume()；
  // 协程恢复执行之后，等待体的 await_resume 函数被调用。
  // 同样地，await_resume 的返回值类型也是不限定的，返回值将作为 co_await 表达式的返回值。
};
```

标准库提供了两个简单的等待体

```C++
struct suspend_never {
    _LIBCPP_HIDE_FROM_ABI
    constexpr bool await_ready() const noexcept { return true; }
    _LIBCPP_HIDE_FROM_ABI
    constexpr void await_suspend(coroutine_handle<>) const noexcept {}
    _LIBCPP_HIDE_FROM_ABI
    constexpr void await_resume() const noexcept {}
};

struct suspend_always {
    _LIBCPP_HIDE_FROM_ABI
    constexpr bool await_ready() const noexcept { return false; }
    _LIBCPP_HIDE_FROM_ABI
    constexpr void await_suspend(coroutine_handle<>) const noexcept {}
    _LIBCPP_HIDE_FROM_ABI
    constexpr void await_resume() const noexcept {}
};
```

# promise_type

promise_type 支持8个自定义函数

```C++
struct promise_type {
  // 异常处理
  void unhandled_exception() noexcept {}
  // 构造协程返回值对象
  Generator get_return_object() { return Generator{this}; }
  // 协程体执行的第一步，可以在这里实现协程的执行调度
  std::suspend_always initial_suspend() noexcept { return {}; }
  // 协程执行完成或者抛出异常之后会先清理局部变量，接着调用 final_suspend，可以在这里处理其他资源的销毁逻辑
  // 如果选择在这里挂起协程，之后当前协程应当通过 coroutine_handle 的 destroy 函数来直接销毁，而不是 resume
  std::suspend_always final_suspend() noexcept { return {}; }
  // co_return 的处理，二选一
  void return_void() {}
  void return_value(int value) {...}
  // co_yield 的处理
  std::suspend_always yield_value(int value) {
    cur_value_ = value;
    return {};
  }
  // co_await 的处理
  std::suspend_always await_transform(int value) {
    cur_value_ = value;
    return {};
  }
  // 内部维护的局部变量
  int cur_value_;
};
```


# Promiser

函数和协程区别：协程的返回值类型能够实例化下面的模板类型 _Coroutine_traits

```C++
template <class _Ret, class = void>
struct _Coroutine_traits {};

template <class _Ret>
struct _Coroutine_traits<_Ret, void_t<typename _Ret::promise_type>> {
    using promise_type = typename _Ret::promise_type;
};

template <class _Ret, class...>
struct coroutine_traits : _Coroutine_traits<_Ret> {};
```


# Probems

## Coroutine对象何时析构？


## test4，test5死锁问题
