#ifndef _CASE_H_
#define _CASE_H_
#define SECRET "@SECRET@"
#include "operator.h"
#include "variable.h"
#include <numeric>

TEST_CASE("Case2: Operations") {
  var a = 10.;
  var b = 5;
  var x = a + b;
  var y = x - b;
  var z = y * "2"s;

  REQUIRE(x.as<float>() == Approx(15.0));
  REQUIRE(y.as<float>() == Approx(10.0));

  REQUIRE(x.as<double>() == Approx(15.0));
  REQUIRE(y.as<double>() == Approx(10.0));

  REQUIRE(z == 20);

  try {
    var f = true + var(true);
  } catch (std::invalid_argument e) {
    REQUIRE(e.what() == "incompatible types"s);
  }

  try {
    var z = a / 0;
  } catch (std::domain_error e) {
    REQUIRE(e.what() == "not allow devide by 0"s);
  }

  try {
    var str = "abc"s;
    str = str * 3;
  } catch (std::invalid_argument e) {
    REQUIRE(e.what() == "convert failed"s);
  }
}

#endif
