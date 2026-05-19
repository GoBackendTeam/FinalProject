#ifndef _CASE_H_
#define _CASE_H_
#define SECRET "29f2dc71-5287-4978-a977-82641c8791bc"
#define DOCTEST_CONFIG_IMPLEMENT
#include "Complex.h"
#include "test.h"

TEST_CASE("Case3: Equality and member functions") {
  Complex c1(8, -15);
  Complex c2(3, -4);
  Complex c3(8, -15);

  CHECK_EQ(c1 , c3);
  CHECK_EQ(c1.real() , 8);
  CHECK_EQ(c1.imag() , -15);
  CHECK_EQ(c1.norm() , 17);
  CHECK_EQ(real(c1) , 8);
  CHECK_EQ(imag(c1) , -15);
  CHECK_EQ(norm(c1) , 17);
}
#endif // _CASE_H_