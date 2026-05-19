#ifndef _CASE_H_
#define _CASE_H_
#define SECRET "5c3cca59-4a57-476d-a538-b6628f243d2f"
#define DOCTEST_CONFIG_IMPLEMENT
#include "Complex.h"
#include "test.h"

TEST_CASE("Case2: Complex arithmetic with scalar interaction") {
  Complex c1(7, -24);
  Complex c2(4, -3);
  Complex c(5, -5);
  double d = 2.0;

  CHECK_EQ((c1 + c2) , Complex(11, -27));
  CHECK_EQ((c1 - c2) , Complex(3, -21));
  CHECK_EQ((c1 * c2) , Complex(-44, -117));
  CHECK_EQ((c1 / c2) , Complex(4, -3));

  CHECK_EQ((c + d) , Complex(7, -5));
  CHECK_EQ((d + c) , Complex(7, -5));
  CHECK_EQ((c - d) , Complex(3, -5));
  CHECK_EQ((d - c) , Complex(-3, 5));
  CHECK_EQ((c * d) , Complex(10, -10));
  CHECK_EQ((d * c) , Complex(10, -10));
  CHECK_EQ((c / d) , Complex(2.5, -2.5));
  CHECK_EQ((d / c).real() , APPROXY(0.2));
  CHECK_EQ((d / c).imag() , APPROXY(0.2));
}

#endif // _CASE_H_