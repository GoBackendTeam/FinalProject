/**
 * @file URL.h
 * @brief 004 URL solution
 * @author Author: Wayne, Hsu
 */
#ifndef URL_H_
#define URL_H_
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

enum DELIMTER {
  PROTOCOL_DIVIDER_LEN = 3, // ://
  AUTH_DIVIDER_LEN = 1,     // @
  PATH_DIVIDER_LEN = 1,     // /
  QUERY_DIVIDER_LEN = 1,    // ?
  USERINFO_DIVIDER_LEN = 1, // :
  PORT_DIVIDER_LEN = 1,     // :
  FRAGMENT_DIVIDER_LEN = 1  // #
};

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
    std::ostringstream oss;
    oss << this->protocol << "://";

    if (!this->username.empty() && !this->password.empty()) {
      oss << this->username << ":" << this->password << "@";
    }

    oss << hostname;

    if (!this->port.empty() && this->port != "0") {
      oss << ":" << this->port;
    }

    if (!this->pathname.empty()) {
      for (const auto& pathname : this->pathname) {
        oss << "/" << pathname;
      }
    }

    if (!this->query.empty()) {
      oss << "?";
      bool first = true;
      for (const auto& q : this->query) {
        for (const auto& qValue : q.second) {
          if (!first) {
            oss << "&";
          }
          oss << q.first << "=" << qValue;

          first = false;
        }
      }
    }

    if (!this->fragment.empty()) {
      oss << "#" << this->fragment;
    }
    return oss.str();
  };

  inline static std::shared_ptr<URL> from(std::string inputUrl) {
    auto url = std::make_shared<URL>();
    std::string decodedUrl = percentDecode(inputUrl);

    struct URLComponents {
      size_t protocolEndPos;   // "://"
      size_t pathStartPos;     // "/"
      size_t queryStartPos;    // "?"
      size_t fragmentStartPos; // "#"
    };

    URLComponents components = {
        std::string::npos,
        std::string::npos,
        std::string::npos,
        std::string::npos,
    };

    // Find Protocol
    components.protocolEndPos = decodedUrl.find("://");

    // Find Fragment
    components.fragmentStartPos = decodedUrl.find("#");

    // Find Query
    std::string beforeFragment =
        (components.fragmentStartPos != std::string::npos) ? decodedUrl.substr(0, components.fragmentStartPos) : decodedUrl;
    components.queryStartPos = beforeFragment.find("?");

    // Find Path
    std::string beforeQuery =
        (components.queryStartPos != std::string::npos) ? beforeFragment.substr(0, components.queryStartPos) : beforeFragment;
    if (components.protocolEndPos != std::string::npos) {
      components.pathStartPos = decodedUrl.find("/", components.protocolEndPos + 3);
    } else {
      components.pathStartPos = beforeQuery.find("/");
    }

    // Set Protocol
    if (components.protocolEndPos != std::string::npos) {
      url->protocol = decodedUrl.substr(0, components.protocolEndPos);
    } else {
      url->protocol = decodedUrl;
      return url;
    }

    // Parse Authority
    std::string authority;
    size_t authorityEndPos = std::string::npos;

    if (components.pathStartPos != std::string::npos) {
      authorityEndPos = components.pathStartPos;
    } else if (components.queryStartPos != std::string::npos) {
      authorityEndPos = components.queryStartPos;
    } else if (components.fragmentStartPos != std::string::npos) {
      authorityEndPos = components.fragmentStartPos;
    } else {
      authorityEndPos = decodedUrl.length();
    }

    authority = decodedUrl.substr(
        components.protocolEndPos + PROTOCOL_DIVIDER_LEN, authorityEndPos - (components.protocolEndPos + PROTOCOL_DIVIDER_LEN));

    auto userinfoEndPos = authority.find("@");
    if (userinfoEndPos != std::string::npos && userinfoEndPos > 0) {
      // Parse userinfo
      std::string userinfo = authority.substr(0, userinfoEndPos);
      auto userinfoDividerPos = userinfo.find(":");
      if (userinfoDividerPos != std::string::npos) {
        url->username = userinfo.substr(0, userinfoDividerPos);
        url->password = userinfo.substr(userinfoDividerPos + USERINFO_DIVIDER_LEN);
      }
      authority = authority.substr(userinfoEndPos + AUTH_DIVIDER_LEN);
    }

    // Set hostname & port
    auto domainEndPos = authority.find(":");
    if (domainEndPos != std::string::npos) {
      url->hostname = authority.substr(0, domainEndPos);
      url->port = authority.substr(domainEndPos + PORT_DIVIDER_LEN);
    } else {
      url->hostname = authority;
    }

    // Set Path
    if (components.pathStartPos != std::string::npos) {
      size_t pathEndPos = std::string::npos;
      if (components.queryStartPos != std::string::npos) {
        pathEndPos = components.queryStartPos;
      } else if (components.fragmentStartPos != std::string::npos) {
        pathEndPos = components.fragmentStartPos;
      } else {
        pathEndPos = decodedUrl.length();
      }

      std::string path =
          decodedUrl.substr(components.pathStartPos + PATH_DIVIDER_LEN, pathEndPos - (components.pathStartPos + PATH_DIVIDER_LEN));

      if (!path.empty()) {
        std::stringstream ss(path);
        std::string pathname;
        while (std::getline(ss, pathname, '/')) {
          if (!pathname.empty()) {
            url->pathname.push_back(pathname);
          }
        }
      }
    }

    // Set Query
    if (components.queryStartPos != std::string::npos) {
      size_t queryEndPos = (components.fragmentStartPos != std::string::npos) ? components.fragmentStartPos : decodedUrl.length();

      std::string query = decodedUrl.substr(
          components.queryStartPos + QUERY_DIVIDER_LEN, queryEndPos - (components.queryStartPos + QUERY_DIVIDER_LEN));

      if (!query.empty()) {
        std::stringstream ss(query);
        std::string queryPair;
        while (std::getline(ss, queryPair, '&')) {
          if (queryPair.empty())
            continue;

          auto queryEqualPos = queryPair.find("=");
          if (queryEqualPos != std::string::npos) {
            std::string key = queryPair.substr(0, queryEqualPos);
            std::string value = queryPair.substr(queryEqualPos + 1);
            url->query[key].push_back(value);
          }
        }
      }
    }

    // Set Fragment
    if (components.fragmentStartPos != std::string::npos) {
      url->fragment = decodedUrl.substr(components.fragmentStartPos + FRAGMENT_DIVIDER_LEN);
    }

    return url;
  };

  static inline std::string percentDecode(const std::string& url) {
    std::ostringstream oss;
    for (size_t i = 0; i < url.size(); ++i) {
      if (url[i] == '%' && i + 2 < url.size()) {
        std::string hex = url.substr(i + 1, 2);
        char ch = static_cast<char>(std::stoi(hex, nullptr, 16));
        oss << ch;
        i += 2;
      } else {
        oss << url[i];
      }
    }
    return oss.str();
  };
};

#endif // !URL_H_
