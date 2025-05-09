#include "AutomatLib.h"

#include <optional>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

#include "ParserLib.h"
#include "Utils.h"

namespace AutomatLib {

  void Automat::addState(const std::tuple<std::string, std::string> &result) {
    states[std::get<0>(result)] = State{std::get<0>(result), std::get<1>(result)};
  }

  void Automat::addTransition(const ParserLib::TransitionRecord &result) {
    transitions.push_back({result.from, result.to, result.input, result.cond, std::nullopt, result.delay});
  }

  void Automat::addVariable(const ParserLib::VariableRecord &result) {
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
    oss << "std::string ";
  }

}