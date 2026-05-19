#ifndef _CASE_H_
#define _CASE_H_
#define SECRET "4675357a-25d8-4dcc-b18b-ff5da54b8f62"
#define DOCTEST_CONFIG_IMPLEMENT
#include <slice.h>
#include <test.h>

#include <algorithm>
#include <deque>
#include <list>
#include <type_traits>
#include <vector>

TEST_CASE("Case3: suitable Based-range For-loop") {
  Slice<int> s({1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15});
  int sum = 0, count = 1;
  for (auto v : s) {
    CHECK_EQ(count, v);
    sum += v;
    ++count;
  }
  CHECK_EQ(sum, 120);
}

#endif
