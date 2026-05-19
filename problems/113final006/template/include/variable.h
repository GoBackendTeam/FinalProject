#ifndef _VARIABLE_H_
#define _VARIABLE_H_
#include "value.h"
#include <stdexcept>

using namespace std::string_literals;

class UnknownType {};

/**
 * @brief
 * In our class, polymorphism includes both "runtime" and "compile-time" forms,
 * and they require different techniques.
 *
 * You should implement:
 * - multiple concrete types (e.g. int, double, float, bool, std::string)
 * - basic conversions (var::as<T>)
 *
 * Based on the interface `IValue` (provided in `value.h`),
 * define the necessary classes in this file to complete the system.
 *
 * Hint:
 * Think about how indirection enables "runtime" behavior,
 * and how templates can capture "compile-time" structure.
 */
template <typename T>
class Value : IValue {};

// var a = 30L;
// a.as<int>         // 30
// a.as<float>       // 30.f
// a.as<std::string> // "30"
// a.as<bool>        // true / false
class var {
  public:
  /**
   * @brief Converts the stored value to the specified type T.
   *
   * This function is enabled for arithmetic and boolean types.
   * The actual conversion behavior should be delegated to the underlying implementation.
   *
   * @tparam T The target type to convert to.
   * @return The stored value converted to type T.
   */
  template <typename T>
  inline T as() const {}

  /**
   * @brief These operators are declared here as friends for access purposes.
   *
   * The actual implementations of arithmetic, comparison, and stream operators
   * are defined in `operator.h`. Please refer to that file for details.
   */
  friend var operator+(const var& lhr, const var& rhs);
  friend var operator-(const var& lhr, const var& rhs);
  friend var operator*(const var& lhr, const var& rhs);
  friend var operator/(const var& lhr, const var& rhs);
  friend bool operator==(const var& lhr, const var& rhs);
  friend bool operator!=(const var& lhr, const var& rhs);
  friend std::ostream& operator<<(std::ostream& stream, const var& v);

  /**
   * @brief Dispatcher responsible for dynamic dispatch of operations.
   *
   * This type should support runtime polymorphism through a virtual table.
   * It allows the `var` class to perform operations (such as arithmetic,
   * comparison, and type conversion) correctly according to the actual type
   * of the stored value—even when that type is only known at runtime.
   *
   * You are expected to "Declare" an appropriate type that enables such behavior.
   */
  using Dispatcher = UnknownType; // Please redefine to the correct type

  private:
  var::Dispatcher dispatcher;
};

/**
 * @brief Converts the stored value to a string.
 *
 * This is a specialization of `as()` for `std::string`.
 * The result should represent a textual form of the value.
 *
 * @return The stored value as a std::string.
 */
template <>
inline std::string var::as<std::string>() const {}

inline std::ostream& operator<<(std::ostream& os, const var& v) {
  return os << v.as<std::string>();
}

static_assert(!SameType<var::Dispatcher, UnknownType>::value, "Not allow `var::Dispatcher` equal `UnknownType`");
#endif