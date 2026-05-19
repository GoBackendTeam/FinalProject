#ifndef _CASE_H_
#define _CASE_H_
#define DOCTEST_CONFIG_IMPLEMENT
#define SECRET "477d8c4a-8df9-4919-b35e-13033aa3696d"
#include "URL.h"
#include "test.h"

TEST_CASE("Case2") {

  {
    // ftp://similar-bookend.org/totus/caecus/tepidus#UL
    auto url = URL::from("ftp%3A%2F%2Fsimilar-bookend.org%2Ftotus%2Fcaecus%2Ftepidus%23UL");
    CHECK_EQ(url->protocol, "ftp");
    CHECK_EQ(url->hostname, "similar-bookend.org");
    CHECK_EQ(url->pathname.size(), 3);
    CHECK_EQ(url->pathname, (URL::Segments{"totus", "caecus", "tepidus"}));
    CHECK_EQ(url->query.empty(), true);

    CHECK_EQ(url->fragment.empty(), false);
    CHECK_EQ(url->fragment, "UL");
    CHECK_EQ(url->location(), "ftp://similar-bookend.org/totus/caecus/tepidus#UL");

    url->protocol = "file";
    CHECK_EQ(url->location(), "file://similar-bookend.org/totus/caecus/tepidus#UL");
  }
  {
    // ssh://wry-recovery.com/nihil/callide
    auto url = URL::from("ssh%3A%2F%2Fwry-recovery.com%2Fnihil%2Fcallide");
    CHECK_EQ(url->protocol, "ssh");
    CHECK_EQ(url->hostname, "wry-recovery.com");
    CHECK_EQ(url->pathname.size(), 2);
    CHECK_EQ(url->pathname, (URL::Segments{"nihil", "callide"}));
    CHECK_EQ(url->query.empty(), true);

    CHECK_EQ(url->fragment.empty(), true);
    CHECK_EQ(url->fragment, "");
    CHECK_EQ(url->location(), "ssh://wry-recovery.com/nihil/callide");

    url->hostname = "yhchen.org";
    CHECK_EQ(url->location(), "ssh://yhchen.org/nihil/callide");
  }
  {
    // ftp://lumpy-omelet.name:38382/possimus?agnitio=socius&deduco=aveho#@/'Ry4@ffrTwN
    auto url = URL::from(
        "ftp%3A%2F%2Flumpy-omelet.name%3A38382%2Fpossimus%3Fagnitio%"
        "3Dsocius%26deduco%3Daveho%23%40%2F'Ry4%40ffrTwN");
    CHECK_EQ(url->protocol, "ftp");
    CHECK_EQ(url->hostname, "lumpy-omelet.name");
    CHECK_EQ(url->pathname.size(), 1);
    CHECK_EQ(url->pathname, (URL::Segments{"possimus"}));
    CHECK_EQ(url->query.empty(), false);
    CHECK_EQ(url->query["agnitio"], (URL::Segments{"socius"}));
    CHECK_EQ(url->query["deduco"], (URL::Segments{"aveho"}));
    CHECK_EQ(url->fragment.empty(), false);
    CHECK_EQ(url->fragment, "@/'Ry4@ffrTwN");
    CHECK_EQ(
        url->location(), "ftp://lumpy-omelet.name:38382/"
                         "possimus?agnitio=socius&deduco=aveho#@/'Ry4@ffrTwN");
  }
}

#endif