#ifndef ITERATOR_H
#define ITERATOR_H

#include <iterator>

template <typename T>
class Iterator {
  public:
  // traits
  using iterator_category = std::random_access_iterator_tag;
  using value_type = T;
  using difference_type = std::ptrdiff_t;
  using pointer = T*;
  using reference = T&;

  private:
  T* ptr;

  public:
  // constructor
  Iterator(T* p = nullptr)
      : ptr(p) {}

  // dereference
  reference operator*() const {
    return *ptr;
  }
  pointer operator->() const {
    return ptr;
  }

  // Increment/Decrement Operators
  Iterator& operator++() {
    ++ptr;
    return *this;
  }

  Iterator operator++(int) {
    Iterator tmp = *this;
    ++ptr;
    return tmp;
  }

  Iterator& operator--() {
    --ptr;
    return *this;
  }

  Iterator operator--(int) {
    Iterator tmp = *this;
    --ptr;
    return tmp;
  }

  Iterator operator+(difference_type n) const {
    return Iterator(ptr + n);
  }

  friend Iterator operator+(difference_type n, const Iterator& it) {
    return it + n;
  }

  Iterator& operator+=(difference_type n) {
    ptr += n;
    return *this;
  }

  Iterator operator-(difference_type n) const {
    return Iterator(ptr - n);
  }

  friend Iterator operator-(difference_type n, const Iterator& it) {
    return it - n;
  }

  Iterator& operator-=(difference_type n) {
    ptr -= n;
    return *this;
  }

  difference_type operator-(const Iterator& other) const {
    return ptr - other.ptr;
  }

  // Random access
  reference operator[](difference_type n) const {
    return ptr[n];
  }

  // Comparators
  // clang-format off
  bool operator==(const Iterator& other) const { return ptr == other.ptr; }
  bool operator!=(const Iterator& other) const { return ptr != other.ptr; }
  bool operator<(const Iterator& other) const { return ptr < other.ptr; }
  bool operator>(const Iterator& other) const { return ptr > other.ptr; }
  bool operator<=(const Iterator& other) const { return ptr <= other.ptr; }
  bool operator>=(const Iterator& other) const { return ptr >= other.ptr; }
};

#endif