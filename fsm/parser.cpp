//
// Created by ReWyn on 28.04.2025.
//

#include "parser.h"
#include <fstream>
#include <iostream>
#include "AutomatLib.h"
#include <ctre.hpp>
#include "Utils.h"

#define DEFAULT_NAME "Unknown"

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
          automat.Name = extractName(line).value_or(DEFAULT_NAME); continue;
        case Comment:
          automat.Comment = line; continue;
        case States:
          parseState(line);
        case Transitions:
          parseTransition(line); continue;
        case Variables:
          parseVariable(line);
      }
    }
  }
  return automat;
}

void parser::parseState(const std::string &line) {
  const auto colon = line.find(':');
  if (colon == std::string::npos) {
    std::cerr << "Can't find colon" << std::endl;
    return;
  }
  auto name = line.substr(0, colon);  //TODO trim it
  automat.addState(name, [name]() {
    std::cout << "Entering state: " << name << std::endl;
  }); //TODO ~~parse the action~~ - let compiler validate it later on
  //TODO change addState to store the action as a string instead of directly parsing it as a function
}

/// Extracts name from line, expecting format of "Name:<name>".
/// The string should be purified prior to calling this function
/// @param line Line from which name will be extracted
/// @return String containing name, otherwise nothing
std::optional<std::string> parser::extractName(const std::string &line) {
  const std::string pureLine = Utils::Purify(line);
  if (const auto colon = pureLine.find(':'); colon != std::string::npos) {
    const auto name = line.substr(colon + 1);
    return name;
  }
  return std::nullopt;
}

void parser::parseVariable(const std::string &line) {
  auto trimmed = Utils::Trim(line);
  if (auto result = ctre::match<varPattern>(trimmed)) {
    auto [whole, type, name, value] = result;   //TODO does it work?
    automat.addVariable(type, name, value);
  }
}


void parser::parseTransition(const std::string &line) {
  auto purified = Utils::Purify(line);
  // TODO test that CTRE works separately
  if (auto result = ctre::match<pattern>(purified)) {
    auto [whole, from, to, input, cond, delay] = result; //TODO does it work?

    // // Replacement in case the following doesn't work
    // // result.erase(std::remove(result.begin(), result.end(), '['), result.end());
    cond.erase(std::ranges::remove(cond, '[').begin(), cond.end());
    cond.erase(std::ranges::remove(cond, ']').begin(), cond.end());
    Utils::Remove(cond, ']');
    Utils::Remove(cond, '[');
    automat.addTransition(from, to, []() {return true;}); //TODO fix: there is no way passing input event or delay
    //TODO condition should ideally be validated by compiler, not Automat
  }
}