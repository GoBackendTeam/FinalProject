#include <cstdint>
#include <iostream>
#include <memory>
#include <typeinfo>

/**
 * @headerfile <typeinfo>
 * @brief Demonstrates usage of `std::type_info` and `typeid` operator.
 * Compares type identities and explains the difference between static and
 * dynamic types.
 * @example
  - typeid(var).name() shows the type name.
  - typeid(*ptr) gives dynamic type if Base has virtual functions.
  - typeid(T1) == typeid(T2) checks type equality at "runtime".
 */
inline void typeinfoExample() {
  struct Base {
    virtual ~Base() = default;
  };

  struct Derive : public Base {};

  // Type comparisons
  std::int8_t charAlias = 0;
  std::int16_t shortAlias = 0;
  std::int32_t intAlias = 0;
  std::int64_t longAlias = 0;

  if (typeid(charAlias) == typeid(char)) {
    std::cout << "`std::int8_t` equals `char`\n";
  } else {
    std::cout << "`std::int8_t` is NOT equal to `char`\n";
  }

  // Dynamic type inspection
  Derive derived;
  Base* basePtr = &derived;
  if (typeid(*basePtr) == typeid(Derive)) {
    std::cout << "Dynamic type of *basePtr is Derive.\n";
  } else {
    std::cout << "Dynamic type of *basePtr is not Derive.\n";
  }

  // Store Typeinfo
  const std::type_info& type_info = typeid(int);
  std::cout << "type_info equals `int` type: " << (type_info == (typeid(int)) ? "True" : "False");
}
/**
 * @brief Program Output
 * `std::int8_t` is NOT equal to `char`
 * Dynamic type of *basePtr is Derive.
 * type_info is `int` type: True
 */

/**
 * @headerfile <memory>
 * @brief Demonstrates usage of `std::shared_ptr` and `std::make_shared`.
 *
 * Shows how to allocate and manage heap-allocated memory safely without using
 * `new` or `delete`.
 *
 * `std::shared_ptr` is a smart pointer that owns and automatically deletes the
 * managed object when it goes out of scope. Ownership is exclusive.
 *
 * `std::make_shared<T>(...)` is the preferred way to construct `shared_ptr`
 * safely.
 * @example
 * - Automatically deletes the managed object when it goes out of scope.
 * - Provides pointer-like syntax (`*ptr`, `ptr->member`) to access the underlying object.
 * - Can be used with custom deleters for advanced resource management.
 * - `std::make_shared<T>` eliminates the need for `new`, improves exception safety.
 */
inline void smartPtrExample() {
  // Traditional manual allocation (requires manual delete)
  int* rawPtr = new int(42);
  std::cout << "rawPtr value: " << *rawPtr << "\n";
  delete rawPtr; // Must remember to free manually

  // Safer heap allocation using std::make_shared
  // Equivalent to: int* ptr = new int(42);
  auto ptr = std::make_shared<int>(42);
  std::cout << "shared_ptr value: " << *ptr << "\n";

  // They are same.
  int* rawPointerArray = new int[5];
  auto smartPointerArray = std::make_unique<int[]>(5);

  /* T[] should use `unique_ptr`, NOT ALLOW use `shared_ptr` */
  // auto smartPointerArray = std::make_shared<int[]>(5);

  /**
   * @brief Compare smart-pointer and raw-pointer.
   * 
   * Raw pointer:
   *   T* var = new T(value);
   *
   * Smart pointer (preferred):
   *   auto var = std::make_shared<T>(value);
   *
   * Where `T` is the type, such as `int`, `char`, etc.
   *
   * `std::make_shared` is safer and exception-safe, and automatically
   * releases the memory when the pointer goes out of scope.
   */
}
