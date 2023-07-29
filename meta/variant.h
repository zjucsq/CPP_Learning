#include <algorithm>
#include <cstddef>
#include <type_traits>
#include <utility>

namespace mystl {
template <typename T0, typename... T>
constexpr size_t max_size = std::max(sizeof(T0), max_size<T...>);
template <typename T> constexpr size_t max_size<T> = sizeof(T);
template <typename U, typename T0, typename... T>
constexpr size_t get_idx = 1 + get_idx<U, T...>;
template <typename T0, typename... T>
constexpr size_t get_idx<T0, T0, T...> = 0;
// template <size_t N, typename T0, typename... T> struct get_type_ {
//   using type = typename get_type_<N - 1, T...>::type;
// };
// template <typename T0, typename... T> struct get_type_<0, T0, T...> {
//   using type = T0;
// };
// template <size_t N, typename T0, typename... T>
// using get_type = typename get_type_<N, T0, T...>::type;

template <typename... T> class variant;
template <size_t N, typename T> struct get_type_;
template <size_t N, typename T0, typename... T>
// struct get_type_<N, variant<T0, T...>> {
//   using type = typename get_type_<N - 1, variant<T...>>::type;
// };
struct get_type_<N, variant<T0, T...>> : get_type_<N - 1, variant<T...>> {};
template <typename T0, typename... T> struct get_type_<0, variant<T0, T...>> {
  using type = T0;
};
template <size_t N, typename T0, typename... T>
using get_type = typename get_type_<N, variant<T0, T...>>::type;

template <typename... T> class variant {
public:
  template <size_t I, class... Types>
  friend constexpr get_type<I, variant<Types...>> &get(variant<Types...> &v);

  template <class U, class... Types>
  friend constexpr U &get(variant<Types...> &v);

  template <class U> constexpr variant(U &&u) noexcept {
    idx = get_idx<U, T...>;
    new (&buf) U(std::forward<U>(u));
  }
  constexpr size_t index() const noexcept { return idx; }

private:
  size_t idx;
  std::aligned_storage_t<max_size<T...>, sizeof(void *)> buf;
};
template <size_t I, class... Types>
constexpr get_type<I, variant<Types...>> &get(variant<Types...> &v) {
  return *reinterpret_cast<get_type<I, variant<Types...>> *>(&v.buf);
};

template <class T, class... Types> constexpr T &get(variant<Types...> &v) {
  return *reinterpret_cast<T *>(&v.buf);
};

// Another way by union
// https://www.cnblogs.com/ipnah/p/15998604.html
template <typename... T> class variant0 {
public:
private:
};

} // namespace mystl
