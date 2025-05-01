//
// Created by ReWyn on 30.04.2025.
//

#include "Utils.h"
#include <algorithm>
#include <cctype>
#include <locale>
#include <optional>
#include <sstream>

std::string Utils::TrimLeft(const std::string &str) {
  const std::string pattern = " \f\n\r\t\v";
  return str.substr(str.find_first_not_of(pattern));
}

std::string Utils::TrimRight(const std::string &str) {
  const std::string pattern = " \f\n\r\t\v";
  return str.substr(0, str.find_last_not_of(pattern) + 1);
}

std::string Utils::Trim(const std::string &str) { return TrimLeft(TrimRight(str)); }

std::string Utils::Purify(const std::string &str) {
  std::string result;
  for (const unsigned char c: str) {
    if (!isspace(c)) {
      result += std::to_string(c);
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


template<typename T>
std::optional<std::string> Utils::ToStringOpt(const T value) {
  if constexpr (IsStdString<T>) {
    return value;
  } else if constexpr (IsStringView<T>) {
    return std::string(value);
  } else if constexpr (IsCString<T>) {
    return value == nullptr ? "" : std::string(value);
  } else if constexpr (std::same_as<std::remove_cvref_t<T>, bool>) {
    std::ostringstream oss;
    oss << std::boolalpha << value;
    return oss.str();
  } else if constexpr (Streamable<T>) {
    std::ostringstream oss;
    oss << value;
    return oss.str();
  } else {
    return std::nullopt;
  }
}

