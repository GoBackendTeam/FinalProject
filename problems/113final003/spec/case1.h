#ifndef _CASE_H_
#define _CASE_H_
#define SECRET "4b8cf48c-276e-4865-ab66-b553bcfdcbf5"
#define DOCTEST_CONFIG_IMPLEMENT
#include <iostream>
#include <sstream>
#include <string>

#include "School.h"
#include "test.h"


inline std::string trimEnd(std::string s) {
  s.erase(s.find_last_not_of("\t\n\r\f\v ") + 1);
  return s;
}

TEST_CASE("Case1") {
  std::ostringstream oss;
  std::streambuf* cout_buf = std::cout.rdbuf(oss.rdbuf());
  School ntust("NTUST", 10000);
  School ntust2("NTUST2", 20000);

  std::cout << ntust << std::endl;
  std::cout << ntust2 << std::endl;

  ntust.admissions(1000);
  std::cout << ntust << std::endl;

  ntust.dropouts(1000);
  std::cout << ntust << std::endl;

  ntust.transfer(1000, ntust2);
  std::cout << ntust << std::endl;
  std::cout << ntust2 << std::endl;
  std::cout.rdbuf(cout_buf);
  auto output = "NTUST	10000	10000\n"
                "NTUST2	20000	20000\n"
                "NTUST	11000	10000\n"
                "NTUST	10000	10000\n"
                "NTUST	9000	10000\n"
                "NTUST2	21000	20000";
  CHECK_EQ(trimEnd(oss.str()), trimEnd(output));
}

#endif // _CASE_H