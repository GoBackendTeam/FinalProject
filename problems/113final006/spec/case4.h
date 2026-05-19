#ifndef _CASE_H_
#define _CASE_H_
#define SECRET "@SECRET@"
#include "operator.h"
#include "variable.h"
#include <numeric>
#include <vector>
#include <string>

TEST_CASE("Case4: computation and nested loop") {
  std::vector<var> symbols = {"x", "y", "z"};
  std::vector<var> coeffs = {1, 2.5, -3};

  std::string expression = "";

  for (std::size_t i = 0; i < symbols.size(); ++i) {
    std::string term = coeffs[i].as<std::string>() + symbols[i].as<std::string>();
    if (i != symbols.size() - 1)
      term += " +";
    expression += term;
  }

  REQUIRE(expression == "1x +2.5y +-3z");

  // Evaluate expression with x=1, y=2, z=3
  std::vector<var> values = {1, 2, 3};
  var result = 0;
  for (std::size_t i = 0; i < values.size(); ++i) {
    result = result + coeffs[i] * values[i];
  }

  REQUIRE(result.as<double>() == Approx(1 * 1 + 2.5 * 2 + -3 * 3)); // = 1 + 5 - 9 = -3
}

#endif
