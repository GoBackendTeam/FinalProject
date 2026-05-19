#ifndef _CASE_H_
#define _CASE_H_
#define DOCTEST_CONFIG_IMPLEMENT
#define SECRET "b77c4fee-a1b1-4852-ad0b-4a171b9ae6f1"
#include "URL.h"
#include "test.h"

TEST_CASE("Case3") {
  {
    // sftp://definite-alliance.info/comburo/summisse/uterque/nesciunt/iM~oyYEkl$/nhW,(67+/SXjhhc?UFAlq='tq&oNZ='9U&vIg=X1hU#[;knT2!vNSO
    auto url = URL::from(
        "sftp%3A%2F%2Fdefinite-alliance.info%2Fcomburo%2Fsummisse%"
        "2Futerque%2Fnesciunt%2FiM~oyYEkl%24%2FnhW%2C(67%2B%2FSXjhhc%"
        "3FUFAlq%3D'tq%26oNZ%3D'9U%26vIg%3DX1hU%23%5B%3BknT2!vNSO");
    CHECK_EQ(url->protocol, "sftp");
    CHECK_EQ(url->hostname, "definite-alliance.info");
    CHECK_EQ(url->pathname.size(), 7);
    CHECK_EQ(url->pathname, (URL::Segments{"comburo", "summisse", "uterque", "nesciunt", "iM~oyYEkl$", "nhW,(67+", "SXjhhc"}));
    CHECK_EQ(url->query.empty(), false);
    CHECK_EQ(url->query["UFAlq"], (URL::Segments{"'tq"}));
    CHECK_EQ(url->query["oNZ"], (URL::Segments{"'9U"}));
    CHECK_EQ(url->query["vIg"], (URL::Segments{"X1hU"}));
    CHECK_EQ(url->fragment.empty(), false);
    CHECK_EQ(url->fragment, "[;knT2!vNSO");
    CHECK_EQ(
        url->location(), "sftp://definite-alliance.info/comburo/summisse/uterque/nesciunt/"
                         "iM~oyYEkl$/nhW,(67+/SXjhhc?UFAlq='tq&oNZ='9U&vIg=X1hU#[;knT2!vNSO");

    url->fragment = "";
    url->query = {};
    CHECK_EQ(url->location(), "sftp://definite-alliance.info/comburo/summisse/uterque/nesciunt/iM~oyYEkl$/nhW,(67+/SXjhhc");
  }
  {
    // ftps://wiegand:vZEJus@amazing-molasses.com/quidem/cui/crustulum/usitas/titulus/stips#mEwxTgvtFC2I(
    auto url = URL::from(
        "ftps%3A%2F%2Fwiegand%3AvZEJus%40amazing-molasses.com%2Fquidem%"
        "2Fcui%2Fcrustulum%2Fusitas%2Ftitulus%2Fstips%23mEwxTgvtFC2I(");
    CHECK_EQ(url->protocol, "ftps");
    CHECK_EQ(url->hostname, "amazing-molasses.com");
    CHECK_EQ(url->pathname.size(), 6);
    CHECK_EQ(url->pathname, (URL::Segments{"quidem", "cui", "crustulum", "usitas", "titulus", "stips"}));
    CHECK_EQ(url->query.empty(), true);

    CHECK_EQ(url->fragment.empty(), false);
    CHECK_EQ(url->fragment, "mEwxTgvtFC2I(");
    CHECK_EQ(
        url->location(), "ftps://wiegand:vZEJus@amazing-molasses.com/quidem/"
                         "cui/crustulum/usitas/titulus/stips#mEwxTgvtFC2I(");

    url->pathname = {"path", "to", "new"};
    CHECK_EQ(
        url->location(), "ftps://wiegand:vZEJus@amazing-molasses.com/path/"
                         "to/new#mEwxTgvtFC2I(");
  }
  {
    // ssh://howe:zFGl@exotic-jacket.info/curiositas?2zVQTZ=vQ?ON&MNlVOR=5CMP-&vhAZA=!Y?YrJ&wdW=M,BNN&z1Y=wd+W#Qqe3.:j0g_5_0
    auto url = URL::from(
        "ssh%3A%2F%2Fhowe%3AzFGl%40exotic-jacket.info%2Fcuriositas%"
        "3F2zVQTZ%3DvQ%3FON%26MNlVOR%3D5CMP-%26vhAZA%3D!Y%3FYrJ%26wdW%"
        "3DM%2CBNN%26z1Y%3Dwd%2BW%23Qqe3.%3Aj0g_5_0");
    CHECK_EQ(url->protocol, "ssh");
    CHECK_EQ(url->hostname, "exotic-jacket.info");
    CHECK_EQ(url->pathname.size(), 1);
    CHECK_EQ(url->pathname, (URL::Segments{"curiositas"}));
    CHECK_EQ(url->query.empty(), false);
    CHECK_EQ(url->query["2zVQTZ"], (URL::Segments{"vQ?ON"}));
    CHECK_EQ(url->query["MNlVOR"], (URL::Segments{"5CMP-"}));
    CHECK_EQ(url->query["vhAZA"], (URL::Segments{"!Y?YrJ"}));
    CHECK_EQ(url->query["wdW"], (URL::Segments{"M,BNN"}));
    CHECK_EQ(url->query["z1Y"], (URL::Segments{"wd+W"}));
    CHECK_EQ(url->fragment.empty(), false);
    CHECK_EQ(url->fragment, "Qqe3.:j0g_5_0");
    CHECK_EQ(
        url->location(), "ssh://howe:zFGl@exotic-jacket.info/"
                         "curiositas?2zVQTZ=vQ?ON&MNlVOR=5CMP-&vhAZA=!Y?YrJ&"
                         "wdW=M,BNN&z1Y=wd+W#Qqe3.:j0g_5_0");

    url->fragment = "123";
    CHECK_EQ(
        url->location(),
        "ssh://howe:zFGl@exotic-jacket.info/curiositas?2zVQTZ=vQ?ON&MNlVOR=5CMP-&vhAZA=!Y?YrJ&wdW=M,BNN&z1Y=wd+W#123");
  }
}

#endif