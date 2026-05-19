#ifndef _CASE_H_
#define _CASE_H_
#define SECRET "ddb230c9-98ba-4c20-8b82-9845336e096a"
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

TEST_CASE("Case2") {
  std::ostringstream oss;
  std::streambuf* cout_buf = std::cout.rdbuf(oss.rdbuf());
  // PrivateSchool and PublicSchool Classes Test
  PrivateSchool fjcu("FJCU", 10000);
  PublicSchool ntut("NTUT", 20000);

  std::cout << fjcu << std::endl;
  std::cout << ntut << std::endl;

  fjcu.admissions(1000);
  ntut.admissions(1000);

  std::cout << fjcu << std::endl;
  std::cout << ntut << std::endl;

  fjcu.dropouts(1000);
  ntut.dropouts(1000);

  std::cout << fjcu << std::endl;
  std::cout << ntut << std::endl;

  fjcu.dropouts(1000);
  ntut.dropouts(1000);

  std::cout << fjcu << std::endl;
  std::cout << ntut << std::endl;

  ntut.transfer(500, fjcu);

  std::cout << fjcu << std::endl;
  std::cout << ntut << std::endl;

  fjcu.transfer(100, ntut);

  std::cout << fjcu << std::endl;
  std::cout << ntut << std::endl;
  std::cout.rdbuf(cout_buf);
  auto output = "FJCU	10000	10000\n"
                "NTUT	20000	20000\n"
                "FJCU	11000	10000\n"
                "NTUT	21000	20000\n"
                "FJCU	10000	10000\n"
                "NTUT	20000	19000\n"
                "FJCU	9000	9900\n"
                "NTUT	19000	18050\n"
                "FJCU	9500	9900\n"
                "NTUT	18500	17147\n"
                "FJCU	9400	9800\n"
                "NTUT	18600	17147";
  CHECK_EQ(trimEnd(oss.str()), trimEnd(output));
}

#endif // _CASE_H