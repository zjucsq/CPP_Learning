#include <optional>
#include <string_view>
#include <type_traits>
#include <utility>

using ParserInput = std::string_view;
template <typename T>
using ParserResult = std::optional<std::pair<T, ParserInput>>;
template <typename T> using Parser = auto(*)(ParserInput) -> ParserResult<T>;

constexpr auto makeCharParser(char c) {
  return [=](ParserInput s) -> ParserResult<char> {
    if (s.empty() || c != s[0])
      return std::nullopt;
    return std::make_pair(s[0], ParserInput{s.begin() + 1, s.end()});
  };
}

template <
    typename P1, typename P2, typename F,
    typename R = std::invoke_result_t<F, ParserResult<P1>, ParserResult<P2>>>
constexpr auto and_(P1 &&p1, P2 &&p2, F &&f) {
  return [=](ParserInput s) -> ParserResult<R> {
    auto r1 = p1(s);
    if (!r1)
      return std::nullopt;
    auto r2 = p2(r1->second);
    if (!r2)
      return std::nullopt;
    return std::make_pair(f(r1->first, r2->first), r2->second);
  };
}

// template <typename P, typename >
