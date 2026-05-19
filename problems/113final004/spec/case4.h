#ifndef _CASE_H_
#define _CASE_H_
#define DOCTEST_CONFIG_IMPLEMENT
#define SECRET "89d03d30-3bcc-4f79-8c96-4455b50251cc"
#include "URL.h"
#include "test.h"

TEST_CASE("Case4") {

  {
    // ftp://severe-humor.biz/acies/vester/constans/textor/varietas/ulciscor/sub/odio/C/fa/9K~&urNWzy/ID-W9m?03a5=S)G9
    auto url = URL::from(
        "ftp%3A%2F%2Fsevere-humor.biz%2Facies%2Fvester%"
        "2Fconstans%2Ftextor%2Fvarietas%2Fulciscor%2Fsub%2Fodio%"
        "2FC%2Ffa%2F9K~%26urNWzy%2FID-W9m%3F03a5%3DS)G9");
    CHECK_EQ(url->protocol, "ftp");
    CHECK_EQ(url->hostname, "severe-humor.biz");
    CHECK_EQ(url->pathname.size(), 12);
    CHECK_EQ(
        url->pathname,
        (URL::Segments{
            "acies", "vester", "constans", "textor", "varietas", "ulciscor", "sub", "odio", "C", "fa", "9K~&urNWzy", "ID-W9m"}));
    CHECK_EQ(url->query.empty(), false);
    CHECK_EQ(url->query["03a5"], (URL::Segments{"S)G9"}));
    CHECK_EQ(url->fragment.empty(), true);

    CHECK_EQ(
        url->location(), "ftp://severe-humor.biz/acies/vester/constans/textor/varietas/"
                         "ulciscor/sub/odio/C/fa/9K~&urNWzy/ID-W9m?03a5=S)G9");
    url->query = URL::Query{
        {"key", {"a", "b", "c"}}
    };
    url->hostname = "www.example.com";
    CHECK_EQ(
        url->location(), "ftp://www.example.com/acies/vester/constans/textor/varietas/"
                         "ulciscor/sub/odio/C/fa/9K~&urNWzy/ID-W9m?key=a&key=b&key=c");
  }
  {
    // ftps://unconscious-lady.net/comburo/illo/xiphias/aranea
    auto url = URL::from("ftps%3A%2F%2Funconscious-lady.net%2F%2F%2Fcomburo%2Fillo%2Fxiphias%2Faranea");
    CHECK_EQ(url->protocol, "ftps");
    CHECK_EQ(url->hostname, "unconscious-lady.net");
    CHECK_EQ(url->pathname.size(), 4);
    CHECK_EQ(url->pathname, (URL::Segments{"comburo", "illo", "xiphias", "aranea"}));
    CHECK_EQ(url->query.empty(), true);
    CHECK_EQ(url->fragment.empty(), true);
    CHECK_EQ(url->location(), "ftps://unconscious-lady.net/comburo/illo/xiphias/aranea");
  }
  {
    // http://wordy-cardboard.com:12750/clamo/angelus/esse/sit/modi/beatae/tego/appello?7nsH=!_7e&YLR=.ES)Q2ZV&nIVFQQ=KtZZ_zZ+&uaYg=3au/N&vd4=o;K
    auto url = URL::from(
        "http%3A%2F%2Fwordy-cardboard.com%3A12750%2Fclamo%2Fangelus%2Fesse%2Fsit%"
        "2Fmodi%2Fbeatae%2Ftego%2Fappello%3F7nsH%3D!_7e%26YLR%3D.ES)Q2ZV%"
        "26nIVFQQ%3DKtZZ_zZ%2B%26uaYg%3D3au%2FN%26vd4%3Do%3BK");
    CHECK_EQ(url->protocol, "http");
    CHECK_EQ(url->hostname, "wordy-cardboard.com");
    CHECK_EQ(url->pathname.size(), 8);
    CHECK_EQ(url->pathname, (URL::Segments{"clamo", "angelus", "esse", "sit", "modi", "beatae", "tego", "appello"}));
    CHECK_EQ(url->query.empty(), false);
    CHECK_EQ(url->query["7nsH"], (URL::Segments{"!_7e"}));
    CHECK_EQ(url->query["YLR"], (URL::Segments{".ES)Q2ZV"}));
    CHECK_EQ(url->query["nIVFQQ"], (URL::Segments{"KtZZ_zZ+"}));
    CHECK_EQ(url->query["uaYg"], (URL::Segments{"3au/N"}));
    CHECK_EQ(url->query["vd4"], (URL::Segments{"o;K"}));
    CHECK_EQ(url->fragment.empty(), true);

    CHECK_EQ(
        url->location(), "http://wordy-cardboard.com:12750/clamo/angelus/esse/sit/modi/beatae/"
                         "tego/appello?7nsH=!_7e&YLR=.ES)Q2ZV&nIVFQQ=KtZZ_zZ+&uaYg=3au/N&vd4=o;K");
  }
}

#endif