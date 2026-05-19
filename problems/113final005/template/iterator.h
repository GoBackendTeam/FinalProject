#ifndef ITERATOR_H
#define ITERATOR_H

#include <iterator>

/**
 * @brief A basic random-access iterator for the Slice container.
 *
 * This iterator wraps a raw pointer and provides full random-access functionality.
 * You are required to complete the missing operator definitions to make it conform to
 * the requirements of a C++ random access iterator.
 *
 * The file `hint.h` includes a minimal example of iterator implementation
 * based on cppreference's iterator concept. You may refer to it for guidance.
 */
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

  /**
   * Please complement `Dereference` and `Arrow` Operator
   */
  reference operator*() const {}

  pointer operator->() const {}

  // Increment Operators
  Iterator& operator++() {
    ++ptr;
    return *this;
  }

  Iterator operator++(int) {
    Iterator tmp = *this;
    ++ptr;
    return tmp;
  }

  Iterator operator+(difference_type n) const {
    return Iterator(ptr + n);
  }

  Iterator& operator+=(difference_type n) {
    ptr += n;
    return *this;
  }

  friend Iterator operator+(difference_type n, const Iterator& it) {
    return it + n;
  }

  /**
   * @breif Decrement Operators
   * Please complete this implementation
   */
  Iterator& operator--() {}

  Iterator operator--(int) {}

  Iterator operator-(difference_type n) const {}

  friend Iterator operator-(difference_type n, const Iterator& it) {}

  Iterator& operator-=(difference_type n) {}

  difference_type operator-(const Iterator& other) const {
    return ptr - other.ptr;
  }

  /**
   * @breif Random access
   * Please complete this implementation
   */
  reference operator[](difference_type n) const {}

  /**
   * @breif Comparators
   * Please complete those implementation
   */
  bool operator==(const Iterator& other) const {}
  bool operator!=(const Iterator& other) const {}
  bool operator<(const Iterator& other) const {}
  bool operator>(const Iterator& other) const {}
  bool operator<=(const Iterator& other) const {}
  bool operator>=(const Iterator& other) const {}
};

#endif