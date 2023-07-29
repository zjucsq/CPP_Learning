// https://www.youtube.com/watch?v=8sEe-4tig_A
#include <coroutine>
#include <iostream>
#include <string>
using namespace std::literals;

struct Chat {
  struct promise_type {
    std::string _msgOut{}, _msgIn{}; // store value for coroutine

    void unhandled_exception() noexcept {} // what to do in case of exception
    Chat get_return_object() { return Chat{this}; } // coroutine creation
    std::suspend_always initial_suspend() noexcept { return {}; } // startup
    std::suspend_always
    yield_value(std::string msg) noexcept { // value from co_yield
      _msgOut = std::move(msg);
      return {};
    }

    auto await_transform(std::string) noexcept { // value from co_await
      struct awaiter { // customized version instead of using suspend_always or
                       // suspend_never
        promise_type &pt;
        constexpr bool await_ready() const noexcept { return true; }
        std::string await_resume() const noexcept {
          return std::move(pt._msgIn);
        }
        void await_suspend(std::coroutine_handle<>) const noexcept {}
      };
      return awaiter{*this};
    }

    void return_value(std::string msg) noexcept {
      _msgOut = std::move(msg);
    } // value from co_return
    std::suspend_always final_suspend() noexcept { return {}; } // ending
  };

  using Handle = std::coroutine_handle<promise_type>;
  Handle mCoroHdl{};

  explicit Chat(promise_type *p) : mCoroHdl(Handle::from_promise(*p)) {}
  Chat(Chat &&rhs) : mCoroHdl(std::exchange(rhs.mCoroHdl, nullptr)) {}

  ~Chat() {
    if (mCoroHdl)
      mCoroHdl.destroy();
  }

  std::string listen() {
    if (!mCoroHdl.done())
      mCoroHdl.resume();
    return std::move(mCoroHdl.promise()._msgOut);
  }

  void answer(std::string msg) {
    mCoroHdl.promise()._msgIn = msg;
    if (!mCoroHdl.done())
      mCoroHdl.resume();
  }
};

Chat Fun() {
  co_yield "hello!\n"s;

  std::cout << co_await std::string{};

  co_return "Here!\n"s;
}

void Use() {
  Chat chat = Fun();

  std::cout << chat.listen();

  chat.answer("where are you?\n"s);

  std::cout << chat.listen();
}

int main() { Use(); }