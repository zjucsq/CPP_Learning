// https://www.jianshu.com/p/c610ad5db6b7
#include <atomic>
#include <thread>
#include <utility>

namespace mystl {
class jthread {
public:
  jthread() noexcept = default;

  template <typename F, typename... Args>
  explicit jthread(F &&f, Args &&...args)
      : m_thread(std::forward<F>(f), std::forward<Args>(args)...) {}

  jthread(const jthread &) = delete;
  jthread &operator=(const jthread &) = delete;

  jthread(jthread &&other) noexcept : m_thread(std::move(other.m_thread)) {}

  jthread &operator=(jthread &&other) noexcept {
    if (this != &other) {
      jthread tmp{std::move(other)};
      swap(tmp);
    }
    return *this;
  }

  ~jthread() {
    if (joinable()) {
      join();
    }
  }

  void join() {
    if (!joinable())
      throw std::logic_error("jthread is not joinable");
    m_thread.join();
  }

  void detach() {
    if (!joinable())
      throw std::logic_error("jthread is not joinable");
    m_thread.detach();
  }

  bool joinable() const noexcept { return m_thread.joinable(); }

  std::thread::id get_id() const noexcept { return m_thread.get_id(); }

  void swap(jthread &other) noexcept { std::swap(m_thread, other.m_thread); }

private:
  std::thread m_thread;
};
} // namespace mystl
