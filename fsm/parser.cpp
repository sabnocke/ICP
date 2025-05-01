//
// Created by ReWyn on 28.04.2025.
//

#include "parser.h"
#include <absl/log/absl_check.h>
#include <ctre.hpp>
#include <fstream>
#include <iostream>
#include "AutomatLib.h"

#include <re2/re2.h>
#include "Utils.h"

#define DEFAULT_NAME "Unknown"

Automat parser::parseAutomat(const std::string &file) {
  std::ifstream ifs(file);
  if (!ifs) {
    std::cerr << "Can't open file " << file << std::endl;
    return automat;
  }
  std::string line;
  enum Section { Name, Comment, Variables, States, Transitions };
  Section currentSection = Name;

  while (std::getline(ifs, line)) {
    if (line.find("Name") != std::string::npos) {
      currentSection = Name;
    } else if (line.find("Comment") != std::string::npos) {
      currentSection = Comment;
    } else if (line.find("Variables") != std::string::npos) {
      currentSection = Variables;
    } else if (line.find("States") != std::string::npos) {
      currentSection = States;
    } else if (line.find("Transitions") != std::string::npos) {
      currentSection = Transitions;
    } else {
      switch (currentSection) {
        case Name:
          automat.Name = extractName(line).value_or(DEFAULT_NAME);
          continue;
        case Comment:
          automat.Comment = line;
          continue;
        case States:
          parseState(line);
        case Transitions:
          parseTransition(line);
          continue;
        case Variables:
          parseVariable(line);
      }
    }
  }
  return automat;
}

void parser::parseState(const std::string &line) {
  const auto trimmed = Utils::Trim(line);
  std::string name, code;
  ABSL_CHECK(RE2::FullMatch(trimmed, R"((?<name>[A-Z0-9]+) *: *\{(?<code>.+)\})", &name, &code));
  automat.addState(name, code);
  // TODO can the code be on multiple lines?
  // TODO if so, then it cannot be detected via regex this easily, would need more parsing and accumulation
}

/// Extracts name from line, expecting format of "Name:<name>".
/// The string should be purified prior to calling this function
/// @param line Line from which name will be extracted
/// @return String containing name, otherwise nothing
std::optional<std::string> parser::extractName(const std::string &line) {
  const std::string pureLine = Utils::Purify(line);
  if (const auto colon = pureLine.find(':'); colon != std::string::npos) {
    return line.substr(colon + 1);
  }
  return std::nullopt;
}

void parser::parseVariable(const std::string &line) {
  const auto trimmed = Utils::Trim(line);
  std::string type, name, value;
  ABSL_CHECK(RE2::FullMatch(trimmed, R"((?<type>\w+) (?<name>\w+) = (?<value>\w+))", &type, &name, &value));
  automat.addVariable(type, name, value);
}


void parser::parseTransition(const std::string &line) {
  const auto trimmed = Utils::Trim(line); // Preserve structure of condition, if any is present
  std::string from, to, input, cond, delay;
  ABSL_CHECK(RE2::FullMatch(trimmed,
                            R"((?<from>\w+) *--> *(?<to>\w+) *: *(?<input>\w*) *\[?(?<cond>.+)?\]?@? *(?<delay>\w+)?)",
                            &from, &to, &input, &cond, &delay));

  automat.addTransition(from, to, input, cond, delay);
}
