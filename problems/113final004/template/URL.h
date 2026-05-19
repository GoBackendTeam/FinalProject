#ifndef URL_H_
#define URL_H_
#include <map>
#include <memory>
#include <string>
#include <vector>

/**
 * @brief Suggested Implementation Steps for a URL Parser
 *
 * Parsing a percent-encoded URL into structured components typically involves the following steps:
 *
 * 1. Percent-decoding
 *    - Apply percent-decoding to the entire input string before structural parsing.
 *    - Decode only once, respecting reserved and unreserved characters per URI standards.
 *
 * 2. Tokenization by structural delimiters
 *    - Identify positions of `://`, `@`, `:`, `?`, and `#` to segment the string into:
 *        - protocol
 *        - authority (which may contain username, password, hostname, and port)
 *        - pathname
 *        - querystring
 *        - fragment
 *    - Prefer a state-machine or layered parsing approach to avoid delimiter conflicts.
 *
 * 3. Authority parsing (if present)
 *    - If authority includes `@`, split it into userinfo and host.
 *    - Inside userinfo, separate username and password (if any) via `:`.
 *    - After userinfo, handle hostname and optional port.
 *
 * 4. Pathname parsing
 *    - The pathname starts after the authority and ends before `?` or `#`.
 *    - Normalize leading slashes; retain meaningful structure.
 *
 * 5. Query string parsing
 *    - Split the query segment by `&` into key-value pairs.
 *    - Each pair is of the form `key=value`.
 *    - Store key-value pairs in a structure that allows sorting by key for deterministic output.
 *
 * 6. Fragment parsing
 *    - Extract everything after the first `#`, if present.
 *    - It may include any character; decode it for completeness.
 *
 * 7. Reconstruction (location method)
 *    - Compose the output URL string using internal fields.
 *    - The pathname must not end with a trailing `/`, unless it is the root.
 *    - The query string must not end with a trailing `&`.
 *    - Keys in the query string must appear in alphabetical order.
 */
class URL {
  public:
  using Query = std::map<std::string, std::vector<std::string>>;
  using Segments = std::vector<std::string>;

  std::string protocol = "";
  std::string username = "";
  std::string password = "";
  std::string hostname = "";
  std::string port = "";
  Segments pathname;
  std::string fragment = "";
  Query query;

  std::string location();

  static std::shared_ptr<URL> from(std::string inputUrl);

  static std::string percentDecode(const std::string& s);
};

#endif // URL_H_
