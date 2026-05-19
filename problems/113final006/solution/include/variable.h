#ifndef _VARIABLE_H_
#define _VARIABLE_H_
#include "value.h"
#include <memory>
#include <sstream>
#include <iostream>
#include <string>
using namespace std::string_literals;

class UnknownType {};

template <typename T>
class Value : public IValue {
  public:
  explicit Value(T value)
      : val(value) {}

  ~Value() override {}

  const std::type_info& type() const override {
    return typeid(T);
  }

  std::string toString() override {
    std::stringstream ss;
    ss << this->get();
    return ss.str();
  }

  double toDouble() override {
    if (Type::isNumeric<T>()) {
      double result;
      std::stringstream ss;
      ss << this->get();
      ss >> result;
      return result;
    }

    if (Type::isString<T>()) {
      try {
        return std::stod(this->toString());
      } catch (std::exception e) {
        throw std::invalid_argument("convert failed");
      }
    }

    throw std::invalid_argument("convert failed");
  }

  const T& get() const {
    return val;
  }

  private:
  T val;
};

class var {
  public:
  using Dispatcher = std::shared_ptr<IValue>;

  template <typename T>
  var(T value) {
    this->dispatcher = std::make_shared<Value<T>>(value);
  }

  template <typename T>
  var& operator=(const T& value) {
    this->dispatcher = std::make_shared<Value<T>>(value);
    return *this;
  }

  template <typename T>
  inline T as() const {
    return static_cast<T>(dispatcher->toDouble());
  }

  template <>
  inline std::string as<std::string>() const {
    return dispatcher->toString();
  }

  const std::type_info& type() const {
    return this->dispatcher->type();
  }

  friend var operator+(const var& lhr, const var& rhs);
  friend var operator-(const var& lhr, const var& rhs);
  friend var operator*(const var& lhr, const var& rhs);
  friend var operator/(const var& lhr, const var& rhs);
  friend bool operator==(const var& lhr, const var& rhs);
  friend bool operator!=(const var& lhr, const var& rhs);
  friend std::ostream& operator<<(std::ostream& stream, const var& v);

  private:
  Dispatcher dispatcher;
};

inline std::ostream& operator<<(std::ostream& stream, const var& v) {
  return (stream << v.as<std::string>());
}

static_assert(!SameType<var::Dispatcher, UnknownType>::value, "Not allow `var::Dispatcher` equal `UnknownType`");
#endif