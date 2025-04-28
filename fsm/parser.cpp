//
// Created by ReWyn on 28.04.2025.
//

#include "parser.h"
#include <fstream>
#include <iostream>
#include "AutomatLib.h"
#include <regex.hpp>
#include <string/trim.hpp>
#include <ctre.hpp>

Automat parser::parseAutomat(const std::string &file) {
  std::ifstream ifs(file);
  if (!ifs) {
    std::cerr << "Can't open file " << file << std::endl;
    return automat;
  }

  std::string line;
  enum Section {Name, Comment, Variables, States, Transitions};

  Section currentSection = Name;

  while (std::getline(ifs, line)) {
    if (line.find("Name") != std::string::npos) {
      currentSection = Section::Name;
    } else if (line.find("Comment") != std::string::npos) {
      currentSection = Section::Comment;
    } else if (line.find("Variables") != std::string::npos) {
      currentSection = Section::Variables;
    } else if (line.find("States") != std::string::npos) {
      currentSection = Section::States;
    } else if (line.find("Transitions") != std::string::npos) {
      currentSection = Section::Transitions;
    } else {
      if (currentSection == Section::States) {
        parseState(line);
      }
      else if (currentSection == Section::Transitions) {
        parseTransition(line);
      }
    }
  }
  return automat;
}

void parser::parseState(const std::string &line) {
  const auto colon = line.find(':');
  if (colon == std::string::npos) {
    return;
  }
  auto name = line.substr(0, colon);  //TODO trim it
  automat.addState(name, [name]() {
    std::cout << "Entering state: " << name << std::endl;
  }); //TODO parse the action

}

void parser::parseTransition(const std::string &line) {
  const auto arrow = line.find("-->");
  const auto colon = line.find(':');
  if (arrow == std::string::npos || colon == std::string::npos) {
    return;
  }
  const auto from = line.substr(0, arrow);
  const auto to = line.substr(arrow + 3, colon - arrow - 3);
  automat.addTransition(
    from, to,
    []() {return true;}); //TODO parse the condition
}