#ifndef _CASE_H_
#define _CASE_H_
#define SECRET "61c0755f-914f-4745-b068-17a4525eae35"
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

TEST_CASE("Case4") {
  std::ostringstream oss;
  std::streambuf* cout_buf = std::cout.rdbuf(oss.rdbuf());
  School ntust("NTUST", 12500);
  PublicSchool ntut("NTUT", 85000);
  PrivateSchool fjcu("FJCU", 25000);

  std::cout << ntust << std::endl;
  std::cout << ntut << std::endl;
  std::cout << fjcu << std::endl;

  ntust.admissions(200);
  std::cout << ntust << std::endl;

  ntust.dropouts(200);
  std::cout << ntust << std::endl;

  ntust.dropouts(100000);
  std::cout << ntust << std::endl;

  fjcu.admissions(1000);
  std::cout << fjcu << std::endl;

  fjcu.admissions(-1000);
  std::cout << fjcu << std::endl;

  fjcu.dropouts(50);
  std::cout << fjcu << std::endl;

  fjcu.dropouts(1000);
  std::cout << fjcu << std::endl;

  ntut.admissions(1000);
  std::cout << ntut << std::endl;

  ntut.apply_growth();
  std::cout << ntut << std::endl;

  ntut.dropouts(1000);
  std::cout << ntut << std::endl;

  std::cout << ntut << std::endl;
  ntut.transfer(1000, ntust);
  std::cout << ntut << std::endl;
  std::cout << ntust << std::endl;

  fjcu.transfer(30000, ntust);
  std::cout << fjcu << std::endl;
  std::cout << ntust << std::endl;
  std::cout.rdbuf(cout_buf);

  auto output = "NTUST	12500	12500\n"
                "NTUT	85000	85000\n"
                "FJCU	25000	25000\n"
                "NTUST	12700	12500\n"
                "NTUST	12500	12500\n"
                "Invalid student count\n"
                "NTUST	12500	12500\n"
                "FJCU	26000	25000\n"
                "Invalid amount\n"
                "FJCU	26000	25000\n"
                "FJCU	25950	25000\n"
                "FJCU	24950	24900\n"
                "NTUT	86000	85000\n"
                "NTUT	86000	89250\n"
                "NTUT	85000	84787\n"
                "NTUT	85000	84787\n"
                "NTUT	84000	80547\n"
                "NTUST	13500	12500\n"
                "Invalid student count\n"
                "FJCU	24950	24900\n"
                "NTUST	13500	12500";
  CHECK_EQ(trimEnd(oss.str()), trimEnd(output));
}

#endif // _CASE_H