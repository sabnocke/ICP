//
// Created by ReWyn on 28.04.2025.
//

#pragma once
#include <algorithm>
#include <ctll.hpp>
#include <string>
#include "AutomatLib.h"

#include <re2/re2.h>

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
};