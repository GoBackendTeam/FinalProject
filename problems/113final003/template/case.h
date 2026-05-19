#ifndef _CASE_H_
#define _CASE_H_
#define DOCTEST_CONFIG_IMPLEMENT
#include "School.h"
#include "test.h"
#include <iostream>
#include <sstream>
#include <string>

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

TEST_CASE("Case2") {
  std::ostringstream oss;
  std::streambuf* cout_buf = std::cout.rdbuf(oss.rdbuf());
  PublicSchool test("TEST", 2222);
  std::cout << test << std::endl;

  test.admissions(2879);
  std::cout << test << std::endl;

  test.dropouts(101);
  std::cout << test << std::endl;

  PublicSchool test2("TEST2", 2879);
  std::cout << test2 << std::endl;

  test2.admissions(2222);
  std::cout << test2 << std::endl;

  test2.dropouts(101);
  std::cout << test2 << std::endl;

  PrivateSchool test3("TEST3", 1000);
  std::cout << test3 << std::endl;

  test3.admissions(0);
  std::cout << test3 << std::endl;

  test3.dropouts(-10000);
  std::cout << test3 << std::endl;

  test3.admissions(9000);
  std::cout << test3 << std::endl;

  test3.dropouts(100000);
  std::cout << test3 << std::endl;

  test3.transfer(0.5, test);
  std::cout << test3 << std::endl;
  std::cout << test << std::endl;

  test3.transfer(1, test);
  std::cout << test3 << std::endl;
  std::cout << test << std::endl;

  test3.transfer(0, test);
  std::cout << test3 << std::endl;
  std::cout << test << std::endl;

  test.transfer(100000, test3);
  std::cout << test3 << std::endl;
  std::cout << test << std::endl;
  std::cout.rdbuf(cout_buf);

  auto output = "TEST	2222	2222\n"
                "TEST	5101	2222\n"
                "TEST	5000	2110\n"
                "TEST2	2879	2879\n"
                "TEST2	5101	2879\n"
                "TEST2	5000	2735\n"
                "TEST3	1000	1000\n"
                "Invalid amount\n"
                "TEST3	1000	1000\n"
                "Invalid amount\n"
                "TEST3	1000	1000\n"
                "TEST3	10000	1000\n"
                "Invalid student count\n"
                "TEST3	10000	1000\n"
                "Invalid amount\n"
                "TEST3	10000	1000\n"
                "TEST	5000	2110\n"
                "TEST3	9999	1000\n"
                "TEST	5001	2110\n"
                "Invalid amount\n"
                "TEST3	9999	1000\n"
                "TEST	5001	2110\n"
                "Invalid student count\n"
                "TEST3	9999	1000\n"
                "TEST	5001	2110";
  CHECK_EQ(trimEnd(oss.str()), trimEnd(output));
}

#endif
