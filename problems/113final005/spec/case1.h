#ifndef _CASE_H_
#define _CASE_H_
#define SECRET "518271c1-9b85-45ce-9da8-7b02dd302716"
#define DOCTEST_CONFIG_IMPLEMENT
#include <slice.h>
#include <test.h>

#include <algorithm>
#include <deque>
#include <list>
#include <type_traits>
#include <vector>


TEST_CASE("Case1: Check type aliases and forbid undefined types") {
  using vector_iter = std::vector<int>::iterator;
  using list_iter = std::list<int>::iterator;
  using deque_iter = std::deque<int>::iterator;
  using Slice_iter = decltype(std::declval<Slice<int>>().begin());

  bool StorageIsUnknownType = std::is_same<Slice<int>::Storage_t, UnknownType>::value;
  CHECK_EQ(StorageIsUnknownType, false);

  auto PredicateIsUnknownType = std::is_same<Slice<int>::Predicate, UnknownType>::value;
  CHECK_EQ(PredicateIsUnknownType, false);

  auto isVectorIter = std::is_same<vector_iter, Slice_iter>::value;
  CHECK_EQ(isVectorIter, false);

  auto isDequeIter = std::is_same<deque_iter, Slice_iter>::value;
  CHECK_EQ(isDequeIter, false);

  auto isListIter = std::is_same<list_iter, Slice_iter>::value;
  CHECK_EQ(isListIter, false);
}

#endif
