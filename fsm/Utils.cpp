#include "Utils.h"

// #include <fast_float/fast_float.h>
#include "external/fast_float.h"
#include <locale>
#include <range/v3/all.hpp>
#include "absl/strings/ascii.h"

namespace Utils {

std::string Trim(const std::string &str) {
  if (str.empty())
    return "";

  auto result_view =
      str | ranges::views::drop_while(IsWhiteSpace) | ranges::views::reverse |
      ranges::views::drop_while(IsWhiteSpace) | ranges::views::reverse;

  return result_view | ranges::to<std::string>();
}

std::vector<std::string> TrimEach(std::vector<std::string> &vec) {
  return vec | ranges::views::transform([](const std::string &str) {
           return Trim(str);
         }) |
         ranges::to<std::vector<std::string>>;
}

std::string_view Trim(const std::string_view str) {
  if (str.empty())
    return {};

  const auto first = ranges::find_if(str, IsNotWhitespace);
  if (first == str.end())
    return {};

  const auto last =
      ranges::find_if(str | ranges::views::reverse, IsNotWhitespace);

  const auto start = ranges::distance(str.begin(), first);
  const auto count = ranges::distance(first, last.base());

  return str.substr(start, count);
}

std::string Remove(const std::string &str, const char c) {
  auto r = str | ranges::views::filter(
                     [c0 = c](const unsigned char &ch) { return ch != c0; });
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
  auto view = str | ranges::views::transform(
                        [](auto c) { return absl::ascii_tolower(c); });
  return view | ranges::to<std::string>();
}
std::string ToLower(const std::string_view str) {
  return ToLower(std::string(str));
}
char ToLower(const char c) { return absl::ascii_tolower(c); }

bool Contains(const std::string &str, const std::string_view view) {
  const auto lower = ToLower(str);
  if (const auto lower_s = ToLower(view);
      lower.find(lower_s) == std::string::npos)
    return false;
  return true;
}

[[deprecated]] std::optional<long long> AttemptIntegerConversion(const std::string &input) {
  const auto first = input.data();
  const auto last = input.data() + input.size();
  long long value;

  if (auto [ptr, ec] = fast_float::from_chars(first, last, value);
      ec != std::errc() || ptr != last) {
    return std::nullopt;
  }

  return value;
}
[[deprecated]] std::optional<double> AttemptDoubleConversion(const std::string &input) {
  const auto first = input.data();
  const auto last = input.data() + input.size();
  double value;

  if (auto [ptr, ec] = fast_float::from_chars(first, last, value);
      ec != std::errc{} || ptr != last) {
    return std::nullopt;
  }

  return value;
}

}  // namespace Utils
