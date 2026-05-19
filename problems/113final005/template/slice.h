#ifndef SLICE_H
#define SLICE_H
#include "iterator.h"

class UnknownType {};

/**
 * @brief A generic container that supports basic functional operations.
 *
 * @tparam T The element type stored in the container.
 *
 * Students are required to complete this class by:
 * - Defining the appropriate type for `Predicate`
 * - Defining the appropriate container type for `Storage`
 * - Implementing all member functions marked with empty bodies
 * - Providing a custom iterator that supports range-based for loops
 * - Supporting basic operations such as `map`, `filter`, `every`
 *
 * Additional helper declarations (constructors, methods, classes, etc.) may be introduced as needed.
 */
template <typename T>
class Slice {
  public:
  /**
   * @brief Represents a predicate function used for filtering or checking elements.
   * Please deduce the appropriate type.
   */
  using Predicate = UnknownType;

  /**
   * @brief Represents the underlying container type.
   * Please define a container type (e.g., std::vector, std::array, T[] or T*) to store internal data.
   */
  template <typename U = void>
  using Storage = UnknownType;

  using Storage_t = Storage<T>;

  // iterator
  Iterator<T> begin() {}
  Iterator<T> end() {}
  T& operator[](size_t index) {}

  /**
   * @brief Construct a Slice from a list of values.
   * Elements must be stored internally in `_data`.
   *
   * @param init An initializer list of elements.
   */
  Slice(std::initializer_list<T> init) {
    // store data into a _data
  }

  /**
   * @brief Apply a transformation function to each element.
   *
   * The return type should be deduced from the result of the functor.
   *
   * @param fn A callable object that transforms a Slice<T> into Slice<U>.
   * @return A new container of transformed elements.
   */
  template <typename Functor>
  auto map(Functor fn) {}

  /**
   * @brief Return a new Slice containing only elements that satisfy the predicate.
   *
   * @param fn A predicate that returns true for elements to keep.
   * @return A new Slice<T> containing filtered elements.
   */
  Slice<T> filter(Predicate fn) {}

  /**
   * @brief Check if all elements satisfy the given predicate.
   *
   * @param fn A predicate to test each element.
   * @return true if all elements satisfy the predicate; false otherwise.
   */
  bool every(Predicate fn) {}

  private:
  /**
   * @brief Internal data container.
   * The container type must be defined via the `Storage` alias.
   */
  Storage_t _data;
};

/**
 * @brief Check
 * Must declare the Storage type, Predicate type and implement iterators
 */
static_assert(!std::is_same<Slice<void*>::Predicate, UnknownType>::value, "Not allow `Slice<T>::Predicate` equal `UnknownType`");
static_assert(!std::is_same<Slice<void*>::Storage_t, UnknownType>::value, "Not allow `Slice<T>::Storage` equal `UnknownType`");
static Slice<int> vaild({1, 2, 3, 4, 5, 6, 7, 8});
static_assert(std::is_same<decltype(vaild.begin()), Iterator<int>>::value, "Should implement iterator");
#endif