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

template <typename P, typename R, typename F>
constexpr auto foldL(P &&p, R acc, F &&f, ParserInput in) {
  while (true) {
    auto r = p(in);
    if (!r)
      return std::make_pair(acc, in);
    acc = f(acc, r->first);
    in = r->second;
  }
}

// 匹配多次的parser
template <typename P, typename F,
          typename R = std::invoke_result_t<F, ParserResult<P>>>
constexpr auto many(P &&p, F &&f) {
  return [=](ParserInput s) -> ParserResult<R> {
    R r;
    while (true) {
      auto r1 = p(s);
      if (!r1)
        break;
      r = f(r, r1->first);
      s = r1->second;
    }
    return std::make_pair(r, s);
  };
}

// 匹配至少一次的parser
template <typename P, typename F,
          typename R = std::invoke_result_t<F, ParserResult<P>>>
constexpr auto many1(P &&p, F &&f) {
  return and_(p, many(p, f), f);
}

// 匹配0次或1次的parser
template <typename P, typename F,
          typename R = std::invoke_result_t<F, ParserResult<P>>>
constexpr auto option(P &&p, F &&f) {
  return [=](ParserInput s) -> ParserResult<R> {
    auto r = p(s);
    if (!r)
      return std::make_pair(f(), s);
    return std::make_pair(f(r->first), r->second);
  };
}

// 匹配四则运算表达式的parser，like 1+2*3 return 7
constexpr auto makeExprParser() {
  auto makeNum = [](int n) { return [=](ParserInput) { return n; }; };
  auto makeOp = [](char c) {
    switch (c) {
    case '+':
      return [](int x, int y) -> int { return x + y; };
    case '-':
      return [](int x, int y) -> int { return x - y; };
    case '*':
      return [](int x, int y) -> int { return x * y; };
    case '/':
      return [](int x, int y) -> int { return x / y; };
    default:
      return [](int, int) -> int { return 0; };
    }
  };
  auto digit = and_(makeCharParser('0'), makeCharParser('9'),
                    [](char c1, char c2) { return c1 - '0'; });
  auto num = many1(digit, [](int x, int y) { return x * 10 + y; });
  auto op = and_(makeCharParser('+'), makeCharParser('*'), makeCharParser('-'),
                 makeCharParser('/'),
                 [](char c1, char c2, char c3, char c4) { return c1; });
  auto expr =
      and_(num,
           many(and_(op, num, makeOp),
                [](int x, int y) { return [=](ParserInput) { return y(x); }; }),
           [](int x, auto f) { return f(x); });
  return expr;
}

int main() {
  auto expr = makeExprParser();
  auto r = expr("1+2*3");
  if (r)
    printf("%d\n", r->first);
  return 0;
}