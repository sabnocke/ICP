//
// Created by ReWyn on 30.04.2025.
//

#include "Utils.h"
#include <absl/log/log.h>
#include <algorithm>
#include <cctype>
#include <iostream>
#include <locale>
#include <optional>
#include <range/v3/algorithm/find_if.hpp>
#include <range/v3/range.hpp>
#include <range/v3/view.hpp>
#include <sstream>

namespace Utils {
  // TODO uses C++20, while project is C++17
  // TODO in case it causes errors remove std::ranges (and add str.begin() and str.end() into find_if)

  std::string Trim(const std::string &str) {
    if (str.empty())
      return {};

    std::string result = str;

    result.erase(
      ranges::find_if(result | ranges::view::reverse, IsNotWhitespace).base(),
      result.end()
      );
    result.erase(
      result.begin(),
      ranges::find_if(result, IsNotWhitespace));

    return result;
  }

  std::string Purify(const std::string &str) {
    std::string result;
    result.reserve(str.size());
    for (const char c: str) {
      if (!std::isspace(c)) {
        result += c;
      }
    }
    return result;
  }

  std::string Remove(const std::string &str, const char c) {
    std::string result = str;
    std::erase(result, c);
    return result;
  }

  std::string Remove(const std::string &str, const std::string &substr) {
    std::string result = str;
    size_t pos = std::string::npos;
    if (str.empty() || substr.empty())
      return result;
    while ((pos = result.find(substr, pos)) != std::string::npos) {
      result.erase(pos, substr.size());
    }
    return result;
  }

} // namespace Utils
