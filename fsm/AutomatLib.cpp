#include "AutomatLib.h"

#include <optional>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

#include "ParserLib.h"
#include "Utils.h"
#include "range/v3/view/map.hpp"

namespace AutomatLib {



void Automat::addState(const std::tuple<std::string, std::string> &result) {
  states += State{std::get<0>(result), std::get<1>(result)};
}

void Automat::addTransition(const Transition &result) {
  transitions << result;
}

void Automat::addVariable(const Variable &result) {
  variables += result;
}

void Automat::addInput(const std::string &name) { inputs.push_back(name); }

void Automat::addOutput(const std::string &name) { outputs.push_back(name); }

void Automat::PrepareIncludes() {
  oss << "#include <string>" << std::endl;
  oss << "#include <functional>" << std::endl;
  oss << "#include \"AutomatRuntime.h\"" << std::endl;
}

void Automat::PrepareHelperVariables() {
  oss << absl::StrFormat("namespace %s {", Name);
  oss << "using std::string;" << std::endl;
  oss << "using namespace AutomatRuntime;" << std::endl;
  oss << "using std::string_view;" << std::endl;
  oss << "#define INPUT(name) Runtime::RegisterSignal(in_map, name)"
      << std::endl;
  oss << "#define OUTPUT(name) Runtime::RegisterSignal(out_map, name)"
      << std::endl;
  oss << "SignalsType in_map = {};" << std::endl;
  oss << "SignalsType out_map = {};" << std::endl;
  oss << "function strtn = [](const string_view value) {return "
         "StringToNumeric(value);};"
      << std::endl;
  oss << "function valueof = [&](const string& name) {return "
         "Runtime::Load(in_map, name);};"
      << std::endl;
  oss << "template <typename T> function output = [](const string& name, const "
         "T& value) {return Runtime::Store(out_map, value, name);};"
      << std::endl;
  const auto first = states.First().Name;
  oss << absl::StrFormat("string activeState = \"%s\"", first) << std::endl;
  for (const auto &sig : inputs) {
    oss << absl::StrFormat("INPUT(\"%s\");", sig);
  }
  for (const auto &sig : outputs) {
    oss << absl::StrFormat("OUTPUT(\"%s\");", sig);
  }
  for (const auto &def : variables.Format()) {
    oss << def << std::endl;
  }
  for (const auto& [name, action] : states.GetPairs()) {
    oss << absl::StrFormat("function %s = [](){", name);
    oss << std::quoted(action);
    oss << ";};" << std::endl;
  }
}

void Automat::PrepareExecuteFunction() {
  oss << "void execute() {" << std::endl;
  oss << "bool terminate = false;" << std::endl;
  oss << "TransitionGroup group" << std::endl;
  for (auto& line : transitions.ExternalAdd("group")) {
    oss << line << std::endl;
  }
  oss << "group.GroupTransitions();" << std::endl;

  oss << "}" << std::endl;
}

}  // namespace AutomatLib