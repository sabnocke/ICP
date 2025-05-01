//
// Created by ReWyn on 30.04.2025.
//

#include "Utils.h"
#include <cctype>
#include <locale>
#include <algorithm>

std::string Utils::TrimLeft(const std::string &str) {
  const std::string pattern = " \f\n\r\t\v";
  return str.substr(str.find_first_not_of(pattern));
}

std::string Utils::TrimRight(const std::string &str) {
  const std::string pattern = " \f\n\r\t\v";
  return str.substr(0, str.find_last_not_of(pattern) + 1);
}

std::string Utils::Trim(const std::string &str) {
  return TrimLeft(TrimRight(str));
}

std::string Utils::Purify(const std::string &str) {
  std::string result = "";
  for (const unsigned c : str) {
    if (!isspace(c)) {
      result += c;
    }
  }
  return result;
}

std::string Utils::Remove(const std::string &str, const char c) {
  std::string result = str;
  result.erase(std::ranges::remove(result, c).begin(), result.end());
  return result;
}

std::string Utils::Remove(const std::string &str, const std::string &substr) {
  std::string result = str;
  size_t pos = std::string::npos;
  while ((pos = result.find(substr, pos)) != std::string::npos) {
    result.erase(pos, substr.size());
  }
  return result;
}
