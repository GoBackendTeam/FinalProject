/**
 * This File "NOT ALLOW" modify
 */
#ifndef _VALUE_H_
#define _VALUE_H_
#include <string>
#include <typeinfo>

// Interface
class IValue {
  public:
  virtual ~IValue() {};
  virtual std::string toString() = 0;
  virtual double toDouble() = 0;
  virtual const std::type_info& type() const = 0;
};

// Helper Function
template <typename T, typename U>
struct SameType {
  constexpr static bool value = false;
};

template <typename T>
struct SameType<T, T> {
  constexpr static bool value = true;
};

namespace Type {
template <typename T>
inline constexpr bool isNumeric() {

  return SameType<T, int>::value || SameType<T, float>::value || SameType<T, double>::value;
}

inline bool isNumeric(const std::type_info& t) {
  return t == typeid(int) || t == typeid(float) || t == typeid(double);
}

template <typename T>
inline bool isString() {
  return SameType<T, std::string>::value;
}

inline bool isString(const std::type_info& t) {
  return t == typeid(std::string);
}

template <typename T>
inline bool isBoolean() {
  return SameType<T, bool>::value;
}

inline bool isBoolean(const std::type_info& t) {
  return t == typeid(bool);
}

inline bool isCalculated(const std::type_info& T, const std::type_info& U) {
  return (!isBoolean(T) && !isBoolean(U));
}

} // namespace Type

#endif