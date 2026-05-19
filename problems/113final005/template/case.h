#ifndef _CASE_H_
#define _CASE_H_
#include "slice.h"
#include "test.h"
#include <algorithm>
#include <deque>
#include <functional>
#include <list>
#include <type_traits>
#include <utility>
#include <vector>

inline bool isEven(int x) {
  return x % 2 == 0;
}

TEST_CASE("Case: Basic Use") {
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

TEST_CASE("Case: suitable Range-based For-loop") {
  Slice<int> s({1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15});
  int sum = 0, count = 1;
  for (auto v : s) {
    CHECK_EQ(count, v);
    sum += v;
    ++count;
  }
  CHECK_EQ(sum, 120);
}

TEST_CASE("Case: STL algorithm compatibility") {
  typedef struct Point {
    int x;
    int y;
  } Point;

  Slice<Point> points({
      {0,  0},
      {1,  2},
      {2,  4},
      {3,  6},
      {4,  8},
      {5, 10},
      {6, 12},
      {7, 14},
      {8, 16},
      {9, 18}
  });

  // 1. std::find_if: find point with x == 3
  auto it = std::find_if(points.begin(), points.end(), [](const Point& p) {
    return p.x == 3;
  });
  CHECK_EQ(it != points.end(), true);
  CHECK_EQ(it->y, 6);

  // 2. std::all_of: all y are even?
  bool allEven = std::all_of(points.begin(), points.end(), [](const Point& p) {
    return p.y % 2 == 0;
  });
  CHECK_EQ(allEven, true);

  // 3. std::any_of: is there any x > 8?
  bool anyLarge = std::any_of(points.begin(), points.end(), [](const Point& p) {
    return p.x > 8;
  });
  CHECK_EQ(anyLarge, true);

  // 4. std::count_if: how many x are odd?
  int oddCount = std::count_if(points.begin(), points.end(), [](const Point& p) {
    return p.x % 2 == 1;
  });
  CHECK_EQ(oddCount, 5);

  // 5. std::for_each: accumulate all y into a sum
  int totalY = 0;
  std::for_each(points.begin(), points.end(), [&](const Point& p) {
    totalY += p.y;
  });
  CHECK_EQ(totalY, 90);

  // 6. std::transform: create a new vector<int> containing only x
  std::vector<int> xs;
  std::transform(points.begin(), points.end(), std::back_inserter(xs), [](const Point& p) {
    return p.x;
  });
  CHECK_EQ(xs.size(), 10);
  CHECK_EQ(xs[0], 0);
  CHECK_EQ(xs[9], 9);
}
#endif
