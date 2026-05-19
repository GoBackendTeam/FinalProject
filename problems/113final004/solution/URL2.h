/**
 * @file URL.h
 * @brief 004 URL solution
 * @author Yan Hau Chen
 */
#ifndef URL_H_
#define URL_H_
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

class URL {
  public:
  using Query = std::map<std::string, std::vector<std::string>>;
  using Segments = std::vector<std::string>;

  std::string protocol = "";
  std::string username = "";
  std::string password = "";
  std::string hostname = "";
  std::string port = "0";
  Segments pathname;
  std::string fragment = "";
  Query query;

  inline std::string location() {
    std::stringstream ss;
    ss << this->protocol << "://" << (this->username.empty() || this->password.empty() ? "" : this->username + ':')
       << (this->username.empty() || this->password.empty() ? "" : this->password + '@') << this->hostname
       << (this->port == "0" ? "" : ":" + this->port) << '/';

    for (auto& seg : this->pathname) {
      ss << seg << '/';
    }

    // 刪除 trailing slace(`/`)
    auto withTrailingSlash = ss.str();
    ss.str(ss.str().substr(0, ss.str().size() - 1));
    ss.seekp(withTrailingSlash.size() - 1);

    ss << (this->query.empty() ? "" : "?");
    for (auto& query : this->query) {
      for (auto& value : query.second) {
        ss << query.first << '=' << value << '&';
      }
    }

    // 若包含 Querystring 刪除 trailing and(`&`)
    if (!this->query.empty()) {
      auto withTrailingAnd = ss.str();
      ss.str(withTrailingAnd.substr(0, withTrailingAnd.size() - 1));
      ss.seekp(withTrailingAnd.size() - 1);
    }

    ss << (this->fragment.empty() ? "" : "#") << this->fragment;
    return ss.str();
  }

  static inline std::shared_ptr<URL> from(std::string inputUrl) {
    auto url = std::make_shared<URL>();
    // Parsing variables
    std::stringstream buf("");
    std::istringstream iss(URL::decode(inputUrl) + "\x1F");
    char cursor;
    enum class Parse { PROTOCOL, AUTHORITY, PATHNAME, QUERTSTRING, FRAGMENT };
    auto state = Parse::PROTOCOL;

    // Temporary variables
    std::vector<std::string> temp{};
    std::function<bool(char)> isDelimter;
    std::string writingKey = "";

    /**
     * 可以嘗試在原始的URL 隨便加一個 char 當作結束標記，本範例使用 `\x1F`(Unit Separator)
     * 小心不要使用 \v \t \n \0 (space) 等會被 isspace 檢測到的字元
     * 若沒有進行此處理，則最後 buf 必然會存在一些資料，請調用本函數進行處理
     */
    auto flush = [&]() {
      switch (state) {
      case Parse::PATHNAME:
        url->pathname.emplace_back(buf.str());
        break;

      case Parse::QUERTSTRING:
        url->query[writingKey].emplace_back(buf.str());
        break;

      case Parse::FRAGMENT:
        url->fragment = buf.str();
        break;

      case Parse::PROTOCOL:
      case Parse::AUTHORITY:
      default:
        /* No thing to do */
        break;
      }
    };

    while (iss >> cursor) {
      /**
       * 當首次看到 `:`，資料必然是 Protocol
       * 將資料讀入 Buffer, 並將 cursor 往後兩個字元( seekg +2 )
       * 那之後，狀態轉移至解析 Authority
       */
      if (state == Parse::PROTOCOL && cursor == ':') {
        url->protocol = buf.str();
        iss.seekg(+2, std::ios::cur);
        state = Parse::AUTHORITY;
        buf.str("");
        continue;
      }

      /**
       * AUTHORITY 的組成為 [USERNAME : PASSWORD @] DOMAIN [: PORT]
       * 使用 `:` 與 `@` 即可分隔出此階段所有的區塊
       * 當讀取到 `/` 後，狀態轉移至解析 Pathname
       */
      isDelimter = [](char ch) {
        return ch == ':' || ch == '@' || ch == '/';
      };
      if (state == Parse::AUTHORITY && isDelimter(cursor)) {
        if (buf.tellp() != 0)
          temp.emplace_back(buf.str());
        buf.str("");
        if (cursor == '/') {
          /**
           * 這裡有個小技巧，只需要判斷 temp.size() 即可
           * - temp.size() = 1，代表只有 DOMAIN
           * - temp.size() = 2，代表包含 DOMAIN PORT
           * - temp.size() = 3，代表包含 USERNAME PASSWORD DOMAIN
           * - temp.size() = 4，代表包含 USERNAME PASSWORD DOMAIN PORT
           */
          switch (temp.size()) {
          case 1:
            url->hostname = temp[0];
            break;
          case 2:
            url->hostname = temp[0];
            url->port = temp[1];
            break;
          case 3:
            url->username = temp[0];
            url->password = temp[1];
            url->hostname = temp[2];
            break;
          case 4:
            url->username = temp[0];
            url->password = temp[1];
            url->hostname = temp[2];
            url->port = temp[3];
            break;
          }
          state = Parse::PATHNAME;
        }
        continue;
      }

      /**
       * PATHNAME 由 `/` 分隔
       * - 看到 `?` 開始解析 Querystring
       * - 看到 `#` 開始解析 Fragment
       */
      isDelimter = [](char ch) {
        return ch == '?' || ch == '#' || ch == '/' || ch == '\x1F';
      };
      if (state == Parse::PATHNAME && isDelimter(cursor)) {
        // clang-format off
        state = (cursor == '?') ? Parse::QUERTSTRING
              : (cursor == '#') ? Parse::FRAGMENT
              : Parse::PATHNAME;
        // clang-format on
        if (!buf.str().empty())
          url->pathname.emplace_back(buf.str());
        buf.str("");
        continue;
      }

      /**
       * Querystring 由 `&` 分隔，組成為 Key=Value [ & Key=Value ]...
       * - 讀到 `=` 解析出 Key
       * - 讀到 `&` 解析出 Value
       * - 讀到 `#` 開始解析 Fragment
       */
      isDelimter = [](char ch) {
        return ch == '&' || ch == '#' || ch == '=' || ch == '\x1F';
      };
      if (state == Parse::QUERTSTRING && isDelimter(cursor)) {
        if (cursor == '=') {
          writingKey = buf.str();
          buf.str("");
          continue;
        } else if (cursor == '&' || cursor == '\x1F') {
          url->query[writingKey].emplace_back(buf.str());
          buf.str("");
          continue;
        } else {
          url->query[writingKey].emplace_back(buf.str());
          buf.str("");
          state = Parse::FRAGMENT;
          continue;
        }
      }

      /**
       * 若 URL 尾部沒有添加終止字元
       * 需要在外部使用 flush 讀出 buf 中剩餘的字串
       */
      if (state == Parse::FRAGMENT && cursor == '\x1F') {
        url->fragment = buf.str();
      }

      buf << cursor;
    }

    /**
     * 若沒有像助教一樣，在結尾處添加 `\x1F` 之類的中止符號
     * Buffer 會剩餘一些字元，可以透過 flush 取出。
     */
    // flush();

#ifdef FOR_STUDENT_WATCHED
    std::cout << "protocol = " << url->protocol << '\n'
              << "username = " << url->username << '\n'
              << "password = " << url->password << '\n'
              << "hostname = " << url->hostname << '\n'
              << "port = " << url->port << '\n'
              << "Pathname = ";
    for (auto& segment : url->pathname)
      std::cout << segment << '/';

    std::cout << "\nQuery:";
    for (auto& pair : url->query) {
      std::cout << "\n\x1F" << pair.first << " = {";
      for (auto& value : pair.second)
        std::cout << value << " ,";
      std::cout << "}";
    }
    std::cout << "\nfragment = " << url->fragment << '\n';
    std::cout << "location = " << url->location() << '\n';
#endif
    return url;
  }

  inline static std::string decode(const std::string& url) {
    std::istringstream iss(url);
    std::stringstream buf("");
    char cursor;

    while (iss >> cursor) {
      if (cursor == '%') {
        // 針對 %XX 進行解碼，讀出兩個字元後，使用 stoi 以 base16 輸出至 Buffer 即可
        auto hexStr = new char;
        iss.get(hexStr, 3);
        auto ASCIICCode = std::stoi(std::string(hexStr), nullptr, 16);
        buf << static_cast<char>(ASCIICCode);
        continue;
      }

      buf << cursor;
    }
    return buf.str();
  };
};

#endif // !URL_H_
