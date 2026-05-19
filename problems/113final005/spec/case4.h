#ifndef _CASE_H_
#define _CASE_H_
#define SECRET "445d090f-b999-45c8-b37f-b5892f6c74c0"
#define DOCTEST_CONFIG_IMPLEMENT
#include <slice.h>
#include <test.h>

#include <algorithm>
#include <deque>
#include <list>
#include <type_traits>
#include <vector>

TEST_CASE("Case4: STL algorithm compatibility") {
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
  bool all_even = std::all_of(points.begin(), points.end(), [](const Point& p) {
    return p.y % 2 == 0;
  });
  CHECK_EQ(all_even, true);

  // 3. std::any_of: is there any x > 8?
  bool any_large = std::any_of(points.begin(), points.end(), [](const Point& p) {
    return p.x > 8;
  });
  CHECK_EQ(any_large, true);

  // 4. std::count_if: how many x are odd?
  int odd_count = std::count_if(points.begin(), points.end(), [](const Point& p) {
    return p.x % 2 == 1;
  });
  CHECK_EQ(odd_count, 5);

  // 5. std::for_each: accumulate all y into a sum
  int total_y = 0;
  std::for_each(points.begin(), points.end(), [&](const Point& p) {
    total_y += p.y;
  });
  CHECK_EQ(total_y, 90);

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
