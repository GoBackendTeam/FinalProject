#ifndef _CASE_H_
#define _CASE_H_
#define SECRET "@SECRET@"
#include "operator.h"
#include "variable.h"

using namespace std::string_literals;

TEST_CASE("Case: Declare", "[variable]") {
  var a = 10;
  var b = 10.;
  var c = 10.f;
  var d = true;
  var str1 = "1234"s;
  var str2 = str1 + "5678"s;

  CHECK(str2 == "12345678"s);
  CHECK(str1 + 100 == 1334);
}

TEST_CASE("Case: Member Functions", "[variable]") {
  var a = 10.;
  var b = 5;
  var x = a + b;
  var y = x - b;
  var z = y * "2"s;

  // doctest::Approx 轉為 Approx
  CHECK(x.as<float>() == Approx(15.0));
  CHECK(y.as<float>() == Approx(10.0));

  CHECK(x.as<double>() == Approx(15.0));
  CHECK(y.as<double>() == Approx(10.0));

  CHECK(z == 20);

  auto expr1 = []() {
    auto f = true + var(true);
  };

  auto expr2 = [&a, &z]() {
    auto z = a / 0;
  };
  CHECK_THROWS_WITH(expr1(), "incompatible types");
  CHECK_THROWS_WITH(expr2(), "not allow devide by 0");
  CHECK_THROWS_WITH("abc"s * 3, "convert failed");
}

TEST_CASE("Case: computation and nested loop", "[logic]") {
  std::vector<var> symbols = {"x", "y", "z"};
  std::vector<var> coeffs = {1, 2.5, -3};

  std::string expression = "";

  for (std::size_t i = 0; i < symbols.size(); ++i) {
    std::string term = coeffs[i].as<std::string>() + symbols[i].as<std::string>();
    if (i != symbols.size() - 1)
      term += " +";
    expression += term;
  }

  CHECK(expression == "1x +2.5y +-3z");

  // Evaluate expression with x=1, y=2, z=3
  std::vector<var> values = {1, 2, 3};
  var result = 0;
  for (std::size_t i = 0; i < values.size(); ++i) {
    result = result + coeffs[i] * values[i];
  }

  CHECK(result.as<double>() == Approx(1 * 1 + 2.5 * 2 + -3 * 3)); // = 1 + 5 - 9 = -3
}

#endif
