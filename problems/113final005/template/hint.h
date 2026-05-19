#include <algorithm>
#include <iostream>

/**
 * @brief iterator example
 *
 * This class provides iterators to traverse a sequence of long integers
 * from a specified starting value (FROM) to an ending value (TO), inclusive.
 * The iteration direction depends on whether TO is greater than or equal to FROM.
 *
 * @tparam FROM The starting value of the range (inclusive).
 * @tparam TO The ending value of the range (inclusive).
 */
template <long FROM, long TO>
struct Range {
  class iterator {
public:
    using iterator_category = std::input_iterator_tag;
    using value_type = long;
    using difference_type = long;
    using pointer = const long*;
    using reference = long;
    explicit iterator(long _num = 0)
        : num(_num) {}

    iterator& operator++() {
      num = TO >= FROM ? num + 1 : num - 1;
      return *this;
    }

    iterator operator++(int) {
      iterator retval = *this;
      ++(*this);
      return retval;
    }

    bool operator==(iterator other) const {
      return num == other.num;
    }

    bool operator!=(iterator other) const {
      return !(*this == other);
    }

    reference operator*() const {
      return num;
    }

private:
    long num = FROM;
  };

  iterator begin() {
    return iterator(FROM);
  }
  iterator end() {
    return iterator(TO >= FROM ? TO + 1 : TO - 1);
  }
};

inline void iterator_example() {
  // std::find requires an input iterator
  auto range = Range<15, 25>();
  auto itr = std::find(range.begin(), range.end(), 18);
  std::cout << *itr << '\n'; // 18

  // Range::iterator also satisfies range-based for requirements
  for (long l : Range<3, 5>())
    std::cout << l << ' '; // 3 4 5
  std::cout << '\n';
}