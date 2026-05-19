#ifndef _CASE_H_
#define _CASE_H_
#define SECRET "@SECRET@"
#include "operator.h"
#include "variable.h"
#include <numeric>

TEST_CASE("Case3: Mixed coercion and operations") {
  var L1 = 30;
  var L2 = 10.0 / "30"s;
  var L3 = true;
  var L4 = "10"s + "20"s;

  REQUIRE(L1 == 30);
  REQUIRE(L2 == 10. / 30.);
  REQUIRE(L3 == 1);
  REQUIRE(L4.as<std::string>() == "1020");

  try {
    var("abc"s) + 10;
  } catch (std::invalid_argument e) {
    auto invalid_argument = true;
    REQUIRE(invalid_argument == true);
  } catch (std::exception e) {
    auto exception = true;
    REQUIRE(exception == false);
  }

  try {
    var(1 - "hello"s) + 10;
  } catch (std::invalid_argument e) {
    auto invalid_argument = true;
    REQUIRE(invalid_argument == true);
  } catch (std::exception e) {
    auto exception = true;
    REQUIRE(exception == false);
  }
}

#endif
