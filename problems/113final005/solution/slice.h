#ifndef SLICE_H
#define SLICE_H
#include <iterator.h>
#include <functional>
#include <vector>

class UnknownType {};

template <typename T>
class Slice {
  public:
  using Predicate = std::function<bool(T)>;

  template <typename U>
  using Storage = std::vector<U>;

  using Storage_t = Storage<T>;

  explicit Slice() {}

  Slice(Storage<T> data)
      : _data(data) {}

  template <typename Iterable>
  Slice(const Iterable& container)
      : _data(std::begin(container), std::end(container)) {}

  size_t size() const {
    return _data.size();
  }

  // iterators
  Iterator<T> begin() {
    return Iterator<T>(&_data[0]);
  }

  Iterator<T> end() {
    return Iterator<T>(&_data[0] + _data.size());
  }

  T& operator[](size_t index) {
    return _data[index];
  }

  template <typename U>
  Slice<U> map(U (*callback)(T arg)) const {
    Storage<U> result = {};
    for (auto& item : this->_data) {
      result.push_back(callback(item));
    }
    return Slice<U>(result);
  }

  Slice<T> filter(Predicate func) const {
    Storage<T> result = {};
    for (auto& item : _data) {
      if (func(item)) {
        result.push_back(item);
      }
    }
    return Slice<T>(result);
  }

  bool every(Predicate func) const {
    for (auto& item : _data) {
      if (!func(item)) {
        return false;
      }
    }
    return true;
  }

  private:
  Storage<T> _data;
};

#endif