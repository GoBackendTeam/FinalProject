#ifndef _OPERATOR_H_
#define _OPERATOR_H_
#include <stdexcept>
#include "variable.h"

/**
 * @brief Possible operations
 * 
 * String and String
 * | Expression               | Result                                          |
 * | ------------------------ | ----------------------------------------------- |
 * | var("1234"s) + "4567"s   |  ✓ var("12345678"s) (string concatenation)     |
 * | var("1234"s) - "4567"s   |  x std::invalid_argument("incompatible types") |
 * | var("1234"s) * "4567"s   |  x std::invalid_argument("incompatible types") |
 * | var("1234"s) / "4567"s   |  x std::invalid_argument("incompatible types") |
 * 
 * Numeric and Numeric
 * | Expression      | Result                                         |
 * | --------------- | ---------------------------------------------- |
 * | var(1) + 2.5    | ✓ var(3.5)                                    |
 * | var(3) - 1.0f   | ✓ var(2.0)                                    |
 * | var(4.0) * 2    | ✓ var(8.0)                                    |
 * | var(10) / 2     | ✓ var(5.0)                                    |
 * | var(1) / 0      | x std::domain_error("not allow devide by 0")  |
 * 
 * Numeric & String
 * | Expression          | Result       |
 * | ------------------- | ------------ |
 * | var(1) + "2.5"s     | ✓ var(3.5)  |
 * | "4.2"s * var(2)     | ✓ var(8.4)  |
 * | var(3.0) - "1.5"s   | ✓ var(1.5)  |
 * | "3.0"s / var(1.5)   | ✓ var(2.0)  |
 * 
 * Numeric & String (non-convertible)
 * | Expression          | Result       |
 * | ------------------- | ------------ |
 * | var(1) + "2.5"s     | ✓ var(3.5)  |
 * | "4.2"s * var(2)     | ✓ var(8.4)  |
 * | var(3.0) - "1.5"s   | ✓ var(1.5)  |
 * | "3.0"s / var(1.5)   | ✓ var(2.0)` |
 */

inline var operator+(const var& lhs, const var& rhs) {
  throw std::invalid_argument("incompatible types");
}

inline var operator-(const var& lhs, const var& rhs) {
  throw std::invalid_argument("incompatible types");
}

inline var operator*(const var& lhs, const var& rhs) {
  throw std::invalid_argument("incompatible types");
}

inline var operator/(const var& lhs, const var& rhs) {
  throw std::invalid_argument("incompatible types");
}

inline bool operator==(const var& lhs, const var& rhs) {
  try {
    return lhs.as<double>() == rhs.as<double>();
  } catch (std::exception _e) {
    return lhs.as<std::string>() == rhs.as<std::string>();
  }
}

inline bool operator!=(const var& lhs, const var& rhs) {
  return !(lhs == rhs);
}

#endif