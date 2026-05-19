#ifndef _CASE_H_
#define _CASE_H_
#define SECRET "645a0fd5-1a97-4ebc-bd93-06518c5be2b2"
#define DOCTEST_CONFIG_IMPLEMENT
#include <slice.h>
#include <test.h>

#include <algorithm>
#include <deque>
#include <iostream>
#include <list>
#include <type_traits>
#include <vector>

inline bool isEven(int x) {
  return x % 2 == 0;
}

TEST_CASE("Case2: Basic Use") {
  Slice<int> original({1, 2, 3, 4, 5, 6});

  // map
  auto squared = original.map(+[](int x) { // [1, 4, 9, 16, 25, 36]
    return x * x;
  });
  CHECK_EQ(squared.size(), 6);
  CHECK_EQ(squared[0], 1);
  CHECK_EQ(squared[5], 36);

  // filter
  std::function<bool(int)> ge10 = [](int x) {
    return x > 10;
  };

  auto filtered = squared.filter(ge10); // [16, 25, 36]
  CHECK_EQ(filtered.size(), 3);
  CHECK_EQ(filtered[0], 16);
  CHECK_EQ(filtered[1], 25);
  CHECK_EQ(filtered[2], 36);

  // every: check if all remaining are even
  bool allIsEven = filtered.every(isEven);
  CHECK_EQ(allIsEven, false); // 25 is odd

  CHECK_EQ(
      original
          .map(+[](int x) {
            return x * x;
          })
          .filter(+[](int x) {
            return x > 10;
          })
          .every(+[](int x) {
            return x > 5;
          }),
      true);
}

#endif
