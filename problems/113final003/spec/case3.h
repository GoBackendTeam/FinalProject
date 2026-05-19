#ifndef _CASE_H_
#define _CASE_H_
#define SECRET "d2074377-fe58-40c7-a456-bdebc1ae868d"
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

TEST_CASE("Case3") {
  std::ostringstream oss;
  std::streambuf* cout_buf = std::cout.rdbuf(oss.rdbuf());
  PrivateSchool fjcu("FJCU", 10000);

  PublicSchool ntut("NTUT", 10000);

  std::cout << fjcu << std::endl;

  fjcu.dropouts(1);
  std::cout << fjcu << std::endl;

  fjcu.dropouts(99);
  std::cout << fjcu << std::endl;

  fjcu.dropouts(100);
  std::cout << fjcu << std::endl;

  std::cout << ntut << std::endl;

  ntut.dropouts(99);
  std::cout << ntut << std::endl;

  ntut.dropouts(101);
  std::cout << ntut << std::endl;

  ntut.dropouts(100);
  std::cout << ntut << std::endl;
  std::cout.rdbuf(cout_buf);
  auto output = "FJCU	10000	10000\n"
                "FJCU	9999	10000\n"
                "FJCU	9900	9900\n"
                "FJCU	9800	9800\n"
                "NTUT	10000	10000\n"
                "NTUT	9901	10000\n"
                "NTUT	9800	9500\n"
                "NTUT	9700	9500\n";
  CHECK_EQ(trimEnd(oss.str()), trimEnd(output));
}

#endif // _CASE_H