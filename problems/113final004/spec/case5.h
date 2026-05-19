#ifndef _CASE_H_
#define _CASE_H_
#define DOCTEST_CONFIG_IMPLEMENT
#define SECRET "e463801d-9b12-4179-b8ea-d448c55702e2"
#include "URL.h"
#include "test.h"

TEST_CASE("Case5") {
  {
    // ftp://blue-jury.net:22071/socius/coniecto/tumultus/nobis/civitas/usus/3pXDg6/6?7kX=5KBS&MmrZ=DExg3&fArZZY=@e-
    auto url = URL::from(
        "ftp%3A%2F%2Fblue-jury.net%3A22071%2Fsocius%2Fconiecto%"
        "2Ftumultus%2Fnobis%2Fcivitas%2Fusus%2F3pXDg6%2F6%3F7kX%"
        "3D5KBS%26MmrZ%3DDExg3%26fArZZY%3D%40e-");
    CHECK_EQ(url->protocol, "ftp");
    CHECK_EQ(url->hostname, "blue-jury.net");
    CHECK_EQ(url->pathname.size(), 8);
    CHECK_EQ(url->pathname, (URL::Segments{"socius", "coniecto", "tumultus", "nobis", "civitas", "usus", "3pXDg6", "6"}));
    CHECK_EQ(url->query.empty(), false);
    CHECK_EQ(url->query["7kX"], (URL::Segments{"5KBS"}));
    CHECK_EQ(url->query["MmrZ"], (URL::Segments{"DExg3"}));
    CHECK_EQ(url->query["fArZZY"], (URL::Segments{"@e-"}));
    CHECK_EQ(url->fragment.empty(), true);
    CHECK_EQ(url->fragment, "");
    CHECK_EQ(
        url->location(), "ftp://blue-jury.net:22071/socius/coniecto/tumultus/nobis/civitas/"
                         "usus/3pXDg6/6?7kX=5KBS&MmrZ=DExg3&fArZZY=@e-");

    url->port = "1111";
    url->pathname = {"a", "b", "c"};
    url->query = URL::Query{
        {"X", {"!@~$%"}}
    };
    url->fragment = "~!@#$%^&*()_=";
    CHECK_EQ(url->location(), "ftp://blue-jury.net:1111/a/b/c?X=!@~$%#~!@#$%^&*()_=");
  }
  {
    // ftps://small-scorn.net/celebrer/spoliatio/tolero#:
    auto url = URL::from("ftps%3A%2F%2Fsmall-scorn.net%2Fcelebrer%2Fspoliatio%2Ftolero%23%3A");
    CHECK_EQ(url->protocol, "ftps");
    CHECK_EQ(url->hostname, "small-scorn.net");
    CHECK_EQ(url->pathname.size(), 3);
    CHECK_EQ(url->pathname, (URL::Segments{"celebrer", "spoliatio", "tolero"}));
    CHECK_EQ(url->query.empty(), true);

    CHECK_EQ(url->fragment.empty(), false);
    CHECK_EQ(url->fragment, ":");
    CHECK_EQ(url->location(), "ftps://small-scorn.net/celebrer/spoliatio/tolero#:");
  }
  {
    // ldap://impassioned-disclosure.org:28935/umerus/perferendis/sui/a/triduana#yQLXEB5(GN
    auto url = URL::from(
        "ldap%3A%2F%2Fimpassioned-disclosure.org%3A28935%2Fumerus%"
        "2Fperferendis%2Fsui%2Fa%2Ftriduana%23yQLXEB5(GN");
    CHECK_EQ(url->protocol, "ldap");
    CHECK_EQ(url->hostname, "impassioned-disclosure.org");
    CHECK_EQ(url->pathname.size(), 5);
    CHECK_EQ(url->pathname, (URL::Segments{"umerus", "perferendis", "sui", "a", "triduana"}));
    CHECK_EQ(url->query.empty(), true);

    CHECK_EQ(url->fragment.empty(), false);
    CHECK_EQ(url->fragment, "yQLXEB5(GN");
    CHECK_EQ(
        url->location(), "ldap://impassioned-disclosure.org:28935/umerus/"
                         "perferendis/sui/a/triduana#yQLXEB5(GN");
    url->port = "0";

    CHECK_EQ(
        url->location(), "ldap://impassioned-disclosure.org/umerus/"
                         "perferendis/sui/a/triduana#yQLXEB5(GN");
  }
}

#endif