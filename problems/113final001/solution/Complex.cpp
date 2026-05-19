#include "Complex.h"
#include <cmath>
#include <cstdio>
#include <string>

Complex::Complex() {
  this->imaginaryValue = 0;
  this->realValue = 0;
}

Complex::Complex(double r) {
  this->realValue = r;
  this->imaginaryValue = 0;
}

Complex::Complex(double r, double i) {
  this->realValue = r;
  this->imaginaryValue = i;
}

double Complex::real() {
  return this->realValue;
}

double Complex::imag() {
  return this->imaginaryValue;
}

double Complex::norm() {
  return sqrt(this->realValue * this->realValue + this->imaginaryValue * this->imaginaryValue);
}

double real(Complex c) {
  return c.realValue;
}

double imag(Complex c) {
  return c.imaginaryValue;
}

double norm(Complex c) {
  return sqrt(c.realValue * c.realValue + c.imaginaryValue * c.imaginaryValue);
}

Complex Complex::operator+(Complex c) {
  return Complex(this->realValue + c.realValue, this->imaginaryValue + c.imaginaryValue);
}

Complex Complex::operator-(Complex c) {
  return Complex(this->realValue - c.realValue, this->imaginaryValue - c.imaginaryValue);
}

Complex Complex::operator*(Complex c) {
  double temp_real;
  double temp_imag;

  temp_real = (this->realValue * c.realValue) - (this->imaginaryValue * c.imaginaryValue);
  temp_imag = (this->realValue * c.imaginaryValue) + (this->imaginaryValue * c.realValue);

  return Complex(temp_real, temp_imag);
}

Complex Complex::operator/(Complex c) {
  double den = c.realValue * c.realValue + c.imaginaryValue * c.imaginaryValue;

  Complex temp(c.realValue, -c.imaginaryValue);

  Complex nume = (*this) * temp;

  temp.realValue = nume.realValue / den;
  temp.imaginaryValue = nume.imaginaryValue / den;

  return temp;
}

Complex operator+(double d, Complex c) {
  return Complex(d + c.realValue, c.imaginaryValue);
}

Complex operator-(double d, Complex c) {
  return Complex(d - c.realValue, -c.imaginaryValue);
}

Complex operator*(double d, Complex c) {
  return Complex(d * c.realValue, d * c.imaginaryValue);
}

Complex operator/(double d, Complex c) {
  Complex temp(d);
  return temp / c;
}

Complex operator+(Complex c, double d) {
  return Complex(d + c.realValue, c.imaginaryValue);
}

Complex operator-(Complex c, double d) {
  return Complex(c.realValue - d, c.imaginaryValue);
}

Complex operator*(Complex c, double d) {
  return Complex(d * c.realValue, d * c.imaginaryValue);
}

Complex operator/(Complex c, double d) {
  return Complex(c.realValue / d, c.imaginaryValue / d);
}

bool operator==(Complex c1, Complex c2) {
  if (c1.realValue == c2.realValue && c1.imaginaryValue == c2.imaginaryValue) {
    return true;
  } else {
    return false;
  }
}

bool operator!=(Complex c1, Complex c2) {
  return !(c1 == c2);
}

std::ostream& operator<<(std::ostream& strm, const Complex& c) {
  strm << c.realValue << " + " << c.imaginaryValue << "*i";
  return strm;
}

std::istream& operator>>(std::istream& strm, Complex& c) {
  std::string inp;
  if (std::getline(strm, inp)) {
#ifdef _MSC_VER
    sscanf_s(inp.c_str(), "%*c = %lf + %lf*i", &(c.realValue), &(c.imaginaryValue));
#else
    sscanf(inp.c_str(), "%*c = %lf + %lf*i", &(c.realValue), &(c.imaginaryValue));
#endif
  }
  return strm;
}
