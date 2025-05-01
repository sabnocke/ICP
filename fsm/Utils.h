//
// Created by ReWyn on 30.04.2025.
//

#pragma once
#include <string>


class Utils {
public:
  static std::string Trim(const std::string &str);
  static std::string TrimLeft(const std::string &str);
  static std::string TrimRight(const std::string &str);
  static std::string Purify(const std::string &str);
  static std::string Remove(const std::string &str, char c);
  static std::string Remove(const std::string &str, const std::string &substr);
private:
  const std::string whitespacePattern = " \f\n\r\t\v";
};

