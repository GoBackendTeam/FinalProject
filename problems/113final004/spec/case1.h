#ifndef _CASE_H_
#define _CASE_H_
#define SECRET "a477df5c-26d4-4a19-9a87-bbc01b1724da"
#include "URL.h"
#include "test.h"

TEST_CASE("Case1") {
  {
    // telnet://liquid-hydrocarbon.info:44829/crustulum/cavus/suppono?demergo=constans
    auto url = URL::from(
        "telnet%3A%2F%2Fliquid-hydrocarbon.info%3A44829%"
        "2Fcrustulum%2Fcavus%2Fsuppono%3Fdemergo%3Dconstans");
    CHECK_EQ(url->protocol, "telnet");
    CHECK_EQ(url->hostname, "liquid-hydrocarbon.info");
    CHECK_EQ(url->pathname.size(), 3);
    CHECK_EQ(url->pathname, (URL::Segments{"crustulum", "cavus", "suppono"}));
    CHECK_EQ(url->query.empty(), false);
    CHECK_EQ(url->query["demergo"], URL::Segments{"constans"});
    CHECK_EQ(url->fragment.empty(), true);

    CHECK_EQ(
        url->location(), "telnet://liquid-hydrocarbon.info:44829/crustulum/"
                         "cavus/suppono?demergo=constans");

    url->fragment = "123456";
    CHECK_EQ(
        url->location(), "telnet://liquid-hydrocarbon.info:44829/crustulum/"
                         "cavus/suppono?demergo=constans#123456");
  }
  {
    // svn://tiny-understanding.biz/solitudo
    auto url = URL::from("svn%3A%2F%2Ftiny-understanding.biz%2Fsolitudo");
    CHECK_EQ(url->protocol, "svn");
    CHECK_EQ(url->hostname, "tiny-understanding.biz");
    CHECK_EQ(url->pathname.size(), 1);
    CHECK_EQ(url->pathname, (URL::Segments{"solitudo"}));
    CHECK_EQ(url->query.empty(), true);

    CHECK_EQ(url->fragment.empty(), true);

    CHECK_EQ(url->location(), "svn://tiny-understanding.biz/solitudo");
    url->query["key"] = {"value"};
    CHECK_EQ(url->location(), "svn://tiny-understanding.biz/solitudo?key=value");
  }
  {
    // svn://bartoletti:4idJdR5b@true-handover.com:8717/recusandae?porro=stabilis
    auto url = URL::from(
        "svn%3A%2F%2Fbartoletti%3A4idJdR5b%40true-handover.com%"
        "3A8717%2Frecusandae%3Fporro%3Dstabilis");
    CHECK_EQ(url->protocol, "svn");
    CHECK_EQ(url->hostname, "true-handover.com");
    CHECK_EQ(url->pathname.size(), 1);
    CHECK_EQ(url->pathname, (URL::Segments{"recusandae"}));
    CHECK_EQ(url->query.empty(), false);
    CHECK_EQ(url->query["porro"], (URL::Segments{"stabilis"}));
    CHECK_EQ(url->fragment.empty(), true);

    CHECK_EQ(
        url->location(), "svn://bartoletti:4idJdR5b@true-handover.com:8717/"
                         "recusandae?porro=stabilis");

    url->username = "user";
    CHECK_EQ(
        url->location(), "svn://user:4idJdR5b@true-handover.com:8717/"
                         "recusandae?porro=stabilis");
  }
}

#endif