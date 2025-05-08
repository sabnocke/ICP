#include "AutomatLib.h"
#include <bits/ios_base.h>
#include <chrono>
#include <functional>
#include <iostream>
#include <optional>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

#include "Utils.h"
#include "parser.h"


namespace AutomatLib {

  void Automat::addState(const std::string &name, const std::string &action) {
    states[name] = State{name, action};
  }
  void Automat::addState(const std::tuple<std::string, std::string> &result) {
    states[std::get<0>(result)] = State{std::get<0>(result), std::get<1>(result)};
  }

  void Automat::addTransition(const std::string &from, const std::string &to, const std::string &input, const std::string &cond, const std::string &delay) {
    transitions.push_back({from, to, input, cond, delay});
  }
  template<typename T>
  void Automat::addTransition(const Parser::TransitionRecord<T> &result) {
    transitions.push_back({result.from, result.to, result.input, result.cond, result.delay});
  }

  void Automat::addVariable(const std::string &type, const std::string &name, const std::string &value) {
    variables[name] = std::make_pair(type, value);
  }

  template<typename T>
  void Automat::addVariable(const Parser::VariableRecord<T> &result) {
    variables[result.name] = std::make_pair(result.type, result.name);
  }

  void Automat::addInput(const std::string &name) {
    inputs.push_back(name);
  }

  void Automat::addOutput(const std::string &name) {
    outputs.push_back(name);
  }

  void Automat::PrepareIncludes() {
    oss << "#include <string>" << std::endl;
  }


  void Automat::PrepareHelperVariables() {
    oss << "std::string "
  }

}