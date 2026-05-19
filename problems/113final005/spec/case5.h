#ifndef _CASE_H_
#define _CASE_H_
#define SECRET "82560d0c-a1fc-4565-bc13-1661e3486965"
#define DOCTEST_CONFIG_IMPLEMENT
#include <slice.h>
#include <test.h>

#include <algorithm>
#include <deque>
#include <iostream>
#include <list>
#include <type_traits>
#include <vector>


struct Vec2 {
  int x;
  int y;
};

struct Vec3 {
  int x;
  int y;
  int z;

  inline bool operator==(const Vec3& other) const {
    return x == other.x && y == other.y && z == other.z;
  }
};

inline std::ostream& operator<<(std::ostream& os, const Vec3& v) {
  return os << "Vec3(" << v.x << "," << v.y << "," << v.z << ")";
}

TEST_CASE("Case5: Complex testing") {
  Slice<Vec2> v2({
      {1, 2},
      {2, 2},
      {3, 3},
      {4, 1},
      {0, 6}
  });

  // map: Vec2 → Vec3, where z = x + y
  auto v3 = v2.map(+[](Vec2 v) {
    return Vec3{v.x, v.y, v.x + v.y};
  });

  CHECK_EQ(v3.size(), 5);
  Vec3 v3_0 = {1, 2, 3};
  CHECK_EQ(v3[0], v3_0);
  Vec3 v3_4 = {0, 6, 6};
  CHECK_EQ(v3[4], v3_4);

  // filter: only keep Vec3 where z > 5
  auto filtered = v3.filter(+[](Vec3 v) {
    return v.z > 5;
  });

  CHECK_EQ(filtered.size(), 2);
  CHECK_EQ(filtered[0].z, 6);
  CHECK_EQ(filtered[1].z, 6);

  // every: check that for all filtered elements, x + y == z
  bool valid = filtered.every(+[](Vec3 v) {
    return (v.x + v.y) == v.z;
  });
  CHECK_EQ(valid, true);
}

#endif
