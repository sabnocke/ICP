#include "Utils.h"

#include <locale>
#include <range/v3/view.hpp>

#include "absl/strings/ascii.h"
#include "absl/strings/match.h"
#include "range/v3/algorithm/find_if.hpp"

namespace Utils {

  std::string Trim(const std::string &str) {
    if (str.empty())
      return "";

    auto result_view = str | ranges::views::drop_while(IsWhiteSpace) | ranges::view::reverse |
                       ranges::view::drop_while(IsWhiteSpace) | ranges::view::reverse;

    return result_view | ranges::to<std::string>();
  }


  std::string_view Trim(const std::string_view str) {
    if (str.empty())
      return {};


    const auto first = ranges::find_if(str, IsNotWhitespace);
    if (first == str.end())
      return {};

    const auto last = ranges::find_if(str | ranges::views::reverse, IsNotWhitespace);

    const auto start = ranges::distance(str.begin(), first);
    const auto count = ranges::distance(first, last.base());

    return str.substr(start, count);
    // return {first, last.base()};
  }

  std::string Remove(const std::string &str, const char c) {
    auto r = str | ranges::views::filter([c0 = c](const unsigned char &ch) { return ch != c0; });
    return r | ranges::to<std::string>();
  }

  std::string Remove(const std::string &str, const std::string &substr) {
    if (str.empty() || substr.empty())
      return str;

    auto parts = str | ranges::views::split(substr);
    auto join = parts | ranges::views::join;
    return join | ranges::to<std::string>();
  }

  std::string ToLower(const std::string &str) {
    auto view = str | ranges::views::transform([](auto c) { return absl::ascii_tolower(c); });
    return view | ranges::to<std::string>();
  }
  std::string ToLower(const std::string_view str) {
    return ToLower(std::string(str));
  }

  bool Contains(const std::string &str, const std::string_view view) {
    const auto lower = ToLower(str);
    const auto lower_s = ToLower(view);
    return absl::StrContains(lower, lower_s);
  }

} // namespace Utils
