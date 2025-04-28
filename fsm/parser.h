//
// Created by ReWyn on 28.04.2025.
//

#pragma once
#include <algorithm>
#include <string>

#include "AutomatLib.h"


class parser {
  public:
  Automat parseAutomat(const std::string &file); // change it to optional
private:
  void parseVariable(const std::string &line);
  void parseState(const std::string &line);
  void parseTransition(const std::string &line);
  Automat automat;
};

inline void ltrim(std::string &s) {
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](const unsigned char ch) {
    return !std::isspace(ch);
  }));
}
inline void rtrim(std::string &s) {
  s.erase(std::find_if(s.rbegin(), s.rend(), [](const unsigned char ch) {
    return !std::isspace(ch);
  }).base(), s.end());
}

inline void trim(std::string &s) {
  ltrim(s);
  rtrim(s);
}