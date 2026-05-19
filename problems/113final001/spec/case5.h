#ifndef _CASE_H_
#define _CASE_H_
#define SECRET "c9e83ab2-6a3e-424a-ac50-49d3872b296e"
#define DOCTEST_CONFIG_IMPLEMENT
#include "Complex.h"
#include "test.h"

TEST_CASE("Case5: Member function and scalar mixed arithmetic") {
  Complex x, y(3), z(-3.2, 2.1);
  x = Complex(3, -4);
  y = Complex(1, -1);

  CHECK_EQ(x.real() , 3);
  CHECK_EQ(x.imag() , -4);
  CHECK_EQ(real(x) , 3);
  CHECK_EQ(imag(x) , -4);
  CHECK_EQ(norm(x) , 5);

  CHECK_EQ((x + y) , Complex(4, -5));
  CHECK_EQ((x * y) , Complex(-1, -7));
  CHECK_EQ((x - y) , Complex(2, -3));
  CHECK_EQ((x / y) , Complex(3.5, -0.5));

  double d = 2.0;
  CHECK_EQ((x + d) , Complex(5, -4));
  CHECK_EQ((x - d) , Complex(1, -4));
  CHECK_EQ((x * d) , Complex(6, -8));
  CHECK_EQ((x / d) , Complex(1.5, -2));
  CHECK_EQ((d + x) , Complex(5, -4));
  CHECK_EQ((d - x) , Complex(-1, 4));
  CHECK_EQ((d * x) , Complex(6, -8));
  CHECK_EQ((d / x).real() , APPROXY(0.24));
  CHECK_EQ((d / x).imag() , APPROXY(0.32));

  Complex two(2, 0);
  CHECK_EQ((two / x).real() , APPROXY(0.24));
  CHECK_EQ((two / x).imag() , APPROXY(0.32));

  std::stringstream ss("x = 10 + 4.5*i\ny = 5 + 6*i");
  ss >> x >> y;
  CHECK_EQ(x , Complex(10, 4.5));
  CHECK_EQ(y , Complex(5, 6));
}
#endif // _CASE_H_