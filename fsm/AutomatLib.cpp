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
    addState(std::get<0>(result), std::get<1>(result));
  }

  void Automat::addTransition(const std::string &from, const std::string &to, const std::string &input, const std::string &cond, const std::string &delay) {
    transitions.push_back({from, to, input, cond, delay});
  }

  void Automat::addTransition(const Parser::ParseTransitionRecord &result) {
    addTransition(result.from, result.to, result.input, result.cond, result.delay);
  }

  void Automat::addVariable(const std::string &type, const std::string &name, const std::string &value) {
    variables[name] = std::make_pair(type, value);
  }
  void Automat::addVariable(const Parser::ParseVariableRecord &result) {
    addVariable(result.type, result.name, result.value);
  }

  //TODO move this to helper function class for AutomatRuntime
  std::optional<std::string> Automat::valueof(const std::string &name) {
    if (inputs.contains(name)) {
      return inputs[name];
    }
    return std::nullopt;
  }


  //TODO move this to helper function class for AutomatRuntime
  template <typename T>
  bool Automat::output(const std::string &outputName, const T &value) {
    if (outputs.contains(outputName)) {
      outputs[outputName] = Utils::ToStringOpt(value);
      return true;
    }
    return false;
  }
}