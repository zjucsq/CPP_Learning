#include <iostream>
#include <optional>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>

using ParserInput = std::string_view;
template <typename T>
using ParserResult = std::optional<std::pair<T, ParserInput>>;
template <typename T> using Parser = auto(*)(ParserInput) -> ParserResult<T>;
template <typename> struct Parser_trait;
template <typename T> struct Parser_trait<Parser<T>> {
  using type = T;
};
template <typename T> using Parser_t = typename Parser_trait<T>::type;

// combine :: Parser a -> Parser b -> (a -> b -> c) -> Parser c
template <typename P1, typename P2, typename F,
          typename R = std::invoke_result_t<F, Parser_t<P1>, Parser_t<P2>>>
constexpr auto combine(P1 &&p1, P2 &&p2, F &&f) {
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

// foldL :: Parser a -> b -> (b -> a -> b) -> ParserInput -> ParserResult b
template <typename P, typename R, typename F>
constexpr auto foldL(P &&p, R acc, F &&f, ParserInput in) -> ParserResult<R> {
  while (true) {
    auto r = p(in);
    if (!r)
      return std::make_pair(acc, in);
    acc = f(acc, r->first);
    in = r->second;
  }
};

// many :: Parser a -> Parser monostate
template <typename P> constexpr auto many(P &&p) {
  return
      [p = std::forward<P>(p)](ParserInput s) -> ParserResult<std::monostate> {
        return FoldL(
            p, std::monostate{}, [](auto acc, auto) { return acc; }, s);
      };
};

// atLeast :: Parser a -> b -> (b -> a -> b) -> Parser b
template <typename P, typename R, typename F>
constexpr auto atLeast(P &&p, R &&init, F &&f) {
  static_assert(std::is_same_v<std::invoke_result_t<F, R, Parser_t<P>>, R>,
                "type mismatch!");
  return [p = std::forward<P>(p), f = std::forward<F>(f),
          init = std::forward<R>(init)](ParserInput s) -> ParserResult<R> {
    auto r = p(s);
    if (!r)
      return std::nullopt;
    return foldL(p, f(init, r->first), f, r->second);
  };
};

// option :: Parser a -> a -> Parser a
template <typename P, typename R = Parser_t<P>>
constexpr auto option(P &&p, R &&defaultV) {
  return [=](ParserInput s) -> ParserResult<R> {
    auto r = p(s);
    if (!r)
      return make_pair(defaultV, s);
    return r;
  };
};

constexpr auto makeCharParser(char c) {
  return [=](ParserInput s) -> ParserResult<char> {
    if (s.empty() || c != s[0])
      return std::nullopt;
    return std::make_pair(s[0], ParserInput{s.begin() + 1, s.end()});
  };
}

// int parser
constexpr auto intParser(ParserInput s) -> ParserResult<int> {
  int r = 0;
  for (auto c : s) {
    if (c < '0' || c > '9')
      break;
    r = r * 10 + (c - '0');
  }
  return std::make_pair(r, ParserInput{s.begin(), s.end()});
}

int main() {
  constexpr ParserInput s = "12345";
  constexpr auto r = intParser(s);
  if (r)
    std::cout << r->first << std::endl;
  return 0;
}