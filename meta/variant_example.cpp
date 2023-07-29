// https://zhuanlan.zhihu.com/p/467395504

#include <numbers>
#include <variant>

struct Shape {
  virtual ~Shape() = default;
  virtual double getArea() const = 0;
  virtual double getPerimeter() const = 0;
};

struct Circle : Shape {
  Circle(double r) : r_(r) {}
  double getArea() const override { return std::numbers::pi * r_ * r_; }
  double getPerimeter() const override { return std::numbers::pi * 2 * r_; }
  double r_;
};

struct Rectangle : Shape {
  Rectangle(double w, double h) : w_(w), h_(h) {}
  double getArea() const override { return w_ * h_; }
  double getPerimeter() const override { return 2 * (w_ + h_); }
  double w_, h_;
};

struct Circle_ {
  double r;
};
double getArea(const Circle_ &c) { return std::numbers::pi * c.r * c.r; }
double getPerimeter(const Circle_ &c) { return std::numbers::pi * 2 * c.r; }

struct Rectangle_ {
  double w, h;
};
double getArea(const Rectangle_ &r) { return r.w * r.h; }
double getPerimeter(const Rectangle_ &r) { return 2 * (r.w + r.h); }

using Shape_ = std::variant<Circle_, Rectangle_>;
double getArea(const Shape_ &s) {
  return std::visit([](const auto &data) { return getArea(data); }, s);
}
double getPerimeter(const Shape_ &s) {
  return std::visit([](const auto &data) { return getPerimeter(data); }, s);
}