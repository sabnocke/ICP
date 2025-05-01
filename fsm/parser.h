//
// Created by ReWyn on 28.04.2025.
//

#pragma once
#include <algorithm>
#include <string>
#include <ctll.hpp>
#include "AutomatLib.h"

//TODO obsolete: pending deletion
struct ParseTransitionResult {
  std::string from;
  std::string to;
  std::string input;
  std::string cond;
  std::string delay;
};

class parser {
  public:
  Automat parseAutomat(const std::string &file); //TODO change it to optional
private:
  void parseVariable(const std::string &line);
  void parseState(const std::string &line);
  void parseTransition(const std::string &line);
  static std::optional<std::string> extractName(const std::string &line);
  Automat automat;
  static constexpr ctll::fixed_string pattern = R"(/(?<from>[\w\d]+)-->(?<to>[\w\d]+):(?<input>[\w\d]*)?(?<cond>\[.+\])?@?(?<delay>[\w\d]+)?/gm)";
  static constexpr ctll::fixed_string varPattern = R"((?<type>\w+) (?<name>\w+) = (?<value>\w+))";
};

//TODO obsolete: pending deletion, use Utils::TrimLeft instead
inline void ltrim(std::string &s) {
  s.erase(s.begin(), std::ranges::find_if(s, [](const unsigned char ch) {
    return !std::isspace(ch);
  }));
}

//TODO obsolete: pending deletion, use Utils::TrimRight instead
inline void rtrim(std::string &s) {
  s.erase(std::find_if(s.rbegin(), s.rend(), [](const unsigned char ch) {
    return !std::isspace(ch);
  }).base(), s.end());
}

//TODO obsolete: pending deletion, use Utils::Trim instead
inline void trim(std::string &s) {
  ltrim(s);
  rtrim(s);
}