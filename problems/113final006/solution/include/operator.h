#ifndef _OPERATOR_H_
#define _OPERATOR_H_
#include "variable.h"
#include <stdexcept>
#include <iostream>

#define TRY_CALC(OP)                                  \
  const auto& LT = lhs.type();                        \
  const auto& RT = rhs.type();                        \
  if (Type::isCalculated(LT, RT)) {                   \
    return var(lhs.as<double>() OP rhs.as<double>()); \
  }

inline var operator+(const var& lhs, const var& rhs) {
  if (Type::isString(lhs.type()) && Type::isString(rhs.type()))
    return var(lhs.as<std::string>() + rhs.as<std::string>());
  TRY_CALC(+)
  throw std::invalid_argument("incompatible types");
}

inline var operator-(const var& lhs, const var& rhs) {
  TRY_CALC(-)
  throw std::invalid_argument("incompatible types");
}

inline var operator*(const var& lhs, const var& rhs) {
  TRY_CALC(*)
  throw std::invalid_argument("incompatible types");
}

inline var operator/(const var& lhs, const var& rhs) {
  const auto& LT = lhs.type();
  const auto& RT = rhs.type();
  if (Type::isCalculated(LT, RT)) {
    if (rhs.as<double>() == 0.0)
      throw std::domain_error("not allow devide by 0");
    return lhs.as<double>() / rhs.as<double>();
  }
  throw std::invalid_argument("incompatible types");
}

inline bool operator==(const var& lhs, const var& rhs) {
  try {
    return lhs.as<double>() == rhs.as<double>();
  } catch(std::exception _e) {
    return lhs.as<std::string>() == rhs.as<std::string>();
  }
}

inline bool operator!=(const var& lhs, const var& rhs) {
  return !(lhs.as<std::string>() == rhs.as<std::string>());
}

#endif