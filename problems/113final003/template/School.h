#ifndef SCHOOL_H_
#define SCHOOL_H_

#include <cmath>
#include <iostream>
#include <string>

class School {
  protected:
  std::string name;
  int studentAmount;
  int studentAmountNextYear;

  public:
  School() {}

  virtual ~School() {}

  virtual void admissions(float amount) {}

  virtual void dropouts(float amount) {}

  virtual void transfer(float amount, School& toSchool) {}

  friend std::ostream& operator<<(std::ostream& os, const School& s) {}
};

class PrivateSchool : public School {
  private:
  int dropoutCount;

  public:
  PrivateSchool() {}

  void dropouts(float amount) override {}
};

class PublicSchool : public School {
  private:
  double growing_rate;

  public:
  PublicSchool() {}

  void apply_growth() {}

  void dropouts(float amount) override {}
};

#endif // SCHOOL_H_
